/* Copyright (c) 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef MYSQL_CONNECTION_ATTRIBUTES_ITERATOR_IMP_H
#define MYSQL_CONNECTION_ATTRIBUTES_ITERATOR_IMP_H

#include <mysql/components/service_implementation.h>
#include <mysql/components/services/mysql_connection_attributes_iterator.h>

/**
  @class mysql_thd_variables_service_imp

  Thread variables' service implementation.

  This service provides functions for plugins and storage engines to
  obtain the thread's THD variables.

  @sa mysql_connection_attributes_iterator
*/

class mysql_connection_attributes_iterator_imp {
 public:
  /**
    Initialize an iterator.

    Also position at the first attribute.

    @param opaque_thd     The session to operate on. Can be null to use the
                          current THD.
    @param[out] iterator  Iterator pointer.

    @return
      @retval false Succeeded.
      @retval true  Failed.

    @sa mysql_connection_attributes_iterator::init
    */
  static DEFINE_BOOL_METHOD(init,
                            (MYSQL_THD opaque_thd,
                             my_h_connection_attributes_iterator *iterator));

  /**
    Deinitialize an iterator.

    @param iterator Iterator pointer.

    @return
      @retval false Succeeded.
      @retval true  Failed.

    @sa mysql_connection_attributes_iterator::deinit
  */
  static DEFINE_BOOL_METHOD(deinit,
                            (my_h_connection_attributes_iterator iterator));

  /**
    Fetch the current name/value pair from the iterator and move it forward.
    Note: the attribute's name and value pointers are valid until the THD
    object is alive.

    @param      opaque_thd      The session to operate on. Can be NULL to use
                                the current THD.
    @param      iterator        Iterator pointer.
    @param[out] name            The attribute name.
    @param[out] name_length     The attribute name's length.
    @param[out] value           The attribute value.
    @param[out] value_length    The attribute value's length.
    @param[out] client_charset  The character set, used for encoding the
                                connection attributes pair

    @return
      @retval false Succeeded.
      @retval true  Failed.

    @sa mysql_connection_attributes_iterator::get
    */
  static DEFINE_BOOL_METHOD(get, (MYSQL_THD opaque_thd,
                                  my_h_connection_attributes_iterator *iterator,
                                  const char **name, size_t *name_length,
                                  const char **value, size_t *value_length,
                                  const char **client_charset));
};

#endif /* !MYSQL_CONNECTION_ATTRIBUTES_ITERATOR_IMP_H */
