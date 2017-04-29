/*
    Copyright 2017, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <f5/threading/sync.hpp>
#include <fost/log>
#include <fost/exception/unexpected_eof.hpp>
#include <fost/pg/connection.hpp>
#include <fost/pg/recordset.hpp>
#include "connection.i.hpp"
#include "reactor.hpp"

#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>


const fostlib::module fostlib::pg::c_fost_pg(c_fost, "pg");


/*
 * fostlib::pg::connection
 */


fostlib::pg::connection::connection()
: pimpl{} {
    f5::sync s;
    boost::asio::spawn(reactor().get_io_service(), s([&](auto yield) {
        pimpl.reset(new impl(reactor().get_io_service(), "/var/run/postgresql/.s.PGSQL.5432", yield));
    }));
    s.wait();
}


fostlib::pg::connection::~connection() = default;


fostlib::pg::recordset fostlib::pg::connection::exec(const utf8_string &sql) {
    f5::sync s;
    boost::asio::spawn(pimpl->socket.get_io_service(), s([&](auto yield) {
        command query{'Q'};
        query.write(sql.underlying().c_str());
        query.send(pimpl->socket, yield);
        throw exceptions::not_implemented(__func__);
        auto reply{pimpl->read(yield)};
        throw exceptions::not_implemented(std::to_string(reply.code));
    }));
    s.wait();
    throw exceptions::not_implemented(__func__);
}


/*
 * fostlib::pg::command
 */


fostlib::pg::command::command() {
}


fostlib::pg::command::command(char c) {
    header.sputc(c);
}


fostlib::pg::command &fostlib::pg::command::write(const char *s) {
    while (*s) {
        byte(*s++);
    }
    byte(char{});
    return *this;
}


void fostlib::pg::command::send(
    boost::asio::local::stream_protocol::socket &socket, boost::asio::yield_context &yield
) {
    const auto bytes{coerce<uint32_t>(4 + buffer.size())};
    const auto send = boost::endian::native_to_big(bytes);
    header.sputn(reinterpret_cast<const char*>(&send), 4);
    std::array<boost::asio::streambuf::const_buffers_type, 2>
        data{{header.data(), buffer.data()}};
    fostlib::log::debug(c_fost_pg)
        ("", "Sending data to Postgres")
        ("size", "bytes", bytes)
        ("size", "header", header.size())
        ("size", "body", buffer.size());
    async_write(socket, data, yield);
}


/*
 * fostlib::pg::response
 */


fostlib::pg::response::response(char c)
: code(c), body(new boost::asio::streambuf) {
}


/*
 * fostlib::pg::connection::impl
 */


fostlib::pg::connection::impl::impl(
    boost::asio::io_service &ios, f5::lstring loc, boost::asio::yield_context &yield
) : socket(ios) {
    boost::asio::local::stream_protocol::endpoint ep(loc.c_str());
    socket.async_connect(ep, yield);
    fostlib::log::debug(c_fost_pg)
        ("", "Connected to unix domain socket")
        ("path", loc.c_str());
    command cmd;
    cmd.write(int32_t{0x0003'0000});
//     cmd.write("user").write("kirit").byte(char{});
    cmd.send(socket, yield);
    auto reply{read(yield)};
    throw exceptions::not_implemented(std::string() + reply.code);
}


fostlib::pg::response fostlib::pg::connection::impl::read(boost::asio::yield_context &yield) {
    const auto transfer = [&](boost::asio::streambuf &buffer, std::size_t bytes) {
        boost::system::error_code error;
        boost::asio::async_read(socket, buffer,
            boost::asio::transfer_exactly(bytes), (yield)[error]);
        if ( error ) {
            throw exceptions::unexpected_eof("Reading bytes from socket", error);
        }
    };
    boost::asio::streambuf header;
    transfer(header, 5u);
    response reply(header.sbumpc());
    return reply;
}

