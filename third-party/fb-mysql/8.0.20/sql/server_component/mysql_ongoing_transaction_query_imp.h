/* Copyright (c) 2018, 2020, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#ifndef MYSQL_ONGOING_TRANSACTION_INCLUDED
#define MYSQL_ONGOING_TRANSACTION_INCLUDED

#include <mysql/components/component_implementation.h>
#include <mysql/components/services/ongoing_transaction_query_service.h>

/**
  An implementation of mysql_ongoing_transactions_query service methods
*/
class mysql_ongoing_transactions_query_imp {
 public:
  /**
    Service that returns the list of thread ids for the currently running
    transactions in the server

    @param[out] thread_ids The list of returned thread ids
    @param[out] length     The number of transactions returned in the list

    @note the caller of this method must free the memory allocated for the list

    @return false if everything when fine, true in case of failure
  */
  static DEFINE_BOOL_METHOD(get_ongoing_server_transactions,
                            (unsigned long **thread_ids,
                             unsigned long *length));
};

#endif /* MYSQL_ONGOING_TRANSACTION_INCLUDED */
