/*
    Copyright 2016-2019, Felspar Co Ltd. <http://support.felspar.com/>

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
*/


#include <fost/pg/recordset.hpp>
#include <fost/pg/stored-procedure.hpp>
#include "connection.hpp"
#include "recordset.hpp"
#include <pqxx/prepared_statement>


namespace {
    template<typename Coll, typename Func>
    auto exec_prepared(
            pqxx::transaction<pqxx::serializable> &trans,
            std::string const &name,
            Coll &collection,
            Func transform) {
        return trans.exec_prepared(
                name,
                pqxx::prepare::make_dynamic_params(collection, transform));
    }
}


fostlib::pg::unbound_procedure::unbound_procedure(
        fostlib::pg::connection &c, std::string n)
: cnx(c), name(n) {}


fostlib::pg::recordset fostlib::pg::unbound_procedure::exec(
        std::vector<fostlib::string> args) {
    return recordset(std::make_unique<recordset::impl>(exec_prepared(
            *cnx.pimpl->trans, name, args,
            [](fostlib::string &arg) { return arg.shrink_to_fit(); })));
}


fostlib::pg::recordset fostlib::pg::unbound_procedure::exec(
        const std::vector<fostlib::json> &jsargs) {
    std::vector<std::optional<std::string>> args;
    std::transform(
            jsargs.begin(), jsargs.end(), std::back_inserter(args),
            [](fostlib::json arg) -> std::optional<std::string> {
                if (arg.isnull()) {
                    return {};
                } else {
                    return static_cast<std::string>(
                            fostlib::coerce<fostlib::string>(arg));
                }
            });
    return recordset(std::make_unique<recordset::impl>(
            exec_prepared(*cnx.pimpl->trans, name, args, [](auto &arg) {
                return arg.has_value() ? arg.value().c_str() : nullptr;
            })));
}
