#cmake_minimum_required(VERSION 2.8.11)
cmake_minimum_required(VERSION 3.5)

# Make PROJECT_SOURCE_DIR, PROJECT_BINARY_DIR, and PROJECT_NAME available.
set(PROJECT_NAME obj-io-test)
project(${PROJECT_NAME})

#set(CMAKE_CXX_FLAGS "-g -Wall")

# Prevent overriding the parent project's compiler/linker
# settings on Windows.
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
#add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src
#                 ${CMAKE_BINARY_DIR}/googletest-build
#                 EXCLUDE_FROM_ALL)

add_subdirectory(test/googletest)

# The gtest/gtest_main targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
if (CMAKE_VERSION VERSION_LESS 2.8.11)
  include_directories("${gtest_SOURCE_DIR}/include")
endif()

enable_testing()

add_executable(obj-io-test
  test/common_channel_test.cc
  test/main.cc
  test/normal_channel_test.cc
  test/position_channel_test.cc
  test/read_test.cc
  test/tex_coord_channel_test.cc
  #utils.cc
  test/write_test.cc)
target_include_directories(obj-io-test
	PRIVATE "include")
target_link_libraries(obj-io-test 
	gtest_main)
add_test(NAME obj-io-test COMMAND obj-io-test)
