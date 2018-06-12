// Copyright(C) 2018 Tommy Hinks <tommy.hinks@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top - level directory of this distribution.

#include <thinks/obj_io/obj_io.h>

#include <gtest/gtest.h>

namespace {

using std::begin;
using std::end;
using std::invalid_argument;
using std::stringstream;
using std::vector;
using thinks::obj_io::make_normal_channel;
using thinks::obj_io::make_position_channel;
using thinks::obj_io::make_tex_coord_channel;
using thinks::obj_io::NormalChannel;
using thinks::obj_io::PositionChannel;
using thinks::obj_io::TexCoordChannel;

class NormalChannelCreator
{
public:
  template<typename ComponentType, typename IndexType>
  static 
  NormalChannel<
    typename vector<ComponentType>::const_iterator,
    typename vector<IndexType>::const_iterator>
  Create(
    const vector<ComponentType>& components,
    const uint32_t components_per_value,
    const vector<IndexType>& indices,
    const uint32_t indices_per_face)
  {
    return make_normal_channel(
      begin(components), end(components), components_per_value,
      begin(indices), end(indices), indices_per_face);
  }
};

class PositionChannelCreator
{
public:
  template<typename ComponentType, typename IndexType>
  static
  PositionChannel<
    typename vector<ComponentType>::const_iterator,
    typename vector<IndexType>::const_iterator>
  Create(
    const vector<ComponentType>& components,
    const uint32_t components_per_value,
    const vector<IndexType>& indices,
    const uint32_t indices_per_face)
  {
    return make_position_channel(
      begin(components), end(components), components_per_value,
      begin(indices), end(indices), indices_per_face);
  }
};

class TexCoordChannelCreator
{
public:
  template<typename ComponentType, typename IndexType>
  static
  TexCoordChannel<
    typename vector<ComponentType>::const_iterator,
    typename vector<IndexType>::const_iterator>
  Create(
    const vector<ComponentType>& components,
    const uint32_t components_per_value,
    const vector<IndexType>& indices,
    const uint32_t indices_per_face)
  {
    return make_tex_coord_channel(
      begin(components), end(components), components_per_value,
      begin(indices), end(indices), indices_per_face);
  }
};

typedef ::testing::Types<
  NormalChannelCreator,
  PositionChannelCreator,
  TexCoordChannelCreator> ChannelCreatorTypes;

template<typename T>
class CommonChannelTest : public ::testing::Test {
protected:
  virtual ~CommonChannelTest() {}
};

TYPED_TEST_CASE(CommonChannelTest, ChannelCreatorTypes);

TYPED_TEST(CommonChannelTest, CtorThrowsIfComponentCountIsZero)
{
  typedef TypeParam ChannelCreatorType;

  const auto components_per_value = uint32_t{ 3 };
  try {
    const auto channel = ChannelCreatorType::Create(
      vector<float>{}, components_per_value,
      vector<uint32_t>{ 0, 1, 2 }, 3);
    FAIL() << "exception not thrown";
  }
  catch (const invalid_argument& ex) {
    auto ss = stringstream();
    ss << "element count (" << 0
      << ") must be a multiple of elements per object ("
      << components_per_value << ")";
    EXPECT_STREQ(ss.str().c_str(), ex.what());
  }
  catch (...) {
    FAIL() << "incorrect exception";
  }
}

TYPED_TEST(CommonChannelTest, CtorThrowsIfComponentsPerValueIsZero)
{
  typedef TypeParam ChannelCreatorType;

  const auto components_per_value = uint32_t{ 0 };
  try {
    const auto channel = ChannelCreatorType::Create(
      vector<float>{ 0.1f, 0.1f }, components_per_value,
      vector<uint32_t>{ 0, 1, 2 }, 3);
  }
  catch (const invalid_argument& ex) {
    EXPECT_STREQ("elements per object cannot be zero", ex.what());
  }
  catch (...) {
    FAIL() << "incorrect exception";
  }
}

TYPED_TEST(CommonChannelTest, CtorThrowsIfComponentCountNotMultipleOfComponentsPerValue)
{
  typedef TypeParam ChannelCreatorType;

  const auto components_per_value = uint32_t{ 3 };
  try {
    const auto channel = ChannelCreatorType::Create(
      vector<float>{ 0.1f, 0.1f }, components_per_value,
      vector<uint32_t>{ 0, 1, 2 }, 3);
    FAIL() << "exception not thrown";
  }
  catch (const invalid_argument& ex) {
    auto ss = stringstream();
    ss << "element count (" << 2
      << ") must be a multiple of elements per object ("
      << components_per_value << ")";
    EXPECT_STREQ(ss.str().c_str(), ex.what());
  }
  catch (...) {
    FAIL() << "incorrect exception";
  }
}

TYPED_TEST(CommonChannelTest, CtorThrowsIfIndexCountIsZero)
{
  typedef TypeParam ChannelCreatorType;

  const auto indices_per_face = uint32_t{ 3 };
  try {
    const auto channel = ChannelCreatorType::Create(
      vector<float>{ 0.1f, 0.2f, 0.3f }, 3,
      vector<uint32_t>{}, indices_per_face);
    FAIL() << "exception not thrown";
  }
  catch (const invalid_argument& ex) {
    auto ss = stringstream();
    ss << "element count (" << 0
      << ") must be a multiple of elements per object ("
      << indices_per_face << ")";
    EXPECT_STREQ(ss.str().c_str(), ex.what());
  }
  catch (...) {
    FAIL() << "incorrect exception";
  }
}

TYPED_TEST(CommonChannelTest, CtorThrowsIfIndicesPerFaceIsLessThanThree)
{
  typedef TypeParam ChannelCreatorType;

  const auto indices_per_face = uint32_t{ 2 };
  try {
    const auto channel = ChannelCreatorType::Create(
      vector<float>{ 0.1f, 0.2f, 0.3f }, 3,
      vector<uint32_t>{ 0, 1 }, indices_per_face);
    FAIL() << "exception not thrown";
  }
  catch (const invalid_argument& ex) {
    auto ss = stringstream();
    ss << "indices per face (" << indices_per_face
      << ") cannot be less than 3";
    EXPECT_STREQ(ss.str().c_str(), ex.what());
  }
  catch (...) {
    FAIL() << "incorrect exception";
  }
}

TYPED_TEST(CommonChannelTest, CtorThrowsIfIndexCountNotMultipleOfIndicesPerFace)
{
  typedef TypeParam ChannelCreatorType;

  const auto indices_per_face = uint32_t{ 3 };
  try {
    const auto channel = ChannelCreatorType::Create(
      vector<float>{ 0.1f, 0.2f, 0.3f }, 3,
      vector<uint32_t>{ 0, 0, 0, 0 }, indices_per_face);
    FAIL() << "exception not thrown";
  }
  catch (const invalid_argument& ex) {
    auto ss = stringstream();
    ss << "element count (" << 4
      << ") must be a multiple of elements per object ("
      << indices_per_face << ")";
    EXPECT_STREQ(ss.str().c_str(), ex.what());
  }
  catch (...) {
    FAIL() << "incorrect exception";
  }
}

TYPED_TEST(CommonChannelTest, CtorThrowIfInvalidIndexRange_MinNotZero)
{
  typedef TypeParam ChannelCreatorType;

  try {
    const auto channel = ChannelCreatorType::Create(
      vector<float>{
        0.1f, 0.2f, 0.3f,
        0.1f, 0.2f, 0.3f,
        0.1f, 0.2f, 0.3f,
        0.1f, 0.2f, 0.3f }, 3,
      vector<uint32_t>{ 1, 2, 3 }, 3);
    FAIL() << "exception not thrown";
  }
  catch (const invalid_argument& ex) {
    auto ss = stringstream();
    ss << "min index (" << 1 << ") must be zero";
    EXPECT_STREQ(ss.str().c_str(), ex.what());
  }
  catch (...) {
    FAIL() << "incorrect exception";
  }
}

TYPED_TEST(CommonChannelTest, CtorThrowIfInvalidIndexRange_MaxNotLessThanValueCount)
{
  typedef TypeParam ChannelCreatorType;

  try {
    const auto channel = ChannelCreatorType::Create(
      vector<float>{
        0.1f, 0.2f, 0.3f,
        0.1f, 0.2f, 0.3f,
        0.1f, 0.2f, 0.3f }, 3,
      vector<uint32_t>{ 0, 1, 3 }, 3);
    FAIL() << "exception not thrown";
  }
  catch (const invalid_argument& ex) {
    auto ss = stringstream();
    ss << "max index (" << 3
      << ") must be less than value count (" << 3 << ")";
    EXPECT_STREQ(ss.str().c_str(), ex.what());
  }
  catch (...) {
    FAIL() << "incorrect exception";
  }
}

} // namespace