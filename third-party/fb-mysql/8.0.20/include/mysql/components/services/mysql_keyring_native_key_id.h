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

#ifndef MYSQL_KEYRING_NATIVE_KEY_ID_H
#define MYSQL_KEYRING_NATIVE_KEY_ID_H

/**
  @file
  implementation for mysql_keyring_native_key_id service
 */

#include <mysql/components/my_service.h>

/**
  @ingroup group_components_services_inventory

  A service to read native keybackend id

  Typically there'll be just one implementation of this by the main
  application.
*/
BEGIN_SERVICE_DEFINITION(mysql_keyring_native_key_id)
/**
  Read a backend key id

  @param key_id the mysql key id
  @param user_id the mysql user id
  @param out backend_key_id a buffer to store the backend key id in
  @param inout key_len: on input the size of the buffer in backend_key_id, on
  output: the number of chars returned in the buffer
  @return Status of performed operation
  @retval false success (valid password)
  @retval true failure (invalid password)

  @sa my_host_application_signal
*/
DECLARE_BOOL_METHOD(get_backend_key_id,
                    (const char *key_id, const char *user_id,
                     char *backend_key_id, size_t max_key_len));

END_SERVICE_DEFINITION(mysql_keyring_native_key_id)

#endif /* MYSQL_KEYRING_NATIVE_KEY_ID_H */
