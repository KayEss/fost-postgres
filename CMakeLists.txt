cmake_minimum_required(VERSION 3.10)
project(fost-postgres)

if(TARGET pgtest)
    set_property(TARGET pgtest PROPERTY EXCLUDE_FROM_ALL TRUE)
endif()

add_subdirectory(Cpp)
