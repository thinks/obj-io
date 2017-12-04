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

#include <algorithm>
#include <sstream>
#include <vector>

using std::begin;
using std::end;
using std::for_each;
using std::runtime_error;
using std::stringstream;
using std::vector;
using thinks::obj_io::make_positions;
using util::CubeMesh;
using util::IncrementAndClampToMaxElement;
using util::IncrementNonZeroElements;
using util::VertexCount;


TEST(PositionsTest, CtorThrowsIfEmptyElements)
{
  const auto pos = vector<float>{};
  const auto idx = vector<uint32_t>{ 0, 1, 2 };
  const auto components_per_vertex = uint32_t{ 3 };
  const auto indices_per_face = uint32_t{ 3 };
  try {
    const auto positions = make_positions(
      begin(pos), end(pos),
      begin(idx), end(idx),
      components_per_vertex, indices_per_face);
    FAIL() << "exception not thrown";
  }
  catch (runtime_error& ex) {
    EXPECT_STREQ("position components cannot be empty", ex.what());
  }
  catch (...) {
    FAIL() << "incorrect exception";
  }
}

TEST(PositionsTest, CtorThrowsIfElementsPerVertexIsNotThreeOrFour)
{
  const auto mesh = CubeMesh();
  const auto components_per_vertex = uint32_t{ 5 };
  const auto indices_per_face = uint32_t{ 3 };
  try {
    const auto positions = make_positions(
      begin(mesh.position_elements), end(mesh.position_elements),
      begin(mesh.position_indices), end(mesh.position_indices),
      components_per_vertex, indices_per_face);
    FAIL() << "exception not thrown";
  }
  catch (runtime_error& ex) {
    auto ss = stringstream();
    ss << "position components per vertex must be 3 or 4, was " 
      << components_per_vertex;
    EXPECT_STREQ(ss.str().c_str(), ex.what());
  }
  catch (...) {
    FAIL() << "incorrect exception";
  }
}

TEST(PositionsTest, CtorThrowsIfElementCountNotMultipleOfElementsPerVertex)
{
  // Add an extra position element.
  auto mesh = CubeMesh();
  mesh.position_elements.push_back(25.f);
  const auto components_per_vertex = uint32_t{ 3 };
  const auto indices_per_face = uint32_t{ 3 };
  try {
    const auto positions = make_positions(
      begin(mesh.position_elements), end(mesh.position_elements),
      begin(mesh.position_indices), end(mesh.position_indices),
      components_per_vertex, indices_per_face);
    FAIL() << "exception not thrown";
  }
  catch (runtime_error& ex) {
    auto ss = stringstream();
    ss << "position component count (" << mesh.position_elements.size()
      << ") must be a multiple of " << components_per_vertex;
    EXPECT_STREQ(ss.str().c_str(), ex.what());
  }
  catch (...) {
    FAIL() << "incorrect exception";
  }
}

TEST(PositionsTest, CtorThrowsIfEmptyIndices)
{
  auto mesh = CubeMesh();
  mesh.position_indices.clear();
  const auto components_per_vertex = uint32_t{ 3 };
  const auto indices_per_face = uint32_t{ 3 };
  try {
    const auto positions = make_positions(
      begin(mesh.position_elements), end(mesh.position_elements),
      begin(mesh.position_indices), end(mesh.position_indices),
      components_per_vertex, indices_per_face);
    FAIL() << "exception not thrown";
  }
  catch (runtime_error& ex) {
    EXPECT_STREQ("position indices cannot be empty", ex.what());
  }
  catch (...) {
    FAIL() << "incorrect exception";
  }
}

TEST(PositionsTest, CtorThrowsIfIndicesPerFaceIsLessThanThree)
{
  const auto mesh = CubeMesh();
  const auto components_per_vertex = uint32_t{ 3 };
  const auto indices_per_face = uint32_t{ 2 };
  try {
    const auto positions = make_positions(
      begin(mesh.position_elements), end(mesh.position_elements),
      begin(mesh.position_indices), end(mesh.position_indices),
      components_per_vertex, indices_per_face);
    FAIL() << "exception not thrown";
  }
  catch (runtime_error& ex) {
    auto ss = stringstream();
    ss << "position indices per face cannot be less than 3, was "
      << indices_per_face;
    EXPECT_STREQ(ss.str().c_str(), ex.what());
  }
  catch (...) {
    FAIL() << "incorrect exception";
  }
}

TEST(PositionsTest, CtorThrowsIfIndexCountNotMultipleOfIndicesPerFace)
{
  // Add an extra index element.
  auto mesh = CubeMesh();
  mesh.position_indices.push_back(0);
  const auto components_per_vertex = uint32_t{ 3 };
  const auto indices_per_face = uint32_t{ 3 };
  try {
    const auto positions = make_positions(
      begin(mesh.position_elements), end(mesh.position_elements),
      begin(mesh.position_indices), end(mesh.position_indices),
      components_per_vertex, indices_per_face);
    FAIL() << "exception not thrown";
  }
  catch (runtime_error& ex) {
    auto ss = stringstream();
    ss << "position index count (" << mesh.position_indices.size()
      << ") must be a multiple of " << indices_per_face;
    EXPECT_STREQ(ss.str().c_str(), ex.what());
  }
  catch (...) {
    FAIL() << "incorrect exception";
  }
}

TEST(PositionsTest, CtorThrowIfInvalidIndexRange_MinNotZero)
{
  auto mesh = CubeMesh();
  IncrementAndClampToMaxElement(mesh.position_indices);
  const auto components_per_vertex = uint32_t{ 3 };
  const auto indices_per_face = uint32_t{ 3 };
  try {
    const auto positions = make_positions(
      begin(mesh.position_elements), end(mesh.position_elements),
      begin(mesh.position_indices), end(mesh.position_indices),
      components_per_vertex, indices_per_face);
    FAIL() << "exception not thrown";
  }
  catch (runtime_error& ex) {
    const auto min_index =
      *min_element(begin(mesh.position_indices), end(mesh.position_indices));
    auto ss = stringstream();
    ss << "position min index must be zero, was " << min_index;
    EXPECT_STREQ(ss.str().c_str(), ex.what());
  }
  catch (...) {
    FAIL() << "incorrect exception";
  }
}

TEST(PositionsTest, CtorThrowIfInvalidIndexRange_MaxTooHigh)
{
  auto mesh = CubeMesh();
  IncrementNonZeroElements(mesh.position_indices);
  const auto components_per_vertex = uint32_t{ 3 };
  const auto indices_per_face = uint32_t{ 3 };
  try {
    const auto positions = make_positions(
      begin(mesh.position_elements), end(mesh.position_elements),
      begin(mesh.position_indices), end(mesh.position_indices),
      components_per_vertex, indices_per_face);
    FAIL() << "exception not thrown";
  }
  catch (runtime_error& ex) {
    const auto max_index =
      *max_element(begin(mesh.position_indices), end(mesh.position_indices));
    auto ss = stringstream();
    ss << "position max index must be less than vertex count ("
      << VertexCount(mesh.position_elements, components_per_vertex) << "), was " 
      << max_index;
    EXPECT_STREQ(ss.str().c_str(), ex.what());
  }
  catch (...) {
    FAIL() << "incorrect exception";
  }
}
