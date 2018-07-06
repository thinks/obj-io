// Copyright(C) 2018 Tommy Hinks <tommy.hinks@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <thinks/obj_io/obj_io.h>
#include <types.h>

#include <array>
#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <catch.hpp>


using std::begin;
using std::distance;
using std::end;
using std::ostream;
using std::ostringstream;
using std::string;
using std::uint32_t;
using std::vector;

using thinks::obj_io::End;
using thinks::obj_io::Index;
using thinks::obj_io::IndexGroup;
using thinks::obj_io::Map;
using thinks::obj_io::Normal;
using thinks::obj_io::Position;
using thinks::obj_io::TexCoord;
using thinks::obj_io::TriangleFace;
using thinks::obj_io::Write;


namespace {

template<
  typename PosMapper,
  typename FaceMapper,
  typename TexMapper,
  typename NmlMapper>
string WriteHelper(
  PosMapper pos_mapper,
  FaceMapper face_mapper,
  TexMapper tex_mapper,
  NmlMapper nml_mapper,
  const bool write_tex_coords,
  const bool write_normals)
{
  auto oss = ostringstream{};
  if (!write_tex_coords && !write_normals)
  {
    Write(
      oss,
      pos_mapper,
      face_mapper);
  }
  else if (write_tex_coords && !write_normals) {
    Write(
      oss,
      pos_mapper,
      face_mapper,
      tex_mapper);
  }
  else if (!write_tex_coords && write_normals) {
    Write(
      oss,
      pos_mapper,
      face_mapper,
      nullptr,
      nml_mapper);
  }
  else {
    Write(
      oss,
      pos_mapper,
      face_mapper,
      tex_mapper,
      nml_mapper);
  }

  return oss.str();
}

string WriteMesh(
  const Mesh& mesh,
  const bool write_tex_coords,
  const bool write_normals)
{
  const auto vtx_iend = end(mesh.vertices);

  // Positions.
  auto pos_vtx_iter = begin(mesh.vertices);
  auto pos_mapper = [&pos_vtx_iter, vtx_iend]() {
    if (pos_vtx_iter == vtx_iend) {
      return End<Position3_f>();
    }

    const auto vtx = *pos_vtx_iter++;
    return Map(Position3_f(vtx.pos.x, vtx.pos.y, vtx.pos.z));
  };

  // Texture coordinates.
  auto tex_vtx_iter = begin(mesh.vertices);
  auto tex_mapper = [&tex_vtx_iter, vtx_iend]() {
    if (tex_vtx_iter == vtx_iend) {
      return End<TexCoord2_f>();
    }

    const auto vtx = *tex_vtx_iter++;
    return Map(TexCoord2_f(vtx.tex.x, vtx.tex.y));
  };

  // Normals.
  auto nml_vtx_iter = begin(mesh.vertices);
  auto nml_mapper = [&nml_vtx_iter, vtx_iend]() {
    if (nml_vtx_iter == vtx_iend) {
      return End<Normal_f>();
    }

    const auto vtx = *nml_vtx_iter++;
    return Map(Normal_f(vtx.normal.x, vtx.normal.y, vtx.normal.z));
  };

  // Faces.
  auto idx_iter = mesh.tri_indices.begin();
  const auto idx_iend = mesh.tri_indices.end();
  auto face_mapper = [&idx_iter, idx_iend]() {
    if (distance(idx_iter, idx_iend) < 3) {
      assert(idx_iter == idx_iend && "trailing indices");
      return End<TriFace_ui32>();
    }

    const auto idx0 = *idx_iter++;
    const auto idx1 = *idx_iter++;
    const auto idx2 = *idx_iter++;
    return Map(TriFace_ui32(Index_ui32(idx0), Index_ui32(idx1), Index_ui32(idx2)));
  };

  return WriteHelper(
    pos_mapper,
    face_mapper,
    tex_mapper,
    nml_mapper,
    write_tex_coords,
    write_normals);
}

string WriteIndexedMesh(
  const IndexedMesh& imesh,
  const bool write_tex_coords,
  const bool write_normals)
{
  // Positions.
  auto pos_iter = begin(imesh.positions);
  auto pos_iend = end(imesh.positions);
  auto pos_mapper = [&pos_iter, pos_iend]() {
    if (pos_iter == pos_iend) {
      return End<Position3_f>();
    }

    const auto pos = *pos_iter++;
    return Map(Position3_f(pos.x, pos.y, pos.z));
  };

  // Texture coordinates.
  auto tex_iter = begin(imesh.tex_coords);
  auto tex_iend = end(imesh.tex_coords);
  auto tex_mapper = [&tex_iter, tex_iend]() {
    if (tex_iter == tex_iend) {
      return End<TexCoord2_f>();
    }

    const auto tex = *tex_iter++;
    return Map(TexCoord2_f(tex.x, tex.y));
  };

  // Normals.
  auto nml_iter = begin(imesh.normals);
  auto nml_iend = end(imesh.normals);
  auto nml_mapper = [&nml_iter, nml_iend]() {
    if (nml_iter == nml_iend) {
      return End<Normal_f>();
    }

    const auto nml = *nml_iter++;
    return Map(Normal_f(nml.x, nml.y, nml.z));
  };

  // Faces.
  auto pos_idx_iter = begin(imesh.position_indices);
  auto pos_idx_iend = end(imesh.position_indices);
  auto tex_idx_iter = begin(imesh.tex_coord_indices);
  auto tex_idx_iend = end(imesh.tex_coord_indices);
  auto nml_idx_iter = begin(imesh.normal_indices);
  auto nml_idx_iend = end(imesh.normal_indices);
  auto face_mapper = 
    [&pos_idx_iter, &tex_idx_iter, &nml_idx_iter,
    pos_idx_iend, tex_idx_iend, nml_idx_iend,
    write_tex_coords, write_normals]() {
    if (distance(pos_idx_iter, pos_idx_iend) < 3 ||
        distance(tex_idx_iter, tex_idx_iend) < 3 ||
        distance(nml_idx_iter, nml_idx_iend) < 3) {
      assert(pos_idx_iter == pos_idx_iend && "trailing position indices");
      assert(tex_idx_iter == tex_idx_iend && "trailing tex coord indices");
      assert(nml_idx_iter == nml_idx_iend && "trailing normal indices");
      return End<TriFaceGroup_ui32>();
    }

    auto g0 = IndexGroup_ui32(*pos_idx_iter++, *tex_idx_iter++, *nml_idx_iter++);
    auto g1 = IndexGroup_ui32(*pos_idx_iter++, *tex_idx_iter++, *nml_idx_iter++);
    auto g2 = IndexGroup_ui32(*pos_idx_iter++, *tex_idx_iter++, *nml_idx_iter++);
    g0.tex_coord_index.second = write_tex_coords;
    g1.tex_coord_index.second = write_tex_coords;
    g2.tex_coord_index.second = write_tex_coords;
    g0.normal_index.second = write_normals;
    g1.normal_index.second = write_normals;
    g2.normal_index.second = write_normals;
    return Map(TriFaceGroup_ui32(g0, g1, g2));
  };

  return WriteHelper(
    pos_mapper,
    face_mapper,
    tex_mapper,
    nml_mapper,
    write_tex_coords,
    write_normals);
}

} // namespace


TEST_CASE("write", "[container]")
{
  // Setup.
  auto mesh = Mesh{};
  mesh.vertices = vector<Vertex>{
    Vertex{
      Vec3f{ 1.f, 2.f, 3.f },
      Vec2f{ 0.f, 0.f },
      Vec3f{ 1.f, 0.f, 0.f }
    },
    Vertex{
      Vec3f{ 4.f, 5.f, 6.f },
      Vec2f{ 0.f, 1.f },
      Vec3f{ 0.f, 1.f, 0.f }
    },
    Vertex{
      Vec3f{ 7.f, 8.f, 9.f },
      Vec2f{ 1.f, 1.f },
      Vec3f{ 0.f, 0.f, 1.f }
    }
  };
  mesh.tri_indices = vector<uint32_t>{ 0, 1, 2, 2, 1, 0 };

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

    // Act & Assert.
    const auto write_tex = false;
    const auto write_nml = false;
    REQUIRE(expected_string == WriteMesh(mesh, write_tex, write_nml));
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

    // Act & Assert.
    const auto write_tex = true;
    const auto write_nml = false;
    REQUIRE(expected_string == WriteMesh(mesh, write_tex, write_nml));
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

    // Act & Assert.
    const auto write_tex = false;
    const auto write_nml = true;
    REQUIRE(expected_string == WriteMesh(mesh, write_tex, write_nml));
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

    // Act & Assert.
    const auto write_tex = true;
    const auto write_nml = true;
    REQUIRE(expected_string == WriteMesh(mesh, write_tex, write_nml));
  }
}

TEST_CASE("write indexed", "[container]")
{
  auto imesh = IndexedMesh{};
  imesh.positions = vector<Vec3f>{
    Vec3f{1.f, 2.f, 3.f},
    Vec3f{4.f, 5.f, 6.f},
    Vec3f{7.f, 8.f, 9.f}
  };
  imesh.position_indices = vector<uint32_t>{ 
    0, 1, 2, 
    2, 1, 0 
  };
  imesh.tex_coords = vector<Vec2f>{ 
    Vec2f{0.f, 0.f}, 
    Vec2f{1.f, 1.f} 
  };
  imesh.tex_coord_indices = vector<uint32_t>{ 
    0, 0, 0, 
    1, 1, 1 
  };
  imesh.normals = vector<Vec3f>{ 
    Vec3f{0.f, 0.f, -1.f}, 
    Vec3f{0.f, 0.f, 1.f} 
  };
  imesh.normal_indices = vector<uint32_t>{ 
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

    // Act & Assert.
    const auto write_tex = false;
    const auto write_nml = false;
    REQUIRE(expected_string == WriteIndexedMesh(imesh, write_tex, write_nml));
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

    // Act & Assert.
    const auto write_tex = true;
    const auto write_nml = false;
    REQUIRE(expected_string == WriteIndexedMesh(imesh, write_tex, write_nml));
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

    // Act & Assert.
    const auto write_tex = false;
    const auto write_nml = true;
    REQUIRE(expected_string == WriteIndexedMesh(imesh, write_tex, write_nml));
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

    // Act & Assert.
    const auto write_tex = true;
    const auto write_nml = true;
    REQUIRE(expected_string == WriteIndexedMesh(imesh, write_tex, write_nml));
  }
}
