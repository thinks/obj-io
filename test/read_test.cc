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
      "bad 0 1 2\n");
    auto iss = std::istringstream(input);
    constexpr auto use_tex_coords = false;
    constexpr auto use_normals = false;
    REQUIRE_THROWS_MATCHES(
      utils::ReadMesh<utils::Mesh<>>(iss, use_tex_coords, use_normals),
      std::runtime_error,
      utils::ExceptionContentMatcher{ "unrecognized line prefix 'bad'" });
  }

  SECTION("parse failure")
  {
    // Note - Not testing this for all types of attributes.
    const auto input = std::string(
      "v 1 2 xxx\n");
    auto iss = std::istringstream(input);
    constexpr auto use_tex_coords = false;
    constexpr auto use_normals = false;
    REQUIRE_THROWS_MATCHES(
      utils::ReadMesh<utils::Mesh<>>(iss, use_tex_coords, use_normals),
      std::runtime_error,
      utils::ExceptionContentMatcher{ "failed parsing 'xxx'" });
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
      utils::ExceptionContentMatcher{ 
        "positions must have 3 or 4 values (found 2)" });
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
      utils::ExceptionContentMatcher{ "expected to parse at most 3 values" });
  }

  SECTION("tex coord value count < 2")
  {
    const auto input = std::string(
      "vt 0");
    auto iss = std::istringstream(input);
    constexpr auto use_tex_coords = true;
    constexpr auto use_normals = false;
    REQUIRE_THROWS_MATCHES(
      utils::ReadMesh<utils::Mesh<>>(iss, use_tex_coords, use_normals),
      std::runtime_error,
      utils::ExceptionContentMatcher{ 
        "texture coordinates must have 2 or 3 values (found 1)" });
  }

  SECTION("too many tex coord values")
  {
    const auto input = std::string(
      "vt 0.0 0.5 1.0\n");
    auto iss = std::istringstream(input);
    constexpr auto use_tex_coords = true;
    constexpr auto use_normals = false;
    REQUIRE_THROWS_MATCHES(
      utils::ReadMesh<utils::Mesh<>>(iss, use_tex_coords, use_normals),
      std::runtime_error,
      utils::ExceptionContentMatcher{ "expected to parse at most 2 values" });
  }

  SECTION("normal value count < 3")
  {
    const auto input = std::string(
      "vn 0 1\n");
    auto iss = std::istringstream(input);
    constexpr auto use_tex_coords = false;
    constexpr auto use_normals = true;
    REQUIRE_THROWS_MATCHES(
      utils::ReadMesh<utils::Mesh<>>(iss, use_tex_coords, use_normals),
      std::runtime_error,
      utils::ExceptionContentMatcher{ 
        "normals must have 3 values (found 2)" });
  }

  SECTION("normal value count > 3")
  {
    const auto input = std::string(
      "vn 0 1 2 3");
    auto iss = std::istringstream(input);
    constexpr auto use_tex_coords = false;
    constexpr auto use_normals = true;
    REQUIRE_THROWS_MATCHES(
      utils::ReadMesh<utils::Mesh<>>(iss, use_tex_coords, use_normals),
      std::runtime_error,
      utils::ExceptionContentMatcher{ "expected to parse at most 3 values" });
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

  SECTION("incomplete face")
  {
    const auto input = std::string(
      "f 1 2\n");
    auto iss = std::istringstream(input);
    constexpr auto use_tex_coords = false;
    constexpr auto use_normals = false;
    REQUIRE_THROWS_MATCHES(
      utils::ReadMesh<utils::Mesh<>>(iss, use_tex_coords, use_normals),
      std::runtime_error,
      utils::ExceptionContentMatcher{ "incomplete face" });
  }

  SECTION("empty position index")
  {
    const auto input = std::string(
      "f 1 2 /3\n");
    auto iss = std::istringstream(input);
    constexpr auto use_tex_coords = false;
    constexpr auto use_normals = false;
    REQUIRE_THROWS_MATCHES(
      utils::ReadIndexedMesh<utils::IndexedMesh<>>(iss, use_tex_coords, use_normals),
      std::runtime_error,
      utils::ExceptionContentMatcher{ "empty position index ('/3')" });
  }

  SECTION("empty normal index")
  {
    const auto input = std::string(
      "f 1 2 3/3/\n");
    auto iss = std::istringstream(input);
    constexpr auto use_tex_coords = false;
    constexpr auto use_normals = false;
    REQUIRE_THROWS_MATCHES(
      utils::ReadIndexedMesh<utils::IndexedMesh<>>(iss, use_tex_coords, use_normals),
      std::runtime_error,
      utils::ExceptionContentMatcher{ "empty normal index ('3/3/')" });
  }

  SECTION("token count > 3")
  {
    const auto input = std::string(
      "f 1 2 1/2/3/4\n");
    auto iss = std::istringstream(input);
    constexpr auto use_tex_coords = false;
    constexpr auto use_normals = false;
    REQUIRE_THROWS_MATCHES(
      utils::ReadIndexedMesh<utils::IndexedMesh<>>(iss, use_tex_coords, use_normals),
      std::runtime_error,
      utils::ExceptionContentMatcher{ 
        "index group can have at most 3 tokens ('1/2/3/4')" });
  }
}
