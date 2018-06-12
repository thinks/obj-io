# Copyright (C) 2018 Tommy Hinks <tommy.hinks@gmail.com>
# This file is subject to the license terms in the LICENSE file
# found in the top-level directory of this distribution.

#if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/catch.hpp)
#    file(DOWNLOAD
#         https://raw.githubusercontent.com/philsquared/Catch/master/single_include/catch.hpp
#         ${CMAKE_CURRENT_BINARY_DIR}/catch.hpp
#         STATUS status
#         LOG log)
#
#    list(GET status 0 status_code)
#    list(GET status 1 status_string)
#
#    if(NOT status_code EQUAL 0)
#        message(FATAL_ERROR "error downloading catch: ${status_string}"
#                "${log}")
#    endif()
#endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows.
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
#add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src
#                 ${CMAKE_BINARY_DIR}/googletest-build
#                 EXCLUDE_FROM_ALL)

add_subdirectory(googletest)

set(tests
	#common_channel_test.cc
	#normal_channel_test.cc
	write_interleaved_test.cc
	#position_channel_test.cc
	#read_test.cc
	#tex_coord_channel_test.cc
	#utils.cc
	#write_test.cc
)

add_executable(thinks_obj_io_test
	main.cc
	${tests})
#target_include_directories(thinks_obj_io_test SYSTEM PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
#target_include_directories(thinks_obj_io_test
#	PRIVATE "include")
target_link_libraries(thinks_obj_io_test PUBLIC 
	thinks_obj_io
	gtest_main)
set_target_properties(thinks_obj_io_test PROPERTIES CXX_STANDARD 11)

add_test(NAME test COMMAND thinks_obj_io_test)
