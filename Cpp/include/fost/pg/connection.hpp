/*
    Copyright 2015-2016, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#pragma once


#include <fost/core>


namespace fostlib {


    namespace pg {


        class recordset;
        class unbound_procedure;


        /// A read/write database connection. Also provides a low level API
        /// for interacting with the database.
        class connection {
            friend class recordset;
            friend class unbound_procedure;
            struct impl;
            std::unique_ptr<impl> pimpl;
        public:
            /// A default connection without host or password
            connection();
            /// Connect to a specified host without specifying a password
            connection(const string &);
            /// Connect to a specified host and database
            connection(const string &, const string &);
            /// Destructor so we can link
            ~connection();

            /// Commit the transaction
            void commit();

            /// Return a recordset range from the execution of the command
            recordset exec(const utf8_string &);
            /// Select statement intended for fetching individual row, or collections
            recordset select(const char *relation, const json &keys);
            /// Perform a one row INSERT statement. Pass a JSON object that specifies
            /// the field names and values
            connection &insert(const char *relation, const json &values);
            /// Perform an UPSERT (INSERT/CONFLICT). Give the keys and values
            connection &upsert(const char *relation, const json &keys, const json &values);

            /// Create an anonymous stored procedure
            unbound_procedure procedure(const utf8_string &);
        };


    }


}

