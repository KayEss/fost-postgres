/*
    Copyright 2015-2016, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <fost/pg/connection.hpp>
#include <fost/pg/recordset.hpp>
#include <fost/pg/stored-procedure.hpp>
#include "connection.hpp"

#include <atomic>
#include <fost/insert>



/*
    fostlib::pg::connection
*/


fostlib::pg::connection::connection()
: pimpl(new impl(utf8_string())) {
}
fostlib::pg::connection::connection(const string &host)
: pimpl(new impl(coerce<utf8_string>("host=" + host))) {
}
fostlib::pg::connection::connection(const string &host, const string &db)
: pimpl(new impl(coerce<utf8_string>("host=" + host + " dbname=" + db))) {
}
namespace {
    std::pair<fostlib::utf8_string, fostlib::json> from_json(const fostlib::json &conf) {
        fostlib::utf8_string dsn;
        fostlib::json effective;
        for ( auto &key : {"host", "dbname", "user"} ) {
            if ( conf.has_key(key) ) {
                fostlib::insert(effective, key, conf[key]);
                dsn += fostlib::utf8_string(key) + "='" +
                    fostlib::coerce<fostlib::utf8_string>(
                        fostlib::coerce<fostlib::string>(conf[key])) + "' ";
            }
        }
        return std::make_pair(dsn, effective);
    }
}
fostlib::pg::connection::connection(const json &conf)
: pimpl(new impl(from_json(conf))) {
}


fostlib::pg::connection::~connection() = default;


const fostlib::json &fostlib::pg::connection::configuration() const {
    return pimpl->configuration;
}


fostlib::pg::recordset fostlib::pg::connection::exec(const utf8_string &sql) {
    return std::move(recordset(*pimpl, sql));
}


void fostlib::pg::connection::commit() {
    pimpl->trans->commit();
    pimpl->trans = std::make_unique<impl::transaction_type>(pimpl->pqcnx);
}


namespace {
    fostlib::string column(const fostlib::json &name) {
        return '"' + fostlib::coerce<fostlib::string>(name) + '"';
    }
    fostlib::string columns(fostlib::string cols, const fostlib::json &def) {
        for ( fostlib::json::const_iterator iter(def.begin()); iter != def.end(); ++iter ) {
            if ( cols.empty() ) {
                cols = column(iter.key());
            } else {
                cols += ", " + column(iter.key());
            }
        }
        return cols;
    }
    fostlib::string columns(const fostlib::json &def) {
        return columns(fostlib::string(), def);
    }

    template<typename T>
    fostlib::string value(T &t, const fostlib::json &val) {
        return t.quote(fostlib::coerce<fostlib::string>(val).std_str());
    }
    template<typename T>
    fostlib::string value_string(T &t, fostlib::string vals, const fostlib::json &def) {
        for ( const auto &val : def ) {
            if ( vals.empty() ) {
                vals = value(t, val);
            } else {
                vals += ", " + value(t, val);
            }
        }
        return vals;
    }
    template<typename T>
    fostlib::string value_string(T &t, const fostlib::json &def) {
        return value_string(t, fostlib::string(), def);
    }
}


fostlib::pg::recordset fostlib::pg::connection::select(const char *relation, const json &values) {
    string select = "SELECT * FROM ", where;
    select += relation;
    for ( fostlib::json::const_iterator iter(values.begin()); iter != values.end(); ++iter ) {
        if ( where.empty() ) {
            where = column(iter.key()) + " = " + value(*pimpl->trans, *iter);
        } else {
            where += " AND " + column(iter.key()) + " = " + value(*pimpl->trans, *iter);
        }
    }
    if ( not where.empty() ) {
        select += " WHERE " + where;
    }
    return exec(coerce<utf8_string>(select));
}


fostlib::pg::connection &fostlib::pg::connection::insert(const char *relation, const json &values) {
    exec(coerce<utf8_string>(
        string("INSERT INTO ") + relation +
            " (" + columns(values) + ") VALUES (" + value_string(*pimpl->trans, values) + ")"));
    return *this;
}


fostlib::pg::connection &fostlib::pg::connection::update(
    const char *relation, const json &keys, const json &values
) {
    string sql("UPDATE "), updates, where;
    sql += relation;
    sql += " SET ";
    for ( fostlib::json::const_iterator iter(values.begin()); iter != values.end(); ++iter ) {
        if ( updates.empty() ) {
            updates = column(iter.key()) + "=" + value(*pimpl->trans, *iter);
        } else {
            updates += ", " + column(iter.key()) + "=" + value(*pimpl->trans, *iter);
        }
    }
    for ( fostlib::json::const_iterator iter(keys.begin()); iter != keys.end(); ++iter ) {
        if ( where.empty() ) {
            where = column(iter.key()) + "=" + value(*pimpl->trans, *iter);
        } else {
            where += " AND " + column(iter.key()) + "=" + value(*pimpl->trans, *iter);
        }
    }
    sql += updates + " WHERE " + where;
    exec(coerce<utf8_string>(sql));
    return *this;
}


fostlib::pg::connection &fostlib::pg::connection::upsert(
    const char *relation, const json &keys, const json &values
) {
    string sql("INSERT INTO "),
        key_names(columns(keys)),
        value_names(columns(key_names, values)),
        updates;
    sql += relation;
    sql += " (" + value_names + ") VALUES "
        "(" + value_string(*pimpl->trans, value_string(*pimpl->trans, keys), values) + ") ";
    sql += "ON CONFLICT (" + key_names + ") DO ";
    for ( fostlib::json::const_iterator iter(values.begin()); iter != values.end(); ++iter ) {
        if ( updates.empty() ) {
            updates = column(iter.key()) + " = EXCLUDED." + column(iter.key());
        } else {
            updates += ", " + column(iter.key()) + " = EXCLUDED." + column(iter.key());
        }
    }
    if ( updates.empty() ) {
        sql += "NOTHING";
    } else {
        sql += "UPDATE SET " + updates;
    }
    exec(coerce<utf8_string>(sql));
    return *this;
}


fostlib::pg::unbound_procedure fostlib::pg::connection::procedure(
    const fostlib::utf8_string &cmd
) {
    static std::atomic<unsigned int> number;
    std::string name = "sp_anon_" + std::to_string(++number);
    pimpl->pqcnx.prepare(name, cmd.underlying());
    return unbound_procedure(*this, name);
}

