/*
    Copyright 2015, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <fost/pg/recordset.hpp>
#include "recordset.hpp"


/*
    fostlib::pg::record
*/



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
    return fostlib::pg::recordset::const_iterator(*pimpl, true);
}


/*
    fostlib::pg::recordset::const_iterator
*/


fostlib::pg::recordset::const_iterator::const_iterator()
: pimpl(new impl(pqxx::result::const_iterator())) {
}
fostlib::pg::recordset::const_iterator::const_iterator(const const_iterator &other)
: pimpl(new impl(other.pimpl->position)) {
}
fostlib::pg::recordset::const_iterator::const_iterator(recordset::impl &rs, bool begin)
: pimpl(new impl(begin ? rs.records.begin() : rs.records.end())) {
}


fostlib::pg::recordset::const_iterator::~const_iterator() = default;


fostlib::pg::record *fostlib::pg::recordset::const_iterator::operator -> () const {
    throw fostlib::exceptions::not_implemented(__FUNCTION__);
}

