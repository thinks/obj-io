// Copyright(C) 2018 Tommy Hinks <tommy.hinks@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <sstream>
#include <string>

#include <catch.hpp>

#include <utils/catch_utils.h>
#include <utils/read_write_utils.h>
#include <utils/type_utils.h>


TEST_CASE("read", "[container]")
{
  const auto input = std::string(
    "# comment\n"
    "" // empty line
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

  SECTION("positions")
  {
    constexpr auto use_tex_coords = false;
    constexpr auto use_normals = false;
    auto iss = std::istringstream(input);
    const auto result = utils::ReadMesh<utils::Mesh<>>(
      iss, use_tex_coords, use_normals);

    typedef decltype(result.mesh) MeshType;
    typedef typename MeshType::VertexType VertexType;
    typedef typename MeshType::IndexType IndexType;
    typedef typename VertexType::PositionType PositionType;

    auto expected_mesh = MeshType{};
    expected_mesh.vertices = std::vector<VertexType>{
      VertexType{ PositionType{ 1.f, 2.f, 3.f } },
      VertexType{ PositionType{ 4.f, 5.f, 6.f } },
      VertexType{ PositionType{ 7.f, 8.f, 9.f } }
    };
    expected_mesh.indices = std::vector<IndexType>{ 0, 1, 2, 2, 1, 0};

    REQUIRE_THAT(result.mesh, utils::MeshMatcher<MeshType>(
      expected_mesh, use_tex_coords, use_normals));
  }

  SECTION("positions and tex coords")
  {
    constexpr auto use_tex_coords = true;
    constexpr auto use_normals = false;
    auto iss = std::istringstream(input);
    const auto mesh = utils::ReadMesh<utils::Mesh<>>(
      iss, use_tex_coords, use_normals).mesh;

    typedef decltype(mesh) MeshType;
    typedef typename MeshType::VertexType VertexType;
    typedef typename MeshType::IndexType IndexType;
    typedef typename VertexType::PositionType PositionType;
    typedef typename VertexType::TexCoordType TexCoordType;

    auto expected_mesh = MeshType{};
    expected_mesh.vertices = std::vector<VertexType>{
      VertexType{ 
        PositionType{ 1.f, 2.f, 3.f },
        TexCoordType{ 0.f, 0.f }
      },
      VertexType{ 
        PositionType{ 4.f, 5.f, 6.f },
        TexCoordType{ 0.f, 1.f }
      },
      VertexType{ 
        PositionType{ 7.f, 8.f, 9.f },
        TexCoordType{ 1.f, 1.f }
      }
    };
    expected_mesh.indices = std::vector<IndexType>{ 0, 1, 2, 2, 1, 0 };

    REQUIRE_THAT(mesh, utils::MeshMatcher<MeshType>(
      expected_mesh, use_tex_coords, use_normals));
  }

  SECTION("positions and normals")
  {
    constexpr auto use_tex_coords = false;
    constexpr auto use_normals = true;
    auto iss = std::istringstream(input);
    const auto mesh = utils::ReadMesh<utils::Mesh<>>(
      iss, use_tex_coords, use_normals).mesh;

    typedef decltype(mesh) MeshType;
    typedef typename MeshType::VertexType VertexType;
    typedef typename MeshType::IndexType IndexType;
    typedef typename VertexType::PositionType PositionType;
    typedef typename VertexType::TexCoordType TexCoordType;
    typedef typename VertexType::NormalType NormalType;

    auto expected_mesh = MeshType{};
    expected_mesh.vertices = std::vector<VertexType>{
      VertexType{
      PositionType{ 1.f, 2.f, 3.f },
      TexCoordType{},
      NormalType{ 1.f, 0.f, 0.f }
    },
      VertexType{
      PositionType{ 4.f, 5.f, 6.f },
      TexCoordType{},
      NormalType{ 0.f, 1.f, 0.f }
    },
      VertexType{
      PositionType{ 7.f, 8.f, 9.f },
      TexCoordType{},
      NormalType{ 0.f, 0.f, 1.f }
    }
    };
    expected_mesh.indices = std::vector<IndexType>{ 0, 1, 2, 2, 1, 0 };

    REQUIRE_THAT(mesh, utils::MeshMatcher<MeshType>(
      expected_mesh, use_tex_coords, use_normals));
  }

  SECTION("positions and tex coords and normals")
  {
    constexpr auto use_tex_coords = true;
    constexpr auto use_normals = true;
    auto iss = std::istringstream(input);
    const auto mesh = utils::ReadMesh<utils::Mesh<>>(
      iss, use_tex_coords, use_normals).mesh;

    typedef decltype(mesh) MeshType;
    typedef typename MeshType::VertexType VertexType;
    typedef typename MeshType::IndexType IndexType;
    typedef typename VertexType::PositionType PositionType;
    typedef typename VertexType::TexCoordType TexCoordType;
    typedef typename VertexType::NormalType NormalType;

    auto expected_mesh = MeshType{};
    expected_mesh.vertices = std::vector<VertexType>{
      VertexType{
      PositionType{ 1.f, 2.f, 3.f },
      TexCoordType{ 0.f, 0.f},
      NormalType{ 1.f, 0.f, 0.f }
    },
      VertexType{
      PositionType{ 4.f, 5.f, 6.f },
      TexCoordType{ 0.f, 1.f },
      NormalType{ 0.f, 1.f, 0.f }
    },
      VertexType{
      PositionType{ 7.f, 8.f, 9.f },
      TexCoordType{ 1.f, 1.f},
      NormalType{ 0.f, 0.f, 1.f }
    }
    };
    expected_mesh.indices = std::vector<IndexType>{ 0, 1, 2, 2, 1, 0 };

    REQUIRE_THAT(mesh, utils::MeshMatcher<MeshType>(
      expected_mesh, use_tex_coords, use_normals));
  }
}

TEST_CASE("read exceptions", "[container]")
{
  SECTION("unrecognized line prefix")
  {
    const auto input = std::string(
      "bad 0 1 2");
    auto iss = std::istringstream(input);
    constexpr auto use_tex_coords = false;
    constexpr auto use_normals = false;
    REQUIRE_THROWS_MATCHES(
      utils::ReadMesh<utils::Mesh<>>(iss, use_tex_coords, use_normals),
      std::runtime_error,
      utils::ExceptionContentMatcher{ "unrecognized line prefix 'bad'" });
  }

  SECTION("position value count < 3")
  {
    const auto input = std::string(
      "v 0 1");
    auto iss = std::istringstream(input);
    constexpr auto use_tex_coords = false;
    constexpr auto use_normals = false;
    REQUIRE_THROWS_MATCHES(
      utils::ReadMesh<utils::Mesh<>>(iss, use_tex_coords, use_normals),
      std::runtime_error,
      utils::ExceptionContentMatcher{ "positions must have 3 or 4 values (found 2)" });
  }

  SECTION("position value count > 4")
  {
    const auto input = std::string(
      "v 0 1 2 3 4");
    auto iss = std::istringstream(input);
    constexpr auto use_tex_coords = false;
    constexpr auto use_normals = false;
    REQUIRE_THROWS_MATCHES(
      utils::ReadMesh<utils::Mesh<>>(iss, use_tex_coords, use_normals),
      std::runtime_error,
      utils::ExceptionContentMatcher{ "expected to parse at most 4 values" });
  }

  SECTION("zero index")
  {
    const auto input = std::string(
      "f 0 1 2");
    auto iss = std::istringstream(input);
    constexpr auto use_tex_coords = false;
    constexpr auto use_normals = false;
    REQUIRE_THROWS_MATCHES(
      utils::ReadMesh<utils::Mesh<>>(iss, use_tex_coords, use_normals),
      std::runtime_error,
      utils::ExceptionContentMatcher{ "parsed index must be greater than zero" });
  }
}
