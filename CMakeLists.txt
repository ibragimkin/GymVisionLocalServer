cmake_minimum_required(VERSION 3.14)
project(GymVisionLocalServer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Поиск зависимостей ---
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)
find_package(Threads REQUIRED)
find_package(yaml-cpp REQUIRED)
# --- Пути ---
set(PROTO_DIR          ${CMAKE_SOURCE_DIR}/proto)
set(GENERATED_PROTO_DIR ${PROTO_DIR}/generated)
file(MAKE_DIRECTORY     ${GENERATED_PROTO_DIR})

# --- Список .proto ---
set(PROTO_FILES
        ${PROTO_DIR}/GymVisionLocal.proto
)

# --- Правила генерации из .proto ---
set(GENERATED_SRCS "")
foreach(PROTO_FILE ${PROTO_FILES})
    get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)

    # Прото сообщения
    add_custom_command(
            OUTPUT
            ${GENERATED_PROTO_DIR}/${PROTO_NAME}.pb.cc
            ${GENERATED_PROTO_DIR}/${PROTO_NAME}.pb.h
            COMMAND
            ${Protobuf_PROTOC_EXECUTABLE}
            ARGS
            --proto_path=${PROTO_DIR}
            --cpp_out=${GENERATED_PROTO_DIR}
            ${PROTO_FILE}
            DEPENDS
            ${PROTO_FILE}
    )

    # gRPC сервисы
    add_custom_command(
            OUTPUT
            ${GENERATED_PROTO_DIR}/${PROTO_NAME}.grpc.pb.cc
            ${GENERATED_PROTO_DIR}/${PROTO_NAME}.grpc.pb.h
            COMMAND
            ${Protobuf_PROTOC_EXECUTABLE}
            ARGS
            --proto_path=${PROTO_DIR}
            --grpc_out=${GENERATED_PROTO_DIR}
            --plugin=protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
            ${PROTO_FILE}
            DEPENDS
            ${PROTO_FILE}
    )

    list(APPEND GENERATED_SRCS
            ${GENERATED_PROTO_DIR}/${PROTO_NAME}.pb.cc
            ${GENERATED_PROTO_DIR}/${PROTO_NAME}.grpc.pb.cc
    )
endforeach()

# --- Собираем библиотеку из сгенерированных .cc ---
add_library(proto_lib ${GENERATED_SRCS})
target_include_directories(proto_lib PUBLIC ${GENERATED_PROTO_DIR})
target_link_libraries(proto_lib
        gRPC::grpc++
        protobuf::libprotobuf
)

# --- Основной исполняемый ---
file(GLOB_RECURSE SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp)

add_executable(GymVisionLocalServer ${SOURCES})
target_include_directories(GymVisionLocalServer PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/third_party
)
target_include_directories(GymVisionLocalServer PRIVATE ${Boost_INCLUDE_DIRS})

target_link_libraries(GymVisionLocalServer
        proto_lib
        yaml-cpp
        sqlite3
        Threads::Threads
        pthread
)
# --- CLI-приложение ---
add_executable(camera_cli cli/cli_main.cpp
        src/camera/ffmpeg_controller.cpp
        src/camera/http_controller.cpp
        src/camera/port_manager.cpp
                src/database/sqlitedb.cpp
        src/config/config.cpp
        src/logger/logger.cpp)

target_include_directories(camera_cli PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/third_party
)

target_link_libraries(camera_cli
        yaml-cpp
        sqlite3
        Threads::Threads
        pthread
)
