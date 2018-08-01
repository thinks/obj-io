// Copyright(C) 2018 Tommy Hinks <tommy.hinks@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef THINKS_OBJ_IO_OBJ_IO_H_INCLUDED
#define THINKS_OBJ_IO_OBJ_IO_H_INCLUDED

#include <array>
#include <exception>
#include <iostream>
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
  static_assert(std::is_integral<IntT>::value, "index value must be integral");

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

namespace detail {

template<typename T>
struct IsIndex : std::false_type {};

// Note: Not decaying the type here.
template<typename IntT>
struct IsIndex<Index<IntT>> : std::true_type {};

template<typename IntT>
struct IsIndex<IndexGroup<IntT>> : std::true_type {};

} // namespace detail

template<typename IndexT>
class TriangleFace
{
public:
  static_assert(detail::IsIndex<IndexT>::value, "face values must be of index type");

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
  static_assert(detail::IsIndex<IndexT>::value, "face values must be of index type");

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
  static_assert(detail::IsIndex<IndexT>::value, "face values must be of index type");

  constexpr PolygonFace() noexcept = default;

  template<typename... Args>
  constexpr PolygonFace(Args&&... args) noexcept
    : values(std::forward<Args>(args)...)
  {
  }

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


template<typename ParseT, typename Func>
struct AddFunc
{
  typedef ParseT ParseType;

  Func func;
};

template<typename ParseT, typename Func>
AddFunc<ParseT, typename std::decay<Func>::type> MakeAddFunc(Func&& func)
{
  return { std::forward<Func>(func) };
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
struct NoOpFuncTag {};

template<typename T>
struct FuncTraits
{
  typedef FuncTag FuncCategory;
};

template<>
struct FuncTraits<std::nullptr_t>
{
  typedef NoOpFuncTag FuncCategory;
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

// No need to validate non-polygon faces, the number 
// of indices for these are enforced in the class templates.
template <typename FaceT>
void ValidateFace(const FaceT& face, StaticFaceTag) {}


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
  auto parsed_values = ParsedValues<ArithT, N>{};
  auto value = typename decltype(parsed_values.values)::value_type{};
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


template<typename AddPositionFuncT>
void ParsePosition(
  std::istringstream* const iss,
  AddPositionFuncT add_position,
  std::uint32_t* const count)
{
  typedef typename AddPositionFuncT::ParseType ParseType;

  const auto parsed = ParseValues<ParseType, 4>(iss);
  if (parsed.value_count == 3 || parsed.value_count == 4) {
    // Position fourth value (w) defaults to 1.
    add_position.func(Position<ParseType, 4>(
      parsed.values[0], 
      parsed.values[1], 
      parsed.values[2],
      parsed.value_count == 4 ? parsed.values[3] : ParseType{ 1 }));
    ++(*count);
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
  sep = index_group_str.find(IndexGroupSeparator(), pos);
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
  sep = index_group_str.find(IndexGroupSeparator(), pos);
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


template<typename AddFaceFuncT>
void ParseFace(
  std::istringstream* const iss,
  AddFaceFuncT add_face,
  std::uint32_t* const count)
{
  typedef typename AddFaceFuncT::ParseType ParseType;
  typedef IndexGroup<ParseType> IndexGroupType;

  const auto index_groups = ParseIndexGroups<ParseType>(iss);

  if (index_groups.size() < 3) {
    throw std::runtime_error("face must have at least three indices");
  }
  else if (index_groups.size() == 3) {
    add_face.func(TriangleFace<IndexGroupType>(
      index_groups[0],
      index_groups[1],
      index_groups[2]));
    ++(*count);
  }
  else if (index_groups.size() == 4) {
    add_face.func(QuadFace<IndexGroupType>(
      index_groups[0],
      index_groups[1],
      index_groups[2],
      index_groups[3]));
    ++(*count);
  }
  else {
    add_face.func(PolygonFace<IndexGroupType>(index_groups));
    ++(*count);
    // TODO - ValidatePolygonFace(...)
  }
}


template<typename AddTexCoordFuncT>
void ParseTexCoord(
  std::istringstream* const iss,
  AddTexCoordFuncT add_tex_coord,
  std::uint32_t* const count,
  FuncTag)
{
  typedef typename AddTexCoordFuncT::ParseType ParseType;

  const auto parsed = ParseValues<ParseType, 3>(iss);
  if (parsed.value_count == 2 || parsed.value_count == 3) {
    // Texture coordinate third value defaults to 1.
    const auto tex = TexCoord<ParseType, 3>(
      parsed.values[0], 
      parsed.values[1],
      parsed.value_count == 3 ? parsed.values[2] : ParseType{ 1 });
    ValidateTexCoord(tex);
    add_tex_coord.func(tex);
    ++(*count);
  }
  else {
    auto oss = std::ostringstream{};
    oss << "texture coordinates must have 2 or 3 values";
    throw std::runtime_error(oss.str());
  }
}

/// Dummy.
template<typename AddTexCoordFuncT>
void ParseTexCoord(std::istringstream* const, AddTexCoordFuncT, std::uint32_t* const, NoOpFuncTag) {}


template<typename AddNormalFuncT>
void ParseNormal(
  std::istringstream* const iss,
  AddNormalFuncT add_normal,
  std::uint32_t* const count,
  FuncTag)
{
  typedef typename AddNormalFuncT::ParseType ParseType;

  const auto parsed = ParseValues<ParseType, 3>(iss);
  if (parsed.value_count == 3) {
    add_normal.func(Normal<ParseType>(
      parsed.values[0], 
      parsed.values[1],
      parsed.values[2]));
    ++(*count);
  }
  else {
    auto oss = std::ostringstream{};
    oss << "normals must have 3 values";
    throw std::runtime_error(oss.str());
  }
}

/// Dummy.
template<typename AddNormalFuncT>
void ParseNormal(std::istringstream* const, AddNormalFuncT, std::uint32_t* const, NoOpFuncTag) {}


template<
  typename AddPositionFuncT,
  typename AddTexCoordFuncT,
  typename AddNormalFuncT,
  typename AddFaceFuncT>
void ParseLine(
  const std::string& line,
  AddPositionFuncT add_position,
  AddFaceFuncT add_face,
  AddTexCoordFuncT add_tex_coord,
  AddNormalFuncT add_normal,
  std::uint32_t* const position_count,
  std::uint32_t* const face_count,
  std::uint32_t* const tex_coord_count,
  std::uint32_t* const normal_count)
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
    ParsePosition(&iss, add_position, position_count);
  }
  else if (prefix == FacePrefix()) {
    ParseFace(&iss, add_face, face_count);
  }
  else if (prefix == TexCoordPrefix()) {
    ParseTexCoord(&iss, add_tex_coord, tex_coord_count,
      typename FuncTraits<AddTexCoordFuncT>::FuncCategory{});
  }
  else if (prefix == NormalPrefix()) {
    ParseNormal(&iss, add_normal, normal_count,
      typename FuncTraits<AddNormalFuncT>::FuncCategory{});
  }
  else {
    auto oss = std::ostringstream{};
    oss << "unrecognized line prefix '" << prefix << "'";
    throw std::runtime_error(oss.str());
  }
}


template<
  typename AddPositionFuncT,
  typename AddTexCoordFuncT,
  typename AddNormalFuncT,
  typename AddFaceFuncT>
void ParseLines(
  std::istream& is, 
  AddPositionFuncT add_position,
  AddFaceFuncT add_face,
  AddTexCoordFuncT add_tex_coord,
  AddNormalFuncT add_normal,
  std::uint32_t* const position_count,
  std::uint32_t* const face_count,
  std::uint32_t* const tex_coord_count,
  std::uint32_t* const normal_count)
{
  auto line = std::string{};
  while (std::getline(is, line)) {
    detail::read::ParseLine(
      line, add_position, add_face, add_tex_coord, add_normal,
      position_count, face_count, tex_coord_count, normal_count);
  }
}

} // namespace read

namespace write {

template<typename IntT>
std::ostream& operator<<(std::ostream& os, const Index<IntT>& index)
{
  typedef decltype(index.value) ValueType;

  // Note that the valid range allows increment of one.
  if (!(ValueType{0} <= index.value && 
    index.value < std::numeric_limits<ValueType>::max())) {
    auto oss = std::ostringstream{};
    oss << "invalid index: " << static_cast<std::int64_t>(index.value);
    throw std::runtime_error(oss.str());
  }

  // Input indices are assumed to be zero-based.
  // OBJ format uses one-based indexing. 
  os << index.value + 1u;
  return os;
}

template<typename IntT>
std::ostream& operator<<(std::ostream& os, const IndexGroup<IntT>& index_group)
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
  template<typename> class MappedTypeCheckerT, 
  typename MapperT, 
  typename ValidatorT>
std::uint32_t WriteMappedLines(
  std::ostream& os,
  const std::string& line_prefix,
  MapperT mapper,
  ValidatorT validator,
  const std::string& newline)
{
  auto count = std::uint32_t{ 0 };
  auto map_result = mapper();
  while (!map_result.is_end) {
    static_assert(
      MappedTypeCheckerT<decltype(map_result.value)>::value,
      "incorrect mapped type");

    validator(map_result.value);

    // Write line.
    os << line_prefix;
    for (const auto& element : map_result.value.values) {
      os << " " << element;
    }
    os << newline;
    count++;

    map_result = mapper();
  }
  return count;
}


template<typename MapperT>
std::uint32_t WritePositions(
  std::ostream& os,
  MapperT mapper,
  const std::string& newline)
{
  return WriteMappedLines<IsPosition>(
    os, 
    PositionPrefix(), 
    mapper, 
    [](const auto&) {}, // No validation.  
    newline);
}


template<typename MapperT>
std::uint32_t WriteTexCoords(
  std::ostream& os,
  MapperT mapper,
  const std::string& newline,
  FuncTag)
{
  return WriteMappedLines<IsTexCoord>(
    os,
    TexCoordPrefix(),
    mapper,
    [](const auto& tex) { ValidateTexCoord(tex); },  
    newline);
}

/// Dummy.
template<typename MapperT>
std::uint32_t WriteTexCoords(
  std::ostream&, 
  MapperT, 
  const std::string&, 
  NoOpFuncTag) 
{
  return 0;
}


template<typename MapperT>
std::uint32_t WriteNormals(
  std::ostream& os,
  MapperT mapper,
  const std::string& newline,
  FuncTag)
{
  return WriteMappedLines<IsNormal>(
    os,
    NormalPrefix(),
    mapper,
    [](const auto&) {}, // No validation.
    newline);
}

/// Dummy.
template<typename MapperT>
std::uint32_t WriteNormals(
  std::ostream&, 
  MapperT, 
  const std::string&, 
  NoOpFuncTag) 
{
  return 0;
}


template<typename MapperT>
std::uint32_t WriteFaces(
  std::ostream& os,
  MapperT mapper,
  const std::string& newline)
{
  return WriteMappedLines<IsFace>(
    os, 
    FacePrefix(), 
    mapper, 
    [](const auto& face) { 
      ValidateFace(face, typename FaceTraits<decltype(face)>::FaceCategory{});
    }, 
    newline);
}

} // namespace write

} // namespace detail


struct ReadResult
{
  std::uint32_t position_count = 0;
  std::uint32_t face_count = 0;
  std::uint32_t tex_coord_count = 0;
  std::uint32_t normal_count = 0;
};

template<
  typename AddPositionFuncT,
  typename AddFaceFuncT,
  typename AddTexCoordFuncT = std::nullptr_t,
  typename AddNormalFuncT = std::nullptr_t>
ReadResult Read(
  std::istream& is,
  AddPositionFuncT add_position,
  AddFaceFuncT add_face,
  AddTexCoordFuncT add_tex_coord = nullptr,
  AddNormalFuncT add_normal = nullptr)
{
  auto result = ReadResult{};
  detail::read::ParseLines(
    is, add_position, add_face, add_tex_coord, add_normal,
    &result.position_count, &result.face_count, &result.tex_coord_count, &result.normal_count);
  return result;
}


struct WriteResult
{
  std::uint32_t position_count = 0;
  std::uint32_t face_count = 0;
  std::uint32_t tex_coord_count = 0;
  std::uint32_t normal_count = 0;
};

template<
  typename PositionMapperT,
  typename FaceMapperT,
  typename TexCoordMapperT = std::nullptr_t,
  typename NormalMapperT = std::nullptr_t>
WriteResult Write(
  std::ostream& os,
  PositionMapperT position_mapper,
  FaceMapperT face_mapper,
  TexCoordMapperT tex_coord_mapper = nullptr,
  NormalMapperT normal_mapper = nullptr,
  const std::string& newline = "\n")
{
  auto result = WriteResult{};
  detail::write::WriteHeader(os, newline);
  result.position_count += 
    detail::write::WritePositions(os, position_mapper, newline);
  result.tex_coord_count += detail::write::WriteTexCoords(os, tex_coord_mapper, newline,
    typename detail::FuncTraits<TexCoordMapperT>::FuncCategory{});
  result.normal_count += 
    detail::write::WriteNormals(os, normal_mapper, newline,
    typename detail::FuncTraits<NormalMapperT>::FuncCategory{});
  result.face_count += 
    detail::write::WriteFaces(os, face_mapper, newline);
  return result;
}

} // namespace obj_io
} // namespace thinks

#endif // THINKS_OBJ_IO_OBJ_IO_H_INCLUDED
