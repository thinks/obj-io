// Copyright(C) 2018 Tommy Hinks <tommy.hinks@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <exception>
#include <string>

#include <catch.hpp>

namespace utils {

struct ExceptionContentMatcher : Catch::MatcherBase<std::exception> {
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

} // namespace utils
