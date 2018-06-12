// Copyright(C) 2018 Tommy Hinks <tommy.hinks@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top - level directory of this distribution.

#include <iostream>
#include <gtest/gtest.h>

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  //::testing::GTEST_FLAG(list_tests) = true;
  //::testing::GTEST_FLAG(filter) = "TestTest.*";
  const auto r = RUN_ALL_TESTS();
  std::cin.get();
  return r;
}
