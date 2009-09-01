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

