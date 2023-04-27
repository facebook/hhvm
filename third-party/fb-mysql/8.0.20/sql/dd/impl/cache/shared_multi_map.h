/* Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef DD_CACHE__SHARED_MULTI_MAP_INCLUDED
#define DD_CACHE__SHARED_MULTI_MAP_INCLUDED

#include <stdio.h>
#include <vector>  // std::vector

#include "my_psi_config.h"
#include "mysql/components/services/mysql_cond_bits.h"
#include "mysql/components/services/mysql_mutex_bits.h"
#include "mysql/components/services/psi_cond_bits.h"
#include "mysql/components/services/psi_mutex_bits.h"
#include "mysql/psi/mysql_cond.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysql/psi/mysql_thread.h"  // mysql_mutex_t, mysql_cond_t
#include "mysql/psi/psi_base.h"
#include "sql/dd/cache/multi_map_base.h"      // Multi_map_base
#include "sql/dd/impl/cache/cache_element.h"  // Cache_element
#include "sql/dd/impl/cache/free_list.h"      // Free_list
#include "sql/dd/types/abstract_table.h"
#include "sql/dd/types/charset.h"
#include "sql/dd/types/collation.h"
#include "sql/dd/types/column_statistics.h"
#include "sql/dd/types/entity_object_table.h"
#include "sql/dd/types/event.h"
#include "sql/dd/types/resource_group.h"
#include "sql/dd/types/routine.h"
#include "sql/dd/types/schema.h"
#include "sql/dd/types/spatial_reference_system.h"
#include "sql/dd/types/tablespace.h"
#include "sql/malloc_allocator.h"  // Malloc_allocator.
#include "sql/mysqld.h"            // max_connections
#include "thr_mutex.h"

namespace dd {
namespace cache {
template <typename K, typename E>
class Element_map;
template <typename T>
class Cache_element;
}  // namespace cache
}  // namespace dd

#ifdef HAVE_PSI_INTERFACE
extern PSI_mutex_key key_object_cache_mutex;
extern PSI_cond_key key_object_loading_cond;
#endif

namespace dd {
namespace cache {

/**
  Implementation of a shared set of maps for a given object type.

  The implementation is an extension of the Multi_map_base, and adds support
  for:

   - Locking and thread synchronization.
   - Broadcasting a miss which has been handled.
   - Tracking object usage.
   - Managing the free list.
   - Tracking cache misses.
   - Maintaining a pool of allocated cache elements.
   - Deleting objects and elements after releasing the mutex.

  The pool of vacant cache elements is used to avoid deleting and allocating
  memory. We keep a pool of up to 'max_connections' elements. When a new
  element is needed, we first try to get one from the pool. If that does
  not succeed, a new element is allocated dynamically. In this case, the
  allocation will happen while the mutex is locked.

  When an element and object are to be removed from the shared map and
  deleted, the size of the pool decides whether the element should be
  added to the pool or not. If it is not added to the pool, it is signed
  up for being deleted by the Autolocker. The element's object is always
  signed up for being deleted, it is never reused even if the corresponding
  element is added to the pool. The Autolocker is used for two purposes:

    1. To ensure that the mutex is locked and unlocked correctly.
    2. To delete unused elements and objects outside the scope
       where the mutex is locked.

  When the Autolocker is deleted, it will first unlock the mutex, then
  it will iterate over the elements and objects and delete them.

  @note The multi map provides support for tracking cache misses, but does
        not handle the miss. The miss must be handled by the user of the
        multi map.

  @tparam  T  Dictionary object type.
*/

template <typename T>
class Shared_multi_map : public Multi_map_base<T> {
 private:
  static const size_t initial_capacity = 256;

  // Inner class to lock and unlock the multi map instance.
  class Autolocker {
   private:
    // Vector containing objects to be deleted unconditionally.
    typedef std::vector<const T *, Malloc_allocator<const T *>>
        Object_list_type;
    Object_list_type m_objects_to_delete;

    // Vector containing elements to be deleted unconditionally, which
    // happens when elements are evicted while the pool is already full.
    typedef std::vector<const Cache_element<T> *,
                        Malloc_allocator<const Cache_element<T> *>>
        Element_list_type;
    Element_list_type m_elements_to_delete;

    Shared_multi_map<T> *m_map;  // Pointer to the enclosing multi map.

   public:
    // Lock the multi map on instantiation.
    explicit Autolocker(Shared_multi_map<T> *map)
        : m_objects_to_delete(Malloc_allocator<const T *>(PSI_INSTRUMENT_ME)),
          m_elements_to_delete(
              Malloc_allocator<const Cache_element<T> *>(PSI_INSTRUMENT_ME)),
          m_map(map) {
      mysql_mutex_lock(&m_map->m_lock);
    }

    // Add object to be deleted.
    void auto_delete(const T *object) { m_objects_to_delete.push_back(object); }

    // Add element to be deleted.
    void auto_delete(const Cache_element<T> *element) {
      m_elements_to_delete.push_back(element);
    }

    // Unlock the multi map when being deleted (e.g. going out of scope)
    // and delete the objects and elements.
    ~Autolocker() {
      mysql_mutex_unlock(&m_map->m_lock);
      // Delete the objects.
      for (typename Object_list_type::const_iterator it =
               m_objects_to_delete.begin();
           it != m_objects_to_delete.end(); ++it)
        delete *it;

      // Delete the elements.
      for (typename Element_list_type::const_iterator it =
               m_elements_to_delete.begin();
           it != m_elements_to_delete.end(); ++it)
        delete *it;
    }
  };

  /**
    We need a mutex to lock this instance for thread safety, as well as
    a condition variable for synchronizing handling of cache misses. We
    provide a set of operations for using these variables.
  */

  mysql_mutex_t m_lock;         // Single mutex to lock the map.
  mysql_cond_t m_miss_handled;  // Broadcast a miss being handled.

  Free_list<Cache_element<T>> m_free_list;  // Free list.
  std::vector<Cache_element<T> *>
      m_element_pool;  // Pool of allocated elements.
  size_t m_capacity;   // Total capacity, i.e., if the
                       // number of elements exceeds this
                       // limit, shrink the free list.

  /**
    Template helper function getting the element map.

    Const and non-const variants.

    @note   Slightly weird syntax is needed to help the parser
            to resolve this correctly.

    @tparam K  Key type.

    @return The element map handling keys of type K.
   */

  template <typename K>
  Element_map<K, Cache_element<T>> *m_map() {
    return Multi_map_base<T>::template m_map<K>();
  }

  template <typename K>
  const Element_map<K, Cache_element<T>> *m_map() const {
    return Multi_map_base<T>::template m_map<K>();
  }

  /**
    Template function to find an element and mark it as used.

    The function looks up the appropriate element map. If the key
    is present in the map, the element is marked as being used and
    returned. If the element was in the free list, it is removed from
    the list.

    @tparam      K        Key type.
    @param       key      Key to use for looking up the element.

    @return               Element found associated with the key, if
                          present, otherwise NULL.
  */

  template <typename K>
  Cache_element<T> *use_if_present(const K &key);

  /**
    Remove an element from the map.

    This function will remove the element from the multi map. This means that
    all keys associated with the element will be removed from the maps, and
    the cache element wrapper will be deleted. The object itself is not
    deleted. It is up to the outer layer to decide what to do with the object.

    @param  element   Element to be removed.
    @param  lock      Autolocker to use for signing up for auto delete.
  */

  void remove(Cache_element<T> *element, Autolocker *lock);

  /**
    Check if the current map capacity is exceeded.

    @return  true  If the cache capacity is exceeded.
             false If there is additional capacity in the cache.
  */

  bool map_capacity_exceeded() const {
    mysql_mutex_assert_owner(&m_lock);
    return this->m_map<const T *>()->size() > m_capacity;
  }

  /**
    Check if the pool capacity is exceeded.

    We keep enough elements to let all connections have a cache miss
    at the same time without needing to allocate a new element, i.e.,
    we use 'max_connections' as the pool capacity.

    @return  true  If the pool capacity is exceeded.
             false If there is additional capacity for vacant elements.
  */

  bool pool_capacity_exceeded() const {
    mysql_mutex_assert_owner(&m_lock);
    return m_element_pool.size() > max_connections;
  }

  /**
    Helper function to evict unused elements from the free list
    until the cache capacity is not exceeded.

    @param  lock      Autolocker to use for signing up for auto delete.
  */

  void rectify_free_list(Autolocker *lock);

  /**
    Helper function to evict all unused elements from the free list
    and the cache. Used during e.g. shutdown.

    @param  lock      Autolocker to use for signing up for auto delete.
  */

  void evict_all_unused(Autolocker *lock);

 public:
  /**
    Initialize the mutex and condition variable.
    Set initial map capacity.
  */

  Shared_multi_map() : m_capacity(initial_capacity) {
    mysql_mutex_init(key_object_cache_mutex, &m_lock, MY_MUTEX_INIT_FAST);
    mysql_cond_init(key_object_loading_cond, &m_miss_handled);
  }

  /**
    Destroy the mutex and condition variable.
  */

  ~Shared_multi_map() {
    mysql_mutex_destroy(&m_lock);
    mysql_cond_destroy(&m_miss_handled);
  }

  /**
    Shutdown the shared map. Delete all objects present.
  */

  void shutdown();

  /**
    Reset the shared map. Locks and deletes all objects present,
    but keeps the element pool and the capacity setting.

    @param       thd      Thread context.

    @retval      true     Failure, e.g. timeout from metadata lock acquisition.
    @retval      false    Otherwise.
  */

  bool reset(THD *thd);

  /**
    Set capacity of the shared map.
  */

  void set_capacity(size_t capacity) {
    Autolocker lock(this);
    m_capacity = capacity;
    rectify_free_list(&lock);
  }

  /**
    Check if an element with the given key is available.
  */

  template <typename K>
  bool available(const K &key) {
    Autolocker lock(this);
    Cache_element<T> *e = nullptr;
    m_map<K>()->get(key, &e);
    return (e != nullptr);
  }

  /**
    Get a wrapper element from the map handling the given key type.

    If the wrapped object is present, mark it as used and return it. If the
    object is in the process of being loaded, wait for loading to be
    completed. If the object is not present, and not being loaded, mark it
    as missed and return.

    @tparam      K        Key type.
    @param       key      Key to use for looking up the object.
    @param [out] element  Wrapper element pointer, if present, either
                          immediately or after a cache miss handled by another
                          thread. NULL if awakened after a miss handled by
                          another thread, which was unable to load the object
                          from elsewhere. NULL if the object is not present,
                          and no other thread is missing it.

    @retval      true     If the object is missed, and this thread must handle
                          the miss.
    @retval      false    Otherwise.
  */

  template <typename K>
  bool get(const K &key, Cache_element<T> **element);

  /**
    Put a new object and element wrapper into the map.

    The new object may be NULL if it does not exist in the DD tables.
    In that case, we must still update the missing set and broadcast,
    if necessary, because other threads may be waiting. Then, the
    element returned is also NULL.

    Otherwise, we check that the actual object instance is not present
    in the map, then all the keys are generated based on the new object.
    The keys that are missed are removed from the missed set, and then
    we verify that all keys are either present or absent.

    If no keys are present, it means we must add the new object. We
    remove the least recently used objects, if cache capacity is exceeded,
    and add the new object to the reverse map. We also mark the object
    as being used. The element wrapper of the new object is returned.

    If all keys are present, we return the wrapper element already present
    and delete the newly read object.

    Finally, if at least one key was missed, we broadcast that the miss
    has been handled.

    @note  This function will delete the newly read object if an object with
           the same keys is already present.

    @note  The function may add a new object which is not registered as being
           missed, i.e., without a preceding cache miss In this case, the
           submitted key is NULL.

    @tparam      K           Key type.
    @param       key         Key used when loading the object.
    @param       object      New object to be added.
    @param [out] element     Element wrapper: NULL if the new object is
                             NULL, otherwise the wrapper of the newly
                             read object or the existing object with
                             the same keys.
  */

  template <typename K>
  void put(const K *key, const T *object, Cache_element<T> **element);

  /**
    Release one element.

    The element must be present and in use. If the element becomes unused,
    it is added to the free list, which is then rectified to enforce its
    capacity constraints.

    @param   element   Element pointer.
  */

  void release(Cache_element<T> *element);

  /**
    Delete an element from the map.

    This function will remove the element from all maps, using remove(),
    and delete the object pointed to. This means that all keys associated
    with the element will be removed from the maps, and the cache element
    wrapper will be deleted. The object may not be accessed after calling
    this function.

    @param   element   Element pointer.
  */

  void drop(Cache_element<T> *element);

  /**
    Delete an object corresponding to the key from the map if exists.

    This function will find the element corresponding to the key if
    it exists. After that it will remove the element from all maps, using
    remove(), and delete the object pointed to. This means that all keys
    associated with the element will be removed from the maps, and the
    cache element wrapper will be deleted.

    @tparam  K         Key type.
    @param   key       Key to be checked.
  */

  template <typename K>
  void drop_if_present(const K &key);

  /**
    Replace the object and re-generate the keys for an element.

    The element must be present and in use. The keys by which it is hashed
    are removed from the internal maps. The new keys are generated, and the
    element, with its new keys, is added to the maps again.

    @param  element   Element for which new keys should be generated.
    @param  object    New object to replace the old one.
  */

  void replace(Cache_element<T> *element, const T *object);

  /**
    Debug dump of the shared multi map to stderr.
  */
  /* purecov: begin inspected */
  void dump() const {
#ifndef DBUG_OFF
    fprintf(stderr, "  --------------------------------\n");
    fprintf(stderr, "  Shared multi map for '%s'\n",
            T::DD_table::instance().name().c_str());
    Multi_map_base<T>::dump();
    fprintf(stderr, "    Free list:\n");
    m_free_list.dump();
    fprintf(stderr, "  --------------------------------\n");
#endif
  }
  /* purecov: end */
};

}  // namespace cache
}  // namespace dd

#endif  // DD_CACHE__SHARED_MULTI_MAP_INCLUDED
