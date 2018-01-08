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
using thinks::obj_io::make_position_channel;
using thinks::obj_io::Write;

namespace {

struct Mesh
{
  uint32_t position_components_per_vertex;
  vector<float> position_components;
  uint32_t position_indices_per_face;
  vector<uint32_t> position_indices;
};

/// Returns a mesh with 8 * 3 vertex elements and 12 * 3 index elements.
/// Centered at origin, vertices in range [-1,1]
Mesh CubeMesh()
{
  auto mesh = Mesh{};
  mesh.position_components_per_vertex = 3;
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
  mesh.position_indices_per_face = 3;
  mesh.position_indices = vector<uint32_t>{
    0, 1, 2,
    1, 0, 3,
    0, 4, 5,
    4, 0, 2,
    6, 0, 5,
    0, 6, 3,
    1, 6, 7,
    6, 1, 3,
    1, 4, 2,
    4, 1, 7,
    6, 4, 7,
    4, 6, 5
  };
  return mesh;
}

} // namespace

TEST(WriteTest, PositionsOnly)
{
  auto mesh = CubeMesh();
  const auto positions = make_position_channel(
    begin(mesh.position_components), end(mesh.position_components),
    mesh.position_components_per_vertex,
    begin(mesh.position_indices), end(mesh.position_indices),
    mesh.position_indices_per_face);
  auto ss = stringstream();
  Write(ss, positions);

  std::cout << ss.str();
}

TEST(WriteTest, PositionsAndTexCoords)
{
  FAIL() << "todo";
}

TEST(WriteTest, PositionsAndNormals)
{
  FAIL() << "todo";
}

TEST(WriteTest, ThrowsIfChannelsNotEqualFaceCount)
{
  FAIL() << "todo";
}


