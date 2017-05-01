/*
    Copyright 2017, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <fost/pg/recordset.hpp>
#include "recordset.i.hpp"


/*
 * fostlib::pg::recordset
 */


fostlib::pg::recordset::recordset(std::unique_ptr<impl> &&p)
: pimpl(std::move(p)) {
}


fostlib::pg::recordset::~recordset() = default;


std::vector<fostlib::nullable<fostlib::string>> fostlib::pg::recordset::columns() const {
    return pimpl->column_names;
}


fostlib::pg::recordset::const_iterator fostlib::pg::recordset::begin() const {
    return const_iterator(*pimpl, true);
}


/*
 * fostlib::pg::recordset::impl
 */


std::size_t fostlib::pg::recordset::impl::row_description(response description) {
    const auto count = description.read_int16();
    column_names.reserve(count);
    column_meta.reserve(count);
    while ( column_names.size() != count ) {
        auto name = description.read_u8_view();
        if ( name.bytes() && *name.begin() != '?' ) {
            column_names.push_back(name);
        } else {
            column_names.push_back(null);
        }
        const auto table_oid = description.read_int32();
        const auto table_column = description.read_int16();
        const auto field_type_oid = description.read_int32();
        const auto data_size = description.read_int16();
        const auto type_modifier = description.read_int32();
        const auto format_code = description.read_int16();
        column_meta.push_back(impl::cmeta{table_oid,
            table_column, field_type_oid,
            data_size, type_modifier, format_code});
    }
    return count;
}


/*
 * fostlib::pg::recordset::const_iterator
 */


fostlib::pg::recordset::const_iterator::const_iterator(fostlib::pg::recordset::impl &rs, bool begin) {
    if ( rs.first_data_row ) {
        pimpl.reset(new impl{std::move(rs.first_data_row.value())});
        rs.first_data_row = null;
    } else {
        throw exceptions::not_implemented("This recordset has already been iterated over");
    }
}


fostlib::pg::recordset::const_iterator::~const_iterator() = default;
