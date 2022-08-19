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

#include "sql/dd/impl/cache/shared_multi_map.h"

#include <new>

#include "my_dbug.h"
#include "my_loglevel.h"
#include "mysql/components/services/log_builtins.h"
#include "mysqld_error.h"
#include "sql/dd/cache/dictionary_client.h"
#include "sql/dd/impl/cache/cache_element.h"
#include "sql/dd/impl/tables/character_sets.h"
#include "sql/dd/impl/tables/collations.h"
#include "sql/dd/impl/tables/column_statistics.h"
#include "sql/dd/impl/tables/events.h"
#include "sql/dd/impl/tables/resource_groups.h"
#include "sql/dd/impl/tables/routines.h"
#include "sql/dd/impl/tables/schemata.h"
#include "sql/dd/impl/tables/spatial_reference_systems.h"
#include "sql/dd/impl/tables/tables.h"
#include "sql/dd/impl/tables/tablespaces.h"
#include "sql/log.h"        // sql_print_warning()
#include "sql/mdl.h"        // MDL_request
#include "sql/sql_class.h"  // THD

namespace dd {
namespace cache {

// Template function to find an element and mark it as used.
template <typename T>
template <typename K>
Cache_element<T> *Shared_multi_map<T>::use_if_present(const K &key) {
  mysql_mutex_assert_owner(&m_lock);
  Cache_element<T> *e = nullptr;
  // Look up in the appropriate map and get the element.
  m_map<K>()->get(key, &e);

  // Use the element if present.
  if (e) {
    // Remove the element from the free list.
    if (e->usage() == 0) m_free_list.remove(e);

    // Mark the element as being used, and return it.
    e->use();
    return e;
  }
  return nullptr;
}

// Remove an element from the map.
template <typename T>
void Shared_multi_map<T>::remove(Cache_element<T> *element, Autolocker *lock) {
  mysql_mutex_assert_owner(&m_lock);

#ifndef DBUG_OFF
  Cache_element<T> *e = nullptr;
  m_map<const T *>()->get(element->object(), &e);

  // The element must be present, and its usage must be 1 (this thread).
  DBUG_ASSERT(e == element);
  DBUG_ASSERT(e->usage() == 1);

  // Get all keys that were created within the element.
  const typename T::Id_key *id_key = element->id_key();
  const typename T::Name_key *name_key = element->name_key();
  const typename T::Aux_key *aux_key = element->aux_key();

  // None of the non-null keys may be missed.
  DBUG_ASSERT(
      (!id_key || !m_map<typename T::Id_key>()->is_missed(*id_key)) &&
      (!name_key || !m_map<typename T::Name_key>()->is_missed(*name_key)) &&
      (!aux_key || !m_map<typename T::Aux_key>()->is_missed(*aux_key)));

  // All non-null keys must exist.
  DBUG_ASSERT(
      (!id_key || m_map<typename T::Id_key>()->is_present(*id_key)) &&
      (!name_key || m_map<typename T::Name_key>()->is_present(*name_key)) &&
      (!aux_key || m_map<typename T::Aux_key>()->is_present(*aux_key)));
#endif

  // Remove the keys and the element from the shared map and the registry.
  Multi_map_base<T>::remove_single_element(element);

  // Sign up the object for being deleted.
  lock->auto_delete(element->object());

  // Reuse the element if there is room for it.
  if (!pool_capacity_exceeded())
    m_element_pool.push_back(element);
  else
    lock->auto_delete(element);
}

// Helper function to evict unused elements from the free list.
template <typename T>
void Shared_multi_map<T>::rectify_free_list(Autolocker *lock) {
  mysql_mutex_assert_owner(&m_lock);
  while (map_capacity_exceeded() && m_free_list.length() > 0) {
    Cache_element<T> *e = m_free_list.get_lru();
    DBUG_ASSERT(e && e->object());
    m_free_list.remove(e);
    // Mark the object as being used to allow it to be removed.
    e->use();
    remove(e, lock);
  }
}

// Helper function to evict all unused elements.
template <typename T>
void Shared_multi_map<T>::evict_all_unused(Autolocker *lock) {
  mysql_mutex_assert_owner(&m_lock);
  while (m_free_list.length()) {
    Cache_element<T> *e = m_free_list.get_lru();
    DBUG_ASSERT(e && e->object());
    m_free_list.remove(e);
    // Mark the object as being used to allow it to be removed.
    e->use();
    remove(e, lock);
  }
}

// Shutdown the shared map. Delete all objects present.
template <typename T>
void Shared_multi_map<T>::shutdown() {
  {
    Autolocker lock(this);
    m_capacity = 0;
    evict_all_unused(&lock);
    if (m_map<const T *>()->size() > 0) {
      /* purecov: begin deadcode */
      LogErr(WARNING_LEVEL, ER_DD_CACHE_NOT_EMPTY_AT_SHUTDOWN);
      dump();
      DBUG_ASSERT(false);
      /* purecov: end */
    }
  }
  // Delete all elements from the pool.
  for (typename std::vector<Cache_element<T> *>::iterator it =
           m_element_pool.begin();
       it != m_element_pool.end(); ++it)
    delete (*it);
  m_element_pool.clear();
}

typedef std::map<Object_id, const String_type> schema_map_t;

template <typename T>
MDL_request *lock_request(THD *, const schema_map_t &, const T *) {
  DBUG_ASSERT(false);
  return nullptr;
}

template <>
MDL_request *lock_request(THD *thd, const schema_map_t &schema_map,
                          const Abstract_table *object) {
  // Fetch the schema to get hold of the schema name.
  const schema_map_t::const_iterator schema_name =
      schema_map.find(object->schema_id());
  if (schema_name == schema_map.end() || object == nullptr) return nullptr;

  MDL_request *request = new (thd->mem_root) MDL_request;
  if (request == nullptr) return nullptr;

  MDL_REQUEST_INIT(request, MDL_key::TABLE, schema_name->second.c_str(),
                   object->name().c_str(), MDL_EXCLUSIVE, MDL_TRANSACTION);
  return request;
}

template <>
MDL_request *lock_request(THD *thd, const schema_map_t &,
                          const Tablespace *object) {
  MDL_request *request = new (thd->mem_root) MDL_request;
  if (request == nullptr || object == nullptr) return nullptr;

  MDL_REQUEST_INIT(request, MDL_key::TABLESPACE, "", object->name().c_str(),
                   MDL_EXCLUSIVE, MDL_TRANSACTION);
  return request;
}

/*
  Reset the shared map. Delete all objects present after acquiring
  metadata locks. Keep the capacity.
*/
template <typename T>
bool Shared_multi_map<T>::reset(THD *thd) {
  /*
    Establish a map from schema ids to schema names. Must do this
    before we can lock the cache partition.
  */
  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());
  std::vector<const Schema *> schema_vector;
  if (thd->dd_client()->fetch_global_components(&schema_vector)) return true;

  schema_map_t schema_map;
  for (const Schema *schema : schema_vector)
    schema_map.insert(
        typename schema_map_t::value_type(schema->id(), schema->name()));

  // Now, we can lock the cache partition and start acquiring MDL.
  Autolocker lock(this);
  MDL_request_list mdl_requests;
  typename Element_map<const T *, Cache_element<T>>::Const_iterator it =
      m_map<const T *>()->begin();
  for (; it != m_map<const T *>()->end(); it++)
    mdl_requests.push_front(
        lock_request(thd, schema_map, it->second->object()));

  if (thd->mdl_context.acquire_locks_nsec(
          &mdl_requests, thd->variables.lock_wait_timeout_nsec))
    return true;

  /*
    We have now locked all objects, hence, once we evict the unused
    object, no objects should be left.
  */
  evict_all_unused(&lock);
  if (m_map<const T *>()->size() > 0) {
    dump();
    DBUG_ASSERT(m_map<const T *>()->size() == 0);
    return true;
  }

  return false;
}

// Get a wrapper element from the map handling the given key type.
template <typename T>
template <typename K>
bool Shared_multi_map<T>::get(const K &key, Cache_element<T> **element) {
  Autolocker lock(this);
  *element = use_if_present(key);
  if (*element) return false;

  // Is the element already missed?
  if (m_map<K>()->is_missed(key)) {
    while (m_map<K>()->is_missed(key))
      mysql_cond_wait(&m_miss_handled, &m_lock);

    *element = use_if_present(key);

    // Here, we return only if element is non-null. An absent element
    // does not mean that the object does not exist, it might have been
    // evicted after the thread handling the first cache miss added
    // it to the cache, before this waiting thread was alerted. Thus,
    // we need to handle this situation as a cache miss if the element
    // is absent.
    if (*element) return false;
  }

  // Mark the key as being missed.
  m_map<K>()->set_missed(key);
  return true;
}

// Put a new object and element wrapper into the map.
template <typename T>
template <typename K>
void Shared_multi_map<T>::put(const K *key, const T *object,
                              Cache_element<T> **element) {
  DBUG_ASSERT(element);
  Autolocker lock(this);
  if (!object) {
    DBUG_ASSERT(key);
    // For a NULL object, we only need to signal that the miss is handled.
    if (m_map<K>()->is_missed(*key)) {
      m_map<K>()->set_miss_handled(*key);
      mysql_cond_broadcast(&m_miss_handled);
    }
    DBUG_ASSERT(*element == nullptr);
    return;
  }

#ifndef DBUG_OFF
  // The new object instance may not be present in the map.
  m_map<const T *>()->get(object, element);
  DBUG_ASSERT(*element == nullptr);
#endif

  // Get a new element, either from the pool, or by allocating a new one.
  if (!m_element_pool.empty()) {
    *element = m_element_pool.back();
    m_element_pool.pop_back();
    (*element)->init();
  } else
    *element = new (std::nothrow) Cache_element<T>();

  // Prepare the element. Assign the object and create keys.
  (*element)->set_object(object);
  (*element)->recreate_keys();

  // Get all keys that were created within the element.
  const typename T::Id_key *id_key = (*element)->id_key();
  const typename T::Name_key *name_key = (*element)->name_key();
  const typename T::Aux_key *aux_key = (*element)->aux_key();

  // There must be at least one key.
  DBUG_ASSERT(id_key || name_key || aux_key);

  // For the non-null keys being missed, set that the miss is handled.
  bool key_missed = false;
  if (id_key && m_map<typename T::Id_key>()->is_missed(*id_key)) {
    key_missed = true;
    m_map<typename T::Id_key>()->set_miss_handled(*id_key);
  }
  if (name_key && m_map<typename T::Name_key>()->is_missed(*name_key)) {
    key_missed = true;
    m_map<typename T::Name_key>()->set_miss_handled(*name_key);
  }
  if (aux_key && m_map<typename T::Aux_key>()->is_missed(*aux_key)) {
    key_missed = true;
    m_map<typename T::Aux_key>()->set_miss_handled(*aux_key);
  }

  // All non-null keys must exist, or none.
  bool all_keys_present =
      (!id_key || m_map<typename T::Id_key>()->is_present(*id_key)) &&
      (!name_key || m_map<typename T::Name_key>()->is_present(*name_key)) &&
      (!aux_key || m_map<typename T::Aux_key>()->is_present(*aux_key));
  bool no_keys_present =
      (!id_key || !m_map<typename T::Id_key>()->is_present(*id_key)) &&
      (!name_key || !m_map<typename T::Name_key>()->is_present(*name_key)) &&
      (!aux_key || !m_map<typename T::Aux_key>()->is_present(*aux_key));

  // If no keys are present, we must add the element.
  if (no_keys_present) {
    // Remove least recently used object(s).
    rectify_free_list(&lock);

    // Mark the element as in use, and register it in all key maps.
    (*element)->use();
    Multi_map_base<T>::add_single_element(*element);

    // In this case, one or more keys may be missed, so we must broadcast.
    if (key_missed) mysql_cond_broadcast(&m_miss_handled);

    // The element and the object is now owned by the cache.
    return;
  }

  // If all keys are already present, we discard the new element.
  if (all_keys_present) {
    DBUG_ASSERT(key);

    // If all keys are present, we sign up the object for being deleted.
    lock.auto_delete(object);

    // The element is added to the pool if room, or signed up for
    // auto delete.
    if (!pool_capacity_exceeded())
      m_element_pool.push_back(*element);
    else
      lock.auto_delete(*element);

    // Then we return the existing element.
    *element = use_if_present(*key);
    DBUG_ASSERT(*element);

    // In this case, no key may be missed, so there is no need to broadcast.
    DBUG_ASSERT(!key_missed);
    return;
  }

  // Must have all_keys_present ^ no_keys_present
  DBUG_ASSERT(false); /* purecov: inspected */
}

// Release one element.
template <typename T>
void Shared_multi_map<T>::release(Cache_element<T> *element) {
  Autolocker lock(this);

#ifndef DBUG_OFF
  // The object must be present, and its usage must be > 0.
  Cache_element<T> *e = nullptr;
  m_map<const T *>()->get(element->object(), &e);
  DBUG_ASSERT(e == element);
  DBUG_ASSERT(e->usage() > 0);

  // Get all keys that were created within the element.
  const typename T::Id_key *id_key = element->id_key();
  const typename T::Name_key *name_key = element->name_key();
  const typename T::Aux_key *aux_key = element->aux_key();

  // All non-null keys must exist.
  DBUG_ASSERT(
      (!id_key || m_map<typename T::Id_key>()->is_present(*id_key)) &&
      (!name_key || m_map<typename T::Name_key>()->is_present(*name_key)) &&
      (!aux_key || m_map<typename T::Aux_key>()->is_present(*aux_key)));
#endif

  // Release the element.
  element->release();

  // If the element is not used, add it to the free list.
  if (element->usage() == 0) {
    m_free_list.add_last(element);
    rectify_free_list(&lock);
  }
}

// Delete an element from the map.
template <typename T>
void Shared_multi_map<T>::drop(Cache_element<T> *element) {
  Autolocker lock(this);
  remove(element, &lock);
}

// Delete an object corresponding to the key from the map if exists.
template <typename T>
template <typename K>
void Shared_multi_map<T>::drop_if_present(const K &key) {
  Autolocker lock(this);

  Cache_element<T> *element = use_if_present(key);

  if (element) {
    remove(element, &lock);
  }
}

// Replace the object and re-generate the keys for an element.
template <typename T>
void Shared_multi_map<T>::replace(Cache_element<T> *element, const T *object) {
  Autolocker lock(this);

#ifndef DBUG_OFF
  // The object must be present, and its usage must be 1 (this thread).
  Cache_element<T> *e = nullptr;
  m_map<const T *>()->get(element->object(), &e);
  DBUG_ASSERT(e == element);
  DBUG_ASSERT(e->usage() == 1);
  DBUG_ASSERT(object);
#endif

  // Remove the single element from the maps, but do not delete the instance.
  Multi_map_base<T>::remove_single_element(element);

  // If different, delete the old object, and replace it by the new one.
  if (element->object() != object) {
    lock.auto_delete(element->object());
    element->set_object(object);
  }

  // Re-create the keys based on the current version of the object.
  element->recreate_keys();

  // Add the element again, now with the newly generated keys.
  Multi_map_base<T>::add_single_element(element);
}

// Explicitly instantiate the types for the various usages.
template class Shared_multi_map<Abstract_table>;
template bool Shared_multi_map<Abstract_table>::get<const Abstract_table *>(
    const Abstract_table *const &, Cache_element<Abstract_table> **);
template bool Shared_multi_map<Abstract_table>::get<Abstract_table::Id_key>(
    const Abstract_table::Id_key &, Cache_element<Abstract_table> **);
template bool Shared_multi_map<Abstract_table>::get<Abstract_table::Name_key>(
    const Abstract_table::Name_key &, Cache_element<Abstract_table> **);
template bool Shared_multi_map<Abstract_table>::get<Abstract_table::Aux_key>(
    const Abstract_table::Aux_key &, Cache_element<Abstract_table> **);
template void Shared_multi_map<Abstract_table>::put<Abstract_table::Id_key>(
    const Abstract_table::Id_key *, const Abstract_table *,
    Cache_element<Abstract_table> **);
template void Shared_multi_map<Abstract_table>::put<Abstract_table::Name_key>(
    const Abstract_table::Name_key *, const Abstract_table *,
    Cache_element<Abstract_table> **);
template void Shared_multi_map<Abstract_table>::put<Abstract_table::Aux_key>(
    const Abstract_table::Aux_key *, const Abstract_table *,
    Cache_element<Abstract_table> **);
template void Shared_multi_map<Abstract_table>::drop_if_present<
    Abstract_table::Id_key>(const Abstract_table::Id_key &);

template class Shared_multi_map<Charset>;
template bool Shared_multi_map<Charset>::get<const Charset *>(
    const Charset *const &, Cache_element<Charset> **);
template bool Shared_multi_map<Charset>::get<Charset::Id_key>(
    const Charset::Id_key &, Cache_element<Charset> **);
template bool Shared_multi_map<Charset>::get<Charset::Name_key>(
    const Charset::Name_key &, Cache_element<Charset> **);
template bool Shared_multi_map<Charset>::get<Charset::Aux_key>(
    const Charset::Aux_key &, Cache_element<Charset> **);
template void Shared_multi_map<Charset>::put<Charset::Id_key>(
    const Charset::Id_key *, const Charset *, Cache_element<Charset> **);
template void Shared_multi_map<Charset>::put<Charset::Name_key>(
    const Charset::Name_key *, const Charset *, Cache_element<Charset> **);
template void Shared_multi_map<Charset>::put<Charset::Aux_key>(
    const Charset::Aux_key *, const Charset *, Cache_element<Charset> **);
template void Shared_multi_map<Charset>::drop_if_present<Charset::Id_key>(
    const Charset::Id_key &);

template class Shared_multi_map<Collation>;
template bool Shared_multi_map<Collation>::get<const Collation *>(
    const Collation *const &, Cache_element<Collation> **);
template bool Shared_multi_map<Collation>::get<Collation::Id_key>(
    const Collation::Id_key &, Cache_element<Collation> **);
template bool Shared_multi_map<Collation>::get<Collation::Name_key>(
    const Collation::Name_key &, Cache_element<Collation> **);
template bool Shared_multi_map<Collation>::get<Collation::Aux_key>(
    const Collation::Aux_key &, Cache_element<Collation> **);
template void Shared_multi_map<Collation>::put<Collation::Id_key>(
    const Collation::Id_key *, const Collation *, Cache_element<Collation> **);
template void Shared_multi_map<Collation>::put<Collation::Name_key>(
    const Collation::Name_key *, const Collation *,
    Cache_element<Collation> **);
template void Shared_multi_map<Collation>::put<Collation::Aux_key>(
    const Collation::Aux_key *, const Collation *, Cache_element<Collation> **);
template void Shared_multi_map<Collation>::drop_if_present<Collation::Id_key>(
    const Collation::Id_key &);

template class Shared_multi_map<Column_statistics>;
template bool
Shared_multi_map<Column_statistics>::get<const Column_statistics *>(
    const Column_statistics *const &, Cache_element<Column_statistics> **);
template bool
Shared_multi_map<Column_statistics>::get<Column_statistics::Id_key>(
    const Column_statistics::Id_key &, Cache_element<Column_statistics> **);
template bool
Shared_multi_map<Column_statistics>::get<Column_statistics::Name_key>(
    const Column_statistics::Name_key &, Cache_element<Column_statistics> **);
template bool
Shared_multi_map<Column_statistics>::get<Column_statistics::Aux_key>(
    const Column_statistics::Aux_key &, Cache_element<Column_statistics> **);
template void
Shared_multi_map<Column_statistics>::put<Column_statistics::Id_key>(
    const Column_statistics::Id_key *, const Column_statistics *,
    Cache_element<Column_statistics> **);
template void
Shared_multi_map<Column_statistics>::put<Column_statistics::Name_key>(
    const Column_statistics::Name_key *, const Column_statistics *,
    Cache_element<Column_statistics> **);
template void
Shared_multi_map<Column_statistics>::put<Column_statistics::Aux_key>(
    const Column_statistics::Aux_key *, const Column_statistics *,
    Cache_element<Column_statistics> **);
template void Shared_multi_map<Column_statistics>::drop_if_present<
    Column_statistics::Id_key>(const Column_statistics::Id_key &);

template class Shared_multi_map<Event>;
template bool Shared_multi_map<Event>::get<const Event *>(
    const Event *const &, Cache_element<Event> **);
template bool Shared_multi_map<Event>::get<Event::Id_key>(
    const Event::Id_key &, Cache_element<Event> **);
template bool Shared_multi_map<Event>::get<Event::Name_key>(
    const Event::Name_key &, Cache_element<Event> **);
template bool Shared_multi_map<Event>::get<Event::Aux_key>(
    const Event::Aux_key &, Cache_element<Event> **);
template void Shared_multi_map<Event>::put<Event::Id_key>(
    const Event::Id_key *, const Event *, Cache_element<Event> **);
template void Shared_multi_map<Event>::put<Event::Name_key>(
    const Event::Name_key *, const Event *, Cache_element<Event> **);
template void Shared_multi_map<Event>::put<Event::Aux_key>(
    const Event::Aux_key *, const Event *, Cache_element<Event> **);
template void Shared_multi_map<Event>::drop_if_present<Event::Id_key>(
    const Event::Id_key &);

template class Shared_multi_map<Routine>;
template bool Shared_multi_map<Routine>::get<const Routine *>(
    const Routine *const &, Cache_element<Routine> **);
template bool Shared_multi_map<Routine>::get<Routine::Id_key>(
    const Routine::Id_key &, Cache_element<Routine> **);
template bool Shared_multi_map<Routine>::get<Routine::Name_key>(
    const Routine::Name_key &, Cache_element<Routine> **);
template bool Shared_multi_map<Routine>::get<Routine::Aux_key>(
    const Routine::Aux_key &, Cache_element<Routine> **);
template void Shared_multi_map<Routine>::put<Routine::Id_key>(
    const Routine::Id_key *, const Routine *, Cache_element<Routine> **);
template void Shared_multi_map<Routine>::put<Routine::Name_key>(
    const Routine::Name_key *, const Routine *, Cache_element<Routine> **);
template void Shared_multi_map<Routine>::put<Routine::Aux_key>(
    const Routine::Aux_key *, const Routine *, Cache_element<Routine> **);
template void Shared_multi_map<Routine>::drop_if_present<Routine::Id_key>(
    const Routine::Id_key &);

template class Shared_multi_map<Schema>;
template bool Shared_multi_map<Schema>::get<const Schema *>(
    const Schema *const &, Cache_element<Schema> **);
template bool Shared_multi_map<Schema>::get<Schema::Id_key>(
    const Schema::Id_key &, Cache_element<Schema> **);
template bool Shared_multi_map<Schema>::get<Schema::Name_key>(
    const Schema::Name_key &, Cache_element<Schema> **);
template bool Shared_multi_map<Schema>::get<Schema::Aux_key>(
    const Schema::Aux_key &, Cache_element<Schema> **);
template void Shared_multi_map<Schema>::put<Schema::Id_key>(
    const Schema::Id_key *, const Schema *, Cache_element<Schema> **);
template void Shared_multi_map<Schema>::put<Schema::Name_key>(
    const Schema::Name_key *, const Schema *, Cache_element<Schema> **);
template void Shared_multi_map<Schema>::put<Schema::Aux_key>(
    const Schema::Aux_key *, const Schema *, Cache_element<Schema> **);
template void Shared_multi_map<Schema>::drop_if_present<Schema::Id_key>(
    const Schema::Id_key &);

template class Shared_multi_map<Spatial_reference_system>;
template bool Shared_multi_map<Spatial_reference_system>::get<
    const Spatial_reference_system *>(
    const Spatial_reference_system *const &,
    Cache_element<Spatial_reference_system> **);
template bool Shared_multi_map<Spatial_reference_system>::get<
    Spatial_reference_system::Id_key>(
    const Spatial_reference_system::Id_key &,
    Cache_element<Spatial_reference_system> **);
template bool Shared_multi_map<Spatial_reference_system>::get<
    Spatial_reference_system::Name_key>(
    const Spatial_reference_system::Name_key &,
    Cache_element<Spatial_reference_system> **);
template bool Shared_multi_map<Spatial_reference_system>::get<
    Spatial_reference_system::Aux_key>(
    const Spatial_reference_system::Aux_key &,
    Cache_element<Spatial_reference_system> **);
template void Shared_multi_map<Spatial_reference_system>::put<
    Spatial_reference_system::Id_key>(
    const Spatial_reference_system::Id_key *, const Spatial_reference_system *,
    Cache_element<Spatial_reference_system> **);
template void Shared_multi_map<Spatial_reference_system>::put<
    Spatial_reference_system::Name_key>(
    const Spatial_reference_system::Name_key *,
    const Spatial_reference_system *,
    Cache_element<Spatial_reference_system> **);
template void Shared_multi_map<Spatial_reference_system>::put<
    Spatial_reference_system::Aux_key>(
    const Spatial_reference_system::Aux_key *, const Spatial_reference_system *,
    Cache_element<Spatial_reference_system> **);
template void Shared_multi_map<Spatial_reference_system>::drop_if_present<
    Spatial_reference_system::Id_key>(const Spatial_reference_system::Id_key &);

template class Shared_multi_map<Tablespace>;
template bool Shared_multi_map<Tablespace>::get<const Tablespace *>(
    const Tablespace *const &, Cache_element<Tablespace> **);
template bool Shared_multi_map<Tablespace>::get<Tablespace::Id_key>(
    const Tablespace::Id_key &, Cache_element<Tablespace> **);
template bool Shared_multi_map<Tablespace>::get<Tablespace::Name_key>(
    const Tablespace::Name_key &, Cache_element<Tablespace> **);
template bool Shared_multi_map<Tablespace>::get<Tablespace::Aux_key>(
    const Tablespace::Aux_key &, Cache_element<Tablespace> **);
template void Shared_multi_map<Tablespace>::put<Tablespace::Id_key>(
    const Tablespace::Id_key *, const Tablespace *,
    Cache_element<Tablespace> **);
template void Shared_multi_map<Tablespace>::put<Tablespace::Name_key>(
    const Tablespace::Name_key *, const Tablespace *,
    Cache_element<Tablespace> **);
template void Shared_multi_map<Tablespace>::put<Tablespace::Aux_key>(
    const Tablespace::Aux_key *, const Tablespace *,
    Cache_element<Tablespace> **);
template void Shared_multi_map<Tablespace>::drop_if_present<Tablespace::Id_key>(
    const Tablespace::Id_key &);

template class Shared_multi_map<Resource_group>;
template bool Shared_multi_map<Resource_group>::get<const Resource_group *>(
    const Resource_group *const &, Cache_element<Resource_group> **);
template bool Shared_multi_map<Resource_group>::get<Resource_group::Id_key>(
    const Resource_group::Id_key &, Cache_element<Resource_group> **);
template bool Shared_multi_map<Resource_group>::get<Resource_group::Name_key>(
    const Resource_group::Name_key &, Cache_element<Resource_group> **);
template bool Shared_multi_map<Resource_group>::get<Resource_group::Aux_key>(
    const Resource_group::Aux_key &, Cache_element<Resource_group> **);
template void Shared_multi_map<Resource_group>::put<Resource_group::Id_key>(
    const Resource_group::Id_key *, const Resource_group *,
    Cache_element<Resource_group> **);
template void Shared_multi_map<Resource_group>::put<Resource_group::Name_key>(
    const Resource_group::Name_key *, const Resource_group *,
    Cache_element<Resource_group> **);
template void Shared_multi_map<Resource_group>::put<Resource_group::Aux_key>(
    const Resource_group::Aux_key *, const Resource_group *,
    Cache_element<Resource_group> **);
template void Shared_multi_map<Resource_group>::drop_if_present<
    Resource_group::Id_key>(const Resource_group::Id_key &);

}  // namespace cache
}  // namespace dd
