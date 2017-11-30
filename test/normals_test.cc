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

#include <gtest/gtest.h>

#include "utils.h"
#include "../include/thinks/obj_io.h"

using std::begin;
using std::end;
using std::runtime_error;
using std::stringstream;
using std::vector;
using thinks::obj_io::make_normals;
using util::CubeMesh;
using util::IncrementAndClampToMaxElement;
using util::IncrementNonZeroElements;
using util::VertexCount;


TEST(NormalsTest, Ctor)
{
  const auto indices_per_face = uint32_t{ 3 };
  const auto mesh = CubeMesh();
  const auto normals = make_normals(
    begin(mesh.normal_elements), end(mesh.normal_elements),
    begin(mesh.normal_indices), end(mesh.normal_indices),
    indices_per_face);
  EXPECT_EQ(begin(mesh.normal_elements), normals.elements_begin);
  EXPECT_EQ(end(mesh.normal_elements), normals.elements_end);
  EXPECT_EQ(begin(mesh.normal_indices), normals.indices_begin);
  EXPECT_EQ(end(mesh.normal_indices), normals.indices_end);
  EXPECT_EQ(3, normals.elements_per_vertex); // Constant.
  EXPECT_EQ(3, normals.indices_per_face);
}

TEST(NormalsTest, CtorEmptyElementsAndIndices)
{
  const auto indices_per_face = uint32_t{ 3 };
  const auto elements = vector<float>{};
  const auto indices = vector<uint32_t>{};
  try {
    const auto normals = make_normals(
      begin(elements), end(elements),
      begin(indices), end(indices),
      indices_per_face);
  }
  catch (...) {
    FAIL() << "empty normals should be allowed";
  }
}

TEST(NormalsTest, CtorThrowsIfElementCountNotMultipleOfElementsPerVertex)
{
  // Add an extra element.
  const auto indices_per_face = uint32_t{ 3 };
  auto mesh = CubeMesh();
  mesh.normal_elements.push_back(0.25f);
  try {
    const auto normals = make_normals(
      begin(mesh.normal_elements), end(mesh.normal_elements),
      begin(mesh.normal_indices), end(mesh.normal_indices),
      indices_per_face);
    FAIL() << "exception not thrown";
  }
  catch (const runtime_error& ex) {
    auto ss = stringstream();
    ss << "normal element count (" << mesh.normal_elements.size()
      << ") must be a multiple of " << 3; // Constant.
    EXPECT_STREQ(ss.str().c_str(), ex.what());
  }
  catch (...) {
    FAIL() << "incorrect exception";
  }
}

TEST(NormalsTest, CtorThrowsIfIndicesPerFaceIsLessThanThree)
{
  // Resize indices to be two per face.
  const auto indices_per_face = uint32_t{ 2 };
  auto mesh = CubeMesh();
  mesh.normal_indices.resize(
    (mesh.normal_indices.size() / 3) * indices_per_face);
  try {
    const auto normals = make_normals(
      begin(mesh.normal_elements), end(mesh.normal_elements),
      begin(mesh.normal_indices), end(mesh.normal_indices),
      indices_per_face);
    FAIL() << "exception not thrown";
  }
  catch (const runtime_error& ex) {
    auto ss = stringstream();
    ss << "normal indices per face must be "
      << "greater than or equal to 3, was "
      << indices_per_face;
    EXPECT_STREQ(ss.str().c_str(), ex.what());
  }
  catch (...) {
    FAIL() << "incorrect exception";
  }
}

TEST(NormalsTest, CtorThrowsIfIndexCountNotMultipleOfIndicesPerFace)
{
  // Add an extra index element.
  auto mesh = CubeMesh();
  mesh.normal_indices.push_back(0);
  const auto indices_per_face = uint32_t{ 3 };
  try {
    const auto normals = make_normals(
      begin(mesh.normal_elements), end(mesh.normal_elements),
      begin(mesh.normal_indices), end(mesh.normal_indices),
      indices_per_face);
    FAIL() << "exception not thrown";
  }
  catch (const runtime_error& ex) {
    auto ss = stringstream();
    ss << "normal index count (" << mesh.normal_indices.size()
      << ") must be a multiple of " << indices_per_face;
    EXPECT_STREQ(ss.str().c_str(), ex.what());
  }
  catch (...) {
    FAIL() << "incorrect exception";
  }
}

TEST(NormalsTest, CtorThrowIfInvalidIndexRange_MinNotZero)
{
  const auto indices_per_face = uint32_t{ 3 };
  auto mesh = CubeMesh();
  IncrementAndClampToMaxElement(mesh.normal_indices);
  try {
    const auto normals = make_normals(
      begin(mesh.normal_elements), end(mesh.normal_elements),
      begin(mesh.normal_indices), end(mesh.normal_indices),
      indices_per_face);
    FAIL() << "exception not thrown";
  }
  catch (const runtime_error& ex) {
    const auto min_index =
      *min_element(begin(mesh.normal_indices), end(mesh.normal_indices));
    auto ss = stringstream();
    ss << "min normal index must be zero, was " << min_index;
    EXPECT_STREQ(ss.str().c_str(), ex.what());
  }
  catch (...) {
    FAIL() << "incorrect exception";
  }
}

TEST(NormalsTest, CtorThrowIfInvalidIndexRange_MaxTooHigh)
{
  const auto indices_per_face = uint32_t{ 3 };
  auto mesh = CubeMesh();
  IncrementNonZeroElements(mesh.normal_indices);
  try {
    const auto normals = make_normals(
      begin(mesh.normal_elements), end(mesh.normal_elements),
      begin(mesh.normal_indices), end(mesh.normal_indices),
      indices_per_face);
    FAIL() << "exception not thrown";
  }
  catch (const runtime_error& ex) {
    const auto max_index =
      *max_element(begin(mesh.normal_indices), end(mesh.normal_indices));
    auto ss = stringstream();
    ss << "max normal index must be less than vertex count ("
      << VertexCount(mesh.normal_elements, 3) << "), was "
      << max_index;
    EXPECT_STREQ(ss.str().c_str(), ex.what());
  }
  catch (...) {
    FAIL() << "incorrect exception";
  }
}
