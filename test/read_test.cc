// Copyright(C) 2018 Tommy Hinks <tommy.hinks@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <utils/read_write_utils.h>
#include <utils/type_utils.h>

#include <cstdint>
#include <sstream>
#include <string>

#include <catch.hpp>


TEST_CASE("read", "[container]")
{
  const std::string input =
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
    auto iss = std::istringstream(input);
    const auto mesh = utils::ReadMesh<utils::Mesh<>>(
      iss, read_tex_coords, read_normals);

    typedef decltype(mesh) MeshType;
    typedef typename MeshType::VertexType VertexType;
    typedef typename VertexType::PositionType PositionType;

    using utils::Equals;
    REQUIRE(Equals(mesh.vertices[0].pos, PositionType{ 1.f, 2.f, 3.f }));
    REQUIRE(Equals(mesh.vertices[1].pos, PositionType{ 4.f, 5.f, 6.f }));
    REQUIRE(Equals(mesh.vertices[2].pos, PositionType{ 7.f, 8.f, 9.f }));
    REQUIRE((
      mesh.indices[0] == 0 &&
      mesh.indices[1] == 1 &&
      mesh.indices[2] == 2));
    REQUIRE((
      mesh.indices[3] == 2 &&
      mesh.indices[4] == 1 &&
      mesh.indices[5] == 0));
  }
}
