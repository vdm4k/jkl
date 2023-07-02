cmake_minimum_required(VERSION 3.14.0)

find_package(GTest "1.11.0" QUIET)
if (GTest_FOUND)
    return()
endif()

message(STATUS "couldn't find GTest in system. will download it")

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.12.0
)


set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(BUILD_GTEST ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)
