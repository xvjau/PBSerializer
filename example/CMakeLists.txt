cmake_minimum_required(VERSION 2.8)

project(pbserializer)

find_package(Protobuf REQUIRED)
find_package(CheckForCPP11 HINTS $ENV{MISC_CMAKE_MODULES})
find_package(Boost COMPONENTS property_tree OPTIONAL)

if ( CPP11 )
    message(STATUS "Using C++ 11")
    add_definitions(${CPP11_DEFS})
else()
    message(FATAL_ERROR "Need a C++ 11 compiler!")
endif()

PROTOBUF_GENERATE_CPP(PROTOBUF_SRCS PROTOBUF_HDRS
    message.proto
)

add_definitions(-Wall -Wextra)

include_directories(.. ${CMAKE_CURRENT_BINARY_DIR} ${Boost_INCLUDE_DIRS} ${PROTOBUF_INCLUDE_DIR})

add_executable(pbserializer main.cpp ${PROTOBUF_SRCS})

target_link_libraries(pbserializer ${Boost_LIBRARIES} ${PROTOBUF_LIBRARY})

