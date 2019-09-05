/**
    Copyright 2015-2019 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#pragma once


#include <fost/pg/connection.hpp>
#include <pqxx/connection>
#include <pqxx/transaction>


struct fostlib::pg::connection::impl {
    pqxx::connection pqcnx;
    using transaction_type = pqxx::transaction<pqxx::serializable>;
    std::unique_ptr<transaction_type> trans;

    json configuration;

    impl(const fostlib::utf8_string &dsn)
    : pqcnx(static_cast<std::string>(dsn)),
      trans(new transaction_type(pqcnx)),
      configuration(dsn) {}

    impl(const std::pair<fostlib::utf8_string, fostlib::json> &dsn)
    : pqcnx(static_cast<std::string>(dsn.first)),
      trans(new transaction_type(pqcnx)),
      configuration(dsn.second) {}
};
