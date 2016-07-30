/*
    Copyright 2016, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#pragma once


#include <fost/core>


namespace fostlib {


    namespace pg {


        class connection;
        class recordset;


//         /// Wraps a bound or partially bound stored procedure
//         class bound_procedure {
//         public:
//             bound_procedure &operator () (const fostlib::string &);
//
//             recordset exec();
//         };


        /// Wraps a stored procedure with no argument bindings
        class unbound_procedure {
            friend class connection;
            connection &cnx;

            unbound_procedure(connection &, std::string);
        public:
            const std::string name;

//             bound_procedure &operator () (const fostlib::string &);

            recordset exec(const std::vector<fostlib::string> &args);
            recordset exec(const std::vector<fostlib::json> &args);
        };


    }


}

