/* Copyright (c) 2014, 2017, Oracle and/or its affiliates. All rights reserved.

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

#ifndef RPL_TRANSACTION_WRITE_SET_CTX_H
#define RPL_TRANSACTION_WRITE_SET_CTX_H

#include <stddef.h>
#include <list>
#include <map>
#include <string>
#include <vector>

#include "my_inttypes.h"

/**
  Server side support to provide a service to plugins to report if
  a given transaction should continue or be aborted.
  Its value is reset on Transaction_ctx::cleanup().
  Its value is set through service service_rpl_transaction_ctx.
*/
class Rpl_transaction_write_set_ctx {
 public:
  Rpl_transaction_write_set_ctx();
  virtual ~Rpl_transaction_write_set_ctx() {}

  /*
    Function to add the write set of the hash of the PKE in the std::vector
    in the transaction_ctx object.

    @param[in] hash - the uint64 type hash value of the PKE.
  */
  void add_write_set(uint64 hash);

  /*
    Function to get the pointer of the write set vector in the
    transaction_ctx object.
  */
  std::vector<uint64> *get_write_set();

  /*
    Cleanup function of the vector which stores the PKE.
  */
  void clear_write_set();

  /*
    mark transactions that include tables with no pk
  */
  void set_has_missing_keys();

  /*
    check if the transaction was marked as having missing keys.

    @retval true  The transaction accesses tables with no PK.
    @retval false All tables referenced in transaction have PK.
   */
  bool get_has_missing_keys();

  /*
    mark transactions that include tables referenced by foreign keys
  */
  void set_has_related_foreign_keys();

  /*
    function to check if the transaction was marked as having missing keys.

    @retval true  If the transaction was marked as being referenced by a foreign
    key
  */
  bool get_has_related_foreign_keys();

  /**
    Function to add a new SAVEPOINT identifier in the savepoint map in the
    transaction_ctx object.

    @param[in] name - the identifier name of the SAVEPOINT.
  */
  void add_savepoint(char *name);

  /**
    Function to delete a SAVEPOINT identifier in the savepoint map in the
    transaction_ctx object.

    @param[in] name - the identifier name of the SAVEPOINT.
  */
  void del_savepoint(char *name);

  /**
    Function to delete all data added to write set and savepoint since
    SAVEPOINT identifier was added to savepoinbt in the transaction_ctx object.

    @param[in] name - the identifier name of the SAVEPOINT.
  */
  void rollback_to_savepoint(char *name);

  /**
    Function to push savepoint data to a list and clear the savepoint map in
    order to create another identifier context, needed on functions ant trigger.
  */
  void reset_savepoint_list();

  /**
    Restore previous savepoint map context, called after executed trigger or
    function.
  */
  void restore_savepoint_list();

 private:
  std::vector<uint64> write_set;
  bool m_has_missing_keys;
  bool m_has_related_foreign_keys;

  /**
    Contains information related to SAVEPOINTs. The key on map is the
    identifier and the value is the size of write set when command was
    executed.
  */
  std::map<std::string, size_t> savepoint;

  /**
    Create a savepoint context hierarchy to support encapsulation of
    identifier name when function or trigger are executed.
  */
  std::list<std::map<std::string, size_t>> savepoint_list;
};

#endif /* RPL_TRANSACTION_WRITE_SET_CTX_H */
