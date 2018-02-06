cmake_minimum_required(VERSION 2.8.11)

# Make PROJECT_SOURCE_DIR, PROJECT_BINARY_DIR, and PROJECT_NAME available.
set(PROJECT_NAME obj-io-test)
project(${PROJECT_NAME})

# set(CMAKE_CXX_FLAGS "-g -Wall")

# Force static runtime libraries for MSVC builds.
if(MSVC)
  set(variables 
    CMAKE_CXX_FLAGS_DEBUG
    CMAKE_CXX_FLAGS_RELEASE
	CMAKE_CXX_FLAGS_RELWITHDEBINFO
	CMAKE_CXX_FLAGS_MINSIZEREL
  )
  foreach(variable ${variables})
	if(${variable} MATCHES "/MD")
	  string(REGEX REPLACE "/MD" "/MT" ${variable} "${${variable}}")
	endif()
  endforeach()
endif()

# This adds another subdirectory, which has 'project(gtest)'.
add_subdirectory(gtest-1.7.0)

enable_testing()

# Include the gtest library. gtest_SOURCE_DIR is available due to
# 'project(gtest)' above.
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(obj-io-test
  common_channel_test.cc
  main.cc
  normal_channel_test.cc
  position_channel_test.cc
  read_test.cc
  tex_coord_channel_test.cc
  #utils.cc
  write_test.cc)

target_link_libraries(obj-io-test gtest gtest_main)

add_test(NAME obj-io-test COMMAND obj-io-test)
