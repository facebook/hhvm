/* Copyright (c) 2016, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef MYSQL_PLUGIN_KEYRING_INCLUDED
#define MYSQL_PLUGIN_KEYRING_INCLUDED

/**
  API for keyring plugin. (MYSQL_KEYRING_PLUGIN)
*/

#include "plugin.h"
#define MYSQL_KEYRING_INTERFACE_VERSION 0x0101

/**
  The descriptor structure for the plugin, that is referred from
  st_mysql_plugin.
*/

struct st_mysql_keyring {
  int interface_version;
  /*!
    Add key to the keyring.

    Obfuscates and adds the key to the keyring. The key is associated with
    key_id and user_id (unique key identifier).

    @param[in] key_id   id of the key to store
    @param[in] key_type type of the key to store
    @param[in] user_id  id of the owner of the key
    @param[in] key      the key itself to be stored. The memory of the key is
                        copied by the keyring, thus the key itself can be freed
                        after it was stored in the keyring.
    @param[in] key_len  the length of the key to be stored

    @return Operation status
      @retval 0 OK
      @retval 1 ERROR
  */
  bool (*mysql_key_store)(const char *key_id, const char *key_type,
                          const char *user_id, const void *key, size_t key_len);
  /*!
    Fetches key from the keyring.

    De-obfuscates and retrieves key associated with key_id and user_id from the
    keyring.

    @param[in]  key_id   id of the key to fetch
    @param[out] key_type type of the fetched key
    @param[in]  user_id  id of the owner of the key
    @param[out] key      the fetched key itself. The memory for this key is
                         allocated by the keyring and needs to be freed by the
                         user when no longer needed. Prior to freeing the memory
                         it needs to be obfuscated or zeroed.
    @param[out] key_len  the length of the fetched key

    @return Operation status
      @retval 0 OK
      @retval 1 ERROR
  */
  bool (*mysql_key_fetch)(const char *key_id, char **key_type,
                          const char *user_id, void **key, size_t *key_len);

  /*!
    Removes key from the keyring.

    Removes the key associated with key_id and user_id from the
    keyring.

    @param[in]  key_id   id of the key to remove
    @param[in]  user_id  id of the owner of the key to remove

    @return Operation status
      @retval 0 OK
      @retval 1 ERROR
  */
  bool (*mysql_key_remove)(const char *key_id, const char *user_id);

  /*!
    Generates and stores the key.

    Generates a random key of length key_len, associates it with key_id, user_id
    and stores it in the keyring.

    @param[in] key_id   id of the key to generate
    @param[in] key_type type of the key to generate
    @param[in] user_id  id of the owner of the generated key
    @param[in] key_len  length of the key to generate

    @return Operation status
      @retval 0 OK
      @retval 1 ERROR
  */
  bool (*mysql_key_generate)(const char *key_id, const char *key_type,
                             const char *user_id, size_t key_len);

  /**
    Keys_iterator object refers to an iterator which is used to iterate
    on a list which refers to Key_metadata. Key_metadata hold information
    about individual keys keyd_id and user_id. Keys_iterator should be used
    in following sequence only.

    void* iterator_ptr;
    char key_id[64]= { 0 };
    char user_id[64]= { 0 };

    plugin_handle->mysql_key_iterator_init(&iterator_ptr);

    if (iterator_ptr == NULL)
      report error;

    while (!(plugin_handle->mysql_key_iterator_get_key(iterator_ptr,
           key_id, user_id)))
    {
       Fetch the keys.
       Perform operations on the fetched keys.
       ..
    }
    plugin_handle->mysql_key_iterator_deinit(iterator_ptr);

    init() method accepts a void pointer which is the made to point to
    Keys_iterator instance. Keys_iterator instance internal pointer points
    to Key_metadata list. This list holds information about all keys stored
    in the backed end data store of keyring plugin. After call to init()
    please check iterator_ptr.

    get_key() method accepts the above iterator_ptr as IN param and then
    fills the passes in key_id and user_id with valid values. This can be
    used to fetch actual key information. Every call to this method will
    change internal pointers to advance to next position, so that the next
    call will fetch the next key.

    deinit() method frees all internal pointers along with iterator_ptr.
  */
  /**
    Initialize an iterator.

    @param[out]  key_iterator   Iterator used to fetch individual keys
                                from key_container.
  */
  void (*mysql_key_iterator_init)(void **key_iterator);

  /**
    Deinitialize an iterator.

    @param[in]   key_iterator   Iterator used to fetch individual keys
                                from key_container.
  */
  void (*mysql_key_iterator_deinit)(void *key_iterator);

  /**
    Get details of key. Every call to this service will change
    internal pointers to advance to next position, so that the next call
    will fetch the next key. In case iterator moves to the end, this service
    will return error.

    @param[in]   key_iterator   Iterator used to fetch individual keys
                                from key_container.
    @param[out]  key_id         id of the key
    @param[out]  user_id        id of the owner

    @return Operation status
      @retval 0 OK
      @retval 1 ERROR
  */
  bool (*mysql_key_iterator_get_key)(void *key_iterator, char *key_id,
                                     char *user_id);
};
#endif
