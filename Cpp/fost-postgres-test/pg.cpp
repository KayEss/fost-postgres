/*
    Copyright 2015, Felspar Co Ltd. http://support.felspar.com/
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
    auto records = cnx.exec("SELECT 1");
    auto record = records.begin();
//     FSL_CHECK_EQ(record->size(), 1u);
}


FSL_TEST_FUNCTION(connect_specified) {
    fostlib::pg::connection cnx("/var/run/postgresql");
    auto records = cnx.exec("SELECT 1");
    auto record = records.begin();
//     FSL_CHECK_EQ(record->size(), 1u);
}

