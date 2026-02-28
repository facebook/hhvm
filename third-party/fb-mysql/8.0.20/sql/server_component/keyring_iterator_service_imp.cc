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

#include "keyring_iterator_service_imp.h"

int my_key_iterator_init(void **);
int my_key_iterator_deinit(void *);
int my_key_iterator_get_key(void *, char *, char *);

void mysql_keyring_iterator_service_init() { return; }

DEFINE_BOOL_METHOD(mysql_keyring_iterator_imp::init,
                   (my_h_keyring_iterator * iterator)) {
  return my_key_iterator_init(reinterpret_cast<void **>(iterator)) != 0;
}

DEFINE_BOOL_METHOD(mysql_keyring_iterator_imp::deinit,
                   (my_h_keyring_iterator iterator)) {
  return my_key_iterator_deinit(reinterpret_cast<void *>(iterator)) != 0;
}

DEFINE_BOOL_METHOD(mysql_keyring_iterator_imp::get,
                   (my_h_keyring_iterator iterator, char *key_id,
                    size_t key_id_size, char *user_id, size_t user_id_size)) {
  if (key_id_size < 64 || user_id_size < 64) return true;

  return my_key_iterator_get_key(reinterpret_cast<void *>(iterator), key_id,
                                 user_id) != 0;
}
