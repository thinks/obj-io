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
#include <exception>
#include <iterator>
#include <ostream>
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
  using namespace std;

  if (denominator == 0) {
    throw runtime_error("denominator cannot be zero");
  }

  return count != 0 && count % denominator == 0;
}


template<typename IterType>
typename std::iterator_traits<IterType>::value_type MinElement(
  const IterType iter_begin,
  const IterType iter_end)
{
  using namespace std;

  if (IsEmpty(iter_begin, iter_end)) {
    throw invalid_argument("cannot find min element of empty range");
  }
  return *min_element(iter_begin, iter_end);
}

template<typename IterType>
typename std::iterator_traits<IterType>::value_type MaxElement(
  const IterType iter_begin,
  const IterType iter_end)
{
  using namespace std;

  if (IsEmpty(iter_begin, iter_end)) {
    throw invalid_argument("cannot find max element of empty range");
  }
  return *max_element(iter_begin, iter_end);
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
    static_assert(std::is_floating_point<ElementType>::value,
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

  void ThrowIfElementsPerVertexIsNotTwoOrThree_()
  {
    if (!(elements_per_vertex == 2 || elements_per_vertex == 3)) {
      auto ss = std::stringstream();
      ss << "tex coord components per value (" 
        << elements_per_vertex << ") must be 2 or 3";
      throw std::runtime_error(ss.str());
    }
  }

  void ThrowIfElementsNotNormalized_()
  {
    typedef typename std::iterator_traits<ElementIter>::value_type ElementType;
    std::for_each(elements_begin, elements_end,
      [](const auto& e) {
        if (!(ElementType(0) <= e && e <= ElementType(1))) {
          auto ss = std::stringstream();
          ss << "tex coord elements must be in range [0, 1], found " << e;
          throw std::runtime_error(ss.str());
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
    const IndexIter indices_begin,
    const IndexIter indices_end,
    const uint32_t indices_per_face)
    : indexed_value_channel_(
      components_begin, components_end, uint32_t{ 3 },
      indices_begin, indices_end, indices_per_face)
  {
    typedef typename std::iterator_traits<ComponentIter>::value_type ComponentType;
    static_assert(std::is_floating_point<ElementType>::value,
      "normal components must be floating point");
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

template<typename ChannelType>
uint32_t ValueCount(const ChannelType& channel)
{
  return Count(channel.components_begin(), channel.components_end()) / 
    channel.components_per_value();
}

template<typename ChannelType>
uint32_t FaceCount(const ChannelType& channel)
{
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

template<
  typename PosIdxType,
  typename TexIdxType,
  typename NmlIdxType>
void WriteFaceIndex(
  std::ostream& os,
  const PosIdxType pos_index,
  const TexIdxType* tex_index,
  const NmlIdxType* nml_index)
{
  os << pos_index + 1; // One-based indices!
  if (tex_index != nullptr) {
    os << "/" << *tex_index + 1; // One-based indices!
  }
  if (nml_index != nullptr) {
    if (tex_index == nullptr) {
      os << "/";
    }
    os << "/" << *nml_index + 1; // One-based indices!
  }
}

template<
  typename PosIdxIter,
  typename TexIdxIter,
  typename NmlIdxIter>
void WriteFace(
  std::ostream& os,
  PosIdxIter& pos_index_iter,
  TexIdxIter* tex_index_iter,
  NmlIdxIter* nml_index_iter,
  const uint32_t indices_per_face,
  const std::string& newline)
{
  // One line per face.
  os << "f: ";
  for (auto i = uint32_t{ 0 }; i < indices_per_face; ++i) {
    WriteFaceIndex(os, 
      *pos_index_iter,
      tex_index_iter != nullptr ? &*(*tex_index_iter) : nullptr,
      nml_index_iter != nullptr ? &*(*nml_index_iter) : nullptr);
    ++pos_index_iter;
    if (tex_index_iter != nullptr) {
      ++(*tex_index_iter);
    }
    if (nml_index_iter != nullptr) {
      ++(*nml_index_iter);
    }
    os << (i != indices_per_face - 1 ? " " : "");
  }
  os << newline;
}

template<
  typename PosCompIter, typename PosIndexIter,
  typename TexCompIter, typename TexIndexIter,
  typename NmlCompIter, typename NmlIndexIter>
void WriteFaces(
  std::ostream& os,
  const PositionChannel<PosCompIter, PosIndexIter>& position_channel,
  const TexCoordChannel<TexCompIter, TexIndexIter>* const tex_coord_channel,
  const NormalChannel<NmlCompIter, NmlIndexIter>* const normal_channel,
  const std::string& newline)
{
  // TODO
  /*
  ThrowIfIndicesPerFaceNotEqualForAllChannels(
  pos_face_channel.indices_per_face,
  tex_index_channel.indices_per_face,
  nml_index_channel.indices_per_face);
  ThrowIfFaceCountNotEqualForAllChannels( // or empty
  Count(pos_face_channel.begin, pos_face_channel.end),
  Count(tex_index_channel.begin, tex_index_channel.end),
  Count(nml_index_channel.begin, nml_index_channel.end));
  // If these two tests hold then the face count is also equal.
  */

  auto pos_index_iter = position_channel.indices_begin();
  TexIndexIter* tex_index_iter = nullptr;
  if (tex_coord_channel != nullptr) {
    tex_index_iter = &tex_coord_channel->indices_begin();
  }
  NmlIndexIter* nml_index_iter = nullptr;
  if (normal_channel != nullptr) {
    nml_index_iter = &normal_channel->indices_begin();
  }
  while (pos_index_iter != position_channel.indices_end())
  {
    WriteFace(os,
      pos_index_iter,
      tex_index_iter,
      nml_index_iter,
      position_channel.indices_per_face(),
      newline);
  }
}

template<
  typename PosCompIter, typename PosIdxIter,
  typename TexCompIter, typename TexIdxIter,
  typename NmlCompIter, typename NmlIdxIter>
std::ostream& Write(
  std::ostream& os,
  const PositionChannel<PosCompIter, PosIdxIter>& position_channel,
  const TexCoordChannel<TexCompIter, TexIdxIter>* const tex_coord_channel,
  const NormalChannel<NmlCompIter, NmlIdxIter>* const normal_channel,
  const std::string& newline)
{
  WriteHeader(os, position_channel, newline);
  WriteValues(os, "v: ", 
    position_channel.components_begin(), 
    position_channel.components_end(),
    position_channel.components_per_value(),
    newline);
  
  if (tex_coord_channel != nullptr) {
    WriteValues(os, "vt: ", 
      tex_coord_channel->components_begin(), 
      tex_coord_channel->components_end(),
      tex_coord_channel->components_per_value(),
      newline);
  }

  if (normal_channel != nullptr) {
    WriteValues(os, "vn: ", 
      normal_channel->components_begin(),
      normal_channel->components_end(),
      normal_channel->components_per_value(),
      newline);
  }

  WriteFaces(os, 
    position_channel, 
    tex_coord_channel, 
    normal_channel, 
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
    PosCompIter, PosIdxIter, 
    PosCompIter, PosIdxIter>(
      os,
      position_channel,
      nullptr,
      nullptr,
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
    PosCompIter, PosIdxIter>(
      os,
      position_channel,
      tex_coord_channel,
      nullptr,
      newline);
}

template<
  typename PosCompIter, typename PosIdxIter,
  typename NmlCompIter, typename NmlIdxIter>
std::ostream& Write(
  std::ostream& os,
  const PositionChannel<PosCompIter, PosIdxIter>& position_channel,
  const NormalChannel<NmlCompIter, NmlIdxIter>& normal_channel,
  const std::string& newline = "\n")
{
  return detail::Write<
    PosCompIter, PosIdxIter,
    PosCompIter, PosIdxIter,
    NmlCompIter, NmlIdxIter>(
      os,
      position_channel,
      nullptr,
      normal_channel,
      newline);
}

template<
  typename PosCompIter, typename PosIdxIter,
  typename TexCompIter, typename TexIdxIter,
  typename NmlCompIter, typename NmlIdxIter>
std::ostream& Write(
  std::ostream& os,
  const PositionChannel<PosCompIter, PosIdxIter>& position_channel,
  const TexCoordChannel<TexCompIter, TexIdxIter>& tex_coord_channel,
  const NormalChannel<NmlCompIter, NmlIdxIter>& normal_channel,
  const std::string& newline = "\n")
{
  return detail::Write<
    PosCompIter, PosIdxIter,
    TexCompIter, TexIdxIter,
    NmlCompIter, NmlIdxIter>(
      os,
      position_channel,
      tex_coord_channel,
      normal_channel,
      newline);
}

} // namespace obj_io
} // namespace thinks

#endif // THINKS_OBJ_IO_H_INCLUDED
