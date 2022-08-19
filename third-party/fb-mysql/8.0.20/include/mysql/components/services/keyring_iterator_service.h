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

#ifndef KEYRING_ITERATOR_H
#define KEYRING_ITERATOR_H

#include <mysql/components/service.h>
#include <mysql/components/services/dynamic_privilege.h>

DEFINE_SERVICE_HANDLE(my_h_keyring_iterator);

/**
  @ingroup group_components_services_inventory

  Keyring iterator component service definition, which enables to iterate over
  items stored within currently used keyring.

  @code
   my_service<SERVICE_TYPE(mysql_keyring_iterator)> service(
      "mysql_keyring_iterator.mysql_server", m_reg_srv);

  if (!service.is_valid()) {
    return;
  }

  my_h_keyring_iterator iterator = nullptr;

  if (service->init(&iterator)) {
    return;
  }

  char key_id[64];
  char user_id[64];

  while (iterator != nullptr && service->get(iterator, key_id, sizeof(key_id)
  user_id, sizeof(user_id))
  == 0) {
    // Do something with key and user_id values.
  }

  service->deinit(iterator);
  @endcode
*/
BEGIN_SERVICE_DEFINITION(mysql_keyring_iterator)

/**
  Initialize an iterator.

  @param[out] iterator Iterator pointer.

  @return
    @retval false Succeeded.
    @retval true  Failed.

  @sa plugin_keyring.h
*/
DECLARE_BOOL_METHOD(init, (my_h_keyring_iterator * iterator));

/**
  Deinitialize an iterator.

  @param iterator Iterator pointer.

  @return
    @retval false Succeeded.
    @retval true  Failed.

  @sa plugin_keyring.h
*/
DECLARE_BOOL_METHOD(deinit, (my_h_keyring_iterator iterator));

/**
  Fetch key info stored under key iterator and move it forward.

  @param      iterator     Iterator pointer.
  @param[out] key_id       The buffer pointer for storing key id.
  @param      key_id_size  Size of the key_id buffer.
  @param[out] user_id      The buffer pointer for storing user id. In case
                           of lack of user id, empty string is returned.
  @param      user_id_size Size of the user_id buffer.

  @return
    @retval false Succeeded.
    @retval true  Failed.

  @sa plugin_keyring.h
*/
DECLARE_BOOL_METHOD(get,
                    (my_h_keyring_iterator iterator, char *key_id,
                     size_t key_id_size, char *user_id, size_t user_id_size));

END_SERVICE_DEFINITION(mysql_keyring_iterator)

#endif /* KEYRING_ITERATOR_H */
