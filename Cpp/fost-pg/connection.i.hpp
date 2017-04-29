/*
    Copyright 2017, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


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

            /// Send the command to the server
            void send(boost::asio::local::stream_protocol::socket &, boost::asio::yield_context &);
        };


        struct response {
            response(char code);
            response(response &&) = default;

            char code;
            std::unique_ptr<boost::asio::streambuf> body;
        };


        struct connection::impl {
            impl(boost::asio::io_service &, f5::lstring, boost::asio::yield_context &);

            boost::asio::local::stream_protocol::socket socket;

            response read(boost::asio::yield_context &);
        };


    }
}

