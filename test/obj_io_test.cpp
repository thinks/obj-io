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

#include "../include/thinks/obj_io.hpp"

#include <sstream>
#include <vector>

using std::begin;
using std::end;
using std::stringstream;
using std::vector;


TEST(ObjIoTest, Test)
{
  auto ss = stringstream();
  const auto pos = vector<float>{ 1.0f, 2.0f, 3.0f };
  const auto tex = vector<float>{ 0.1f, 0.2f };
  const auto idx = vector<uint32_t>{ 0, 1, 2 };
  thinks::obj_io::Write(
    ss,
    begin(pos), end(pos),
    begin(tex), end(tex),
    begin(idx), end(idx));

  EXPECT_TRUE(false) << ss.str();
}


