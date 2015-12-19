/*
    Copyright 2015, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#pragma once


#include <fost/core>
#include <fost/pg/connection.hpp>


namespace fostlib {


    namespace pg {


        class recordset;


        /// A single row in the results
        class record {
            std::vector<json> fields;
            record(std::vector<json>);

            friend class recordset;
        };


        /// A range-based recordset
        class recordset {
            struct impl;
            std::unique_ptr<impl> pimpl;
        private:
            recordset(connection::impl &, const utf8_string &);

        public:
            /// Allow move
            recordset(recordset&&);
            /// Allow public desctruction
            ~recordset();

            /// The recordset iterator
            class const_iterator {
                struct impl;
                std::unique_ptr<impl> pimpl;
                const_iterator(recordset::impl&, bool);
            public:
                /// Default construct needs to be allowed
                const_iterator();
                /// Allow copying
                const_iterator(const const_iterator &);
                /// Allow destruction
                ~const_iterator();

                record *operator -> () const;

                friend class recordset;
            };

            /// The first record
            const_iterator begin() const;
            /// The end of the recordset
            const_iterator end() const;

            friend class const_iterator;
            friend class connection;
        };


    }


}

