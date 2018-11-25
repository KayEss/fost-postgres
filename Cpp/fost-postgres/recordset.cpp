/*
    Copyright 2015-2017, Felspar Co Ltd. http://support.felspar.com/
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


fostlib::pg::record::record(std::size_t columns) : fields(columns) {}


/*
    fostlib::pg::recordset
*/


fostlib::pg::recordset::recordset(std::unique_ptr<recordset::impl> &&p)
: pimpl(std::move(p)) {}
fostlib::pg::recordset::recordset(connection::impl &cnx, const utf8_string &sql)
: pimpl(new impl(cnx, sql)) {}
fostlib::pg::recordset::recordset(recordset &&other)
: pimpl(std::move(other.pimpl)) {}


fostlib::pg::recordset::~recordset() {}


std::vector<fostlib::nullable<fostlib::string>>
        fostlib::pg::recordset::columns() const {
    std::vector<fostlib::nullable<fostlib::string>> names;
    names.reserve(pimpl->names.size());
    std::map<int64_t, fostlib::string> oid_prefix;
    for (auto c : pimpl->names) {
        if (c == nullptr || c[0] == 0) {
            names.push_back(fostlib::null);
        } else {
            const string colname{c};
            const auto table =
                    pimpl->records.column_table(coerce<int>(names.size()));
            if (colname.endswith("__tableoid")) {
                oid_prefix[table] = colname.substr(0, colname.length() - 8);
                names.push_back(colname);
            } else if (oid_prefix.find(table) != oid_prefix.end()) {
                names.push_back(oid_prefix[table] + colname);
            } else {
                names.push_back(colname);
            }
        }
    }
    return names;
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
    int64_t int_parser(const std::string &value) {
        int64_t ret{0};
        auto pos = value.begin();
        if (not boost::spirit::qi::parse(
                    pos, value.end(), boost::spirit::qi::int_parser<int64_t>(),
                    ret)
            && pos == value.end()) {
            throw fostlib::exceptions::parse_error(
                    "Whilst parsing an int", value);
        } else {
            return ret;
        }
    }
    double float_parser(const std::string &value) {
        double ret{0};
        auto pos = value.begin();
        if (not boost::spirit::qi::parse(
                    pos, value.end(), boost::spirit::qi::double_, ret)
            && pos == value.end()) {
            throw fostlib::exceptions::parse_error(
                    "Whilst parsing a double", value);
        } else {
            return ret;
        }
    }

    void
            fillin(const std::vector<pqxx::oid> &types,
                   pqxx::result::const_iterator pos,
                   std::vector<fostlib::json> &fields) {
        for (pqxx::row::size_type index{0}; index != fields.size(); ++index) {
            if (pos[index].is_null()) {
                fields[index] = fostlib::json();
            } else {
                switch (types[index]) {
                case 16: // bool
                    fields[index] = fostlib::json(
                            pos[index].c_str()[0] == 't' ? true : false);
                    break;
                case 21: // int2
                case 23: // int4
                case 20: // int8
                case 26: // oid
                    fields[index] =
                            fostlib::json(int_parser(pos[index].c_str()));
                    break;
                case 700: // float4
                case 701: // float8
                    fields[index] =
                            fostlib::json(float_parser(pos[index].c_str()));
                    break;
                case 114: // json
                case 3802: // jsonb
                    fields[index] = fostlib::json::parse(pos[index].c_str());
                    break;
                case 1114: // timestamp without time zone
                    throw fostlib::exceptions::not_implemented(
                            __FUNCTION__,
                            "Timestamp fields without time zones are "
                            "explicitly disabled. "
                            "Fix your schema to use 'timestamp with time "
                            "zone'");
                default:
#ifdef DEBUG
                    fostlib::log::warning(fostlib::pg::c_fost_pg)(
                            "", "Postgres type decoding -- unknown type OID")(
                            "oid", types[index]);
#endif
                case 25: // text
                case 1043: // varchar
                case 1082: // date
                case 1083: // time
                case 1184: // timestamp with time zone
                case 1700: // numeric
                case 2950: // uuid
                    fields[index] =
                            fostlib::coerce<fostlib::json>(pos[index].c_str());
                }
            }
        }
    }
}


fostlib::pg::recordset::const_iterator::const_iterator()
: pimpl(new impl(pqxx::result::const_iterator(), 0u)) {}
fostlib::pg::recordset::const_iterator::const_iterator(
        const const_iterator &other)
: pimpl(new impl(
          other.pimpl->rs, other.pimpl->position, other.pimpl->row.size())) {
    pimpl->row = other.pimpl->row;
}
fostlib::pg::recordset::const_iterator::const_iterator(
        recordset::impl &rs, bool begin)
: pimpl(new impl(
          &rs,
          begin ? rs.records.begin() : rs.records.end(),
          rs.records.columns())) {
    if (pimpl->position != rs.records.end()) {
        fillin(rs.types, pimpl->position, pimpl->row.fields);
    }
}


fostlib::pg::recordset::const_iterator::~const_iterator() = default;


fostlib::pg::recordset::const_iterator &fostlib::pg::recordset::const_iterator::
        operator=(const fostlib::pg::recordset::const_iterator &other) {
    pimpl.reset(new impl(
            other.pimpl->rs, other.pimpl->position, other.pimpl->row.size()));
    pimpl->row = other.pimpl->row;
    return *this;
}


bool fostlib::pg::recordset::const_iterator::
        operator==(const const_iterator &other) const {
    if (pimpl && other.pimpl) {
        return pimpl->position == other.pimpl->position;
    } else {
        return pimpl == other.pimpl;
    }
}


const fostlib::pg::record *fostlib::pg::recordset::const_iterator::
        operator->() const {
    return &pimpl->row;
}
const fostlib::pg::record &fostlib::pg::recordset::const_iterator::
        operator*() const {
    return pimpl->row;
}


fostlib::pg::recordset::const_iterator &fostlib::pg::recordset::const_iterator::
        operator++() {
    if (++pimpl->position != pimpl->rs->records.end()) {
        fillin(pimpl->rs->types, pimpl->position, pimpl->row.fields);
    }
    return *this;
}
fostlib::pg::recordset::const_iterator fostlib::pg::recordset::const_iterator::
        operator++(int) {
    auto result = *this;
    this->operator++();
    return result;
}
