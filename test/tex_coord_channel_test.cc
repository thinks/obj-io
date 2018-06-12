// Copyright(C) 2018 Tommy Hinks <tommy.hinks@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top - level directory of this distribution.

#include <thinks/obj_io/obj_io.h>

#include <sstream>
#include <vector>

#include <gtest/gtest.h>

using std::begin;
using std::end;
using std::invalid_argument;
using std::stringstream;
using std::vector;
using thinks::obj_io::make_tex_coord_channel;


TEST(TexCoordChannelTest, CtorThrowsIfComponentsPerValueIsNotTwoOrThree)
{
  const auto components_per_value = uint32_t{ 1 };
  const auto indices_per_face = uint32_t{ 3 };
  try {
    const auto channel = make_tex_coord_channel(
      vector<float>{ 0.1f, 0.2f, 0.3f }, components_per_value,
      vector<uint32_t>{ 0, 1, 2 }, indices_per_face);
    FAIL() << "exception not thrown";
  }
  catch (const invalid_argument& ex) {
    auto ss = stringstream();
    ss << "tex coord components per value (" 
      << components_per_value << ") must be 2 or 3";
    EXPECT_STREQ(ss.str().c_str(), ex.what());
  }
  catch (...) {
    FAIL() << "incorrect exception";
  }
}

TEST(TexCoordChannelTest, CtorThrowIfComponentsNotNormalized)
{
  const auto invalid_components = vector<float>{ -0.1f, 1.1f };
  for (const auto invalid_component : invalid_components) {
    const auto components_per_value = uint32_t{ 2 };
    const auto indices_per_face = uint32_t{ 3 };
    try {
      const auto channel = make_tex_coord_channel(
        vector<float>{ 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, invalid_component}, 
        components_per_value,
        vector<uint32_t>{ 0, 1, 2 }, indices_per_face);
      FAIL() << "exception not thrown";
    }
    catch (const invalid_argument& ex) {
      auto ss = stringstream();
      ss << "tex coord elements must be in range [0, 1], found " 
        << invalid_component;
      EXPECT_STREQ(ss.str().c_str(), ex.what());
    }
    catch (...) {
      FAIL() << "incorrect exception";
    }
  }
}
