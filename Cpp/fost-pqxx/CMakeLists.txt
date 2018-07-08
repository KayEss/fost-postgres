add_library(fost-pqxx
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
target_compile_options(fost-pqxx PUBLIC -std=c++14)
target_include_directories(fost-pqxx
        PUBLIC ../../../PostgreSQL/pqxx/include/
        PRIVATE /usr/include/postgresql/
    )
target_link_libraries(fost-pqxx pq)
set_target_properties(fost-pqxx PROPERTIES DEBUG_POSTFIX "-d")
install(TARGETS fost-pqxx LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
