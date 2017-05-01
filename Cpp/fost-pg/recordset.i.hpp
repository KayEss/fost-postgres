/*
    Copyright 2017, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#pragma once

#include "connection.i.hpp"


namespace fostlib {
    namespace pg {


        struct recordset::impl {
            connection::impl &cnx;

            impl(connection::impl &c)
            : cnx(c) {
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

            nullable<response> first_data_row;
        };


        struct recordset::const_iterator::impl {
            recordset::impl &rsp;
            response data_row;
            record data;

            bool finished;
            std::size_t row_number;

            std::size_t decode_row();
            bool next_record(boost::asio::yield_context &);
        };


    }
}

