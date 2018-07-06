// Copyright(C) 2018 Tommy Hinks <tommy.hinks@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <thinks/obj_io/obj_io.h>

#include <vector>


struct Vec2f
{
  float x;
  float y;
};

struct Vec3f
{
  float x;
  float y;
  float z;
};

struct Vertex
{
  Vec3f pos;
  Vec2f tex;
  Vec3f normal;
  Vec3f color; // Attribute that is not supported by OBJ format.
};

struct Mesh
{
  std::vector<Vertex> vertices;
  std::vector<std::uint32_t> tri_indices;
};

struct IndexedMesh
{
  std::vector<Vec3f> positions;
  std::vector<Vec2f> tex_coords;
  std::vector<Vec3f> normals;
  std::vector<std::uint32_t> position_indices;
  std::vector<std::uint32_t> tex_coord_indices;
  std::vector<std::uint32_t> normal_indices;
};

typedef thinks::obj_io::Position<float, 3> Position3_f;
typedef thinks::obj_io::Position<float, 4> Position4_f;
typedef thinks::obj_io::TexCoord<float, 2> TexCoord2_f;
typedef thinks::obj_io::Normal<float> Normal_f;
typedef thinks::obj_io::Index<std::uint32_t> Index_ui32;
typedef thinks::obj_io::IndexGroup<std::uint32_t> IndexGroup_ui32;
typedef thinks::obj_io::TriangleFace<Index_ui32> TriFace_ui32;
typedef thinks::obj_io::TriangleFace<IndexGroup_ui32> TriFaceGroup_ui32;
