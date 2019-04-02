// Copyright(C) 2018 Tommy Hinks <tommy.hinks@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#pragma once

#include <array>
#include <cstdint>
#include <vector>

namespace utils {

template <typename FloatT>
struct Vec2 {
  typedef FloatT ValueType;

  FloatT x = FloatT{0};
  FloatT y = FloatT{0};
};

template <typename FloatT>
struct Vec3 {
  typedef FloatT ValueType;

  FloatT x = FloatT{0};
  FloatT y = FloatT{0};
  FloatT z = FloatT{0};
};

template <typename FloatT>
struct Vec4 {
  typedef FloatT ValueType;

  FloatT x = FloatT{0};
  FloatT y = FloatT{0};
  FloatT z = FloatT{0};
  FloatT w = FloatT{0};
};

template <typename T>
struct VecSize;

template <typename T>
struct VecSize<Vec2<T>> {
  static constexpr std::size_t value = 2;
};

template <typename T>
struct VecSize<Vec3<T>> {
  static constexpr std::size_t value = 3;
};

template <typename T>
struct VecSize<Vec4<T>> {
  static constexpr std::size_t value = 4;
};

template <typename VecT>
struct VecMaker;

template <typename FloatT>
struct VecMaker<Vec2<FloatT>> {
  template <typename T, std::size_t N>
  static constexpr Vec2<FloatT> Make(const std::array<T, N>& a) {
    static_assert(VecSize<Vec2<FloatT>>::value <=
                      std::tuple_size<std::array<T, N>>::value,
                  "vec type must be smaller or equal than array");
    return Vec2<FloatT>{a[0], a[1]};
  }
};

template <typename FloatT>
struct VecMaker<Vec3<FloatT>> {
  template <typename T, std::size_t N>
  static constexpr Vec3<FloatT> Make(const std::array<T, N>& a) noexcept {
    static_assert(VecSize<Vec3<FloatT>>::value <=
                      std::tuple_size<std::array<T, N>>::value,
                  "vec type must be smaller or equal than array");
    return Vec3<FloatT>{a[0], a[1], a[2]};
  }
};

template <typename FloatT>
struct VecMaker<Vec4<FloatT>> {
  template <typename T, std::size_t N>
  static constexpr Vec4<FloatT> Make(const std::array<T, N>& a) {
    static_assert(VecSize<Vec4<FloatT>>::value <=
                      std::tuple_size<std::array<T, N>>::value,
                  "vec type must be smaller or equal than array");
    return Vec4<FloatT>{a[0], a[1], a[2], a[3]};
  }
};

template <typename FloatT>
bool Equals(const Vec2<FloatT>& lhs, const Vec2<FloatT>& rhs) {
  return lhs.x == rhs.x && lhs.y == rhs.y;
}

template <typename FloatT>
bool Equals(const Vec3<FloatT>& lhs, const Vec3<FloatT>& rhs) {
  return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}

template <typename FloatT>
bool Equals(const Vec4<FloatT>& lhs, const Vec4<FloatT>& rhs) {
  return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z && lhs.w == rhs.w;
}

template <typename PositionT = Vec3<float>, typename TexCoordT = Vec2<float>,
          typename NormalT = Vec3<float>, typename ColorT = Vec4<float>>
struct Vertex {
  typedef PositionT PositionType;
  typedef TexCoordT TexCoordType;
  typedef NormalT NormalType;
  typedef ColorT ColorType;

  PositionType pos;
  TexCoordType tex;
  NormalType normal;

  // Attribute that is not supported by OBJ format,
  // just to make it interesting...
  ColorType color;
};

template <typename VertexT = Vertex<>, typename IntT = std::uint32_t,
          std::size_t IndicesPerFaceT = 3>
struct Mesh {
  typedef VertexT VertexType;
  typedef IntT IndexType;
  static constexpr std::size_t IndicesPerFace = IndicesPerFaceT;

  std::vector<VertexType> vertices;
  std::vector<IntT> indices;
};

template <typename VertexT = Vertex<>, typename IntT = std::uint32_t>
using TriangleMesh = Mesh<VertexT, IntT, 3>;

template <typename VertexT = Vertex<>, typename IntT = std::uint32_t>
using QuadMesh = Mesh<VertexT, IntT, 4>;

template <typename PositionT = Vec3<float>, typename TexCoordT = Vec2<float>,
          typename NormalT = Vec3<float>, typename IntT = std::uint32_t,
          std::size_t IndicesPerFaceT = 3>
struct IndexGroupMesh {
  typedef PositionT PositionType;
  typedef TexCoordT TexCoordType;
  typedef NormalT NormalType;
  typedef IntT IndexType;
  static constexpr std::size_t IndicesPerFace = IndicesPerFaceT;

  std::vector<PositionType> positions;
  std::vector<TexCoordType> tex_coords;
  std::vector<NormalType> normals;
  std::vector<IndexType> position_indices;
  std::vector<IndexType> tex_coord_indices;
  std::vector<IndexType> normal_indices;
};

template <typename PositionT = Vec3<float>, typename TexCoordT = Vec2<float>,
          typename NormalT = Vec3<float>, typename IntT = std::uint32_t>
using IndexGroupTriangleMesh =
    IndexGroupMesh<PositionT, TexCoordT, NormalT, IntT, 3>;

template <typename PositionT = Vec3<float>, typename TexCoordT = Vec2<float>,
          typename NormalT = Vec3<float>, typename IntT = std::uint32_t>
using IndexGroupQuadMesh =
    IndexGroupMesh<PositionT, TexCoordT, NormalT, IntT, 4>;

}  // namespace utils
