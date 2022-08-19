/* Copyright (c) 2019, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef MYSQL_CONNECTION_ATTRIBUTES_ITERATOR_H
#define MYSQL_CONNECTION_ATTRIBUTES_ITERATOR_H

#include <mysql/components/service.h>
#include <mysql/components/services/mysql_current_thread_reader.h>  // MYSQL_THD
#include <stddef.h>

/**
  @ingroup group_components_services_inventory

  A service to read the connection attributes from the current session
  It provides a read-only iterator over the attributes.
  This is some example code to use the iterator:

  @code

  // at init time. try to reuse the service handles as much as possible

  my_service<SERVICE_TYPE(mysql_current_thread_reader)> thd_reader(
     "mysql_current_thread_reader", m_reg_srv);
   my_service<SERVICE_TYPE(mysql_connection_attributes_iterator)> service(
      "mysql_connection_attributes_iterator", m_reg_srv);

  if (!service.is_valid() || !thd_reader.is_valid) {
    return; //error
  }

      ...

  // at parse time

  MYSQL_THD thd;
  if (thd_reader->get(&thd))
    return; //error

  my_h_connection_attributes_iterator iterator;

  MYSQL_LEX_CSTRING name;
  MYSQL_LEX_CSTRING value;
  const char *charset_string;
  const CHARSET_INFO *charset = nullptr;

  my_h_connection_attributes_iterator iterator;
  if (service->init(thd, &iterator)) return;  // error

  while (!service->get(thd, &iterator, &name.str, &name.length, &value.str,
                        &value.length, &charset_string)) {
    // Do something with name and value
  }

  service->deinit(iterator);

  @endcode
*/

typedef const char *my_h_connection_attributes_iterator;

BEGIN_SERVICE_DEFINITION(mysql_connection_attributes_iterator)

/**
  Initialize an iterator.

  Also position at the first attribute.

  @param      thd             The session to operate on. Can be NULL to use the
                              current THD.
  @param[out] iterator  Iterator pointer.

  @return
    @retval false Succeeded.
    @retval true  Failed.

  @sa mysql_connection_attributes_iterator_imp::init
*/
DECLARE_BOOL_METHOD(init, (MYSQL_THD thd,
                           my_h_connection_attributes_iterator *iterator));

/**
  Deinitialize an iterator.

  @param iterator Iterator pointer.

  @return
    @retval false Succeeded.
    @retval true  Failed.

  @sa mysql_connection_attributes_iterator_imp::deinit
*/
DECLARE_BOOL_METHOD(deinit, (my_h_connection_attributes_iterator iterator));

/**
  Fetch the current name/value pair from the iterator and move it forward.
  Note: the attribute's name and value pointers are valid until the THD object
  is alive.

  @param      thd             The session to operate on. Can be NULL to use the
                              current THD.
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

  @sa mysql_connection_attributes_iterator_imp::get
*/
DECLARE_BOOL_METHOD(get,
                    (MYSQL_THD thd,
                     my_h_connection_attributes_iterator *iterator,
                     const char **name, size_t *name_length, const char **value,
                     size_t *value_length, const char **client_charset));

END_SERVICE_DEFINITION(mysql_connection_attributes_iterator)

#endif /* MYSQL_CONNECTION_ATTRIBUTES_ITERATOR_H */
