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
  Vec2 tex_coord;
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

void WriteMesh(const std::string& filename, const Mesh& mesh)
{
  using namespace std;

  const auto vtx_iend = std::end(mesh.vertices);

  // Mappers have two responsibilities:
  // (1) - Iterating over a certain attribute of the mesh (e.g. positions).
  // (2) - Translating from users types to writable types (e.g. Vec3 -> Position<float, 3>)

  // Positions.
  auto pos_vtx_iter = begin(mesh.vertices);
  auto pos_mapper = [&pos_vtx_iter, vtx_iend]() {
    typedef thinks::obj_io::Position<float, 3> ObjPositionType;

    if (pos_vtx_iter == vtx_iend) {
      // End indicates that no further need to be made to this mapper.
      return thinks::obj_io::End<ObjPositionType>();
    }

    // Map indicates that more positions may be available after this one.
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

} // namespace

namespace examples {

void SimpleExample()
{
  auto mesh = Mesh{
    std::vector<Vertex>{
      Vertex {
        Vec3{ 1.f, 0.f, 0.f }, // position
        Vec2{ 1.f, 0.f },      // tex_coord
        Vec3{ 0.f, 0.f, 1.f }  // normal
      },
      Vertex {
        Vec3{ 0.f, 1.f, 0.f },
        Vec2{ 0.f, 1.f },
        Vec3{ 0.f, 0.f, 1.f }
      },
      Vertex {
        Vec3{ 1.f, 1.f, 0.f },
        Vec2{ 1.f, 1.f },
        Vec3{ 0.f, 0.f, 1.f }
      },
    },
    std::vector<std::uint16_t>{ 0, 1, 2 }
  };
  const auto filename = "D:/tmp/simple_example.obj";
  WriteMesh(filename, mesh);
  auto mesh2 = ReadMesh(filename);
}

} // namespace examples

#endif // THINKS_OBJ_IO_SIMPLE_EXAMPLE_H_INCLUDED