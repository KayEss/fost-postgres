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


fostlib::pg::record::record(std::size_t columns)
: fields(columns) {
}


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


namespace {
    void fillin(pqxx::result::const_iterator pos, std::vector<fostlib::json> &fields) {
        for ( pqxx::row::size_type index{0}; index != fields.size(); ++index ) {
            if ( pos[index].is_null() ) {
                fields[index] = fostlib::json();
            } else {
                fields[index] = fostlib::json(pos[index].c_str());
            }
        }
    }
}


fostlib::pg::recordset::const_iterator::const_iterator()
: pimpl(new impl(pqxx::result::const_iterator(), 0u)) {
}
fostlib::pg::recordset::const_iterator::const_iterator(const const_iterator &other)
: pimpl(new impl(other.pimpl->position, other.pimpl->row.size())) {
    pimpl->row = other.pimpl->row;
}
fostlib::pg::recordset::const_iterator::const_iterator(recordset::impl &rs, bool begin)
: pimpl(new impl(begin ? rs.records.begin() : rs.records.end(), rs.records.columns())) {
    if ( pimpl->position != rs.records.end() ) {
        fillin(pimpl->position, pimpl->row.fields);
    }
}


fostlib::pg::recordset::const_iterator::~const_iterator() = default;


const fostlib::pg::record *fostlib::pg::recordset::const_iterator::operator -> () const {
    return &pimpl->row;
}
const fostlib::pg::record &fostlib::pg::recordset::const_iterator::operator * () const {
    return pimpl->row;
}

