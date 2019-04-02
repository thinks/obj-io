// Copyright(C) 2018 Tommy Hinks <tommy.hinks@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <sstream>
#include <string>

#include "catch.hpp"

#include "utils/catch_utils.h"
#include "utils/read_write_utils.h"
#include "utils/type_utils.h"

TEST_CASE("READ", "[container]") {
  using MeshType = utils::Mesh<>;
  using IndexType = MeshType::IndexType; 
  using VertexType = typename MeshType::VertexType;
  using PositionType = typename VertexType::PositionType;
  using TexCoordType = typename VertexType::TexCoordType;
  using NormalType = typename VertexType::NormalType;

  typedef utils::Mesh<> MeshType;
  typedef typename MeshType::IndexType IndexType;
  typedef typename MeshType::VertexType VertexType;
  typedef typename VertexType::PositionType PositionType;
  typedef typename VertexType::TexCoordType TexCoordType;
  typedef typename VertexType::NormalType NormalType;

  const auto input = std::string(
      "# comment\n"
      ""  // empty line
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

  SECTION("positions") {
    constexpr auto use_tex_coords = false;
    constexpr auto use_normals = false;

    auto iss = std::istringstream(input);
    const auto read_result =
        utils::ReadMesh<MeshType>(iss, use_tex_coords, use_normals);

    auto expected_mesh = MeshType{};
    expected_mesh.vertices =
        std::vector<VertexType>{VertexType{PositionType{1.f, 2.f, 3.f}},
                                VertexType{PositionType{4.f, 5.f, 6.f}},
                                VertexType{PositionType{7.f, 8.f, 9.f}}};
    expected_mesh.indices = std::vector<IndexType>{0, 1, 2, 2, 1, 0};

    REQUIRE_THAT(read_result.mesh, utils::MeshMatcher<MeshType>(
      expected_mesh, use_tex_coords, use_normals));
  }

  SECTION("positions and tex coords")
  {
    constexpr auto use_tex_coords = true;
    constexpr auto use_normals = false;

    auto iss = std::istringstream(input);
    const auto read_result = utils::ReadMesh<MeshType>(
      iss, use_tex_coords, use_normals);

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
    expected_mesh.indices = std::vector<IndexType>{ 
      0, 1, 2, 
      2, 1, 0 };

    REQUIRE_THAT(read_result.mesh, utils::MeshMatcher<MeshType>(
      expected_mesh, use_tex_coords, use_normals));
  }

  SECTION("positions and normals")
  {
    constexpr auto use_tex_coords = false;
    constexpr auto use_normals = true;

    auto iss = std::istringstream(input);
    const auto read_result = utils::ReadMesh<MeshType>(
      iss, use_tex_coords, use_normals);

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
    expected_mesh.indices = std::vector<IndexType>{ 
      0, 1, 2, 
      2, 1, 0 
    };

    REQUIRE_THAT(read_result.mesh, utils::MeshMatcher<MeshType>(
      expected_mesh, use_tex_coords, use_normals));
  }

  SECTION("positions and tex coords and normals")
  {
    constexpr auto use_tex_coords = true;
    constexpr auto use_normals = true;

    auto iss = std::istringstream(input);
    const auto read_result = utils::ReadMesh<utils::Mesh<>>(
      iss, use_tex_coords, use_normals);

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
    expected_mesh.indices = std::vector<IndexType>{ 
      0, 1, 2, 
      2, 1, 0 
    };

    REQUIRE_THAT(read_result.mesh, utils::MeshMatcher<MeshType>(
      expected_mesh, use_tex_coords, use_normals));
  }
}


TEST_CASE("READ - index group", "[container]")
{
  typedef utils::IndexGroupMesh<> MeshType;
  typedef typename MeshType::IndexType IndexType;
  typedef typename MeshType::PositionType PositionType;
  typedef typename MeshType::TexCoordType TexCoordType;
  typedef typename MeshType::NormalType NormalType;

  SECTION("positions")
  {
    constexpr auto use_tex_coords = false;
    constexpr auto use_normals = false;

    const auto input = std::string(
      "# comment\n"
      "" // empty line
      "v 1 2 3\n"
      "v 4 5 6\n"
      "v 7 8 9\n"
      "f 1 2 3\n"
      "f 3 2 1\n");

    auto iss = std::istringstream(input);
    const auto read_result = utils::ReadIndexGroupMesh<MeshType>(
      iss, use_tex_coords, use_normals);

    auto expected_mesh = MeshType{};
    expected_mesh.positions = std::vector<PositionType>{
      PositionType{ 1.f, 2.f, 3.f },
      PositionType{ 4.f, 5.f, 6.f },
      PositionType{ 7.f, 8.f, 9.f }
    };
    expected_mesh.position_indices = std::vector<IndexType>{ 
      0, 1, 2, 
      2, 1, 0
    };

    REQUIRE_THAT(read_result.mesh, utils::IndexGroupMeshMatcher<MeshType>(
      expected_mesh, use_tex_coords, use_normals));
  }

  SECTION("positions and tex coords")
  {
    constexpr auto use_tex_coords = true;
    constexpr auto use_normals = false;

    const auto input = std::string(
      "# comment\n"
      "" // empty line
      "v 1 2 3\n"
      "v 4 5 6\n"
      "v 7 8 9\n"
      "vt 0 0\n"
      "vt 0 1\n"
      "vt 1 1\n"
      "f 1/3 2/2 3/1\n"
      "f 3/1 2/2 1/3\n");

    auto iss = std::istringstream(input);
    const auto read_result = utils::ReadIndexGroupMesh<MeshType>(
      iss, use_tex_coords, use_normals);

    auto expected_mesh = MeshType{};
    expected_mesh.positions = std::vector<PositionType>{
      PositionType{ 1.f, 2.f, 3.f },
      PositionType{ 4.f, 5.f, 6.f },
      PositionType{ 7.f, 8.f, 9.f }
    };
    expected_mesh.tex_coords = std::vector<TexCoordType>{
      TexCoordType{ 0.f, 0.f },
      TexCoordType{ 0.f, 1.f },
      TexCoordType{ 1.f, 1.f }
    };
    expected_mesh.position_indices = std::vector<IndexType>{ 
      0, 1, 2, 
      2, 1, 0
    };
    expected_mesh.tex_coord_indices = std::vector<IndexType>{ 
      2, 1, 0, 
      0, 1, 2
    };

    REQUIRE_THAT(read_result.mesh, utils::IndexGroupMeshMatcher<MeshType>(
      expected_mesh, use_tex_coords, use_normals));
  }

  SECTION("positions and normals")
  {
    constexpr auto use_tex_coords = false;
    constexpr auto use_normals = true;

    const auto input = std::string(
      "# comment\n"
      "" // empty line
      "v 1 2 3\n"
      "v 4 5 6\n"
      "v 7 8 9\n"
      "vn 1 0 0\n"
      "vn 0 1 0\n"
      "vn 0 0 1\n"
      "f 1//3 2//2 3//1\n"
      "f 3//1 2//2 1//3\n");

    auto iss = std::istringstream(input);
    const auto read_result = utils::ReadIndexGroupMesh<MeshType>(
      iss, use_tex_coords, use_normals);

    auto expected_mesh = MeshType{};
    expected_mesh.positions = std::vector<PositionType>{
      PositionType{ 1.f, 2.f, 3.f },
      PositionType{ 4.f, 5.f, 6.f },
      PositionType{ 7.f, 8.f, 9.f }
    };
    expected_mesh.normals = std::vector<NormalType>{
      NormalType{ 1.f, 0.f, 0.f },
      NormalType{ 0.f, 1.f, 0.f },
      NormalType{ 0.f, 0.f, 1.f }
    };
    expected_mesh.position_indices = std::vector<IndexType>{ 
      0, 1, 2, 
      2, 1, 0
    };
    expected_mesh.normal_indices = std::vector<IndexType>{ 
      2, 1, 0, 
      0, 1, 2
    };

    REQUIRE_THAT(read_result.mesh, utils::IndexGroupMeshMatcher<MeshType>(
      expected_mesh, use_tex_coords, use_normals));
  }

  SECTION("positions and tex coords and normals")
  {
    constexpr auto use_tex_coords = true;
    constexpr auto use_normals = true;

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
      "f 1/3/3 2/2/2 3/1/1\n"
      "f 3/1/1 2/2/2 1/3/3\n");

    auto iss = std::istringstream(input);
    const auto read_result = utils::ReadIndexGroupMesh<MeshType>(
      iss, use_tex_coords, use_normals);

    auto expected_mesh = MeshType{};
    expected_mesh.positions = std::vector<PositionType>{
      PositionType{ 1.f, 2.f, 3.f },
      PositionType{ 4.f, 5.f, 6.f },
      PositionType{ 7.f, 8.f, 9.f }
    };
    expected_mesh.tex_coords = std::vector<TexCoordType>{
      TexCoordType{ 0.f, 0.f },
      TexCoordType{ 0.f, 1.f },
      TexCoordType{ 1.f, 1.f }
    };
    expected_mesh.normals = std::vector<NormalType>{
      NormalType{ 1.f, 0.f, 0.f },
      NormalType{ 0.f, 1.f, 0.f },
      NormalType{ 0.f, 0.f, 1.f }
    };
    expected_mesh.position_indices = std::vector<IndexType>{ 
      0, 1, 2, 
      2, 1, 0
    };
    expected_mesh.tex_coord_indices = std::vector<IndexType>{ 
      2, 1, 0, 
      0, 1, 2
    };
    expected_mesh.normal_indices = std::vector<IndexType>{ 
      2, 1, 0, 
      0, 1, 2
    };

    REQUIRE_THAT(read_result.mesh, utils::IndexGroupMeshMatcher<MeshType>(
      expected_mesh, use_tex_coords, use_normals));
  }
}


TEST_CASE("READ - unrecognized line prefix")
{
  typedef utils::Mesh<> MeshType;

  constexpr auto use_tex_coords = false;
  constexpr auto use_normals = false;

  const auto input = std::string(
    "bad 0 1 2\n");
  auto iss = std::istringstream(input);

  REQUIRE_THROWS_MATCHES(
    utils::ReadMesh<MeshType>(iss, use_tex_coords, use_normals),
    std::runtime_error,
    utils::ExceptionContentMatcher{ "unrecognized line prefix 'bad'" });
}


TEST_CASE("READ - position errors", "[container]")
{
  typedef utils::Mesh<> MeshType;
  typedef typename MeshType::VertexType VertexType;
  typedef typename VertexType::PositionType PositionType;

  constexpr auto use_tex_coords = false;
  constexpr auto use_normals = false;

  SECTION("position value count < 3")
  {
    const auto input = std::string(
      "v 0 1\n");
    auto iss = std::istringstream(input);

    REQUIRE_THROWS_MATCHES(
      utils::ReadMesh<MeshType>(iss, use_tex_coords, use_normals),
      std::runtime_error,
      utils::ExceptionContentMatcher{ 
        "positions must have 3 or 4 values (found 2)" });
  }

  SECTION("position value count > size")
  {
    static_assert(utils::VecSize<PositionType>::value == 3,
      "position size must be 3");

    const auto input = std::string(
      "v 0 1 2 3\n");
    auto iss = std::istringstream(input);

    REQUIRE_THROWS_MATCHES(
      utils::ReadMesh<MeshType>(iss, use_tex_coords, use_normals),
      std::runtime_error,
      utils::ExceptionContentMatcher{ "expected to parse at most 3 values" });
  }
}


TEST_CASE("READ - face errors", "[container]")
{
  constexpr auto use_tex_coords = false;
  constexpr auto use_normals = false;

  SECTION("incomplete face")
  {
    typedef utils::Mesh<> MeshType;

    const auto input = std::string(
      "f 1 2\n");
    auto iss = std::istringstream(input);

    REQUIRE_THROWS_MATCHES(
      utils::ReadMesh<MeshType>(iss, use_tex_coords, use_normals),
      std::runtime_error,
      utils::ExceptionContentMatcher{ 
        "expected 3 face indices (found 2)" });
  }

  SECTION("invalid polygon")
  {
    typedef std::uint32_t IndexType;
    constexpr auto kIndicesPerFace = std::size_t{ 5 };
    typedef utils::Mesh<utils::Vertex<>, IndexType, kIndicesPerFace> MeshType;

    const auto input = std::string(
      "f 1 2\n");
    auto iss = std::istringstream(input);

    REQUIRE_THROWS_MATCHES(
      utils::ReadMesh<MeshType>(iss, use_tex_coords, use_normals),
      std::runtime_error,
      utils::ExceptionContentMatcher{ 
        "faces must have at least 3 indices (found 2)" });
  }
}


TEST_CASE("READ - texture coordinate errors", "[container]")
{
  typedef utils::Mesh<> MeshType;
  typedef typename MeshType::VertexType VertexType;
  typedef typename VertexType::TexCoordType TexCoordType;

  constexpr auto use_tex_coords = true;
  constexpr auto use_normals = false;

  SECTION("texture coordinate value count < 2")
  {
    const auto input = std::string(
      "vt 0\n");
    auto iss = std::istringstream(input);

    REQUIRE_THROWS_MATCHES(
      utils::ReadMesh<MeshType>(iss, use_tex_coords, use_normals),
      std::runtime_error,
      utils::ExceptionContentMatcher{ 
        "texture coordinates must have 2 or 3 values (found 1)" });
  }

  SECTION("texture coordinate value count > size")
  {
    static_assert(utils::VecSize<TexCoordType>::value == 2,
      "tex coord size must be 2");

    const auto input = std::string(
      "vt 0.0 0.5 1.0\n");
    auto iss = std::istringstream(input);

    REQUIRE_THROWS_MATCHES(
      utils::ReadMesh<MeshType>(iss, use_tex_coords, use_normals),
      std::runtime_error,
      utils::ExceptionContentMatcher{ "expected to parse at most 2 values" });
  }

  SECTION("texture coordinate value < 0")
  {
    const auto input = std::string(
      "vt -0.1 0.0\n");
    auto iss = std::istringstream(input);

    REQUIRE_THROWS_MATCHES(
      utils::ReadMesh<MeshType>(iss, use_tex_coords, use_normals),
      std::runtime_error,
      utils::ExceptionContentMatcher{ 
        "texture coordinate values must be in range [0, 1] (found -0.1)" });
  }

  SECTION("texture coordinate value > 1")
  {
    const auto input = std::string(
      "vt 0.0 1.1\n");
    auto iss = std::istringstream(input);

    REQUIRE_THROWS_MATCHES(
      utils::ReadMesh<MeshType>(iss, use_tex_coords, use_normals),
      std::runtime_error,
      utils::ExceptionContentMatcher{ 
        "texture coordinate values must be in range [0, 1] (found 1.1)" });
  }
}


TEST_CASE("READ - normal errors", "[container]")
{
  typedef utils::Mesh<> MeshType;
  typedef typename MeshType::VertexType VertexType;
  typedef typename VertexType::NormalType NormalType;

  constexpr auto use_tex_coords = false;
  constexpr auto use_normals = true;

  SECTION("normal value count < 3")
  {
    const auto input = std::string(
      "vn 0 1\n");
    auto iss = std::istringstream(input);

    REQUIRE_THROWS_MATCHES(
      utils::ReadMesh<MeshType>(iss, use_tex_coords, use_normals),
      std::runtime_error,
      utils::ExceptionContentMatcher{ 
        "normals must have 3 values (found 2)" });
  }

  SECTION("normal value count > 3")
  {
    const auto input = std::string(
      "vn 0 1 2 3\n");
    auto iss = std::istringstream(input);

    REQUIRE_THROWS_MATCHES(
      utils::ReadMesh<MeshType>(iss, use_tex_coords, use_normals),
      std::runtime_error,
      utils::ExceptionContentMatcher{ "expected to parse at most 3 values" });
  }
}


TEST_CASE("READ - default values", "[container]")
{
  typedef utils::Vec4<float> PositionType;
  typedef utils::Vec3<float> TexCoordType;
  typedef utils::Vertex<PositionType, TexCoordType> VertexType;
  typedef utils::Mesh<VertexType> MeshType;

  SECTION("position w defaults to 1")
  {
    constexpr auto use_tex_coords = false;
    constexpr auto use_normals = false;

    const auto input = std::string(
      "v 0.1 0.2 0.3\n");
    auto iss = std::istringstream(input);

    const auto read_result =
      utils::ReadMesh<MeshType>(iss, use_tex_coords, use_normals);

    REQUIRE(utils::Equals(
      read_result.mesh.vertices[0].pos, 
      PositionType{ 0.1f, 0.2f, 0.3f, 1.f }));
  }

  SECTION("texture coordinate w defaults to 1")
  {
    constexpr auto use_tex_coords = true;
    constexpr auto use_normals = false;

    const auto input = std::string(
      "v 0.1 0.2 0.3\n"
      "vt 0.1 0.2\n");
    auto iss = std::istringstream(input);

    const auto read_result =
      utils::ReadMesh<MeshType>(iss, use_tex_coords, use_normals);

    REQUIRE(utils::Equals(
      read_result.mesh.vertices[0].tex, 
      TexCoordType{ 0.1f, 0.2f, 1.f }));
  }
}


TEST_CASE("READ - parse value error")
{
  typedef utils::Mesh<> MeshType;

  constexpr auto use_tex_coords = false;
  constexpr auto use_normals = false;

  // Note - Not testing this for all types of attributes.
  const auto input = std::string(
    "v 1 2 xxx\n");
  auto iss = std::istringstream(input);

  REQUIRE_THROWS_MATCHES(
    utils::ReadMesh<MeshType>(iss, use_tex_coords, use_normals),
    std::runtime_error,
    utils::ExceptionContentMatcher{ "failed parsing 'xxx'" });
}


TEST_CASE("READ - index range", "[container]")
{
  typedef utils::Vec3<float> PositionType;
  typedef utils::Vec2<float> TexCoordType;
  typedef utils::Vec3<float> NormalType;
  typedef utils::Vec3<float> ColorType;
  typedef utils::Vertex<PositionType, TexCoordType, NormalType, ColorType> VertexType;
  typedef std::int16_t IndexType;
  typedef utils::Mesh<VertexType, IndexType> MeshType;

  constexpr auto use_tex_coords = false;
  constexpr auto use_normals = false;

  SECTION("zero index")
  {
    const auto input = std::string(
      "f 0 1 2\n");
    auto iss = std::istringstream(input);

    REQUIRE_THROWS_MATCHES(
      utils::ReadMesh<MeshType>(iss, use_tex_coords, use_normals),
      std::runtime_error,
      utils::ExceptionContentMatcher{ "parsed index must be greater than zero" });
  }
}


TEST_CASE("READ - index group errors", "[container]")
{
  typedef utils::IndexGroupMesh<> MeshType;

  constexpr auto use_tex_coords = false;
  constexpr auto use_normals = false;

  SECTION("empty position index")
  {
    const auto input = std::string(
      "f 1 2 /3\n");
    auto iss = std::istringstream(input);

    REQUIRE_THROWS_MATCHES(
      utils::ReadIndexGroupMesh<MeshType>(iss, use_tex_coords, use_normals),
      std::runtime_error,
      utils::ExceptionContentMatcher{ "empty position index ('/3')" });
  }

  SECTION("empty normal index")
  {
    const auto input = std::string(
      "f 1 2 3/3/\n");
    auto iss = std::istringstream(input);

    REQUIRE_THROWS_MATCHES(
      utils::ReadIndexGroupMesh<MeshType>(iss, use_tex_coords, use_normals),
      std::runtime_error,
      utils::ExceptionContentMatcher{ "empty normal index ('3/3/')" });
  }

  SECTION("token count > 3")
  {
    const auto input = std::string(
      "f 1 2 1/2/3/4\n");
    auto iss = std::istringstream(input);

    REQUIRE_THROWS_MATCHES(
      utils::ReadIndexGroupMesh<MeshType>(iss, use_tex_coords, use_normals),
      std::runtime_error,
      utils::ExceptionContentMatcher{ 
        "index group can have at most 3 tokens ('1/2/3/4')" });
  }
}
