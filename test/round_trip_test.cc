// Copyright(C) 2018 Tommy Hinks <tommy.hinks@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <sstream>
#include <vector>

#include "catch2/catch.hpp"
#include "catch_mesh_matcher.h"
#include "mesh_types.h"
#include "read_write_utils.h"

namespace {

TEST_CASE("ROUND_TRIP") {
  using PositionType = Vec4<float>;
  using TexCoordType = Vec3<float>;
  using NormalType = Vec3<float>;
  using ColorType = Vec3<float>;
  using VertexType =
      Vertex<PositionType, TexCoordType, NormalType, ColorType>;
  using IndexType = std::uint32_t;
  using MeshType = Mesh<VertexType, IndexType>;

  // Setup.
  auto mesh = MeshType{};
  mesh.vertices = std::vector<VertexType>{
      VertexType{PositionType{1.f, 2.f, 3.f, .1f}, 
                 TexCoordType{0.f, 0.f, .1f},
                 NormalType{1.f, 0.f, 0.f}},
      VertexType{PositionType{4.f, 5.f, 6.f, .2f}, 
                 TexCoordType{0.f, 1.f, .2f},
                 NormalType{0.f, 1.f, 0.f}},
      VertexType{PositionType{7.f, 8.f, 9.f, .3f}, 
                 TexCoordType{1.f, 1.f, .3f},
                 NormalType{0.f, 0.f, 1.f}}};
  mesh.indices = std::vector<IndexType>{
      0, 1, 2, 
      2, 1, 0};

  constexpr auto use_tex_coords = true;
  constexpr auto use_normals = true;

  // Write.
  const auto write_result = WriteMesh(mesh, use_tex_coords, use_normals);

  // Read.
  auto iss = std::istringstream(write_result.mesh_str);
  const auto read_result =
      ReadMesh<MeshType>(iss, use_tex_coords, use_normals);

  REQUIRE_THAT(read_result.mesh,
               MeshMatcher<MeshType>(mesh, use_tex_coords, use_normals));
}

TEST_CASE("ROUND_TRIP - index groups") {
  using PositionType = Vec4<float>;
  using TexCoordType = Vec3<float>;
  using NormalType = Vec3<float>;
  using IndexType = std::uint32_t;
  using MeshType =
      IndexGroupMesh<PositionType, TexCoordType, NormalType, IndexType>;

  // Setup.
  auto mesh = MeshType{};
  mesh.positions = std::vector<PositionType>{
      PositionType{1.f, 2.f, 3.f, .1f},
      PositionType{4.f, 5.f, 6.f, .2f},
      PositionType{7.f, 8.f, 9.f, .3f}};
  mesh.position_indices = std::vector<IndexType>{
      0, 1, 2, 
      2, 1, 0};
  mesh.tex_coords = std::vector<TexCoordType>{
      TexCoordType{.1f, .2f, .3f},
      TexCoordType{.4f, .5f, .6f}};
  mesh.tex_coord_indices = std::vector<IndexType>{
      0, 0, 0, 
      1, 1, 1};
  mesh.normals = std::vector<NormalType>{
      NormalType{.6f, .5f, .4f},
      NormalType{.3f, .2f, .1f}};
  mesh.normal_indices = std::vector<IndexType>{
      1, 1, 1, 
      0, 0, 0};

  constexpr auto use_tex_coords = true;
  constexpr auto use_normals = true;

  // Write.
  const auto write_result =
      WriteIndexGroupMesh(mesh, use_tex_coords, use_normals);

  // Read.
  auto iss = std::istringstream(write_result.mesh_str);
  const auto read_result =
      ReadIndexGroupMesh<MeshType>(iss, use_tex_coords, use_normals);

  REQUIRE_THAT(read_result.mesh, IndexGroupMeshMatcher<MeshType>(
                                     mesh, use_tex_coords, use_normals));
}

} // namespace
