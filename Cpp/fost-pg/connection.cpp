/*
    Copyright 2017, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <f5/threading/sync.hpp>
#include <fost/pg/connection.hpp>
#include <fost/pg/recordset.hpp>
#include "connection.i.hpp"
#include "reactor.hpp"

#include <boost/asio/write.hpp>


const fostlib::module fostlib::pg::c_fost_pg(c_fost, "pg");


/*
 * fostlib::pg::connection
 */


fostlib::pg::connection::connection()
: pimpl{} {
    f5::sync s;
    boost::asio::spawn(reactor().get_io_service(), [&](auto yield) {
        pimpl.reset(new impl(reactor().get_io_service(), "/var/run/postgresql/.s.PGSQL.5432", yield));
        s.done();
    });
    s.wait();
}


fostlib::pg::connection::~connection() = default;


fostlib::pg::recordset fostlib::pg::connection::exec(const utf8_string &sql) {
    throw exceptions::not_implemented(__func__);
}


/*
 * fostlib::pg::command
 */


fostlib::pg::command::command() {
}


void fostlib::pg::command::send(
    boost::asio::local::stream_protocol::socket &socket, boost::asio::yield_context &yield
) {
    const auto bytes{coerce<uint32_t>(4 + buffer.size())};
    const auto send = boost::endian::native_to_big(bytes);
    header.sputn(reinterpret_cast<const char*>(&send), 4);
    std::array<boost::asio::streambuf::const_buffers_type, 2>
        data{{header.data(), buffer.data()}};
    async_write(socket, data, yield);
}


/*
 * fostlib::pg::connection::impl
 */


fostlib::pg::connection::impl::impl(
    boost::asio::io_service &ios, f5::lstring loc, boost::asio::yield_context &yield
) : socket(ios) {
    boost::asio::local::stream_protocol::endpoint ep(loc.c_str());
    socket.async_connect(ep, yield);
    command cmd;
    cmd.write(int32_t{0x0003'0000});
    cmd.send(socket, yield);
}

