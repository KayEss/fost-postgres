## If this fails you may need more packages. Although libpqxx only needs
## `libpq-dev` the cmake module also looks for `postgresql-server-dev-all`
find_package(PostgreSQL REQUIRED)

add_library(fost-pqxx STATIC
        ../../../PostgreSQL/pqxx/src/array.cxx
        ../../../PostgreSQL/pqxx/src/binarystring.cxx
        ../../../PostgreSQL/pqxx/src/connection_base.cxx
        ../../../PostgreSQL/pqxx/src/connection.cxx
        ../../../PostgreSQL/pqxx/src/cursor.cxx
        ../../../PostgreSQL/pqxx/src/dbtransaction.cxx
        ../../../PostgreSQL/pqxx/src/errorhandler.cxx
        ../../../PostgreSQL/pqxx/src/except.cxx
        ../../../PostgreSQL/pqxx/src/field.cxx
        ../../../PostgreSQL/pqxx/src/largeobject.cxx
        ../../../PostgreSQL/pqxx/src/nontransaction.cxx
        ../../../PostgreSQL/pqxx/src/notification.cxx
        ../../../PostgreSQL/pqxx/src/pipeline.cxx
        ../../../PostgreSQL/pqxx/src/prepared_statement.cxx
        ../../../PostgreSQL/pqxx/src/result.cxx
        ../../../PostgreSQL/pqxx/src/robusttransaction.cxx
        ../../../PostgreSQL/pqxx/src/row.cxx
        ../../../PostgreSQL/pqxx/src/statement_parameters.cxx
        ../../../PostgreSQL/pqxx/src/strconv.cxx
        ../../../PostgreSQL/pqxx/src/subtransaction.cxx
        ../../../PostgreSQL/pqxx/src/tablereader.cxx
        ../../../PostgreSQL/pqxx/src/tablestream.cxx
        ../../../PostgreSQL/pqxx/src/tablewriter.cxx
        ../../../PostgreSQL/pqxx/src/transaction_base.cxx
        ../../../PostgreSQL/pqxx/src/transaction.cxx
        ../../../PostgreSQL/pqxx/src/util.cxx
        ../../../PostgreSQL/pqxx/src/version.cxx
    )
target_include_directories(fost-pqxx BEFORE
        PUBLIC ../../../PostgreSQL/pqxx/include/
    )
target_include_directories(fost-pqxx
        PRIVATE ${PostgreSQL_INCLUDE_DIRS}
    )
if(CMAKE_VERSION VERSION_LESS "3.8")
    message(WARNING "This verison of cmake is too old to request C++17 for fost-pqxx")
    target_compile_options(fost-pqxx PUBLIC -std=c++17)
else()
    target_compile_features(fost-pqxx PUBLIC cxx_std_17)
endif()

target_link_libraries(fost-pqxx ${PostgreSQL_LIBRARIES})
set_target_properties(fost-pqxx PROPERTIES DEBUG_POSTFIX "-d")
install(TARGETS fost-pqxx LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)

