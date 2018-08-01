# OBJ-IO
This repository contains a [single-file](https://github.com/thinks/obj-io/blob/master/include/thinks/obj_io/obj_io.h), header-only, no-dependencies C++ implementation of the [OBJ file format](https://en.wikipedia.org/wiki/Wavefront_.obj_file). All code in this repository is released under the [MIT license](https://en.wikipedia.org/wiki/MIT_License), as per the included [license file](https://github.com/thinks/obj-io/blob/master/LICENSE). The code herein has not been optimized for speed, but rather for generality, readability, and robustness.  

Invariably, those in need of the provided tools are likely to have written simple functions for writing (and/or reading) OBJ files at some point. Those utilities, however, were probably hard-coded for the mesh class at hand and not easily generalizable. Even though the OBJ format is embarrasingly simple there are a few pit-falls (did you forget that OBJ files use one-based indexing?). The goal here is to use the type system available in C++ to make it so that writing erroneous code becomes exceedingly difficult. The price to pay for this is learning yet another API, which in this case translates to a handful of functions, as shown in the examples below. 

## The OBJ File Format
The [OBJ file format](https://en.wikipedia.org/wiki/Wavefront_.obj_file) is ubiquitous in the field of computer graphics. While it is arguably not the most efficient way to store meshes on disk, its simplicity has made it widely supported. The OBJ file format is extremely useful for debugging and for transferring meshes between different software packages.

It should be noted that we currently only support _geometric vertices_ (i.e. positions), _texture coordinates_, _normals_, and _face elements_. These are the most fundamental properties of meshes and should cover the vast majority of use cases.  

## Examples
Mesh representations vary wildly across different frameworks. It seems fairly likely that most frameworks have their own representation. Because of this, our distribution provide methods for reading and writing OBJ files assuming no knowledge of a mesh class. Instead, our methods rely on callbacks that feed the methods with the required information. As such, our methods act more as middle-ware than some out-of-the-box solution. While this approach requires some additional work for users, it provides great flexibility and arguably makes this distribution more usable in the long run.

A simple example illustrates how to read and write a mesh using our method. Let's assume we have the following simple mesh class.
```cpp
struct Vec2 
{
  float x;
  float y;
};

struct Vec3
{
  float x;
  float y;
  float z;
};

struct Vertex
{
  Vec3 position;
  Vec2 tex_coord;
  Vec3 normal;
};

struct Mesh
{
  std::vector<Vertex> vertices;
  std::vector<uint16_t> indices;
};
```
This type of layout is common since it fits nicely with several widely used APIs for uploading a mesh to the GPU for rendering. Now, let's assume that we have an OBJ file from which we want to populate a mesh. A simple implementation could be as follows.
```cpp
//#include relevant std headers.

#include <thinks/obj_io/obj_io.h>

Mesh ReadMesh(const std::string& filename)
{
  using namespace std;

  auto mesh = Mesh{};

  // We cannot assume the order in which callbacks are invoked, 
  // so we need to keep track of which vertex to add properties to.
  // The first encountered position gets added to the first vertex, etc.
  auto pos_count = uint32_t{ 0 };
  auto tex_count = uint32_t{ 0 };
  auto nml_count = uint32_t{ 0 };

  // Positions.
  auto add_position = [&mesh, &pos_count](const auto& pos) {
    // Check if we need a new vertex.
    if (mesh.vertices.size() <= pos_count) {
      mesh.vertices.push_back(Vertex{});
    }

    // Write the position property of current vertex and 
    // set position index to next vertex. Values are translated
    // from OBJ representation to our vector class.
    mesh.vertices[pos_count++].position = Vec3{ pos.values[0], pos.values[1], pos.values[2] };
  };

  // Faces.
  auto add_face = [&mesh](const auto& face) {
    assert(face.values.size() == 3 && "expecting only triangle faces");

    // Add triangle indices into the linear storage of our mesh class.
    for (const auto index : face.values) {
      mesh.indices.push_back(index.position_index.value);
    }
  };

  // Texture coordinates [optional].
  auto add_tex_coord = [&mesh, &tex_count](const auto& tex) {
    if (mesh.vertices.size() <= tex_count) {
      mesh.vertices.push_back(Vertex{});
    }
    mesh.vertices[tex_count++].tex_coord = Vec2{ tex.values[0], tex.values[1] };
  };

  // Normals [optional].
  auto add_normal = [&mesh, &nml_count](const auto& nml) {
    if (mesh.vertices.size() <= nml_count) {
      mesh.vertices.push_back(Vertex{});
    }
    mesh.vertices[nml_count++].normal = Vec3{ nml.values[0], nml.values[1], nml.values[2] };
  };

  // Open the OBJ file and populate the mesh while parsing it.
  // Note that we provide the MakeAddFunc with the type to use 
  // while parsing. In this case the type is the same as that of 
  // the storage in our mesh class.
  auto ifs = ifstream(filename);
  assert(ifs);
  const auto result = thinks::obj_io::Read(
    ifs,
    thinks::obj_io::MakeAddFunc<float>(add_position),
    thinks::obj_io::MakeAddFunc<uint16_t>(add_face),
    thinks::obj_io::MakeAddFunc<float>(add_tex_coord),
    thinks::obj_io::MakeAddFunc<float>(add_normal));
  ifs.close();

  // Some sanity checks.
  assert(
    result.position_count == result.tex_coord_count &&
    result.position_count == result.normal_count &&
    "incomplete vertices in file");
  assert(result.position_count == mesh.vertices.size() &&
    "bad position count");
  assert(result.tex_coord_count == mesh.vertices.size() &&
    "bad tex_coord count");
  assert(result.normal_count == mesh.vertices.size() &&
    "bad normal count");
  assert(
    pos_count == tex_count && pos_count == nml_count &&
    "all vertices must be completely initialized");

  return mesh;
}

``` 
A nice feature of reading a mesh this way is that we avoid memory spikes. The mesh data is never duplicated, as it might be if the `Read` method were to build its own internal representation of the mesh. Also, note that the `Read` method has no knowledge of the `Mesh` class itself, it simply calls the provided lambdas while parsing the OBJ file. Writing a mesh is done in a similar fashion.
```cpp
//#include relevant std headers.

#include <thinks/obj_io/obj_io.h>

void WriteMesh(const std::string& filename, const Mesh& mesh)
{
  using namespace std;

  const auto vtx_iend = end(mesh.vertices);

  // Mappers have two responsibilities:
  // (1) - Iterating over a certain attribute of the mesh (e.g. positions).
  // (2) - Translating from users types to OBJ types (e.g. Vec3 -> Position<float, 3>)

  // Positions.
  auto pos_vtx_iter = begin(mesh.vertices);
  auto pos_mapper = [&pos_vtx_iter, vtx_iend]() {
    typedef thinks::obj_io::Position<float, 3> ObjPositionType;

    if (pos_vtx_iter == vtx_iend) {
      // End indicates that no further calls should be made to this mapper,
      // in this case because the captured iterator has reached the end
      // of the vector.
      return thinks::obj_io::End<ObjPositionType>();
    }

    // Map indicates that additional positions may be available after this one.
    const auto pos = (*pos_vtx_iter++).position;
    return thinks::obj_io::Map(ObjPositionType(pos.x, pos.y, pos.z));
  };

  // Faces.
  auto idx_iter = mesh.indices.begin();
  const auto idx_iend = mesh.indices.end();
  auto face_mapper = [&idx_iter, idx_iend]() {
    typedef thinks::obj_io::Index<uint16_t> ObjIndexType;
    typedef thinks::obj_io::TriangleFace<ObjIndexType> ObjFaceType;

    // Check that there are 3 more indices (trailing indices handled below).
    if (distance(idx_iter, idx_iend) < 3) {
      return thinks::obj_io::End<ObjFaceType>();
    }

    // Create a face from the mesh indices.
    const auto idx0 = ObjIndexType(*idx_iter++);
    const auto idx1 = ObjIndexType(*idx_iter++);
    const auto idx2 = ObjIndexType(*idx_iter++);
    return thinks::obj_io::Map(ObjFaceType(idx0, idx1, idx2));
  };

  // Texture coordinates [optional]. 
  auto tex_vtx_iter = begin(mesh.vertices);
  auto tex_mapper = [&tex_vtx_iter, vtx_iend]() {
    typedef thinks::obj_io::TexCoord<float, 2> ObjTexCoordType;

    if (tex_vtx_iter == vtx_iend) {
      return thinks::obj_io::End<ObjTexCoordType>();
    }

    const auto tex = (*tex_vtx_iter++).tex_coord;
    return thinks::obj_io::Map(ObjTexCoordType(tex.x, tex.y));
  };

  // Normals [optional]. 
  auto nml_vtx_iter = begin(mesh.vertices);
  auto nml_mapper = [&nml_vtx_iter, vtx_iend]() {
    typedef thinks::obj_io::Normal<float> ObjNormalType;

    if (nml_vtx_iter == vtx_iend) {
      return thinks::obj_io::End<ObjNormalType>();
    }

    const auto nml = (*nml_vtx_iter++).normal;
    return thinks::obj_io::Map(ObjNormalType(nml.x, nml.y, nml.z));
  };

  // Open the OBJ file and pass in the mappers, which will be called 
  // internally to write the contents of the mesh to the file.
  auto ofs = ofstream(filename);
  assert(ofs);
  const auto result = thinks::obj_io::Write(
    ofs,
    pos_mapper,
    face_mapper,
    tex_mapper,
    nml_mapper);
  ofs.close();

  // Some sanity checks.
  assert(result.position_count == mesh.vertices.size() &&
    "bad position count");
  assert(result.tex_coord_count == mesh.vertices.size() &&
    "bad position count");
  assert(result.normal_count == mesh.vertices.size() &&
    "bad normal count");
  assert(result.face_count == mesh.indices.size() / 3 &&
    "bad face count");
  assert(idx_iter == idx_iend && "trailing indices");
}
```
Again, the `Write` method has no direct knowledge of the `Mesh` class. The relevant information is provided through the lambdas that are passed in. A complete code example using the above methods can be found in the [examples](https://github.com/thinks/obj-io/tree/master/examples) folder. More advanced mesh I/O utilities built on top of the provided framework can be found in the [test/utils/read_write_utils.h](https://github.com/thinks/obj-io/blob/master/test/utils/read_write_utils.h) file.

## Tests
The tests for this distribution are written in the [catch2](https://github.com/catchorg/Catch2) framework. A [snapshot](https://github.com/thinks/obj-io/blob/master/test/catch.hpp) of the single header version of catch2 is included in this repository. 

Running the tests is simple. In a terminal do the following (and similar for `Debug`):
```bash
$ cd d:
$ git clone git@github.com:/thinks/obj-io.git D:/obj-io
$ mkdir build-obj-io
$ cd build-obj-io
$ cmake ../obj-io
$ cmake --build . --config Release
$ ctest . -C Release
```
For more detailed test output locate the test executable (_thinks_obj_io_test.exe_) in the build tree and run it directly.


## Future Work
* _Improved read performance_ - The current implementation is rather naive in that it reads only a single line at a time. Additionally, many operations are done using `std::string` operations, which is not ideal performance-wise.
* _Optional validation_ - It would be nice to have optional mechanisms to perform validation such as checking that face indices are within the range of the other attributes. However, this has recieved low priority since it can easily be done by the user before/after reading/writing.
