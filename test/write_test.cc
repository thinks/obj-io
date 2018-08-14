// Copyright(C) 2018 Tommy Hinks <tommy.hinks@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <exception>
#include <string>
#include <vector>

#include <catch.hpp>

#include <utils/catch_utils.h>
#include <utils/read_write_utils.h>
#include <utils/type_utils.h>


TEST_CASE("write", "[container]")
{
  typedef utils::TriangleMesh<> MeshType;
  typedef typename MeshType::IndexType IndexType;
  typedef typename MeshType::VertexType VertexType;
  typedef typename VertexType::PositionType PositionType;
  typedef typename VertexType::TexCoordType TexCoordType;
  typedef typename VertexType::NormalType NormalType;

  // Setup.
  auto mesh = MeshType{};
  mesh.vertices = std::vector<VertexType>{
    VertexType{
      PositionType{ 1.f, 2.f, 3.f },
      TexCoordType{ 0.f, 0.f },
      NormalType{ 1.f, 0.f, 0.f }
    },
    VertexType{
      PositionType{ 4.f, 5.f, 6.f },
      TexCoordType{ 0.f, 1.f },
      NormalType{ 0.f, 1.f, 0.f }
    },
    VertexType{
      PositionType{ 7.f, 8.f, 9.f },
      TexCoordType{ 1.f, 1.f },
      NormalType{ 0.f, 0.f, 1.f }
    }
  };
  mesh.indices = std::vector<IndexType>{ 0, 1, 2, 2, 1, 0 };

  SECTION("positions")
  {
    // Arrange.
    const auto expected_string = std::string(
      "# Written by https://github.com/thinks/obj-io\n"
      "v 1 2 3\n"
      "v 4 5 6\n"
      "v 7 8 9\n"
      "f 1 2 3\n"
      "f 3 2 1\n"); 

    // Act.
    constexpr auto write_tex = false;
    constexpr auto write_nml = false;
    const auto write_result = utils::WriteMesh(mesh, write_tex, write_nml);

    // Assert.
    REQUIRE(expected_string == write_result.mesh_str);
  }

  SECTION("positions and tex coords")
  {
    // Arrange.
    const auto expected_string = std::string(
      "# Written by https://github.com/thinks/obj-io\n"
      "v 1 2 3\n"
      "v 4 5 6\n"
      "v 7 8 9\n"
      "vt 0 0\n"
      "vt 0 1\n"
      "vt 1 1\n"
      "f 1 2 3\n"
      "f 3 2 1\n");

    // Act.
    constexpr auto write_tex = true;
    constexpr auto write_nml = false;
    const auto write_result = utils::WriteMesh(mesh, write_tex, write_nml);

    // Assert.
    REQUIRE(expected_string == write_result.mesh_str);
  }

  SECTION("positions and normals")
  {
    // Arrange.
    const auto expected_string = std::string(
      "# Written by https://github.com/thinks/obj-io\n"
      "v 1 2 3\n"
      "v 4 5 6\n"
      "v 7 8 9\n"
      "vn 1 0 0\n"
      "vn 0 1 0\n"
      "vn 0 0 1\n"
      "f 1 2 3\n"
      "f 3 2 1\n");

    // Act.
    constexpr auto write_tex = false;
    constexpr auto write_nml = true;
    const auto write_result = utils::WriteMesh(mesh, write_tex, write_nml);

    // Assert.
    REQUIRE(expected_string == write_result.mesh_str);
  }

  SECTION("positions and tex coords and normals") 
  {
    // Arrange.
    const auto expected_string = std::string(
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
      "f 3 2 1\n");

    // Act.
    constexpr auto write_tex = true;
    constexpr auto write_nml = true;
    const auto write_result = utils::WriteMesh(mesh, write_tex, write_nml);

    // Assert.
    REQUIRE(expected_string == write_result.mesh_str);
  }
}


TEST_CASE("write group index", "[container]")
{
  typedef utils::IndexGroupMesh<> MeshType;
  typedef typename MeshType::PositionType PositionType;
  typedef typename MeshType::TexCoordType TexCoordType;
  typedef typename MeshType::NormalType NormalType;
  typedef typename MeshType::IndexType IndexType;

  // Setup.
  auto imesh = MeshType{};
  imesh.positions = std::vector<PositionType>{
    PositionType{ 1.f, 2.f, 3.f },
    PositionType{ 4.f, 5.f, 6.f },
    PositionType{ 7.f, 8.f, 9.f }
  };
  imesh.position_indices = std::vector<IndexType>{ 
    0, 1, 2, 
    2, 1, 0 
  };
  imesh.tex_coords = std::vector<TexCoordType>{
    TexCoordType{ 0.f, 0.f }, 
    TexCoordType{ 1.f, 1.f }
  };
  imesh.tex_coord_indices = std::vector<IndexType>{ 
    0, 0, 0, 
    1, 1, 1 
  };
  imesh.normals = std::vector<NormalType>{ 
    NormalType{ 0.f, 0.f, -1.f },
    NormalType{ 0.f, 0.f, 1.f }
  };
  imesh.normal_indices = std::vector<IndexType>{
    1, 1, 1, 
    0, 0, 0 
  };

  SECTION("positions")
  {
    // Arrange.
    const auto expected_string = std::string(
      "# Written by https://github.com/thinks/obj-io\n"
      "v 1 2 3\n"
      "v 4 5 6\n"
      "v 7 8 9\n"
      "f 1 2 3\n"
      "f 3 2 1\n");

    // Act.
    constexpr auto write_tex = false;
    constexpr auto write_nml = false;
    const auto write_result = utils::WriteIndexGroupMesh(imesh, write_tex, write_nml);

    // Assert.
    REQUIRE(expected_string == write_result.mesh_str);
  }

  SECTION("positions and tex coords")
  {
    // Arrange.
    const auto expected_string = std::string(
      "# Written by https://github.com/thinks/obj-io\n"
      "v 1 2 3\n"
      "v 4 5 6\n"
      "v 7 8 9\n"
      "vt 0 0\n"
      "vt 1 1\n"
      "f 1/1 2/1 3/1\n"
      "f 3/2 2/2 1/2\n");

    // Act.
    constexpr auto write_tex = true;
    constexpr auto write_nml = false;
    const auto write_result = utils::WriteIndexGroupMesh(imesh, write_tex, write_nml);

    // Assert.
    REQUIRE(expected_string == write_result.mesh_str);
  }

  SECTION("positions and normals")
  {
    // Arrange.
    const auto expected_string = std::string(
      "# Written by https://github.com/thinks/obj-io\n"
      "v 1 2 3\n"
      "v 4 5 6\n"
      "v 7 8 9\n"
      "vn 0 0 -1\n"
      "vn 0 0 1\n"
      "f 1//2 2//2 3//2\n"
      "f 3//1 2//1 1//1\n");

    // Act.
    constexpr auto write_tex = false;
    constexpr auto write_nml = true;
    const auto write_result = utils::WriteIndexGroupMesh(imesh, write_tex, write_nml);

    // Assert.
    REQUIRE(expected_string == write_result.mesh_str);
  }

  SECTION("positions and tex coords and normals")
  {
    // Arrange.
    const auto expected_string = std::string(
      "# Written by https://github.com/thinks/obj-io\n"
      "v 1 2 3\n"
      "v 4 5 6\n"
      "v 7 8 9\n"
      "vt 0 0\n"
      "vt 1 1\n"
      "vn 0 0 -1\n"
      "vn 0 0 1\n"
      "f 1/1/2 2/1/2 3/1/2\n"
      "f 3/2/1 2/2/1 1/2/1\n");

    // Act.
    constexpr auto write_tex = true;
    constexpr auto write_nml = true;
    const auto write_result = utils::WriteIndexGroupMesh(imesh, write_tex, write_nml);

    // Assert.
    REQUIRE(expected_string == write_result.mesh_str);
  }
}


TEST_CASE("write quads")
{
  // Four indices per face.
  typedef utils::IndexGroupMesh<
    utils::Vec4<float>,
    utils::Vec3<float>,
    utils::Vec3<float>,
    std::uint16_t, 4> MeshType;
  typedef typename MeshType::IndexType IndexType;
  typedef typename MeshType::PositionType PositionType;
  typedef typename MeshType::TexCoordType TexCoordType;
  typedef typename MeshType::NormalType NormalType;

  // Setup.
  auto imesh = MeshType{};
  imesh.positions = std::vector<PositionType>{
    PositionType{ 1.f, 2.f, 3.f, 1.f },
    PositionType{ 4.f, 5.f, 6.f, 1.f },
    PositionType{ 7.f, 8.f, 9.f, 1.f },
    PositionType{ 10.f, 11.f, 12.f, 1.f }
  };
  imesh.position_indices = std::vector<IndexType>{ 
    0, 1, 2, 3,
    3, 2, 1, 0 
  };
  imesh.tex_coords = std::vector<TexCoordType>{
    TexCoordType{ 0.f, .5f, 0.f },
    TexCoordType{ .5f, 0.f, 0.f }, 
    TexCoordType{ 0.f, 1.f, 0.f }, 
    TexCoordType{ 1.f, 0.f, 0.f }
  };
  imesh.tex_coord_indices = std::vector<IndexType>{ 
    0, 1, 2, 3,
    3, 2, 1, 0 
  };
  imesh.normals = std::vector<NormalType>{ 
    NormalType{ 1.f, 0.f, 0.f },
    NormalType{ -1.f, 0.f, 0.f },
    NormalType{ 0.f, 1.f, 0.f },
    NormalType{ 0.f, -1.f, 0.f }
  };
  imesh.normal_indices = std::vector<IndexType>{
    0, 1, 2, 3,
    3, 2, 1, 0 
  };

  const auto expected_string = std::string(
    "# Written by https://github.com/thinks/obj-io\n"
    "v 1 2 3 1\n"
    "v 4 5 6 1\n"
    "v 7 8 9 1\n"
    "v 10 11 12 1\n"
    "vt 0 0.5 0\n"
    "vt 0.5 0 0\n"
    "vt 0 1 0\n"
    "vt 1 0 0\n"
    "vn 1 0 0\n"
    "vn -1 0 0\n"
    "vn 0 1 0\n"
    "vn 0 -1 0\n"
    "f 1/1/1 2/2/2 3/3/3 4/4/4\n"
    "f 4/4/4 3/3/3 2/2/2 1/1/1\n");

  constexpr auto write_tex = true;
  constexpr auto write_nml = true;
  const auto write_result = utils::WriteIndexGroupMesh(imesh, write_tex, write_nml);

  REQUIRE(expected_string == write_result.mesh_str);
}


TEST_CASE("write polygons")
{
  // Five indices per face.
  typedef utils::IndexGroupMesh<
    utils::Vec4<float>,
    utils::Vec3<float>,
    utils::Vec3<float>,
    std::uint16_t, 5> MeshType;
  typedef typename MeshType::IndexType IndexType;
  typedef typename MeshType::PositionType PositionType;
  typedef typename MeshType::TexCoordType TexCoordType;
  typedef typename MeshType::NormalType NormalType;

  // Setup.
  auto imesh = MeshType{};
  imesh.positions = std::vector<PositionType>{
    PositionType{ 1.f, 2.f, 3.f, 1.f },
    PositionType{ 4.f, 5.f, 6.f, 1.f },
    PositionType{ 7.f, 8.f, 9.f, 1.f },
    PositionType{ 10.f, 11.f, 12.f, 1.f },
    PositionType{ 13.f, 14.f, 15.f, 1.f }
  };
  imesh.position_indices = std::vector<IndexType>{ 
    0, 1, 2, 3, 4,
    4, 3, 2, 1, 0 
  };
  imesh.tex_coords = std::vector<TexCoordType>{
    TexCoordType{ 0.f, 0.f, 0.f }, 
    TexCoordType{ 0.f, .5f, 0.f },
    TexCoordType{ .5f, 0.f, 0.f }, 
    TexCoordType{ 0.f, 1.f, 0.f }, 
    TexCoordType{ 1.f, 0.f, 0.f }
  };
  imesh.tex_coord_indices = std::vector<IndexType>{ 
    0, 1, 2, 3, 4,
    4, 3, 2, 1, 0 
  };
  imesh.normals = std::vector<NormalType>{ 
    NormalType{ 1.f, 0.f, 0.f },
    NormalType{ -1.f, 0.f, 0.f },
    NormalType{ 0.f, 1.f, 0.f },
    NormalType{ 0.f, -1.f, 0.f },
    NormalType{ 0.f, 0.f, 1.f }
  };
  imesh.normal_indices = std::vector<IndexType>{
    0, 1, 2, 3, 4,
    4, 3, 2, 1, 0 
  };

  const auto expected_string = std::string(
    "# Written by https://github.com/thinks/obj-io\n"
    "v 1 2 3 1\n"
    "v 4 5 6 1\n"
    "v 7 8 9 1\n"
    "v 10 11 12 1\n"
    "v 13 14 15 1\n"
    "vt 0 0 0\n"
    "vt 0 0.5 0\n"
    "vt 0.5 0 0\n"
    "vt 0 1 0\n"
    "vt 1 0 0\n"
    "vn 1 0 0\n"
    "vn -1 0 0\n"
    "vn 0 1 0\n"
    "vn 0 -1 0\n"
    "vn 0 0 1\n"
    "f 1/1/1 2/2/2 3/3/3 4/4/4 5/5/5\n"
    "f 5/5/5 4/4/4 3/3/3 2/2/2 1/1/1\n");

  constexpr auto write_tex = true;
  constexpr auto write_nml = true;
  const auto write_result = utils::WriteIndexGroupMesh(imesh, write_tex, write_nml);

  REQUIRE(expected_string == write_result.mesh_str);
}


TEST_CASE("write texture coordinate range", "[container]")
{
  typedef utils::Mesh<> MeshType;
  typedef typename MeshType::VertexType VertexType;
  typedef typename MeshType::IndexType IndexType;
  typedef typename VertexType::PositionType PositionType;
  typedef typename VertexType::TexCoordType TexCoordType;
  typedef typename VertexType::NormalType NormalType;

  constexpr auto write_tex = true;
  constexpr auto write_nml = false;

  SECTION("texture coordinate range < 0")
  {
    auto mesh = MeshType{};
    mesh.vertices = std::vector<VertexType>{
      VertexType{
        PositionType{},
        TexCoordType{ -0.1f, 0.f },
        NormalType{}
      }
    };
    mesh.indices = std::vector<IndexType>{ 0, 0, 0 };

    REQUIRE_THROWS_MATCHES(
      utils::WriteMesh(mesh, write_tex, write_nml),
      std::runtime_error,
      utils::ExceptionContentMatcher{
        "texture coordinate values must be in range [0, 1] (found -0.1)" });
  }

  SECTION("texture coordinate range > 1")
  {
    auto mesh = MeshType{};
    mesh.vertices = std::vector<VertexType>{
      VertexType{
        PositionType{},
        TexCoordType{ 0.f, 1.1f },
        NormalType{}
      }
    };
    mesh.indices = std::vector<IndexType>{ 0, 0, 0 };

    REQUIRE_THROWS_MATCHES(
      utils::WriteMesh(mesh, write_tex, write_nml),
      std::runtime_error,
      utils::ExceptionContentMatcher{
        "texture coordinate values must be in range [0, 1] (found 1.1)" });
  }
}


TEST_CASE("write index range", "[container]")
{
  // Signed index type.
  typedef utils::Mesh<utils::Vertex<>, std::int8_t, 3> MeshType;
  typedef typename MeshType::VertexType VertexType;
  typedef typename MeshType::IndexType IndexType;
  typedef typename VertexType::PositionType PositionType;
  typedef typename VertexType::TexCoordType TexCoordType;
  typedef typename VertexType::NormalType NormalType;

  constexpr auto write_tex = false;
  constexpr auto write_nml = false;

  SECTION("negative index")
  {
    auto mesh = MeshType{};
    mesh.indices = std::vector<IndexType>{ 0, 1, -1 };

    REQUIRE_THROWS_MATCHES(
      utils::WriteMesh(mesh, write_tex, write_nml),
      std::runtime_error,
      utils::ExceptionContentMatcher{ "invalid index: -1" });
  }

  SECTION("max index")
  {
    auto mesh = MeshType{};
    mesh.indices = std::vector<IndexType>{ 0, 1, 127 };

    REQUIRE_THROWS_MATCHES(
      utils::WriteMesh(mesh, write_tex, write_nml),
      std::runtime_error,
      utils::ExceptionContentMatcher{ "invalid index: 127" });
  }
}


TEST_CASE("write polygon index count")
{
  // Only two indices per face.
  typedef utils::Mesh<utils::Vertex<>, std::uint16_t, 2> MeshType;
  typedef typename MeshType::IndexType IndexType;

  constexpr auto write_tex = false;
  constexpr auto write_nml = false;

  auto mesh = MeshType{};
  mesh.indices = std::vector<IndexType>{ 0, 1 };

  REQUIRE_THROWS_MATCHES(
    utils::WriteMesh(mesh, write_tex, write_nml),
    std::runtime_error,
    utils::ExceptionContentMatcher{ "faces must have at least 3 indices (found 2)" });
}
