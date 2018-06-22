// Copyright(C) 2018 Tommy Hinks <tommy.hinks@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <thinks/obj_io/obj_io_mapper.h>

#include <array>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

using thinks::obj_io::Face;
using thinks::obj_io::Index;
using thinks::obj_io::Normal;
using thinks::obj_io::Position;
using thinks::obj_io::TexCoord;
using thinks::obj_io::Read;
using thinks::obj_io::Write;

namespace 
{

struct Vec2f
{
  float x;
  float y;
};

struct Vec3f
{
  float x;
  float y;
  float z;
};

struct Vertex
{
  Vec3f pos;
  Vec2f tex;
  Vec3f normal;
  Vec3f color; // Attribute that is not supported by OBJ format.
};

struct Mesh
{
  std::vector<Vertex> vertices;
  std::vector<uint32_t> tri_indices;
};

void WriteMesh(
  std::ostream& os, 
  const Mesh& mesh,
  const bool use_tex = false,
  const bool use_nml = false)
{
  using namespace std;

  typedef Position<float, 3> Position3f;
  typedef TexCoord<float, 2> TexCoord2f;
  typedef Normal<float> Normalf;
  typedef Index<uint32_t> Indexui;
  typedef Face<Indexui, 3> Face3;

  const auto vtx_iend = mesh.vertices.end();

  // Positions.
  auto pos_vtx_iter = mesh.vertices.begin();
  auto pos_mapper = [&pos_vtx_iter, vtx_iend]() -> pair<Position3f, bool> {
    if (pos_vtx_iter != vtx_iend) {
      const auto vtx = *pos_vtx_iter++;
      return make_pair(Position3f(vtx.pos.x, vtx.pos.y, vtx.pos.z), true);
    }

    return make_pair(Position3f{}, false);
  };

  // Texture coordinates.
  auto tex_vtx_iter = mesh.vertices.begin();
  auto tex_mapper = [&tex_vtx_iter, vtx_iend]() -> pair<TexCoord2f, bool> {
    if (tex_vtx_iter != vtx_iend) {
      const auto vtx = *tex_vtx_iter++;
      return make_pair(TexCoord2f(vtx.tex.x, vtx.tex.y), true);
    }

    return make_pair(TexCoord2f{}, false);
  };

  // Normals.
  auto nml_vtx_iter = mesh.vertices.begin();
  auto nml_mapper = [&nml_vtx_iter, vtx_iend]() -> pair<Normalf, bool> {
    if (nml_vtx_iter != vtx_iend) {
      const auto vtx = *nml_vtx_iter++;
      return make_pair(Normalf(vtx.normal.x, vtx.normal.y, vtx.normal.z), true);
    }

    return make_pair(Normalf{}, false);
  };

  // Faces.
  auto idx_iter = mesh.tri_indices.begin();
  const auto idx_iend = mesh.tri_indices.end();
  auto face_mapper = [&idx_iter, idx_iend]() -> pair<Face3, bool> {
    if (idx_iter != idx_iend) {
      const auto idx0 = *idx_iter++;
      // throw?
      const auto idx1 = *idx_iter++;
      // throw?
      const auto idx2 = *idx_iter++;
      return make_pair(Face3(Indexui(idx0), Indexui(idx1), Indexui(idx2)), true);
    }

    return make_pair(Face3{}, false);
  };

  if (!use_tex && !use_nml)
  {
    thinks::obj_io::Write(
      os,
      pos_mapper,
      face_mapper);
  }
  else if (use_tex && !use_nml) {
    thinks::obj_io::Write(
      os,
      pos_mapper,
      face_mapper,
      tex_mapper);
  }
  else if (!use_tex && use_nml) {
    thinks::obj_io::Write(
      os,
      pos_mapper,
      face_mapper,
      nullptr,
      nml_mapper);
  }
  else {
    thinks::obj_io::Write(
      os,
      pos_mapper,
      face_mapper,
      tex_mapper,
      nml_mapper);
  }
}

} // namespace


TEST(WriteInterleavedTest, Positions)
{
  // Arrange.
  const std::string expected_string =
    "# Written by https://github.com/thinks/obj-io\n"
    "v 1 2 3\n"
    "v 4 5 6\n"
    "v 7 8 9\n"
    "f 1 2 3\n"
    "f 3 2 1\n";

  auto mesh = Mesh{};
  mesh.vertices = std::vector<Vertex>{
    Vertex{
      Vec3f{ 1.f, 2.f, 3.f }
    },
    Vertex{
      Vec3f{ 4.f, 5.f, 6.f }
    },
    Vertex{
      Vec3f{ 7.f, 8.f, 9.f }
    }
  };
  mesh.tri_indices = std::vector<std::uint32_t>{ 0, 1, 2, 2, 1, 0 };
  auto ss = std::stringstream();

  // Act.
  WriteMesh(ss, mesh);

  // Assert.
  EXPECT_STREQ(expected_string.c_str(), ss.str().c_str());
}

TEST(WriteInterleavedTest, PositionsAndTexCoords)
{
  // Arrange.
  const std::string expected_string =
    "# Written by https://github.com/thinks/obj-io\n"
    "v 1 2 3\n"
    "v 4 5 6\n"
    "v 7 8 9\n"
    "vt 0 0\n"
    "vt 0 1\n"
    "vt 1 1\n"
    "f 1 2 3\n"
    "f 3 2 1\n";

  auto mesh = Mesh{};
  mesh.vertices = std::vector<Vertex>{
    Vertex{
      Vec3f{ 1.f, 2.f, 3.f },
      Vec2f{ 0.f, 0.f }
    },
    Vertex{
      Vec3f{ 4.f, 5.f, 6.f },
      Vec2f{ 0.f, 1.f }
    },
    Vertex{
      Vec3f{ 7.f, 8.f, 9.f },
      Vec2f{ 1.f, 1.f }
    }
  };
  mesh.tri_indices = std::vector<std::uint32_t>{ 0, 1, 2, 2, 1, 0 };
  auto ss = std::stringstream();

  // Act.
  const auto use_tex = true;
  WriteMesh(ss, mesh, use_tex);

  // Assert.
  EXPECT_STREQ(expected_string.c_str(), ss.str().c_str());
}

TEST(WriteInterleavedTest, PositionsAndNormals)
{
  // Arrange.
  const std::string expected_string =
    "# Written by https://github.com/thinks/obj-io\n"
    "v 1 2 3\n"
    "v 4 5 6\n"
    "v 7 8 9\n"
    "vn 1 0 0\n"
    "vn 0 1 0\n"
    "vn 0 0 1\n"
    "f 1 2 3\n"
    "f 3 2 1\n";

  auto mesh = Mesh{};
  mesh.vertices = std::vector<Vertex>{
    Vertex{
      Vec3f{ 1.f, 2.f, 3.f },
      Vec2f{ 0.f, 0.f },
      Vec3f{ 1.f, 0.f, 0.f }
    },
    Vertex{
      Vec3f{ 4.f, 5.f, 6.f },
      Vec2f{ 0.f, 0.f },
      Vec3f{ 0.f, 1.f, 0.f }
    },
    Vertex{
      Vec3f{ 7.f, 8.f, 9.f },
      Vec2f{ 0.f, 0.f },
      Vec3f{ 0.f, 0.f, 1.f }
    }
  };
  mesh.tri_indices = std::vector<std::uint32_t>{ 0, 1, 2, 2, 1, 0 };
  auto ss = std::stringstream();

  // Act.
  const auto use_tex = false;
  const auto use_nml = true;
  WriteMesh(ss, mesh, use_tex, use_nml);

  // Assert.
  EXPECT_STREQ(expected_string.c_str(), ss.str().c_str());
}

TEST(WriteInterleavedTest, PositionsAndTexCoordsAndNormals)
{
  // Arrange.
  const std::string expected_string =
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

  auto mesh = Mesh{};
  mesh.vertices = std::vector<Vertex>{
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
  mesh.tri_indices = std::vector<std::uint32_t>{ 0, 1, 2, 2, 1, 0 };
  auto ss = std::stringstream();

  // Act.
  const auto use_tex = true;
  const auto use_nml = true;
  WriteMesh(ss, mesh, use_tex, use_nml);

  // Assert.
  EXPECT_STREQ(expected_string.c_str(), ss.str().c_str());
}


TEST(WriteInterleavedTest, Test)
{
  auto mesh = Mesh{};
  mesh.vertices = std::vector<Vertex>{
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
  mesh.tri_indices = std::vector<std::uint32_t>{ 0, 1, 2, 2, 1, 0 };

  auto ss = std::stringstream();

  WriteMesh(ss, mesh);

  std::cout << ss.str() << std::endl;
}

TEST(WriteInterleavedTest, Read)
{
  // Arrange.
  const std::string input =
    "# Written by https://github.com/thinks/obj-io\n"
    "" // empty
    "v 1 2 3\n"
    "v 4 5 6\n"
    "v 7 8 9\n"
    "vt 0 0\n"
    "vt 0 1\n"
    "vt 1 1\n"
    "vn 1 0 0\n"
    "vn 0 1 0\n"
    "vn 0 0 1\n"
    "f 1 2 3/4/5\n"
    "f 3//1 2 1\n";

  auto ss = std::stringstream(input);

  Read<float, std::uint32_t>(
    ss, 
    [](const auto& pos) { std::cout << "v " << pos.values[0] << " " << pos.values[1] << " " << pos.values[2] << std::endl; },
    [](const auto& face) { 
      std::cout << "f " 
        << face.values[0] << " " 
        << face.values[1] << " " 
        << face.values[2] << std::endl; });

  auto f = [](const auto v) -> void
  {
    std::cout << v.values.size() << std::endl; 
  };

  f(Position<float, 3>{});
  f(Position<float, 4>{});

}


