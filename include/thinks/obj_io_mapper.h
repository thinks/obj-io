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

#include <iostream>

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
    os << component << " ";
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
    WriteValue(os, "v ", value, newline);
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

    WriteValue(os, "vt ", value, newline);
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

    WriteValue(os, "vn ", value, newline);
  }
}

template<
  typename PosContainerType, typename PosMapper,
  typename TexContainerType, typename TexMapper,
  typename NmlContainerType, typename NmlMapper>
std::ostream& Write(
  std::ostream& os,
  const PosContainerType& pos_container,
  const PosMapper pos_mapper,
  const TexContainerType& tex_container,
  const TexMapper tex_mapper,
  const NmlContainerType& nml_container,
  const NmlMapper nml_mapper,
  const std::string& newline)
{
  WritePositions(os, pos_container, pos_mapper, newline);
  WriteTexCoords(os, tex_container, tex_mapper, newline);
  WriteNormals(os, nml_container, nml_mapper, newline);

  return os;
}


} // namespace detail

template<typename PosContainerType, typename PosMapper>
std::ostream& Write(
  std::ostream& os,
  const PosContainerType& pos_container, 
  const PosMapper pos_mapper,
  const std::string& newline = "\n")
{
  detail::Write(
    os,
    pos_container, pos_mapper,
    std::vector<float>{}, [](float) { return std::array<float, 2>{}; },
    std::vector<float>{}, [](float) { return std::array<float, 3>{}; },
    newline);
  return os;
}

} // namespace obj_io
} // namespace thinks

#endif // THINKS_OBJ_IO_MAPPER_H_INCLUDED
