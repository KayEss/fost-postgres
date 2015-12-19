/*
    Copyright 2015, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <fost/pg/recordset-range.hpp>


/*
    fostlib::pg::recordset_range
*/


fostlib::pg::recordset_range::recordset_range() {
}


fostlib::pg::recordset_range::const_iterator fostlib::pg::recordset_range::begin() const {
    return fostlib::pg::recordset_range::const_iterator();
}


/*
    fostlib::pg::recordset_range::const_iterator
*/


fostlib::pg::recordset_range::record_type *fostlib::pg::recordset_range::const_iterator::operator -> () const {
    return &row;
}

