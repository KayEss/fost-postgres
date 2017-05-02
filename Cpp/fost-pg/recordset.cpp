/*
    Copyright 2017, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <f5/threading/sync.hpp>
#include <fost/insert>
#include <fost/log>
#include <fost/parse/parse.hpp>
#include <fost/exception/parse_error.hpp>
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


fostlib::pg::recordset::const_iterator fostlib::pg::recordset::end() const {
    return const_iterator(*pimpl, false);
}


/*
 * fostlib::pg::recordset::impl
 */


std::size_t fostlib::pg::recordset::impl::row_description(response desc_packet) {
    decoder description(desc_packet);
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
    if ( begin && rs.first_data_row ) {
        pimpl.reset(new impl{rs, std::move(rs.first_data_row.value()),
            record{rs.column_names.size()}, false, 1u});
        rs.first_data_row = null;
        pimpl->decode_row();
    } else if ( not begin ) {
        pimpl.reset(new impl{rs, response(), record{0}, true, 0u});
    } else {
        throw exceptions::not_implemented("This recordset has already been iterated over");
    }
}



fostlib::pg::recordset::const_iterator::~const_iterator() = default;


const fostlib::pg::record &fostlib::pg::recordset::const_iterator::operator * () const {
    return pimpl->data;
}


fostlib::pg::recordset::const_iterator &fostlib::pg::recordset::const_iterator::operator ++ () {
    f5::sync s;
    boost::asio::spawn(pimpl->rsp.cnx.socket.get_io_service(), s([&](auto yield) {
        pimpl->next_record(yield);
    }));
    s.wait();
    return *this;
}


bool fostlib::pg::recordset::const_iterator::operator == (const const_iterator &r) const {
    return &pimpl->rsp == &r.pimpl->rsp &&
        ((pimpl->finished && r.pimpl->finished) || pimpl->row_number == r.pimpl->row_number);
}


/*
 * fostlib::pg::recordset::const_iterator::impl
 */


namespace {
    inline int64_t int_parser(fostlib::utf::u8_view value) {
        int64_t ret{};
        auto pos = value.begin();
        if ( not boost::spirit::qi::parse(pos, value.end(),
                boost::spirit::qi::int_parser<int64_t>(), ret) && pos == value.end() )
        {
            throw fostlib::exceptions::parse_error("Whilst parsing an int", value);
        } else {
            return ret;
        }
    }
//     inline double float_parser(fostlib::utf::u8_view value) {
//         double ret{0};
//         auto pos = value.begin();
//         if ( not boost::spirit::qi::parse(pos, value.end(),
//                 boost::spirit::qi::double_, ret) && pos == value.end() )
//         {
//             throw fostlib::exceptions::parse_error("Whilst parsing a double", value);
//         } else {
//             return ret;
//         }
//     }
}


std::size_t fostlib::pg::recordset::const_iterator::impl::decode_row() {
    decoder decode(data_row);
    const auto cols = decode.read_int16();
    if ( cols != rsp.column_meta.size() ) {
        exceptions::not_implemented error(__func__, "Mismatch of column counts");
        insert(error.data(), "expected", rsp.column_meta.size());
        insert(error.data(), "got", cols);
        throw error;
    }
    data.fields.clear();
    while ( decode.remaining() ) {
        const auto bytes = decode.read_int32();
        if ( bytes == -1 ) {
            data.fields.push_back(json());
        } else if ( rsp.column_meta[data.size()].format_code == 0 ) {
            const auto str = decode.read_u8_view(bytes);
            switch ( rsp.column_meta[data.size()].field_type_oid ) {
            case 23: // int32
                data.fields.push_back(json(int_parser(str)));
                break;
            case 25: // text
                data.fields.push_back(json(string(str)));
                break;
            default:
                fostlib::log::warning(c_fost_pg)
                    ("", "Column value")
                    ("name", rsp.column_names[data.size()])
                    ("field_type_oid", rsp.column_meta[data.size()].field_type_oid)
                    ("type_modifier", rsp.column_meta[data.size()].type_modifier)
                    ("format_code", rsp.column_meta[data.size()].format_code)
                    ("bytes", bytes);
                data.fields.push_back(json(string(str)));
            }
        } else {
            throw exceptions::not_implemented(__func__, "Binary format");
        }
    }
    return cols;
}


bool fostlib::pg::recordset::const_iterator::impl::next_record(boost::asio::yield_context &yield) {
    while ( true ) {
        auto reply{rsp.cnx.read(yield)};
        decoder decode(reply);
        if ( reply.type == 'C' ) {
            fostlib::log::debug(c_fost_pg)
                ("", "Command close")
                ("message", decode.read_string());
        } else if ( reply.type == 'D' ) {
            data_row = std::move(reply);
            decode_row();
            ++row_number;
            return finished;
        } else if ( reply.type == 'Z' ) {
            finished = true;
            return finished;
        } else {
            throw exceptions::not_implemented(__func__, reply.code());
        }
    }
}


/*
 * fostlib::pg::record
 */


fostlib::pg::record::record(std::size_t cols) {
    fields.reserve(cols);
}

