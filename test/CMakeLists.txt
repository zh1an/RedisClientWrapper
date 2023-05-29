cmake_minimum_required(VERSION 3.0)

project(RCWTest)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../lib)
set(CMAKE_PDB_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../lib)

add_executable(${PROJECT_NAME} main.cpp)

find_library(RCWLIB NAMES RedisClientWrapper PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../lib)
message(STATUS ${RCWLIB})

find_package(GTest CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE GTest::gmock GTest::gtest GTest::gmock_main GTest::gtest_main ${RCWLIB})

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})