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


fostlib::pg::recordset_range fostlib::pg::connection::exec(const utf8_string &sql) {
    return recordset_range(*pimpl, sql);
}

