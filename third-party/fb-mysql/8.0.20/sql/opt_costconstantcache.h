#ifndef OPT_COSTCONSTANTCACHE_INCLUDED
#define OPT_COSTCONSTANTCACHE_INCLUDED

/*
   Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "my_dbug.h"
#include "mysql/components/services/mysql_mutex_bits.h"
#include "mysql/psi/mysql_mutex.h"
#include "sql/opt_costconstants.h"  // Cost_model_constants

/**
  This class implements a cache for "cost constant sets". This cache
  is responsible for creating the set of cost constant, giving new
  sessions access to the latest versions of the cost constants, and
  for re-reading the cost constant tables in the case where these have
  been updated.

  The cost constant cache keeps a copy of the current set of cost
  constants. Each time a new session initializes its Cost_model_server
  object (by calling Cost_model_server::init() in lex_start()), the
  Cost_model_server object will request the cost constant cache to
  give it the current version of the cost constants. This is done by
  calling Cost_constant_cache::get_cost_constants(). This function
  will just return a pointer to the current set of cost constants.  As
  time goes, new cost constant sets might be created and added to the
  cost constant cache. In order to know when a cost constant set can
  be deleted, reference counting is used. Each time a session asks for
  the cost constants, the reference counter is incremented. When the
  session releases the cost constant set by calling
  @c release_cost_constants(), the reference counter will be
  decremented. When the reference counter becomes zero, the cost
  constant set is deleted.
*/

class Cost_constant_cache {
 public:
  /**
    Creates an empty cost constant cache. To initialize it with default
    cost constants, @c init() must be called. To use cost constants from
    the cost constant tables, @c reload() must be called.
  */
  Cost_constant_cache();

  /**
    Destructor for the cost constant cache. Before the cost constant cache
    is deleted, @c close() must have been called.
  */
  ~Cost_constant_cache();

  /**
    Initialize the cost module.

    The cost constants will be initialized with the default values found in
    the source code. To start using the cost constant values found in
    the configuration tables, the @c reload() function must be called.
  */

  void init();

  /**
    Close the cost constant cache.

    All resources owned by the cost constant cache are released.
  */

  void close();

  /**
    Reload all cost constants from the configuration tables.
  */

  void reload();

  /**
    Get the currently used set of cost constants.

    This function will just return a pointer to a shared version of the
    cost constants. For tracking of how many sessions that is using the
    set and to be able to know when it is safe to delete the cost constant
    object, reference counting is used. This function will increase the
    ref count for the returned cost constant object. To decrease the reference
    counter when the cost constants are no longer used,
    @c release_cost_constants() must be called.

    @note To ensure that the reference counter is only incremented once for
    each session that uses the cost constant set, this function should only
    be called once per session.

    @return pointer to the cost constants
  */

  const Cost_model_constants *get_cost_constants() {
    mysql_mutex_lock(&LOCK_cost_const);

    // Increase the ref count on the cost constant object
    current_cost_constants->inc_ref_count();

    mysql_mutex_unlock(&LOCK_cost_const);

    return current_cost_constants;
  }

  /**
    Releases the cost constant set.

    This will decrement the reference counter on the cost constant set and
    if nobody is using it, it will be deleted. This function should be
    called each time a client (a session) no longer has any use for a
    cost constant set that it has previously gotten from calling
    @c get_cost_constants()

    @param cost_constants pointer to the cost constant set
  */

  void release_cost_constants(const Cost_model_constants *cost_constants) {
    DBUG_ASSERT(cost_constants != nullptr);

    /*
      The reason for using a const cast here is to be able to keep
      the cost constant object const outside of this module.
    */
    Cost_model_constants *cost =
        const_cast<Cost_model_constants *>(cost_constants);

    mysql_mutex_lock(&LOCK_cost_const);

    const unsigned int ref_count = cost->dec_ref_count();

    mysql_mutex_unlock(&LOCK_cost_const);

    // If none is using these cost constants then delete them
    if (ref_count == 0) delete cost;
  }

 private:
  /**
    Create default cost constants.

    This will create cost constants based on default values defined in the
    source code.
  */

  Cost_model_constants *create_defaults() const;

  /**
    Replace the current cost constants with a new set of cost constants.

    @param new_cost_constants the new cost constants
  */

  void update_current_cost_constants(Cost_model_constants *new_cost_constants);

  /**
    The current set of cost constants that will be used by new sessions.
  */
  Cost_model_constants *current_cost_constants;

  /**
    Mutex protecting the pointer to the current cost constant set and
    reference counting on all cost constant sets.
  */
  mysql_mutex_t LOCK_cost_const;

  bool m_inited;
};

/**
  Initializes the optimizer cost module. This should be done during
  startup from mysqld.cc.
*/

void init_optimizer_cost_module(bool enable_plugins);

/**
  Deletes the optimizer cost module. This should be called when
  the server stops to release allocated resources.
*/

void delete_optimizer_cost_module();

/**
  Reloads the optimizer cost constants from the cost constant tables.

  @note In order to read the cost constant tables, a THD is needed. This
  function will create a new temporary THD that will be used for
  this. In case the caller already has a THD this will not be used.
*/
void reload_optimizer_cost_constants();

#endif /* OPT_COSTCONSTANTCACHE_INCLUDED */
