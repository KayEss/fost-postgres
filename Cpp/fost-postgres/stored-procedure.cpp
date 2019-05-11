/*
    Copyright 2016-2019, Felspar Co Ltd. <http://support.felspar.com/>

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
*/


#include <fost/pg/recordset.hpp>
#include <fost/pg/stored-procedure.hpp>
#include "connection.hpp"
#include "recordset.hpp"


fostlib::pg::unbound_procedure::unbound_procedure(
        fostlib::pg::connection &c, std::string n)
: cnx(c), name(n) {}


fostlib::pg::recordset fostlib::pg::unbound_procedure::exec(
        const std::vector<fostlib::string> &args) {
    auto sp = cnx.pimpl->trans->prepared(name);
    for (const auto &a : args) sp(static_cast<std::string>(a));
    return recordset(std::make_unique<recordset::impl>(sp.exec()));
}


fostlib::pg::recordset fostlib::pg::unbound_procedure::exec(
        const std::vector<fostlib::json> &args) {
    auto sp = cnx.pimpl->trans->prepared(name);
    for (const auto &a : args) {
        if (a.isnull()) {
            sp();
        } else {
            sp(fostlib::coerce<fostlib::string>(a).shrink_to_fit());
        }
    }
    return recordset(std::make_unique<recordset::impl>(sp.exec()));
}
