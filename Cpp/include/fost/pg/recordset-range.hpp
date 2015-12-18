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


        class connection;


        /// A range-based recordset
        class recordset_range {
            friend class connection;
        private:
            recordset_range();
        };


    }


}

