/*
    Copyright 2015-2016, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#pragma once


#include <fost/core>
#include <fost/pg/connection.hpp>


namespace fostlib {


    namespace pg {


        class record;
        class unbound_procedure;


        /// A range-based recordset
        class recordset {
            friend class unbound_procedure;

            struct impl;
            std::unique_ptr<impl> pimpl;

            recordset(std::unique_ptr<impl> &&p);
            recordset(connection::impl &, const utf8_string &);

        public:
            /// Allow move
            recordset(recordset&&);
            /// Allow public desctruction
            ~recordset();

            /// Return the column names
            std::vector<fostlib::nullable<fostlib::string>> columns() const;

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

                /// Compare for equality
                bool operator == (const const_iterator &) const;

                /// Dereference the iterator
                const record *operator -> () const;
                /// Dereference the iterator
                const record &operator * () const;

                /// Move to the next row
                const_iterator &operator ++ ();

                friend class recordset;
            };

            /// The first record
            const_iterator begin() const;
            /// The end of the recordset
            const_iterator end() const;

            friend class const_iterator;
            friend class connection;
        };


        /// A single row in the results
        class record {
            std::vector<json> fields;
            record(std::size_t);

        public:
            /// The number of columns
            std::size_t size() const {
                return fields.size();
            }

            /// Return the value in the specified field number
            const json &operator [] (std::size_t index) const {
                return fields[index];
            }

            friend class recordset::const_iterator;
        };


    }


}

