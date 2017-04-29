/*
    Copyright 2017, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "reactor.hpp"


f5::boost_asio::reactor_pool &fostlib::pg::reactor() {
    static f5::boost_asio::reactor_pool r;
    return r;
}

