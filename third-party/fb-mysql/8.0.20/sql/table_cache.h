/* Copyright (c) 2012, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef TABLE_CACHE_INCLUDED
#define TABLE_CACHE_INCLUDED

#include <stddef.h>
#include <sys/types.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

#include "lex_string.h"
#include "my_base.h"
#include "my_dbug.h"
#include "my_psi_config.h"
#include "mysql/components/services/mysql_mutex_bits.h"
#include "mysql/components/services/psi_mutex_bits.h"
#include "mysql/psi/mysql_mutex.h"
#include "sql/handler.h"
#include "sql/sql_base.h"
#include "sql/sql_class.h"
#include "sql/sql_plist.h"
#include "sql/system_variables.h"
#include "sql/table.h"

class Table_cache_element;

extern ulong table_cache_size_per_instance, table_cache_instances;

/**
  Cache for open TABLE objects.

  The idea behind this cache is that most statements don't need to
  go to a central table definition cache to get a TABLE object and
  therefore don't need to lock LOCK_open mutex.
  Instead they only need to go to one Table_cache instance (the
  specific instance is determined by thread id) and only lock the
  mutex protecting this cache.
  DDL statements that need to remove all TABLE objects from all caches
  need to lock mutexes for all Table_cache instances, but they are rare.

  This significantly increases scalability in some scenarios.
*/

class Table_cache {
 private:
  /**
    The table cache lock protects the following data:

    1) m_unused_tables list.
    2) m_cache hash.
    3) used_tables, free_tables lists in Table_cache_element objects in
       this cache.
    4) m_table_count - total number of TABLE objects in this cache.
    5) the element in TABLE_SHARE::cache_element[] array that corresponds
       to this cache,
    6) in_use member in TABLE object.
    7) Also ownership of mutexes for all caches are required to update
       the refresh_version and table_def_shutdown_in_progress variables
       and TABLE_SHARE::version member.

    The intention is that any query that finds a cached table object in
    its designated table cache should only need to lock this mutex
    instance and there should be no need to lock LOCK_open. LOCK_open is
    still required however to create and release TABLE objects. However
    most usage of the MySQL Server should be able to set the cache size
    big enough so that the majority of the queries only need to lock this
    mutex instance and not LOCK_open.
  */
  mysql_mutex_t m_lock;

  /**
    The hash of Table_cache_element objects, each table/table share that
    has any TABLE object in the Table_cache has a Table_cache_element from
    which the list of free TABLE objects in this table cache AND the list
    of used TABLE objects in this table cache is stored.
    We use Table_cache_element::share::table_cache_key as key for this hash.
  */
  std::unordered_map<std::string, std::unique_ptr<Table_cache_element>> m_cache;

  /**
    List that contains all TABLE instances for tables in this particular
    table cache that are in not use by any thread. Recently used TABLE
    instances are appended to the end of the list. Thus the beginning of
    the list contains which have been least recently used.
  */
  TABLE *m_unused_tables;

  /**
    Total number of TABLE instances for tables in this particular table
    cache (both in use by threads and not in use).
    This value summed over all table caches is accessible to users as
    Open_tables status variable.
  */
  uint m_table_count;

#ifdef HAVE_PSI_INTERFACE
  static PSI_mutex_key m_lock_key;
  static PSI_mutex_info m_mutex_keys[];
#endif

 private:
#ifdef EXTRA_DEBUG
  void check_unused();
#else
  void check_unused() {}
#endif
  inline void link_unused_table(TABLE *table);
  inline void unlink_unused_table(TABLE *table);

  inline void free_unused_tables_if_necessary(THD *thd,
                                              bool acquire_lock = true);

 public:
  bool init();
  void destroy();
  static void init_psi_keys();

  /** Acquire lock on table cache instance. */
  void lock() { mysql_mutex_lock(&m_lock); }
  /** Release lock on table cache instance. */
  void unlock() { mysql_mutex_unlock(&m_lock); }
  /** Assert that caller owns lock on the table cache. */
  void assert_owner() { mysql_mutex_assert_owner(&m_lock); }

  inline TABLE *get_table(THD *thd, const char *key, size_t key_length,
                          TABLE_SHARE **share);

  inline void release_table(THD *thd, TABLE *table);

  inline bool add_used_table(THD *thd, TABLE *table, bool acquire_lock = true);
  inline void remove_table(TABLE *table);

  /** Get number of TABLE instances in the cache. */
  uint cached_tables() const { return m_table_count; }

  void free_all_unused_tables();
  void free_old_unused_tables(time_point cutpoint);

#ifndef DBUG_OFF
  void print_tables();
#endif
};

/**
  Container class for all table cache instances in the system.
*/

class Table_cache_manager {
 public:
  /** Maximum supported number of table cache instances. */
  static const int MAX_TABLE_CACHES = 64;

  /** Default number of table cache instances */
  static const int DEFAULT_MAX_TABLE_CACHES = 16;

  bool init();
  void destroy();

  /** Get instance of table cache to be used by particular connection. */
  Table_cache *get_cache(THD *thd) {
    return &m_table_cache[thd->thread_id() % table_cache_instances];
  }

  /** Get index for the table cache in container. */
  uint cache_index(Table_cache *cache) const {
    return static_cast<uint>(cache - &m_table_cache[0]);
  }

  uint cached_tables();

  void lock_all_and_tdc();
  void unlock_all_and_tdc();
  void assert_owner_all();
  void assert_owner_all_and_tdc();

  void free_table(THD *thd, enum_tdc_remove_table_type remove_type,
                  TABLE_SHARE *share);

  void free_all_unused_tables();
  void free_old_unused_tables(time_point cutpoint);

#ifndef DBUG_OFF
  void print_tables();
#endif

  friend class Table_cache_iterator;

 private:
  /**
    An array of Table_cache instances.
    Only the first table_cache_instances elements in it are used.
  */
  Table_cache m_table_cache[MAX_TABLE_CACHES];
};

extern Table_cache_manager table_cache_manager;

/**
  Element that represents the table in the specific table cache.
  Plays for table cache instance role similar to role of TABLE_SHARE
  for table definition cache.

  It is an implementation detail of Table_cache and is present
  in the header file only to allow inlining of some methods.
*/

class Table_cache_element {
 private:
  /*
    Doubly-linked (back-linked) lists of used and unused TABLE objects
    for this table in this table cache (one such list per table cache).
  */
  typedef I_P_List<
      TABLE, I_P_List_adapter<TABLE, &TABLE::cache_next, &TABLE::cache_prev>>
      TABLE_list;

  TABLE_list used_tables;
  TABLE_list free_tables;
  TABLE_SHARE *share;

 public:
  Table_cache_element(TABLE_SHARE *share_arg) : share(share_arg) {}

  TABLE_SHARE *get_share() const { return share; }

  friend class Table_cache;
  friend class Table_cache_manager;
  friend class Table_cache_iterator;
};

/**
  Iterator which allows to go through all used TABLE instances
  for the table in all table caches.
*/

class Table_cache_iterator {
  const TABLE_SHARE *share;
  uint current_cache_index;
  TABLE *current_table;

  inline void move_to_next_table();

 public:
  /**
    Construct iterator over all used TABLE objects for the table share.

    @note Assumes that caller owns locks on all table caches.
  */
  inline Table_cache_iterator(const TABLE_SHARE *share_arg);
  inline TABLE *operator++(int);
  inline void rewind();
};

/**
  Add table to the tail of unused tables list for table cache
  (i.e. as the most recently used table in this list).
*/

void Table_cache::link_unused_table(TABLE *table) {
  if (m_unused_tables) {
    table->next = m_unused_tables;
    table->prev = m_unused_tables->prev;
    m_unused_tables->prev = table;
    table->prev->next = table;
  } else
    m_unused_tables = table->next = table->prev = table;
  check_unused();
}

/** Remove table from the unused tables list for table cache. */

void Table_cache::unlink_unused_table(TABLE *table) {
  table->next->prev = table->prev;
  table->prev->next = table->next;
  if (table == m_unused_tables) {
    m_unused_tables = m_unused_tables->next;
    if (table == m_unused_tables) m_unused_tables = nullptr;
  }
  check_unused();
}

/**
  Free unused TABLE instances if total number of TABLE objects
  in table cache has exceeded table_cache_size_per_instance
  limit.

  @note That we might need to free more than one instance during
        this call if table_cache_size was changed dynamically.
*/

void Table_cache::free_unused_tables_if_necessary(THD *thd, bool acquire_lock) {
  /*
    We have too many TABLE instances around let us try to get rid of them.

    Note that we might need to free more than one TABLE object, and thus
    need the below loop, in case when table_cache_size is changed dynamically,
    at server run time.
  */
  if (m_table_count > table_cache_size_per_instance && m_unused_tables) {
    if (acquire_lock) mysql_mutex_lock(&LOCK_open);
    while (m_table_count > table_cache_size_per_instance && m_unused_tables) {
      TABLE *table_to_free = m_unused_tables;
      remove_table(table_to_free);
      intern_close_table(table_to_free);
      thd->status_var.table_open_cache_overflows++;
    }
    if (acquire_lock) mysql_mutex_unlock(&LOCK_open);
  }
}

/**
   Add newly created TABLE object which is going to be used right away
   to the table cache.

   @note Caller should own lock on the table cache.

   @note Sets TABLE::in_use member as side effect.

   @retval false - success.
   @retval true  - failure.
*/

bool Table_cache::add_used_table(THD *thd, TABLE *table, bool acquire_lock) {
  Table_cache_element *el;

  assert_owner();

  DBUG_ASSERT(table->in_use == thd);

  /*
    Try to get Table_cache_element representing this table in the cache
    from array in the TABLE_SHARE.
  */
  el = table->s->cache_element[table_cache_manager.cache_index(this)];

  if (!el) {
    /*
      If TABLE_SHARE doesn't have pointer to the element representing table
      in this cache, the element for the table must be absent from table the
      cache.

      Allocate new Table_cache_element object and add it to the cache
      and array in TABLE_SHARE.
    */
    std::string key(table->s->table_cache_key.str,
                    table->s->table_cache_key.length);
    DBUG_ASSERT(m_cache.count(key) == 0);

    el = new Table_cache_element(table->s);
    m_cache.emplace(key, std::unique_ptr<Table_cache_element>(el));
    table->s->cache_element[table_cache_manager.cache_index(this)] = el;
  }

  /* Add table to the used tables list */
  el->used_tables.push_front(table);

  m_table_count++;

  free_unused_tables_if_necessary(thd, acquire_lock);

  return false;
}

/**
   Prepare used or unused TABLE instance for destruction by removing
   it from the table cache.

   @note Caller should own lock on the table cache.
*/

void Table_cache::remove_table(TABLE *table) {
  Table_cache_element *el =
      table->s->cache_element[table_cache_manager.cache_index(this)];

  assert_owner();

  if (table->in_use) {
    /* Remove from per-table chain of used TABLE objects. */
    el->used_tables.remove(table);
  } else {
    /* Remove from per-table chain of unused TABLE objects. */
    el->free_tables.remove(table);

    /* And per-cache unused chain. */
    unlink_unused_table(table);
  }

  m_table_count--;

  if (el->used_tables.is_empty() && el->free_tables.is_empty()) {
    std::string key(table->s->table_cache_key.str,
                    table->s->table_cache_key.length);
    m_cache.erase(key);
    /*
      Remove reference to deleted cache element from array
      in the TABLE_SHARE.
    */
    table->s->cache_element[table_cache_manager.cache_index(this)] = nullptr;
  }
}

/**
  Get an unused TABLE instance from the table cache.

  @param      thd         Thread context.
  @param      key         Key identifying table.
  @param      key_length  Length of key for the table.
  @param[out] share       NULL - if table cache doesn't contain any
                          information about the table (i.e. doesn't have
                          neither used nor unused TABLE objects for it).
                          Pointer to TABLE_SHARE for the table otherwise.

  @note Caller should own lock on the table cache.
  @note Sets TABLE::in_use member as side effect.

  @retval non-NULL - pointer to unused TABLE object, "share" out-parameter
                     contains pointer to TABLE_SHARE for this table.
  @retval NULL     - no unused TABLE object was found, "share" parameter
                     contains pointer to TABLE_SHARE for this table if there
                     are used TABLE objects in cache and NULL otherwise.
*/

TABLE *Table_cache::get_table(THD *thd, const char *key, size_t key_length,
                              TABLE_SHARE **share) {
  TABLE *table;

  assert_owner();

  *share = nullptr;

  std::string key_str(key, key_length);
  const auto el_it = m_cache.find(key_str);
  if (el_it == m_cache.end()) return nullptr;
  Table_cache_element *el = el_it->second.get();

  *share = el->share;

  if ((table = el->free_tables.front())) {
    DBUG_ASSERT(!table->in_use);

    /*
      Unlink table from list of unused TABLE objects for this
      table in this cache.
    */
    el->free_tables.remove(table);

    /* Unlink table from unused tables list for this cache. */
    unlink_unused_table(table);

    /*
      Add table to list of used TABLE objects for this table
      in the table cache.
    */
    el->used_tables.push_front(table);

    table->in_use = thd;
    /* The ex-unused table must be fully functional. */
    DBUG_ASSERT(table->db_stat && table->file);
    /* The children must be detached from the table. */
    DBUG_ASSERT(!table->file->ha_extra(HA_EXTRA_IS_ATTACHED_CHILDREN));

    // update access time
    table->set_last_access_time();
  }

  return table;
}

/**
  Put used TABLE instance back to the table cache and mark
  it as unused.

  @note Caller should own lock on the table cache.
  @note Sets TABLE::in_use member as side effect.
*/

void Table_cache::release_table(THD *thd, TABLE *table) {
  Table_cache_element *el =
      table->s->cache_element[table_cache_manager.cache_index(this)];

  assert_owner();

  DBUG_ASSERT(table->in_use);
  DBUG_ASSERT(table->file);

  /* We shouldn't put the table to 'unused' list if the share is old. */
  DBUG_ASSERT(!table->s->has_old_version());

  table->in_use = nullptr;

  /* Remove TABLE from the list of used objects for the table in this cache. */
  el->used_tables.remove(table);
  /* Add TABLE to the list of unused objects for the table in this cache. */
  el->free_tables.push_front(table);
  /* Also link it last in the list of unused TABLE objects for the cache. */
  link_unused_table(table);

  /*
    We free the least used tables, not the subject table, to keep the LRU order.
    Note that in most common case the below call won't free anything.
  */
  free_unused_tables_if_necessary(thd);
}

/**
  Construct iterator over all used TABLE objects for the table share.

  @note Assumes that caller owns locks on all table caches.
*/
Table_cache_iterator::Table_cache_iterator(const TABLE_SHARE *share_arg)
    : share(share_arg), current_cache_index(0), current_table(nullptr) {
  table_cache_manager.assert_owner_all();
  move_to_next_table();
}

/** Helper that moves iterator to the next used TABLE for the table share. */

void Table_cache_iterator::move_to_next_table() {
  for (; current_cache_index < table_cache_instances; ++current_cache_index) {
    Table_cache_element *el;

    if ((el = share->cache_element[current_cache_index])) {
      if ((current_table = el->used_tables.front())) break;
    }
  }
}

/**
  Get current used TABLE instance and move iterator to the next one.

  @note Assumes that caller owns locks on all table caches.
*/

TABLE *Table_cache_iterator::operator++(int) {
  table_cache_manager.assert_owner_all();

  TABLE *result = current_table;

  if (current_table) {
    Table_cache_element::TABLE_list::Iterator it(
        share->cache_element[current_cache_index]->used_tables, current_table);

    current_table = ++it;

    if (!current_table) {
      ++current_cache_index;
      move_to_next_table();
    }
  }

  return result;
}

void Table_cache_iterator::rewind() {
  current_cache_index = 0;
  current_table = nullptr;
  move_to_next_table();
}

#endif /* TABLE_CACHE_INCLUDED */
