/**
    Copyright 2008-2020 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#include "fost-postgres-test.hpp"


using namespace fostlib;


const setting<string> read_dsn(
        "fost-postgres-test/basic.cpp",
        "Postgres tests",
        "Read connection",
        "pqxx/",
        true);
const setting<string> write_dsn(
        "fost-postgres-test/basic.cpp",
        "Postgres tests",
        "Write connection",
        "pqxx/",
        true);
