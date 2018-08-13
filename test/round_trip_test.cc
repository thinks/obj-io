// Copyright(C) 2018 Tommy Hinks <tommy.hinks@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <sstream>
#include <vector>

#include <catch.hpp>

#include <utils/catch_utils.h>
#include <utils/read_write_utils.h>
#include <utils/type_utils.h>


TEST_CASE("round trip")
{
  typedef utils::Mesh<
    utils::Vertex<
      utils::Vec4<float>,
      utils::Vec3<float>,
      utils::Vec3<float>,
      utils::Vec4<float>>, 
    std::uint32_t> MeshType;
  typedef typename MeshType::VertexType VertexType;
  typedef typename VertexType::PositionType PositionType;
  typedef typename VertexType::TexCoordType TexCoordType;
  typedef typename VertexType::NormalType NormalType;
  typedef typename MeshType::IndexType IndexType;

  // Setup.
  auto mesh = MeshType{};
  mesh.vertices = std::vector<VertexType>{
    VertexType{
      PositionType{ 1.f, 2.f, 3.f, .1f },
      TexCoordType{ 0.f, 0.f, .1f },
      NormalType{ 1.f, 0.f, 0.f }
    },
    VertexType{
      PositionType{ 4.f, 5.f, 6.f, .2f },
      TexCoordType{ 0.f, 1.f, .2f },
      NormalType{ 0.f, 1.f, 0.f }
    },
    VertexType{
      PositionType{ 7.f, 8.f, 9.f, .3f },
      TexCoordType{ 1.f, 1.f, .3f },
      NormalType{ 0.f, 0.f, 1.f }
    }
  };
  mesh.indices = std::vector<IndexType>{ 0, 1, 2, 2, 1, 0 };

  constexpr auto use_tex_coords = true;
  constexpr auto use_normals = true;

  // Write.
  const auto write_result = 
    utils::WriteMesh(mesh, use_tex_coords, use_normals);

  // Read.
  auto iss = std::istringstream(write_result.mesh_str);
  const auto read_result = 
    utils::ReadMesh<MeshType>(iss, use_tex_coords, use_normals);

  REQUIRE_THAT(read_result.mesh, utils::MeshMatcher<MeshType>(
    mesh, use_tex_coords, use_normals));
}


TEST_CASE("round trip index groups")
{
  typedef utils::IndexGroupMesh<
    utils::Vec4<float>,
    utils::Vec3<float>,
    utils::Vec3<float>,
    std::uint32_t> MeshType;
  typedef typename MeshType::PositionType PositionType;
  typedef typename MeshType::TexCoordType TexCoordType;
  typedef typename MeshType::NormalType NormalType;
  typedef typename MeshType::IndexType IndexType;

  // Setup.
  auto imesh = MeshType{};
  imesh.positions = std::vector<PositionType>{
    PositionType{ 1.f, 2.f, 3.f, .1f },
    PositionType{ 4.f, 5.f, 6.f, .2f },
    PositionType{ 7.f, 8.f, 9.f, .3f }
  };
  imesh.position_indices = std::vector<IndexType>{
    0, 1, 2,
    2, 1, 0
  };
  imesh.tex_coords = std::vector<TexCoordType>{
    TexCoordType{ .1f, .2f, .3f },
    TexCoordType{ .4f, .5f, .6f }
  };
  imesh.tex_coord_indices = std::vector<IndexType>{
    0, 0, 0,
    1, 1, 1
  };
  imesh.normals = std::vector<NormalType>{
    NormalType{ .6f, .5f, .4f },
    NormalType{ .3f, .2f, .1f }
  };
  imesh.normal_indices = std::vector<IndexType>{
    1, 1, 1,
    0, 0, 0
  };

  constexpr auto use_tex_coords = true;
  constexpr auto use_normals = true;

  // Write.
  const auto write_result = 
    utils::WriteIndexGroupMesh(imesh, use_tex_coords, use_normals);

  // Read.
  auto iss = std::istringstream(write_result.mesh_str);
  const auto read_result = 
    utils::ReadIndexGroupMesh<MeshType>(iss, use_tex_coords, use_normals);

  REQUIRE_THAT(read_result.mesh, utils::IndexedMeshMatcher<MeshType>(
    imesh, use_tex_coords, use_normals));
}
