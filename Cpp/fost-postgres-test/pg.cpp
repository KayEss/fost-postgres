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
    FSL_CHECK_EQ(record->size(), 1u);
    FSL_CHECK_EQ((*record)[0], fostlib::json(1));
    FSL_CHECK(records.begin() != records.end());
    FSL_CHECK(++records.begin() == records.end());
}


FSL_TEST_FUNCTION(connect_specified) {
    fostlib::pg::connection cnx("/var/run/postgresql");
    auto records = cnx.exec("SELECT 1");
    auto record = records.begin();
    FSL_CHECK_EQ(record->size(), 1u);
    FSL_CHECK_EQ((*record)[0], fostlib::json(1));
}


namespace {
    template<typename A>
    void check(const char *sql, A value) {
        fostlib::pg::connection cnx;
        auto records = cnx.exec(sql);
        auto record = records.begin();
        FSL_CHECK_EQ(record->size(), 1u);
        FSL_CHECK_EQ((*record)[0], fostlib::json(value));
        FSL_CHECK(records.begin() != records.end());
        FSL_CHECK(++records.begin() == records.end());
    }
}
FSL_TEST_FUNCTION(type_null) {
    check("SELECT NULL", fostlib::json());
}
FSL_TEST_FUNCTION(type_bool) {
    check("SELECT 't'::bool", true);
    check("SELECT 'f'::bool", false);
}
FSL_TEST_FUNCTION(type_int2) {
    check("SELECT 1::int2", 1);
}
FSL_TEST_FUNCTION(type_int4) {
    check("SELECT 1::int4", 1);
}
FSL_TEST_FUNCTION(type_int8) {
    check("SELECT 1::int8", 1);
}
FSL_TEST_FUNCTION(type_float4) {
    check("SELECT 1::float4", 1.0);
}
FSL_TEST_FUNCTION(type_float8) {
    check("SELECT 1::float8", 1.0);
}
FSL_TEST_FUNCTION(type_json) {
    check("SELECT 'null'::json", fostlib::json());
    check("SELECT 'true'::json", true);
    check("SELECT 'false'::json", false);
    check("SELECT '{}'::json", fostlib::json::object_t());
}
FSL_TEST_FUNCTION(type_jsonb) {
    check("SELECT 'null'::jsonb", fostlib::json());
    check("SELECT 'true'::jsonb", true);
    check("SELECT 'false'::jsonb", false);
    check("SELECT '{}'::jsonb", fostlib::json::object_t());
}

