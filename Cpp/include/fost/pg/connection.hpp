/*
    Copyright 2015, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <fost/core>


namespace fostlib {


    namespace pg {


        /// A read/write database connection
        class connection {
            struct impl;
            std::unique_ptr<impl> pimpl;
        public:
            /// Connect to a specified host without specifying a password
            connection(const string &);
            /// Destructor so we can link
            ~connection();
        };


    }


}

