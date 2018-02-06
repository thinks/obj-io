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

#ifndef THINKS_OBJ_IO_H_INCLUDED
#define THINKS_OBJ_IO_H_INCLUDED

#include <algorithm>
#include <array>
#include <cassert>
#include <cstring>
#include <exception>
#include <memory>
#include <iterator>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

namespace thinks {
namespace obj_io {
namespace detail {

template<typename Iter>
uint32_t Count(const Iter iter_begin, const Iter iter_end)
{
  return static_cast<uint32_t>(std::distance(iter_begin, iter_end));
}

template<typename Iter>
bool IsEmpty(const Iter iter_begin, const Iter iter_end)
{
  return iter_begin == iter_end;
}

inline
uint32_t IsMultipleOf(const uint32_t count, const uint32_t denominator)
{
  if (denominator == 0) {
    throw std::invalid_argument("denominator cannot be zero");
  }
  return count != 0 && count % denominator == 0;
}


template<typename IterType>
typename std::iterator_traits<IterType>::value_type MinElement(
  const IterType iter_begin,
  const IterType iter_end)
{
  if (IsEmpty(iter_begin, iter_end)) {
    throw std::invalid_argument("cannot find min element of empty range");
  }
  return *std::min_element(iter_begin, iter_end);
}

template<typename IterType>
typename std::iterator_traits<IterType>::value_type MaxElement(
  const IterType iter_begin,
  const IterType iter_end)
{
  if (IsEmpty(iter_begin, iter_end)) {
    throw std::invalid_argument("cannot find max element of empty range");
  }
  return *std::max_element(iter_begin, iter_end);
}


template<typename ElementIter>
class Channel
{
public:
  Channel(
    const ElementIter elements_begin,
    const ElementIter elements_end,
    const uint32_t elements_per_object)
    : elements_begin(elements_begin)
    , elements_end(elements_end)
    , elements_per_object(elements_per_object)
  {
    ThrowIfElementsPerObjectIsZero_();
    ThrowIfElementCountNotMultipleOfElementsPerObject_();
    // Note that this implies that element count cannot be zero.
  }

  const ElementIter elements_begin;
  const ElementIter elements_end;
  const uint32_t elements_per_object;

private:
  void ThrowIfElementsPerObjectIsZero_()
  {
    if (elements_per_object == 0) {
      throw std::invalid_argument("elements per object cannot be zero");
    }
  }

  void ThrowIfElementCountNotMultipleOfElementsPerObject_()
  {
    const auto element_count = Count(elements_begin, elements_end);
    if (!IsMultipleOf(element_count, elements_per_object)) {
      auto ss = std::stringstream();
      ss << "element count (" << element_count
        << ") must be a multiple of elements per object (" 
        << elements_per_object << ")";
      throw std::invalid_argument(ss.str());
    }
  }
};


template<typename ComponentIter>
class ValueChannel
{
public:
  ValueChannel(
    const ComponentIter components_begin,
    const ComponentIter components_end,
    const uint32_t components_per_value)
    : channel_(components_begin, components_end, components_per_value)
  {
    typedef typename std::iterator_traits<ComponentIter>::value_type ComponentType;
    static_assert(std::is_arithmetic<ComponentType>::value,
      "components must be arithmetic");
  }

  ComponentIter components_begin() const { return channel_.elements_begin; }
  ComponentIter components_end() const { return channel_.elements_end; }
  uint32_t components_per_value() const { return channel_.elements_per_object; }

private:
  const detail::Channel<ComponentIter> channel_;
};


template<typename IndexIter>
struct FaceChannel
{
public:
  FaceChannel(
    const IndexIter indices_begin,
    const IndexIter indices_end,
    const uint32_t indices_per_face)
    : channel_(indices_begin, indices_end, indices_per_face)
  {
    typedef typename std::iterator_traits<IndexIter>::value_type IndexType;
    static_assert(std::is_integral<IndexType>::value,
      "indices must be integral");

    ThrowIfIndicesPerFaceLessThanThree_();
  }

  IndexIter indices_begin() const { return channel_.elements_begin; }
  IndexIter indices_end() const { return channel_.elements_end; }
  uint32_t indices_per_face() const { return channel_.elements_per_object; }

private:
  const detail::Channel<IndexIter> channel_;

  void ThrowIfIndicesPerFaceLessThanThree_()
  {
    if (!(indices_per_face() >= 3)) {
      auto ss = std::stringstream();
      ss << "indices per face (" << indices_per_face()
        << ") cannot be less than 3";
      throw std::invalid_argument(ss.str());
    }
  }
};


template<typename ComponentIter>
uint32_t ValueCount(const ValueChannel<ComponentIter>& value_channel)
{
  const auto component_count = Count(
    value_channel.components_begin(),
    value_channel.components_end());
  return component_count / value_channel.components_per_value();
}

template<typename IndexIter>
uint32_t FaceCount(const FaceChannel<IndexIter>& face_channel)
{
  const auto index_count = Count(
    face_channel.indices_begin,
    face_channel.indices_end);
  return index_count / face_channel.indices_per_face;
}


template<typename ComponentIter, typename IndexIter>
class IndexedValueChannel
{
public:
  IndexedValueChannel(
    const ComponentIter components_begin,
    const ComponentIter components_end,
    const uint32_t components_per_value,
    const IndexIter indices_begin,
    const IndexIter indices_end,
    const uint32_t indices_per_face)
    : value_channel_(components_begin, components_end, components_per_value)
    , face_channel_(indices_begin, indices_end, indices_per_face)
  {
    ThrowIfInvalidIndexRange_();
  }

  ComponentIter components_begin() const { return value_channel_.components_begin(); }
  ComponentIter components_end() const { return value_channel_.components_end(); }
  uint32_t components_per_value() const { return value_channel_.components_per_value(); }

  IndexIter indices_begin() const { return face_channel_.indices_begin(); }
  IndexIter indices_end() const { return face_channel_.indices_end(); }
  uint32_t indices_per_face() const { return face_channel_.indices_per_face(); }

private:
  const ValueChannel<ComponentIter> value_channel_;
  const FaceChannel<IndexIter> face_channel_;

  void ThrowIfInvalidIndexRange_()
  {
    // TODO: Could check that all indices in range exist?!
    //       If they don't it means that there are unreferences values.
    const auto min_index = detail::MinElement(
      face_channel_.indices_begin(), face_channel_.indices_end());
    if (min_index != 0) {
      auto ss = std::stringstream();
      ss << "min index (" << min_index << ") must be zero";
      throw std::invalid_argument(ss.str());
    }

    const auto value_count = detail::ValueCount(value_channel_);
    const auto max_index = detail::MaxElement(
      face_channel_.indices_begin(), face_channel_.indices_end());
    if (!(max_index < value_count)) {
      auto ss = std::stringstream();
      ss << "max index (" << max_index
        << ") must be less than value count ("
        << value_count << ")";
      throw std::invalid_argument(ss.str());
    }
  }
};

} // namespace detail


template<typename ComponentIter, typename IndexIter>
class PositionChannel
{
public:
  PositionChannel(
    const ComponentIter components_begin,
    const ComponentIter components_end,
    const uint32_t components_per_value,
    const IndexIter indices_begin,
    const IndexIter indices_end,
    const uint32_t indices_per_face)
    : indexed_value_channel_(
        components_begin, components_end, components_per_value,
        indices_begin, indices_end, indices_per_face)
  {
    ThrowIfComponentsPerValueIsNotThreeOrFour_();
  }

  ComponentIter components_begin() const { return indexed_value_channel_.components_begin(); }
  ComponentIter components_end() const { return indexed_value_channel_.components_end(); }
  uint32_t components_per_value() const { return indexed_value_channel_.components_per_value(); }

  IndexIter indices_begin() const { return indexed_value_channel_.indices_begin(); }
  IndexIter indices_end() const { return indexed_value_channel_.indices_end(); }
  uint32_t indices_per_face() const { return indexed_value_channel_.indices_per_face(); }

private:
  const detail::IndexedValueChannel<ComponentIter, IndexIter> 
    indexed_value_channel_;

  void ThrowIfComponentsPerValueIsNotThreeOrFour_()
  {
    if (!(components_per_value() == 3 || components_per_value() == 4)) {
      auto ss = std::stringstream();
      ss << "position components per value ("
        << components_per_value() << ") must be 3 or 4";
      throw std::invalid_argument(ss.str());
    }
  }
};

/// Named constructor to help with template type deduction.
template<typename ComponentIter, typename IndexIter>
PositionChannel<ComponentIter, IndexIter> make_position_channel(
  const ComponentIter components_begin,
  const ComponentIter components_end,
  const uint32_t components_per_value,
  const IndexIter indices_begin,
  const IndexIter indices_end,
  const uint32_t indices_per_face)
{
  return PositionChannel<ComponentIter, IndexIter>(
    components_begin, components_end, components_per_value,
    indices_begin, indices_end, indices_per_face);
}

template<typename ComponentType, typename IndexType>
PositionChannel<
  typename std::vector<ComponentType>::const_iterator, 
  typename std::vector<IndexType>::const_iterator> 
make_position_channel(
  const std::vector<ComponentType>& components,
  const uint32_t components_per_value,
  const std::vector<IndexType>& indices,
  const uint32_t indices_per_face)
{
  return make_position_channel(
    std::begin(components), std::end(components), components_per_value,
    std::begin(indices), std::end(indices), indices_per_face);
}


template<typename ComponentIter, typename IndexIter>
class TexCoordChannel
{
public:
  TexCoordChannel(
    const ComponentIter components_begin,
    const ComponentIter components_end,
    const uint32_t components_per_value,
    const IndexIter indices_begin,
    const IndexIter indices_end,
    const uint32_t indices_per_face)
    : indexed_value_channel_(
        components_begin, components_end, components_per_value,
        indices_begin, indices_end, indices_per_face)
  {
    typedef typename std::iterator_traits<ComponentIter>::value_type ComponentType;
    static_assert(std::is_floating_point<ComponentType>::value,
      "tex coord components must be floating point");

    ThrowIfComponentsPerValueIsNotTwoOrThree_();
    ThrowIfComponentsNotNormalized_();
  }

  ComponentIter components_begin() const { return indexed_value_channel_.components_begin(); }
  ComponentIter components_end() const { return indexed_value_channel_.components_end(); }
  uint32_t components_per_value() const { return indexed_value_channel_.components_per_value(); }

  IndexIter indices_begin() const { return indexed_value_channel_.indices_begin(); }
  IndexIter indices_end() const { return indexed_value_channel_.indices_end(); }
  uint32_t indices_per_face() const { return indexed_value_channel_.indices_per_face(); }

private:
  const detail::IndexedValueChannel<ComponentIter, IndexIter>
    indexed_value_channel_;

  void ThrowIfComponentsPerValueIsNotTwoOrThree_()
  {
    if (!(components_per_value() == 2 || components_per_value() == 3)) {
      auto ss = std::stringstream();
      ss << "tex coord components per value (" 
        << components_per_value() << ") must be 2 or 3";
      throw std::invalid_argument(ss.str());
    }
  }

  void ThrowIfComponentsNotNormalized_()
  {
    typedef typename std::iterator_traits<ComponentIter>::value_type ComponentType;
    std::for_each(components_begin(), components_end(),
      [](const auto e) {
        if (!(ComponentType(0) <= e && e <= ComponentType(1))) {
          auto ss = std::stringstream();
          ss << "tex coord elements must be in range [0, 1], found " << e;
          throw std::invalid_argument(ss.str());
        }
      });
  }
};

/// Named constructor to help with template type deduction.
template<typename ComponentIter, typename IndexIter>
TexCoordChannel<ComponentIter, IndexIter> make_tex_coord_channel(
  const ComponentIter components_begin,
  const ComponentIter components_end,
  const uint32_t components_per_value,
  const IndexIter indices_begin,
  const IndexIter indices_end,
  const uint32_t indices_per_face)
{
  return TexCoordChannel<ComponentIter, IndexIter>(
    components_begin, components_end, components_per_value,
    indices_begin, indices_end, indices_per_face);
}

/// Named constructor to help with template type deduction.
template<typename ComponentType, typename IndexType>
TexCoordChannel<
  typename std::vector<ComponentType>::const_iterator,
  typename std::vector<IndexType>::const_iterator>
make_tex_coord_channel(
    const std::vector<ComponentType>& components,
    const uint32_t components_per_value,
    const std::vector<IndexType>& indices,
    const uint32_t indices_per_face)
{
  return make_tex_coord_channel(
    std::begin(components), std::end(components), components_per_value,
    std::begin(indices), std::end(indices), indices_per_face);
}


template<typename ComponentIter, typename IndexIter>
class NormalChannel
{
public:
  NormalChannel(
    const ComponentIter components_begin,
    const ComponentIter components_end,
    const uint32_t components_per_value,
    const IndexIter indices_begin,
    const IndexIter indices_end,
    const uint32_t indices_per_face)
    : indexed_value_channel_(
        components_begin, components_end, components_per_value,
        indices_begin, indices_end, indices_per_face)
  {
    typedef typename std::iterator_traits<ComponentIter>::value_type ComponentType;
    static_assert(std::is_floating_point<ComponentType>::value,
      "normal components must be floating point");

    ThrowIfComponentsPerValueIsNotThree_();
  }

  ComponentIter components_begin() const { return indexed_value_channel_.components_begin(); }
  ComponentIter components_end() const { return indexed_value_channel_.components_end(); }
  uint32_t components_per_value() const { return indexed_value_channel_.components_per_value(); }

  IndexIter indices_begin() const { return indexed_value_channel_.indices_begin(); }
  IndexIter indices_end() const { return indexed_value_channel_.indices_end(); }
  uint32_t indices_per_face() const { return indexed_value_channel_.indices_per_face(); }

private:
  const detail::IndexedValueChannel<ComponentIter, IndexIter>
    indexed_value_channel_;

  void ThrowIfComponentsPerValueIsNotThree_()
  {
    if (components_per_value() != 3) {
      auto ss = std::stringstream();
      ss << "normal components per value ("
        << components_per_value() << ") must be 3";
      throw std::invalid_argument(ss.str());
    }
  }
};

/// Named constructor to help with template type deduction.
template<typename ComponentIter, typename IndexIter>
NormalChannel<ComponentIter, IndexIter> make_normal_channel(
  const ComponentIter components_begin,
  const ComponentIter components_end,
  const uint32_t components_per_value,
  const IndexIter indices_begin,
  const IndexIter indices_end,
  const uint32_t indices_per_face)
{
  return NormalChannel<ComponentIter, IndexIter>(
    components_begin, components_end, components_per_value,
    indices_begin, indices_end, indices_per_face);
}

/// Named constructor to help with template type deduction.
template<typename ComponentType, typename IndexType>
NormalChannel<
  typename std::vector<ComponentType>::const_iterator,
  typename std::vector<IndexType>::const_iterator>
make_normal_channel(
  const std::vector<ComponentType>& components,
  const uint32_t components_per_value,
  const std::vector<IndexType>& indices,
  const uint32_t indices_per_face)
{
  return make_normal_channel(
    std::begin(components), std::end(components), components_per_value,
    std::begin(indices), std::end(indices), indices_per_face);
}


namespace detail {

template<
  typename PosIndexType, 
  typename TexIndexType, 
  typename NmlIndexType>
void ThrowIfIndicesPerFaceNotEqualForAllChannels(
  const PosIndexType pos_indices_per_face,
  const TexIndexType tex_indices_per_face,
  const NmlIndexType nml_indices_per_face)
{
  const auto invalid_tex =
    tex_indices_per_face != TexIndexType{ 0 } &&
    tex_indices_per_face != pos_indices_per_face;
  const auto invalid_nml =
    nml_indices_per_face != NmlIndexType{ 0 } &&
    nml_indices_per_face != pos_indices_per_face;
  if (invalid_tex || invalid_nml) {
    auto ss = std::stringstream();
    ss << "indices per face must be equal for all channels: "
      << "positions (" << pos_indices_per_face << ")";
    if (tex_indices_per_face != TexIndexType{ 0 }) {
      ss << ", tex_coords (" << tex_indices_per_face << ")";
    }
    if (nml_indices_per_face != NmlIndexType{ 0 }) {
      ss << ", normals (" << nml_indices_per_face << ")";
    }
    throw std::invalid_argument(ss.str());
  }
}

inline
void ThrowIfIndexCountNotEqualForAllChannels(
  const uint32_t pos_index_count,
  const uint32_t tex_index_count,
  const uint32_t nml_index_count)
{
  assert(pos_index_count > 0 && 
    "position index count must be greater than zero");

  // Allow count to be zero for tex coord and normal indices.
  const auto invalid_tex =
    tex_index_count != 0 &&
    tex_index_count != pos_index_count;
  const auto invalid_nml =
    nml_index_count != 0 &&
    nml_index_count != pos_index_count;
  if (invalid_tex || invalid_nml) {
    auto ss = std::stringstream();
    ss << "index count must be equal for all channels: "
      << "positions (" << pos_index_count << ")";
    if (tex_index_count != 0) {
      ss << ", tex_coords (" << tex_index_count << ")";
    }
    if (nml_index_count != 0) {
      ss << ", normals (" << nml_index_count << ")";
    }
    throw std::invalid_argument(ss.str());
  }
}

template<typename ChannelType>
uint32_t ValueCount(const ChannelType& channel)
{
  assert(channel.components_per_value() != 0 &&
    "components per value cannot be zero");
  return Count(channel.components_begin(), channel.components_end()) /
    channel.components_per_value();
}

template<typename ChannelType>
uint32_t FaceCount(const ChannelType& channel)
{
  assert(channel.indices_per_face() != 0 && 
    "indices per face cannot be zero");
  return Count(channel.indices_begin(), channel.indices_end()) /
    channel.indices_per_face();
}

template<typename ComponentIter, typename IndexIter>
void WriteHeader(
  std::ostream& os,
  const PositionChannel<ComponentIter, IndexIter>& position_channel,
  const std::string& newline)
{
  os << "# Generated by https://github.com/thinks/obj-io" << newline
    << "# Vertex count: " << ValueCount(position_channel) << newline
    << "# Face count: " << FaceCount(position_channel) << newline;
}

template<typename ComponentIter>
void WriteValues(
  std::ostream& os,
  const std::string& line_start,
  const ComponentIter components_begin,
  const ComponentIter components_end,
  const uint32_t components_per_value,
  const std::string& newline)
{
  // TODO: no checks?

  // One line per value.
  auto component_iter = components_begin;
  while (component_iter != components_end) {
    os << line_start;
    for (auto i = uint32_t{ 0 }; i < components_per_value; ++i) {
      os << *component_iter++ << (i != components_per_value - 1 ? " " : "");
    }
    os << newline;
  }
}

template<typename PosIndexType, typename TexIndexType, typename NmlIndexType>
void WriteFaceIndex(
  std::ostream& os,
  const PosIndexType pos_index,
  const TexIndexType* const tex_index,
  const NmlIndexType* const nml_index)
{
  // One-based indices!
  os << pos_index + 1;
  if (tex_index != nullptr && nml_index != nullptr) {
    os << "/" << *tex_index + 1 << "/" << *nml_index + 1;
  }
  else if (tex_index != nullptr) {
    os << "/" << *tex_index + 1;
  }
  else if (nml_index != nullptr) {
    os << "//" << *nml_index + 1;
  }
}

template<typename PosIndexIter, typename TexIndexIter, typename NmlIndexIter>
void WriteFaces(
  std::ostream& os,
  const std::string& line_start,
  const PosIndexIter pos_indices_begin, 
  const PosIndexIter pos_indices_end,
  const uint32_t pos_indices_per_face,
  const TexIndexIter tex_indices_begin, 
  const TexIndexIter tex_indices_end,
  const uint32_t tex_indices_per_face,
  const NmlIndexIter nml_indices_begin,
  const NmlIndexIter nml_indices_end,
  const uint32_t nml_indices_per_face,
  const std::string& newline)
{
  ThrowIfIndicesPerFaceNotEqualForAllChannels(
    pos_indices_per_face,
    tex_indices_per_face,
    nml_indices_per_face);
  ThrowIfIndexCountNotEqualForAllChannels(
    Count(pos_indices_begin, pos_indices_end),
    Count(tex_indices_begin, tex_indices_end),
    Count(nml_indices_begin, nml_indices_end));
  // If these two tests hold then the face count is also equal.

  auto pos_index_iter = pos_indices_begin;
  auto tex_index_iter = tex_indices_begin;
  auto nml_index_iter = nml_indices_begin;
  while (pos_index_iter != pos_indices_end) {
    // One line per face.
    os << line_start;
    for (auto i = uint32_t{ 0 }; i < pos_indices_per_face; ++i) {
      WriteFaceIndex(os,
        *pos_index_iter,
        tex_index_iter != TexIndexIter{} ? &(*tex_index_iter) : nullptr,
        nml_index_iter != NmlIndexIter{} ? &(*nml_index_iter) : nullptr);
      ++pos_index_iter;
      if (tex_index_iter != TexIndexIter{}) {
        ++tex_index_iter;
      }
      if (nml_index_iter != NmlIndexIter{}) {
        ++nml_index_iter;
      }
      os << (i != pos_indices_per_face - 1 ? " " : "");
    }
    os << newline;
  }
}

template<
  typename PosCompIter, typename PosIndexIter,
  typename TexCompIter, typename TexIndexIter,
  typename NmlCompIter, typename NmlIndexIter>
std::ostream& Write(
  std::ostream& os,
  const PositionChannel<PosCompIter, PosIndexIter>& position_channel,
  const TexCoordChannel<TexCompIter, TexIndexIter>* const tex_coord_channel,
  const NormalChannel<NmlCompIter, NmlIndexIter>* const normal_channel,
  const std::string& newline)
{
  WriteHeader(os, position_channel, newline);
  WriteValues(os, "v ", 
    position_channel.components_begin(), 
    position_channel.components_end(),
    position_channel.components_per_value(),
    newline);
  
  if (tex_coord_channel != nullptr) {
    WriteValues(os, "vt ", 
      tex_coord_channel->components_begin(), 
      tex_coord_channel->components_end(),
      tex_coord_channel->components_per_value(),
      newline);
  }

  if (normal_channel != nullptr) {
    WriteValues(os, "vn ", 
      normal_channel->components_begin(),
      normal_channel->components_end(),
      normal_channel->components_per_value(),
      newline);
  }

  WriteFaces(os, "f ",
    position_channel.indices_begin(),
    position_channel.indices_end(),
    position_channel.indices_per_face(),
    tex_coord_channel != nullptr ? 
      tex_coord_channel->indices_begin() : TexIndexIter{} /* dummy */,
    tex_coord_channel != nullptr ? 
      tex_coord_channel->indices_end() : TexIndexIter{} /* dummy */,
    tex_coord_channel != nullptr ? 
      tex_coord_channel->indices_per_face() : 0,
    normal_channel != nullptr ? 
      normal_channel->indices_begin() : NmlIndexIter{} /* dummy */,
    normal_channel != nullptr ? 
      normal_channel->indices_end() : NmlIndexIter{} /* dummy */,
    normal_channel != nullptr ? 
      normal_channel->indices_per_face() : 0,
    newline);

  return os;
}

} // namespace detail


template<typename PosCompIter, typename PosIdxIter>
std::ostream& Write(
  std::ostream& os,
  const PositionChannel<PosCompIter, PosIdxIter>& position_channel,
  const std::string& newline = "\n")
{
  return detail::Write<
    PosCompIter, PosIdxIter,
    PosCompIter, PosIdxIter,  // Dummy!
    PosCompIter, PosIdxIter>( // Dummy!
      os,
      position_channel,
      nullptr, // No tex coords.
      nullptr, // No normals.
      newline);
}

template<
  typename PosCompIter, typename PosIdxIter,
  typename TexCompIter, typename TexIdxIter>
std::ostream& Write(
  std::ostream& os,
  const PositionChannel<PosCompIter, PosIdxIter>& position_channel,
  const TexCoordChannel<TexCompIter, TexIdxIter>& tex_coord_channel,
  const std::string& newline = "\n")
{
  return detail::Write<
    PosCompIter, PosIdxIter,
    TexCompIter, TexIdxIter,
    PosCompIter, PosIdxIter>( // Dummy!
      os,
      position_channel,
      &tex_coord_channel,
      nullptr, // No normals.
      newline);
}

template<
  typename PosCompIter, typename PosIndexIter,
  typename NmlCompIter, typename NmlIndexIter>
std::ostream& Write(
  std::ostream& os,
  const PositionChannel<PosCompIter, PosIndexIter>& position_channel,
  const NormalChannel<NmlCompIter, NmlIndexIter>& normal_channel,
  const std::string& newline = "\n")
{
  return detail::Write<
    PosCompIter, PosIndexIter,
    PosCompIter, PosIndexIter, // Dummy!
    NmlCompIter, NmlIndexIter>(
      os,
      position_channel,
      nullptr, // No tex coords.
      &normal_channel,
      newline);
}

template<
  typename PosCompIter, typename PosIndexIter,
  typename TexCompIter, typename TexIndexIter,
  typename NmlCompIter, typename NmlIndexIter>
std::ostream& Write(
  std::ostream& os,
  const PositionChannel<PosCompIter, PosIndexIter>& position_channel,
  const TexCoordChannel<TexCompIter, TexIndexIter>& tex_coord_channel,
  const NormalChannel<NmlCompIter, NmlIndexIter>& normal_channel,
  const std::string& newline = "\n")
{
  return detail::Write<
    PosCompIter, PosIndexIter,
    TexCompIter, TexIndexIter,
    NmlCompIter, NmlIndexIter>(
      os,
      position_channel,
      &tex_coord_channel,
      &normal_channel,
      newline);
}


namespace detail {

template<typename CompType>
void ParseComponents(
  std::istringstream* iss,
  std::vector<CompType>* const components,
  uint32_t* const components_per_value)
{
  auto components_size_before = components->size();
  auto component = CompType{};
  while (*iss >> component) {
    components->push_back(component);
  }
  const auto component_count = components->size() - components_size_before;

  if (*components_per_value == 0) {
    // If this is the first component stream to be unpacked 
    // (for a certain channel) store its component count. 
    // All subsequent components streams  must have the same count.
    *components_per_value = component_count;
  }
  else if (*components_per_value != component_count) {
    // throw! all positions must have same number of components per value
  }
}

inline
void ParseFaceIndices(
  std::istringstream* iss,
  std::vector<uint32_t>* position_indices,
  std::vector<uint32_t>* tex_coord_indices,
  std::vector<uint32_t>* normal_indices,
  uint32_t* const position_indices_per_face,
  uint32_t* const tex_coord_indices_per_face,
  uint32_t* const normal_indices_per_face)
{  
  auto pos_size_before = position_indices->size();
  auto tex_size_before = tex_coord_indices->size();
  auto nml_size_before = normal_indices->size();

  auto face_index_group = std::string();
  while (*iss >> face_index_group) {
    auto fig_ss = std::istringstream(face_index_group);
    auto index_str = std::string();
    auto indices = std::array<uint32_t, 3>{{0, 0, 0}};
    auto i = uint32_t{ 0 };
    while (std::getline(fig_ss, index_str, '/')) {
      auto index_ss = std::istringstream(index_str);
      index_ss >> indices[i++];
    }

    // Make indices zero-based!
    if (indices[0] == 0) {
      // throw - face must have position index!
    }
    position_indices->push_back(indices[0] - 1);
    if (indices[1] != 0) {
      tex_coord_indices->push_back(indices[1] - 1);
    }
    if (indices[2] != 0) {
      normal_indices->push_back(indices[2] - 1);
    }
  }
  const auto pos_count = position_indices->size() - pos_size_before;
  const auto tex_count = tex_coord_indices->size() - tex_size_before;
  const auto nml_count = normal_indices->size() - nml_size_before;

  if (*position_indices_per_face == 0) {
    *position_indices_per_face = pos_count;
  }
  else if (*position_indices_per_face != pos_count) {
    // throw
  }
  
  if (*tex_coord_indices_per_face == 0) {
    *tex_coord_indices_per_face = tex_count;
  }
  else if (*tex_coord_indices_per_face != tex_count) {
    // throw
  }

  if (*normal_indices_per_face == 0) {
    *normal_indices_per_face = nml_count;
  }
  else if (*normal_indices_per_face != nml_count) {
    // throw
  }

  // all face must have same number of index groups
  // all index groups must have same number of indices.
}

} // namespace detail


template<typename ComponentType>
class Mesh
{
public:
  typedef typename std::vector<ComponentType>::const_iterator ComponentIter;
  typedef typename std::vector<uint32_t>::const_iterator IndexIter;

  Mesh(
    const std::vector<ComponentType>& position_components, // Positions.
    const uint32_t position_components_per_value,
    const std::vector<uint32_t>& position_indices,
    const uint32_t position_indices_per_face,
    const std::vector<ComponentType>& tex_coord_components, // Tex coords.
    const uint32_t tex_coord_components_per_value,
    const std::vector<uint32_t>& tex_coord_indices,
    const uint32_t tex_coord_indices_per_face, 
    const std::vector<ComponentType>& normal_components, // Normals.
    const uint32_t normal_components_per_value,
    const std::vector<uint32_t>& normal_indices,
    const uint32_t normal_indices_per_face)
    : position_components_(position_components)
    , position_indices_(position_indices)
    , tex_coord_components_(tex_coord_components)
    , tex_coord_indices_(tex_coord_indices)
    , normal_components_(normal_components)
    , normal_indices_(normal_indices)
    , position_channel_(make_position_channel(
        position_components_,
        position_components_per_value,
        position_indices_,
        position_indices_per_face))
    , tex_coord_channel_(nullptr)
    , normal_channel_(nullptr)
  {
    if (tex_coord_indices.size() == position_indices.size() &&
        tex_coord_indices_per_face == position_indices_per_face) {
      tex_coord_channel_ = 
        std::make_unique<TexCoordChannel<ComponentIter, IndexIter>>(
          std::begin(tex_coord_components_),
          std::end(tex_coord_components_),
          tex_coord_components_per_value,
          std::begin(tex_coord_indices_),
          std::end(tex_coord_indices_),
          tex_coord_indices_per_face);
    }

    if (normal_indices.size() == position_indices.size() &&
        normal_indices_per_face == position_indices_per_face) {
      normal_channel_ = 
        std::make_unique<NormalChannel<ComponentIter, IndexIter>>(
          std::begin(normal_components_),
          std::end(normal_components_),
          normal_components_per_value,
          std::begin(normal_indices_),
          std::end(normal_indices_),
          normal_indices_per_face);
    }

    // TODO: check that channels are compatible!?
  }

  const PositionChannel<ComponentIter, IndexIter>& position_channel() const
  {
    return position_channel_;
  }

  const TexCoordChannel<ComponentIter, IndexIter>* tex_coord_channel() const
  {
    return tex_coord_channel_.get();
  }

  const NormalChannel<ComponentIter, IndexIter>* normal_channel() const
  {
    return normal_channel_.get();
  }

private:
  std::vector<ComponentType> position_components_;
  std::vector<uint32_t> position_indices_;

  std::vector<ComponentType> tex_coord_components_;
  std::vector<uint32_t> tex_coord_indices_;

  std::vector<ComponentType> normal_components_;
  std::vector<uint32_t> normal_indices_;

  PositionChannel<ComponentIter, IndexIter> position_channel_;
  std::unique_ptr<TexCoordChannel<ComponentIter, IndexIter>> tex_coord_channel_;
  std::unique_ptr<NormalChannel<ComponentIter, IndexIter>> normal_channel_;
};


template<typename ComponentType>
Mesh<ComponentType> Read(std::istream& is)
{
  using std::vector;

  // Positions.
  auto position_components = vector<ComponentType>{};
  auto position_components_per_value = uint32_t{ 0 };
  auto position_indices = vector<uint32_t>{};
  auto position_indices_per_face = uint32_t{ 0 };
  // Tex coords.
  auto tex_coord_components = vector<ComponentType>{};
  auto tex_coord_components_per_value = uint32_t{ 0 };
  auto tex_coord_indices = vector<uint32_t>{};
  auto tex_coord_indices_per_face = uint32_t{ 0 };
  // Normals.
  auto normal_components = vector<ComponentType>{};
  auto normal_components_per_value = uint32_t{ 0 };
  auto normal_indices = vector<uint32_t>{};
  auto normal_indices_per_face = uint32_t{ 0 };

  auto line = std::string();
  while (std::getline(is, line)) {
    auto iss = std::istringstream(line);

    auto prefix = std::string();
    std::getline(iss, prefix, ' ');

    if (prefix.empty() || prefix == "#") {
      // Ignore empty lines and comments.
      continue;
    }
    else if (prefix == "v") {
      detail::ParseComponents(
        &iss, 
        &position_components, 
        &position_components_per_value);
    }
    else if (prefix == "vt") {
      detail::ParseComponents(
        &iss,
        &tex_coord_components,
        &tex_coord_components_per_value);
    }
    else if (prefix == "vn") {
      detail::ParseComponents(
        &iss,
        &normal_components,
        &normal_components_per_value);
    }
    else if (prefix == "f") {
      detail::ParseFaceIndices(
        &iss,
        &position_indices,
        &tex_coord_indices,
        &normal_indices,
        &position_indices_per_face,
        &tex_coord_indices_per_face,
        &normal_indices_per_face);
    }
  }

  return Mesh<ComponentType>(
    position_components,
    position_components_per_value,
    position_indices,
    position_indices_per_face,
    tex_coord_components,
    tex_coord_components_per_value,
    tex_coord_indices,
    tex_coord_indices_per_face,
    normal_components,
    normal_components_per_value,
    normal_indices,
    normal_indices_per_face);
}

} // namespace obj_io
} // namespace thinks

#endif // THINKS_OBJ_IO_H_INCLUDED
