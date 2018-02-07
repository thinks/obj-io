// Copyright 2017 Tommy Hinks
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#include <exception>
#include <iterator>
#include <sstream>
#include <vector>

#include "gtest/gtest.h"

#include "../include/thinks/obj_io.h"

using std::distance;
using std::istringstream;
using std::ostringstream;
using std::runtime_error;
using std::vector;
using thinks::obj_io::make_normal_channel;
using thinks::obj_io::make_position_channel;
using thinks::obj_io::make_tex_coord_channel;
using thinks::obj_io::Read;
using thinks::obj_io::Write;

namespace {

  struct Mesh
  {
    vector<float> position_components;
    vector<uint32_t> position_indices;
    uint32_t position_components_per_vertex;
    uint32_t position_indices_per_face;

    vector<float> tex_coord_components;
    vector<uint32_t> tex_coord_indices;
    uint32_t tex_coord_components_per_vertex;
    uint32_t tex_coord_indices_per_face;

    vector<float> normal_components;
    vector<uint32_t> normal_indices;
    uint32_t normal_indices_per_face;
  };

  /// Returns a mesh with:
  /// -  8 * 3 position elements 
  /// - 12 * 3 position index elements
  /// -  4 * 2 tex coord elements
  /// - 12 * 3 tex coord indices
  /// -  6 * 3 normal elements
  /// - 12 * 3 normal indices
  ///
  /// Centered at origin, positions in range [-1,1].
  Mesh CubeMesh()
  {
    auto mesh = Mesh{};
    mesh.position_components = vector<float>{
      1.f, 1.f, -1.f,
      1.f, -1.f, 1.f,
      1.f, -1.f, -1.f,
      1.f, 1.f, 1.f,
      -1.f, -1.f, -1.f,
      -1.f, 1.f, -1.f,
      -1.f, 1.f, 1.f,
      -1.f, -1.f, 1.f
    };
    mesh.position_indices = vector<uint32_t>{
      0, 1, 2,  // X+
      1, 0, 3,
      6, 4, 7,  // X-
      4, 6, 5,
      6, 0, 5,  // Y+
      0, 6, 3,
      1, 4, 2,  // Y-
      4, 1, 7,
      1, 6, 7,  // Z+
      6, 1, 3,
      0, 4, 5,  // Z-
      4, 0, 2
    };
    mesh.position_components_per_vertex = 3;
    mesh.position_indices_per_face = 3;

    mesh.tex_coord_components = vector<float>{
      0.f, 0.f,
      1.f, 0.f,
      0.f, 1.f,
      1.f, 1.f
    };
    mesh.tex_coord_indices = vector<uint32_t>{
      0, 1, 2,
      1, 0, 3,
      0, 1, 2,
      1, 0, 3,
      0, 1, 2,
      1, 0, 3,
      0, 1, 2,
      1, 0, 3,
      0, 1, 2,
      1, 0, 3,
      0, 1, 2,
      1, 0, 3
    };
    mesh.tex_coord_components_per_vertex = 2;
    mesh.tex_coord_indices_per_face = 3;

    mesh.normal_components = vector<float>{
      1.f, 0.f, 0.f,
      -1.f, 0.f, 0.f,
      0.f, 1.f, 0.f,
      0.f, -1.f, 0.f,
      0.f, 0.f, 1.f,
      0.f, 0.f, -1.f
    };
    mesh.normal_indices = vector<uint32_t>{
      0, 0, 0,
      0, 0, 0,
      1, 1, 1,
      1, 1, 1,
      2, 2, 2,
      2, 2, 2,
      3, 3, 3,
      3, 3, 3,
      4, 4, 4,
      4, 4, 4,
      5, 5, 5,
      5, 5, 5
    };
    mesh.normal_indices_per_face = 3;

    return mesh;
  }

} // namespace

TEST(ReadTest, ThrowsIfDifferentComponentCountsInPositionChannel)
{
  // Arrange.
  auto iss = istringstream(
    "v 0 0 0\n"
    "v 1 1 1 1\n" // Four components!
    "v 2 2 2\n"
    "f 1 2 3\n"
  );

  // Act.
  try {
    const auto mesh = Read<float>(iss);
    FAIL() << "exception not thrown";
  }
  catch (const runtime_error& ex) {
    auto ss = ostringstream();
    ss << "invalid component count (" << 4 << ")"
      << ", expected " << 3;
    EXPECT_STREQ(ss.str().c_str(), ex.what());
  }
  catch (...) {
    FAIL() << "incorrect exception";
  }
}

TEST(ReadTest, ThrowsIfDifferentComponentCountsInTexCoordChannel)
{
  // Arrange.
  auto iss = istringstream(
    "v 0 0 0\n"
    "v 1 1 1\n"
    "v 2 2 2\n"
    "vt 1 1\n"
    "vt 0.5 0.5\n"
    "vt 0 0 0\n"  // Three components!
    "f 1/1 2/2 3/3\n"
  );

  // Act.
  try {
    const auto mesh = Read<float>(iss);
    FAIL() << "exception not thrown";
  }
  catch (const runtime_error& ex) {
    auto ss = ostringstream();
    ss << "invalid component count (" << 3 << ")"
      << ", expected " << 2;
    EXPECT_STREQ(ss.str().c_str(), ex.what());
  }
  catch (...) {
    FAIL() << "incorrect exception";
  }
}

TEST(ReadTest, ThrowsIfDifferentComponentCountsInNormalChannel)
{
  // Arrange.
  auto iss = istringstream(
    "v 0 0 0\n"
    "v 1 1 1\n"
    "v 2 2 2\n"
    "vn 1 1 1\n"
    "vn 0 0 0\n"  
    "vn 1 1\n" // Two components!
    "f 1//1 2//2 3//3\n"
  );

  // Act.
  try {
    const auto mesh = Read<float>(iss);
    FAIL() << "exception not thrown";
  }
  catch (const runtime_error& ex) {
    auto ss = ostringstream();
    ss << "invalid component count (" << 2 << ")"
      << ", expected " << 3;
    EXPECT_STREQ(ss.str().c_str(), ex.what());
  }
  catch (...) {
    FAIL() << "incorrect exception";
  }
}

TEST(ReadTest, ThrowsIfEmptyComponents)
{
  // Arrange.
  auto iss = istringstream(
    "v 0 0 0\n"
    "v \n" // Empty components!
    "v 2 2 2\n"
    "f 1 2 3\n"
  );

  // Act.
  try {
    const auto mesh = Read<float>(iss);
    FAIL() << "exception not thrown";
  }
  catch (const runtime_error& ex) {
    EXPECT_STREQ("empty components", ex.what());
  }
  catch (...) {
    FAIL() << "incorrect exception";
  }
}

TEST(ReadTest, ThrowsIfEmptyFace)
{
  // Arrange.
  auto iss = istringstream(
    "v 0 0 0\n"
    "v 1 1 1\n"
    "v 2 2 2\n"
    "f\n"
  );

  // Act.
  try {
    const auto mesh = Read<float>(iss);
    FAIL() << "exception not thrown";
  }
  catch (const runtime_error& ex) {
    EXPECT_STREQ("empty face", ex.what());
  }
  catch (const std::exception& ex)
  {
    FAIL() << ex.what();
  }
  catch (...) {
    FAIL() << "incorrect exception";
  }
}

TEST(ReadTest, ThrowsIfIndexGroupHasMoreThanThreeIndices)
{
  // Arrange.
  auto iss = istringstream(
    "v 0 0 0\n"
    "v 1 1 1\n"
    "v 2 2 2\n"
    "f 1/1/1/1 2/2/2/2 3/3/3/3 4/4/4/4\n"
  );

  // Act.
  try {
    const auto mesh = Read<float>(iss);
    FAIL() << "exception not thrown";
  }
  catch (const runtime_error& ex) {
    EXPECT_STREQ("cannot have more than three indices", ex.what());
  }
  catch (const std::exception& ex)
  {
    FAIL() << ex.what();
  }
  catch (...) {
    FAIL() << "incorrect exception";
  }
}



TEST(ReadTest, ThrowsIfParseFailed)
{
  // Arrange.
  auto iss = istringstream(
    "v 0 0 0\n"
    "v 1 1 1\n"
    "v X 2 2\n"
    "f 1 2 3\n"
  );

  // Act.
  try {
    const auto mesh = Read<float>(iss);
    FAIL() << "exception not thrown";
  }
  catch (const runtime_error& ex) {
    EXPECT_STREQ("failed parsing 'X'", ex.what());
  }
  catch (...) {
    FAIL() << "incorrect exception";
  }
}


TEST(ReadTest, CorrectMesh)
{
  // Arrange.
  auto iss = istringstream(
    "# Generated by https://github.com/thinks/obj-io\n"
    "# Vertex count: 8\n"
    "# Face count: 12\n"
    "v 1 1 -1\n"
    "v 1 -1 1\n"
    "v 1 -1 -1\n"
    "v 1 1 1\n"
    "v -1 -1 -1\n"
    "v -1 1 -1\n"
    "v -1 1 1\n"
    "v -1 -1 1\n"
    "vt 0 0\n"
    "vt 1 0\n"
    "vt 0 1\n"
    "vt 1 1\n"
    "vn 1 0 0\n"
    "vn -1 0 0\n"
    "vn 0 1 0\n"
    "vn 0 -1 0\n"
    "vn 0 0 1\n"
    "vn 0 0 -1\n"
    "f 1/1/1 2/2/1 3/3/1\n"
    "f 2/2/1 1/1/1 4/4/1\n"
    "f 7/1/2 5/2/2 8/3/2\n"
    "f 5/2/2 7/1/2 6/4/2\n"
    "f 7/1/3 1/2/3 6/3/3\n"
    "f 1/2/3 7/1/3 4/4/3\n"
    "f 2/1/4 5/2/4 3/3/4\n"
    "f 5/2/4 2/1/4 8/4/4\n"
    "f 2/1/5 7/2/5 8/3/5\n"
    "f 7/2/5 2/1/5 4/4/5\n"
    "f 1/1/6 5/2/6 6/3/6\n"
    "f 5/2/6 1/1/6 3/4/6\n");

  // Act.
  const auto mesh = Read<float>(iss);

  // Assert.
  const auto pos_channel = mesh.position_channel();
  const auto position_component_count = distance(
    pos_channel.components_begin(), 
    pos_channel.components_end());
  const auto position_index_count = distance(
    pos_channel.indices_begin(),
    pos_channel.indices_end());
  EXPECT_EQ(24, position_component_count);
  EXPECT_EQ(36, position_index_count);
  EXPECT_EQ(3, pos_channel.components_per_value());
  EXPECT_EQ(3, pos_channel.indices_per_face());

  const auto tex_channel = *mesh.tex_coord_channel();
  const auto tex_component_count = distance(
    tex_channel.components_begin(),
    tex_channel.components_end());
  const auto tex_index_count = distance(
    tex_channel.indices_begin(),
    tex_channel.indices_end());
  EXPECT_EQ(8, tex_component_count);
  EXPECT_EQ(36, tex_index_count);
  EXPECT_EQ(2, tex_channel.components_per_value());
  EXPECT_EQ(3, tex_channel.indices_per_face());

  const auto nml_channel = *mesh.normal_channel();
  const auto nml_component_count = distance(
    nml_channel.components_begin(),
    nml_channel.components_end());
  const auto nml_index_count = distance(
    nml_channel.indices_begin(),
    nml_channel.indices_end());
  EXPECT_EQ(18, nml_component_count);
  EXPECT_EQ(36, nml_index_count);
  EXPECT_EQ(3, nml_channel.components_per_value());
  EXPECT_EQ(3, nml_channel.indices_per_face());
}


TEST(ReadTest, RoundTrip)
{
  EXPECT_TRUE(false) << "todo";
}