cmake_minimum_required(VERSION 3.10)
project(eq_plagin_test VERSION 0.1.0)

enable_testing()

add_executable(${PROJECT_NAME}
    source/AudioProcessorTest.cpp
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${GOOGLETEST_SOURCE_DIR}/googletest/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../my_plagin/include
        ${JUCE_SOURCE_DIR}/modules

)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        eq_plagin
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})
