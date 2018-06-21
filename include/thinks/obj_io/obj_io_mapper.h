// Copyright(C) 2018 Tommy Hinks <tommy.hinks@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef THINKS_OBJ_IO_MAPPER_H_INCLUDED
#define THINKS_OBJ_IO_MAPPER_H_INCLUDED

#include <array>
#include <exception>
#include <iostream>
#include <sstream>
#include <type_traits>

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


namespace detail {

template<typename T>
struct IsPosition : std::false_type {};

template<typename T, std::size_t N>
struct IsPosition<Position<T, N>> : std::true_type {}; // Ignore CV issues.

template<typename T>
struct IsTexCoord : std::false_type {};

template<typename T, std::size_t N>
struct IsTexCoord<TexCoord<T, N>> : std::true_type {}; // Ignore CV issues.

template<typename T>
struct IsNormal : std::false_type {};

template<typename T>
struct IsNormal<Normal<T>> : std::true_type {}; // Ignore CV issues.

template<typename T>
struct IsFace : std::false_type {};

template<typename IndexT, std::size_t N>
struct IsFace<Face<IndexT, N>> : std::true_type {}; // Ignore CV issues.

namespace read {

template<typename T, std::size_t N>
std::size_t ParseValues(
  std::istringstream* const iss,
  std::array<T, N>* const values)
{
  typedef typename std::array<T, N>::value_type ValueType;

  auto value_count = std::size_t{0};
  auto value = ValueType{};
  while (*iss >> value || !iss->eof()) {
    if (iss->fail()) {
      iss->clear();
      auto dummy = std::string{};
      *iss >> dummy;
      auto oss = std::ostringstream{};
      oss << "failed parsing '" << dummy << "'";
      throw std::runtime_error(oss.str());
    }

    if (value_count >= values->size()) {
      // error
    }

    (*values)[value_count++] = value;
  }

  return value_count;
}


template<typename T, typename AddPosition>
void ParsePosition(
  std::istringstream* const iss,
  AddPosition add_position)
{
  auto pos = Position<T, 4>{};
  const auto value_count = ParseValues<T>(iss, &pos.values);
  if (value_count < 3) {
    auto oss = std::ostringstream{};
    oss << "positions must have at least 3 values";
    throw std::runtime_error(oss.str());
  }
  if (value_count == 3) {
    pos.values[3] = T{1}; // Default w is 1.0.
  }

  add_position(pos);
}


template<typename T, typename AddFace>
void ParseFace(
  std::istringstream* const iss,
  AddFace add_face)
{

}


struct AddTag {};
struct NullAddTag {};

template<typename T>
struct AddTraits
{
  typedef AddTag AddCategory;
};

template<>
struct AddTraits<std::nullptr_t>
{
  typedef NullAddTag AddCategory;
};


template<typename T, typename AddTexCoord>
void ParseTexCoord(
  std::istringstream* const iss,
  AddTexCoord add_tex_coord,
  AddTag)
{
  auto tex = TexCoord<T, 3>{};
  const auto value_count = ParseValues<T>(iss, &tex.values);
  if (value_count < 2) {
    auto oss = std::ostringstream{};
    oss << "texture coordinates must have at least 2 values";
    throw std::runtime_error(oss.str());
  }
  if (value_count == 2) {
    tex.values[2] = T{0}; // Default w is 0.0.
  }

  add_tex_coord(tex);
}

/// Dummy.
template<typename T, typename AddTexCoord>
void ParseTexCoord(
  std::istringstream* const,
  AddTexCoord,
  NullAddTag)
{
}


template<typename T, typename AddNormal>
void ParseNormal(
  std::istringstream* const iss,
  AddNormal add_normal,
  AddTag)
{
  auto nml = Normal<T>{};
  const auto value_count = ParseValues<T>(iss, &nml.values);
  if (value_count != 3) {
    auto oss = std::ostringstream{};
    oss << "normals must have 3 values";
    throw std::runtime_error(oss.str());
  }

  add_normal(nml);
}

/// Dummy.
template<typename T, typename AddNormal>
void ParseNormal(
  std::istringstream* const,
  AddNormal,
  NullAddTag)
{
}


template<
  typename FloatT,
  typename IndexT,
  typename AddPosition,
  typename AddTexCoord,
  typename AddNormal,
  typename AddFace>
void ParseLine(
  const std::string& line,
  AddPosition add_position,
  AddFace add_face,
  AddTexCoord add_tex_coord,
  AddNormal add_normal)
{
  auto iss = std::istringstream(line);

  // Prefix is token before first whitespace.
  auto prefix = std::string{};
  iss >> prefix;

  if (prefix.empty() || prefix == "#") {
    return; // Ignore empty lines and comments.
  }
  else if (prefix == "v") {
    ParsePosition<FloatT>(&iss, add_position);
  }
  else if (prefix == "f") {
    ParseFace<IndexT>(&iss, add_face);
  }
  else if (prefix == "vt") {
    ParseTexCoord<FloatT>(&iss, add_tex_coord,
      typename AddTraits<AddTexCoord>::AddCategory{});
  }
  else if (prefix == "vn") {
    ParseNormal<FloatT>(&iss, add_normal,
      typename AddTraits<AddNormal>::AddCategory{});
  }
  else {
    auto oss = std::ostringstream{};
    oss << "unrecognized line prefix '" << prefix << "'";
    throw std::runtime_error(oss.str());
  }
}

} // namespace read

namespace write {

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

} // namespace write

} // namespace detail


template<
  typename FloatT,
  typename IndexT,
  typename AddPosition,
  typename AddFace,
  typename AddTexCoord = std::nullptr_t,
  typename AddNormal = std::nullptr_t>
void Read(
  std::istream& is,
  AddPosition add_position,
  AddFace add_face,
  AddTexCoord add_tex_coord = nullptr,
  AddNormal add_normal = nullptr)
{
  auto line = std::string{};
  while (std::getline(is, line)) {
    detail::read::ParseLine<FloatT, IndexT>(
      line, add_position, add_face, add_tex_coord, add_normal);
  }
}


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
  detail::write::WriteHeader(os, newline);
  detail::write::WritePositions(os, pos_mapper, newline);
  detail::write::WriteTexCoords(os, tex_mapper, newline,
    typename detail::write::MapperTraits<TexMapper>::MapperCategory{});
  detail::write::WriteNormals(os, nml_mapper, newline,
    typename detail::write::MapperTraits<NmlMapper>::MapperCategory{});
  detail::write::WriteFaces(os, face_mapper, newline);
}

} // namespace obj_io
} // namespace thinks

#endif // THINKS_OBJ_IO_MAPPER_H_INCLUDED
