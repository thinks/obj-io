# Copyright (C) 2018 Tommy Hinks <tommy.hinks@gmail.com>
# This file is subject to the license terms in the LICENSE file
# found in the top-level directory of this distribution.

set(tests
    write_test.cc
    read_test.cc
    round_trip_test.cc)

add_executable(thinks_obj_io_test
    catch_main.cc
    ${tests})
target_include_directories(thinks_obj_io_test SYSTEM PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(thinks_obj_io_test 
    PRIVATE 
        thinks::obj_io
        Catch2::Catch2)
set_target_properties(thinks_obj_io_test PROPERTIES CXX_STANDARD 11)

add_test(NAME test COMMAND thinks_obj_io_test)
