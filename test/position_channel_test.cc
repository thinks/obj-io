// Copyright(C) 2018 Tommy Hinks <tommy.hinks@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top - level directory of this distribution.

#include <thinks/obj_io/obj_io.h>

#include <sstream>
#include <vector>

#include <gtest/gtest.h>

using std::invalid_argument;
using std::stringstream;
using std::vector;
using thinks::obj_io::make_position_channel;


TEST(PositionChannelTest, CtorThrowsIfComponentsPerValueIsNotThreeOrFour)
{
  const auto components_per_value = uint32_t{ 2 };
  try {
    const auto channel = make_position_channel(
      vector<float>{ 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f }, components_per_value,
      vector<uint32_t>{ 0, 1, 2 }, 3);
    FAIL() << "exception not thrown";
  }
  catch (const invalid_argument& ex) {
    auto ss = stringstream();
    ss << "position components per value (" 
      << components_per_value << ") must be 3 or 4";
    EXPECT_STREQ(ss.str().c_str(), ex.what());
  }
  catch (...) {
    FAIL() << "incorrect exception";
  }
}
