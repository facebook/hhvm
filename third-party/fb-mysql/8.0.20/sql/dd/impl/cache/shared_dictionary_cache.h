/* Copyright (c) 2015, 2017, Oracle and/or its affiliates. All rights reserved.

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

#ifndef DD_CACHE__SHARED_DICTIONARY_CACHE_INCLUDED
#define DD_CACHE__SHARED_DICTIONARY_CACHE_INCLUDED

#include <stdio.h>

#include "sql/dd/impl/cache/shared_multi_map.h"  // Shared_multi_map
#include "sql/dd/types/abstract_table.h"
#include "sql/dd/types/charset.h"                   // Charset
#include "sql/dd/types/collation.h"                 // Collation
#include "sql/dd/types/column_statistics.h"         // Column_statistics
#include "sql/dd/types/event.h"                     // Event
#include "sql/dd/types/routine.h"                   // Routine
#include "sql/dd/types/schema.h"                    // Schema
#include "sql/dd/types/spatial_reference_system.h"  // Spatial_reference_system
#include "sql/dd/types/table.h"                     // IWYU pragma: keep
#include "sql/dd/types/tablespace.h"                // Tablespace
#include "sql/handler.h"                            // enum_tx_isolation

class THD;

namespace dd {
namespace cache {

/**
  Shared dictionary cache containing several maps.

  The dictionary cache is mainly a collection of shared maps for the
  object types supported. The functions dispatch to the appropriate
  map based on the key and object type parameter. Cache misses are handled
  by retrieving the object from the storage adapter singleton.

  The shared dictionary cache itself does not handle concurrency at this
  outer layer. Concurrency is handled by the various instances of the
  shared multi map.
*/

template <typename T>
class Cache_element;

class Shared_dictionary_cache {
 private:
  // Collation and character set cache sizes are chosen so that they can hold
  // all collations and character sets built into the server. The spatial
  // reference system cache size is chosen to hold a reasonable number of SRSs
  // for normal server use.
  static const size_t collation_capacity = 256;
  static const size_t column_statistics_capacity = 32;
  static const size_t charset_capacity = 64;
  static const size_t event_capacity = 256;
  static const size_t spatial_reference_system_capacity = 256;
  /**
    Maximum number of DD resource group objects to be kept in
    cache. We use value of 32 which is a fairly reasonable upper limit
    of resource group configurations that may be in use.
  */
  static const size_t resource_group_capacity = 32;

  Shared_multi_map<Abstract_table> m_abstract_table_map;
  Shared_multi_map<Charset> m_charset_map;
  Shared_multi_map<Collation> m_collation_map;
  Shared_multi_map<Column_statistics> m_column_stat_map;
  Shared_multi_map<Event> m_event_map;
  Shared_multi_map<Resource_group> m_resource_group_map;
  Shared_multi_map<Routine> m_routine_map;
  Shared_multi_map<Schema> m_schema_map;
  Shared_multi_map<Spatial_reference_system> m_spatial_reference_system_map;
  Shared_multi_map<Tablespace> m_tablespace_map;

  template <typename T>
  struct Type_selector {};  // Dummy type to use for
                            // selecting map instance.

  /**
    Overloaded functions to use for selecting map instance based
    on a key type. Const and non-const variants.
  */

  Shared_multi_map<Abstract_table> *m_map(Type_selector<Abstract_table>) {
    return &m_abstract_table_map;
  }
  Shared_multi_map<Charset> *m_map(Type_selector<Charset>) {
    return &m_charset_map;
  }
  Shared_multi_map<Collation> *m_map(Type_selector<Collation>) {
    return &m_collation_map;
  }
  Shared_multi_map<Column_statistics> *m_map(Type_selector<Column_statistics>) {
    return &m_column_stat_map;
  }
  Shared_multi_map<Event> *m_map(Type_selector<Event>) { return &m_event_map; }
  Shared_multi_map<Resource_group> *m_map(Type_selector<Resource_group>) {
    return &m_resource_group_map;
  }
  Shared_multi_map<Routine> *m_map(Type_selector<Routine>) {
    return &m_routine_map;
  }
  Shared_multi_map<Schema> *m_map(Type_selector<Schema>) {
    return &m_schema_map;
  }
  Shared_multi_map<Spatial_reference_system> *m_map(
      Type_selector<Spatial_reference_system>) {
    return &m_spatial_reference_system_map;
  }
  Shared_multi_map<Tablespace> *m_map(Type_selector<Tablespace>) {
    return &m_tablespace_map;
  }

  const Shared_multi_map<Abstract_table> *m_map(
      Type_selector<Abstract_table>) const {
    return &m_abstract_table_map;
  }
  const Shared_multi_map<Charset> *m_map(Type_selector<Charset>) const {
    return &m_charset_map;
  }
  const Shared_multi_map<Collation> *m_map(Type_selector<Collation>) const {
    return &m_collation_map;
  }
  const Shared_multi_map<Column_statistics> *m_map(
      Type_selector<Column_statistics>) const {
    return &m_column_stat_map;
  }
  const Shared_multi_map<Schema> *m_map(Type_selector<Schema>) const {
    return &m_schema_map;
  }
  const Shared_multi_map<Spatial_reference_system> *m_map(
      Type_selector<Spatial_reference_system>) const {
    return &m_spatial_reference_system_map;
  }
  const Shared_multi_map<Tablespace> *m_map(Type_selector<Tablespace>) const {
    return &m_tablespace_map;
  }
  const Shared_multi_map<Resource_group> *m_map(
      Type_selector<Resource_group>) const {
    return &m_resource_group_map;
  }

  /**
    Template function to get a map instance.

    To support generic code, the map instances are available through
    template function instances. This allows looking up the
    appropriate instance based on the key type. We must use
    overloading to accomplish this (see above). Const and non-const
    variants.

    @tparam  T  Dictionary object type.

    @return  The shared map handling objects of type T.
  */

  template <typename T>
  Shared_multi_map<T> *m_map() {
    return m_map(Type_selector<T>());
  }

  template <typename T>
  const Shared_multi_map<T> *m_map() const {
    return m_map(Type_selector<T>());
  }

  Shared_dictionary_cache() {}

 public:
  static Shared_dictionary_cache *instance();

  // Set capacity of the shared maps.
  static void init();

  // Shutdown the shared maps.
  static void shutdown();

  // Reset the shared cache. Optionally keep the core DD table meta data.
  static void reset(bool keep_dd_entities);

  // Reset the table and tablespace partitions.
  static bool reset_tables_and_tablespaces(THD *thd);

  /**
    Check if an element with the given key is available.

    @param key   Key to check for presence.

    @retval true   The key exist.
    @retval false  The key does not exist.
  */

  template <typename K, typename T>
  bool available(const K &key) {
    return m_map<T>()->available(key);
  }

  /**
    Get an element from the cache, given the key.

    The operation retrieves an element by one of its keys from the cache
    (possibly involving a cache miss, which will need the thd to handle the
    miss) and returns it through the parameter. If there is no element for
    the given key, NULL is returned. The cache owns the returned element,
    i.e., the caller must not delete it. After using the element, release()
    must be called for every element received via get(). The reference
    counter for the element is incremented if the element is retrieved from
    the shared cache.

    @tparam      K       Key type.
    @tparam      T       Dictionary object type.
    @param       thd     Thread context.
    @param       key     Key to use for looking up the object.
    @param [out] element Element pointer, if present. NULL if not present.

    @retval      false   No error.
    @retval      true    Error (from handling a cache miss).
  */

  template <typename K, typename T>
  bool get(THD *thd, const K &key, Cache_element<T> **element);

  /**
    Read an object directly from disk, given the key.

    The operation retrieves an object by one of its keys from the persistent
    dd tables. The object is returned without being added to the shared
    cache. The object returned is owned by the caller, who thus becomes
    responsible of deleting it.

    @tparam      K         Key type.
    @tparam      T         Dictionary object type.
    @param       thd       Thread context.
    @param       key       Key to use for looking up the object.
    @param       isolation Isolation level to use.
    @param [out] object    Object pointer, if present. NULL if not present.

    @retval      false   No error.
    @retval      true    Error (from reading from the DD tables).
  */

  template <typename K, typename T>
  bool get_uncached(THD *thd, const K &key, enum_tx_isolation isolation,
                    const T **object) const;

  /**
    Add an object to the shared cache.

    The object may not be present already. The object is added to the cache,
    the use counter of its element wrapper in incremented, and the element
    pointer is returned. The user must release the object afterwards. The
    cache is the owner of the returned element and object.

    @tparam  T        Dictionary object type.
    @param   object   Object pointer to be added. May not be NULL.
    @param   element  Element pointer, if present. NULL if not present.
  */

  template <typename T>
  void put(const T *object, Cache_element<T> **element);

  /**
    Release an element used by a client.

    The element must be present and in use. If the element becomes unused,
    it is added to the free list, which is then rectified to enforce
    its capacity constraints.

    @tparam  T         Dictionary object type.
    @param   e         Element pointer.
  */

  template <typename T>
  void release(Cache_element<T> *e) {
    m_map<T>()->release(e);
  }

  /**
    Delete an element from the cache.

    This function will remove the element from the cache and delete the
    object pointed to. This means that all keys associated with the element
    will be removed from the maps, and the cache element wrapper will be
    deleted. The object may not be accessed after calling this function.

    @tparam  T         Dictionary object type.
    @param   element   Element pointer.
  */

  template <typename T>
  void drop(Cache_element<T> *element) {
    m_map<T>()->drop(element);
  }

  /**
    Delete an element corresponding to the key from the cache if exists.

    This function will find the element corresponding to the key if
    it exists. After that it will remove the element from the cache
    i.e. all maps, and delete the object pointed to. This means that
    all keys associated with the element will be removed from the maps,
    and the cache element wrapper will be deleted.

    @tparam  K         Key type.
    @tparam  T         Dictionary object type.
    @param   key       Key to be checked.
  */

  template <typename K, typename T>
  void drop_if_present(const K &key) {
    m_map<T>()->drop_if_present(key);
  }

  /**
    Replace the object and re-create the keys for an element.

    The operation removes the current keys from the internal maps in the
    cache, assigns the new object to the element, generates new keys based
    on the new object, and inserts the new keys into the internal maps in the
    cache. The old object is deleted.

    @tparam  T         Dictionary object type.
    @param   element   Element pointer.
    @param   object    New object to replace the old one.
  */

  template <typename T>
  void replace(Cache_element<T> *element, const T *object) {
    m_map<T>()->replace(element, object);
  }

  /**
    Debug dump of a shared cache partition to stderr.

    @tparam  T         Dictionary object type.
  */

  template <typename T>
  void dump() const {
#ifndef DBUG_OFF
    fprintf(stderr, "================================\n");
    fprintf(stderr, "Shared dictionary cache\n");
    m_map<T>()->dump();
    fprintf(stderr, "================================\n");
#endif
  }
};

}  // namespace cache
}  // namespace dd

#endif  // DD_CACHE__SHARED_DICTIONARY_CACHE_INCLUDED
