cmake_minimum_required(VERSION 3.14)
project(GymVisionLocalServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Указание корневого include/ как пути для заголовков
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/proto)

find_package(yaml-cpp REQUIRED)
find_package(Threads REQUIRED)
# find_package(Protobuf REQUIRED)
# find_package(gRPC REQUIRED)

# Поиск всех исходников
file(GLOB_RECURSE SOURCES
        ${CMAKE_SOURCE_DIR}/src/*.cpp
)

add_executable(GymVisionLocalServer ${SOURCES})

target_link_libraries(GymVisionLocalServer
        yaml-cpp
        sqlite3
        pthread
         grpc++
         protobuf
)
