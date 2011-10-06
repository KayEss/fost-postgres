/*
    Copyright 2008-2011, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <fost/db-driver-sql>
#include <fost/exception/transaction_fault.hpp>
#include <fost/exception/unexpected_eof.hpp>

#include <pqxx/connection>
#include <pqxx/nontransaction>
#include <pqxx/transaction>
#include <pqxx/tuple>


namespace {


    FSL_EXPORT const class pqInterface : public fostlib::sql_driver {
    public:
        pqInterface()
        : sql_driver( L"pqxx" ) {
        }

        void create_database( fostlib::dbconnection &dbc, const fostlib::string &name ) const;
        void drop_database( fostlib::dbconnection &dbc, const fostlib::string &name ) const;

        int64_t next_id( fostlib::dbconnection &dbc, const fostlib::string &counter ) const;
        int64_t current_id( fostlib::dbconnection &dbc, const fostlib::string &counter ) const;
        void used_id( fostlib::dbconnection &dbc, const fostlib::string &counter, int64_t value ) const;

        boost::shared_ptr< fostlib::dbinterface::read > reader( fostlib::dbconnection &dbc ) const;

    protected:
        fostlib::sql::statement mangle( const fostlib::sql::table_name &name ) const {
            fostlib::sql::statement quote( L"\"" );
            return quote + fostlib::sql::statement( name.underlying() ) + quote;
        }
        fostlib::sql::statement mangle( const fostlib::sql::column_name &name ) const {
            fostlib::sql::statement quote( L"\"" );
            return quote + fostlib::sql::statement( name.underlying() ) + quote;
        }
    } c_pqxx_interface;


    class pqRead : public fostlib::dbinterface::read {
        pqxx::connection m_pqcon;
    public:
        boost::scoped_ptr< pqxx::work > m_transaction;

        pqRead( fostlib::dbconnection &d )
        : read( d ), m_pqcon( fostlib::coerce< fostlib::utf8_string >( d.configuration()[ L"read" ].get< fostlib::string >().value() ).underlying().c_str() ) {
            transaction();
        }
        ~pqRead()
        try {
            m_transaction.reset();
        } catch ( ... ) {
            fostlib::absorbException();
        }

        void transaction() {
            m_transaction.reset( new pqxx::transaction<>( m_pqcon ) );
        }

        boost::shared_ptr< fostlib::dbinterface::recordset > query( const fostlib::meta_instance &item, const fostlib::json &key ) const;
        boost::shared_ptr< fostlib::dbinterface::recordset > query( const fostlib::sql::statement &command ) const;

        boost::shared_ptr< fostlib::dbinterface::write > writer();
    };


    class pqRecordset : public fostlib::dbinterface::recordset {
        pqxx::result m_rs;
        pqxx::result::const_iterator m_position;
        std::vector< fostlib::string > m_names;
        mutable std::map<
            pqxx::tuple::size_type,
            fostlib::nullable< fostlib::json >
        > m_fields;
    public:
        pqRecordset(
            const fostlib::dbconnection &dbc,
            const pqRead &reader, const fostlib::sql::statement &cmd
        ) : fostlib::dbinterface::recordset( cmd ),
                m_rs( reader.m_transaction->exec(
                    fostlib::coerce< fostlib::utf8_string >( cmd.underlying() ).underlying()
                ) ),
                m_position( m_rs.begin() ),
                m_names( m_rs.columns() ) {
            for ( pqxx::tuple::size_type p( 0 ); p < m_rs.columns(); ++p )
                m_names[ p ] = fostlib::string( m_rs.column_name( p ) );
        }

        bool eof() const {
            return m_position == m_rs.end();
        }

        void moveNext() {
            ++m_position;
            m_fields.clear();
        }

        std::size_t fields() const {
            return m_rs.columns();
        }

        const fostlib::string &name( std::size_t f ) const {
            if ( f >= fields() )
                throw fostlib::exceptions::out_of_range< std::size_t >( 0, fields(), f );
            return m_names[ f ];
        }

        const fostlib::json &field( std::size_t i ) const {
            try {
                if ( eof() )
                    throw fostlib::exceptions::unexpected_eof( L"Recordset is at EOF" );

                pqxx::tuple::size_type n( i );
                if ( m_fields[ n ].isnull() ) {
                    if ( m_position.at( n ).is_null() )
                        m_fields[ n ] = fostlib::json();
                    else
                        m_fields[ n ] = fostlib::json( m_position[ n ].c_str() );
                }
                return m_fields[ n ].value();
            } catch ( pqxx::range_error &e ) {
                throw fostlib::exceptions::out_of_range< std::size_t >(
                    "Accessing a column number not in the recordset",
                    0, m_fields.size(), i
                );
            }
        }

        const fostlib::json &field( const fostlib::string &name ) const {
            try {
                return field( m_rs.column_number(
                    fostlib::coerce< fostlib::utf8_string >( name ).underlying()
                ) );
            } catch ( fostlib::exceptions::exception &e ) {
                e.info() << L"Whilst fetching column named: " << name << std::endl;
                throw;
            }
        }

        fostlib::json to_json() const {
            throw fostlib::exceptions::not_implemented(
                L"json pqRecordset::to_json() const"
            );
        }
    };


    class pqWrite : public fostlib::sql_driver::write {
        pqRead &m_reader;
        boost::scoped_ptr< pqxx::connection > m_pqcon;
        boost::scoped_ptr< pqxx::work > m_transaction;
    public:
        pqWrite( fostlib::dbconnection &dbc, pqRead &reader )
        : write( reader ), m_reader( reader ) {
            m_reader.m_transaction.reset();
            m_pqcon.reset( new pqxx::connection(
                fostlib::coerce< fostlib::utf8_string >(
                    dbc.configuration()[ L"write" ].get< fostlib::string >().value()
                ).underlying()
            ) );
            m_transaction.reset( new pqxx::work( *m_pqcon ) );
        }

        void drop_table( const fostlib::meta_instance &definition );
        void drop_table( const fostlib::string &table );

        void insert( const fostlib::instance &object );

        void execute( const fostlib::sql::statement &cmd ) {
            try {
                m_transaction->exec(
                    fostlib::coerce< fostlib::utf8_string >(
                        cmd.underlying()
                    ).underlying()
                );
            } catch ( std::exception &e ) {
                throw fostlib::exceptions::transaction_fault( fostlib::string( e.what() ) );
            }
        }

        void commit() {
            m_transaction->commit();
            rollback();
        }

        void rollback() {
            m_transaction.reset();
            m_pqcon.reset();
            m_reader.transaction();
        }
    };


}


/*
    Interface
*/


void pqInterface::create_database( fostlib::dbconnection &dbc, const fostlib::string &name ) const {
    pqxx::connection con(
        fostlib::coerce< fostlib::utf8_string >( dbc.configuration()[ L"write" ].get< fostlib::string >().value() ).underlying()
    );
    pqxx::nontransaction tran( con );
    tran.exec( "CREATE DATABASE \"" + fostlib::coerce< fostlib::utf8_string >( name ).underlying() + "\"" );
}


void pqInterface::drop_database( fostlib::dbconnection &dbc, const fostlib::string &name ) const {
    pqxx::connection con( fostlib::coerce< fostlib::utf8_string >(
        dbc.configuration()[ L"write" ].get< fostlib::string >().value()
    ).underlying() );
    pqxx::nontransaction tran( con );
    tran.exec( "DROP DATABASE \"" + fostlib::coerce< fostlib::utf8_string >( name ).underlying() + "\"" );
}

int64_t pqInterface::next_id( fostlib::dbconnection &dbc, const fostlib::string &counter ) const {
    throw fostlib::exceptions::not_implemented( L":Interface::next_id( fostlib::DBConnection &dbc, const fostlib::string &counter ) const" );
}
int64_t pqInterface::current_id( fostlib::dbconnection &dbc, const fostlib::string &counter ) const {
    throw fostlib::exceptions::not_implemented( L"::Interface::current_id( fostlib::DBConnection &dbc, const fostlib::string &counter ) const" );
}
void pqInterface::used_id( fostlib::dbconnection &dbc, const fostlib::string &counter, int64_t value ) const {
    throw fostlib::exceptions::not_implemented( L"::Interface::used_id( fostlib::DBConnection &dbc, const fostlib::string &counter, int64_t value ) const" );
}


boost::shared_ptr< fostlib::dbinterface::read > pqInterface::reader( fostlib::dbconnection &dbc ) const {
    try {
        return boost::shared_ptr< fostlib::dbinterface::read >( new ::pqRead( dbc ) );
    } catch ( std::exception ) {
        throw;
    }
}


/*
    pqRead
*/


boost::shared_ptr< fostlib::dbinterface::recordset > pqRead::query( const fostlib::meta_instance &item, const fostlib::json &key ) const {
    throw fostlib::exceptions::not_implemented( L"boost::shared_ptr< recordset > pqRead::query( const meta_instance &item, const json &key ) const" );
}


boost::shared_ptr< fostlib::dbinterface::recordset > pqRead::query( const fostlib::sql::statement &command ) const {
    return boost::shared_ptr< fostlib::dbinterface::recordset >( new pqRecordset( m_connection, *this, command ) );
}


boost::shared_ptr< fostlib::dbinterface::write > pqRead::writer() {
    return boost::shared_ptr< fostlib::dbinterface::write >( new pqWrite( m_connection, *this ) );
}


/*
    pqWrite
*/


void pqWrite::drop_table(class fostlib::meta_instance const &) {
    throw fostlib::exceptions::not_implemented( L"pqWrite::drop_table(class fostlib::meta_instance const &)" );
}
void pqWrite::insert(class fostlib::instance const &) {
    throw fostlib::exceptions::not_implemented( L"pqWrite::insert(class fostlib::instance const &)" );
}


void pqWrite::drop_table( const fostlib::string &/*table*/ ) {
    throw fostlib::exceptions::not_implemented( L"void pqWrite::drop_table( const wstring &table ) const" );
}

