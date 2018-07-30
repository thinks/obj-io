// Copyright(C) 2018 Tommy Hinks <tommy.hinks@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef THINKS_OBJ_IO_SIMPLE_EXAMPLE_H_INCLUDED
#define THINKS_OBJ_IO_SIMPLE_EXAMPLE_H_INCLUDED

#include <cstdint>
#include <string>
#include <vector>


namespace examples {

struct Vec2
{
  float x;
  float y;
};

struct Vec3
{
  float x;
  float y;
  float z;
};

struct Vertex
{
  Vec3 position;
  Vec2 tex_coord;
  Vec3 normal;
};

struct Mesh
{
  std::vector<Vertex> vertices;
  std::vector<std::uint16_t> indices;
};


void WriteMesh(const std::string& filename, const Mesh& mesh);
Mesh ReadMesh(const std::string& filename);


void SimpleExample();

} // namespace examples

#endif // THINKS_OBJ_IO_SIMPLE_EXAMPLE_H_INCLUDED