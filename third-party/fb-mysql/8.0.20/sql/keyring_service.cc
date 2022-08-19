/*  Copyright (c) 2016, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include <stddef.h>

#include <functional>
#include "my_inttypes.h"
#include "mysql/plugin.h"
#include "mysql/plugin_keyring.h" /* keyring plugin */
#include "sql/current_thd.h"
#include "sql/set_var.h"
#include "sql/sql_plugin.h"
#include "sql/sql_plugin_ref.h"

/**
  @class Callback

  @brief Class that stores callback function reference as well as the result
         of the callback function call (invoke method). Callback is called
         using the plugin descriptor pointer, so the callback can call plugin
         exposed function.
*/
class Callback {
 public:
  /**
    Constructor.

    @param callback Lambda function that is called using the invoke method.
  */
  explicit Callback(std::function<bool(st_mysql_keyring *keyring)> callback)
      : m_callback(callback), m_result(true) {}

  /**
    Invoke the underlying callback using the specified parameter and store
    the result of the operation.

    @param keyring Keyring plugin descriptor pointer.
  */
  void invoke(st_mysql_keyring *keyring) { m_result = m_callback(keyring); }

  /**
    Result of the invoke operation.

    @return Result of the invoke operation.
  */
  bool result() { return m_result; }

 private:
  /**
    Callback function.
  */
  const std::function<bool(st_mysql_keyring *keyring)> m_callback;

  /**
    Result of the _callback function call.
  */
  bool m_result;
};

/**
  Callback function that is called on the plugin.

  @param plugin Plugin reference.
  @param arg    Opaque Callback pointer.

  @return This function always returns true.
*/
static bool key_plugin_cb_fn(THD *, plugin_ref plugin, void *arg) {
  plugin = my_plugin_lock(nullptr, &plugin);
  if (plugin) {
    Callback *callback = reinterpret_cast<Callback *>(arg);
    callback->invoke(
        reinterpret_cast<st_mysql_keyring *>(plugin_decl(plugin)->info));
  }
  plugin_unlock(nullptr, plugin);
  // this function should get executed only for the first plugin. This is why
  // it always returns error. plugin_foreach will stop after first iteration.
  return true;
}

/**
  Iterate over plugins of the MYSQL_KEYRING_PLUGIN type and call the function
  specified by the argument.

  @param fn           Function that can call plugin defined function.
  @param check_access Perform access check.

  @return Result of the fn call.
*/
static bool iterate_plugins(std::function<bool(st_mysql_keyring *keyring)> fn,
                            bool check_access = true) {
  Callback callback(fn);
  if (check_access && keyring_access_test()) return true;
  plugin_foreach(current_thd, key_plugin_cb_fn, MYSQL_KEYRING_PLUGIN,
                 &callback);
  return callback.result();
}

/**
  Iterates over all active keyring plugins and calls the mysql_key_fetch API
  for the first one found.

  @sa st_mysql_keyring::mysql_key_fetch, mysql_keyring_service_st
*/
int my_key_fetch(const char *key_id, char **key_type, const char *user_id,
                 void **key, size_t *key_len) {
  return iterate_plugins(
      [&](st_mysql_keyring *keyring) {
        return keyring->mysql_key_fetch(key_id, key_type, user_id, key,
                                        key_len);
      },
      false);
}

/**
  Iterates over all active keyring plugins calls the mysql_key_store API
  for the first one found.

  @sa st_mysql_keyring::mysql_key_store, mysql_keyring_service_st
*/
int my_key_store(const char *key_id, const char *key_type, const char *user_id,
                 const void *key, size_t key_len) {
  return iterate_plugins([&](st_mysql_keyring *keyring) {
    return keyring->mysql_key_store(key_id, key_type, user_id, key, key_len);
  });
}

/**
  Iterates over all active keyring plugins and calls the mysql_key_remove API
  for the first one found.

  @sa st_mysql_keyring::mysql_key_remove, mysql_keyring_service_st
*/
int my_key_remove(const char *key_id, const char *user_id) {
  return iterate_plugins([&](st_mysql_keyring *keyring) {
    return keyring->mysql_key_remove(key_id, user_id);
  });
}

/**
  Iterates over all active keyring plugins and calls the mysql_key_generate API
  for the first one found.

  @sa st_mysql_keyring::mysql_key_generate, mysql_keyring_service_st
*/
int my_key_generate(const char *key_id, const char *key_type,
                    const char *user_id, size_t key_len) {
  return iterate_plugins([&](st_mysql_keyring *keyring) {
    return keyring->mysql_key_generate(key_id, key_type, user_id, key_len);
  });
}

/**
  Iterates over all active keyring plugins and calls
  the mysql_key_iterator_init API for the first one found.

  @sa st_mysql_keyring::mysql_key_iterator_init, mysql_keyring_service_st
*/
int my_key_iterator_init(void **key_iterator) {
  return iterate_plugins([&](st_mysql_keyring *keyring) {
    keyring->mysql_key_iterator_init(key_iterator);
    return false;
  });
}

/**
  Iterates over all active keyring plugins and calls
  the mysql_key_iterator_deinit API for the first one found.

  @sa st_mysql_keyring::mysql_key_iterator_deinit, mysql_keyring_service_st
*/
int my_key_iterator_deinit(void *key_iterator) {
  return iterate_plugins([&](st_mysql_keyring *keyring) {
    keyring->mysql_key_iterator_deinit(key_iterator);
    return false;
  });
}

/**
  Iterates over all active keyring plugins and calls
  the mysql_key_iterator_get_key API for the first one found.

  @sa st_mysql_keyring::mysql_key_iterator_get_key, mysql_keyring_service_st
*/
int my_key_iterator_get_key(void *key_iterator, char *key_id, char *user_id) {
  return iterate_plugins([&](st_mysql_keyring *keyring) {
    return keyring->mysql_key_iterator_get_key(key_iterator, key_id, user_id);
  });
}
