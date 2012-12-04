PROTOBUF_GENERATE_CPP(PROTOBUF_SRCS PROTOBUF_HDRS message.proto)

include_directories(.. ${CMAKE_CURRENT_BINARY_DIR})

add_executable(example main.cpp ${PROTOBUF_SRCS})

target_link_libraries(example pbserializer pthread ${Boost_LIBRARIES} ${PROTOBUF_LIBRARY})

