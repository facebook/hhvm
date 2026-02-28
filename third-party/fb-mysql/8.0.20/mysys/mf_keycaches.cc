/* Copyright (c) 2003, 2019, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   Without limiting anything contained in the foregoing, this file,
   which is part of C Driver for MySQL (Connector/C), is also subject to the
   Universal FOSS Exception, version 1.0, a copy of which can be found at
   http://oss.oracle.com/licenses/universal-foss-exception.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/**
  @file mysys/mf_keycaches.cc
  Handling of multiple key caches.

  The idea is to have a thread safe hash on the table name,
  with a default key cache value that is returned if the table name is not in
  the cache.
*/

#include <string.h>
#include <sys/types.h>
#include <string>

#include "keycache.h"
#include "m_ctype.h"
#include "map_helpers.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "mysql/psi/mysql_rwlock.h"
#include "mysql/service_mysql_alloc.h"
#include "mysys/mysys_priv.h"
#include "template_utils.h"

using std::string;

/*****************************************************************************
  General functions to handle SAFE_HASH objects.

  A SAFE_HASH object is used to store the hash, the lock and default value
  needed by the rest of the key cache code.
  This is a separate struct to make it easy to later reuse the code for other
  purposes

  All entries are linked in a list to allow us to traverse all elements
  and delete selected ones. (HASH doesn't allow any easy ways to do this).
*****************************************************************************/

/*
  Struct to store a key and pointer to object
*/

struct SAFE_HASH_ENTRY {
  char *key;
  uint length;
  uchar *data;
  SAFE_HASH_ENTRY *next, **prev;
};

struct SAFE_HASH {
  mysql_rwlock_t lock;
  malloc_unordered_map<string, unique_ptr_my_free<SAFE_HASH_ENTRY>> hash{
      key_memory_SAFE_HASH_ENTRY};
  uchar *default_value;
  SAFE_HASH_ENTRY *root;
};

/*
  Init a SAFE_HASH object

  SYNOPSIS
    safe_hash_init()
    hash		safe_hash handler
    elements		Expected max number of elements
    default_value	default value

  NOTES
    In case of error we set hash->default_value to 0 to allow one to call
    safe_hash_free on an object that couldn't be initialized.

  RETURN
    0  ok
    1  error
*/

static bool safe_hash_init(SAFE_HASH *hash, uchar *default_value) {
  DBUG_TRACE;
  mysql_rwlock_init(key_SAFE_HASH_lock, &hash->lock);
  hash->default_value = default_value;
  hash->root = nullptr;
  return false;
}

/*
  Free a SAFE_HASH object

  NOTES
    This is safe to call on any object that has been sent to safe_hash_init()
*/

static void safe_hash_free(SAFE_HASH *hash) {
  /*
    Test if safe_hash_init succeeded. This will also guard us against multiple
    free calls.
  */
  if (hash->default_value) {
    hash->hash.clear();
    mysql_rwlock_destroy(&hash->lock);
    hash->default_value = nullptr;
  }
}

/*
  Return the value stored for a key or default value if no key
*/

static uchar *safe_hash_search(SAFE_HASH *hash, const uchar *key, uint length) {
  uchar *result;
  DBUG_TRACE;
  mysql_rwlock_rdlock(&hash->lock);
  auto it = hash->hash.find(string(pointer_cast<const char *>(key), length));
  if (it == hash->hash.end())
    result = hash->default_value;
  else
    result = it->second->data;
  mysql_rwlock_unlock(&hash->lock);
  DBUG_PRINT("exit", ("data: %p", result));
  return result;
}

/*
  Associate a key with some data

  SYONOPSIS
    safe_hash_set()
    hash			Hash handle
    key				key (path to table etc..)
    length			Length of key
    data			data to to associate with the data

  NOTES
    This can be used both to insert a new entry and change an existing
    entry.
    If one associates a key with the default key cache, the key is deleted

  RETURN
    0  ok
    1  error (Can only be EOM). In this case my_message() is called.
*/

static bool safe_hash_set(SAFE_HASH *hash, const uchar *key, uint length,
                          uchar *data) {
  SAFE_HASH_ENTRY *entry;
  bool error = false;
  DBUG_TRACE;
  DBUG_PRINT("enter", ("key: %.*s  data: %p", length, key, data));
  string key_str(pointer_cast<const char *>(key), length);

  mysql_rwlock_wrlock(&hash->lock);
  entry = find_or_nullptr(hash->hash, key_str);

  if (data == hash->default_value) {
    /*
      The key is to be associated with the default entry. In this case
      we can just delete the entry (if it existed) from the hash as a
      search will return the default entry
    */
    if (!entry) /* nothing to do */
      goto end;
    /* unlink entry from list */
    if ((*entry->prev = entry->next)) entry->next->prev = entry->prev;
    hash->hash.erase(key_str);
    goto end;
  }
  if (entry) {
    /* Entry existed;  Just change the pointer to point at the new data */
    entry->data = data;
  } else {
    if (!(entry = (SAFE_HASH_ENTRY *)my_malloc(key_memory_SAFE_HASH_ENTRY,
                                               sizeof(*entry) + length,
                                               MYF(MY_WME)))) {
      error = true;
      goto end;
    }
    entry->key = (char *)(entry + 1);
    memcpy(entry->key, key, length);
    entry->length = length;
    entry->data = data;
    /* Link entry to list */
    if ((entry->next = hash->root)) entry->next->prev = &entry->next;
    entry->prev = &hash->root;
    hash->root = entry;
    hash->hash.emplace(
        string(pointer_cast<const char *>(entry->key), entry->length),
        unique_ptr_my_free<SAFE_HASH_ENTRY>(entry));
  }

end:
  mysql_rwlock_unlock(&hash->lock);
  return error;
}

/*
  Change all entres with one data value to another data value

  SYONOPSIS
    safe_hash_change()
    hash			Hash handle
    old_data			Old data
    new_data			Change all 'old_data' to this

  NOTES
    We use the linked list to traverse all elements in the hash as
    this allows us to delete elements in the case where 'new_data' is the
    default value.
*/

static void safe_hash_change(SAFE_HASH *hash, uchar *old_data,
                             uchar *new_data) {
  SAFE_HASH_ENTRY *entry, *next;
  DBUG_TRACE;

  mysql_rwlock_wrlock(&hash->lock);

  for (entry = hash->root; entry; entry = next) {
    next = entry->next;
    if (entry->data == old_data) {
      if (new_data == hash->default_value) {
        if ((*entry->prev = entry->next)) entry->next->prev = entry->prev;
        hash->hash.erase(string(entry->key, entry->length));
      } else
        entry->data = new_data;
    }
  }

  mysql_rwlock_unlock(&hash->lock);
}

/*****************************************************************************
  Functions to handle the key cache objects
*****************************************************************************/

/* Variable to store all key cache objects */
static SAFE_HASH key_cache_hash;

bool multi_keycache_init(void) {
  return safe_hash_init(&key_cache_hash, (uchar *)dflt_key_cache);
}

void multi_keycache_free(void) { safe_hash_free(&key_cache_hash); }

/*
  Get a key cache to be used for a specific table.

  SYNOPSIS
    multi_key_cache_search()
    key				key to find (usually table path)
    uint length			Length of key.

  NOTES
    This function is coded in such a way that we will return the
    default key cache even if one never called multi_keycache_init.
    This will ensure that it works with old MyISAM clients.

  RETURN
    key cache to use
*/

KEY_CACHE *multi_key_cache_search(uchar *key, uint length) {
  if (key_cache_hash.hash.empty()) return dflt_key_cache;
  return (KEY_CACHE *)safe_hash_search(&key_cache_hash, key, length);
}

/*
  Assosiate a key cache with a key


  SYONOPSIS
    multi_key_cache_set()
    key				key (path to table etc..)
    length			Length of key
    key_cache			cache to assococite with the table

  NOTES
    This can be used both to insert a new entry and change an existing
    entry
*/

bool multi_key_cache_set(const uchar *key, uint length, KEY_CACHE *key_cache) {
  return safe_hash_set(&key_cache_hash, key, length, (uchar *)key_cache);
}

void multi_key_cache_change(KEY_CACHE *old_data, KEY_CACHE *new_data) {
  safe_hash_change(&key_cache_hash, (uchar *)old_data, (uchar *)new_data);
}
