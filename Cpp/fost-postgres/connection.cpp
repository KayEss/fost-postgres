/*
    Copyright 2015, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <fost/pg/connection.hpp>
#include <fost/pg/recordset.hpp>
#include "connection.hpp"


fostlib::pg::connection::connection()
: pimpl(new impl(utf8_string())) {
}
fostlib::pg::connection::connection(const string &host)
: pimpl(new impl(coerce<utf8_string>("host=" + host))) {
}


fostlib::pg::connection::~connection() = default;


fostlib::pg::recordset fostlib::pg::connection::exec(const utf8_string &sql) {
    return std::move(recordset(*pimpl, sql));
}


void fostlib::pg::connection::commit() {
    pimpl->trans.commit();
}


namespace {
    fostlib::string column(const fostlib::json &name) {
        return '"' + fostlib::coerce<fostlib::string>(name) + '"';
    }
    fostlib::string columns(fostlib::string cols, const fostlib::json &def) {
        for ( fostlib::json::const_iterator iter(def.begin()); iter != def.end(); ++iter ) {
            if ( cols.empty() ) {
                cols = column(iter.key());
            } else {
                cols += ", " + column(iter.key());
            }
        }
        return cols;
    }
    fostlib::string columns(const fostlib::json &def) {
        return columns(fostlib::string(), def);
    }

    fostlib::string value(const fostlib::json &val) {
        return '\'' + fostlib::coerce<fostlib::string>(val) + '\'';
    }
    fostlib::string value_string(fostlib::string vals, const fostlib::json &def) {
        for ( const auto &val : def ) {
            if ( vals.empty() ) {
                vals = value(val);
            } else {
                vals += ", " + value(val);
            }
        }
        return vals;
    }
    fostlib::string value_string(const fostlib::json &def) {
        return value_string(fostlib::string(), def);
    }
}


fostlib::pg::connection &fostlib::pg::connection::insert(const char *relation, const json &values) {
    exec(coerce<utf8_string>(
        string("INSERT INTO ") + relation +
            " (" + columns(values) + ") VALUES (" + value_string(values) + ")"));
    return *this;
}


fostlib::pg::connection &fostlib::pg::connection::upsert(
    const char *relation, const json &keys, const json &values
) {
    string sql("INSERT INTO "),
        key_names(columns(keys)),
        value_names(columns(key_names, values)),
        updates;
    sql += relation;
    sql += " (" + value_names + ") VALUES (" + value_string(value_string(keys), values) + ") ";
    sql += "ON CONFLICT (" + key_names + ") DO ";
    for ( fostlib::json::const_iterator iter(values.begin()); iter != values.end(); ++iter ) {
        if ( updates.empty() ) {
            updates = column(iter.key()) + " = EXCLUDED." + column(iter.key());
        } else {
            updates += ", " + column(iter.key()) + " = EXCLUDED." + column(iter.key());
        }
    }
    if ( updates.empty() ) {
        sql += "NOTHING";
    } else {
        sql += "UPDATE SET " + updates;
    }
    exec(coerce<utf8_string>(sql));
    return *this;
}

