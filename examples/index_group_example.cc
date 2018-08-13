// Copyright(C) 2018 Tommy Hinks <tommy.hinks@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <index_group_example.h>

#include <cassert>
#include <fstream>
#include <vector>

#include <thinks/obj_io/obj_io.h>


namespace {

struct Vec3
{
  float x;
  float y;
  float z;
};

struct Mesh
{
  std::vector<Vec3> positions;
  std::vector<Vec3> normals;
  std::vector<std::uint16_t> position_indices;
  std::vector<std::uint16_t> normal_indices;
};


Mesh ReadMesh(const std::string& filename)
{
  using namespace std;

  auto mesh = Mesh{};

  // Positions.
  auto add_position = 
    thinks::obj_io::MakeAddFunc<thinks::obj_io::Position<float, 3>>(
      [&mesh](const auto& pos) {
        mesh.positions.push_back(
          Vec3{ pos.values[0], pos.values[1], pos.values[2] });
      });

  // Normals.
  auto add_normal = 
    thinks::obj_io::MakeAddFunc<thinks::obj_io::Normal<float>>(
      [&mesh](const auto& nml) {
        mesh.normals.push_back(
          Vec3{ nml.values[0], nml.values[1], nml.values[2] });
      });

  // Faces.
  typedef thinks::obj_io::TriangleFace<thinks::obj_io::IndexGroup<uint16_t>> ObjFaceType;
  auto add_face = thinks::obj_io::MakeAddFunc<ObjFaceType>(
    [&mesh](const auto& face) {
      mesh.position_indices.push_back(face.values[0].position_index.value);
      mesh.position_indices.push_back(face.values[1].position_index.value);
      mesh.position_indices.push_back(face.values[2].position_index.value);
      assert(face.values[0].normal_index.second && "index group must have normal");
      assert(face.values[1].normal_index.second && "index group must have normal");
      assert(face.values[2].normal_index.second && "index group must have normal");
      mesh.normal_indices.push_back(face.values[0].normal_index.first.value);
      mesh.normal_indices.push_back(face.values[1].normal_index.first.value);
      mesh.normal_indices.push_back(face.values[2].normal_index.first.value);
  });

  auto ifs = ifstream(filename);
  assert(ifs);
  const auto result = thinks::obj_io::Read(
    ifs,
    add_position,
    add_face,
    nullptr, // no texture coordinates.
    add_normal);
  ifs.close();

  return mesh;
}


void WriteMesh(const std::string& filename, const Mesh& mesh)
{
  using namespace std;

  // Positions.
  const auto pos_iend = end(mesh.positions);
  auto pos_iter = begin(mesh.positions);
  auto pos_mapper = [&pos_iter, pos_iend]() {
    typedef thinks::obj_io::Position<float, 3> ObjPositionType;

    if (pos_iter == pos_iend) {
      return thinks::obj_io::End<ObjPositionType>();
    }

    const auto pos = *pos_iter++;
    return thinks::obj_io::Map(ObjPositionType(pos.x, pos.y, pos.z));
  };

  // Faces.
  const auto pos_idx_iend = end(mesh.position_indices);
  const auto nml_idx_iend = end(mesh.normal_indices);
  auto pos_idx_iter = begin(mesh.position_indices);
  auto nml_idx_iter = begin(mesh.normal_indices);
  auto face_mapper = [&pos_idx_iter, &nml_idx_iter, pos_idx_iend, nml_idx_iend]() {
    typedef thinks::obj_io::IndexGroup<uint16_t> ObjIndexType;
    typedef thinks::obj_io::TriangleFace<ObjIndexType> ObjFaceType;

    if (distance(pos_idx_iter, pos_idx_iend) < 3 || 
        distance(nml_idx_iter, nml_idx_iend) < 3) {
      return thinks::obj_io::End<ObjFaceType>();
    }

    const auto pos_idx0 = *pos_idx_iter++;
    const auto pos_idx1 = *pos_idx_iter++;
    const auto pos_idx2 = *pos_idx_iter++;
    const auto nml_idx0 = *nml_idx_iter++;
    const auto nml_idx1 = *nml_idx_iter++;
    const auto nml_idx2 = *nml_idx_iter++;

    // No texture coordinates.
    return thinks::obj_io::Map(ObjFaceType(
      ObjIndexType(pos_idx0, make_pair(0, false), make_pair(nml_idx0, true)),
      ObjIndexType(pos_idx1, make_pair(0, false), make_pair(nml_idx1, true)),
      ObjIndexType(pos_idx2, make_pair(0, false), make_pair(nml_idx2, true))));
  };

  // Normals.
  const auto nml_iend = end(mesh.normals);
  auto nml_iter = begin(mesh.normals);
  auto nml_mapper = [&nml_iter, nml_iend]() {
    typedef thinks::obj_io::Normal<float> ObjNormalType;

    if (nml_iter == nml_iend) {
      return thinks::obj_io::End<ObjNormalType>();
    }

    const auto nml = *nml_iter++;
    return thinks::obj_io::Map(ObjNormalType(nml.x, nml.y, nml.z));
  };

  auto ofs = ofstream(filename);
  assert(ofs);
  const auto result = thinks::obj_io::Write(
    ofs,
    pos_mapper,
    face_mapper,
    nullptr, // No texture coordinates.
    nml_mapper);
  ofs.close();
}

} // namespace

namespace examples {

void IndexGroupExample(const std::string& filename)
{
  using namespace std;

  auto mesh = Mesh{
    // Positions.
    vector<Vec3>{
      Vec3{ 1.f, 1.f, 0.f },
      Vec3{ 0.f, 1.f, 0.f },
      Vec3{ 1.f, 0.f, 0.f }
    },
    // Normals.
    vector<Vec3>{
      Vec3{ 0.f, 0.f, 1.f },
      Vec3{ 0.f, 0.f, 1.f },
      Vec3{ 0.f, 0.f, 1.f }
    },
    vector<uint16_t>{ 0, 1, 2 },
    vector<uint16_t>{ 0, 1, 2 }
  };

  WriteMesh(filename, mesh);
  const auto mesh2 = ReadMesh(filename);
}

} // namespace examples
