/*
    Copyright 2017, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <fost/pg/connection.hpp>
#include <fost/pg/recordset.hpp>


struct fostlib::pg::connection::impl {
};


fostlib::pg::connection::connection()
: pimpl(new impl) {
}


fostlib::pg::connection::~connection() = default;


fostlib::pg::recordset fostlib::pg::connection::exec(const utf8_string &sql) {
    throw exceptions::not_implemented(__func__);
}

