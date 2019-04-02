# Copyright (C) 2018 Tommy Hinks <tommy.hinks@gmail.com>
# This file is subject to the license terms in the LICENSE file
# found in the top-level directory of this distribution.

cmake_minimum_required(VERSION 3.1)
project(obj_io)

set(header_files
	${CMAKE_CURRENT_SOURCE_DIR}/include/thinks/obj_io/obj_io.h
)
add_library(thinks_obj_io INTERFACE)
target_sources(thinks_obj_io INTERFACE ${header_files})
target_include_directories(thinks_obj_io INTERFACE include)

string(TOLOWER "${CMAKE_CURRENT_SOURCE_DIR}" current_source_dir_lower)
string(TOLOWER "${CMAKE_SOURCE_DIR}" source_dir_lower)
if(current_source_dir_lower STREQUAL source_dir_lower)
    message(STATUS "obj-io: enable testing")
    enable_testing()
    add_subdirectory(test)
	add_subdirectory(examples)
endif()