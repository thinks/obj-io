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

#include "thinks/obj_io.h"

using std::distance;
using std::exception;
using std::istringstream;
using std::ostringstream;
using std::runtime_error;
using std::vector;
//using thinks::obj_io::make_normal_channel;
//using thinks::obj_io::make_position_channel;
//using thinks::obj_io::make_tex_coord_channel;
using thinks::obj_io::Read;
//using thinks::obj_io::Write;


TEST(ReadTest, Value_ThrowsIfFailedToParseComponent)
{
  // Arrange.
  auto iss = istringstream(
    "v 0 0 0\n"
    "v 1 X 1\n" // Invalid component!
    "v 2 2 2\n"
    "f 1 2 3\n");

  // Act.
  try {
    const auto mesh = Read<float>(iss);
    FAIL() << "exception not thrown";
  }
  catch (const runtime_error& ex) {
    EXPECT_STREQ("failed parsing 'X'", ex.what());
  }
  catch (const exception& ex) {
    FAIL() << ex.what();
  }
  catch (...) {
    FAIL() << "incorrect exception";
  }
}

TEST(ReadTest, Value_ThrowsIfEmptyComponents)
{
  // Arrange.
  auto iss = istringstream(
    "v 0 0 0\n"
    "v \n" // Empty components!
    "v 2 2 2\n"
    "f 1 2 3\n");

  // Act.
  try {
    const auto mesh = Read<float>(iss);
    FAIL() << "exception not thrown";
  }
  catch (const runtime_error& ex) {
    EXPECT_STREQ("empty components", ex.what());
  }
  catch (const exception& ex) {
    FAIL() << ex.what();
  }
  catch (...) {
    FAIL() << "incorrect exception";
  }
}

TEST(ReadTest, Value_ThrowsIfDifferentComponentCountsInPositionChannel)
{
  // Arrange.
  auto iss = istringstream(
    "v 0 0 0\n"   // First vertex has three components.
    "v 1 1 1 1\n" // Another vertex has four components.
    "v 2 2 2\n"
    "f 1 2 3\n");

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
  catch (const exception& ex) {
    FAIL() << ex.what();
  }
  catch (...) {
    FAIL() << "incorrect exception";
  }
}

TEST(ReadTest, Value_ThrowsIfDifferentComponentCountsInTexCoordChannel)
{
  // Arrange.
  auto iss = istringstream(
    "v 0 0 0\n"
    "v 1 1 1\n"
    "v 2 2 2\n"
    "vt 1 1\n"
    "vt 0.1 0.1\n"      // First tex coord has two components.
    "vt 0.2 0.2 0.2\n"  // Another tex coord has three components.
    "vt 0.3 0.3\n"
    "f 1/1 2/2 3/3\n");

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
  catch (const exception& ex) {
    FAIL() << ex.what();
  }
  catch (...) {
    FAIL() << "incorrect exception";
  }
}

TEST(ReadTest, Value_ThrowsIfDifferentComponentCountsInNormalChannel)
{
  // Arrange.
  auto iss = istringstream(
    "v 0 0 0\n"
    "v 1 1 1\n"
    "v 2 2 2\n"
    "vn 1 1 1\n"  // First normal has three components.
    "vn 0 0\n"    // Another normal has two components.
    "vn 1 1 1\n" 
    "f 1//1 2//2 3//3\n");

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
  catch (const exception& ex) {
    FAIL() << ex.what();
  }
  catch (...) {
    FAIL() << "incorrect exception";
  }
}

TEST(ReadTest, Face_ThrowsIfFailedToParseIndex)
{
  // Arrange.
  auto iss = istringstream(
    "v 0 0 0\n"
    "v 1 1 1\n" 
    "v 2 2 2\n"
    "f 1 Y 3\n"); // Invalid index!

  // Act.
  try {
    const auto mesh = Read<float>(iss);
    FAIL() << "exception not thrown";
  }
  catch (const runtime_error& ex) {
    EXPECT_STREQ("failed parsing 'Y'", ex.what());
  }
  catch (const exception& ex) {
    FAIL() << ex.what();
  }
  catch (...) {
    FAIL() << "incorrect exception";
  }
}

TEST(ReadTest, Face_ThrowsIfEmptyFace)
{
  // Arrange.
  auto iss = istringstream(
    "v 0 0 0\n"
    "v 1 1 1\n"
    "v 2 2 2\n"
    "f\n");

  // Act.
  try {
    const auto mesh = Read<float>(iss);
    FAIL() << "exception not thrown";
  }
  catch (const runtime_error& ex) {
    EXPECT_STREQ("empty face", ex.what());
  }
  catch (const exception& ex) {
    FAIL() << ex.what();
  }
  catch (...) {
    FAIL() << "incorrect exception";
  }
}

TEST(ReadTest, Face_ThrowsIfIndexGroupHasMoreThanThreeIndices)
{
  // Arrange.
  auto iss = istringstream(
    "v 0 0 0\n"
    "v 1 1 1\n"
    "v 2 2 2\n"
    "f 1/1/1/1 2/2/2/2 3/3/3/3 4/4/4/4\n");

  // Act.
  try {
    const auto mesh = Read<float>(iss);
    FAIL() << "exception not thrown";
  }
  catch (const runtime_error& ex) {
    EXPECT_STREQ("index group cannot have more than three indices", ex.what());
  }
  catch (const exception& ex) {
    FAIL() << ex.what();
  }
  catch (...) {
    FAIL() << "incorrect exception";
  }
}

TEST(ReadTest, Face_ThrowsIfDifferentIndexGroupCount)
{
  // Arrange.
  auto iss = istringstream(
    "v 0 0 0\n"
    "v 1 1 1\n"
    "v 2 2 2\n"
    "v 3 3 3\n"
    "f 1 2 3\n"   // First face has three index groups. 
    "f 1 2 3 4\n" // Another face has four index groups.
  );

  // Act.
  try {
    const auto mesh = Read<float>(iss);
    FAIL() << "exception not thrown";
  }
  catch (const runtime_error& ex) {
    auto oss = ostringstream();
    oss << "invalid index group count (" << 4 << ")"
      << ", expected " << 3;
    EXPECT_STREQ(oss.str().c_str(), ex.what());
  }
  catch (const exception& ex) {
    FAIL() << ex.what();
  }
  catch (...) {
    FAIL() << "incorrect exception";
  }
}

TEST(ReadTest, Face_ThrowsIfMissingPositionIndex)
{
  // Arrange.
  auto iss = istringstream(
    "v 0 0 0\n"
    "v 1 1 1\n"
    "v 2 2 2\n"
    "f 1 /2 3\n" // Middle index group has no position index.
  );

  // Act.
  try {
    const auto mesh = Read<float>(iss);
    FAIL() << "exception not thrown";
  }
  catch (const runtime_error& ex) {
    EXPECT_STREQ("missing position index ('/2')", ex.what());
  }
  catch (const exception& ex) {
    FAIL() << ex.what();
  }
  catch (...) {
    FAIL() << "incorrect exception";
  }
}



/*
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
*/

TEST(DISABLED_ReadTest, RoundTrip)
{
  EXPECT_TRUE(false) << "todo";
}