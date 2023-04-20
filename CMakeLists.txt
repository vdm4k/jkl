cmake_minimum_required(VERSION 3.14.0)
project(system VERSION 0.9.1 DESCRIPTION "system library")

set(H_FILES
    include/system/result.h
    include/system/thread/config.h
    include/system/thread/statistic.h
    include/system/thread/thread.h
    include/system/time.h
)

set(CPP_FILES
    source/thread/thread.cpp
    source/time.cpp
)

find_package(Threads REQUIRED)
add_library(${PROJECT_NAME} ${CPP_FILES} ${H_FILES})
add_library(bro::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
add_library(system::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
target_compile_options(${PROJECT_NAME} PRIVATE  -Wall -Wextra -Wpedantic -Wno-vla -Wno-unused-local-typedefs)

target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC
    $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/${PROJECT_NAME}>)

#sanitizer
option(WITH_SANITIZER "Enable address sanitizer" OFF)

if(WITH_SANITIZER)
    target_compile_options(${PROJECT_NAME} INTERFACE -fsanitize=address -fPIC)
    target_link_options(${PROJECT_NAME} INTERFACE -fsanitize=address)
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC Threads::Threads)

#examples
option(WITH_TESTS "Build tests" OFF)
if(WITH_TESTS)
    add_subdirectory(tests)
endif()
