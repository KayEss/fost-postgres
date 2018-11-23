if(TARGET pgtest)
    set_property(TARGET pgtest PROPERTY EXCLUDE_FROM_ALL TRUE)
endif()

add_subdirectory(Cpp)
