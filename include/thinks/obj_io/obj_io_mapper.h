// Copyright(C) 2018 Tommy Hinks <tommy.hinks@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef THINKS_OBJ_IO_MAPPER_H_INCLUDED
#define THINKS_OBJ_IO_MAPPER_H_INCLUDED

#include <array>
#include <cassert>
#include <functional>
#include <iostream>
#include <sstream>
#include <type_traits>
#include <utility>

namespace thinks {
namespace obj_io {

template<typename T, std::size_t N>
class Position
{
public:
  static_assert(
    std::is_arithmetic<T>::value,
    "position values must be arithmetic");
  static_assert(
    N == 3u || N == 4u,
    "position value count must be 3 or 4");

  Position() noexcept = default;

  Position(const T x, const T y, const T z) noexcept
    : values{ x, y, z }
  {
    static_assert(N == 3u, "position value count must be 3");
  }

  Position(const T x, const T y, const T z, const T w) noexcept
    : values{ x, y, z, w }
  {
    static_assert(N == 4u, "position value count must be 4");
  }

  std::array<T, N> values;
};

template<typename T>
struct IsPosition : std::false_type {};

// Ignore CV issues.
template<typename T, std::size_t N>
struct IsPosition<Position<T, N>> : std::true_type {};


template<typename T, std::size_t N>
class TexCoord
{
public:
  static_assert(
    std::is_floating_point<T>::value,
    "texture coordinate values must be floating point");
  static_assert(
    N == 2u || N == 3u,
    "texture coordinate value count must be 2 or 3");

  TexCoord() noexcept = default;

  TexCoord(const T u, const T v)
    : values{ u, v }
  {
    static_assert(N == 2u, "texture coordinate value count must be 2");
    ThrowIfInvalidRange_();
  }

  TexCoord(const T u, const T v, const T w)
    : values{ u, v, w }
  {
    static_assert(N == 3u, "texture coordinate value count must be 3");
    ThrowIfInvalidRange_();
  }

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
struct IsTexCoord : std::false_type {};

// Ignore CV issues.
template<typename T, std::size_t N>
struct IsTexCoord<TexCoord<T, N>> : std::true_type {};


template<typename T>
class Normal
{
public:
  static_assert(
    std::is_arithmetic<T>::value,
    "normal values must be arithmetic");

  Normal() noexcept = default;

  Normal(const T x, const T y, const T z) noexcept
    : values{ x, y, z }
  {
  }

  std::array<T, 3> values;
};

template<typename T>
struct IsNormal : std::false_type {};

// Ignore CV issues.
template<typename T>
struct IsNormal<Normal<T>> : std::true_type {};


template<typename T>
class Index
{
public:
  static_assert(std::is_integral<T>::value, "index must be integral");

  Index() noexcept = default;

  explicit Index(const T i) 
    : value(i)
  {
    if (!(value >= T(0))) {
      auto oss = std::ostringstream();
      oss << "index must be >= 0, found " << value;
      throw std::invalid_argument(oss.str());
    }
  }

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
    const Index<T> position_index,
    const std::pair<Index<T>, bool>& tex_coord_index,
    const std::pair<Index<T>, bool>& normal_index) noexcept
    : position_index(position_index)
    , tex_coord_index(tex_coord_index)
    , normal_index(normal_index)
  {
  }

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


template<typename IndexT, std::size_t N>
class Face
{
public:
  static_assert(N >= 3u, "face index count must be at least 3");

  Face() noexcept = default;

  // Triangle.
  Face(const IndexT i0, const IndexT i1, const IndexT i2) noexcept
    : values{ i0, i1, i2 }
  {
    static_assert(N == 3u, "triangle index count must be 3");
  }

  // Quad.
  Face(const IndexT i0, const IndexT i1, const IndexT i2, const IndexT i3) noexcept
    : values{ i0, i1, i2, i3 }
  {
    static_assert(N == 4u, "quad index count must be 4");
  }

  std::array<IndexT, N> values;
};

template<typename T>
struct IsFace : std::false_type {};

// Ignore CV issues.
template<typename IndexT, std::size_t N>
struct IsFace<Face<IndexT, N>> : std::true_type {};


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
  auto mapped = pos_mapper();
  while (mapped.second) {
    static_assert(
      IsPosition<decltype(mapped.first)>::value, 
      "incorrect mapped type");
    WriteLine(os, "v", mapped.first, newline);
    mapped = pos_mapper();
  }
}


// Tag dispatch for optional vertex attributes, e.g. tex coords and normals.
struct MapperTag {};
struct NullMapperTag {};

template<typename T>
struct MapperTraits
{
  typedef MapperTag MapperCategory;
};

template<>
struct MapperTraits<std::nullptr_t>
{
  typedef NullMapperTag MapperCategory;
};


template<typename TexMapper>
void WriteTexCoords(
  std::ostream& os,
  TexMapper tex_mapper,
  const std::string& newline,
  MapperTag)
{
  auto mapped = tex_mapper();
  while (mapped.second) {
    static_assert(
      IsTexCoord<decltype(mapped.first)>::value,
      "incorrect mapped type");
    WriteLine(os, "vt", mapped.first, newline);
    mapped = tex_mapper();
  }
}

/// Dummy.
template<typename TexMapper>
void WriteTexCoords(
  std::ostream&,
  TexMapper,
  const std::string&,
  NullMapperTag)
{
}


template<typename NmlMapper>
void WriteNormals(
  std::ostream& os,
  NmlMapper nml_mapper,
  const std::string& newline,
  MapperTag)
{
  auto mapped = nml_mapper();
  while (mapped.second) {
    static_assert(
      IsNormal<decltype(mapped.first)>::value,
      "incorrect mapped type");
    WriteLine(os, "vn", mapped.first, newline);
    mapped = nml_mapper();
  }
}

template<typename NmlMapper>
void WriteNormals(
  std::ostream&,
  NmlMapper,
  const std::string&,
  NullMapperTag)
{
}


template<typename FaceMapper>
void WriteFaces(
  std::ostream& os,
  FaceMapper face_mapper,
  const std::string& newline)
{
  auto mapped = face_mapper();
  while (mapped.second) {
    static_assert(
      IsFace<decltype(mapped.first)>::value,
      "incorrect mapped type");
    WriteLine(os, "f", mapped.first, newline);
    mapped = face_mapper();
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
    typename detail::MapperTraits<TexMapper>::MapperCategory{});
  detail::WriteNormals(os, nml_mapper, newline,
    typename detail::MapperTraits<NmlMapper>::MapperCategory{});
  detail::WriteFaces(os, face_mapper, newline);
}

} // namespace obj_io
} // namespace thinks

#endif // THINKS_OBJ_IO_MAPPER_H_INCLUDED
