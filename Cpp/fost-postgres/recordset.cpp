/*
    Copyright 2015, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <fost/pg/recordset.hpp>
#include "recordset.hpp"


/*
    fostlib::pg::recordset
*/


fostlib::pg::recordset::recordset(connection::impl &cnx, const utf8_string &sql)
: pimpl(new impl(cnx, sql)) {
}
fostlib::pg::recordset::recordset(recordset &&other)
: pimpl(std::move(other.pimpl)) {
}


fostlib::pg::recordset::~recordset() {
}


fostlib::pg::recordset::const_iterator fostlib::pg::recordset::begin() const {
    return fostlib::pg::recordset::const_iterator();
}


/*
    fostlib::pg::recordset::const_iterator
*/


fostlib::pg::recordset::record_type *fostlib::pg::recordset::const_iterator::operator -> () const {
    return &row;
}

