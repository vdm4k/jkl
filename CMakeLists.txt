cmake_minimum_required(VERSION 3.1.0)
project(jkl VERSION 0.0.2 DESCRIPTION "jkl library")

option(WITH_SYSTEM "Build application" ON)
if(WITH_SYSTEM)
    add_subdirectory(system)
endif()
