cmake_minimum_required(VERSION 2.8)

project(pbserializer)

find_package(Protobuf REQUIRED)
find_package(Boost COMPONENTS property_tree OPTIONAL)

PROTOBUF_GENERATE_CPP(PROTOBUF_SRCS PROTOBUF_HDRS message.proto)

add_definitions(-Wall -Wextra)

include_directories(.. ${CMAKE_CURRENT_BINARY_DIR} ${Boost_INCLUDE_DIRS} ${PROTOBUF_INCLUDE_DIR})

enable_testing()

add_executable(tests ../pbserializer.cpp tests.cpp ${PROTOBUF_SRCS})

target_link_libraries(tests pthread ${Boost_LIBRARIES} ${PROTOBUF_LIBRARY} gtest gtest_main)

add_test(
    NAME tests
    COMMAND tests
)