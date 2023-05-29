cmake_minimum_required(VERSION 3.0)
project(RedisClientWrapper)

set(CMAKE_CXX_STANDARD 11)

if (WIN32)
    add_definitions(-DREDIS_CLIENT_WRAPPER_SHARED)
endif ()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_PDB_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)

add_library(${PROJECT_NAME} SHARED redisclientwrapper.cpp redisclientwrapper.h)

# this is heuristically generated, and may not be correct
find_package(redis++ CONFIG REQUIRED)
if (WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE redis++::redis++)
else ()
    target_link_libraries(${PROJECT_NAME} PRIVATE redis++::redis++_static)
endif ()

option(BUILD_TEST "Whether or not to build the tests" OFF)
if (${BUILD_TEST})
    enable_testing()

    add_subdirectory(test)
endif ()
