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

#ifndef DD_CACHE__CACHE_ELEMENT_INCLUDED
#define DD_CACHE__CACHE_ELEMENT_INCLUDED

#include "my_dbug.h"
#include "sql/dd/impl/raw/object_keys.h"  // Primary_id_key
#include "sql/dd/string_type.h"           // dd::String_type

namespace dd_cache_unittest {
class CacheTestHelper;
}

namespace dd {
namespace cache {

// Forward declare Shared_multi_map for friend directive.
template <typename T>
class Shared_multi_map;

// Forward declare Storage_adapter for friend directive,
// needed for unit tests.
class Storage_adapter;

/**
  Implementation of a cache element.

  This template class implements a wrapper to support caching of
  arbitrary objects. The wrapper provides support for reference counting,
  but does not make any assumptions regarding the semantics of this
  functionality. The enforcement of such assumptions must be built into
  the layer using the cache element implementation.

  The cache element stores copies of the keys that are used for looking
  up the object in the cache. This is needed to support fast reverse lookup
  of keys, given the object instance, e.g. to enable removing old keys when
  new keys must be created. The keys are stored in pre-allocated memory.

  @note The usage of the reference counter is not implemented by means of
        atomic operations. Locking at an outer level takes care of race
        conditions.

  @tparam  T  Dictionary object type being wrapped.
*/

template <typename T>
class Cache_element {
  friend class Storage_adapter;                     // Unit test access.
  friend class dd_cache_unittest::CacheTestHelper;  // Unit test access.
  friend class Shared_multi_map<T>;                 // Access to changing data.
  friend class Dictionary_client;                   // Access to changing data.

 private:
  const T *m_object;   // Pointer to the actual object.
  uint m_ref_counter;  // Number of concurrent object usages.

  /**
    Helper class to represent a key instance. We also need to
    represent whether the instance is NULL.

    @tparam  K  Key type.
  */

  template <typename K>
  class Key_wrapper {
   public:
    bool is_null;
    K key;
    Key_wrapper() : is_null(true), key() {}
  };

  Key_wrapper<typename T::Id_key> m_id_key;      // The id key for the object.
  Key_wrapper<typename T::Name_key> m_name_key;  // The name key for the object.
  Key_wrapper<typename T::Aux_key> m_aux_key;    // The aux key for the object.

  // Helper functions using overloading to get keys using a template.
  template <typename K>
  struct Type_selector {};

  const T *const *get_key(Type_selector<const T *>) const {
    return m_object ? &m_object : nullptr;
  }

  const typename T::Id_key *get_key(Type_selector<typename T::Id_key>) const {
    return id_key();
  }

  const typename T::Name_key *get_key(
      Type_selector<typename T::Name_key>) const {
    return name_key();
  }

  const typename T::Aux_key *get_key(Type_selector<typename T::Aux_key>) const {
    return aux_key();
  }

  // Delete all keys.
  void delete_keys() {
    m_id_key.is_null = true;
    m_name_key.is_null = true;
    m_aux_key.is_null = true;
  }

  // Increment the reference counter associated with the object.
  void use() { m_ref_counter++; }

  // Let the cache element point to another object.
  void set_object(const T *replacement_object) {
    m_object = replacement_object;
  }

  // Update the keys based on the object pointed to.
  void recreate_keys() {
    DBUG_ASSERT(m_object);
    m_id_key.is_null = m_object->update_id_key(&m_id_key.key);
    m_name_key.is_null = m_object->update_name_key(&m_name_key.key);
    m_aux_key.is_null = m_object->update_aux_key(&m_aux_key.key);
  }

 public:
  // Initialize an instance to having NULL pointers and 0 count.
  Cache_element()
      : m_object(nullptr),
        m_ref_counter(0),
        m_id_key(),
        m_name_key(),
        m_aux_key() {} /* purecov: tested */

  // Note that the object being pointed to is not deleted implicitly.
  ~Cache_element() { delete_keys(); }

  // Initialize an existing instance.
  void init() {
    m_object = nullptr;
    m_ref_counter = 0;
    delete_keys();
  }

  // Decrement the reference counter associated with the object.
  void release() {
    DBUG_ASSERT(m_ref_counter > 0);
    m_ref_counter--;
  }

  // Return current number of usages of the object.
  uint usage() const { return m_ref_counter; }

  // Return the object pointer.
  const T *object() const { return m_object; }

  // Get the id key.
  const typename T::Id_key *id_key() const {
    return m_id_key.is_null ? nullptr : &m_id_key.key;
  }

  // Get the name key.
  const typename T::Name_key *name_key() const {
    return m_name_key.is_null ? nullptr : &m_name_key.key;
  }

  // Get the aux key.
  const typename T::Aux_key *aux_key() const {
    return m_aux_key.is_null ? nullptr : &m_aux_key.key;
  }

  /**
    Template function to get a pointer to a key based on the type.

    @tparam  K  Key type.
  */

  template <typename K>
  const K *get_key() const {
    return get_key(Type_selector<K>());
  }

  // Debug dump of the element to stderr.
  /* purecov: begin inspected */
  void dump(const String_type &prefix MY_ATTRIBUTE((unused)) = "      ") const {
#ifndef DBUG_OFF
    fprintf(stderr, "%sobj: %p, id: %llu, cnt: %u", prefix.c_str(), m_object,
            m_object ? m_object->id() : 0, m_ref_counter);
    fprintf(stderr, ", id_k: %s",
            m_id_key.is_null ? "NULL" : m_id_key.key.str().c_str());
    fprintf(stderr, ", name_k: %s",
            m_name_key.is_null ? "NULL" : m_name_key.key.str().c_str());
    fprintf(stderr, ", aux_k: %s\n",
            m_aux_key.is_null ? "NULL" : m_aux_key.key.str().c_str());
#endif
  }
  /* purecov: end */
};

}  // namespace cache
}  // namespace dd

#endif  // DD_CACHE__CACHE_ELEMENT_INCLUDED
