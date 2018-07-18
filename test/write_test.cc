// Copyright(C) 2018 Tommy Hinks <tommy.hinks@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <utils/catch_utils.h>
#include <utils/read_write_utils.h>
#include <utils/type_utils.h>

#include <exception>
#include <string>
#include <vector>

#include <catch.hpp>


using std::runtime_error;
using std::string;
using std::uint8_t;
using std::uint32_t;
using std::vector;


TEST_CASE("write", "[container]")
{
  typedef utils::TriangleMesh<utils::Vertex<>, std::uint32_t> MeshType;
  typedef typename MeshType::VertexType VertexType;
  typedef typename VertexType::PositionType PositionType;
  typedef typename VertexType::TexCoordType TexCoordType;
  typedef typename VertexType::NormalType NormalType;
  typedef typename MeshType::IndexType IndexType;

  // Setup.
  auto mesh = MeshType{};
  mesh.vertices = vector<VertexType>{
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
  mesh.indices = vector<IndexType>{ 0, 1, 2, 2, 1, 0 };

  SECTION("positions")
  {
    // Arrange.
    const string expected_string =
      "# Written by https://github.com/thinks/obj-io\n"
      "v 1 2 3\n"
      "v 4 5 6\n"
      "v 7 8 9\n"
      "f 1 2 3\n"
      "f 3 2 1\n"; 

    // Act.
    const auto write_tex = false;
    const auto write_nml = false;
    const auto mesh_string = utils::WriteMesh(mesh, write_tex, write_nml);

    // Assert.
    REQUIRE(expected_string == mesh_string);
  }

  SECTION("positions and tex coords")
  {
    // Arrange.
    const string expected_string =
      "# Written by https://github.com/thinks/obj-io\n"
      "v 1 2 3\n"
      "v 4 5 6\n"
      "v 7 8 9\n"
      "vt 0 0\n"
      "vt 0 1\n"
      "vt 1 1\n"
      "f 1 2 3\n"
      "f 3 2 1\n";

    // Act.
    const auto write_tex = true;
    const auto write_nml = false;
    const auto mesh_string = utils::WriteMesh(mesh, write_tex, write_nml);

    // Assert.
    REQUIRE(expected_string == mesh_string);
  }

  SECTION("positions and normals")
  {
    // Arrange.
    const string expected_string =
      "# Written by https://github.com/thinks/obj-io\n"
      "v 1 2 3\n"
      "v 4 5 6\n"
      "v 7 8 9\n"
      "vn 1 0 0\n"
      "vn 0 1 0\n"
      "vn 0 0 1\n"
      "f 1 2 3\n"
      "f 3 2 1\n";

    // Act.
    const auto write_tex = false;
    const auto write_nml = true;
    const auto mesh_string = utils::WriteMesh(mesh, write_tex, write_nml);
    
    // Assert.
    REQUIRE(expected_string == mesh_string);
  }

  SECTION("positions and tex coords and normals") 
  {
    // Arrange.
    const string expected_string =
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

    // Act.
    const auto write_tex = true;
    const auto write_nml = true;
    const auto mesh_string = utils::WriteMesh(mesh, write_tex, write_nml);

    // Assert.
    REQUIRE(expected_string == mesh_string);
  }
}

TEST_CASE("write indexed", "[container]")
{
  typedef utils::IndexedMesh<> MeshType;
  typedef typename MeshType::PositionType PositionType;
  typedef typename MeshType::TexCoordType TexCoordType;
  typedef typename MeshType::NormalType NormalType;
  typedef typename MeshType::IndexType IndexType;

  // Setup.
  auto imesh = MeshType{};
  imesh.positions = vector<PositionType>{
    PositionType{ 1.f, 2.f, 3.f },
    PositionType{ 4.f, 5.f, 6.f },
    PositionType{ 7.f, 8.f, 9.f }
  };
  imesh.position_indices = vector<IndexType>{ 
    0, 1, 2, 
    2, 1, 0 
  };
  imesh.tex_coords = vector<TexCoordType>{
    TexCoordType{ 0.f, 0.f }, 
    TexCoordType{ 1.f, 1.f }
  };
  imesh.tex_coord_indices = vector<IndexType>{ 
    0, 0, 0, 
    1, 1, 1 
  };
  imesh.normals = vector<NormalType>{ 
    NormalType{ 0.f, 0.f, -1.f },
    NormalType{ 0.f, 0.f, 1.f }
  };
  imesh.normal_indices = vector<IndexType>{
    1, 1, 1, 
    0, 0, 0 
  };

  SECTION("positions")
  {
    // Arrange.
    const string expected_string =
      "# Written by https://github.com/thinks/obj-io\n"
      "v 1 2 3\n"
      "v 4 5 6\n"
      "v 7 8 9\n"
      "f 1 2 3\n"
      "f 3 2 1\n";

    // Act.
    const auto write_tex = false;
    const auto write_nml = false;
    const auto mesh_string = utils::WriteIndexedMesh(imesh, write_tex, write_nml);

    // Assert.
    REQUIRE(expected_string == mesh_string);
  }

  SECTION("positions and indexed tex coords")
  {
    // Arrange.
    const string expected_string =
      "# Written by https://github.com/thinks/obj-io\n"
      "v 1 2 3\n"
      "v 4 5 6\n"
      "v 7 8 9\n"
      "vt 0 0\n"
      "vt 1 1\n"
      "f 1/1 2/1 3/1\n"
      "f 3/2 2/2 1/2\n";

    // Act.
    const auto write_tex = true;
    const auto write_nml = false;
    const auto mesh_string = utils::WriteIndexedMesh(imesh, write_tex, write_nml);

    // Assert.
    REQUIRE(expected_string == mesh_string);
  }

  SECTION("positions and indexed normals")
  {
    // Arrange.
    const string expected_string =
      "# Written by https://github.com/thinks/obj-io\n"
      "v 1 2 3\n"
      "v 4 5 6\n"
      "v 7 8 9\n"
      "vn 0 0 -1\n"
      "vn 0 0 1\n"
      "f 1//2 2//2 3//2\n"
      "f 3//1 2//1 1//1\n";

    // Act.
    const auto write_tex = false;
    const auto write_nml = true;
    const auto mesh_string = utils::WriteIndexedMesh(imesh, write_tex, write_nml);
    
    // Assert.
    REQUIRE(expected_string == mesh_string);
  }

  SECTION("positions and indexed tex coords and indexed normals")
  {
    // Arrange.
    const string expected_string =
      "# Written by https://github.com/thinks/obj-io\n"
      "v 1 2 3\n"
      "v 4 5 6\n"
      "v 7 8 9\n"
      "vt 0 0\n"
      "vt 1 1\n"
      "vn 0 0 -1\n"
      "vn 0 0 1\n"
      "f 1/1/2 2/1/2 3/1/2\n"
      "f 3/2/1 2/2/1 1/2/1\n";

    // Act.
    const auto write_tex = true;
    const auto write_nml = true;
    const auto mesh_string = utils::WriteIndexedMesh(imesh, write_tex, write_nml);
    
    // Assert.
    REQUIRE(expected_string == mesh_string);
  }
}


TEST_CASE("write exceptions", "[container]")
{
  typedef utils::Mesh<utils::Vertex<>, std::int8_t, 3> MeshType;
  typedef typename MeshType::VertexType VertexType;
  typedef typename VertexType::PositionType PositionType;
  typedef typename VertexType::TexCoordType TexCoordType;
  typedef typename VertexType::NormalType NormalType;
  typedef typename MeshType::IndexType IndexType;

  SECTION("negative index")
  {
    auto mesh = MeshType{};
    mesh.indices = vector<IndexType>{ 0, 1, -1 };

    const auto write_tex = false;
    const auto write_nml = false;
    REQUIRE_THROWS_MATCHES(
      utils::WriteMesh(mesh, write_tex, write_nml),
      std::runtime_error,
      utils::ExceptionContentMatcher{ "invalid index: -1" });
  }

  SECTION("max index")
  {
    auto mesh = MeshType{};
    mesh.indices = vector<IndexType>{ 0, 1, 127 };

    const auto write_tex = false;
    const auto write_nml = false;
    REQUIRE_THROWS_MATCHES(
      utils::WriteMesh(mesh, write_tex, write_nml),
      std::runtime_error,
      utils::ExceptionContentMatcher{ "invalid index: 127" });
  }

  SECTION("less than three indices per face")
  {
    auto mesh = utils::Mesh<utils::Vertex<>, std::uint8_t, 2>{};
    mesh.indices = vector<std::uint8_t>{ 0, 1 };

    const auto write_tex = false;
    const auto write_nml = false;
    REQUIRE_THROWS_MATCHES(
      utils::WriteMesh(mesh, write_tex, write_nml),
      std::runtime_error,
      utils::ExceptionContentMatcher{ "face must have at least three indices" });
  }

  SECTION("texture coordinate range [< 0]")
  {
    auto mesh = MeshType{};
    mesh.vertices = vector<VertexType>{
      VertexType{
        PositionType{ 1.f, 2.f, 3.f },
        TexCoordType{ -0.1f, 0.f },
        NormalType{ 1.f, 0.f, 0.f }
      },
    };
    mesh.indices = vector<IndexType>{ 0, 0, 0 };

    const auto write_tex = true;
    const auto write_nml = false;
    REQUIRE_THROWS_MATCHES(
      utils::WriteMesh(mesh, write_tex, write_nml),
      std::runtime_error,
      utils::ExceptionContentMatcher{
        "texture coordinate values must be in range [0, 1]" });
  }

  SECTION("texture coordinate range [> 1]")
  {
    auto mesh = MeshType{};
    mesh.vertices = vector<VertexType>{
      VertexType{
      PositionType{ 1.f, 2.f, 3.f },
      TexCoordType{ 0.f, 1.1f },
      NormalType{ 1.f, 0.f, 0.f }
    },
    };
    mesh.indices = vector<IndexType>{ 0, 0, 0 };

    const auto write_tex = true;
    const auto write_nml = false;
    REQUIRE_THROWS_MATCHES(
      utils::WriteMesh(mesh, write_tex, write_nml),
      std::runtime_error,
      utils::ExceptionContentMatcher{
        "texture coordinate values must be in range [0, 1]" });
  }
}
