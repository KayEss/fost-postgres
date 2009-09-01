/*
    Copyright 2009, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "fost-postgres-test.hpp"
#include <fost/db>
#include <fost/test>


using namespace fostlib;


FSL_TEST_SUITE( table_creation );


namespace {
    void check_create_table( const string &name, bool not_null ) {
        dbconnection dbc( read_dsn.value() + L" dbname=FSL_Test", write_dsn.value() + L" dbname=FSL_Test" );

        fostlib::meta_instance types( name );
        types
            .primary_key( L"id", L"integer" )
            .field( L"test_integer", L"integer", not_null )
            .field( L"test_varchar", L"varchar", not_null, 128 )
            .field( L"test_text", L"text", not_null )
            .field( L"test_boolean", L"boolean", not_null )
            .field( L"test_float", L"float", not_null )
            .field( L"test_date", L"date", not_null )
            .field( L"test_timestamp", L"timestamp", not_null )
        ;
        dbtransaction transaction( dbc );
        transaction.create_table( types );
        transaction.commit();
    }
}
FSL_TEST_FUNCTION( types ) {
    check_create_table( "types_not_null", true );
    check_create_table( "types_null", false );
}


FSL_TEST_FUNCTION( foreign_key ) {
    dbconnection dbc( read_dsn.value() + L" dbname=FSL_Test", write_dsn.value() + L" dbname=FSL_Test" );

    meta_instance simple( L"fk_simple" ), ref1("fk_ref1");
    /*
        Create a simple object definition
    */
    simple
        .primary_key(L"id", L"integer")
        .field(L"name", L"varchar", true, 10)
    ;
    // Add a non-nullable reference to a simple object
    ref1
        .primary_key(L"id", L"integer")
        .field(L"simple", simple, false)
    ;
    // Finally create the tables
    dbtransaction transaction( dbc );
    transaction
        .create_table( simple )
        .create_table( ref1 )
        .commit();
}

