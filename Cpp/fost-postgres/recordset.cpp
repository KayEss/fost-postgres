/*
    Copyright 2015, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <fost/core>
#include <fost/exception/parse_error.hpp>
#include <fost/log>
#include <fost/parse/parse.hpp>
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
fostlib::pg::recordset::const_iterator fostlib::pg::recordset::end() const {
    return fostlib::pg::recordset::const_iterator(*pimpl, false);
}


/*
    fostlib::pg::recordset::const_iterator
*/


namespace {
    int64_t int_parser(const char *value) {
        int64_t ret{0};
        if ( !boost::spirit::parse(value,
                boost::spirit::int_parser<int64_t>()[phoenix::var(ret) = phoenix::arg1]).full )
            throw fostlib::exceptions::parse_error("Whilst parsing an int", value);
        return ret;
    }
    double float_parser(const char *value) {
        double ret{0};
        if ( !boost::spirit::parse(value,
                boost::spirit::real_parser<double>()[phoenix::var(ret) = phoenix::arg1]).full )
            throw fostlib::exceptions::parse_error("Whilst parsing a float", value);
        return ret;
    }

    void fillin(
        const std::vector<pqxx::oid> &types,
        pqxx::result::const_iterator pos,
        std::vector<fostlib::json> &fields
    ) {
        for ( pqxx::row::size_type index{0}; index != fields.size(); ++index ) {
            if ( pos[index].is_null() ) {
                fields[index] = fostlib::json();
            } else {
                switch ( types[index] ) {
                case 16: // bool
                    fields[index] = fostlib::json(pos[index].c_str()[0] == 't' ? true : false);
                    break;
                case 21: // int2
                case 23: // int4
                case 20: // int8
                    fields[index] = fostlib::json(int_parser(pos[index].c_str()));
                    break;
                case 700: // float4
                case 701: // float8
                    fields[index] = fostlib::json(float_parser(pos[index].c_str()));
                    break;
                case 114: // json
                    fields[index] = fostlib::json::parse(pos[index].c_str());
                    break;
                default:
                    fields[index] = fostlib::coerce<fostlib::json>(
                        fostlib::utf8_string(pos[index].c_str()));
                }
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
        fillin(rs.types, pimpl->position, pimpl->row.fields);
    }
}


fostlib::pg::recordset::const_iterator::~const_iterator() = default;


bool fostlib::pg::recordset::const_iterator::operator == (const const_iterator &other) const {
    if ( pimpl && other.pimpl ) {
        return pimpl->position == other.pimpl->position;
    } else {
        return pimpl == other.pimpl;
    }
}


const fostlib::pg::record *fostlib::pg::recordset::const_iterator::operator -> () const {
    return &pimpl->row;
}
const fostlib::pg::record &fostlib::pg::recordset::const_iterator::operator * () const {
    return pimpl->row;
}


fostlib::pg::recordset::const_iterator &fostlib::pg::recordset::const_iterator::operator ++ () {
    ++pimpl->position;
    return *this;
}

