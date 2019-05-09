// Copyright(C) 2018 Tommy Hinks <tommy.hinks@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#pragma once

#include <array>
#include <exception>
#include <iostream>
#include <sstream>
#include <string>

#include "thinks/obj_io/obj_io.h"
#include "mesh_types.h"

struct WriteResult {
  thinks::ObjWriteResult write_result;
  std::string mesh_str;
};

template <typename MeshT>
struct ReadResult {
  thinks::ObjReadResult read_result;
  MeshT mesh;
};

namespace read_write_utils_internal {

template <typename ObjT>
struct ObjTypeMaker;

template <typename FloatT>
struct ObjTypeMaker<thinks::ObjPosition<FloatT, 3>> {
  template <typename VecFloatT>
  static constexpr thinks::ObjPosition<FloatT, 3> Make(
      const Vec3<VecFloatT>& v) noexcept {
    return {v.x, v.y, v.z};
  }
};

template <typename FloatT>
struct ObjTypeMaker<thinks::ObjPosition<FloatT, 4>> {
  template <typename VecFloatT>
  static constexpr thinks::ObjPosition<FloatT, 4> Make(
      const Vec4<VecFloatT>& v) noexcept {
    return {v.x, v.y, v.z, v.w};
  }
};

template <typename FloatT>
struct ObjTypeMaker<thinks::ObjTexCoord<FloatT, 2>> {
  template <typename VecFloatT>
  static constexpr thinks::ObjTexCoord<FloatT, 2> Make(
      const Vec2<VecFloatT>& v) noexcept {
    return {v.x, v.y};
  }
};

template <typename FloatT>
struct ObjTypeMaker<thinks::ObjTexCoord<FloatT, 3>> {
  template <typename VecFloatT>
  static constexpr thinks::ObjTexCoord<FloatT, 3> Make(
      const Vec3<VecFloatT>& v) noexcept {
    return {v.x, v.y, v.z};
  }
};

template <typename FloatT>
struct ObjTypeMaker<thinks::ObjNormal<FloatT>> {
  template <typename VecFloatT>
  static constexpr thinks::ObjNormal<FloatT> Make(
      const Vec3<VecFloatT>& v) noexcept {
    return {v.x, v.y, v.z};
  }
};

template <typename IndexT>
struct ObjTypeMaker<thinks::ObjTriangleFace<IndexT>> {
  static constexpr thinks::ObjTriangleFace<IndexT> Make(
      const std::array<IndexT, 3>& a) noexcept {
    return {a[0], a[1], a[2]};
  }
};

template <typename IndexT>
struct ObjTypeMaker<thinks::ObjQuadFace<IndexT>> {
  static constexpr thinks::ObjQuadFace<IndexT> Make(
      const std::array<IndexT, 4>& a) noexcept {
    return {a[0], a[1], a[2], a[3]};
  }
};

template <typename IndexT>
struct ObjTypeMaker<thinks::ObjPolygonFace<IndexT>> {
  template <std::size_t N>
  static constexpr thinks::ObjPolygonFace<IndexT> Make(
      const std::array<IndexT, N>& a) noexcept {
    auto face = thinks::ObjPolygonFace<IndexT>{};

    // Heap allocation!
    face.values.resize(std::tuple_size<std::array<IndexT, N>>::value);

    for (std::size_t i = 0; i < face.values.size(); ++i) {
      face.values[i] = a[i];
    }
    return face;
  }
};

template <std::size_t IndicesPerFaceT, typename IndexT>
struct FaceSelector {
  using Type = thinks::ObjPolygonFace<IndexT>;
};

template <typename IndexT>
struct FaceSelector<3, IndexT> {
  using Type = thinks::ObjTriangleFace<IndexT>;
};

template <typename IndexT>
struct FaceSelector<4, IndexT> {
  using Type = thinks::ObjQuadFace<IndexT>;
};

template <typename AddPositionFuncT, typename AddFaceFuncT,
          typename AddTexCoordFuncT, typename AddNormalFuncT>
thinks::ObjReadResult ReadHelper(
    std::istream& is, AddPositionFuncT&& add_position, AddFaceFuncT&& add_face,
    AddTexCoordFuncT&& add_tex_coord, AddNormalFuncT&& add_normal,
    const bool read_tex_coords, const bool read_normals) {
  using thinks::ReadObj;

  auto result = thinks::ObjReadResult{};
  if (!read_tex_coords && !read_normals) {
    result = ReadObj(is, std::forward<AddPositionFuncT>(add_position),
                     std::forward<AddFaceFuncT>(add_face));
  } else if (read_tex_coords && !read_normals) {
    result = ReadObj(is, std::forward<AddPositionFuncT>(add_position),
                     std::forward<AddFaceFuncT>(add_face), add_tex_coord);
  } else if (!read_tex_coords && read_normals) {
    result = ReadObj(is, std::forward<AddPositionFuncT>(add_position),
                     std::forward<AddFaceFuncT>(add_face),
                     nullptr /* add_tex_coord */,
                     std::forward<AddNormalFuncT>(add_normal));
  } else {
    result = ReadObj(is, std::forward<AddPositionFuncT>(add_position),
                     std::forward<AddFaceFuncT>(add_face),
                     std::forward<AddTexCoordFuncT>(add_tex_coord),
                     std::forward<AddNormalFuncT>(add_normal));
  }
  return result;
}

template <typename PosMapperT, typename FaceMapperT, typename TexMapperT,
          typename NmlMapperT>
WriteResult WriteHelper(PosMapperT&& pos_mapper, FaceMapperT&& face_mapper,
                        TexMapperT&& tex_mapper, NmlMapperT&& nml_mapper,
                        const bool write_tex_coords, const bool write_normals) {
  using thinks::WriteObj;

  auto result = thinks::ObjWriteResult{};
  auto oss = std::ostringstream{};
  if (!write_tex_coords && !write_normals) {
    result = WriteObj(oss, std::forward<PosMapperT>(pos_mapper),
                      std::forward<FaceMapperT>(face_mapper));
  } else if (write_tex_coords && !write_normals) {
    result = WriteObj(oss, std::forward<PosMapperT>(pos_mapper),
                      std::forward<FaceMapperT>(face_mapper),
                      std::forward<TexMapperT>(tex_mapper));
  } else if (!write_tex_coords && write_normals) {
    result = WriteObj(oss, std::forward<PosMapperT>(pos_mapper),
                      std::forward<FaceMapperT>(face_mapper), 
                      nullptr, // No texture coordinates!
                      std::forward<NmlMapperT>(nml_mapper));
  } else {
    result = WriteObj(oss, std::forward<PosMapperT>(pos_mapper),
                      std::forward<FaceMapperT>(face_mapper),
                      std::forward<TexMapperT>(tex_mapper),
                      std::forward<NmlMapperT>(nml_mapper));
  }

  return {result, oss.str()};
}

}  // namespace read_write_utils_internal

template <typename MeshT>
ReadResult<MeshT> ReadMesh(std::istream& is, const bool read_tex_coords,
                           const bool read_normals) {
  using thinks::MakeObjAddFunc;

  using MeshType = MeshT;
  using VertexType = MeshType::VertexType;

  auto mesh = MeshType{};
  auto pos_count = uint32_t{0};
  auto tex_count = uint32_t{0};
  auto nml_count = uint32_t{0};

  // Positions.
  using PositionType = VertexType::PositionType;
  using ObjPositionType = thinks::ObjPosition<PositionType::ValueType,
                                              VecSize<PositionType>::value>;

  auto add_position =
      MakeObjAddFunc<ObjPositionType>([&mesh, &pos_count](const auto& pos) {
        if (mesh.vertices.size() <= pos_count) {
          mesh.vertices.push_back(VertexType{});
        }
        mesh.vertices[pos_count++].pos =
            VecMaker<PositionType>::Make(pos.values);
      });

  // Faces.
  using ObjFaceType = read_write_utils_internal::FaceSelector<
      MeshType::IndicesPerFace, thinks::ObjIndex<MeshType::IndexType>>::Type;

  auto add_face = MakeObjAddFunc<ObjFaceType>([&mesh](const auto& face) {
    if (face.values.size() != MeshType::IndicesPerFace) {
      throw std::runtime_error("unexpected face index count");
    }
    for (const auto idx : face.values) {
      mesh.indices.push_back(idx.value);
    }
  });

  // Texture coordinates [optional].
  using TexCoordType = VertexType::TexCoordType;
  using ObjTexCoordType = thinks::ObjTexCoord<TexCoordType::ValueType,
                                              VecSize<TexCoordType>::value>;

  auto add_tex_coord =
      MakeObjAddFunc<ObjTexCoordType>([&mesh, &tex_count](const auto& tex) {
        if (mesh.vertices.size() <= tex_count) {
          mesh.vertices.push_back(VertexType{});
        }
        mesh.vertices[tex_count++].tex =
            VecMaker<TexCoordType>::Make(tex.values);
      });

  // Normals [optional].
  using NormalType = VertexType::NormalType;
  using ObjNormalType = thinks::ObjNormal<NormalType::ValueType>;

  auto add_normal =
      MakeObjAddFunc<ObjNormalType>([&mesh, &nml_count](const auto& nml) {
        if (mesh.vertices.size() <= nml_count) {
          mesh.vertices.push_back(VertexType{});
        }
        mesh.vertices[nml_count++].normal =
            VecMaker<NormalType>::Make(nml.values);
      });

  const auto result = read_write_utils_internal::ReadHelper(
      is, add_position, add_face, add_tex_coord, add_normal, read_tex_coords,
      read_normals);

  // Some sanity checks...
  if (read_tex_coords && pos_count != tex_count) {
    throw std::runtime_error("tex coord count must match position count");
  }
  if (read_normals && pos_count != nml_count) {
    throw std::runtime_error("normal count must match position count");
  }

  if (result.position_count != mesh.vertices.size()) {
    throw std::runtime_error("bad position count");
  }
  if (read_tex_coords && result.tex_coord_count != mesh.vertices.size()) {
    throw std::runtime_error("bad tex coord count");
  }
  if (read_normals && result.normal_count != mesh.vertices.size()) {
    throw std::runtime_error("bad normal count");
  }
  if (result.face_count != mesh.indices.size() / MeshType::IndicesPerFace) {
    throw std::runtime_error("bad face count");
  }

  return {result, mesh};
}

template <typename IndexedMeshT>
ReadResult<IndexedMeshT> ReadIndexGroupMesh(std::istream& is,
                                            const bool read_tex_coords,
                                            const bool read_normals) {
  using thinks::MakeObjAddFunc;
  using MeshType = IndexedMeshT;

  auto mesh = MeshType{};

  // Positions.
  using PositionType = MeshType::PositionType;
  using ObjPositionType = thinks::ObjPosition<PositionType::ValueType,
                                              VecSize<PositionType>::value>;

  auto add_position = MakeObjAddFunc<ObjPositionType>([&mesh](const auto& pos) {
    mesh.positions.push_back(VecMaker<PositionType>::Make(pos.values));
  });

  // Faces.
  using ObjFaceType = read_write_utils_internal::FaceSelector<
      MeshType::IndicesPerFace,
      thinks::ObjIndexGroup<MeshType::IndexType>>::Type;

  auto add_face = MakeObjAddFunc<ObjFaceType>(
      [&mesh, read_tex_coords, read_normals](const auto& face) {
        if (face.values.size() != MeshType::IndicesPerFace) {
          throw std::runtime_error("unexpected face index count");
        }
        for (const auto idx : face.values) {
          mesh.position_indices.push_back(idx.position_index.value);

          if (read_tex_coords && idx.tex_coord_index.second) {
            mesh.tex_coord_indices.push_back(idx.tex_coord_index.first.value);
          }
          if (read_normals && idx.normal_index.second) {
            mesh.normal_indices.push_back(idx.normal_index.first.value);
          }
        }
      });

  // Texture coordinates [optional.
  using TexCoordType = MeshType::TexCoordType;
  using ObjTexCoordType = thinks::ObjTexCoord<TexCoordType::ValueType,
                                              VecSize<TexCoordType>::value>;

  auto add_tex_coord =
      MakeObjAddFunc<ObjTexCoordType>([&mesh](const auto& tex) {
        mesh.tex_coords.push_back(VecMaker<TexCoordType>::Make(tex.values));
      });

  // Normals [optional].
  using NormalType = MeshType::NormalType;
  using ObjNormalType = thinks::ObjNormal<NormalType::ValueType>;

  auto add_normal = MakeObjAddFunc<ObjNormalType>([&mesh](const auto& nml) {
    mesh.normals.push_back(VecMaker<NormalType>::Make(nml.values));
  });

  const auto result = read_write_utils_internal::ReadHelper(
      is, add_position, add_face, add_tex_coord, add_normal, read_tex_coords,
      read_normals);

  // Some sanity checks...
  if (result.position_count != mesh.positions.size()) {
    throw std::runtime_error("bad position count");
  }
  if (read_tex_coords && result.tex_coord_count != mesh.tex_coords.size()) {
    throw std::runtime_error("bad tex coord count");
  }
  if (read_normals && result.normal_count != mesh.normals.size()) {
    throw std::runtime_error("bad normal count");
  }
  if (result.face_count !=
      mesh.position_indices.size() / MeshType::IndicesPerFace) {
    throw std::runtime_error("bad face count");
  }
  if (read_tex_coords && result.face_count != mesh.tex_coord_indices.size() /
                                                  MeshType::IndicesPerFace) {
    throw std::runtime_error("bad face count");
  }
  if (read_normals && result.face_count != mesh.normal_indices.size() /
                                               MeshType::IndicesPerFace) {
    throw std::runtime_error("bad face count");
  }

  return {result, mesh};
}

template <typename MeshT>
WriteResult WriteMesh(const MeshT& mesh, const bool write_tex_coords,
                      const bool write_normals) {
  using thinks::ObjEnd;
  using thinks::ObjMap;
  using read_write_utils_internal::FaceSelector;
  using read_write_utils_internal::ObjTypeMaker;
  using read_write_utils_internal::WriteHelper;

  using MeshType = MeshT;
  using VertexType = MeshType::VertexType;

  const auto vtx_iend = std::end(mesh.vertices);

  // Positions.
  auto pos_vtx_iter = begin(mesh.vertices);
  auto pos_mapper = [&pos_vtx_iter, vtx_iend]() {
    using PositionType = VertexType::PositionType;
    using ObjPositionType = thinks::ObjPosition<PositionType::ValueType,
                                                VecSize<PositionType>::value>;

    return pos_vtx_iter == vtx_iend
               ? ObjEnd<ObjPositionType>()
               : ObjMap(ObjTypeMaker<ObjPositionType>::Make(
                     (*pos_vtx_iter++).pos));
  };

  // Texture coordinates.
  auto tex_vtx_iter = begin(mesh.vertices);
  auto tex_mapper = [&tex_vtx_iter, vtx_iend]() {
    using TexCoordType = VertexType::TexCoordType;
    using ObjTexCoordType = thinks::ObjTexCoord<TexCoordType::ValueType,
                                                VecSize<TexCoordType>::value>;

    return tex_vtx_iter == vtx_iend
               ? ObjEnd<ObjTexCoordType>()
               : ObjMap(ObjTypeMaker<ObjTexCoordType>::Make(
                     (*tex_vtx_iter++).tex));
  };

  // Normals.
  auto nml_vtx_iter = begin(mesh.vertices);
  auto nml_mapper = [&nml_vtx_iter, vtx_iend]() {
    using NormalType = VertexType::NormalType;
    using ObjNormalType = thinks::ObjNormal<NormalType::ValueType>;

    return nml_vtx_iter == vtx_iend ? ObjEnd<ObjNormalType>()
                                    : ObjMap(ObjTypeMaker<ObjNormalType>::Make(
                                          (*nml_vtx_iter++).normal));
  };

  // Faces.
  auto idx_iter = mesh.indices.begin();
  const auto idx_iend = mesh.indices.end();
  auto face_mapper = [&idx_iter, idx_iend]() {
    using MeshIndexType = MeshType::IndexType;
    using ObjIndexType = thinks::ObjIndex<MeshIndexType>;
    using ObjFaceType =
        FaceSelector<MeshType::IndicesPerFace, ObjIndexType>::Type;

    if (std::distance(idx_iter, idx_iend) < MeshType::IndicesPerFace) {
      return ObjEnd<ObjFaceType>();
    }

    auto idx_buf = std::array<ObjIndexType, MeshType::IndicesPerFace>{};
    for (auto& idx : idx_buf) {
      idx = ObjIndexType(*idx_iter++);
    }
    return ObjMap(ObjTypeMaker<ObjFaceType>::Make(idx_buf));
  };

  const auto result = read_write_utils_internal::WriteHelper(
      pos_mapper, face_mapper, tex_mapper, nml_mapper, write_tex_coords,
      write_normals);
  const auto wr = result.write_result;

  // Some sanity checks...
  if (wr.position_count != mesh.vertices.size()) {
    throw std::runtime_error("bad position count");
  }
  if (write_tex_coords && wr.tex_coord_count != mesh.vertices.size()) {
    throw std::runtime_error("bad tex coord count");
  }
  if (write_normals && wr.normal_count != mesh.vertices.size()) {
    throw std::runtime_error("bad normal count");
  }
  if (wr.face_count != mesh.indices.size() / MeshType::IndicesPerFace) {
    throw std::runtime_error("bad index count");
  }

  return result;
}

template <typename IndexedMeshT>
WriteResult WriteIndexGroupMesh(const IndexedMeshT& imesh,
                                const bool write_tex_coords,
                                const bool write_normals) {
  using thinks::ObjEnd;
  using thinks::ObjMap;
  using read_write_utils_internal::FaceSelector;
  using read_write_utils_internal::ObjTypeMaker;
  using read_write_utils_internal::WriteHelper;

  using MeshType = IndexedMeshT;

  // Positions.
  auto pos_iter = std::begin(imesh.positions);
  const auto pos_iend = std::end(imesh.positions);
  auto pos_mapper = [&pos_iter, pos_iend]() {
    using PositionType = MeshType::PositionType;
    using ObjPositionType = thinks::ObjPosition<PositionType::ValueType,
                                                VecSize<PositionType>::value>;

    return pos_iter == pos_iend
               ? ObjEnd<ObjPositionType>()
               : ObjMap(ObjTypeMaker<ObjPositionType>::Make(*pos_iter++));
  };

  // Texture coordinates.
  auto tex_iter = std::begin(imesh.tex_coords);
  const auto tex_iend = std::end(imesh.tex_coords);
  auto tex_mapper = [&tex_iter, tex_iend]() {
    using TexCoordType = MeshType::TexCoordType;
    using ObjTexCoordType = thinks::ObjTexCoord<TexCoordType::ValueType,
                                                VecSize<TexCoordType>::value>;

    return tex_iter == tex_iend
               ? ObjEnd<ObjTexCoordType>()
               : ObjMap(ObjTypeMaker<ObjTexCoordType>::Make(*tex_iter++));
  };

  // Normals.
  auto nml_iter = std::begin(imesh.normals);
  const auto nml_iend = std::end(imesh.normals);
  auto nml_mapper = [&nml_iter, nml_iend]() {
    using NormalType = MeshType::NormalType;
    using ObjNormalType = thinks::ObjNormal<NormalType::ValueType>;

    return nml_iter == nml_iend
               ? ObjEnd<ObjNormalType>()
               : ObjMap(ObjTypeMaker<ObjNormalType>::Make(*nml_iter++));
  };

  // Faces.
  auto pos_idx_iter = std::begin(imesh.position_indices);
  auto pos_idx_iend = std::end(imesh.position_indices);
  auto tex_idx_iter = std::begin(imesh.tex_coord_indices);
  auto tex_idx_iend = std::end(imesh.tex_coord_indices);
  auto nml_idx_iter = std::begin(imesh.normal_indices);
  auto nml_idx_iend = std::end(imesh.normal_indices);
  auto face_mapper = [&pos_idx_iter, &tex_idx_iter, &nml_idx_iter, pos_idx_iend,
                      tex_idx_iend, nml_idx_iend, write_tex_coords,
                      write_normals]() {
    using MeshIndexType = MeshType::IndexType;
    using ObjIndexGroupType = thinks::ObjIndexGroup<MeshIndexType>;
    using ObjFaceType =
        FaceSelector<MeshType::IndicesPerFace, ObjIndexGroupType>::Type;

    if (std::distance(pos_idx_iter, pos_idx_iend) < MeshType::IndicesPerFace ||
        std::distance(tex_idx_iter, tex_idx_iend) < MeshType::IndicesPerFace ||
        std::distance(nml_idx_iter, nml_idx_iend) < MeshType::IndicesPerFace) {
      return ObjEnd<ObjFaceType>();
    }

    auto idx_group_buf =
        std::array<ObjIndexGroupType, MeshType::IndicesPerFace>{};
    for (auto& idx_group : idx_group_buf) {
      idx_group =
          ObjIndexGroupType(*pos_idx_iter++, *tex_idx_iter++, *nml_idx_iter++);
      idx_group.tex_coord_index.second = write_tex_coords;
      idx_group.normal_index.second = write_normals;
    }
    return ObjMap(ObjTypeMaker<ObjFaceType>::Make(idx_group_buf));
  };

  const auto result = read_write_utils_internal::WriteHelper(
      pos_mapper, face_mapper, tex_mapper, nml_mapper, write_tex_coords,
      write_normals);
  const auto wr = result.write_result;

  // Some sanity checks...
  if (wr.position_count != imesh.positions.size()) {
    throw std::runtime_error("bad position count");
  }
  if (write_tex_coords && wr.tex_coord_count != imesh.tex_coords.size()) {
    throw std::runtime_error("bad tex coord count");
  }
  if (write_normals && wr.normal_count != imesh.normals.size()) {
    throw std::runtime_error("bad normal count");
  }
  if (wr.face_count !=
      imesh.position_indices.size() / MeshType::IndicesPerFace) {
    throw std::runtime_error("bad position index count");
  }
  if (write_tex_coords && wr.face_count != imesh.tex_coord_indices.size() /
                                               MeshType::IndicesPerFace) {
    throw std::runtime_error("bad tex coord index count");
  }
  if (write_normals && wr.face_count != imesh.position_indices.size() /
                                            MeshType::IndicesPerFace) {
    throw std::runtime_error("bad normal index count");
  }

  return result;
}
