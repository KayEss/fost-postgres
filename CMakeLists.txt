enable_testing()
if(NOT TARGET pgtest)
    add_custom_target(pgtest)
    set_property(TARGET pgtest PROPERTY EXCLUDE_FROM_ALL TRUE)
endif()

add_subdirectory(Cpp)
