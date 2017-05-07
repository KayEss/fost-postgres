/*
    Copyright 2017, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <fost/main>
#include <fost/postgres>

#include <experimental/iterator>


namespace {
    const fostlib::setting<fostlib::nullable<fostlib::string>> c_host(__FILE__,
        "select", "DB Host", fostlib::null, true);
    const fostlib::setting<fostlib::nullable<fostlib::string>> c_user(__FILE__,
        "select", "DB User", fostlib::null, true);
}


FSL_MAIN("select", "SELECT data\nCopyright 2017, Felspar Co Ltd")
    (fostlib::ostream &out, fostlib::arguments &args)
{
    if ( args.size() < 3 ) {
        out << "Missing arguments\n\n    "
            << args[0] << " [options] dbname \"SELECT .... FROM ...\"\n"
            << "      -h     hostname or path\n"
            << "      -U     username\n" << std::endl;
        return 1;
    }
    args.commandSwitch("h", c_host);
    args.commandSwitch("U", c_user);

    fostlib::json dsn;
    fostlib::insert(dsn, "dbname", args[1]);
    if ( c_host.value() )
        fostlib::insert(dsn, "host", c_host.value().value());
    if ( c_user.value() )
        fostlib::insert(dsn, "user", c_user.value().value());
    fostlib::pg::connection cnx(dsn);

    auto results = cnx.exec(args[2].value().c_str());
    {
        auto comma = std::experimental::make_ostream_joiner(std::cout, ",");
        for ( auto &&name : results.columns() ) {
            *comma++ = fostlib::json::unparse(fostlib::json(name), false);
        }
    }
    for ( auto &&row : results ) {
        out << '\n';
        auto comma = std::experimental::make_ostream_joiner(std::cout, ",");
        for ( auto &&col : row ) {
            *comma++ = fostlib::json::unparse(col, false);
        }
    }
    out << std::endl;

    return 0;
}

