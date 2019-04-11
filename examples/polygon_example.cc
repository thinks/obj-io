// Copyright(C) 2018 Tommy Hinks <tommy.hinks@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include "polygon_example.h"

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

struct PolygonFace {
  std::vector<std::uint16_t> indices;
};

struct Mesh {
  std::vector<Vec3> positions;
  std::vector<PolygonFace> faces;
};

Mesh ReadMesh(const std::string& filename) {
  auto mesh = Mesh{};

  // Positions.
  auto add_position = thinks::obj_io::MakeAddFunc<
      thinks::obj_io::Position<float, 3>>([&mesh](const auto& pos) {
    mesh.positions.push_back(Vec3{pos.values[0], pos.values[1], pos.values[2]});
  });

  // Faces.
  using ObjFaceType =
      thinks::obj_io::PolygonFace<thinks::obj_io::Index<uint16_t>>;
  auto add_face =
      thinks::obj_io::MakeAddFunc<ObjFaceType>([&mesh](const auto& face) {
        auto polygon = PolygonFace{};
        for (const auto index : face.values) {
          polygon.indices.push_back(index.value);
        }
        mesh.faces.push_back(polygon);
      });

  auto ifs = std::ifstream(filename);
  assert(ifs);
  const auto result = thinks::obj_io::Read(ifs, add_position, add_face);
  ifs.close();

  return mesh;
}

void WriteMesh(const std::string& filename, const Mesh& mesh) {
  // Positions.
  const auto pos_iend = std::end(mesh.positions);
  auto pos_iter = std::begin(mesh.positions);
  auto pos_mapper = [&pos_iter, pos_iend]() {
    using ObjPositionType = thinks::obj_io::Position<float, 3>;

    if (pos_iter == pos_iend) {
      return thinks::obj_io::End<ObjPositionType>();
    }

    const auto pos = *pos_iter++;
    return thinks::obj_io::Map(ObjPositionType(pos.x, pos.y, pos.z));
  };

  // Faces.
  const auto face_iend = std::end(mesh.faces);
  auto face_iter = std::begin(mesh.faces);
  auto face_mapper = [&face_iter, face_iend]() {
    using ObjIndexType = thinks::obj_io::IndexGroup<uint16_t>;
    using ObjFaceType = thinks::obj_io::PolygonFace<ObjIndexType>;

    if (face_iter == face_iend) {
      return thinks::obj_io::End<ObjFaceType>();
    }

    auto face = *face_iter++;
    auto obj_face = ObjFaceType{};
    for (const auto index : face.indices) {
      obj_face.values.push_back(ObjIndexType(index));
    }

    return thinks::obj_io::Map(obj_face);
  };

  auto ofs = std::ofstream(filename);
  assert(ofs);
  const auto result = thinks::obj_io::Write(ofs, pos_mapper, face_mapper);
  ofs.close();
}

}  // namespace

namespace examples {

void PolygonExample(const std::string& filename) {
  auto mesh =
      Mesh{
          // Positions.
          std::vector<Vec3>{
              Vec3{1.f, 1.f, 0.f}, 
              Vec3{0.f, 1.f, 0.f},
              Vec3{1.f, 0.f, 0.f}, 
              Vec3{0.f, 0.f, 0.f}},
           // Faces.
           std::vector<PolygonFace>{
              PolygonFace{
                  std::vector<uint16_t>{0, 1, 2, 3}},
              PolygonFace{
                  std::vector<uint16_t>{0, 1, 2}}}};

  WriteMesh(filename, mesh);
  const auto mesh2 = ReadMesh(filename);
}

}  // namespace examples
