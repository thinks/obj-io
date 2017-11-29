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

#include "utils.h"

#include <vector>

using std::vector;

namespace util {

Mesh CubeMesh()
{
  auto mesh = Mesh{};
  mesh.position_elements = vector<float>{
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
  mesh.tex_coord_elements = vector<float>{
    0.0, 0.0,
    1.0, 0.0,
    0.0, 1.0,
    1.0, 1.0
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

  return mesh;
}

} // namespace util
