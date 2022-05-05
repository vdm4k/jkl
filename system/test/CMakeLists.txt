cmake_minimum_required(VERSION 3.3.2)
project(system_test VERSION 1.0.0 DESCRIPTION "network stack test library" LANGUAGES CXX)

if(UNIX AND NOT APPLE)
    set(COMMON_OS_SPECIFIC_OPTIONS "-g3;-ggdb3")
endif()

set(COMMON_OPTIONS ${COMMON_OS_SPECIFIC_OPTIONS})
set(DEBUG_OPTIONS "-O0;${COMMON_OPTIONS}")
set(RELEASE_OPTIONS "-O2;-DNDEBUG;${COMMON_OPTIONS}")

find_package(GTest REQUIRED)
find_package(system REQUIRED)

file(GLOB_RECURSE CPP_FILES ${${PROJECT_NAME}_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE H_FILES   ${${PROJECT_NAME}_SOURCE_DIR}/*.h)

add_executable(${PROJECT_NAME} ${CPP_FILES} ${H_FILES})

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
target_compile_options(${PROJECT_NAME} PUBLIC "-Wall;-Wextra"
    PRIVATE "$<$<CONFIG:DEBUG>:${DEBUG_OPTIONS}>"
    PRIVATE "$<$<CONFIG:RELEASE>:${RELEASE_OPTIONS}>")

target_include_directories(${PROJECT_NAME}
    PUBLIC
    $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/${PROJECT_NAME}>
)

if(NOT DEFINED SANITIZER)
    option(SANITIZER "Sanitizer" OFF)
endif()

set_property(TARGET ${PROJECT_NAME} PROPERTY INTERFACE_LINK_LIBRARIES
    -Wl,--whole-archive,-pthread,--no-whole-archive)

if(SANITIZER)
    target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:-fsanitize=address>)
    target_link_libraries(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:-fsanitize=address>
        system::system GTest::gtest GTest::gtest_main ${CMAKE_THREAD_LIBS_INIT})
else()
    target_link_libraries(${PROJECT_NAME}
        PUBLIC
        system::system GTest::gtest GTest::gtest_main ${CMAKE_THREAD_LIBS_INIT})
endif()

enable_testing()
add_test(${PROJECT_NAME} ${PROJECT_NAME})
