// Copyright(C) 2018 Tommy Hinks <tommy.hinks@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef THINKS_OBJ_IO_SIMPLE_EXAMPLE_H_INCLUDED
#define THINKS_OBJ_IO_SIMPLE_EXAMPLE_H_INCLUDED

#include <cassert>
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

#include <thinks/obj_io/obj_io.h>

namespace {

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
  Vec2 tex_coords;
  Vec3 normal;
};

struct Mesh
{
  std::vector<Vertex> vertices;
  std::vector<std::uint16_t> indices;
};

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
      mesh.vertices.push_back(VertexType{});
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
      mesh.vertices.push_back(VertexType{});
    }
    mesh.vertices[tex_count++].tex_coord = 
      Vec2{ tex.values[0], tex.values[1] };
  };

  // Normals [optional].
  auto add_normal = [&mesh, &nml_count](const auto& nml) {
    if (mesh.vertices.size() <= nml_count) {
      mesh.vertices.push_back(VertexType{});
    }
    mesh.vertices[nml_count++].normal = 
      Vec3{ nml.values[0], nml.values[1], nml.values[2] };
  };

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

void WriteMesh(const std::string& filename, const Mesh& mesh)
{

}

} // namespace

namespace examples {

void SimpleExample()
{

}

} // namespace examples

#endif // THINKS_OBJ_IO_SIMPLE_EXAMPLE_H_INCLUDED