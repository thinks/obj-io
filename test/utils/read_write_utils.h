// Copyright(C) 2018 Tommy Hinks <tommy.hinks@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef THINKS_OBJ_IO_UTILS_READ_WRITE_UTILS_H_INCLUDED
#define THINKS_OBJ_IO_UTILS_READ_WRITE_UTILS_H_INCLUDED

#include <utils/type_utils.h>
#include <thinks/obj_io/obj_io.h>

#include <array>
#include <exception>
#include <iostream>
#include <sstream>
#include <string>


namespace utils {
namespace detail {

template<typename ObjT>
struct ObjTypeMaker;

template<typename FloatT>
struct ObjTypeMaker<thinks::obj_io::Position<FloatT, 3>>
{
  template<typename VecFloatT>
  static constexpr
    thinks::obj_io::Position<FloatT, 3> Make(const Vec3<VecFloatT>& v) noexcept
  {
    return thinks::obj_io::Position<FloatT, 3>(v.x, v.y, v.z);
  }
};

template<typename FloatT>
struct ObjTypeMaker<thinks::obj_io::Position<FloatT, 4>>
{
  template<typename VecFloatT>
  static constexpr
    thinks::obj_io::Position<FloatT, 4> Make(const Vec4<VecFloatT>& v) noexcept
  {
    return thinks::obj_io::Position<FloatT, 4>(v.x, v.y, v.z, v.w);
  }
};

template<typename FloatT>
struct ObjTypeMaker<thinks::obj_io::TexCoord<FloatT, 2>>
{
  template<typename VecFloatT>
  static constexpr
    thinks::obj_io::TexCoord<FloatT, 2> Make(const Vec2<VecFloatT>& v) noexcept
  {
    return thinks::obj_io::TexCoord<FloatT, 2>(v.x, v.y);
  }
};

template<typename FloatT>
struct ObjTypeMaker<thinks::obj_io::TexCoord<FloatT, 3>>
{
  template<typename VecFloatT>
  static constexpr
    thinks::obj_io::TexCoord<FloatT, 3> Make(const Vec3<VecFloatT>& v) noexcept
  {
    return thinks::obj_io::TexCoord<FloatT, 3>(v.x, v.y, v.z);
  }
};

template<typename FloatT>
struct ObjTypeMaker<thinks::obj_io::Normal<FloatT>>
{
  template<typename VecFloatT>
  static constexpr
    thinks::obj_io::Normal<FloatT> Make(const Vec3<VecFloatT>& v) noexcept
  {
    return thinks::obj_io::Normal<FloatT>(v.x, v.y, v.z);
  }
};

template<typename IndexT>
struct ObjTypeMaker<thinks::obj_io::TriangleFace<IndexT>>
{
  static constexpr
    thinks::obj_io::TriangleFace<IndexT> Make(const std::array<IndexT, 3>& a) noexcept
  {
    return thinks::obj_io::TriangleFace<IndexT>(a[0], a[1], a[2]);
  }
};

template<typename IndexT>
struct ObjTypeMaker<thinks::obj_io::QuadFace<IndexT>>
{
  static constexpr
    thinks::obj_io::QuadFace<IndexT> Make(const std::array<IndexT, 4>& a) noexcept
  {
    return thinks::obj_io::TriangleFace<IndexT>(a[0], a[1], a[2], a[3]);
  }
};

template<typename IndexT>
struct ObjTypeMaker<thinks::obj_io::PolygonFace<IndexT>>
{
  template<std::size_t N>
  static constexpr
    thinks::obj_io::PolygonFace<IndexT> Make(const std::array<IndexT, N>& a) noexcept
  {
    auto face = thinks::obj_io::PolygonFace<IndexT>{};

    // Heap allocation!
    face.values.resize(std::tuple_size<std::array<IndexT, N>>::value); 
    
    for (auto i = std::size_t{ 0 }; i < face.values.size(); ++i) {
      face.values[i] = a[i];
    }
    return face;
  }
};


template<std::size_t IndicesPerFaceT, typename IndexT>
struct FaceSelector
{
  typedef thinks::obj_io::PolygonFace<IndexT> Type;
};

template<typename IndexT>
struct FaceSelector<3, IndexT>
{
  typedef thinks::obj_io::TriangleFace<IndexT> Type;
};

template<typename IndexT>
struct FaceSelector<4, IndexT>
{
  typedef thinks::obj_io::QuadFace<IndexT> Type;
};


template<
  typename FloatT,
  typename IntT,
  typename AddPosition,
  typename AddFace,
  typename AddTexCoord,
  typename AddNormal>
void ReadHelper(
  std::istream& is,
  AddPosition add_position,
  AddFace add_face,
  AddTexCoord add_tex_coord,
  AddNormal add_normal,
  const bool read_tex_coords,
  const bool read_normals)
{
  using thinks::obj_io::Read;

  if (!read_tex_coords && !read_normals)
  {
    Read<FloatT, IntT>(
      is,
      add_position,
      add_face);
  }
  else if (read_tex_coords && !read_normals) {
    Read<FloatT, IntT>(
      is,
      add_position,
      add_face,
      add_tex_coord);
  }
  else if (!read_tex_coords && read_normals) {
    Read<FloatT, IntT>(
      is,
      add_position,
      add_face,
      nullptr,
      add_normal);
  }
  else {
    Read<FloatT, IntT>(
      is,
      add_position,
      add_face,
      add_tex_coord,
      add_normal);
  }
}


template<
  typename PosMapper,
  typename FaceMapper,
  typename TexMapper,
  typename NmlMapper>
std::string WriteHelper(
  PosMapper pos_mapper,
  FaceMapper face_mapper,
  TexMapper tex_mapper,
  NmlMapper nml_mapper,
  const bool write_tex_coords,
  const bool write_normals)
{
  using thinks::obj_io::Write;

  auto oss = std::ostringstream{};
  if (!write_tex_coords && !write_normals)
  {
    Write(
      oss,
      pos_mapper,
      face_mapper);
  }
  else if (write_tex_coords && !write_normals) {
    Write(
      oss,
      pos_mapper,
      face_mapper,
      tex_mapper);
  }
  else if (!write_tex_coords && write_normals) {
    Write(
      oss,
      pos_mapper,
      face_mapper,
      nullptr,
      nml_mapper);
  }
  else {
    Write(
      oss,
      pos_mapper,
      face_mapper,
      tex_mapper,
      nml_mapper);
  }

  return oss.str();
}

} // namespace detail


template<typename MeshT>
MeshT ReadMesh(
  std::istream& is,
  const bool read_tex_coords,
  const bool read_normals)
{
  typedef MeshT MeshType;
  typedef typename MeshType::IndexType IndexType;
  typedef typename MeshType::VertexType VertexType;
  typedef typename VertexType::PositionType PositionType;
  typedef typename VertexType::TexCoordType TexCoordType;
  typedef typename VertexType::NormalType NormalType;

  auto mesh = MeshType{};
  auto pos_count = uint32_t{ 0 };
  auto tex_count = uint32_t{ 0 };
  auto nml_count = uint32_t{ 0 };

  // Positions.
  auto add_position = [&mesh, &pos_count](const auto& pos) {
    if (mesh.vertices.size() <= pos_count) {
      mesh.vertices.push_back(VertexType{});
    }
    mesh.vertices[pos_count++].pos = VecMaker<PositionType>::Make(pos.values);
  };

  // Texture coordinates.
  auto add_tex_coord = [&mesh, &tex_count](const auto& tex) {
    if (mesh.vertices.size() <= tex_count) {
      mesh.vertices.push_back(VertexType{});
    }
    mesh.vertices[tex_count++].tex = VecMaker<TexCoordType>::Make(tex.values);
  };

  // Normals.
  auto add_normal = [&mesh, &nml_count](const auto& nml) {
    if (mesh.vertices.size() <= nml_count) {
      mesh.vertices.push_back(VertexType{});
    }
    mesh.vertices[nml_count++].normal = VecMaker<NormalType>::Make(nml.values);
  };

  // Faces.
  auto add_face = [&mesh](const auto& face) {
    if (face.values.size() != MeshType::IndicesPerFace) {
      throw std::runtime_error("unexpected face index count");
    }
    for (const auto idx : face.values) {
      mesh.indices.push_back(idx.position_index.value);
    }
  };

  // float type???
  detail::ReadHelper<float, IndexType>(
    is,
    add_position,
    add_face,
    add_tex_coord,
    add_normal,
    read_tex_coords,
    read_normals);

  if (read_tex_coords && pos_count != tex_count ||
      read_normals && pos_count != nml_count) {
    throw std::runtime_error("all channels must have same value count");
  }

  return mesh;
}


template <typename IndexedMeshT>
IndexedMeshT ReadIndexedMesh(
  std::istream& is,
  const bool read_tex_coords,
  const bool read_normals)
{
  return IndexedMeshT{};
}


template<typename MeshT>
std::string WriteMesh(
  const MeshT& mesh,
  const bool write_tex_coords,
  const bool write_normals)
{
  using thinks::obj_io::End;
  using thinks::obj_io::Map;
  using detail::FaceSelector;
  using detail::ObjTypeMaker;
  using detail::WriteHelper;

  typedef MeshT MeshType;
  typedef typename MeshType::VertexType VertexType;

  const auto vtx_iend = std::end(mesh.vertices);

  // Positions.
  typedef typename VertexType::PositionType PositionType;
  using thinks::obj_io::Position;
  typedef Position<PositionType::ValueType, VecSize<PositionType>::value>
    ObjPositionType;

  auto pos_vtx_iter = begin(mesh.vertices);
  auto pos_mapper = [&pos_vtx_iter, vtx_iend]() {
    return pos_vtx_iter == vtx_iend ?
      End<ObjPositionType>() :
      Map(ObjTypeMaker<ObjPositionType>::Make((*pos_vtx_iter++).pos));
  };

  // Texture coordinates.
  typedef typename VertexType::TexCoordType TexCoordType;
  using thinks::obj_io::TexCoord;
  typedef TexCoord<TexCoordType::ValueType, VecSize<TexCoordType>::value>
    ObjTexCoordType;

  auto tex_vtx_iter = begin(mesh.vertices);
  auto tex_mapper = [&tex_vtx_iter, vtx_iend]() {
    return tex_vtx_iter == vtx_iend ? 
      End<ObjTexCoordType>() :
      Map(ObjTypeMaker<ObjTexCoordType>::Make((*tex_vtx_iter++).tex));
  };

  // Normals.
  typedef typename VertexType::NormalType NormalType;
  using thinks::obj_io::Normal;
  typedef Normal<NormalType::ValueType> ObjNormalType;
  auto nml_vtx_iter = begin(mesh.vertices);
  auto nml_mapper = [&nml_vtx_iter, vtx_iend]() {
    return nml_vtx_iter == vtx_iend ?
      End<ObjNormalType>() :
      Map(ObjTypeMaker<ObjNormalType>::Make((*nml_vtx_iter++).normal));
  };

  // Faces.
  typedef typename MeshType::IndexType MeshIndexType;
  using thinks::obj_io::Index;
  typedef Index<MeshIndexType> ObjIndexType;
  typedef FaceSelector<MeshType::IndicesPerFace, ObjIndexType>::Type 
    ObjFaceType;

  auto idx_iter = mesh.indices.begin();
  const auto idx_iend = mesh.indices.end();
  auto face_mapper = [&idx_iter, idx_iend]() {
    if (std::distance(idx_iter, idx_iend) < MeshType::IndicesPerFace) {
      return End<ObjFaceType>();
    }

    auto idx_buf = std::array<ObjIndexType, MeshType::IndicesPerFace>{};
    for (auto& idx : idx_buf) {
      idx = ObjIndexType(*idx_iter++);
    }
    return Map(ObjTypeMaker<ObjFaceType>::Make(idx_buf));
  };

  const auto mesh_str = WriteHelper(
    pos_mapper,
    face_mapper,
    tex_mapper,
    nml_mapper,
    write_tex_coords,
    write_normals);

  // Verify that all indices where mapped.
  if (idx_iter != idx_iend) {
    throw std::runtime_error("trailing indices");
  }

  return mesh_str;
}


template<typename IndexedMeshT>
std::string WriteIndexedMesh(
  const IndexedMeshT& imesh,
  const bool write_tex_coords,
  const bool write_normals)
{
  using thinks::obj_io::End;
  using thinks::obj_io::Map;
  using detail::FaceSelector;
  using detail::ObjTypeMaker;
  using detail::WriteHelper;

  typedef IndexedMeshT MeshType;

  // Positions.
  typedef typename MeshType::PositionType PositionType;
  using thinks::obj_io::Position;
  typedef Position<PositionType::ValueType, VecSize<PositionType>::value>
    ObjPositionType;
  auto pos_iter = std::begin(imesh.positions);
  const auto pos_iend = std::end(imesh.positions);
  auto pos_mapper = [&pos_iter, pos_iend]() {
    return pos_iter == pos_iend ?
      End<ObjPositionType>() :
      Map(ObjTypeMaker<ObjPositionType>::Make(*pos_iter++));
  };

  // Texture coordinates.
  typedef typename MeshType::TexCoordType TexCoordType;
  using thinks::obj_io::TexCoord;
  typedef TexCoord<TexCoordType::ValueType, VecSize<TexCoordType>::value>
    ObjTexCoordType;
  auto tex_iter = std::begin(imesh.tex_coords);
  const auto tex_iend = std::end(imesh.tex_coords);
  auto tex_mapper = [&tex_iter, tex_iend]() {
    return tex_iter == tex_iend ?
      End<ObjTexCoordType>() :
      Map(ObjTypeMaker<ObjTexCoordType>::Make(*tex_iter++));
  };

  // Normals.
  typedef typename MeshType::NormalType NormalType;
  using thinks::obj_io::Normal;
  typedef Normal<NormalType::ValueType> ObjNormalType;
  auto nml_iter = std::begin(imesh.normals);
  const auto nml_iend = std::end(imesh.normals);
  auto nml_mapper = [&nml_iter, nml_iend]() {
    return nml_iter == nml_iend ?
      End<ObjNormalType>() :
      Map(ObjTypeMaker<ObjNormalType>::Make(*nml_iter++));
  };

  // Faces.
  typedef typename MeshType::IndexType MeshIndexType;
  using thinks::obj_io::IndexGroup;
  typedef IndexGroup<MeshIndexType> ObjIndexGroupType;
  typedef FaceSelector<MeshType::IndicesPerFace, ObjIndexGroupType>::Type
    ObjFaceType;

  auto pos_idx_iter = std::begin(imesh.position_indices);
  auto pos_idx_iend = std::end(imesh.position_indices);
  auto tex_idx_iter = std::begin(imesh.tex_coord_indices);
  auto tex_idx_iend = std::end(imesh.tex_coord_indices);
  auto nml_idx_iter = std::begin(imesh.normal_indices);
  auto nml_idx_iend = std::end(imesh.normal_indices);
  auto face_mapper =
    [&pos_idx_iter, &tex_idx_iter, &nml_idx_iter,
    pos_idx_iend, tex_idx_iend, nml_idx_iend,
    write_tex_coords, write_normals]() {
    if (std::distance(pos_idx_iter, pos_idx_iend) < MeshType::IndicesPerFace ||
        std::distance(tex_idx_iter, tex_idx_iend) < MeshType::IndicesPerFace ||
        std::distance(nml_idx_iter, nml_idx_iend) < MeshType::IndicesPerFace) {
      return End<ObjFaceType>();
    }

    auto idx_group_buf = std::array<ObjIndexGroupType, MeshType::IndicesPerFace>{};
    for (auto& idx_group : idx_group_buf) {
      idx_group = ObjIndexGroupType(*pos_idx_iter++, *tex_idx_iter++, *nml_idx_iter++);
      idx_group.tex_coord_index.second = write_tex_coords;
      idx_group.normal_index.second = write_normals;
    }
    return Map(ObjTypeMaker<ObjFaceType>::Make(idx_group_buf));
  };

  const auto mesh_str = WriteHelper(
    pos_mapper,
    face_mapper,
    tex_mapper,
    nml_mapper,
    write_tex_coords,
    write_normals);

  // Verify that all indices where mapped.
  if (pos_idx_iter != pos_idx_iend) {
    throw std::runtime_error("trailing position indices");
  }

  if (tex_idx_iter != tex_idx_iend) {
    throw std::runtime_error("trailing texture coordinate indices");
  }

  if (nml_idx_iter != nml_idx_iend) {
    throw std::runtime_error("trailing normal indices");
  }

  return mesh_str;
}

} // namespace utils

#endif // THINKS_OBJ_IO_UTILS_READ_WRITE_UTILS_H_INCLUDED
