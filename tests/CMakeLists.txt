include(FetchContent)
FetchContent_Declare(
    GoogleTest
    URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(GoogleTest)
file(GLOB TESTS "*.cpp")

add_executable(unit_tests
    ${TESTS}
)

target_link_libraries(unit_tests
    network-c-lib
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(unit_tests)