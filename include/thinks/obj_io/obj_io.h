// Copyright(C) 2018 Tommy Hinks <tommy.hinks@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef THINKS_OBJ_IO_OBJ_IO_H_INCLUDED
#define THINKS_OBJ_IO_OBJ_IO_H_INCLUDED

#include <array>
#include <exception>
#include <functional>
#include <iostream>
#include <limits>
#include <sstream>
#include <type_traits>
#include <utility>
#include <vector>


namespace thinks {
namespace obj_io {

template<typename ArithT, std::size_t N>
class Position
{
public:
  static_assert(
    std::is_arithmetic<ArithT>::value,
    "position values must be arithmetic");
  static_assert(
    N == 3 || N == 4,
    "position value count must be 3 or 4");

  constexpr Position() noexcept = default;

  constexpr Position(const ArithT x, const ArithT y, const ArithT z) noexcept
    : values{ x, y, z }
  {
    static_assert(N == 3, "position value count must be 3");
  }

  constexpr Position(
    const ArithT x, const ArithT y, const ArithT z, const ArithT w) noexcept
    : values{ x, y, z, w }
  {
    static_assert(N == 4, "position value count must be 4");
  }

  std::array<ArithT, N> values;
};


template<typename FloatT, std::size_t N>
class TexCoord
{
public:
  static_assert(
    std::is_floating_point<FloatT>::value,
    "texture coordinate values must be floating point");
  static_assert(
    N == 2 || N == 3,
    "texture coordinate value count must be 2 or 3");

  constexpr TexCoord() noexcept = default;

  constexpr TexCoord(const FloatT u, const FloatT v) noexcept
    : values{ u, v }
  {
    static_assert(N == 2, "texture coordinate value count must be 2");
  }

  constexpr TexCoord(const FloatT u, const FloatT v, const FloatT w) noexcept
    : values{ u, v, w }
  {
    static_assert(N == 3, "texture coordinate value count must be 3");
  }

  std::array<FloatT, N> values;
};


template<typename ArithT>
class Normal
{
public:
  static_assert(
    std::is_arithmetic<ArithT>::value,
    "normal values must be arithmetic");

  constexpr Normal() noexcept = default;

  constexpr Normal(const ArithT x, const ArithT y, const ArithT z) noexcept
    : values{ x, y, z }
  {
  }

  std::array<ArithT, 3> values;
};


template<typename IntT>
class Index
{
public:
  static_assert(std::is_integral<IntT>::value, "index must be integral");

  constexpr Index() noexcept = default;

  constexpr explicit Index(const IntT idx) noexcept
    : value(idx)
  {
  }

  IntT value;
};


template<typename IntT>
class IndexGroup
{
public:
  constexpr IndexGroup() noexcept
    : position_index{}
    , tex_coord_index(Index<IntT>{}, false)
    , normal_index(Index<IntT>{}, false)
  {
  }

  constexpr IndexGroup(const IntT pos_idx) noexcept
    : position_index(pos_idx)
    , tex_coord_index(Index<IntT>{}, false)
    , normal_index(Index<IntT>{}, false)
  {
  }

  constexpr IndexGroup(
    const IntT pos_idx,
    const IntT tex_idx,
    const IntT nml_idx) noexcept
    : position_index(pos_idx)
    , tex_coord_index(tex_idx, true)
    , normal_index(nml_idx, true)
  {
  }

  constexpr IndexGroup(
    const Index<IntT> position_index,
    const std::pair<Index<IntT>, bool>& tex_coord_index,
    const std::pair<Index<IntT>, bool>& normal_index) noexcept
    : position_index(position_index)
    , tex_coord_index(tex_coord_index)
    , normal_index(normal_index)
  {
  }

  // Note: Optional would have been nice instead of bool-pairs here.
  Index<IntT> position_index;
  std::pair<Index<IntT>, bool> tex_coord_index;
  std::pair<Index<IntT>, bool> normal_index;
};


template<typename IndexT>
class TriangleFace
{
public:
  constexpr TriangleFace() noexcept = default;

  constexpr TriangleFace(const IndexT i0, const IndexT i1, const IndexT i2) noexcept
    : values{ i0, i1, i2 }
  {
  }

  std::array<IndexT, 3> values;
};

template<typename IndexT>
class QuadFace
{
public:
  constexpr QuadFace() noexcept = default;

  constexpr QuadFace(const IndexT i0, const IndexT i1, const IndexT i2, const IndexT i3) noexcept
    : values{ i0, i1, i2, i3 }
  {
  }

  std::array<IndexT, 4> values;
};

template<typename IndexT>
class PolygonFace
{
public:
  constexpr PolygonFace() noexcept = default;

  std::vector<IndexT> values;
};


template <typename T>
class MapResult
{
public:
  constexpr MapResult(const T& value, const bool is_end) noexcept
    : value(value)
    , is_end(is_end)
  {
  }

  T value;
  bool is_end;
};


template <typename T>
MapResult<T> Map(const T& value) noexcept
{
  return MapResult<T>(value, false);
}

template <typename T>
MapResult<T> End() noexcept
{
  return MapResult<T>(T{}, true);
}


namespace detail {

template<typename T>
struct IsPositionImpl : std::false_type {};

template<typename T, std::size_t N>
struct IsPositionImpl<Position<T, N>> : std::true_type {};

template<typename T>
using IsPosition = IsPositionImpl<typename std::decay<T>::type>;


template<typename T>
struct IsTexCoordImpl : std::false_type {};

template<typename T, std::size_t N>
struct IsTexCoordImpl<TexCoord<T, N>> : std::true_type {};

template<typename T>
using IsTexCoord = IsTexCoordImpl<typename std::decay<T>::type>;


template<typename T>
struct IsNormalImpl : std::false_type {};

template<typename T>
struct IsNormalImpl<Normal<T>> : std::true_type {};

template<typename T>
using IsNormal = IsNormalImpl<typename std::decay<T>::type>;


template<typename T>
struct IsFaceImpl : std::false_type {};

template<typename IndexT>
struct IsFaceImpl<TriangleFace<IndexT>> : std::true_type {};

template<typename IndexT>
struct IsFaceImpl<QuadFace<IndexT>> : std::true_type {}; 

template<typename IndexT>
struct IsFaceImpl<PolygonFace<IndexT>> : std::true_type {};

template<typename T>
using IsFace = IsFaceImpl<typename std::decay<T>::type>;


// Face traits.
struct StaticFaceTag {};
struct DynamicFaceTag {};

template<typename T>
struct FaceTraitsImpl; // Not implemented!

template<typename IndexT>
struct FaceTraitsImpl<TriangleFace<IndexT>>
{
  typedef StaticFaceTag FaceCategory;
};

template<typename IndexT>
struct FaceTraitsImpl<QuadFace<IndexT>>
{
  typedef StaticFaceTag FaceCategory;
};

template<typename IndexT>
struct FaceTraitsImpl<PolygonFace<IndexT>>
{
  typedef DynamicFaceTag FaceCategory;
};

template<typename T>
using FaceTraits = FaceTraitsImpl<typename std::decay<T>::type>;


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


template<typename FloatT, std::size_t N>
void ValidateTexCoord(const TexCoord<FloatT, N>& tex_coord)
{
  for (const auto v : tex_coord.values) {
    if (!(FloatT{0} <= v && v <= FloatT{1})) {
      throw std::runtime_error(
        "texture coordinate values must be in range [0, 1]");
    }
  }
}


template <typename FaceT>
void ValidateFace(const FaceT& face, DynamicFaceTag)
{
  if (!(face.values.size() >= 3)) {
    throw std::runtime_error(
      "face must have at least three indices");
  }
}

template <typename FaceT>
void ValidateFace(const FaceT& face, StaticFaceTag)
{
}


constexpr inline const char* CommentPrefix() { return "#"; }
constexpr inline const char* PositionPrefix() { return "v"; }
constexpr inline const char* FacePrefix() { return "f"; }
constexpr inline const char* TexCoordPrefix() { return "vt"; }
constexpr inline const char* NormalPrefix() { return "vn"; }
constexpr inline const char* IndexGroupSeparator() { return "/"; }


namespace read {

template<typename ArithT, std::size_t N>
struct ParsedValues
{
  typedef typename std::array<ArithT, N> ValueType;

  std::array<ArithT, N> values;
  std::size_t value_count = 0;
};


template<typename T>
bool ParseValue(std::istringstream* const iss, T* const t)
{
  if (*iss >> *t || !iss->eof()) {
    if (iss->fail()) {
      iss->clear(); // Clear status bits.
      auto dummy = std::string{};
      *iss >> dummy;
      auto oss = std::ostringstream{};
      oss << "failed parsing '" << dummy << "'";
      throw std::runtime_error(oss.str());
    }
    return true;
  }
  return false;
}


template<typename ArithT, std::size_t N>
ParsedValues<ArithT, N> ParseValues(std::istringstream* const iss)
{
  typedef typename ParsedValues<ArithT, N>::ValueType::value_type ValueType;

  auto parsed_values = ParsedValues<ArithT, N>{};
  auto value = ValueType{};
  while (ParseValue(iss, &value)) {
    if (parsed_values.value_count >= parsed_values.values.size()) {
      auto oss = std::ostringstream{};
      oss << "expected to parse at most " 
        << parsed_values.values.size() << " values";
      throw std::runtime_error(oss.str());
    }

    parsed_values.values[parsed_values.value_count++] = value;
  }

  return parsed_values;
}


template<typename ArithT, typename AddPosition>
void ParsePosition(
  std::istringstream* const iss,
  AddPosition add_position)
{
  const auto parsed = ParseValues<ArithT, 4>(iss);
  if (parsed.value_count == 3 || parsed.value_count == 4) {
    // Position fourth value defaults to 1.
    add_position(Position<ArithT, 4>(
      parsed.values[0], 
      parsed.values[1], 
      parsed.values[2],
      parsed.value_count == 4 ? parsed.values[3] : ArithT{ 1 }));
  }
  else {
    auto oss = std::ostringstream{};
    oss << "positions must have 3 or 4 values";
    throw std::runtime_error(oss.str());
  }
}


template<typename IntT>
Index<IntT> ParseIndex(const std::string& token)
{
  auto iss = std::istringstream(token);
  auto value = IntT{0};
  ParseValue(&iss, &value);

  if (!(value > 0)) {
    throw std::runtime_error("parsed index must be greater than zero");
  }

  // Convert to zero-based index.
  return Index<IntT>(value - 1u);
}


template<typename IntT>
IndexGroup<IntT> ParseIndexGroup(const std::string& index_group_str)
{
  if (index_group_str.empty()) {
    throw std::runtime_error("empty index group");
  }

  auto index_group = IndexGroup<IntT>{};
  auto pos = std::size_t{0};
  auto sep = std::string::npos;

  // Position index.
  sep = index_group_str.find('/', pos);
  if (sep == std::string::npos) {
    // No delimiter found, try to convert entire string into position index.
    index_group.position_index = ParseIndex<IntT>(index_group_str);

    // Not possible to have other indices without delimiter.
    return index_group;
  }
  else {
    // Found delimiter.
    if (sep == 0) {
      // Index group cannot start with delimiter.
      throw std::runtime_error("missing position index");
    }
    index_group.position_index = 
      ParseIndex<IntT>(index_group_str.substr(pos, sep));
    pos = sep + 1u; // Skip past delimiter.
  }

  // Texture coordinate index.
  sep = index_group_str.find('/', pos);
  if (sep == std::string::npos) {
    index_group.tex_coord_index.first =
      ParseIndex<IntT>(index_group_str.substr(pos));
    index_group.tex_coord_index.second = true;

    // Not possible to have other indices without delimiter.
    return index_group;
  }
  else if (sep - pos > 0) {
    index_group.tex_coord_index.first =
      ParseIndex<IntT>(index_group_str.substr(pos, sep - pos));
    index_group.tex_coord_index.second = true;
    pos = sep + 1u; // Skip past separator.
  }

  // Normal index.
  if (pos < index_group_str.size()) {
    // Convert to zero-based index.
    index_group.normal_index.first =
      ParseIndex<IntT>(index_group_str.substr(pos));
    index_group.normal_index.second = true;
  }

  return index_group;
}


template<typename IntT>
std::vector<IndexGroup<IntT>> ParseIndexGroups(std::istringstream* const iss)
{
  auto index_groups = std::vector<IndexGroup<IntT>>{};
  auto index_group_str = std::string{};
  while (*iss >> index_group_str) {
    const auto index_group = ParseIndexGroup<IntT>(index_group_str);
    index_groups.push_back(index_group);
  }

  return index_groups;
}


template<typename IntT, typename AddFace>
void ParseFace(
  std::istringstream* const iss,
  AddFace add_face)
{
  const auto index_groups = ParseIndexGroups<IntT>(iss);

  if (index_groups.size() < 3) {
    // error
  }
  else if (index_groups.size() == 3) {
    add_face(TriangleFace<IndexGroup<IntT>>(
      index_groups[0],
      index_groups[1],
      index_groups[2]));
  }
  else if (index_groups.size() == 4) {
    add_face(QuadFace<IndexGroup<IntT>>(
      index_groups[0],
      index_groups[1],
      index_groups[2],
      index_groups[3]));
  }
  else {
    // polygon
    // ValidatePolygonFace(...)
  }
}


template<typename FloatT, typename AddTexCoord>
void ParseTexCoord(
  std::istringstream* const iss,
  AddTexCoord add_tex_coord,
  FuncTag)
{
  const auto parsed = ParseValues<FloatT, 3>(iss);
  if (parsed.value_count == 2 || parsed.value_count == 3) {
    // Texture coordinate third value defaults to 1.
    const auto tex = TexCoord<FloatT, 3>(
      parsed.values[0], 
      parsed.values[1],
      parsed.value_count == 3 ? parsed.values[2] : FloatT{ 1 });
    ValidateTexCoord(tex);
    add_tex_coord(tex);
  }
  else {
    auto oss = std::ostringstream{};
    oss << "texture coordinates must have 2 or 3 values";
    throw std::runtime_error(oss.str());
  }
}

/// Dummy.
template<typename FloatT, typename AddTexCoord>
void ParseTexCoord(
  std::istringstream* const,
  AddTexCoord,
  NullFuncTag)
{
}


template<typename ArithT, typename AddNormal>
void ParseNormal(
  std::istringstream* const iss,
  AddNormal add_normal,
  FuncTag)
{
  const auto parsed = ParseValues<ArithT, 3>(iss);
  if (parsed.value_count == 3) {
    add_normal(Normal<ArithT>(
      parsed.values[0], 
      parsed.values[1],
      parsed.values[2]));
  }
  else {
    auto oss = std::ostringstream{};
    oss << "normals must have 3 values";
    throw std::runtime_error(oss.str());
  }
}

/// Dummy.
template<typename ArithT, typename AddNormal>
void ParseNormal(
  std::istringstream* const,
  AddNormal,
  NullFuncTag)
{
}


template<
  typename FloatT,
  typename IntT,
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
    ParseFace<IntT>(&iss, add_face);
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


template<
  typename FloatT,
  typename IndexT,
  typename AddPosition,
  typename AddTexCoord,
  typename AddNormal,
  typename AddFace>
void ParseLines(
  std::istream& is, 
  AddPosition add_position,
  AddFace add_face,
  AddTexCoord add_tex_coord,
  AddNormal add_normal)
{
  auto line = std::string{};
  while (std::getline(is, line)) {
    detail::read::ParseLine<FloatT, IndexT>(
      line, add_position, add_face, add_tex_coord, add_normal);
  }
}

} // namespace read

namespace write {

template<typename T>
std::ostream& operator<<(std::ostream& os, const Index<T>& index)
{
  // Note that the valid range allows increment of one.
  if (!(T{0} <= index.value && index.value < std::numeric_limits<T>::max())) {
    throw std::runtime_error("invalid index");
  }

  // OBJ format uses one-based indexing. 
  os << index.value + 1u;
  return os;
}

template<typename T>
std::ostream& operator<<(std::ostream& os, const IndexGroup<T>& index_group)
{
  os << index_group.position_index;
  if (index_group.tex_coord_index.second &&
    index_group.normal_index.second) {
    os << IndexGroupSeparator() << index_group.tex_coord_index.first
      << IndexGroupSeparator() << index_group.normal_index.first;
  }
  else if (index_group.tex_coord_index.second) {
    os << IndexGroupSeparator() << index_group.tex_coord_index.first;
  }
  else if (index_group.normal_index.second) {
    os << IndexGroupSeparator() << IndexGroupSeparator() 
      << index_group.normal_index.first;
  }
  return os;
}


inline
void WriteHeader(std::ostream& os, const std::string& newline)
{
  os << CommentPrefix() 
    << " Written by https://github.com/thinks/obj-io" << newline;
}


template<
  template<typename> class MappedTypeChecker, 
  typename Mapper, 
  typename Validator>
void WriteMappedLines(
  std::ostream& os,
  const std::string& line_prefix,
  Mapper mapper,
  Validator validator,
  const std::string& newline)
{
  auto map_result = mapper();
  while (!map_result.is_end) {
    static_assert(
      MappedTypeChecker<decltype(map_result.value)>::value,
      "incorrect mapped type");
    validator(map_result.value);

    // Write line.
    os << line_prefix;
    for (const auto value : map_result.value.values) {
      os << " " << value;
    }
    os << newline;

    map_result = mapper();
  }
}


template<typename PosMapper>
void WritePositions(
  std::ostream& os,
  PosMapper pos_mapper,
  const std::string& newline)
{
  WriteMappedLines<IsPosition>(
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
  WriteMappedLines<IsTexCoord>(
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
  WriteMappedLines<IsNormal>(
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
  WriteMappedLines<IsFace>(
    os, 
    FacePrefix(), 
    face_mapper, 
    [](const auto& face) { 
      ValidateFace(face, typename FaceTraits<decltype(face)>::FaceCategory{});
    }, 
    newline);
}

} // namespace write

} // namespace detail


template<
  typename FloatT,
  typename IntT,
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
  detail::read::ParseLines<FloatT, IntT>(
    is, add_position, add_face, add_tex_coord, add_normal);
}


template<
  typename PositionMapper,
  typename FaceMapper,
  typename TexCoordMapper = std::nullptr_t,
  typename NormalMapper = std::nullptr_t>
void Write(
  std::ostream& os,
  PositionMapper pos_mapper,
  FaceMapper face_mapper,
  TexCoordMapper tex_coord_mapper = nullptr,
  NormalMapper normal_mapper = nullptr,
  const std::string& newline = "\n")
{
  detail::write::WriteHeader(os, newline);
  detail::write::WritePositions(os, pos_mapper, newline);
  detail::write::WriteTexCoords(os, tex_coord_mapper, newline,
    typename detail::FuncTraits<TexCoordMapper>::FuncCategory{});
  detail::write::WriteNormals(os, normal_mapper, newline,
    typename detail::FuncTraits<NormalMapper>::FuncCategory{});
  detail::write::WriteFaces(os, face_mapper, newline);
}

} // namespace obj_io
} // namespace thinks

#endif // THINKS_OBJ_IO_OBJ_IO_H_INCLUDED
