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

  return count != 0 && count % denominator == 0; // TODO - count != 0??
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


#if 0
inline
void ThrowIfComponentCountNotMultipleOfComponentsPerValue(
  const uint32_t component_count,
  const uint32_t components_per_value)
{
  if (!IsMultipleOf(component_count, components_per_value)) {
    auto ss = std::stringstream();
    ss << "component count (" << component_count << ") is not a multiple of "
      << "components per value (" << components_per_value << ")";
    throw std::invalid_argument(ss.str());
  }
}
#endif




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
      ss << "channel element count (" << element_count
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
      "value channel components must be arithmetic");
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
      "face channel components must be integral");

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
      ss << "face channel indices per face (" << indices_per_face()
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
  return PositionChannel<
    typename std::vector<ComponentType>::const_iterator,
    typename std::vector<IndexType>::const_iterator>(
      std::begin(components), std::end(components), components_per_value,
      std::begin(indices), std::end(indices), indices_per_face);
}


template<typename ElementIter, typename IndexIter>
class TexCoords
{
public:
  TexCoords(
    const ElementIter elements_begin,
    const ElementIter elements_end,
    const IndexIter indices_begin,
    const IndexIter indices_end,
    const uint32_t elements_per_vertex,
    const uint32_t indices_per_face)
    : elements_begin(elements_begin)
    , elements_end(elements_end)
    , indices_begin(indices_begin)
    , indices_end(indices_end)
    , elements_per_vertex(elements_per_vertex)
    , indices_per_face(indices_per_face)
  {
    using namespace std;

    typedef typename iterator_traits<ElementIter>::value_type ElementType;
    typedef typename iterator_traits<IndexIter>::value_type IndexType;

    static_assert(is_floating_point<ElementType>::value,
      "tex coord elements must be floating point");
    static_assert(is_integral<IndexType>::value,
      "tex coord index elements must be integral");

    // Elements.
    ThrowIfElementsPerVertexIsNotTwoOrThree_(elements_per_vertex);
    ThrowIfElementCountNotMultipleOfElementsPerVertex_(
      elements_begin, elements_end, elements_per_vertex);
    ThrowIfElementsNotNormalized_(
      elements_begin, elements_end);

    // Indices.
    ThrowIfIndicesPerFaceIsLessThanThree_(indices_per_face);
    ThrowIfIndexCountNotMultipleOfIndicesPerFace_(
      indices_begin, indices_end, indices_per_face);
    ThrowIfInvalidIndexRange_(
      indices_begin, indices_end,
      elements_begin, elements_end, elements_per_vertex);
  }

  const ElementIter elements_begin;
  const ElementIter elements_end;
  const IndexIter indices_begin;
  const IndexIter indices_end;
  const uint32_t elements_per_vertex;
  const uint32_t indices_per_face;

private:
  static void ThrowIfElementsPerVertexIsNotTwoOrThree_(
    const uint32_t elements_per_vertex)
  {
    using namespace std;
    if (!(elements_per_vertex == 2 || elements_per_vertex == 3)) {
      auto ss = stringstream();
      ss << "tex coord elements per vertex must be 2 or 3, was "
        << elements_per_vertex;
      throw runtime_error(ss.str());
    }
  }

  static void ThrowIfElementCountNotMultipleOfElementsPerVertex_(
    const ElementIter elements_begin,
    const ElementIter elements_end,
    const uint32_t elements_per_vertex)
  {
    using namespace std;
    const auto element_count = distance(elements_begin, elements_end);
    if (!(element_count % elements_per_vertex == 0)) {
      auto ss = stringstream();
      ss << "tex coord element count (" << element_count
        << ") must be a multiple of " << elements_per_vertex;
      throw runtime_error(ss.str());
    }
  }

  static void ThrowIfElementsNotNormalized_(
    const ElementIter elements_begin,
    const ElementIter elements_end)
  {
    using namespace std;
    typedef typename iterator_traits<ElementIter>::value_type ElementType;
    for_each(elements_begin, elements_end,
      [](const auto& e) {
        if (!(ElementType(0) <= e && e <= ElementType(1))) {
          auto ss = stringstream();
          ss << "tex coord elements must be in range [0, 1], found " << e;
          throw runtime_error(ss.str());
        }
      });
  }

  static void ThrowIfIndicesPerFaceIsLessThanThree_(
    const uint32_t indices_per_face)
  {
    using namespace std;
    if (!(indices_per_face >= 3)) {
      auto ss = stringstream();
      ss << "tex coord indices per face must be "
        << "greater than or equal to 3, was "
        << indices_per_face;
      throw runtime_error(ss.str());
    }
  }

  static void ThrowIfIndexCountNotMultipleOfIndicesPerFace_(
    const IndexIter indices_begin,
    const IndexIter indices_end,
    const uint32_t indices_per_face)
  {
    using namespace std;
    const auto index_count = distance(indices_begin, indices_end);
    if (index_count % indices_per_face != 0) {
      auto ss = stringstream();
      ss << "tex coord index count (" << index_count
        << ") must be a multiple of " << indices_per_face;
      throw runtime_error(ss.str());
    }
  }

  static void ThrowIfInvalidIndexRange_(
    const IndexIter indices_begin,
    const IndexIter indices_end,
    const ElementIter elements_begin,
    const ElementIter elements_end,
    const uint32_t elements_per_vertex)
  {
    using namespace std;

    if (detail::IsEmpty(indices_begin, indices_end)) {
      return;
    }

    const auto min_index = *min_element(indices_begin, indices_end);
    const auto max_index = *max_element(indices_begin, indices_end);
    if (min_index != 0) {
      auto ss = stringstream();
      ss << "min tex coord index must be zero, was " << min_index;
      throw runtime_error(ss.str());
    }

    const auto vertex_count =
      distance(elements_begin, elements_end) / elements_per_vertex;
    if (max_index >= vertex_count) {
      auto ss = stringstream();
      ss << "max tex coord index must be less than vertex count ("
        << vertex_count << "), was " << max_index;
      throw runtime_error(ss.str());
    }
  }
};

/// Named constructor to help with template type deduction.
template<typename ElementIter, typename IndexIter>
TexCoords<ElementIter, IndexIter> make_tex_coords(
  const ElementIter elements_begin,
  const ElementIter elements_end,
  const IndexIter indices_begin,
  const IndexIter indices_end,
  const uint32_t elements_per_vertex,
  const uint32_t indices_per_face)
{
  return TexCoords<ElementIter, IndexIter>(
    elements_begin, elements_end,
    indices_begin, indices_end,
    elements_per_vertex,
    indices_per_face);
}


template<typename ElementIter, typename IndexIter>
class Normals
{
public:
  Normals(
    const ElementIter elements_begin,
    const ElementIter elements_end,
    const IndexIter indices_begin,
    const IndexIter indices_end,
    const uint32_t indices_per_face)
    : elements_begin(elements_begin)
    , elements_end(elements_end)
    , indices_begin(indices_begin)
    , indices_end(indices_end)
    , components_per_vertex(3)
    , indices_per_face(indices_per_face)
  {
    using namespace std;

    typedef typename iterator_traits<ElementIter>::value_type ElementType;
    typedef typename iterator_traits<IndexIter>::value_type IndexType;

    static_assert(is_floating_point<ElementType>::value,
      "normal elements must be floating point");
    static_assert(is_integral<IndexType>::value,
      "normal index elements must be integral");

    // Elements, can be empty.
    ThrowIfElementsPerVertexIsNotThree_(components_per_vertex);
    ThrowIfElementCountNotMultipleOfElementsPerVertex_(
      elements_begin, elements_end, components_per_vertex);

    // Indices, can be empty.
    ThrowIfIndicesPerFaceIsLessThanThree_(indices_per_face);
    ThrowIfIndexCountNotMultipleOfIndicesPerFace_(
      indices_begin, indices_end, indices_per_face);
    ThrowIfInvalidIndexRange_(
      indices_begin, indices_end,
      elements_begin, elements_end, 
      components_per_vertex);
  }

  const ElementIter elements_begin;
  const ElementIter elements_end;
  const IndexIter indices_begin;
  const IndexIter indices_end;
  const uint32_t components_per_vertex;
  const uint32_t indices_per_face;

private:
  static void ThrowIfElementsPerVertexIsNotThree_(
    const uint32_t elements_per_vertex)
  {
    using namespace std;
    if (!(elements_per_vertex == 3)) {
      auto ss = stringstream();
      ss << "normal elements per vertex must be 3, was "
        << elements_per_vertex;
      throw runtime_error(ss.str());
    }
  }

  static void ThrowIfElementCountNotMultipleOfElementsPerVertex_(
    const ElementIter elements_begin,
    const ElementIter elements_end,
    const uint32_t components_per_vertex)
  {
    using namespace std;
    const auto element_count = distance(elements_begin, elements_end);
    if (element_count % components_per_vertex != 0) {
      auto ss = stringstream();
      ss << "normal component count (" << element_count
        << ") must be a multiple of " << components_per_vertex;
      throw runtime_error(ss.str());
    }
  }

  static void ThrowIfIndicesPerFaceIsLessThanThree_(
    const uint32_t indices_per_face)
  {
    using namespace std;
    if (!(indices_per_face >= 3)) {
      auto ss = stringstream();
      ss << "normal indices per face must be "
        << "greater than or equal to 3, was "
        << indices_per_face;
      throw runtime_error(ss.str());
    }
  }

  static void ThrowIfIndexCountNotMultipleOfIndicesPerFace_(
    const IndexIter indices_begin,
    const IndexIter indices_end,
    const uint32_t indices_per_face)
  {
    using namespace std;
    const auto index_count = distance(indices_begin, indices_end);
    if (index_count % indices_per_face != 0) {
      auto ss = stringstream();
      ss << "normal index count (" << index_count
        << ") must be a multiple of " << indices_per_face;
      throw runtime_error(ss.str());
    }
  }

  static void ThrowIfInvalidIndexRange_(
    const IndexIter indices_begin,
    const IndexIter indices_end,
    const ElementIter elements_begin,
    const ElementIter elements_end,
    const uint32_t components_per_vertex)
  {
    using namespace std;

    if (detail::IsEmpty(indices_begin, indices_end)) {
      return;
    }

    const auto min_index = *min_element(indices_begin, indices_end);
    const auto max_index = *max_element(indices_begin, indices_end);
    if (min_index != 0) {
      auto ss = stringstream();
      ss << "min normal index must be zero, was " << min_index;
      throw runtime_error(ss.str());
    }

    const auto vertex_count =
      distance(elements_begin, elements_end) / components_per_vertex;
    if (max_index >= vertex_count) {
      auto ss = stringstream();
      ss << "max normal index must be less than vertex count ("
        << vertex_count << "), was " << max_index;
      throw runtime_error(ss.str());
    }
  }
};

/// Named constructor to help with template type deduction.
template<typename ElementIter, typename IndexIter>
Normals<ElementIter, IndexIter> make_normals(
  const ElementIter elements_begin,
  const ElementIter elements_end,
  const IndexIter indices_begin,
  const IndexIter indices_end,
  const uint32_t indices_per_face)
{
  return Normals<ElementIter, IndexIter>(
    elements_begin, elements_end,
    indices_begin, indices_end,
    indices_per_face);
}

namespace detail {

template<typename ComponentIter, typename IndexIter>
void WriteHeader(
  std::ostream& os,
  const PositionChannel<ComponentIter, IndexIter>& position_channel,
  const std::string& newline)
{
  const auto value_count = ValueCount(position_channel.value_channel);
  const auto face_count = FaceCount(position_channel.face_channel);
  os << "# Generated by https://github.com/thinks/obj-io" << newline
    << "# Vertex count: " << vertex_count << newline
    << "# Face count: " << face_count << newline;
}

template<typename ComponentIter>
void WriteValueChannel(
  std::ostream& os,
  const std::string& line_start,
  const ValueChannel<ComponentIter>& value_channel,
  const std::string& newline)
{
  ThrowIfComponentCountNotMultipleOfComponentsPerValue(
    Count(vertex_channel.begin, vertex_channel.end),
    vertex_channel.components_per_element);

  // One line per value.
  auto component_iter = vertex_channel.begin;
  while (component_iter != vertex_channel.end) {
    os << line_start;
    for (auto i = uint32_t{ 0 }; i < value_channel.components_per_value; ++i) {
      os << *component_iter++ <<
        (i != value_channel.components_per_value - 1 ? " " : "");
    }
    os << newline;
  }
}

template<typename PosIndexIter, typename TexIndexIter, typename NmlIndexIter>
void WriteFaceChannels(
  std::ostream& os,
  const FaceChannel<PosIndexIter>& pos_face_channel,
  const FaceChannel<TexIndexIter>& tex_face_channel,
  const FaceChannel<NmlIndexIter>& nml_face_channel,
  const std::string& newline)
{
  ThrowIfIndicesPerFaceNotEqualForAllIndexChannels(
    pos_face_channel.indices_per_face,
    tex_index_channel.indices_per_face,
    nml_index_channel.indices_per_face);
  ThrowIfIndexCountNotEqualForAllIndexChannels( // or empty
    Count(pos_face_channel.begin, pos_face_channel.end),
    Count(tex_index_channel.begin, tex_index_channel.end),
    Count(nml_index_channel.begin, nml_index_channel.end));
  // If these two tests hold then the face count is also equal.

  const auto tex_empty = IsEmpty(tex_index_channel.begin, tex_index_channel.end);
  const auto nml_empty = IsEmpty(nml_index_channel.begin, nml_index_channel.end);
  auto pos_index_iter = pos_face_channel.begin;
  auto tex_index_iter = tex_index_channel.begin;
  auto nml_index_iter = nml_index_channel.begin;
  while (pos_index_iter != pos_index_channel.end &&
    tex_index_iter != tex_index_channel.end &&
    nml_index_iter != nml_index_channel.end) 
  {
    os << "f: ";
    for (auto i = uint32_t{ 0 }; i < pos_face_channel.indices_per_face; ++i) {
      os << *pos_index_iter + 1; // One-based indices!
      ++pos_index_iter;
      if (!tex_empty) {
        os << "/" << *tex_index_iter + 1; // One-based indices!
        ++tex_index_iter;
      }
      if (!nml_empty) {
        if (tex_empty) {
          os << "/";
        }
        os << "/" << *nml_index_iter + 1; // One-based indices!
        ++nml_index_iter;
      }
      os << (i != pos_face_channel.indices_per_face.indices_per_face - 1 ? " " : "");
    }
    os << newline;
  }
}

template<
  typename PosIter, typename PosIdxIter,
  typename TexIter, typename TexIdxIter,
  typename NmlIter, typename NmlIdxIter>
  std::ostream& Write(
    std::ostream& os,
    const PositionChannel<PosIter, PosIdxIter>& positions,
    const TexCoords<TexIter, TexIdxIter>* const tex_coords,
    const Normals<NmlIter, NmlIdxIter>* const normals,
    const std::string& newline)
{
  WriteHeader(os, positions.vertex_channel, positions.index_channel, newline);
  WriteVertexChannel(os, "v: ", positions.vertex_channel, newline);
  WriteVertexChannel(os, "vt: ", tex_coords.vertex_channel, newline);
  WriteVertexChannel(os, "vn: ", normals.vertex_channel, newline);
  WriteFaces(
    os,
    positions.index_channel,
    tex_coords.index_channel,
    normals.index_channel,
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
  return detail::Write(
    os,
    &position_channel,
    nullptr,
    nullptr,
    newline);
}

} // namespace obj_io
} // namespace thinks

#endif // THINKS_OBJ_IO_H_INCLUDED
