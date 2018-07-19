// Copyright(C) 2018 Tommy Hinks <tommy.hinks@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef THINKS_OBJ_IO_UTILS_CATCH_UTILS_H_INCLUDED
#define THINKS_OBJ_IO_UTILS_CATCH_UTILS_H_INCLUDED

#include <exception>
#include <string>

#include <catch.hpp>

#include <utils/type_utils.h>

namespace utils {

struct ExceptionContentMatcher : Catch::MatcherBase<std::exception> 
{
  ExceptionContentMatcher(const std::string& target)
    : target_(target)
  {
  }

  bool match(const std::exception& matchee) const override
  {
    return matchee.what() == target_;
  }

  std::string describe() const override
  {
    return "exception message is:\"" + target_ + "\"";
  }

private:
  std::string target_;
};

template<typename MeshT>
struct MeshMatcher : Catch::MatcherBase<MeshT>
{
  typedef MeshT MeshType;

  MeshMatcher(const MeshType& target)
    : target_(target)
  {
  }

  bool match(const MeshType& matchee) const override
  {
    if (matchee.vertices.size() != target_.vertices.size() ||
        matchee.indices.size() != target_.indices.size()) {
      return false;
    }

    for (auto i = std::size_t{ 0 }; i < matchee.vertices.size(); ++i) {
      if (!Equals(matchee.vertices[i].pos, target_.vertices[i].pos) ||
          !Equals(matchee.vertices[i].tex, target_.vertices[i].tex) ||
          !Equals(matchee.vertices[i].normal, target_.vertices[i].normal)) {
        return false;
      }
    }

    for (auto i = std::size_t{ 0 }; i < matchee.indices.size(); ++i) {
      if (matchee.indices[i] != target_.indices[i]) {
        return false;
      }
    }

    return true;
  }

  std::string describe() const override
  {
    return "mesh mismatch";
  }

private:
  MeshType target_;
};

template<typename MeshT>
struct IndexedMeshMatcher : Catch::MatcherBase<MeshT>
{
  typedef MeshT MeshType;

  IndexedMeshMatcher(const MeshType& target)
    : target_(target)
  {
  }

  bool match(const MeshType& matchee) const override
  {
    if (matchee.positions.size() != target_.positions.size() ||
        matchee.tex_coords.size() != target_.tex_coords.size() ||
        matchee.normals.size() != target_.normals.size() ||
        matchee.position_indices.size() != target_.position_indices.size() ||
        matchee.tex_coord_indices.size() != target_.tex_coord_indices.size() ||
        matchee.normal_indices.size() != target_.normal_indices.size()) {
      return false;
    }

    for (auto i = std::size_t{ 0 }; i < matchee.positions.size(); ++i) {
      if (!Equals(matchee.positions[i], target_.positions[i])) {
        return false;
      }
    }

    for (auto i = std::size_t{ 0 }; i < matchee.tex_coords.size(); ++i) {
      if (!Equals(matchee.tex_coords[i], target_.tex_coords[i])) {
        return false;
      }
    }

    for (auto i = std::size_t{ 0 }; i < matchee.normals.size(); ++i) {
      if (!Equals(matchee.normals[i], target_.normals[i])) {
        return false;
      }
    }

    for (auto i = std::size_t{ 0 }; i < matchee.position_indices.size(); ++i) {
      if (matchee.position_indices[i] != target_.position_indices[i]) {
        return false;
      }
    }

    for (auto i = std::size_t{ 0 }; i < matchee.tex_coord_indices.size(); ++i) {
      if (matchee.tex_coord_indices[i] != target_.tex_coord_indices[i]) {
        return false;
      }
    }

    for (auto i = std::size_t{ 0 }; i < matchee.normal_indices.size(); ++i) {
      if (matchee.normal_indices[i] != target_.normal_indices[i]) {
        return false;
      }
    }

    return true;
  }

  std::string describe() const override
  {
    return "indexed mesh mismatch";
  }

private:
  MeshType target_;
};

} // namespace utils

#endif // THINKS_OBJ_IO_UTILS_CATCH_UTILS_H_INCLUDED
