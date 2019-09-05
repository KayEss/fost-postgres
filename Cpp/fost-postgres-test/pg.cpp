/**
    Copyright 2015-2019 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#include "fost-postgres-test.hpp"
#include <fost/postgres>
#include <fost/test>

#include <cstdlib>


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
    const char *pghost = std::getenv("PGHOST");
    if (pghost == nullptr) pghost = "/var/run/postgresql";
    fostlib::pg::connection cnx(pghost);
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
    // We don't care about the table, we want to test the transformation and it
    // shouldn't throw an error
    void use_value_in_where_clause(fostlib::json const target) {
        fostlib::json values;
        fostlib::insert(values, "table_name", target);
        fostlib::pg::connection cnx;
        auto records = cnx.select("information_schema.tables", values);
        auto record = records.begin();
        FSL_CHECK_EQ((*record)[0], fostlib::json());
    }
}
FSL_TEST_FUNCTION(type_null) { check("SELECT NULL", fostlib::json()); }
FSL_TEST_FUNCTION(type_bool) {
    check("SELECT 't'::bool", true);
    check("SELECT 'f'::bool", false);
}
FSL_TEST_FUNCTION(type_int2) { check("SELECT 1::int2", 1); }
FSL_TEST_FUNCTION(type_int4) { check("SELECT 1::int4", 1); }
FSL_TEST_FUNCTION(type_int8) { check("SELECT 1::int8", 1); }
FSL_TEST_FUNCTION(type_float4) { check("SELECT 1::float4", 1.0); }
FSL_TEST_FUNCTION(type_float8) { check("SELECT 1::float8", 1.0); }
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


FSL_TEST_FUNCTION(rows) {
    fostlib::pg::connection cnx;
    auto records = cnx.exec("SELECT 1 UNION SELECT 2 UNION SELECT 3");
    auto record = records.begin();
    FSL_CHECK_EQ(record->size(), 1u);
    FSL_CHECK_EQ((*record)[0], fostlib::json(1));
    ++record;
    FSL_CHECK_EQ((*record)[0], fostlib::json(2));
    ++record;
    FSL_CHECK_EQ((*record)[0], fostlib::json(3));
    FSL_CHECK(++record == records.end());
}

FSL_TEST_FUNCTION(transform_array_to_string_type) {
    fostlib::json arr;
    fostlib::jcursor().push_back(arr, fostlib::json());
    FSL_CHECK(arr.isarray());
    use_value_in_where_clause(arr);
}

FSL_TEST_FUNCTION(transform_object_to_string_type) {
    fostlib::json obj;
    fostlib::insert(obj, "", fostlib::json());
    FSL_CHECK(obj.isobject());
    use_value_in_where_clause(obj);
}

FSL_TEST_FUNCTION(transform_number_to_string_type) {
    auto atom = fostlib::json(5);
    FSL_CHECK(atom.isatom());
    use_value_in_where_clause(atom);
}

FSL_TEST_FUNCTION(transform_boolean_to_string_type) {
    auto boolean = fostlib::json(false);
    FSL_CHECK(boolean.isatom());
    use_value_in_where_clause(boolean);
}

FSL_TEST_FUNCTION(transform_null_to_string_type) {
    auto n = fostlib::json();
    FSL_CHECK(n.isnull());
    use_value_in_where_clause(n);
}