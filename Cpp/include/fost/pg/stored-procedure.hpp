/**
    Copyright 2016-2019 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#pragma once


#include <fost/core>


namespace fostlib {


    namespace pg {


        class connection;
        class recordset;


        /// Wraps a stored procedure with no argument bindings
        class unbound_procedure {
            friend class connection;
            connection &cnx;

            unbound_procedure(connection &, std::string);

          public:
            const std::string name;

            recordset exec(std::vector<fostlib::string> args);
            recordset exec(const std::vector<fostlib::json> &args);
        };


    }


}
