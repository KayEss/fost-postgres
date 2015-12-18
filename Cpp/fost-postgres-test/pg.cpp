/*
    Copyright 2015, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "fost-postgres-test.hpp"
#include <fost/postgres>
#include <fost/test>


using namespace fostlib;


FSL_TEST_SUITE(pg);


FSL_TEST_FUNCTION(connect_default) {
    fostlib::pg::connection cnx;
}


FSL_TEST_FUNCTION(connect_specified) {
    fostlib::pg::connection cnx2("/var/run/postgres");
}

