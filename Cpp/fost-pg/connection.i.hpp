/*
    Copyright 2017, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#pragma once

#include <pgasio/connection.hpp>
#include <fost/unicode>

#include <boost/asio/local/stream_protocol.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/endian/conversion.hpp>

#include <fost/pg/connection.hpp>


namespace fostlib {
    namespace pg {


        /// A command being sent to Postgres
        struct command {
            /// Used only for the initial connect instruction because this one
            /// has no command value
            command();
            /// Used for normal commands
            command(char instruction);

            boost::asio::streambuf header, buffer;

            /// Add the specified bytes to the buffer
            template<typename B, typename = std::enable_if_t<std::is_integral<B>::value>>
            command &bytes(array_view<B> av) {
                static_assert(sizeof(B) == 1, "Must add an array of bytes");
                buffer.sputn(av.data(), av.size());
                return *this;
            }

            /// Add a single byte to the buffer
            template<typename B, typename = std::enable_if_t<std::is_integral<B>::value>>
            command &byte(B b) {
                static_assert(sizeof(B) == 1, "Must add a single byte");
                buffer.sputc(b);
                return *this;
            }

            template<typename I, typename = std::enable_if_t<std::is_integral<I>::value>>
            command &write(I i) {
                if ( sizeof(i) > 1 ) { // TODO: Should be constexpr if
                    auto v = boost::endian::native_to_big(i);
                    return bytes(array_view<char>(reinterpret_cast<char*>(&v), sizeof(v)));
                } else {
                    return byte(char(i));
                }
            }

            command &write(const char *);
            command &write(utf::u8_view);

            /// Send the command to the server
            void send(boost::asio::local::stream_protocol::socket &, boost::asio::yield_context &);
        };


        struct decoder : public pgasio::decoder {
            decoder(pgasio::byte_view b)
            : pgasio::decoder(b) {
            }

            utf::u8_view read_u8_view(std::size_t bytes) {
                const auto view = read_bytes(bytes);
                return array_view<unsigned char>(view.data(), view.size());
            }
            utf::u8_view read_u8_view() {
                const auto view = read_string_view();
                return array_view<unsigned char>(view.data(), view.size());
            }
            string read_string() {
                return read_u8_view();
            }
        };


        struct response {
            pgasio::header header;
            std::vector<unsigned char> body;

            response()
            : header{} {
            }
            template<typename S>
            response(pgasio::header h, S &socket, boost::asio::yield_context&yield)
            : header{h}, body{h.packet_body(socket, yield)} {
            }

            ~response();

            /// Make movable only
            response(const response &) = delete;
            response &operator = (const response &) = delete;
            response(response &&) = default;
            response &operator = (response &&) = default;

            utf::u8_view code() const {
                return array_view<unsigned char>(
                    reinterpret_cast<const unsigned char *>(&header.type), 1);
            }
            unsigned char *data() {
                return body.data();
            }
            std::size_t size() {
                return body.size();
            }

            operator decoder () const {
                return decoder{body};
            }
        };


        struct connection::impl {
            impl(boost::asio::io_service &, f5::lstring, boost::asio::yield_context &);
            impl(boost::asio::io_service &, const char *loc, const char *user,
                 utf::u8_view database, boost::asio::yield_context &);

            boost::asio::local::stream_protocol::socket socket;

            response read(boost::asio::yield_context &);
        };


    }
}

