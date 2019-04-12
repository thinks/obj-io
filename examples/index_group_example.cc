// Copyright(C) 2018 Tommy Hinks <tommy.hinks@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include "index_group_example.h"

#include <cassert>
#include <fstream>
#include <vector>

#include "thinks/obj_io/obj_io.h"

namespace {

struct Vec3 {
  float x;
  float y;
  float z;
};

struct Mesh {
  std::vector<Vec3> positions;
  std::vector<Vec3> normals;
  std::vector<std::uint16_t> position_indices;
  std::vector<std::uint16_t> normal_indices;
};

Mesh ReadMesh(const std::string& filename) {
  auto mesh = Mesh{};

  // Positions.
  auto add_position = thinks::MakeObjAddFunc<
      thinks::ObjPosition<float, 3>>([&mesh](const auto& pos) {
    mesh.positions.push_back(Vec3{pos.values[0], pos.values[1], pos.values[2]});
  });

  // Normals.
  auto add_normal = thinks::MakeObjAddFunc<
      thinks::ObjNormal<float>>([&mesh](const auto& nml) {
    mesh.normals.push_back(Vec3{nml.values[0], nml.values[1], nml.values[2]});
  });

  // Faces.
  using ObjFaceType = thinks::ObjTriangleFace<thinks::ObjIndexGroup<uint16_t>>;
  auto add_face =
      thinks::MakeObjAddFunc<ObjFaceType>([&mesh](const auto& face) {
        mesh.position_indices.push_back(face.values[0].position_index.value);
        mesh.position_indices.push_back(face.values[1].position_index.value);
        mesh.position_indices.push_back(face.values[2].position_index.value);
        assert(face.values[0].normal_index.second &&
               "index group must have normal");
        assert(face.values[1].normal_index.second &&
               "index group must have normal");
        assert(face.values[2].normal_index.second &&
               "index group must have normal");
        mesh.normal_indices.push_back(face.values[0].normal_index.first.value);
        mesh.normal_indices.push_back(face.values[1].normal_index.first.value);
        mesh.normal_indices.push_back(face.values[2].normal_index.first.value);
      });

  auto ifs = std::ifstream(filename);
  assert(ifs);
  const auto result = thinks::ReadObj(ifs, add_position, add_face,
                                      nullptr,  // no texture coordinates.
                                      add_normal);
  ifs.close();

  return mesh;
}

void WriteMesh(const std::string& filename, const Mesh& mesh) {
  // Positions.
  const auto pos_iend = std::end(mesh.positions);
  auto pos_iter = std::begin(mesh.positions);
  auto pos_mapper = [&pos_iter, pos_iend]() {
    using ObjPositionType = thinks::ObjPosition<float, 3>;

    if (pos_iter == pos_iend) {
      return thinks::ObjEnd<ObjPositionType>();
    }

    const auto pos = *pos_iter++;
    return thinks::ObjMap(ObjPositionType(pos.x, pos.y, pos.z));
  };

  // Faces.
  const auto pos_idx_iend = std::end(mesh.position_indices);
  const auto nml_idx_iend = std::end(mesh.normal_indices);
  auto pos_idx_iter = std::begin(mesh.position_indices);
  auto nml_idx_iter = std::begin(mesh.normal_indices);
  auto face_mapper = [&pos_idx_iter, &nml_idx_iter, pos_idx_iend,
                      nml_idx_iend]() {
    using ObjIndexType = thinks::ObjIndexGroup<uint16_t>;
    using ObjFaceType = thinks::ObjTriangleFace<ObjIndexType>;

    if (distance(pos_idx_iter, pos_idx_iend) < 3 ||
        distance(nml_idx_iter, nml_idx_iend) < 3) {
      return thinks::ObjEnd<ObjFaceType>();
    }

    const auto pos_idx0 = *pos_idx_iter++;
    const auto pos_idx1 = *pos_idx_iter++;
    const auto pos_idx2 = *pos_idx_iter++;
    const auto nml_idx0 = *nml_idx_iter++;
    const auto nml_idx1 = *nml_idx_iter++;
    const auto nml_idx2 = *nml_idx_iter++;

    // No texture coordinates.
    return thinks::ObjMap(
        ObjFaceType(ObjIndexType(pos_idx0, std::make_pair(0, false),
                                 std::make_pair(nml_idx0, true)),
                    ObjIndexType(pos_idx1, std::make_pair(0, false),
                                 std::make_pair(nml_idx1, true)),
                    ObjIndexType(pos_idx2, std::make_pair(0, false),
                                 std::make_pair(nml_idx2, true))));
  };

  // Normals.
  const auto nml_iend = end(mesh.normals);
  auto nml_iter = begin(mesh.normals);
  auto nml_mapper = [&nml_iter, nml_iend]() {
    using ObjNormalType = thinks::ObjNormal<float>;

    if (nml_iter == nml_iend) {
      return thinks::ObjEnd<ObjNormalType>();
    }

    const auto nml = *nml_iter++;
    return thinks::ObjMap(ObjNormalType(nml.x, nml.y, nml.z));
  };

  auto ofs = std::ofstream(filename);
  assert(ofs);
  const auto result = thinks::WriteObj(ofs, pos_mapper, face_mapper,
                                       nullptr,  // No texture coordinates.
                                       nml_mapper);
  ofs.close();
}

}  // namespace

namespace examples {

void IndexGroupExample(const std::string& filename) {
  auto mesh = Mesh{// Positions.
                   std::vector<Vec3>{
                      Vec3{1.f, 1.f, 0.f}, 
                      Vec3{0.f, 1.f, 0.f},
                      Vec3{1.f, 0.f, 0.f}},
                   // Normals.
                   std::vector<Vec3>{
                      Vec3{0.f, 0.f, 1.f}, 
                      Vec3{0.f, 0.f, 1.f},
                      Vec3{0.f, 0.f, 1.f}},
                   std::vector<uint16_t>{0, 1, 2}, 
                   std::vector<uint16_t>{0, 1, 2}};

  WriteMesh(filename, mesh);
  const auto mesh2 = ReadMesh(filename);
}

}  // namespace examples

#if 0
void WriteObjFile(const std::string &file_name, const MyMesh &mesh) {
  auto ofs = std::ofstream(file_name.c_str());
  for (const auto& pos : mesh.positions) {
    ofs << "v " << pos.x << " " << pos.y << " " << pos.z << "\n";
  }
  for (const auto& tri : mesh.triangles) {
    ofs << "f " << tri.i0 << " " << tri.i1 << " " << tri.i2 << "\n";
  }
  ofs.close();
}


// struct-of-arrays
struct Mesh {
  std::vector<vec3f> positions;
  std::vector<vec2f> tex_coords;
  std::vector<vec3i> indices;
};

struct Vertex {
  vec3f position;
  vec2f tex_coord;
};

// array-of-structs
struct Mesh {
  std::vector<Vertex> positions;
  std::vector<vec3i> indices;
};


template <typename MeshT>
void WriteObjFile(const MeshT &mesh);

void WriteObjFile(const std::vector<vec3f> &positions,
                  const std::vector<vec2f> &tex_coords,
                  const std::vector<vec3i> &indices);

void WriteObjFile(const float* const positions, const std::uint32_t position_count,
                  const float* const tex_coords, const std::uint32_t tex_coord_count,
                  const int* const indices, const std::uint32_t index_count);
#endif
