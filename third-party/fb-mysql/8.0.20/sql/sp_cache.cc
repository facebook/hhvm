/* Copyright (c) 2002, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/sp_cache.h"

#include <stddef.h>
#include <atomic>
#include <memory>
#include <string>

#include "lex_string.h"
#include "m_ctype.h"
#include "map_helpers.h"
#include "my_dbug.h"
#include "sql/psi_memory_key.h"
#include "sql/sp_head.h"

/*
  Cache of stored routines.
*/

class sp_cache {
 public:
  sp_cache() : m_hashtable(system_charset_info, key_memory_sp_cache) {}

  ~sp_cache() {}

  /**
   Inserts a sp_head object into a hash table.

   @returns Success status
     @return true Failure
     @return false Success
  */
  bool insert(sp_head *sp) {
    m_hashtable.emplace(to_string(sp->m_qname),
                        std::unique_ptr<sp_head, sp_head_deleter>(sp));
    return false;
  }

  sp_head *lookup(char *name, size_t namelen) {
    return find_or_nullptr(m_hashtable, std::string(name, namelen));
  }

  void remove(sp_head *sp) { m_hashtable.erase(to_string(sp->m_qname)); }

  /**
    Remove all elements from a stored routine cache if the current
    number of elements exceeds the argument value.

    @param[in] upper_limit_for_elements  Soft upper limit of elements that
                                         can be stored in the cache.
  */
  void enforce_limit(ulong upper_limit_for_elements) {
    if (m_hashtable.size() > upper_limit_for_elements) m_hashtable.clear();
  }

 private:
  struct sp_head_deleter {
    void operator()(sp_head *sp) const { sp_head::destroy(sp); }
  };

  /* All routines in this cache */
  collation_unordered_map<std::string,
                          std::unique_ptr<sp_head, sp_head_deleter>>
      m_hashtable;
};  // class sp_cache

static std::atomic<int64> atomic_Cversion{0};

/*
  Clear the cache *cp and set *cp to NULL.

  SYNOPSIS
    sp_cache_clear()
    cp  Pointer to cache to clear

  NOTE
    This function doesn't invalidate other caches.
*/

void sp_cache_clear(sp_cache **cp) {
  sp_cache *c = *cp;

  if (c) {
    delete c;
    *cp = nullptr;
  }
}

/*
  Insert a routine into the cache.

  SYNOPSIS
    sp_cache_insert()
     cp  The cache to put routine into
     sp  Routine to insert.

  TODO: Perhaps it will be more straightforward if in case we returned an
        error from this function when we couldn't allocate sp_cache. (right
        now failure to put routine into cache will cause a 'SP not found'
        error to be reported at some later time)
*/

void sp_cache_insert(sp_cache **cp, sp_head *sp) {
  sp_cache *c;

  if (!(c = *cp)) {
    if (!(c = new sp_cache())) return;  // End of memory error
  }
  sp->set_sp_cache_version(sp_cache_version());
  DBUG_PRINT("info", ("sp_cache: inserting: %.*s", (int)sp->m_qname.length,
                      sp->m_qname.str));
  c->insert(sp);
  *cp = c;  // Update *cp if it was NULL
}

/*
  Look up a routine in the cache.
  SYNOPSIS
    sp_cache_lookup()
      cp    Cache to look into
      name  Name of rutine to find

  NOTE
    An obsolete (but not more obsolete then since last
    sp_cache_flush_obsolete call) routine may be returned.

  RETURN
    The routine or
    NULL if the routine not found.
*/

sp_head *sp_cache_lookup(sp_cache **cp, sp_name *name) {
  sp_cache *c = *cp;
  if (!c) return nullptr;
  return c->lookup(name->m_qname.str, name->m_qname.length);
}

/*
  Invalidate all routines in all caches.

  SYNOPSIS
    sp_cache_invalidate()

  NOTE
    This is called when a VIEW definition is created or modified (and in some
    other contexts). We can't destroy sp_head objects here as one may modify
    VIEW definitions from prelocking-free SPs.
*/

void sp_cache_invalidate() {
  DBUG_PRINT("info", ("sp_cache: invalidating"));
  atomic_Cversion++;
}

/**
  Remove an out-of-date SP from the cache.

  @param[in] cp  Cache to flush
  @param[in] sp  SP to remove.

  @note This invalidates pointers to sp_head objects this thread
  uses. In practice that means 'dont call this function when
  inside SP'.
*/

void sp_cache_flush_obsolete(sp_cache **cp, sp_head **sp) {
  if ((*sp)->sp_cache_version() < sp_cache_version() && !(*sp)->is_invoked()) {
    (*cp)->remove(*sp);
    *sp = nullptr;
  }
}

/**
  Return the current global version of the cache.
*/

int64 sp_cache_version() { return atomic_Cversion; }

/**
  Enforce that the current number of elements in the cache don't exceed
  the argument value by flushing the cache if necessary.

  @param[in] c  Cache to check
  @param[in] upper_limit_for_elements  Soft upper limit for number of sp_head
                                       objects that can be stored in the cache.
*/
void sp_cache_enforce_limit(sp_cache *c, ulong upper_limit_for_elements) {
  if (c) c->enforce_limit(upper_limit_for_elements);
}
