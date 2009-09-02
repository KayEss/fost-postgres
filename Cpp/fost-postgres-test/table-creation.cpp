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

    /*
        Create a simple object definition and a multi-key one
    */
    meta_instance simple( L"fk_simple" ), multi_pk( "fk_multi" );
    simple
        .primary_key(L"id", L"integer")
        .field(L"name", L"varchar", true, 10)
    ;
    multi_pk
        .primary_key(L"id", L"integer")
        .primary_key(L"name", L"varchar", 10)
    ;
    // Add a non-nullable reference to a simple object
    meta_instance ref1("fk_ref1");
    ref1
        .primary_key(L"id", L"integer")
        .field(L"simple", simple, true)
    ;
    // Add a non-nullable reference to a multi_pk object
    meta_instance ref2("fk_ref2");
    ref2
        .primary_key(L"id", L"integer")
        .field(L"multi", multi_pk, true)
    ;
    // Add a nullable reference to a multi_pk object
    meta_instance ref3("fk_ref3");
    ref3
        .primary_key(L"id", L"integer")
        .field(L"multi", multi_pk, false)
    ;
    FSL_CHECK_EQ( ref3[L"multi"].not_null(), false );
    // Finally create the tables
    dbtransaction transaction( dbc );
    transaction
        .create_table( simple )
        .create_table( multi_pk )
        .create_table( ref1 )
        .create_table( ref2 )
        .create_table( ref3 )
        .commit();
}

