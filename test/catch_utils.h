// Copyright(C) 2018 Tommy Hinks <tommy.hinks@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#pragma once

#include <exception>
#include <sstream>
#include <string>

#include "catch2/catch.hpp"
#include "type_utils.h"

namespace utils {

struct ExceptionContentMatcher : Catch::MatcherBase<std::exception> {
  ExceptionContentMatcher(const std::string& target) : target_(target) {}

  bool match(const std::exception& matchee) const override {
    return matchee.what() == target_;
  }

  std::string describe() const override {
    auto oss = std::ostringstream{};
    oss << "exception message is: '" << target_ << "'";
    return oss.str();
  }

 private:
  std::string target_;
};

template <typename MeshT>
struct MeshMatcher : Catch::MatcherBase<MeshT> {
  using MeshType = MeshT;

  MeshMatcher(const MeshType& target, const bool match_tex_coords,
              const bool match_normals)
      : target_(target),
        match_tex_coords_(match_tex_coords),
        match_normals_(match_normals) {}

  bool match(const MeshType& matchee) const override {
    if (matchee.vertices.size() != target_.vertices.size() ||
        matchee.indices.size() != target_.indices.size()) {
      return false;
    }

    for (std::size_t i = 0; i < matchee.vertices.size(); ++i) {
      if (!Equals(matchee.vertices[i].pos, target_.vertices[i].pos)) {
        return false;
      }

      if (match_tex_coords_ &&
          !Equals(matchee.vertices[i].tex, target_.vertices[i].tex)) {
        return false;
      }

      if (match_normals_ &&
          !Equals(matchee.vertices[i].normal, target_.vertices[i].normal)) {
        return false;
      }
    }

    for (std::size_t i = 0; i < matchee.indices.size(); ++i) {
      if (matchee.indices[i] != target_.indices[i]) {
        return false;
      }
    }

    return true;
  }

  std::string describe() const override {
    auto oss = std::ostringstream{};
    oss << "mesh mismatch ("
        << "match_tex_coords: " << (match_tex_coords_ ? "true" : "false")
        << ", "
        << "match_normals: " << (match_normals_ ? "true" : "false") << ")";
    return oss.str();
  }

 private:
  MeshType target_;
  bool match_tex_coords_;
  bool match_normals_;
};

template <typename MeshT>
struct IndexGroupMeshMatcher : Catch::MatcherBase<MeshT> {
  using MeshType = MeshT;

  IndexGroupMeshMatcher(const MeshType& target, const bool match_tex_coords,
                        const bool match_normals)
      : target_(target),
        match_tex_coords_(match_tex_coords),
        match_normals_(match_normals) {}

  bool match(const MeshType& matchee) const override {
    // Positions.
    if (matchee.positions.size() != target_.positions.size() ||
        matchee.position_indices.size() != target_.position_indices.size()) {
      return false;
    }

    for (std::size_t i = 0; i < matchee.positions.size(); ++i) {
      if (!Equals(matchee.positions[i], target_.positions[i])) {
        return false;
      }
    }

    for (std::size_t i = 0; i < matchee.position_indices.size(); ++i) {
      if (matchee.position_indices[i] != target_.position_indices[i]) {
        return false;
      }
    }

    // Texture coordinates.
    if (match_tex_coords_) {
      if (matchee.tex_coords.size() != target_.tex_coords.size() ||
          matchee.tex_coord_indices.size() !=
              target_.tex_coord_indices.size()) {
        return false;
      }

      for (std::size_t i = 0; i < matchee.tex_coords.size(); ++i) {
        if (!Equals(matchee.tex_coords[i], target_.tex_coords[i])) {
          return false;
        }
      }

      for (std::size_t i = 0; i < matchee.tex_coord_indices.size(); ++i) {
        if (matchee.tex_coord_indices[i] != target_.tex_coord_indices[i]) {
          return false;
        }
      }
    }

    // Normals.
    if (match_normals_) {
      if (matchee.normals.size() != target_.normals.size() ||
          matchee.normal_indices.size() != target_.normal_indices.size()) {
        return false;
      }

      for (std::size_t i = 0; i < matchee.normals.size(); ++i) {
        if (!Equals(matchee.normals[i], target_.normals[i])) {
          return false;
        }
      }

      for (std::size_t i = 0; i < matchee.normal_indices.size(); ++i) {
        if (matchee.normal_indices[i] != target_.normal_indices[i]) {
          return false;
        }
      }
    }

    return true;
  }

  std::string describe() const override {
    auto oss = std::ostringstream{};
    oss << "indexed mesh mismatch ("
        << "match_tex_coords: " << (match_tex_coords_ ? "true" : "false")
        << ", "
        << "match_normals: " << (match_normals_ ? "true" : "false") << ")";
    return oss.str();
  }

 private:
  MeshType target_;
  bool match_tex_coords_;
  bool match_normals_;
};

}  // namespace utils
