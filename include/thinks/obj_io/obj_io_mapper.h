// Copyright(C) 2018 Tommy Hinks <tommy.hinks@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top - level directory of this distribution.

#ifndef THINKS_OBJ_IO_MAPPER_H_INCLUDED
#define THINKS_OBJ_IO_MAPPER_H_INCLUDED

#include <array>
#include <cassert>
#include <iostream>
#include <sstream>
#include <type_traits>

namespace thinks {
namespace obj_io {

/// Minimalistic generalization of string_view.
template<typename T>
class BufferView
{
public:
  typedef T ValueType;
  typedef std::size_t SizeType;
  typedef const T& ConstReference;
  typedef const T* ConstIterator;

  /// Create empty buffer view.
  constexpr 
  BufferView() noexcept
    : data_(nullptr)
    , size_(0)
  {}

  constexpr
  BufferView(const T* data, const size_t size) noexcept
    : data_(data)
    , size_(size)
  {}

  constexpr ConstIterator begin() const noexcept 
  { 
    return data_;
  }

  constexpr ConstIterator end() const noexcept 
  { 
    return data_ + size_; 
  }

  constexpr SizeType size() const noexcept 
  { 
    return size_; 
  }

  constexpr bool empty() const noexcept 
  { 
    return data_ == nullptr || size_ == 0; 
  }

  /// No bounds checking, similar to std::string_view.
  constexpr ConstReference operator[](SizeType pos) const noexcept
  { 
    return data_[pos];
  }

private:
  const ValueType* data_;
  SizeType size_;
};

/// Named constructor to help with template type deduction.
template<typename T>
BufferView<T> MakeBufferView(const T* data, const size_t size)
{
  return BufferView<T>(data, size);
}


namespace detail {

inline
void WriteHeader(std::ostream& os, const std::string& newline)
{
  os << "# Written by https://github.com/thinks/obj-io" << newline;
}


template<typename ValueType>
void WriteValue(
  std::ostream& os,
  const std::string& line_start,
  const ValueType& value,
  const std::string& newline)
{
  typedef typename ValueType::value_type ComponentType;
  static_assert(
    std::is_arithmetic<ComponentType>::value,
    "value components must be arithmetic");
  static_assert(
    std::tuple_size<ValueType>::value > 0,
    "value component count must be greater than zero");

  os << line_start;
  for (const auto component : value) {
    os << " " << component;
  }
  os << newline;
}

template<typename ValueType>
void WritePosition(
  std::ostream& os,
  const ValueType& value,
  const std::string& newline)
{
  typedef typename ValueType::value_type ComponentType;
  static_assert(
    std::is_arithmetic<ComponentType>::value,
    "position components must be arithmetic");
  static_assert(
    std::tuple_size<ValueType>::value == 3 || 
    std::tuple_size<ValueType>::value == 4,
    "position component count must be 3 or 4");

  WriteValue(os, "v", value, newline);
}

template<typename ValueType>
void WriteTexCoord(
  std::ostream& os,
  const ValueType& value,
  const std::string& newline)
{
  typedef typename ValueType::value_type ComponentType;
  static_assert(
    std::is_floating_point<ComponentType>::value,
    "texture coordinate components must be floating point");
  static_assert(
    std::tuple_size<ValueType>::value == 2 ||
    std::tuple_size<ValueType>::value == 3,
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

template<typename ValueType>
void WriteNormal(
  std::ostream& os,
  const ValueType& value,
  const std::string& newline)
{
  typedef typename ValueType::value_type ComponentType;
  static_assert(
    std::is_arithmetic<ComponentType>::value,
    "normal components must be arithmetic");
  static_assert(
    std::tuple_size<ValueType>::value == 3,
    "normal component count must be 3");

  WriteValue(os, "vn", value, newline);
}

template<typename ValueType>
void WriteFace(
  std::ostream& os,
  const ValueType& value,
  const std::string& newline)
{
  typedef typename ValueType::value_type ComponentType;
  static_assert(
    std::is_integral<ComponentType>::value,
    "face indices must be integral");
  static_assert(
    std::tuple_size<ValueType>::value >= 3,
    "face index count must be at least 3");

  auto incremented_value = value;
  for (auto& component : incremented_value) {
    if (!(component >= ComponentType(0))) {
      auto oss = std::ostringstream();
      oss << "face indices must be >= 0, found " << component;
      throw std::invalid_argument(oss.str());
    }

    // OBJ format uses one-based indexing. 
    component += 1u;
  }

  WriteValue(os, "f", incremented_value, newline);
}


template<typename PosMapper>
void WritePositions(
  std::ostream& os,
  PosMapper pos_mapper,
  const std::string& newline)
{
  auto pos = pos_mapper();
  while (pos.second) {
    WritePosition(os, pos.first, newline);
    pos = pos_mapper();
  }
}


// Tag dispatch for optional vertex attributes, e.g. tex coords and normals.
struct mapper_tag {};
struct null_mapper_tag {};

template<typename T>
struct mapper_traits
{
  typedef mapper_tag mapper_category;
};

template<>
struct mapper_traits<std::nullptr_t>
{
  typedef null_mapper_tag mapper_category;
};


template<typename TexMapper>
void WriteTexCoords(
  std::ostream& os,
  TexMapper tex_mapper,
  const std::string& newline,
  mapper_tag)
{
  auto tex = tex_mapper();
  while (tex.second) {
    WriteTexCoord(os, tex.first, newline);
    tex = tex_mapper();
  }
}

/// Dummy.
template<typename TexMapper>
void WriteTexCoords(
  std::ostream&,
  TexMapper,
  const std::string&,
  null_mapper_tag)
{
}


template<typename NmlMapper>
void WriteNormals(
  std::ostream& os,
  NmlMapper nml_mapper,
  const std::string& newline,
  mapper_tag)
{
  auto nml = nml_mapper();
  while (nml.second) {
    WriteNormal(os, nml.first, newline);
    nml = nml_mapper();
  }
}

/// Dummy.
template<typename NmlMapper>
void WriteNormals(
  std::ostream&,
  NmlMapper,
  const std::string&,
  null_mapper_tag)
{
}


template<typename FaceMapper>
void WriteFaces(
  std::ostream& os,
  FaceMapper face_mapper,
  const std::string& newline)
{
  auto face = face_mapper();
  while (face.second) {
    WriteFace(os, face.first, newline);
    face = face_mapper();
  }
}

} // namespace detail


template<
  typename PosMapper,
  typename FaceMapper,
  typename TexMapper = std::nullptr_t,
  typename NmlMapper = std::nullptr_t>
void Write(
  std::ostream& os,
  PosMapper pos_mapper,
  FaceMapper face_mapper,
  TexMapper tex_mapper = nullptr,
  NmlMapper nml_mapper = nullptr,
  const std::string& newline = "\n")
{
  detail::WriteHeader(os, newline);
  detail::WritePositions(os, pos_mapper, newline);
  detail::WriteTexCoords(os, tex_mapper, newline,
    typename detail::mapper_traits<TexMapper>::mapper_category());
  detail::WriteNormals(os, nml_mapper, newline,
    typename detail::mapper_traits<NmlMapper>::mapper_category());
  detail::WriteFaces(os, face_mapper, newline);
}

} // namespace obj_io
} // namespace thinks

#endif // THINKS_OBJ_IO_MAPPER_H_INCLUDED







#if 0
void WriteIndexGroup(
  std::ostream& os,
  const uint32_t pos_index,
  const uint32_t* const tex_index,
  const uint32_t* const nml_index)
{
  os << " " << pos_index + 1;
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

template<typename IndexType>
void WriteFaces(
  std::ostream& os,
  const BufferView<IndexType> pos_index_buf,
  const BufferView<IndexType> tex_index_buf,
  const BufferView<IndexType> nml_index_buf,
  const uint32_t indices_per_face,
  const std::string& newline)
{
  static_assert(
    std::is_integral<IndexType>::value,
    "indices must be integral");

  if (indices_per_face < 3) {
    auto oss = std::ostringstream();
    oss << "indices per face (" << indices_per_face << ") cannot be less than 3";
    throw std::invalid_argument(oss.str());
  }

  if (!(!pos_index_buf.empty() && pos_index_buf.size() % indices_per_face == 0)) {
    auto oss = std::ostringstream();
    oss << "position index count (" << pos_index_buf.size()
      << ") must be multiple of indices per face ("
      << indices_per_face << ")";
    throw std::invalid_argument(oss.str());
  }

  if (!(tex_index_buf.empty() || pos_index_buf.size() == tex_index_buf.size())) {
    auto oss = std::ostringstream();
    oss << "texture coordinate index count (" << tex_index_buf.size()
      << ") must be same as position index count ("
      << pos_index_buf.size() << ")";
    throw std::invalid_argument(oss.str());
  }

  if (!(nml_index_buf.empty() || pos_index_buf.size() == nml_index_buf.size())) {
    auto oss = std::ostringstream();
    oss << "normal index count (" << nml_index_buf.size()
      << ") must be same as position index count ("
      << pos_index_buf.size() << ")";
    throw std::invalid_argument(oss.str());
  }

  const auto face_count = pos_index_buf.size() / indices_per_face;
  auto pos_buf = std::vector<uint32_t>{};
  auto tex_buf = std::vector<uint32_t>{};
  auto nml_buf = std::vector<uint32_t>{};

  for (auto i = uint32_t{ 0 }; i < face_count; ++i) {
    for (auto j = uint32_t{ 0 }; j < indices_per_face; ++j) {
      const auto k = i * face_count + j;
      pos_buf.push_back(pos_index_buf[k]);
      if (!tex_index_buf.empty()) {
        tex_buf.push_back(tex_index_buf[k]);
      }
      if (!nml_index_buf.empty()) {
        nml_buf.push_back(nml_index_buf[k]);
      }
    }
    WriteFace(os, pos_buf, tex_buf, nml_buf, newline);
    pos_buf.clear();
    tex_buf.clear();
    nml_buf.clear();
  }
}
#endif
