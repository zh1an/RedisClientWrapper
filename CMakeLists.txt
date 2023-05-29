cmake_minimum_required(VERSION 3.0)
project(RedisClientWrapper)

set(CMAKE_CXX_STANDARD 11)

if (WIN32)
    add_definitions(-DREDIS_CLIENT_WRAPPER_SHARED)
endif ()

add_library(${PROJECT_NAME} SHARED redisclientwrapper.cpp redisclientwrapper.h)

# this is heuristically generated, and may not be correct
find_package(redis++ CONFIG REQUIRED)
if (WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE redis++::redis++)
else ()
    target_link_libraries(${PROJECT_NAME} PRIVATE redis++::redis++_static)
endif ()
