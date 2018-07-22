# obj-io
This repository contains a single-file, header-only, no-dependencies C++ implementation of the [OBJ file format](https://en.wikipedia.org/wiki/Wavefront_.obj_file). All code in this repository is released under the [MIT license](https://en.wikipedia.org/wiki/MIT_License), as per the included [license file](https://github.com/thinks/obj-io/blob/master/LICENSE). The code herein has not been optimized for speed, but rather for readability, robustness, and generality.  

## The OBJ File Format
The [OBJ file format](https://en.wikipedia.org/wiki/Wavefront_.obj_file) is commonly used throughout the field of computer graphics. While it is arguably not the most efficient way to store meshes on disk, the fact that it is widely supported has made it ubiquitous. The OBJ file format is extremely useful for debugging and for transferring meshes between different software packages.

## Examples
Mesh representations vary wildly across different frameworks. It seems fairly likely that most frameworks have their own representation. Because of this, the methods provided for reading and writing OBJ files assume no knowledge of a mesh class. Instead, the methods rely on callbacks provided by users that provide the methods with the required information. As such, the methods act more as middle-ware than some out-of-the-box solution. While this approach requires some additional work for users, it provides great flexibility and arguably makes this distribution more usable in the long run.

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
This type of layout is common since it fits nicely with how mesh data can be easily uploaded to the GPU for rendering. Now, let's assume that we have an OBJ file from which we want to populate a mesh. A simple implementation could be as follows.
```cpp
//#include relevant std headers.

#include <thinks/obj_io/obj_io.h>

Mesh ReadMesh(const std::string& filename)
{
  using namespace std;

  auto mesh = Mesh{};

  // We cannot assume the order in which callbacks are invoked, 
  // so we need to keep track of which vertex to add properties to.
  auto pos_count = uint32_t{ 0 };
  auto tex_count = uint32_t{ 0 };
  auto nml_count = uint32_t{ 0 };

  // Positions.
  auto add_position = [&mesh, &pos_count](const auto& pos) {
    if (mesh.vertices.size() <= pos_count) {
      mesh.vertices.push_back(Vertex{});
    }
    mesh.vertices[pos_count++].position = 
      Vec3{ pos.values[0], pos.values[1], pos.values[2] };
  };

  // Faces.
  auto add_face = [&mesh](const auto& face) {
    assert(face.values.size() == 3 && "expecting only triangle faces");
    for (const auto index : face.values) {
      mesh.indices.push_back(index.position_index.value);
    }
  };

  // Texture coordinates [optional].
  auto add_tex_coord = [&mesh, &tex_count](const auto& tex) {
    if (mesh.vertices.size() <= tex_count) {
      mesh.vertices.push_back(Vertex{});
    }
    mesh.vertices[tex_count++].tex_coord = 
      Vec2{ tex.values[0], tex.values[1] };
  };

  // Normals [optional].
  auto add_normal = [&mesh, &nml_count](const auto& nml) {
    if (mesh.vertices.size() <= nml_count) {
      mesh.vertices.push_back(Vertex{});
    }
    mesh.vertices[nml_count++].normal = 
      Vec3{ nml.values[0], nml.values[1], nml.values[2] };
  };

  // Open the OBJ file and populate the mesh while parsing it.
  auto ifs = ifstream(filename);
  assert(ifs);
  thinks::obj_io::Read(
    ifs,
    thinks::obj_io::MakeAddFunc<float>(add_position), 
    thinks::obj_io::MakeAddFunc<uint16_t>(add_face),
    thinks::obj_io::MakeAddFunc<float>(add_tex_coord),
    thinks::obj_io::MakeAddFunc<float>(add_normal));
  ifs.close();

  assert(pos_count == tex_count && pos_count == nml_count && 
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

  const auto vtx_iend = std::end(mesh.vertices);

  // Positions.
  auto pos_vtx_iter = begin(mesh.vertices);
  auto pos_mapper = [&pos_vtx_iter, vtx_iend]() {
    typedef thinks::obj_io::Position<float, 3> ObjPositionType;

    if (pos_vtx_iter == vtx_iend) {
      return thinks::obj_io::End<ObjPositionType>();
    }

    const auto pos = (*pos_vtx_iter++).position;
    return thinks::obj_io::Map(ObjPositionType(pos.x, pos.y, pos.z));
  };

  // Faces.
  auto idx_iter = mesh.indices.begin();
  const auto idx_iend = mesh.indices.end();
  auto face_mapper = [&idx_iter, idx_iend]() {
    typedef thinks::obj_io::Index<uint16_t> ObjIndexType;
    typedef thinks::obj_io::TriangleFace<ObjIndexType> ObjFaceType;

    if (distance(idx_iter, idx_iend) < 3) {
      return thinks::obj_io::End<ObjFaceType>();
    }

    const auto idx0 = ObjIndexType(*idx_iter++);
    const auto idx1 = ObjIndexType(*idx_iter++);
    const auto idx2 = ObjIndexType(*idx_iter++);
    return thinks::obj_io::Map(ObjFaceType(idx0, idx1, idx2));
  };

  // Texture coordinates. [optional]
  auto tex_vtx_iter = begin(mesh.vertices);
  auto tex_mapper = [&tex_vtx_iter, vtx_iend]() {
    typedef thinks::obj_io::TexCoord<float, 2> ObjTexCoordType;

    if (tex_vtx_iter == vtx_iend) {
      return thinks::obj_io::End<ObjTexCoordType>();
    }

    const auto tex = (*tex_vtx_iter++).tex_coord;
    return thinks::obj_io::Map(ObjTexCoordType(tex.x, tex.y));
  };

  // Normals. [optional]
  auto nml_vtx_iter = begin(mesh.vertices);
  auto nml_mapper = [&nml_vtx_iter, vtx_iend]() {
    typedef thinks::obj_io::Normal<float> ObjNormalType;

    if (nml_vtx_iter == vtx_iend) {
      return thinks::obj_io::End<ObjNormalType>();
    }

    const auto nml = (*nml_vtx_iter++).normal;
    return thinks::obj_io::Map(ObjNormalType(nml.x, nml.y, nml.z));
  };

  auto ofs = ofstream(filename);
  assert(ofs);
  thinks::obj_io::Write(
    ofs,
    pos_mapper, 
    face_mapper,
    tex_mapper,
    nml_mapper);
  ofs.close();

  assert(idx_iter == idx_iend && "trailing indices");
}
```
Again, the `Write` method has no direct knowledge of the `Mesh` class. The relevant information is provided through the lambdas that are passed in. A complete code example using the above methods can be found in the [examples](https://github.com/thinks/obj-io/tree/master/examples) folder. A more advanced framework built on top of the provided framework can be found in the [test/utils](https://github.com/thinks/obj-io/tree/master/test/utils) folder.

## Tests
The tests for this distribution are written in the [catch2](https://github.com/catchorg/Catch2) framework. A snapshot of the single header version of catch2 is included in this repository. 


CTest

## Future Work

* Add [optional] mesh validation, check indices in range.

* vertices must be same size, 3 or 4 elements.
* all primitives must be same size, greater than or equal to 3, triangle and up.
* does not support materials.

