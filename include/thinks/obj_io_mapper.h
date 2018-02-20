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

#ifndef THINKS_OBJ_IO_MAPPER_H_INCLUDED
#define THINKS_OBJ_IO_MAPPER_H_INCLUDED

#include <array>
#include <iostream>
#include <sstream>

namespace thinks {
namespace obj_io {
namespace detail {

template<typename ValueType>
void WriteValue(
  std::ostream& os,
  const std::string& line_start,
  const ValueType& value,
  const std::string& newline)
{
  os << line_start;
  for (const auto component : value) {
    os << " " << component;
  }
  os << newline;
}

template<typename PosContainerType, typename PosMapper>
void WritePositions(
  std::ostream& os,
  const PosContainerType& pos_container,
  const PosMapper pos_mapper,
  const std::string& newline)
{
  for (const auto& position : pos_container) {
    const auto value = pos_mapper(position);

    typedef decltype(value) ValueType;
    typedef typename ValueType::value_type ComponentType;
    static_assert(
      std::is_floating_point<ComponentType>::value,
      "position components must be floating point");
    constexpr auto component_count = std::tuple_size<ValueType>::value;
    static_assert(
      component_count == 3 || component_count == 4,
      "position component count must be 3 or 4");

    WriteValue(os, "v", value, newline);
  }
}

template<typename TexContainerType, typename TexMapper>
void WriteTexCoords(
  std::ostream& os,
  const TexContainerType& tex_container,
  const TexMapper tex_mapper,
  const std::string& newline)
{
  for (const auto& tex_coord : tex_container) {
    const auto value = tex_mapper(tex_coord);

    typedef decltype(value) ValueType;
    typedef typename ValueType::value_type ComponentType;
    static_assert(
      std::is_floating_point<ComponentType>::value,
      "texture coordinate components must be floating point");
    constexpr auto component_count = std::tuple_size<ValueType>::value;
    static_assert(
      component_count == 2 || component_count == 3,
      "texture coordinate component count must be 2 or 3");

    for (const auto component : value) {
      if (!(ComponentType(0) <= component && component <= ComponentType(1))) {
        auto oss = std::ostringstream();
        oss << "texture coordinate elements must be in range [0, 1], found " 
          << component;
        throw std::invalid_argument(oss.str());
      }
    }

    WriteValue(os, "vt", value, newline);
  }
}

template<typename NmlContainerType, typename NmlMapper>
void WriteNormals(
  std::ostream& os,
  const NmlContainerType& nml_container,
  const NmlMapper nml_mapper,
  const std::string& newline)
{
  for (const auto& normal : nml_container) {
    const auto value = nml_mapper(normal);

    typedef decltype(value) ValueType;
    typedef typename ValueType::value_type ComponentType;
    static_assert(
      std::is_floating_point<ComponentType>::value,
      "normal components must be floating point");
    constexpr auto component_count = std::tuple_size<ValueType>::value;
    static_assert(
      component_count == 3,
      "normal component count must be 3");

    WriteValue(os, "vn", value, newline);
  }
}

void WriteIndexGroup(
  std::ostream& os,
  const uint32_t pos_index,
  const uint32_t* const tex_index,
  const uint32_t* const nml_index)
{
  os << " ";
  if (tex_index != nullptr && nml_index != nullptr) {
    os << pos_index + 1 << "/" << *tex_index + 1 << "/" << *nml_index + 1;
  }
  else if (tex_index != nullptr) {
    os << pos_index + 1 << "/" << *tex_index + 1;
  }
  else if (nml_index != nullptr) {
    os << pos_index + 1 << "//" << *nml_index + 1;
  }
  else {
    os << pos_index + 1;
  }
}

void WriteFace(
  std::ostream& os,
  const std::vector<uint32_t>& pos_indices,
  const std::vector<uint32_t>& tex_indices,
  const std::vector<uint32_t>& nml_indices,
  const std::string& newline)
{
  const uint32_t* tex_index = nullptr;
  const uint32_t* nml_index = nullptr;

  os << "f";
  for (auto i = uint32_t{ 0 }; i < pos_indices.size(); ++i) {
    if (!tex_indices.empty()) {
      tex_index = &tex_indices[i];
    }
    if (!nml_indices.empty()) {
      nml_index = &nml_indices[i];
    }
    WriteIndexGroup(os, pos_indices[i], tex_index, nml_index);
  }
  os << newline;
}

template<typename IndexContainerType>
void WriteFaces(
  std::ostream& os,
  const IndexContainerType& pos_indices,
  const IndexContainerType& tex_indices,
  const IndexContainerType& nml_indices,
  const uint32_t indices_per_face,
  const std::string& newline)
{
  if (indices_per_face < 3) {
    auto oss = std::ostringstream();
    oss << "indices per face (" << indices_per_face << ") cannot be less than 3";
    throw std::invalid_argument(oss.str());
  }

  if (!(!pos_indices.empty() && pos_indices.size() % indices_per_face == 0)) {
    auto oss = std::ostringstream();
    oss << "position index count (" << pos_indices.size()
      << ") must be multiple of indices per face ("
      << indices_per_face << ")";
    throw std::invalid_argument(oss.str());
  }

  if (!(tex_indices.empty() || pos_indices.size() == tex_indices.size())) {
    auto oss = std::ostringstream();
    oss << "texture coordinate index count (" << tex_indices.size()
      << ") must be same as position index count ("
      << pos_indices.size() << ")";
    throw std::invalid_argument(oss.str());
  }

  if (!(nml_indices.empty() || pos_indices.size() == nml_indices.size())) {
    auto oss = std::ostringstream();
    oss << "normal index count (" << nml_indices.size()
      << ") must be same as position index count ("
      << pos_indices.size() << ")";
    throw std::invalid_argument(oss.str());
  }

  const auto face_count = pos_indices.size() / indices_per_face;
  auto pos_buf = std::vector<uint32_t>{};
  auto tex_buf = std::vector<uint32_t>{};
  auto nml_buf = std::vector<uint32_t>{};

  for (auto i = uint32_t{ 0 }; i < face_count; ++i) {
    for (auto j = uint32_t{ 0 }; j < indices_per_face; ++j) {
      const auto k = i * face_count + j;
      pos_buf.push_back(pos_indices[k]);
      if (!tex_indices.empty()) {
        tex_buf.push_back(tex_indices[k]);
      }
      if (!nml_indices.empty()) {
        nml_buf.push_back(nml_indices[k]);
      }
    }
    WriteFace(os, pos_buf, tex_buf, nml_buf, newline);
    pos_buf.clear();
    tex_buf.clear();
    nml_buf.clear();
  }
}

template<
  typename PosContainerType, typename PosMapper,
  typename TexContainerType, typename TexMapper,
  typename NmlContainerType, typename NmlMapper,
  typename IndexContainerType>
std::ostream& Write(
  std::ostream& os,
  const PosContainerType& pos_values,
  const PosMapper pos_mapper,
  const TexContainerType& tex_values,
  const TexMapper tex_mapper,
  const NmlContainerType& nml_values,
  const NmlMapper nml_mapper,
  const IndexContainerType& pos_indices,
  const IndexContainerType& tex_indices,
  const IndexContainerType& nml_indices,
  const uint32_t indices_per_face,
  const std::string& newline)
{
  // WriteHeader();
  WritePositions(os, pos_values, pos_mapper, newline);
  WriteTexCoords(os, tex_values, tex_mapper, newline);
  WriteNormals(os, nml_values, nml_mapper, newline);
  WriteFaces(os, pos_indices, tex_indices, nml_indices, indices_per_face, newline);
  return os;
}

} // namespace detail


template<
  typename PosContainerType, typename PosMapper,
  typename IndexContainerType>
std::ostream& Write(
  std::ostream& os,
  const PosContainerType& pos_container, 
  const PosMapper pos_mapper,
  const IndexContainerType pos_indices,
  const uint32_t indices_per_face,
  const std::string& newline = "\n")
{
  detail::Write(
    os,
    pos_container, pos_mapper,
    std::array<float, 0>{}, [](float) { return std::array<float, 2>{}; },
    std::array<float, 0>{}, [](float) { return std::array<float, 3>{}; },
    pos_indices,
    IndexContainerType{},
    IndexContainerType{},
    indices_per_face,
    newline);
  return os;
}

} // namespace obj_io
} // namespace thinks

#endif // THINKS_OBJ_IO_MAPPER_H_INCLUDED
