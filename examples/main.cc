// Copyright(C) 2018 Tommy Hinks <tommy.hinks@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <iostream>

#include <index_group_example.h>
#include <polygon_example.h>
#include <simple_example.h>

int main(int argc, char* argv[])
{
  examples::IndexGroupExample("./index_group_example.obj");
  examples::PolygonExample("./polygon_example.obj");
  examples::SimpleExample("./simple_example.obj");

  return 0;
}
