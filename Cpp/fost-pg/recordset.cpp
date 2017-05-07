/*
    Copyright 2017, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "recordset.i.hpp"
#include <f5/threading/sync.hpp>
#include <fost/insert>
#include <fost/parse/parse.hpp>
#include <fost/exception/parse_error.hpp>
#include <fost/pg/recordset.hpp>


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
    if ( begin ) {
        auto cols = rs.column_names.size();
        pimpl.reset(new impl{rs, record{cols}});
        if ( rs.block ) {
            pimpl->decode_row();
        } else {
            pimpl->finished = true;
        }
    } else {
        pimpl.reset(new impl{rs});
    }
}



fostlib::pg::recordset::const_iterator::~const_iterator() = default;


const fostlib::pg::record &fostlib::pg::recordset::const_iterator::operator * () const {
    return pimpl->data;
}


fostlib::pg::recordset::const_iterator &fostlib::pg::recordset::const_iterator::operator ++ () {
    if ( not pimpl->rsp.fields.size() ) {
        if ( pimpl->rsp.next_body_size ) {
            f5::sync s;
            boost::asio::spawn(pimpl->rsp.cnx.socket.get_io_service(), s([&](auto yield) {
                pgasio::record_block block{pimpl->rsp.column_meta.size()};
                pimpl->rsp.next_body_size =
                    block.read_rows(pimpl->rsp.cnx.socket, pimpl->rsp.next_body_size, yield);
                pimpl->rsp.fields = block.fields();
                pimpl->rsp.block = std::move(block);
            }));
            s.wait();
        } else {
            pimpl->finished = true;
            pimpl->rsp.block = null;
        }
    }
    if ( not pimpl->finished ) pimpl->decode_row();
    return *this;
}


bool fostlib::pg::recordset::const_iterator::operator == (const const_iterator &r) const {
    return &pimpl->rsp == &r.pimpl->rsp &&
        ((pimpl->finished && r.pimpl->finished) ||
            (not pimpl->finished && not r.pimpl->finished &&
                pimpl->row_number == r.pimpl->row_number));
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
    inline double float_parser(fostlib::utf::u8_view value) {
        double ret{0};
        auto pos = value.begin();
        if ( not boost::spirit::qi::parse(pos, value.end(),
                boost::spirit::qi::double_, ret) && pos == value.end() )
        {
            throw fostlib::exceptions::parse_error("Whilst parsing a double", value);
        } else {
            return ret;
        }
    }
}


std::size_t fostlib::pg::recordset::const_iterator::impl::decode_row() {
    auto fields = rsp.fields.slice(0, rsp.column_meta.size());
    rsp.fields = rsp.fields.slice(fields.size());
    data.fields.clear();
    for ( data.fields.clear(); fields.size(); fields = fields.slice(1) ) {
        if ( fields[0].data() == nullptr ) {
            data.fields.push_back(json());
        } else if ( rsp.column_meta[data.size()].format_code == 0 ) {
            const utf::u8_view str{reinterpret_cast<const char *>(fields[0].data()), fields[0].size()};
            switch ( rsp.column_meta[data.size()].field_type_oid ) {
            case 16: // bool
                data.fields.push_back(fostlib::json(str.data()[0] == 't' ? true : false));
                break;
            case 21: // int2
            case 23: // int4
            case 20: // int8
            case 26: // oid
                data.fields.push_back(json(int_parser(str)));
                break;
            case 700: // float4
            case 701: // float8
                data.fields.push_back(fostlib::json(float_parser(str)));
                break;
            case 114: // json
            case 3802: // jsonb
                data.fields.push_back(fostlib::json::parse(str));
                break;
            case 1114: // timestamp without time zone
                throw fostlib::exceptions::not_implemented(__FUNCTION__,
                    "Timestamp fields without time zones are explicitly disabled. "
                    "Fix your schema to use 'timestamp with time zone'");
            default:
#ifdef DEBUG
                fostlib::log::warning(c_fost_pg)
                    ("", "Postgres type decoding -- unknown type OID")
                    ("name", rsp.column_names[data.size()])
                    ("field_type_oid", rsp.column_meta[data.size()].field_type_oid)
                    ("type_modifier", rsp.column_meta[data.size()].type_modifier)
                    ("format_code", rsp.column_meta[data.size()].format_code)
                    ("bytes", str.bytes())
                    ("string", str);
#endif
            case 25: // text
            case 1043: // varchar
            case 1082: // date
            case 1083: // time
            case 1184: // timestamp with time zone
            case 1700: // numeric
            case 2950: // uuid
                data.fields.push_back(json(string(str)));
            }
        } else {
            throw exceptions::not_implemented(__func__, "Binary format");
        }
    }
    return data.size();
}


/*
 * fostlib::pg::record
 */


fostlib::pg::record::record(std::size_t cols) {
    fields.reserve(cols);
}

