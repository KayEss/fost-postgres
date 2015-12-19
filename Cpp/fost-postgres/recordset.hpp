/*
    Copyright 2015, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#pragma once


#include <fost/pg/recordset.hpp>
#include "connection.hpp"


struct fostlib::pg::recordset::impl {
    pqxx::result records;
    std::vector<pqxx::oid> types;

    impl(connection::impl &cnx, const utf8_string &sql)
    : records(cnx.trans.exec(sql.underlying())), types(records.columns()) {
        for ( pqxx::row::size_type index{0}; index != types.size(); ++index ) {
            types[index] = records.column_type(index);
        }
    }
};


struct fostlib::pg::recordset::const_iterator::impl {
    pqxx::result::const_iterator position;
    record row;

    impl(pqxx::result::const_iterator pos, std::size_t cols)
    : position(pos), row(cols) {
    }
};

