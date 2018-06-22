// Copyright(C) 2018 Tommy Hinks <tommy.hinks@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef THINKS_OBJ_IO_MAPPER_H_INCLUDED
#define THINKS_OBJ_IO_MAPPER_H_INCLUDED

#include <array>
#include <exception>
#include <iostream>
#include <limits>
#include <sstream>
#include <type_traits>
#include <utility>
#include <vector>


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

  Position() noexcept
  {
    std::fill(std::begin(values), std::end(values), 
      std::numeric_limits<T>::quiet_NaN());
  }

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

  TexCoord() 
  {
    std::fill(std::begin(values), std::end(values), 
      std::numeric_limits<T>::quiet_NaN());
  }

  TexCoord(const T u, const T v)
    : values{ u, v }
  {
    static_assert(N == 2u, "texture coordinate value count must be 2");
  }

  TexCoord(const T u, const T v, const T w)
    : values{ u, v, w }
  {
    static_assert(N == 3u, "texture coordinate value count must be 3");
  }

  std::array<T, N> values;

#if 0
  static T CheckedValue_(const T v)
  {
    if (!(T{0} <= v && v <= T{1})) {
      throw std::invalid_argument(
        "texture coordinate values must be in range [0, 1]");
    }
    return v;
  }
#endif
};


template<typename T>
class Normal
{
public:
  static_assert(
    std::is_arithmetic<T>::value,
    "normal values must be arithmetic");

  Normal() noexcept
    : values{ 
        std::numeric_limits<T>::quiet_NaN(), 
        std::numeric_limits<T>::quiet_NaN(), 
        std::numeric_limits<T>::quiet_NaN() }
  {
  }

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

  Index()
    : value(std::numeric_limits<T>::max())
  {
  }

  explicit Index(const T i) 
    : value(i)
  {
  }

  T value;
};

template<typename T>
std::ostream& operator<<(std::ostream& os, const Index<T>& index)
{
  if (!(T{0} <= index.value && index.value < std::numeric_limits<T>::max())) {
    throw std::runtime_error("invalid index");
  }

  // OBJ format uses one-based indexing. 
  os << index.value + 1u;
  return os;
}


template<typename T>
class IndexGroup
{
public:
  IndexGroup() noexcept
    : position_index{}
    , tex_coord_index(Index<T>{}, false)
    , normal_index(Index<T>{}, false)
  {
  }

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

  Face() = default;

  // Triangle.
  Face(const IndexT i0, const IndexT i1, const IndexT i2) noexcept
    : values{ i0, i1, i2 }
  {
    static_assert(N == 3u, "triangle face index count must be 3");
  }

  // Quad.
  Face(const IndexT i0, const IndexT i1, const IndexT i2, const IndexT i3) noexcept
    : values{ i0, i1, i2, i3 }
  {
    static_assert(N == 4u, "quad face index count must be 4");
  }

  std::array<IndexT, N> values;
};


namespace detail {

template<typename T>
struct IsPosition : std::false_type {};

template<typename T, std::size_t N>
struct IsPosition<Position<T, N>> : std::true_type {}; // Ignoring CV issues for now.

template<typename T>
struct IsTexCoord : std::false_type {};

template<typename T, std::size_t N>
struct IsTexCoord<TexCoord<T, N>> : std::true_type {}; // Ignoring CV issues for now.

template<typename T>
struct IsNormal : std::false_type {};

template<typename T>
struct IsNormal<Normal<T>> : std::true_type {}; // Ignoring CV issues for now.

template<typename T>
struct IsFace : std::false_type {};

template<typename IndexT, std::size_t N>
struct IsFace<Face<IndexT, N>> : std::true_type {}; // Ignoring CV issues for now.


// Tag dispatch for optional vertex attributes, e.g. tex coords and normals.
struct FuncTag {};
struct NullFuncTag {};

template<typename T>
struct FuncTraits
{
  typedef FuncTag FuncCategory;
};

template<>
struct FuncTraits<std::nullptr_t>
{
  typedef NullFuncTag FuncCategory;
};


template<typename T, std::size_t N>
void ValidateTexCoord(const TexCoord<T, N>& tex_coord)
{
  for (const auto v : tex_coord.values) {
    if (!(T{0} <= v && v <= T{1})) {
      throw std::runtime_error(
        "texture coordinate values must be in range [0, 1]");
    }
  }
}


constexpr inline const char* CommentPrefix()
{
  return "#";
}

constexpr inline const char* PositionPrefix()
{
  return "v";
}

constexpr inline const char* FacePrefix()
{
  return "f";
}

constexpr inline const char* TexCoordPrefix()
{
  return "vt";
}

constexpr inline const char* NormalPrefix()
{
  return "vn";
}


namespace read {

template<typename T, std::size_t N>
std::pair<std::array<T, N>, std::size_t> ParseValues(std::istringstream* const iss)
{
  typedef typename std::array<T, N>::value_type ValueType;

  auto values = std::array<T, N>{};
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

    if (value_count >= std::tuple_size<std::array<T, N>>::value) {
      auto oss = std::ostringstream{};
      oss << "expected to parse at most " 
        << std::tuple_size<std::array<T, N>>::value << " values";
      throw std::runtime_error(oss.str());
    }

    values[value_count++] = value;
  }

  return std::make_pair(values, value_count);
}


template<typename T, typename AddPosition>
void ParsePosition(
  std::istringstream* const iss,
  AddPosition add_position)
{
  const auto parsed = ParseValues<T, 4>(iss);
  if (parsed.second == 3) {
    add_position(Position<T, 3>(
      parsed.first[0], parsed.first[1], parsed.first[2]));
  }
  else if (parsed.second == 4) {
    add_position(Position<T, 4>(
      parsed.first[0], parsed.first[1], parsed.first[2], parsed.first[3]));
  }
  else {
    auto oss = std::ostringstream{};
    oss << "positions must have 3 or 4 values";
    throw std::runtime_error(oss.str());
  }
}


template<typename IndexT>
IndexT ParseIndex(const std::string& token)
{
  auto iss = std::istringstream(token);
  auto t = IndexT{};
  iss >> t;
  if (iss.fail()) {
    iss.clear(); // Clear status bits.
    auto dummy = std::string{};
    iss >> dummy;
    auto oss = std::ostringstream{};
    oss << "failed parsing index '" << dummy << "'";
    throw std::runtime_error(oss.str());
  }
  return t;
}


template<typename IndexT>
IndexGroup<IndexT> ParseIndexGroup(const std::string& index_group_str)
{
  auto index_group = IndexGroup<IndexT>{};
  index_group.tex_coord_index.second = false;
  index_group.normal_index.second = false;
  auto pos = std::size_t{0};
  auto sep = std::string::npos;
  auto len = std::size_t{0};

  // check empty string!

  // Position index.
  sep = index_group_str.find('/', pos);
  if (sep == std::string::npos) {
    // No delimiter found, try to convert entire string into position index.
    // Convert to zero-based index.
    index_group.position_index.value = ParseIndex<IndexT>(index_group_str) - 1u;

    // Not possible to have other indices without delimiter.
    return index_group;
  }
  else {
    // Found delimiter.
    if (sep == 0) {
      throw std::runtime_error("missing position index");
    }
    // Convert to zero-based index.
    index_group.position_index.value = 
      ParseIndex<IndexT>(index_group_str.substr(pos, sep)) - 1u;
  }
  pos = sep + 1u; // Skip past delimiter.

  // Texture coordinate index.
  sep = index_group_str.find('/', pos);
  if (sep == std::string::npos) {
    // Convert to zero-based index.
    index_group.tex_coord_index.first.value =
      ParseIndex<IndexT>(index_group_str.substr(pos)) - 1u;
    index_group.tex_coord_index.second = true;

    // Not possible to have other indices without delimiter.
    return index_group;
  }
  else if (sep - pos > 0) {
    // Convert to zero-based index.
    index_group.tex_coord_index.first.value =
      ParseIndex<IndexT>(index_group_str.substr(pos, sep - pos)) - 1u;
    index_group.tex_coord_index.second = true;
  }
  pos = sep + 1u; // Skip past separator.

  // Normal index.
  if (pos < index_group_str.size()) {
    // Convert to zero-based index.
    index_group.normal_index.first.value =
      ParseIndex<IndexT>(index_group_str.substr(pos)) - 1u;
    index_group.normal_index.second = true;
  }

  return index_group;
}


template<typename IndexT>
std::vector<IndexGroup<IndexT>> ParseIndexGroups(std::istringstream* const iss)
{
  auto index_groups = std::vector<IndexGroup<IndexT>>{};
  auto index_group_str = std::string{};
  while (*iss >> index_group_str) {
    const auto index_group = ParseIndexGroup<IndexT>(index_group_str);
    index_groups.push_back(index_group);
  }

  return index_groups;
}


template<typename IndexT, typename AddFace>
void ParseFace(
  std::istringstream* const iss,
  AddFace add_face)
{
  const auto index_groups = ParseIndexGroups<IndexT>(iss);

  if (index_groups.size() < 3) {
    // error
  }
  else if (index_groups.size() == 3) {
    add_face(Face<IndexGroup<IndexT>, 3>(
      index_groups[0],
      index_groups[1],
      index_groups[2]));
  }
  else if (index_groups.size() == 4) {
    add_face(Face<IndexGroup<IndexT>, 4>(
      index_groups[0],
      index_groups[1],
      index_groups[2],
      index_groups[3]));
  }
  else {
    // polygon
  }
}


template<typename T, typename AddTexCoord>
void ParseTexCoord(
  std::istringstream* const iss,
  AddTexCoord add_tex_coord,
  FuncTag)
{
  const auto parsed = ParseValues<T, 3>(iss, &values);
  if (parsed.second == 2) {
    ValidateTexCoord(tex);
    add_tex_coord(TexCoord<T, 2>(
      parsed.first[0], parsed.first[1]));
  }
  else if (parsed.second == 3) {
    ValidateTexCoord(tex);
    add_tex_coord(TexCoord<T, 3>(
      parsed.first[0], parsed.first[1], parsed.first[2]));
  }
  else {
    auto oss = std::ostringstream{};
    oss << "texture coordinates must have 2 or 3 values";
    throw std::runtime_error(oss.str());
  }
}

/// Dummy.
template<typename T, typename AddTexCoord>
void ParseTexCoord(
  std::istringstream* const,
  AddTexCoord,
  NullFuncTag)
{
}


template<typename T, typename AddNormal>
void ParseNormal(
  std::istringstream* const iss,
  AddNormal add_normal,
  FuncTag)
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
  NullFuncTag)
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

  // Prefix is first non-whitespace token.
  auto prefix = std::string{};
  iss >> prefix;

  // Parse the rest of the line depending on prefix.
  if (prefix.empty() || prefix == CommentPrefix()) {
    return; // Ignore empty lines and comments.
  }
  else if (prefix == PositionPrefix()) {
    ParsePosition<FloatT>(&iss, add_position);
  }
  else if (prefix == FacePrefix()) {
    ParseFace<IndexT>(&iss, add_face);
  }
  else if (prefix == TexCoordPrefix()) {
    ParseTexCoord<FloatT>(&iss, add_tex_coord,
      typename FuncTraits<AddTexCoord>::FuncCategory{});
  }
  else if (prefix == NormalPrefix()) {
    ParseNormal<FloatT>(&iss, add_normal,
      typename FuncTraits<AddNormal>::FuncCategory{});
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
  os << CommentPrefix() 
    << " Written by https://github.com/thinks/obj-io" << newline;
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


template<
  template<typename> class MappedTypeChecker, 
  typename Mapper, 
  typename Validator>
void WriteMappedLine(
  std::ostream& os,
  const std::string& line_prefix,
  Mapper mapper,
  Validator validator,
  const std::string& newline)
{
  auto mapped = mapper();
  while (mapped.second) {
    static_assert(
      MappedTypeChecker<decltype(mapped.first)>::value,
      "incorrect mapped type");
    validator(mapped.first);
    WriteLine(os, line_prefix, mapped.first, newline);
    mapped = mapper();
  }
}


template<typename PosMapper>
void WritePositions(
  std::ostream& os,
  PosMapper pos_mapper,
  const std::string& newline)
{
  WriteMappedLine<IsPosition>(
    os, 
    PositionPrefix(), 
    pos_mapper, 
    [](const auto&) {}, // No validation.  
    newline);
}


template<typename TexMapper>
void WriteTexCoords(
  std::ostream& os,
  TexMapper tex_mapper,
  const std::string& newline,
  FuncTag)
{
  WriteMappedLine<IsTexCoord>(
    os,
    TexCoordPrefix(),
    tex_mapper,
    [](const auto& tex) { ValidateTexCoord(tex); },  
    newline);
}

/// Dummy.
template<typename TexMapper>
void WriteTexCoords(
  std::ostream&,
  TexMapper,
  const std::string&,
  NullFuncTag)
{
}


template<typename NmlMapper>
void WriteNormals(
  std::ostream& os,
  NmlMapper nml_mapper,
  const std::string& newline,
  FuncTag)
{
  WriteMappedLine<IsNormal>(
    os,
    NormalPrefix(),
    nml_mapper,
    [](const auto&) {}, // No validation.
    newline);
}

/// Dummy.
template<typename NmlMapper>
void WriteNormals(
  std::ostream&,
  NmlMapper,
  const std::string&,
  NullFuncTag)
{
}


template<typename FaceMapper>
void WriteFaces(
  std::ostream& os,
  FaceMapper face_mapper,
  const std::string& newline)
{
  // validate indices???
  WriteMappedLine<IsFace>(
    os, 
    FacePrefix(), 
    face_mapper, 
    [](const auto&) {}, // No validation. 
    newline);
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
    typename detail::FuncTraits<TexMapper>::FuncCategory{});
  detail::write::WriteNormals(os, nml_mapper, newline,
    typename detail::FuncTraits<NmlMapper>::FuncCategory{});
  detail::write::WriteFaces(os, face_mapper, newline);
}

} // namespace obj_io
} // namespace thinks

#endif // THINKS_OBJ_IO_MAPPER_H_INCLUDED
