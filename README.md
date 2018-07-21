# obj-io
This repository contains a single-file, header-only, no-dependencies C++ implementation of the [OBJ file format](https://en.wikipedia.org/wiki/Wavefront_.obj_file). All code in this repository is released under the [MIT license](https://en.wikipedia.org/wiki/MIT_License), as per the included [license file](https://github.com/thinks/obj-io/blob/master/LICENSE).

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
#include <cassert>
#include <cstdint>
#include <fstream>
#include <string>

#include <thinks/obj_io/obj_io.h>

Mesh ReadMesh(const std::string& filename)
{
  using namespace std;
  using thinks::obj_io::MakeAddFunc;
  using thinks::obj_io::Read;

  auto mesh = Mesh{};
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
  Read(
    ifs,
    MakeAddFunc<float>(add_position), 
    MakeAddFunc<uint16_t>(add_face),
    MakeAddFunc<float>(add_tex_coord),
    MakeAddFunc<float>(add_normal));
  ifs.close();

  assert(pos_count == tex_count && pos_count == nml_count && 
    "all vertices must be completely initialized");

  return mesh;
}
``` 
A nice feature of reading a mesh this way is that we avoid memory spikes. The mesh data is never duplicated, as it might be if the `Read` method built its own internal representation of the mesh. Also, note that the `Read` method has no knowledge of the `Mesh` class itself, it simply calls the provided lambdas while parsing the OBJ file. 

## Tests
use catch2[link to github], header included in this repo.
CTest

## Future Work

* Add [optional] mesh validation, check indices in range.

* vertices must be same size, 3 or 4 elements.
* all primitives must be same size, greater than or equal to 3, triangle and up.
* does not support materials.

