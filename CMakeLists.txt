cmake_minimum_required(VERSION 3.14)
project(GymVisionLocalServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Поиск зависимостей
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)
find_package(Threads REQUIRED)
find_package(yaml-cpp REQUIRED)

# Протобуфы
file(GLOB PROTO_SRCS
        ${CMAKE_SOURCE_DIR}/proto/generated/*.pb.cc
        ${CMAKE_SOURCE_DIR}/proto/generated/*.grpc.pb.cc
)

add_library(proto_lib ${PROTO_SRCS})

target_include_directories(proto_lib PUBLIC ${CMAKE_SOURCE_DIR}/proto/generated)

target_link_libraries(proto_lib
        gRPC::grpc++
        protobuf::libprotobuf
)

# Основной исполняемый файл
file(GLOB_RECURSE SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp)

add_executable(GymVisionLocalServer ${SOURCES})
target_include_directories(GymVisionLocalServer PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(GymVisionLocalServer
        proto_lib
        yaml-cpp
        sqlite3
        Threads::Threads
)
