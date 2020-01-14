add_library(fost-postgres
        connection.cpp
        recordset.cpp
        stored-procedure.cpp
    )
target_include_directories(fost-postgres
        PUBLIC
            $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/libpqxx/include>
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/libpqxx/include>
            ../include
    )
target_link_libraries(fost-postgres pqxx fost-core)
set_target_properties(fost-postgres PROPERTIES DEBUG_POSTFIX "-d")
install(TARGETS fost-postgres LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
install(DIRECTORY ../include/fost DESTINATION include)
