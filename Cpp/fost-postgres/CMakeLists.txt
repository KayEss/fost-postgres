add_library(fost-postgres
        connection.cpp
        recordset.cpp
        stored-procedure.cpp
    )
target_include_directories(fost-postgres PUBLIC ../include)
target_link_libraries(fost-postgres pqxx_shared fost-core)
set_target_properties(fost-postgres PROPERTIES DEBUG_POSTFIX "-d")
install(TARGETS fost-postgres LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
