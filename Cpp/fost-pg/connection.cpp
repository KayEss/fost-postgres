/*
    Copyright 2017, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "connection.i.hpp"
#include "reactor.hpp"
#include "recordset.i.hpp"

#include <pgasio/network.hpp>

#include <f5/threading/sync.hpp>
#include <fost/insert>
#include <fost/log>
#include <fost/exception/unexpected_eof.hpp>

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


fostlib::pg::connection::connection(const fostlib::json &dsn) {
    f5::sync s;
    boost::asio::spawn(reactor().get_io_service(), s([&](auto yield) {
        auto const host = coerce<nullable<string>>(dsn["host"]);
        auto const database = coerce<nullable<string>>(dsn["dbname"]);
        auto const user = coerce<nullable<string>>(dsn["user"]);
        pimpl.reset(new impl(reactor().get_io_service(),
            host.value_or("/var/run/postgresql/.s.PGSQL.5432").c_str(),
            user.value_or("kirit"), database.value_or(""), yield));
    }));
    s.wait();
}


fostlib::pg::connection::~connection() = default;


fostlib::pg::recordset fostlib::pg::connection::exec(const utf8_string &sql) {
    f5::sync s;
    auto rs = std::make_unique<recordset::impl>(*pimpl);
    boost::asio::spawn(pimpl->socket.get_io_service(), s([&](auto yield) {
        command query{'Q'};
        query.write(sql.underlying().c_str());
        query.send(pimpl->socket, yield);
        while ( true ) {
            auto reply{pimpl->read(yield)};
            if ( reply.type == 'D' ) {
                rs->first_data_row = std::move(reply);
                return;
            } else if ( reply.type == 'T' ) {
                rs->row_description(std::move(reply));
            } else {
                throw exceptions::not_implemented(__func__, reply.code());
            }
        }
    }));
    s.wait();
    return recordset(std::move(rs));
}


/*
 * fostlib::pg::connection::impl
 */


fostlib::pg::connection::impl::impl(
    boost::asio::io_service &ios, f5::lstring loc, boost::asio::yield_context &yield
) : impl(ios, loc.c_str(), "kirit", utf::u8_view(), yield) {
}
fostlib::pg::connection::impl::impl(
    boost::asio::io_service &ios, const char *loc, utf::u8_view user,
    utf::u8_view database, boost::asio::yield_context &yield
) : socket(ios)
{
    auto logger = fostlib::log::info(c_fost_pg);
    logger
        ("path", loc)
        ("dbname", database)
        ("user", user);
    boost::asio::local::stream_protocol::endpoint ep(loc);
    socket.async_connect(ep, yield);
    logger("", "Connected to unix domain socket");
    command cmd;
    cmd.write(int32_t{0x0003'0000});
    cmd.write("user").write(user);
    if ( database.bytes() ) cmd.write("database").write(database);
    cmd.byte(char{});
    cmd.send(socket, yield);
    while ( true ) {
        auto reply{read(yield)};
        decoder decode(reply);
        if ( reply.type == 'K' ) {
            logger("cancellation", "process-id", decode.read_int32());
            logger("cancellation", "secret", decode.read_int32());
        } else if ( reply.type == 'R' ) {
            logger("authentication", "ok");
        } else if ( reply.type == 'S' ) {
            logger("setting", decode.read_string(), decode.read_string());
        } else if ( reply.type == 'Z' ) {
            logger("", "Connected to Postgres");
            return;
        } else {
            throw fostlib::exceptions::not_implemented(__func__, reply.code());
        }
    }
}


fostlib::pg::response fostlib::pg::connection::impl::read(boost::asio::yield_context &yield) {
    const auto header = pgasio::packet_header(socket, yield);
    fostlib::log::debug(c_fost_pg)
        ("", "Read length and control byte")
        ("code", string() + header.type)
        ("bytes", header.total_size)
        ("body", header.body_size);
    response reply(header.type, header.body_size);
    pgasio::transfer(socket, reply.body, reply.size(), yield);
    if ( reply.type == 'E' ) {
        exceptions::not_implemented error(__func__, "Postgres returned an error");
        decoder decode(reply);
        while ( decode.remaining() > 1 ) {
            switch ( auto control = decode.read_byte() ) {
                /// See the Postgres documentation for the possible values that
                /// are sent here.
                /// https://www.postgresql.org/docs/current/static/protocol-error-fields.html
            default:
                fostlib::insert(error.data(), "Unknown", string() + control, decode.read_string());
            }
        }
        throw error;
    }
    return reply;
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


fostlib::pg::command &fostlib::pg::command::write(utf::u8_view str) {
    for ( std::size_t index{}; index < str.bytes(); ++index )
        byte(str.data()[index]);
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


fostlib::pg::response::response(char c, std::size_t size)
: type(c), body(size) {
}


fostlib::pg::response::~response() = default;
