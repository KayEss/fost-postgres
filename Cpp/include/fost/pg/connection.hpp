/*
    Copyright 2015, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#pragma once


#include <fost/core>


namespace fostlib {


    namespace pg {


        class recordset;


        /// A read/write database connection
        class connection {
            friend class recordset;
            struct impl;
            std::unique_ptr<impl> pimpl;
        public:
            /// A default connection without host or password
            connection();
            /// Connect to a specified host without specifying a password
            connection(const string &);
            /// Destructor so we can link
            ~connection();

            /// Return a recordset range from the execution of the command
            recordset exec(const utf8_string &);
        };


    }


}

