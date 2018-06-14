// Copyright(C) 2018 Tommy Hinks <tommy.hinks@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef THINKS_OBJ_IO_MAPPER_H_INCLUDED
#define THINKS_OBJ_IO_MAPPER_H_INCLUDED

#include <array>
#include <cassert>
#include <iostream>
#include <sstream>
#include <type_traits>

namespace thinks {
namespace obj_io {

/// Only constructible for N = 3 and N = 4.
template<typename T, std::size_t N>
class Position
{
public:
  template<typename = typename std::enable_if<N == 3u || N == 4u>::type>
  Position() = default;

  template<typename = typename std::enable_if<N == 3u>::type>
  Position(const T x, const T y, const T z) noexcept
    : values{ x, y, z }
  {}

  template <typename = typename std::enable_if<N == 4u>::type>
  Position(const T x, const T y, const T z, const T w) noexcept
    : values{ x, y, z, w }
  {}

  static_assert(
    std::is_arithmetic<T>::value,
    "position values must be arithmetic");

  std::array<T, N> values;
};


/// Only constructible for N = 2 and N = 3.
template<typename T, std::size_t N>
class TexCoord
{
public:
  template<typename = typename std::enable_if<N == 2u || N == 3u>::type>
  TexCoord() = default;

  template<typename = typename std::enable_if<N == 2u>::type>
  TexCoord(const T u, const T v)
    : values{ u, v }
  {
    ThrowIfInvalidRange_();
  }

  template <typename = typename std::enable_if<N == 3u>::type>
  TexCoord(const T u, const T v, const T w)
    : values{ u, v, w }
  {
    ThrowIfInvalidRange_();
  }

  static_assert(
    std::is_floating_point<T>::value,
    "texture coordinate values must be floating point");

  std::array<T, N> values;

private:
  void ThrowIfInvalidRange_()
  {
    for (const auto value : values) {
      if (!(T(0) <= value && value <= T(1))) {
        auto oss = std::ostringstream();
        oss << "texture coordinate values must be in range [0, 1], found "
          << value;
        throw std::invalid_argument(oss.str());
      }
    }
  }
};


template<typename T>
class Normal
{
public:
  Normal() = default;

  Normal(const T x, const T y, const T z) noexcept
    : values{ x, y, z }
  {
  }

  static_assert(
    std::is_arithmetic<T>::value,
    "normal values must be arithmetic");

  std::array<T, 3> values;
};


template<typename T>
class Index
{
public:
  explicit Index(const T i)
    : value(i)
  {
    if (!(value >= ValueType(0))) {
    auto oss = std::ostringstream();
    oss << "face indices must be >= 0, found " << value;
    throw std::invalid_argument(oss.str());
    }
  }

  static_assert(std::is_integral<T>::value, "index must be integral");

  T value;
};

template<typename T>
std::ostream& operator<<(std::ostream& os, const Index<T>& index)
{
  // OBJ format uses one-based indexing. 
  // Not checking for overflow here!
  os << index.value + 1u;
  return os;
}


template<typename T>
class IndexGroup
{
public:
  IndexGroup(
    const Index<T> pos,
    const std::pair<Index<T>, bool>& tex,
    const std::pair<Index<T>, bool>& nml) noexcept
    : position_index(pos)
    , tex_coord_index(tex)
    , normal_index(nml)
  {}

  static_assert(
    std::is_integral<T>::value,
    "face indices must be integral");

  // Note: Optional would have been nice here.
  Index<T> position_index;
  std::pair<Index<T>, bool> tex_coord_index;
  std::pair<Index<T>, bool> normal_index;
};

template<typename T>
std::ostream& operator<<(std::ostream& os, const IndexGroup<T>& index_group)
{
  os << index_group.position_index;
  if (index_group.tex_coord_index.second &&
    index_group.normal_index.second) {
    os << "/" << index_group.tex_coord_index.first 
      << "/" << index_group.normal_index.first;
  }
  else if (index_group.tex_coord_index.second) {
    os << "/" << index_group.tex_coord_index.first;
  }
  else if (index_group.normal_index.second) {
    os << "//" << index_group.normal_index.first;
  }
  return os;
}


template<typename Index, std::size_t N>
class Face
{
public:
  template<typename = typename std::enable_if<N >= 3u>::type>
  Face() = default;

  // Triangle.
  template<typename = typename std::enable_if<N == 3u>::type>
  Face(const Index& i0, const Index& i1, const Index& i2) noexcept
    : values{ i0, i1, i2 }
  {}

  // Quad.
  template<typename = typename std::enable_if<N == 4u>::type>
  Face(const Index& i0, const Index& i1, const Index& i2, const Index& i3) noexcept
    : values{ i0, i1, i2, i3 }
  {}

  static_assert(N >= 3, "face index count must be at least 3");

  std::array<Index, N> values;
};


namespace detail {

inline
void WriteHeader(std::ostream& os, const std::string& newline)
{
  os << "# Written by https://github.com/thinks/obj-io" << newline;
}


template<typename Attribute>
void WriteLine(
  std::ostream& os,
  const std::string& line_start,
  const Attribute& attribute,
  const std::string& newline)
{
  os << line_start;
  for (const auto value : attribute.values) {
    os << " " << value;
  }
  os << newline;
}


template<typename PosMapper>
void WritePositions(
  std::ostream& os,
  PosMapper pos_mapper,
  const std::string& newline)
{
  auto pos = pos_mapper();
  while (pos.second) {
    WriteLine(os, "v", pos.first, newline);
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
    WriteLine(os, "vt", tex.first, newline);
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
    WriteLine(os, "vn", nml.first, newline);
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
    WriteLine(os, "f", face.first, newline);
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
    typename detail::mapper_traits<TexMapper>::mapper_category{});
  detail::WriteNormals(os, nml_mapper, newline,
    typename detail::mapper_traits<NmlMapper>::mapper_category{});
  detail::WriteFaces(os, face_mapper, newline);
}

} // namespace obj_io
} // namespace thinks

#endif // THINKS_OBJ_IO_MAPPER_H_INCLUDED
