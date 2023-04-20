cmake_minimum_required(VERSION 3.3.2)
project(system_test VERSION 1.0.0 DESCRIPTION "system library tests" LANGUAGES CXX)

enable_testing()
find_package(GTest REQUIRED)

file(GLOB_RECURSE CPP_FILES ${${PROJECT_NAME}_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE H_FILES   ${${PROJECT_NAME}_SOURCE_DIR}/*.h)

add_executable(${PROJECT_NAME} ${CPP_FILES} ${H_FILES})

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
target_compile_options(${PROJECT_NAME} PUBLIC "-Wall;-Wextra"
    PRIVATE "$<$<CONFIG:DEBUG>:${DEBUG_OPTIONS}>"
    PRIVATE "$<$<CONFIG:RELEASE>:${RELEASE_OPTIONS}>")

#sanitizer
option(WITH_SANITIZER "Enable address sanitizer" OFF)

if(WITH_SANITIZER)
    target_compile_options(${PROJECT_NAME} INTERFACE -fsanitize=address -fPIC)
    target_link_options(${PROJECT_NAME} INTERFACE -fsanitize=address)
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC system::system GTest::gtest GTest::gtest_main)

gtest_discover_tests(${PROJECT_NAME})
