if(TARGET stress AND TARGET pgtest)
    add_library(fost-postgres-test STATIC EXCLUDE_FROM_ALL
            config.cpp
            pg.cpp
        )
    target_link_libraries(fost-postgres-test fost-postgres)
    stress_test(fost-postgres-test)
    add_dependencies(pgtest fost-postgres-test-check)
endif()
