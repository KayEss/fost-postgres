add_library(fost-postgres
        connection.cpp
        recordset.cpp
        stored-procedure.cpp
    )
target_include_directories(fost-postgres PUBLIC ../include)
target_link_libraries(fost-postgres fost-core fost-pqxx)
install(TARGETS fost-postgres LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
