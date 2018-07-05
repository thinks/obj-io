// Copyright(C) 2018 Tommy Hinks <tommy.hinks@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <thinks/obj_io/obj_io.h>
#include <types.h>

#include <cassert>
#include <iostream>
#include <sstream>
#include <string>

#include <catch.hpp>


using std::istringstream;
using std::istream;
using std::string;
using std::uint32_t;

using thinks::obj_io::Read;


namespace {

template<
  typename FloatT, 
  typename IntT,
  typename AddPosition,
  typename AddFace,
  typename AddTexCoord,
  typename AddNormal>
void ReadHelper(
  istream& is,
  AddPosition add_position,
  AddFace add_face,
  AddTexCoord add_tex_coord,
  AddNormal add_normal,
  const bool read_tex_coords,
  const bool read_normals)
{
  if (!read_tex_coords && !read_normals)
  {
    Read<FloatT, IntT>(
      is,
      add_position,
      add_face);
  }
  else if (read_tex_coords && !read_normals) {
    Read<FloatT, IntT>(
      is,
      add_position,
      add_face,
      add_tex_coord);
  }
  else if (!read_tex_coords && read_normals) {
    Read<FloatT, IntT>(
      is,
      add_position,
      add_face,
      nullptr,
      add_normal);
  }
  else {
    Read<FloatT, IntT>(
      is,
      add_position,
      add_face,
      add_tex_coord,
      add_normal);
  }
}

template <typename FloatT, typename IntT>
Mesh ReadMesh(
  istream& is,
  const bool read_tex_coords,
  const bool read_normals)
{
  auto mesh = Mesh{};
  auto pos_count = uint32_t{ 0 };
  auto tex_count = uint32_t{ 0 };
  auto nml_count = uint32_t{ 0 };

  // Positions.
  auto add_position = [&mesh, &pos_count](const auto& pos) {
    if (mesh.vertices.size() <= pos_count) {
      mesh.vertices.push_back(Vertex{});
    }
    mesh.vertices[pos_count].pos.x = pos.values[0];
    mesh.vertices[pos_count].pos.y = pos.values[1];
    mesh.vertices[pos_count].pos.z = pos.values[2];
    ++pos_count;
  };

  // Tex coords.
  auto add_tex_coord = [&mesh, &tex_count](const auto& tex) {
    if (mesh.vertices.size() <= tex_count) {
      mesh.vertices.push_back(Vertex{});
    }
    mesh.vertices[tex_count].tex.x = tex.values[0];
    mesh.vertices[tex_count].tex.y = tex.values[1];
    ++tex_count;
  };

  // Normals.
  auto add_normal = [&mesh, &nml_count](const auto& nml) {
    if (mesh.vertices.size() <= nml_count) {
      mesh.vertices.push_back(Vertex{});
    }
    mesh.vertices[nml_count].normal.x = nml.values[0];
    mesh.vertices[nml_count].normal.y = nml.values[1];
    mesh.vertices[nml_count].normal.z = nml.values[2];
    ++nml_count;
  };

  // Faces.
  auto add_face = [&mesh](const auto& face) {
    assert(face.values.size() == 3 && "expected triangles");
    mesh.tri_indices.push_back(face.values[0].position_index.value);
    mesh.tri_indices.push_back(face.values[1].position_index.value);
    mesh.tri_indices.push_back(face.values[2].position_index.value);
  };

  ReadHelper<FloatT, IntT>(
    is, 
    add_position, 
    add_face, 
    add_tex_coord, 
    add_normal, 
    read_tex_coords, 
    read_normals);

  assert(pos_count == tex_count);
  assert(pos_count == nml_count);

  return mesh;
}

bool Equals(const Vec2f& lhs, const Vec2f& rhs)
{
  return lhs.x == rhs.x && lhs.y == rhs.y;
}

bool Equals(const Vec3f& lhs, const Vec3f& rhs)
{
  return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}

} // namespace


TEST_CASE("read", "[container]")
{
  const string input =
    "# Written by https://github.com/thinks/obj-io\n"
    "v 1 2 3\n"
    "v 4 5 6\n"
    "v 7 8 9\n"
    "vt 0 0\n"
    "vt 0 1\n"
    "vt 1 1\n"
    "vn 1 0 0\n"
    "vn 0 1 0\n"
    "vn 0 0 1\n"
    "f 1 2 3\n"
    "f 3 2 1\n";

  SECTION("positions")
  {
    const auto read_tex_coords = false;
    const auto read_normals = false;
    auto iss = istringstream(input);
    const auto mesh = ReadMesh<float, uint32_t>(iss, read_tex_coords, read_normals);
    REQUIRE(Equals(mesh.vertices[0].pos, Vec3f{ 1.f, 2.f, 3.f }));
    REQUIRE(Equals(mesh.vertices[1].pos, Vec3f{ 4.f, 5.f, 6.f }));
    REQUIRE(Equals(mesh.vertices[2].pos, Vec3f{ 7.f, 8.f, 9.f }));
    REQUIRE((
      mesh.tri_indices[0] == 0 &&
      mesh.tri_indices[1] == 1 &&
      mesh.tri_indices[2] == 2));
    REQUIRE((
      mesh.tri_indices[3] == 2 &&
      mesh.tri_indices[4] == 1 &&
      mesh.tri_indices[5] == 0));

  }
}
