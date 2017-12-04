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

#include <algorithm>
#include <vector>

namespace util {

struct Mesh
{
  std::vector<float> position_elements;
  std::vector<uint32_t> position_indices;

  std::vector<float> tex_coord_elements;
  std::vector<uint32_t> tex_coord_indices;

  std::vector<float> normal_elements;
  std::vector<uint32_t> normal_indices;
};

/// Returns a mesh with:
/// -  8 * 3 position elements 
/// - 12 * 3 position index elements
/// -  4 * 2 tex coord elements
/// - 12 * 3 tex coord indices
/// -  6 * 3 normal elements
/// - 12 * 3 normal indices
///
/// Centered at origin, vertices in range [-1,1].
Mesh CubeMesh();


template <typename T> inline
void IncrementAndClampToMaxElement(
  std::vector<T>& v)
{
  const auto max_element_iter = std::max_element(begin(v), end(v));
  if (max_element_iter == end(v)) {
    throw std::runtime_error("undefined max element");
  }

  std::for_each(begin(v), end(v), 
    [=](T& i) { 
      ++i;
      if (i > *max_element_iter) {
        i = *max_element_iter;
      }
    });
}

template <typename T> inline
void IncrementNonZeroElements(std::vector<T>& v)
{
  std::for_each(begin(v), end(v),
    [](T& i) {
      if (i != T(0)) {
        ++i;
      }
    });
}

template <typename T> inline
uint32_t VertexCount(
  const std::vector<T>& elements,
  const uint32_t components_per_vertex)
{
  return static_cast<uint32_t>(elements.size() / components_per_vertex);
}

} // namespace util


