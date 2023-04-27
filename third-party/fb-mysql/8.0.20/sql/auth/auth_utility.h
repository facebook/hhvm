/* Copyright (c) 2018, 2019, Oracle and/or its affiliates. All rights reserved.

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
/* Internals */

#ifndef AUTH_UTILITY_INCLUDED
#define AUTH_UTILITY_INCLUDED

#include "my_alloc.h"
#include "mysql/psi/mysql_rwlock.h"
#include "rwlock_scoped_lock.h"

#include <map>

/**
 Class to manage MEM_ROOT. It either accepts and initialized MEM_ROOT
 or initializes a new one and controls its lifespan.
*/
class Mem_root_base {
 public:
  explicit Mem_root_base(MEM_ROOT *mem_root);
  Mem_root_base(const Mem_root_base &) = delete;
  Mem_root_base &operator=(const Mem_root_base &) = delete;
  ~Mem_root_base();
  MEM_ROOT *get_mem_root() const;

 protected:
  MEM_ROOT *m_mem_root;
  MEM_ROOT m_internal_mem_root;

 private:
  Mem_root_base();
  bool m_inited;
};

/**
  Return MEM_ROOT handle.
*/
inline MEM_ROOT *Mem_root_base::get_mem_root() const { return m_mem_root; }

/**
  Map with RWLock protections
*/
template <typename K, typename V>
class Map_with_rw_lock {
 public:
  Map_with_rw_lock(PSI_rwlock_key key) { mysql_rwlock_init(key, &m_lock); }
  Map_with_rw_lock(const Map_with_rw_lock &) = delete;
  Map_with_rw_lock(Map_with_rw_lock &&) = delete;
  Map_with_rw_lock &operator=(const Map_with_rw_lock &) = delete;
  Map_with_rw_lock &operator=(Map_with_rw_lock &&) = delete;
  ~Map_with_rw_lock() {
    m_map.clear();
    mysql_rwlock_destroy(&m_lock);
  }

  /**
    Search an entry

    @param [in]  key   Key
    @param [out] value Value

    @returns status of search
      @retval false Entry not found
      @retval true  Entry found. Check value.
  */
  bool find(const K &key, V &value) {
    rwlock_scoped_lock rdlock(&m_lock, false, __FILE__, __LINE__);
    const auto search_itr = m_map.find(key);
    if (search_itr != m_map.end()) {
      value = search_itr->second;
      return true;
    }
    return false;
  }

  /**
    Insert an entry

    @param [in] key   Key
    @param [in] value Value

    @returns status of insertion
      @retval false Error
      @retval true  Success
  */
  bool insert(K key, V value) {
    rwlock_scoped_lock wrlock(&m_lock, true, __FILE__, __LINE__);
    auto returns = m_map.insert(std::make_pair(key, value));
    return returns.second;
  }

  /**
    Delete an entry

    @param [in] key Key to the element to be erased
  */
  void erase(K key) {
    rwlock_scoped_lock wrlock(&m_lock, true, __FILE__, __LINE__);
    m_map.erase(key);
  }

  /**
    Check limit and clear if needed

    @param [in] size Limit to check against
  */
  void clear_if_greater(size_t size) {
    rwlock_scoped_lock wrlock(&m_lock, true, __FILE__, __LINE__);
    if (m_map.size() >= size) m_map.clear();
  }

  /** Get the size of the map */
  size_t size() {
    rwlock_scoped_lock rdlock(&m_lock, false, __FILE__, __LINE__);
    return m_map.size();
  }

 private:
  /** Map */
  std::map<K, V> m_map;
  /** Lock to protect m_map */
  mysql_rwlock_t m_lock;
};

#endif /* AUTH_UTILITY_INCLUDED */
