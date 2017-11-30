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
using thinks::obj_io::make_tex_coords;
using util::CubeMesh;
using util::IncrementAndClampToMaxElement;
using util::IncrementNonZeroElements;
using util::VertexCount;


TEST(TexCoordsTest, Ctor)
{
  const auto mesh = CubeMesh();
  const auto elements_per_vertex = uint32_t{ 2 };
  const auto indices_per_face = uint32_t{ 3 };
  const auto tex_coords = make_tex_coords(
    begin(mesh.tex_coord_elements), end(mesh.tex_coord_elements),
    begin(mesh.tex_coord_indices), end(mesh.tex_coord_indices),
    elements_per_vertex, indices_per_face);
  EXPECT_EQ(begin(mesh.tex_coord_elements), tex_coords.elements_begin);
  EXPECT_EQ(end(mesh.tex_coord_elements), tex_coords.elements_end);
  EXPECT_EQ(begin(mesh.tex_coord_indices), tex_coords.indices_begin);
  EXPECT_EQ(end(mesh.tex_coord_indices), tex_coords.indices_end);
  EXPECT_EQ(2, tex_coords.elements_per_vertex);
  EXPECT_EQ(3, tex_coords.indices_per_face);
}

TEST(TexCoordsTest, CtorEmptyElementsAndIndices)
{
  FAIL() << "not implemented";
}

TEST(TexCoordsTest, CtorThrowIfElementsNotNormalized)
{
  const auto invalid_elements = vector<float>{ -0.1f, 1.1f };
  for (const auto invalid_element : invalid_elements) {
    auto mesh = CubeMesh();
    mesh.tex_coord_elements.back() = invalid_element;
    const auto elements_per_vertex = uint32_t{ 2 };
    const auto indices_per_face = uint32_t{ 3 };
    try {
      const auto tex_coords = make_tex_coords(
        begin(mesh.tex_coord_elements), end(mesh.tex_coord_elements),
        begin(mesh.tex_coord_indices), end(mesh.tex_coord_indices),
        elements_per_vertex, indices_per_face);
      FAIL() << "exception not thrown";
    }
    catch (const runtime_error& ex) {
      auto ss = stringstream();
      ss << "tex coord elements must be in range [0, 1], found " 
        << invalid_element;
      EXPECT_STREQ(ss.str().c_str(), ex.what());
    }
    catch (...) {
      FAIL() << "incorrect exception";
    }
  }
}

TEST(TexCoordsTest, CtorThrowsIfElementsPerVertexIsNotTwoOrThree)
{
  const auto mesh = CubeMesh();
  const auto elements_per_vertex = uint32_t{ 5 };
  const auto indices_per_face = uint32_t{ 3 };
  try {
    const auto tex_coords = make_tex_coords(
      begin(mesh.tex_coord_elements), end(mesh.tex_coord_elements),
      begin(mesh.tex_coord_indices), end(mesh.tex_coord_indices),
      elements_per_vertex, indices_per_face);
    FAIL() << "exception not thrown";
  }
  catch (const runtime_error& ex) {
    auto ss = stringstream();
    ss << "tex coord elements per vertex must be 2 or 3, was "
      << elements_per_vertex;
    EXPECT_STREQ(ss.str().c_str(), ex.what());
  }
  catch (...) {
    FAIL() << "incorrect exception";
  }
}

TEST(TexCoordsTest, CtorThrowsIfElementCountNotMultipleOfElementsPerVertex)
{
  // Add an extra element.
  auto mesh = CubeMesh();
  mesh.tex_coord_elements.push_back(0.25f);
  const auto elements_per_vertex = uint32_t{ 2 };
  const auto indices_per_face = uint32_t{ 3 };
  try {
    const auto tex_coords = make_tex_coords(
      begin(mesh.tex_coord_elements), end(mesh.tex_coord_elements),
      begin(mesh.tex_coord_indices), end(mesh.tex_coord_indices),
      elements_per_vertex, indices_per_face);
    FAIL() << "exception not thrown";
  }
  catch (const runtime_error& ex) {
    auto ss = stringstream();
    ss << "tex coord element count (" << mesh.tex_coord_elements.size()
      << ") must be a multiple of " << elements_per_vertex;
    EXPECT_STREQ(ss.str().c_str(), ex.what());
  }
  catch (...) {
    FAIL() << "incorrect exception";
  }
}

TEST(TexCoordsTest, CtorThrowsIfIndicesPerFaceIsLessThanThree)
{
  // Resize indices to be two per face.
  const auto elements_per_vertex = uint32_t{ 2 };
  const auto indices_per_face = uint32_t{ 2 };
  auto mesh = CubeMesh();
  mesh.tex_coord_indices.resize(
    (mesh.tex_coord_indices.size() / 3) * indices_per_face);
  try {
    const auto tex_coords = make_tex_coords(
      begin(mesh.tex_coord_elements), end(mesh.tex_coord_elements),
      begin(mesh.tex_coord_indices), end(mesh.tex_coord_indices),
      elements_per_vertex, indices_per_face);
    FAIL() << "exception not thrown";
  }
  catch (const runtime_error& ex) {
    auto ss = stringstream();
    ss << "tex coord indices per face must be "
      << "greater than or equal to 3, was "
      << indices_per_face;
    EXPECT_STREQ(ss.str().c_str(), ex.what());
  }
  catch (...) {
    FAIL() << "incorrect exception";
  }
}

TEST(TexCoordsTest, CtorThrowsIfIndexCountNotMultipleOfIndicesPerFace)
{
  // Add an extra index element.
  auto mesh = CubeMesh();
  mesh.tex_coord_indices.push_back(0);
  const auto elements_per_vertex = uint32_t{ 2 };
  const auto indices_per_face = uint32_t{ 3 };
  try {
    const auto tex_coords = make_tex_coords(
      begin(mesh.tex_coord_elements), end(mesh.tex_coord_elements),
      begin(mesh.tex_coord_indices), end(mesh.tex_coord_indices),
      elements_per_vertex, indices_per_face);
    FAIL() << "exception not thrown";
  }
  catch (const runtime_error& ex) {
    auto ss = stringstream();
    ss << "tex coord index count (" << mesh.tex_coord_indices.size()
      << ") must be a multiple of " << indices_per_face;
    EXPECT_STREQ(ss.str().c_str(), ex.what());
  }
  catch (...) {
    FAIL() << "incorrect exception";
  }
}

TEST(TexCoordsTest, CtorThrowIfInvalidIndexRange_MinNotZero)
{
  auto mesh = CubeMesh();
  IncrementAndClampToMaxElement(mesh.tex_coord_indices);
  const auto elements_per_vertex = uint32_t{ 2 };
  const auto indices_per_face = uint32_t{ 3 };
  try {
    const auto tex_coords = make_tex_coords(
      begin(mesh.tex_coord_elements), end(mesh.tex_coord_elements),
      begin(mesh.tex_coord_indices), end(mesh.tex_coord_indices),
      elements_per_vertex, indices_per_face);
    FAIL() << "exception not thrown";
  }
  catch (const runtime_error& ex) {
    const auto min_index = 
      *min_element(begin(mesh.tex_coord_indices), end(mesh.tex_coord_indices));
    auto ss = stringstream();
    ss << "min tex coord index must be zero, was " << min_index;
    EXPECT_STREQ(ss.str().c_str(), ex.what());
  }
  catch (...) {
    FAIL() << "incorrect exception";
  }
}

TEST(TexCoordsTest, CtorThrowIfInvalidIndexRange_MaxTooHigh)
{
  auto mesh = CubeMesh();
  IncrementNonZeroElements(mesh.tex_coord_indices);
  const auto elements_per_vertex = uint32_t{ 2 };
  const auto indices_per_face = uint32_t{ 3 };
  try {
    const auto tex_coords = make_tex_coords(
      begin(mesh.tex_coord_elements), end(mesh.tex_coord_elements),
      begin(mesh.tex_coord_indices), end(mesh.tex_coord_indices),
      elements_per_vertex, indices_per_face);
    FAIL() << "exception not thrown";
  }
  catch (const runtime_error& ex) {
    const auto max_index =
      *max_element(begin(mesh.tex_coord_indices), end(mesh.tex_coord_indices));
    auto ss = stringstream();
    ss << "max tex coord index must be less than vertex count ("
      << VertexCount(mesh.tex_coord_elements, elements_per_vertex) << "), was "
      << max_index;
    EXPECT_STREQ(ss.str().c_str(), ex.what());
  }
  catch (...) {
    FAIL() << "incorrect exception";
  }
}
