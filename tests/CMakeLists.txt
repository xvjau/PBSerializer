find_library(LIBPTHREAD NAMES pthread)
find_library(LIBGTEST NAMES gtest)
find_library(LIBGTEST_MAIN NAMES gtest_main)

set(LIBPBSERIALIZER ${CMAKE_CURRENT_BINARY_DIR}/../libpbserializer.a)

PROTOBUF_GENERATE_CPP(PROTOBUF_SRCS PROTOBUF_HDRS message.proto)

include_directories(.. ${CMAKE_CURRENT_BINARY_DIR})

enable_testing()

add_executable(tests tests.cpp ${PROTOBUF_SRCS})

target_link_libraries(tests ${LIBPBSERIALIZER} ${LIBPTHREAD} ${PROTOBUF_LIBRARY} ${LIBGTEST} ${LIBGTEST_MAIN})

add_custom_command(TARGET tests COMMAND cp ${CMAKE_SOURCE_DIR}/tests/example.json ${CMAKE_BINARY_DIR}/tests)

add_test(
    NAME tests
    COMMAND tests
)
