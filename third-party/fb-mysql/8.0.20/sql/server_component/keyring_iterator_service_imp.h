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

#ifndef KEYRING_ITERATOR_IMP_H
#define KEYRING_ITERATOR_IMP_H

#include <mysql/components/service_implementation.h>
#include <mysql/components/services/keyring_iterator_service.h>

/**
  @class mysql_keyring_iterator_imp

  Keyring iterator component service implementation.
*/
class mysql_keyring_iterator_imp {
 public:
  /**
    Initialize an iterator.

    @param[out] iterator Iterator pointer.

    @return
      @retval false Succeeded.
      @retval true  Failed.

    @sa plugin_keyring.h
  */
  static DEFINE_BOOL_METHOD(init, (my_h_keyring_iterator * iterator));

  /**
    Deinitialize an iterator.

    @param iterator Iterator pointer.

    @return
      @retval false Succeeded.
      @retval true  Failed.

    @sa plugin_keyring.h
  */
  static DEFINE_BOOL_METHOD(deinit, (my_h_keyring_iterator iterator));

  /**
    Fetch key info stored under key iterator and move it forward.

    @param      iterator     Iterator pointer.
    @param[out] key_id       The buffer pointer for storing key id.
    @param      key_id_size  key_id buffer size. Value must not be less than 64
    bytes.
    @param[out] user_id      The buffer pointer for storing user id.
    @param      user_id_size user_id buffer size. Value must not be less than 64
    bytes.

    @return
      @retval false Succeeded.
      @retval true  Failed.

    @sa plugin_keyring.h
  */
  static DEFINE_BOOL_METHOD(get, (my_h_keyring_iterator iterator, char *key_id,
                                  size_t key_id_size, char *user_id,
                                  size_t user_id_size));
};

#endif /* KEYRING_ITERATOR_IMP_H */
