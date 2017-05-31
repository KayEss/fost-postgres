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

#include <cstdlib>


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
            user.value_or(std::getenv("LOGNAME")).c_str(), database.value_or("").c_str(), yield));
    }));
    s.wait();
    exec("BEGIN TRANSACTION ISOLATION LEVEL SERIALIZABLE");
}


fostlib::pg::connection::~connection() = default;


fostlib::pg::recordset fostlib::pg::connection::exec(const utf8_string &sql) {
    f5::sync s;
    auto rsp = std::make_unique<recordset::impl>(*pimpl);
    boost::asio::spawn(pimpl->socket.get_io_service(),
        [s = &s, this, &sql, rs=rsp.get()](auto yield)
    {
        command query{'Q'};
        query.c_str(sql.underlying().c_str());
        query.send(pimpl->socket, yield);
        while ( true ) {
            auto header = pgasio::message_header(pimpl->socket, yield);
            if ( header.type == 'C' ) {
                response(header, pimpl->socket, yield);
                auto zed = pgasio::message_header(pimpl->socket, yield);
                if ( zed.type == 'Z' ) {
                    response(zed, pimpl->socket, yield);
                    s->done();
                    return;
                } else {
                    throw exceptions::not_implemented(__func__,
                        "Expected Z message after recordset end (C) message");
                }
            } else if ( header.type == 'D' ) {
                const auto columns = rs->column_meta.size();
                auto blocks = std::make_shared<f5::boost_asio::channel<pgasio::record_block>>(
                    pimpl->socket.get_io_service(), 3);
                pgasio::record_block block{rs->column_meta.size(), header.body_size, header.body_size};
                auto next_block_size = block.read_rows(pimpl->socket, header.body_size, yield);
                auto transform = [columns, rs](pgasio::record_block &block) {
                    recordset::impl::record_data data;
                    const std::size_t rows = block.fields().size() / columns;
                    data.reserve(rows);
                    for ( auto fields = block.fields(); fields.size(); fields = fields.slice(columns) ) {
                        std::vector<fostlib::json> row;
                        row.reserve(columns);
                        rs->decode_fields(row, fields.slice(0, columns));
                        data.push_back(std::move(row));
                    }
                    return data;
                };
                rs->record_block = transform(block);;
                rs->records = rs->record_block;
                s->done();
                boost::asio::spawn(pimpl->socket.get_io_service(), [rs, blocks, transform](auto yield) {
                    while ( true ) {
                        auto block = blocks->consume(yield);
                        if ( block.fields().data() == nullptr ) {
                            blocks->close();
                        }
                        rs->records_data.produce(transform(block), yield);
                    }
                });
                while ( next_block_size ) {
                    pgasio::record_block block(rs->column_meta.size());
                    next_block_size = block.read_rows(pimpl->socket, next_block_size, yield);
                    blocks->produce(std::move(block), yield);
                }
                blocks->produce(pgasio::record_block{}, yield);
                return;
            } else if ( header.type == 'T' ) {
                rs->row_description(response(header, pimpl->socket, yield));
            } else {
                throw exceptions::not_implemented(__func__, fostlib::string() + header.type);
            }
        }
    });
    s.wait();
    return recordset(std::move(rsp));
}


/*
 * fostlib::pg::connection::impl
 */


fostlib::pg::connection::impl::impl(
    boost::asio::io_service &ios, f5::lstring loc, boost::asio::yield_context &yield
) : impl(ios, loc.c_str(), std::getenv("LOGNAME"), "", yield) {
}
fostlib::pg::connection::impl::impl(
    boost::asio::io_service &ios, const char *loc, const char *user,
    const char *database, boost::asio::yield_context &yield
) : cnx(pgasio::handshake(pgasio::unix_domain_socket(ios, loc, yield), user, database, yield)),
    socket(cnx.socket)
{
    auto logger = fostlib::log::info(c_fost_pg);
    logger
        ("", "Connected to unix domain socket")
        ("path", loc)
        ("dbname", database)
        ("user", user)
        ("cancellation", "process-id", cnx.process_id)
        ("cancellation", "secret", cnx.secret);
    for ( const auto &setting : cnx.settings  ) {
        logger("setting", setting.first.c_str(), utf::u8_view(setting.second));
    }
}


fostlib::pg::response fostlib::pg::connection::impl::read(boost::asio::yield_context &yield) {
    try {
        const auto header = pgasio::message_header(socket, yield);
        fostlib::log::debug(c_fost_pg)
            ("", "Read length and control byte")
            ("code", string() + header.type)
            ("bytes", header.total_size)
            ("body", header.body_size);
        return response{header, socket, yield};
    } catch ( pgasio::postgres_error &e ) {
        exceptions::not_implemented error(__func__, "Postgres returned an error");
        for ( const auto &m : e.messages ) {
            switch ( auto control = m.first ) {
            default:
                fostlib::insert(error.data(), "Unknown", string() + m.first, string(m.second));
            }
        }
        throw error;
    }
}


/*
 * fostlib::pg::command
 */


fostlib::pg::command::command()
: pgasio::command(0) {
}


fostlib::pg::command::command(char c)
: pgasio::command(c) {
}


/*
 * fostlib::pg::response
 */


fostlib::pg::response::~response() = default;
