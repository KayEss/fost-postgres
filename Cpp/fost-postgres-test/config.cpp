/*
    Copyright 2008-2009, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "fost-postgres-test.hpp"


using namespace fostlib;


const setting< string > read_dsn( L"fost-postgres-test/basic.cpp", L"Postgres tests", L"Read connection", L"pqxx/user=Test password=tester host=localhost", true );
const setting< string > write_dsn( L"fost-postgres-test/basic.cpp", L"Postgres tests", L"Write connection", L"pqxx/user=Test password=tester host=localhost", true );


