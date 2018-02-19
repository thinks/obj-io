// Copyright 2018 Tommy Hinks
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

#include <array>
#include <iostream>
#include <vector>

#include "gtest/gtest.h"

#include "thinks/obj_io_mapper.h"


template <typename T, std::size_t N>
class Vec
{
public:
  typedef T value_type;
  static const std::size_t size = N;
  Vec() {}
  T& operator[](std::size_t i) { return _data[i]; }
  const T& operator[](std::size_t i) const { return _data[i]; }
private:
  T _data[N];
};

struct Vec3f
{
  float x;
  float y;
  float z;
};



TEST(TestTest, MyTest)
{

  auto positions = std::vector<Vec3f>{
    Vec3f{1.f, 1.f, 1.f},
    Vec3f{2.f, 2.f, 2.f}, 
  };
  auto vec3f_mapper = [](const auto v) {
    return std::array<float, 3>{{ v.x, v.y, v.z }};
  };
  thinks::obj_io::Write(std::cout, positions, vec3f_mapper);

}