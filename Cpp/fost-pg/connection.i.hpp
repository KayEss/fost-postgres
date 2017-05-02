/*
    Copyright 2017, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#pragma once

#include <fost/unicode>

#include <boost/asio/local/stream_protocol.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/endian/conversion.hpp>


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


        struct decoder {
            array_view<unsigned char> buffer;

            std::size_t remaining() {
                return buffer.size();
            }

            unsigned char read_byte() {
                if ( not buffer.size() ) throw exceptions::not_implemented(__func__,
                    "No bytes remaining");
                const auto byte = buffer[0];
                buffer = buffer.slice(1);
                return byte;
            }

            int16_t read_int16() {
                return (read_byte() << 8) + read_byte();
            }
            int32_t read_int32() {
                return (read_byte() << 24) + (read_byte() << 16)
                    + (read_byte() << 8) + read_byte();
            }

            utf::u8_view read_u8_view(std::size_t bytes) {
                if ( buffer.size() < bytes ) throw exceptions::not_implemented(__func__,
                    "No bytes remaining");
                auto ret = buffer.slice(0, bytes);
                buffer = buffer.slice(bytes);
                return ret;
            }
            utf::u8_view read_u8_view() {
                if ( not buffer.size() ) throw exceptions::not_implemented(__func__,
                    "No bytes remaining");
                auto start = buffer;
                while ( read_byte() != 0 );
                return array_view<unsigned char>(start.data(), buffer.data() - start.data() - 1);
            }
            string read_string() {
                return read_u8_view();
            }
        };


        struct response {
            char type;
            std::vector<unsigned char> body;

            response()
            : type{} {
            }
            response(char code, std::size_t size);

            ~response();

            /// Make movable only
            response(const response &) = delete;
            response &operator = (const response &) = delete;
            response(response &&) = default;
            response &operator = (response &&) = default;

            utf::u8_view code() const {
                return array_view<unsigned char>(
                    reinterpret_cast<const unsigned char *>(&type), 1);
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
            impl(boost::asio::io_service &, const char *loc, utf::u8_view user,
                 utf::u8_view database, boost::asio::yield_context &);

            boost::asio::local::stream_protocol::socket socket;

            response read(boost::asio::yield_context &);
        };


    }
}

