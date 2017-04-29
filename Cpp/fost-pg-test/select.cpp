/*
    Copyright 2017, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <fost/postgres>
#include <fost/test>


FSL_TEST_SUITE(select);


FSL_TEST_FUNCTION(basic) {
    fostlib::pg::connection cnx;
    auto rs = cnx.exec("SELECT 1");
}
