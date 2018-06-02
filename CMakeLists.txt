cmake_minimum_required(VERSION 2.8.11)

# Download and unpack googletest at configure time
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src
                 ${CMAKE_BINARY_DIR}/googletest-build
                 EXCLUDE_FROM_ALL)

#target_include_directories(gmock_main SYSTEM BEFORE INTERFACE
#  "${gtest_SOURCE_DIR}/include" "${gmock_SOURCE_DIR}/include")
target_include_directories(gtest_main SYSTEM BEFORE INTERFACE
    "${gtest_SOURCE_DIR}/include")

add_custom_target(
    lint
    COMMAND clang-format
    -i
    */*.hh */*.cc)

project(reindex)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(./)
if (APPLE)
    include_directories(./Pods/libc++/)
endif()

add_executable(reindex.test tests/test_reindex.cc)
target_link_libraries(reindex.test gtest_main)

include(CTest)

#does the application run
add_test(NAME reindex_test_run COMMAND reindex.test)
