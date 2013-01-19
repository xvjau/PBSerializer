cmake_minimum_required(VERSION 2.8)

project(pbserializer)

find_package(Protobuf REQUIRED)

add_definitions(-Wall -Wextra -ftrack-macro-expansion=2)

include_directories(${PROTOBUF_INCLUDE_DIR})

add_library(pbserializer pbserializer.cpp)
add_definitions(-fPIC)
add_subdirectory(example)
add_subdirectory(tests)

install(TARGETS pbserializer DESTINATION lib)
install(FILES pbserializer.h DESTINATION include)
