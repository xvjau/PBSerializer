
PROTOBUF_GENERATE_CPP(PROTOBUF_SRCS PROTOBUF_HDRS message.proto)

include_directories(.. ${CMAKE_CURRENT_BINARY_DIR})

enable_testing()

add_executable(tests tests.cpp ${PROTOBUF_SRCS})

target_link_libraries(tests pbserializer pthread ${Boost_LIBRARIES} ${PROTOBUF_LIBRARY} gtest gtest_main)

add_test(
    NAME tests
    COMMAND tests
)