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
    auto rs = cnx.exec("SELECT 1 AS col1, 2");
    auto names = rs.columns();
    FSL_CHECK_EQ(names.size(), 2u);
    FSL_CHECK(names[0]);
    FSL_CHECK_EQ(names[0].value(), "col1");
    FSL_CHECK(not names[1]);
    auto pos = rs.begin();
    FSL_CHECK(pos != rs.end());
    auto row = *pos;
    FSL_CHECK_EQ(row.size(), 2u);
    FSL_CHECK_EQ(row[0], fostlib::json(1));
    FSL_CHECK_EQ(row[1], fostlib::json(2));
    ++pos;
    FSL_CHECK(pos == rs.end());
}


FSL_TEST_FUNCTION(rows) {
    fostlib::pg::connection cnx;
    auto rs = cnx.exec("SELECT ('a') AS c "
        "UNION SELECT ('b') UNION SELECT ('c') ORDER BY c");
    auto pos = rs.begin();
    auto row = *pos;
    FSL_CHECK_EQ(row.size(), 1u);
    FSL_CHECK_EQ(row[0], fostlib::json("a"));
    row = *++pos;
    FSL_CHECK_EQ(row.size(), 1u);
    FSL_CHECK_EQ(row[0], fostlib::json("b"));
    row = *++pos;
    FSL_CHECK_EQ(row.size(), 1u);
    FSL_CHECK_EQ(row[0], fostlib::json("c"));
    FSL_CHECK(++pos == rs.end());
}

