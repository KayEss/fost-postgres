/*
    Copyright 2017, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#pragma once

#include "connection.i.hpp"
#include <pgasio/recordset.hpp>
#include <fost/pg/recordset.hpp>
#include <f5/threading/channel.hpp>


namespace fostlib {
    namespace pg {


        struct recordset::impl {
            connection::impl &cnx;

            impl(connection::impl &c, std::size_t buffer_size = 3)
            : cnx(c),
                records_data(cnx.socket.get_io_service(), buffer_size)
            {
            }

            std::vector<fostlib::nullable<fostlib::string>> column_names;
            struct cmeta {
                int32_t table_oid;
                int16_t table_column;
                int32_t field_type_oid;
                int16_t data_size;
                int32_t type_modifier;
                int16_t format_code;
            };
            std::vector<cmeta> column_meta;
            std::size_t row_description(response);

            using record_data = std::vector<std::vector<fostlib::json>>;
            f5::boost_asio::channel<record_data> records_data;
            record_data record_block;
            pgasio::array_view<record_data::value_type> records;

            void decode_fields(
                std::vector<fostlib::json> &into,
                pgasio::array_view<const pgasio::byte_view> fields) const;
        };


        struct recordset::const_iterator::impl {
            impl(recordset::impl &rs)
            : rsp(rs), data{0}, finished{true}, row_number{} {
            }
            impl(recordset::impl &rs, record r)
            : rsp(rs), data(std::move(r)),
                finished(false), row_number{}
            {
            }

            recordset::impl &rsp;
            record data;
            bool finished;
            std::size_t row_number;

            std::size_t decode_row();
        };


    }
}

