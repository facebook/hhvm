/* Copyright (c) 2007, 2020, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/mdl.h"

#include <time.h>
#include <algorithm>
#include <atomic>
#include <functional>

#include "lf.h"
#include "m_ctype.h"
#include "my_dbug.h"
#include "my_macros.h"
#include "my_murmur3.h"
#include "my_sharedlib.h"
#include "my_sys.h"
#include "my_systime.h"
#include "my_thread.h"
#include "mysql/components/services/psi_cond_bits.h"
#include "mysql/components/services/psi_memory_bits.h"
#include "mysql/components/services/psi_mutex_bits.h"
#include "mysql/components/services/psi_rwlock_bits.h"
#include "mysql/psi/mysql_cond.h"
#include "mysql/psi/mysql_mdl.h"
#include "mysql/psi/mysql_memory.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysql/psi/mysql_stage.h"
#include "mysql/psi/psi_base.h"
#include "mysql/psi/psi_mdl.h"
#include "mysql/service_thd_wait.h"
#include "mysqld_error.h"
#include "prealloced_array.h"
#include "sql/auth/auth_acls.h"
#include "sql/debug_sync.h"
#include "sql/mysqld.h"
#include "sql/sql_class.h"
#include "sql/sql_lex.h"
#include "sql/sql_parse.h"  // support_high_priority
#include "sql/thr_malloc.h"

extern MYSQL_PLUGIN_IMPORT CHARSET_INFO *system_charset_info;

static PSI_memory_key key_memory_MDL_context_acquire_locks;

#ifdef HAVE_PSI_INTERFACE
static PSI_mutex_key key_MDL_wait_LOCK_wait_status;

static PSI_mutex_info all_mdl_mutexes[] = {{&key_MDL_wait_LOCK_wait_status,
                                            "MDL_wait::LOCK_wait_status", 0, 0,
                                            PSI_DOCUMENT_ME}};

static PSI_rwlock_key key_MDL_lock_rwlock;
static PSI_rwlock_key key_MDL_context_LOCK_waiting_for;

static PSI_rwlock_info all_mdl_rwlocks[] = {
    {&key_MDL_lock_rwlock, "MDL_lock::rwlock", PSI_FLAG_RWLOCK_PR, 0,
     PSI_DOCUMENT_ME},
    {&key_MDL_context_LOCK_waiting_for, "MDL_context::LOCK_waiting_for",
     PSI_FLAG_RWLOCK_PR, 0, PSI_DOCUMENT_ME}};

static PSI_cond_key key_MDL_wait_COND_wait_status;

static PSI_cond_info all_mdl_conds[] = {{&key_MDL_wait_COND_wait_status,
                                         "MDL_context::COND_wait_status", 0, 0,
                                         PSI_DOCUMENT_ME}};

static PSI_memory_info all_mdl_memory[] = {
    {&key_memory_MDL_context_acquire_locks, "MDL_context::acquire_locks", 0, 0,
     PSI_DOCUMENT_ME}};

/**
  Initialise all the performance schema instrumentation points
  used by the MDL subsystem.
*/
static void init_mdl_psi_keys(void) {
  int count;

  count = static_cast<int>(array_elements(all_mdl_mutexes));
  mysql_mutex_register("sql", all_mdl_mutexes, count);

  count = static_cast<int>(array_elements(all_mdl_rwlocks));
  mysql_rwlock_register("sql", all_mdl_rwlocks, count);

  count = static_cast<int>(array_elements(all_mdl_conds));
  mysql_cond_register("sql", all_mdl_conds, count);

  count = static_cast<int>(array_elements(all_mdl_memory));
  mysql_memory_register("sql", all_mdl_memory, count);

  MDL_key::init_psi_keys();
}
#endif /* HAVE_PSI_INTERFACE */

/**
  Thread state names to be used in case when we have to wait on resource
  belonging to certain namespace.
  Warning: stage_info->m_key can be modified by pfs_register_stage_v1
*/

MDL_key::PSI_stage_info_with_name
    MDL_key::m_namespace_to_wait_state_name[NAMESPACE_END] = {
        {{0, "Waiting for global read lock", 0, PSI_DOCUMENT_ME},
         "global read"},
        {{0, "Waiting for backup lock", 0, PSI_DOCUMENT_ME}, "backup"},
        {{0, "Waiting for tablespace metadata lock", 0, PSI_DOCUMENT_ME},
         "tablespace metadata"},
        {{0, "Waiting for schema metadata lock", 0, PSI_DOCUMENT_ME},
         "schema metadata"},
        {{0, "Waiting for table metadata lock", 0, PSI_DOCUMENT_ME},
         "table metadata"},
        {{0, "Waiting for stored function metadata lock", 0, PSI_DOCUMENT_ME},
         "stored function metadata"},
        {{0, "Waiting for stored procedure metadata lock", 0, PSI_DOCUMENT_ME},
         "stored procedure metadata"},
        {{0, "Waiting for trigger metadata lock", 0, PSI_DOCUMENT_ME},
         "trigger metadata"},
        {{0, "Waiting for event metadata lock", 0, PSI_DOCUMENT_ME},
         "event metadata"},
        {{0, "Waiting for commit lock", 0, PSI_DOCUMENT_ME}, "commit"},
        {{0, "User lock", 0, PSI_DOCUMENT_ME},
         /* Be compatible with old status. */ "user"},
        {{0, "Waiting for locking service lock", 0, PSI_DOCUMENT_ME},
         "locking service"},
        {{0, "Waiting for spatial reference system lock", 0, PSI_DOCUMENT_ME},
         "spatial reference system"},
        {{0, "Waiting for acl cache lock", 0, PSI_DOCUMENT_ME}, "acl cache"},
        {{0, "Waiting for column statistics lock", 0, PSI_DOCUMENT_ME},
         "column statistics"},
        {{0, "Waiting for resource groups metadata lock", 0, PSI_DOCUMENT_ME},
         "resource groups metadata"},
        {{0, "Waiting for foreign key metadata lock", 0, PSI_DOCUMENT_ME},
         "foreign key metadata"},
        {{0, "Waiting for check constraint metadata lock", 0, PSI_DOCUMENT_ME},
         "constraint metadata"}};

#ifdef HAVE_PSI_INTERFACE
void MDL_key::init_psi_keys() {
  int i;
  int count;
  PSI_stage_info *info MY_ATTRIBUTE((unused));

  count =
      static_cast<int>(array_elements(MDL_key::m_namespace_to_wait_state_name));
  for (i = 0; i < count; i++) {
    /* mysql_stage_register wants an array of pointers, registering 1 by 1. */
    info = &MDL_key::m_namespace_to_wait_state_name[i].stage_info;
    mysql_stage_register("sql", &info, 1);
  }
}
#endif

static bool mdl_initialized = false;

/**
  A collection of all MDL locks. A singleton,
  there is only one instance of the map in the server.
*/

class MDL_map {
 public:
  void init();
  void destroy();

  inline MDL_lock *find(LF_PINS *pins, const MDL_key *key, bool *pinned);
  inline MDL_lock *find_or_insert(LF_PINS *pins, const MDL_key *key,
                                  bool *pinned);

  /**
    Decrement unused MDL_lock objects counter.
  */
  void lock_object_used() { --m_unused_lock_objects; }

  /**
    Increment unused MDL_lock objects counter. If number of such objects
    exceeds threshold and unused/total objects ratio is high enough try
    to free some of them.
  */
  void lock_object_unused(MDL_context *ctx, LF_PINS *pins) {
    /*
      Use thread local copy of unused locks counter for performance/
      scalability reasons. It is updated on both successful and failed
      attempts to delete unused MDL_lock objects in order to avoid infinite
      loops,
    */
    int32 unused_locks = ++m_unused_lock_objects;

    while (unused_locks > mdl_locks_unused_locks_low_water &&
           (unused_locks > m_locks.count * MDL_LOCKS_UNUSED_LOCKS_MIN_RATIO)) {
      /*
        If number of unused lock objects exceeds low water threshold and
        unused/total objects ratio is high enough - try to do random dive
        into m_locks hash, find an unused object by iterating upwards
        through its split-ordered list and try to free it.
        If we fail to do this - update local copy of unused objects
        counter and retry if needed,

        Note that:
        *) It is not big deal if "m_unused_lock_objects" due to races becomes
           negative temporarily as we perform signed comparison.
        *) There is a good chance that we will find an unused object quickly
           because unused/total ratio is high enough.
        *) There is no possibility for infinite loop since our PRNG works
           in such way that we eventually cycle through all LF_HASH hash
           buckets (@sa MDL_context::get_random()).
        *) Thanks to the fact that we choose random object to expel -
           objects which are used more often will naturally stay
           in the cache and rarely used objects will be expelled from it.
        *) Non-atomic read of LF_HASH::count which happens above should be
           OK as LF_HASH code does them too + preceding atomic operation
           provides memory barrier.
      */
      remove_random_unused(ctx, pins, &unused_locks);
    }
  }

  /**
    Get number of unused MDL_lock objects in MDL_map cache.

    @note Does non-atomic read so can return stale results. This is OK since
          this method is used only in unit-tests. The latter employ means
          of thread synchronization which are external to MDL and prevent
          memory reordering/ensure that thread calling this method have
          up-to-date view on the memory. @sa m_unused_lock_objects.
  */
  int32 get_unused_locks_count() const { return m_unused_lock_objects.load(); }

  /**
    Allocate pins which are necessary for MDL_context/thread to be able
    to work with MDL_map container.
  */
  LF_PINS *get_pins() { return lf_hash_get_pins(&m_locks); }

  /**
    Check if MDL_lock object corresponding to the key is going to be
    singleton.
  */
  bool is_lock_object_singleton(const MDL_key *mdl_key) const {
    return (mdl_key->mdl_namespace() == MDL_key::GLOBAL ||
            mdl_key->mdl_namespace() == MDL_key::COMMIT ||
            mdl_key->mdl_namespace() == MDL_key::ACL_CACHE ||
            mdl_key->mdl_namespace() == MDL_key::BACKUP_LOCK);
  }

 private:
  void remove_random_unused(MDL_context *ctx, LF_PINS *pins,
                            int32 *unused_locks);

  /** LF_HASH with all locks in the server. */
  LF_HASH m_locks;
  /** Pre-allocated MDL_lock object for GLOBAL namespace. */
  MDL_lock *m_global_lock;
  /** Pre-allocated MDL_lock object for COMMIT namespace. */
  MDL_lock *m_commit_lock;
  /** Pre-allocated MDL_lock object for ACL_CACHE namespace. */
  MDL_lock *m_acl_cache_lock;
  /** Pre-allocated MDL_lock object for BACKUP_LOCK namespace. */
  MDL_lock *m_backup_lock;

  /**
    Number of unused MDL_lock objects in the server.

    Updated using atomic operations, read using both atomic and ordinary
    reads. We assume that ordinary reads of 32-bit words can't result in
    partial results, but may produce stale results thanks to memory
    reordering, LF_HASH seems to be using similar assumption.

    Note that due to fact that updates to this counter are not atomic with
    marking MDL_lock objects as used/unused it might easily get negative
    for some short period of time. Code which uses its value needs to take
    this into account.
  */
  std::atomic<int32> m_unused_lock_objects;
};

/**
  Threshold for number of unused MDL_lock objects.

  We will start considering freeing some unused objects only after exceeding
  this value and if unused/total objects ratio is high enough.

  Normally this threshold is constant. It is exposed outside of MDL subsystem
  as a variable only in order to simplify unit testing.
*/
int32 mdl_locks_unused_locks_low_water =
    MDL_LOCKS_UNUSED_LOCKS_LOW_WATER_DEFAULT;

/**
  A context of the recursive traversal through all contexts
  in all sessions in search for deadlock.
*/

class Deadlock_detection_visitor : public MDL_wait_for_graph_visitor {
 public:
  Deadlock_detection_visitor(MDL_context *start_node_arg)
      : m_start_node(start_node_arg),
        m_victim(nullptr),
        m_current_search_depth(0),
        m_found_deadlock(false) {}
  virtual bool enter_node(MDL_context *node);
  virtual void leave_node(MDL_context *node);

  virtual bool inspect_edge(MDL_context *dest);

  MDL_context *get_victim() const { return m_victim; }

 private:
  /**
    Change the deadlock victim to a new one if it has lower deadlock
    weight.
  */
  void opt_change_victim_to(MDL_context *new_victim);

 private:
  /**
    The context which has initiated the search. There
    can be multiple searches happening in parallel at the same time.
  */
  MDL_context *m_start_node;
  /** If a deadlock is found, the context that identifies the victim. */
  MDL_context *m_victim;
  /** Set to the 0 at start. Increased whenever
    we descend into another MDL context (aka traverse to the next
    wait-for graph node). When MAX_SEARCH_DEPTH is reached, we
    assume that a deadlock is found, even if we have not found a
    loop.
  */
  uint m_current_search_depth;
  /** true if we found a deadlock. */
  bool m_found_deadlock;
  /**
    Maximum depth for deadlock searches. After this depth is
    achieved we will unconditionally declare that there is a
    deadlock.

    @note This depth should be small enough to avoid stack
          being exhausted by recursive search algorithm.

    TODO: Find out what is the optimal value for this parameter.
          Current value is safe, but probably sub-optimal,
          as there is an anecdotal evidence that real-life
          deadlocks are even shorter typically.
  */
  static const uint MAX_SEARCH_DEPTH = 32;
};

/**
  Enter a node of a wait-for graph. After
  a node is entered, inspect_edge() will be called
  for all wait-for destinations of this node. Then
  leave_node() will be called.
  We call "enter_node()" for all nodes we inspect,
  including the starting node.

  @retval  true  Maximum search depth exceeded.
  @retval  false OK.
*/

bool Deadlock_detection_visitor::enter_node(MDL_context *node) {
  m_found_deadlock = ++m_current_search_depth >= MAX_SEARCH_DEPTH;
  if (m_found_deadlock) {
    DBUG_ASSERT(!m_victim);
    opt_change_victim_to(node);
  }
  return m_found_deadlock;
}

/**
  Done inspecting this node. Decrease the search
  depth. If a deadlock is found, and we are
  backtracking to the start node, optionally
  change the deadlock victim to one with lower
  deadlock weight.
*/

void Deadlock_detection_visitor::leave_node(MDL_context *node) {
  --m_current_search_depth;
  if (m_found_deadlock) opt_change_victim_to(node);
}

/**
  Inspect a wait-for graph edge from one MDL context to another.

  @retval true   A loop is found.
  @retval false  No loop is found.
*/

bool Deadlock_detection_visitor::inspect_edge(MDL_context *node) {
  m_found_deadlock = node == m_start_node;
  return m_found_deadlock;
}

/**
  Change the deadlock victim to a new one if it has lower deadlock
  weight.

  @param new_victim New candidate for deadlock victim.
*/

void Deadlock_detection_visitor::opt_change_victim_to(MDL_context *new_victim) {
  if (m_victim == nullptr ||
      m_victim->get_deadlock_weight() >= new_victim->get_deadlock_weight()) {
    /* Swap victims, unlock the old one. */
    MDL_context *tmp = m_victim;
    m_victim = new_victim;
    m_victim->lock_deadlock_victim();
    if (tmp) tmp->unlock_deadlock_victim();
  }
}

/**
  Get a bit corresponding to enum_mdl_type value in a granted/waiting bitmaps
  and compatibility matrices.
*/

#define MDL_BIT(A) static_cast<MDL_lock::bitmap_t>(1U << A)

/**
  The lock context. Created internally for an acquired lock.
  For a given name, there exists only one MDL_lock instance,
  and it exists only when the lock has been granted.
  Can be seen as an MDL subsystem's version of TABLE_SHARE.

  This is an abstract class which lacks information about
  compatibility rules for lock types. They should be specified
  in its descendants.
*/

class MDL_lock {
 public:
  typedef unsigned short bitmap_t;

  class Ticket_list {
   public:
    typedef I_P_List<MDL_ticket,
                     I_P_List_adapter<MDL_ticket, &MDL_ticket::next_in_lock,
                                      &MDL_ticket::prev_in_lock>,
                     I_P_List_null_counter, I_P_List_fast_push_back<MDL_ticket>>
        List;
    operator const List &() const { return m_list; }
    Ticket_list() : m_bitmap(0) {}

    void add_ticket(MDL_ticket *ticket);
    void remove_ticket(MDL_ticket *ticket);
    bool is_empty() const { return m_list.is_empty(); }
    bitmap_t bitmap() const { return m_bitmap; }

   private:
    void clear_bit_if_not_in_list(enum_mdl_type type);

   private:
    /** List of tickets. */
    List m_list;
    /** Bitmap of types of tickets in this list. */
    bitmap_t m_bitmap;
  };

  typedef Ticket_list::List::Iterator Ticket_iterator;

  typedef longlong fast_path_state_t;

  /**
    Helper struct which defines how different types of locks are handled
    for a specific MDL_lock. In practice we use only two strategies: "scoped"
    lock strategy for locks in GLOBAL, COMMIT, TABLESPACE and SCHEMA namespaces
    and "object" lock strategy for all other namespaces.
  */
  struct MDL_lock_strategy {
    /**
      Compatibility (or rather "incompatibility") matrices for lock types.

      Array of bitmaps which elements specify which granted locks are
      incompatible with the type of lock being requested.
    */
    bitmap_t m_granted_incompatible[MDL_TYPE_END];
    /**
      Arrays of bitmaps which elements specify which waiting locks are
      incompatible with the type of lock being requested. Basically, each
      array defines priorities between lock types.
      We need 4 separate arrays since in order to prevent starvation for
      some of lock request types, we use different priority matrices:
      0) in "normal" situation.
      1) in situation when the number of successively granted "piglet" requests
         exceeds the max_write_lock_count limit.
      2) in situation when the number of successively granted "hog" requests
         exceeds the max_write_lock_count limit.
      3) in situation when both "piglet" and "hog" counters exceed limit.
    */
    bitmap_t m_waiting_incompatible[4][MDL_TYPE_END];
    /**
      Array of increments for "unobtrusive" types of lock requests for locks.
      @sa MDL_lock::get_unobtrusive_lock_increment().
    */
    fast_path_state_t m_unobtrusive_lock_increment[MDL_TYPE_END];
    /**
      Indicates that locks of this type are affected by
      the max_write_lock_count limit.
    */
    bool m_is_affected_by_max_write_lock_count;

#ifndef DBUG_OFF
    /**
     Indicate that a type is legal with this strategy. Only for asserts and
     debug-only checks.
     */
    bool legal_type[MDL_TYPE_END];
#endif /* not defined DBUG_OFF */

    /**
      Pointer to a static method which determines if the type of lock
      requested requires notification of conflicting locks. NULL if there
      are no lock types requiring notification.
    */
    bool (*m_needs_notification)(const MDL_ticket *ticket);
    /**
      Pointer to a static method which allows notification of owners of
      conflicting locks about the fact that a type of lock requiring
      notification was requested.
    */
    void (*m_notify_conflicting_locks)(MDL_context *ctx, MDL_lock *lock);

    bool (*m_kill_conflicting_locks)(MDL_context *ctx, MDL_lock *lock,
                                     enum_mdl_type kill_lower_than);
    /**
      Pointer to a static method which converts information about
      locks granted using "fast" path from fast_path_state_t
      representation to bitmap of lock types.
    */
    bitmap_t (*m_fast_path_granted_bitmap)(const MDL_lock &lock);
    /**
      Pointer to a static method which determines if waiting for the lock
      should be aborted when when connection is lost. NULL if locks of
      this type don't require such aborts.
    */
    bool (*m_needs_connection_check)(const MDL_lock *lock);
  };

 public:
  /** The key of the object (data) being protected. */
  MDL_key key;
  /**
    Read-write lock protecting this lock context.

    @note The fact that we use read-write lock prefers readers here is
          important as deadlock detector won't work correctly otherwise.

          For example, imagine that we have following waiters graph:

                       ctxA -> obj1 -> ctxB -> obj1 -|
                        ^                            |
                        |----------------------------|

          and both ctxA and ctxB start deadlock detection process:

            ctxA read-locks obj1             ctxB read-locks obj2
            ctxA goes deeper                 ctxB goes deeper

          Now ctxC comes in who wants to start waiting on obj1, also
          ctxD comes in who wants to start waiting on obj2.

            ctxC tries to write-lock obj1   ctxD tries to write-lock obj2
            ctxC is blocked                 ctxD is blocked

          Now ctxA and ctxB resume their search:

            ctxA tries to read-lock obj2    ctxB tries to read-lock obj1

          If m_rwlock prefers writes (or fair) both ctxA and ctxB would be
          blocked because of pending write locks from ctxD and ctxC
          correspondingly. Thus we will get a deadlock in deadlock detector.
          If m_wrlock prefers readers (actually ignoring pending writers is
          enough) ctxA and ctxB will continue and no deadlock will occur.
  */
  mysql_prlock_t m_rwlock;

  const bitmap_t *incompatible_granted_types_bitmap() const {
    return m_strategy->m_granted_incompatible;
  }

  const bitmap_t *incompatible_waiting_types_bitmap() const {
    return m_strategy
        ->m_waiting_incompatible[m_current_waiting_incompatible_idx];
  }

  /**
    Get index of priority matrice in MDL_lock_strategy::m_waiting_incompatible
    array which corresponds to current values of the m_piglet_lock_count and
    m_hog_lock_count counters and the max_write_lock_count threshold.
  */
  uint get_incompatible_waiting_types_bitmap_idx() const {
    mysql_prlock_assert_write_owner(&m_rwlock);
    /*
      To prevent starvation for lock types with lower priority use:

      *) MDL_lock_strategy::m_waiting_incompatible[0] matrice by default.
      *) MDL_lock_strategy::m_waiting_incompatible[1] when the number of
         successively granted "piglet" requests exceeds max_write_lock_count.
      *) MDL_lock_strategy::m_waiting_incompatible[2] when the number of
         successively granted "hog" requests exceeds max_write_lock_count.
      *) MDL_lock_strategy::m_waiting_incompatible[3] when both "piglet" and
         "hog" counters exceed this limit.
    */
    uint idx = 0;
    if (m_piglet_lock_count >= max_write_lock_count) idx += 1;
    if (m_hog_lock_count >= max_write_lock_count) idx += 2;
    return idx;
  }

  /**
    Switch priority matrice for the MDL_lock object if m_piglet_lock_count or/
    and m_hog_lock_count counters have crossed max_write_lock_count threshold.

    @returns true - if priority matrice has been changed, false - otherwise.
  */
  bool switch_incompatible_waiting_types_bitmap_if_needed() {
    mysql_prlock_assert_write_owner(&m_rwlock);

    uint new_idx = get_incompatible_waiting_types_bitmap_idx();
    if (m_current_waiting_incompatible_idx == new_idx) return false;
    m_current_waiting_incompatible_idx = new_idx;
    return true;
  }

  bool has_pending_conflicting_lock(enum_mdl_type type);

  bool can_grant_lock(enum_mdl_type type,
                      const MDL_context *requestor_ctx) const;

  void reschedule_waiters();

  void remove_ticket(MDL_context *ctx, LF_PINS *pins,
                     Ticket_list MDL_lock::*queue, MDL_ticket *ticket);

  bool visit_subgraph(MDL_ticket *waiting_ticket,
                      MDL_wait_for_graph_visitor *gvisitor);

  bool needs_notification(const MDL_ticket *ticket) const {
    return m_strategy->m_needs_notification
               ? m_strategy->m_needs_notification(ticket)
               : false;
  }

  void notify_conflicting_locks(MDL_context *ctx) {
    if (m_strategy->m_notify_conflicting_locks)
      m_strategy->m_notify_conflicting_locks(ctx, this);
  }

  bool kill_conflicting_locks(MDL_context *ctx, enum_mdl_type kill_lower_than) {
    return m_strategy->m_kill_conflicting_locks
               ? m_strategy->m_kill_conflicting_locks(ctx, this,
                                                      kill_lower_than)
               : false;
  }

  bool needs_connection_check() const {
    return m_strategy->m_needs_connection_check
               ? m_strategy->m_needs_connection_check(this)
               : false;
  }

  inline static bool needs_hton_notification(
      MDL_key::enum_mdl_namespace mdl_namespace);

  bool is_affected_by_max_write_lock_count() const {
    return m_strategy->m_is_affected_by_max_write_lock_count;
  }

  /**
    If we just have granted a lock of "piglet" or "hog" type and there are
    pending lower priority locks, increase the appropriate counter. If this
    counter now exceeds the max_write_lock_count threshold, switch priority
    matrice for the MDL_lock object.

    @returns true - if priority matrice has been changed, false - otherwise.
  */
  bool count_piglets_and_hogs(enum_mdl_type type) {
    mysql_prlock_assert_write_owner(&m_rwlock);

    if ((MDL_BIT(type) & MDL_OBJECT_HOG_LOCK_TYPES) != 0) {
      if (m_waiting.bitmap() & ~MDL_OBJECT_HOG_LOCK_TYPES) {
        m_hog_lock_count++;
        if (switch_incompatible_waiting_types_bitmap_if_needed()) return true;
      }
    } else if (type == MDL_SHARED_WRITE) {
      if (m_waiting.bitmap() & MDL_BIT(MDL_SHARED_READ_ONLY)) {
        m_piglet_lock_count++;
        if (switch_incompatible_waiting_types_bitmap_if_needed()) return true;
      }
    }
    return false;
  }

  /**
    @returns "Fast path" increment for request for "unobtrusive" type
              of lock, 0 - if it is request for "obtrusive" type of
              lock.

    @note We split all lock types for each of MDL namespaces
          in two sets:

          A) "unobtrusive" lock types
            1) Each type from this set should be compatible with all other
               types from the set (including itself).
            2) These types should be common for DML operations

          Our goal is to optimize acquisition and release of locks of this
          type by avoiding complex checks and manipulations on m_waiting/
          m_granted bitmaps/lists. We replace them with a check of and
          increment/decrement of integer counters.
          We call the latter type of acquisition/release "fast path".
          Use of "fast path" reduces the size of critical section associated
          with MDL_lock::m_rwlock lock in the common case and thus increases
          scalability.

          The amount by which acquisition/release of specific type
          "unobtrusive" lock increases/decreases packed counter in
          MDL_lock::m_fast_path_state is returned by this function.

          B) "obtrusive" lock types
            1) Granted or pending lock of those type is incompatible with
               some other types of locks or with itself.
            2) Not common for DML operations

          These locks have to be always acquired involving manipulations on
          m_waiting/m_granted bitmaps/lists, i.e. we have to use "slow path"
          for them. Moreover in the presence of active/pending locks from
          "obtrusive" set we have to acquire using "slow path" even locks of
          "unobtrusive" type.

    @see MDL_scoped_lock::m_unobtrusive_lock_increment and
    @see MDL_object_lock::m_unobtrusive_lock_increment for
        definitions of these sets for scoped and per-object locks.
  */
  inline static fast_path_state_t get_unobtrusive_lock_increment(
      const MDL_request *request);

  /**
    @returns "Fast path" increment if type of lock is "unobtrusive" type,
              0 - if it is "obtrusive" type of lock.
  */
  fast_path_state_t get_unobtrusive_lock_increment(enum_mdl_type type) const {
    return m_strategy->m_unobtrusive_lock_increment[type];
  }

  /**
    Check if type of lock requested is "obtrusive" type of lock.

    @sa MDL_lock::get_unobtrusive_lock_increment() description.
  */
  bool is_obtrusive_lock(enum_mdl_type type) const {
    return get_unobtrusive_lock_increment(type) == 0;
  }

  /**
    Return set of types of lock requests which were granted using
    "fast path" algorithm in the bitmap_t form.

    This method is only called from MDL_lock::can_grant_lock() and its
    return value is only important when we are trying to figure out if
    we can grant an obtrusive lock. But this means that the HAS_OBTRUSIVE
    flag is set so all changes to m_fast_path_state happen under protection
    of MDL_lock::m_rwlock (see invariant [INV1]).
    Since can_grant_lock() is called only when MDL_lock::m_rwlock is held,
    it is safe to do an ordinary read of m_fast_path_state here.
  */
  bitmap_t fast_path_granted_bitmap() const {
    return m_strategy->m_fast_path_granted_bitmap(*this);
  }

  /** List of granted tickets for this lock. */
  Ticket_list m_granted;
  /** Tickets for contexts waiting to acquire a lock. */
  Ticket_list m_waiting;

 private:
  /**
    Number of times high priority, "hog" lock requests (X, SNRW, SNW) have been
    granted while lower priority lock requests (all other types) were waiting.
    Currently used only for object locks. Protected by m_rwlock lock.
  */
  ulong m_hog_lock_count;
  /**
    Number of times high priority, "piglet" lock requests (SW) have been
    granted while locks requests with lower priority (SRO) were waiting.
    Currently used only for object locks. Protected by m_rwlock lock.
  */
  ulong m_piglet_lock_count;
  /**
    Index of one of the MDL_lock_strategy::m_waiting_incompatible
    arrays which represents the current priority matrice.
  */
  uint m_current_waiting_incompatible_idx;

 public:
  /**
    Do "expensive" part of MDL_lock object initialization,
    Called by LF_ALLOCATOR for each newly malloc()'ed MDL_lock object, is not
    called in cases when LF_ALLOCATOR decides to reuse object which was
    returned to it earlier. "Full" initialization happens later by calling
    MDL_lock::reinit(). So @sa MDL_lock::reiniti()
  */
  MDL_lock() : m_obtrusive_locks_granted_waiting_count(0) {
    mysql_prlock_init(key_MDL_lock_rwlock, &m_rwlock);
  }

  inline void reinit(const MDL_key *mdl_key);

  ~MDL_lock() { mysql_prlock_destroy(&m_rwlock); }

  inline static MDL_lock *create(const MDL_key *key);
  inline static void destroy(MDL_lock *lock);

  inline MDL_context *get_lock_owner() const;

  /**
    Get MDL lock strategy corresponding to MDL key.

    @param key Reference to MDL_key object

    @return the type of strategy scoped or object corresponding to MDL key.
  */

  inline static const MDL_lock_strategy *get_strategy(const MDL_key &key) {
    switch (key.mdl_namespace()) {
      case MDL_key::GLOBAL:
      case MDL_key::TABLESPACE:
      case MDL_key::SCHEMA:
      case MDL_key::COMMIT:
      case MDL_key::BACKUP_LOCK:
      case MDL_key::RESOURCE_GROUPS:
      case MDL_key::FOREIGN_KEY:
      case MDL_key::CHECK_CONSTRAINT:
        return &m_scoped_lock_strategy;
      default:
        return &m_object_lock_strategy;
    }
  }

 public:
  /**
    Number of granted or waiting lock requests of "obtrusive" type.
    Also includes "obtrusive" lock requests for which we about to check
    if they can be granted.


    @sa MDL_lock::get_unobtrusive_lock_increment() description.

    @note This number doesn't include "unobtrusive" locks which were acquired
          using "slow path".
  */
  uint m_obtrusive_locks_granted_waiting_count;
  /**
    Flag in MDL_lock::m_fast_path_state that indicates that the MDL_lock
    object was marked for destruction and will be destroyed once all threads
    referencing to it through hazard pointers have unpinned it.
    Set using atomic compare-and-swap AND under protection of
    MDL_lock::m_rwlock lock.
    Thanks to this can be read either by using atomic compare-and-swap OR
    using ordinary read under protection of MDL_lock::m_rwlock lock.
  */
  static const fast_path_state_t IS_DESTROYED = 1ULL << 62;
  /**
    Flag in MDL_lock::m_fast_path_state that indicates that there are
    "obtrusive" locks which are granted, waiting or for which we are
    about to check if they can be granted.
    Corresponds to "MDL_lock::m_obtrusive_locks_granted_waiting_count == 0"
    predicate.
    Set using atomic compare-and-swap AND under protection of
    MDL_lock::m_rwlock lock.
    Thanks to this can be read either by using atomic compare-and-swap OR
    using ordinary read under protection of MDL_lock::m_rwlock lock.

    Invariant [INV1]: When this flag is set all changes to m_fast_path_state
    member has to be done under protection of m_rwlock lock.
  */
  static const fast_path_state_t HAS_OBTRUSIVE = 1ULL << 61;
  /**
    Flag in MDL_lock::m_fast_path_state that indicates that there are
    "slow" path locks which are granted, waiting or for which we are
    about to check if they can be granted.
    Corresponds to MDL_lock::m_granted/m_waiting lists being non-empty
    (except special case in MDL_context::try_acquire_lock()).
    Set using atomic compare-and-swap AND under protection of m_rwlock
    lock. The latter is necessary because value of this flag needs to be
    synchronized with contents of MDL_lock::m_granted/m_waiting lists.
  */
  static const fast_path_state_t HAS_SLOW_PATH = 1ULL << 60;
  /**
    Combination of IS_DESTROYED/HAS_OBTRUSIVE/HAS_SLOW_PATH flags and packed
    counters of specific types of "unobtrusive" locks which were granted using
    "fast path".

    @see MDL_scoped_lock::m_unobtrusive_lock_increment and
        @see MDL_object_lock::m_unobtrusive_lock_increment for details about how
        counts of different types of locks are packed into this field.

    @note Doesn't include "unobtrusive" locks granted using "slow path".

    @note We use combination of atomic operations and protection by
          MDL_lock::m_rwlock lock to work with this member:

          * Write and Read-Modify-Write operations are always carried out
            atomically. This is necessary to avoid lost updates on 32-bit
            platforms among other things.
          * In some cases Reads can be done non-atomically because we don't
            really care about value which they will return (for example,
            if further down the line there will be an atomic compare-and-swap
            operation, which will validate this value and provide the correct
            value if the validation will fail).
          * In other cases Reads can be done non-atomically since they happen
            under protection of MDL_lock::m_rwlock and there is some invariant
            which ensures that concurrent updates of the m_fast_path_state
            member can't happen while  MDL_lock::m_rwlock is held
            (@sa IS_DESTROYED, HAS_OBTRUSIVE, HAS_SLOW_PATH).

    @note IMPORTANT!!!
          In order to enforce the above rules and other invariants,
          MDL_lock::m_fast_path_state should not be updated directly.
          Use fast_path_state_cas()/add()/reset() wrapper methods instead.
  */
  std::atomic<fast_path_state_t> m_fast_path_state;

  /**
    Wrapper for atomic compare-and-swap operation on m_fast_path_state member
    which enforces locking and other invariants.
  */
  bool fast_path_state_cas(fast_path_state_t *old_state,
                           fast_path_state_t new_state) {
    /*
      IS_DESTROYED, HAS_OBTRUSIVE and HAS_SLOW_PATH flags can be set or
      cleared only while holding MDL_lock::m_rwlock lock.
      If HAS_SLOW_PATH flag is set all changes to m_fast_path_state
      should happen under protection of MDL_lock::m_rwlock ([INV1]).
    */
#if !defined(DBUG_OFF)
    if (((*old_state & (IS_DESTROYED | HAS_OBTRUSIVE | HAS_SLOW_PATH)) !=
         (new_state & (IS_DESTROYED | HAS_OBTRUSIVE | HAS_SLOW_PATH))) ||
        *old_state & HAS_OBTRUSIVE) {
      mysql_prlock_assert_write_owner(&m_rwlock);
    }
#endif
    /*
      We should not change state of destroyed object
      (fast_path_state_reset() being exception).
    */
    DBUG_ASSERT(!(*old_state & IS_DESTROYED));

    return atomic_compare_exchange_strong(&m_fast_path_state, old_state,
                                          new_state);
  }

  /**
    Wrapper for atomic add operation on m_fast_path_state member
    which enforces locking and other invariants.
  */
  fast_path_state_t fast_path_state_add(fast_path_state_t value) {
    /*
      Invariant [INV1] requires all changes to m_fast_path_state happen
      under protection of m_rwlock if HAS_OBTRUSIVE flag is set.
      Since this operation doesn't check this flag it can be called only
      under protection of m_rwlock.
    */
    mysql_prlock_assert_write_owner(&m_rwlock);

    fast_path_state_t old_state = m_fast_path_state.fetch_add(value);

    /*
      We should not change state of destroyed object
      (fast_path_state_reset() being exception).
    */
    DBUG_ASSERT(!(old_state & IS_DESTROYED));
    return old_state;
  }

  /**
    Wrapper for resetting m_fast_path_state enforcing locking invariants.
  */
  void fast_path_state_reset() {
    /* HAS_DESTROYED flag can be cleared only under protection of m_rwlock. */
    mysql_prlock_assert_write_owner(&m_rwlock);
    m_fast_path_state.store(0);
  }

  /**
    Pointer to strategy object which defines how different types of lock
    requests should be handled for the namespace to which this lock belongs.
    @sa MDL_lock::m_scoped_lock_strategy and MDL_lock:m_object_lock_strategy.
  */
  const MDL_lock_strategy *m_strategy;

  static bool scoped_lock_kill_conflicting_locks(MDL_context *ctx,
                                                 MDL_lock *lock,
                                                 enum_mdl_type kill_lower_than);
  /**
    Get bitmap of "unobtrusive" locks granted using "fast path" algorithm
    for scoped locks.

    @sa MDL_lock::fast_path_granted_bitmap() for explanation about why it
        is safe to use non-atomic read of MDL_lock::m_fast_path_state here.
  */
  static bitmap_t scoped_lock_fast_path_granted_bitmap(const MDL_lock &lock) {
    return (lock.m_fast_path_state.load() & 0xFFFFFFFFFFFFFFFULL)
               ? MDL_BIT(MDL_INTENTION_EXCLUSIVE)
               : 0;
  }

  /**
    Check if we are requesting X lock on the object, so threads holding
    conflicting S/SH metadata locks on it need to be notified.

    @sa MDL_lock::object_lock_notify_conflicting_locks.
  */
  static bool object_lock_needs_notification(const MDL_ticket *ticket) {
    return (ticket->get_type() == MDL_EXCLUSIVE);
  }
  static void object_lock_notify_conflicting_locks(MDL_context *ctx,
                                                   MDL_lock *lock);

  static bool object_lock_kill_conflicting_locks(MDL_context *ctx,
                                                 MDL_lock *lock,
                                                 enum_mdl_type kill_lower_than);
  /**
    Get bitmap of "unobtrusive" locks granted using "fast path" algorithm
    for per-object locks.

    @sa MDL_lock::fast_path_granted_bitmap() for explanation about why it
        is safe to use non-atomic read of MDL_lock::m_fast_path_state here.
  */
  static bitmap_t object_lock_fast_path_granted_bitmap(const MDL_lock &lock) {
    bitmap_t result = 0;
    fast_path_state_t fps = lock.m_fast_path_state;
    if (fps & 0xFFFFFULL) result |= MDL_BIT(MDL_SHARED);
    if (fps & (0xFFFFFULL << 20)) result |= MDL_BIT(MDL_SHARED_READ);
    if (fps & (0xFFFFFULL << 40)) result |= MDL_BIT(MDL_SHARED_WRITE);
    return result;
  }

  /**
    Check if MDL_lock object represents user-level lock,locking service
    lock or acl cache lock, so threads waiting for it need to check if
    connection is lost and abort waiting when it is.
  */
  static bool object_lock_needs_connection_check(const MDL_lock *lock) {
    return (lock->key.mdl_namespace() == MDL_key::USER_LEVEL_LOCK ||
            lock->key.mdl_namespace() == MDL_key::LOCKING_SERVICE ||
            lock->key.mdl_namespace() == MDL_key::ACL_CACHE);
  }

  /**
    Bitmap with "hog" lock types for object locks.

    Locks of these types can easily starve out lower priority locks.
    To prevent this we only grant them max_write_lock_count times in
    a row while other lock types are waiting.
  */
  static const bitmap_t MDL_OBJECT_HOG_LOCK_TYPES =
      (MDL_BIT(MDL_SHARED_NO_WRITE) | MDL_BIT(MDL_SHARED_NO_READ_WRITE) |
       MDL_BIT(MDL_EXCLUSIVE));

  static const MDL_lock_strategy m_scoped_lock_strategy;
  static const MDL_lock_strategy m_object_lock_strategy;
};

static MDL_map mdl_locks;

static const uchar *mdl_locks_key(const uchar *record, size_t *length) {
  const MDL_lock *lock = pointer_cast<const MDL_lock *>(record);
  *length = lock->key.length();
  return lock->key.ptr();
}

/**
  Initialize the metadata locking subsystem.

  This function is called at server startup.

  In particular, initializes the new global mutex and
  the associated condition variable: LOCK_mdl and COND_mdl.
  These locking primitives are implementation details of the MDL
  subsystem and are private to it.
*/

void mdl_init() {
  DBUG_ASSERT(!mdl_initialized);
  mdl_initialized = true;

#ifdef HAVE_PSI_INTERFACE
  init_mdl_psi_keys();
#endif

  mdl_locks.init();
}

/**
  Release resources of metadata locking subsystem.

  Destroys the global mutex and the condition variable.
  Called at server shutdown.
*/

void mdl_destroy() {
  if (mdl_initialized) {
    mdl_initialized = false;
    mdl_locks.destroy();
  }
}

/**
  Get number of unused MDL_lock objects in MDL_map cache.
  Mostly needed for unit-testing.
*/

int32 mdl_get_unused_locks_count() {
  return mdl_locks.get_unused_locks_count();
}

extern "C" {
static void mdl_lock_cons(uchar *arg) {
  new (arg + LF_HASH_OVERHEAD) MDL_lock();
}

static void mdl_lock_dtor(uchar *arg) {
  MDL_lock *lock = (MDL_lock *)(arg + LF_HASH_OVERHEAD);
  lock->~MDL_lock();
}

static void mdl_lock_reinit(uchar *dst_arg, const uchar *src_arg) {
  MDL_lock *dst = (MDL_lock *)dst_arg;
  const MDL_key *src = (const MDL_key *)src_arg;
  dst->reinit(src);
}

/**
  Adapter function which allows to use murmur3 with LF_HASH implementation.
*/

static uint murmur3_adapter(const LF_HASH *, const uchar *key, size_t length) {
  return murmur3_32(key, length, 0);
}

} /* extern "C" */

/** Initialize the container for all MDL locks. */

void MDL_map::init() {
  MDL_key global_lock_key(MDL_key::GLOBAL, "", "");
  MDL_key commit_lock_key(MDL_key::COMMIT, "", "");
  MDL_key acl_cache_lock_key(MDL_key::ACL_CACHE, "", "");
  MDL_key backup_lock_key(MDL_key::BACKUP_LOCK, "", "");

  m_global_lock = MDL_lock::create(&global_lock_key);
  m_commit_lock = MDL_lock::create(&commit_lock_key);
  m_acl_cache_lock = MDL_lock::create(&acl_cache_lock_key);
  m_backup_lock = MDL_lock::create(&backup_lock_key);

  m_unused_lock_objects = 0;

  lf_hash_init2(&m_locks, sizeof(MDL_lock), LF_HASH_UNIQUE, 0, 0, mdl_locks_key,
                &my_charset_bin, &murmur3_adapter, &mdl_lock_cons,
                &mdl_lock_dtor, &mdl_lock_reinit);
}

/**
  Destroy the container for all MDL locks.
  @pre It must be empty.
*/

void MDL_map::destroy() {
  MDL_lock::destroy(m_global_lock);
  MDL_lock::destroy(m_commit_lock);
  MDL_lock::destroy(m_acl_cache_lock);
  MDL_lock::destroy(m_backup_lock);

  lf_hash_destroy(&m_locks);
}

/**
  Find MDL_lock object corresponding to the key.

  @param[in,out]  pins     LF_PINS to be used for pinning pointers during
                           look-up and returned MDL_lock object.
  @param[in]      mdl_key  Key for which MDL_lock object needs to be found.
  @param[out]     pinned   true  - if MDL_lock object is pinned,
                           false - if MDL_lock object doesn't require pinning
                                   (i.e. it is an object for GLOBAL, COMMIT or
                                   ACL_CACHE namespaces).

  @retval MY_LF_ERRPTR   - Failure (OOM)
  @retval other-non-NULL - MDL_lock object found.
  @retval NULL           - Object not found.
*/

MDL_lock *MDL_map::find(LF_PINS *pins, const MDL_key *mdl_key, bool *pinned) {
  MDL_lock *lock = nullptr;

  if (is_lock_object_singleton(mdl_key)) {
    /*
      Avoid look up in m_locks hash when lock for GLOBAL, COMMIT or ACL_CACHE
      namespace is requested. Return pointer to pre-allocated MDL_lock instance
      instead. Such an optimization allows us to avoid a few atomic operations
      for any statement changing data.

      It works since these namespaces contain only one element so keys
      for them look like '<namespace-id>\0\0'.
    */
    DBUG_ASSERT(mdl_key->length() == 3);

    switch (mdl_key->mdl_namespace()) {
      case MDL_key::GLOBAL:
        lock = m_global_lock;
        break;
      case MDL_key::COMMIT:
        lock = m_commit_lock;
        break;
      case MDL_key::ACL_CACHE:
        lock = m_acl_cache_lock;
        break;
      case MDL_key::BACKUP_LOCK:
        lock = m_backup_lock;
        break;
      default:
        DBUG_ASSERT(false);
    }

    *pinned = false;

    return lock;
  }

  lock = static_cast<MDL_lock *>(
      lf_hash_search(&m_locks, pins, mdl_key->ptr(), mdl_key->length()));

  if (lock == nullptr || lock == MY_LF_ERRPTR) {
    lf_hash_search_unpin(pins);
    *pinned = false;  // Avoid warnings on older compilers.
    return lock;
  }

  *pinned = true;

  return lock;
}

/**
  Find MDL_lock object corresponding to the key, create it
  if it does not exist.

  @param[in,out]  pins     LF_PINS to be used for pinning pointers during
                           look-up and returned MDL_lock object.
  @param[in]      mdl_key  Key for which MDL_lock object needs to be found.
  @param[out]     pinned   true  - if MDL_lock object is pinned,
                           false - if MDL_lock object doesn't require pinning
                                   (i.e. it is an object for GLOBAL, COMMIT or
                                   ACL_CACHE namespaces).

  @retval non-NULL - Success. MDL_lock instance for the key with
                     locked MDL_lock::m_rwlock.
  @retval NULL     - Failure (OOM).
*/

MDL_lock *MDL_map::find_or_insert(LF_PINS *pins, const MDL_key *mdl_key,
                                  bool *pinned) {
  MDL_lock *lock = nullptr;

  while ((lock = find(pins, mdl_key, pinned)) == nullptr) {
    /*
      MDL_lock for key isn't present in hash, try to insert new object.
      This can fail due to concurrent inserts.
    */
    int rc = lf_hash_insert(&m_locks, pins, mdl_key);
    if (rc == -1) /* If OOM. */
      return nullptr;
    else if (rc == 0) {
      /*
        New MDL_lock object is not used yet. So we need to
        increment number of unused lock objects.
      */
      ++m_unused_lock_objects;
    }
  }
  if (lock == MY_LF_ERRPTR) {
    /* If OOM in lf_hash_search. */
    return nullptr;
  }

  return lock;
}

extern "C" {
/**
  Helper function which allows to check if MDL_lock object stored in LF_HASH
  is unused - i.e. doesn't have any locks on both "fast" and "slow" paths
  and is not marked as deleted.
*/
static int mdl_lock_match_unused(const uchar *arg) {
  const MDL_lock *lock = (const MDL_lock *)arg;
  /*
    It is OK to check MDL_lock::m_fast_path_state non-atomically here
    since the fact that MDL_lock object is unused will be properly
    validated later anyway.
  */
  return (lock->m_fast_path_state.load() == 0);
}
} /* extern "C" */

/**
  Try to find random MDL_lock object in MDL_map for which there are no "fast"
  path nor "slow" path locks. If found - mark it as destroyed, remove object
  from MDL_map and return it back to allocator.

  @param[in]     ctx           Context on which behalf we are trying to remove
                               unused object. Primarily needed to generate
                               random value to be used for random dive into
                               the hash in MDL_map.
  @param[in,out] pins          Pins for the calling thread to be used for
                               hash lookup and deletion.
  @param[out]    unused_locks  Number of unused lock objects after operation.

  @note
    In reality MDL_lock object will be returned to allocator once it is no
    longer pinned by any threads.
*/

void MDL_map::remove_random_unused(MDL_context *ctx, LF_PINS *pins,
                                   int32 *unused_locks) {
  DEBUG_SYNC(ctx->get_thd(), "mdl_remove_random_unused_before_search");

  /*
    Try to find an unused MDL_lock object by doing random dive into the hash.
    Since this method is called only when unused/total lock objects ratio is
    high enough, there is a good chance for this technique to succeed.
  */
  MDL_lock *lock = static_cast<MDL_lock *>(lf_hash_random_match(
      &m_locks, pins, &mdl_lock_match_unused, ctx->get_random()));

  if (lock == nullptr || lock == MY_LF_ERRPTR) {
    /*
      We were unlucky and no unused objects were found. This can happen,
      for example, if our random dive into LF_HASH was close to the tail
      of split-ordered list used in its implementation or if some other
      thread managed to destroy or start re-using MDL_lock object
      concurrently.
     */
    lf_hash_search_unpin(pins);
    *unused_locks = m_unused_lock_objects;
    return;
  }

  DEBUG_SYNC(ctx->get_thd(), "mdl_remove_random_unused_after_search");

  /*
    Acquire MDL_lock::m_rwlock to ensure that IS_DESTROYED flag is set
    atomically AND under protection of MDL_lock::m_rwlock, so it can be
    safely read using both atomics and ordinary read under protection of
    m_rwlock. This also means that it is safe to unpin MDL_lock object
    after we have checked its IS_DESTROYED flag if we keep m_rwlock lock.
  */
  mysql_prlock_wrlock(&lock->m_rwlock);

  if (lock->m_fast_path_state & MDL_lock::IS_DESTROYED) {
    /*
      Somebody has managed to mark MDL_lock object as destroyed before
      we have acquired MDL_lock::m_rwlock.
    */
    mysql_prlock_unlock(&lock->m_rwlock);
    lf_hash_search_unpin(pins);
    *unused_locks = m_unused_lock_objects;
    return;
  }
  lf_hash_search_unpin(pins);

  /*
    Atomically check that number of "fast path" and "slow path" locks is 0 and
    set IS_DESTROYED flag.

    This is the only place where we rely on the fact that our compare-and-swap
    operation can't spuriously fail i.e. is of strong kind.
  */
  MDL_lock::fast_path_state_t old_state = 0;

  if (lock->fast_path_state_cas(&old_state, MDL_lock::IS_DESTROYED)) {
    /*
      There were no "fast path" or "slow path" references and we
      have successfully set IS_DESTROYED flag.
    */
    mysql_prlock_unlock(&lock->m_rwlock);

    DEBUG_SYNC(ctx->get_thd(),
               "mdl_remove_random_unused_after_is_destroyed_set");

    /*
      Even though other threads can't rely on the MDL_lock object being around
      once IS_DESTROYED flag is set, we know that it was not removed from
      the hash yet (as it is responsibility of the current thread, i.e. one
      which executes this MDL_map::remove_random_unused() call) and thus was
      not deallocated.
      And since lf_hash_delete() finds and pins the object for the key as its
      first step and keeps pins until its end it is safe to use MDL_lock::key
      as parameter to lf_hash_delete().
    */
    int rc =
        lf_hash_delete(&m_locks, pins, lock->key.ptr(), lock->key.length());

    /* The MDL_lock object must be present in the hash. */
    DBUG_ASSERT(rc != 1);

    if (rc == -1) {
      /*
        In unlikely case of OOM MDL_lock object stays in the hash. The best
        thing we can do is to reset IS_DESTROYED flag. The object will be
        destroyed either by further calls to lf_hash_delete() or by final
        call to lf_hash_destroy().
        Resetting needs to happen atomically AND under protection of
        MDL_lock::m_rwlock so it safe to read this flag both using atomics
        and ordinary reads under protection of m_rwlock lock.
      */
      mysql_prlock_wrlock(&lock->m_rwlock);
      lock->fast_path_state_reset();
      mysql_prlock_unlock(&lock->m_rwlock);
    } else {
      /* Success. */
      *unused_locks = --m_unused_lock_objects;
    }
  } else {
    /*
      Some other thread has managed to find and use this MDL_lock object after
      it has been found by the above call to lf_hash_random_match().
      There are/were "fast" or "slow path" references so MDL_lock object can't
      be deleted.

      Assert that compare-and-swap operation is of strong kind and can't
      fail spuriously.
    */
    DBUG_ASSERT(old_state != 0);
    mysql_prlock_unlock(&lock->m_rwlock);
    *unused_locks = m_unused_lock_objects;
  }
}

/**
  Initialize a metadata locking context.

  This is to be called when a new server connection is created.
*/

MDL_context::MDL_context()
    : m_owner(nullptr),
      m_needs_thr_lock_abort(false),
      m_force_dml_deadlock_weight(false),
      m_waiting_for(nullptr),
      m_pins(nullptr),
      m_rand_state(UINT_MAX32),
      m_ignore_owner_thd(false) {
  mysql_prlock_init(key_MDL_context_LOCK_waiting_for, &m_LOCK_waiting_for);
}

/**
  Destroy metadata locking context.

  Assumes and asserts that there are no active or pending locks
  associated with this context at the time of the destruction.

  Currently does nothing. Asserts that there are no pending
  or satisfied lock requests. The pending locks must be released
  prior to destruction. This is a new way to express the assertion
  that all tables are closed before a connection is destroyed.
*/

void MDL_context::destroy() {
  DBUG_ASSERT(m_ticket_store.is_empty());

  mysql_prlock_destroy(&m_LOCK_waiting_for);
  if (m_pins) lf_hash_put_pins(m_pins);
}

/**
  Allocate pins which are necessary to work with MDL_map container
  if they are not allocated already.
*/

bool MDL_context::fix_pins() {
  if (!m_pins) m_pins = mdl_locks.get_pins();
  return (m_pins == nullptr);
}

/**
  Initialize a lock request.

  This is to be used for every lock request.

  Note that initialization and allocation are split into two
  calls. This is to allow flexible memory management of lock
  requests. Normally a lock request is stored in statement memory
  (e.g. is a member of struct TABLE_LIST), but we would also like
  to allow allocation of lock requests in other memory roots,
  for example in the grant subsystem, to lock privilege tables.

  The MDL subsystem does not own or manage memory of lock requests.

  @param  mdl_namespace  Id of namespace of object to be locked
  @param  db_arg         Name of database to which the object belongs
  @param  name_arg       Name of of the object
  @param  mdl_type_arg   The MDL lock type for the request.
  @param  mdl_duration_arg   The MDL duration for the request.
  @param  src_file       Source file name issuing the request.
  @param  src_line       Source line number issuing the request.
*/

void MDL_request::init_with_source(MDL_key::enum_mdl_namespace mdl_namespace,
                                   const char *db_arg, const char *name_arg,
                                   enum_mdl_type mdl_type_arg,
                                   enum_mdl_duration mdl_duration_arg,
                                   const char *src_file, uint src_line) {
#if !defined(DBUG_OFF)
  // Make sure all I_S tables (except ndb tables) are in CAPITAL letters.
  bool is_ndb_table = (name_arg && (strncmp(name_arg, "ndb", 3) == 0));
  DBUG_ASSERT(
      mdl_namespace != MDL_key::TABLE ||
      my_strcasecmp(system_charset_info, "information_schema", db_arg) ||
      is_ndb_table || !name_arg ||
      my_isupper(system_charset_info, name_arg[0]));
#endif

  key.mdl_key_init(mdl_namespace, db_arg, name_arg);
  type = mdl_type_arg;
  duration = mdl_duration_arg;
  ticket = nullptr;
  m_src_file = src_file;
  m_src_line = src_line;
}

/**
  Initialize a lock request using pre-built MDL_key.

  @sa MDL_request::init(namespace, db, name, type).

  @param key_arg       The pre-built MDL key for the request.
  @param mdl_type_arg  The MDL lock type for the request.
  @param mdl_duration_arg   The MDL duration for the request.
  @param src_file      Source file name issuing the request.
  @param src_line      Source line number issuing the request.
*/

void MDL_request::init_by_key_with_source(const MDL_key *key_arg,
                                          enum_mdl_type mdl_type_arg,
                                          enum_mdl_duration mdl_duration_arg,
                                          const char *src_file, uint src_line) {
  key.mdl_key_init(key_arg);
  type = mdl_type_arg;
  duration = mdl_duration_arg;
  ticket = nullptr;
  m_src_file = src_file;
  m_src_line = src_line;
}

/**
  Initialize a lock request using partial MDL key.

  @sa MDL_request::init(namespace, db, name, type).

  @remark The partial key must be "<database>\0<name>\0".

  @param  namespace_arg       Id of namespace of object to be locked
  @param  part_key_arg        Partial key.
  @param  part_key_length_arg Partial key length
  @param  db_length_arg       Database name length.
  @param  mdl_type_arg        The MDL lock type for the request.
  @param  mdl_duration_arg    The MDL duration for the request.
  @param  src_file            Source file name issuing the request.
  @param  src_line            Source line number issuing the request.
*/

void MDL_request::init_by_part_key_with_source(
    MDL_key::enum_mdl_namespace namespace_arg, const char *part_key_arg,
    size_t part_key_length_arg, size_t db_length_arg,
    enum_mdl_type mdl_type_arg, enum_mdl_duration mdl_duration_arg,
    const char *src_file, uint src_line) {
  key.mdl_key_init(namespace_arg, part_key_arg, part_key_length_arg,
                   db_length_arg);
  type = mdl_type_arg;
  duration = mdl_duration_arg;
  ticket = nullptr;
  m_src_file = src_file;
  m_src_line = src_line;
}

/**
  Auxiliary functions needed for creation/destruction of MDL_lock objects.
*/

inline MDL_lock *MDL_lock::create(const MDL_key *mdl_key) {
  MDL_lock *result = new (std::nothrow) MDL_lock();
  if (result) result->reinit(mdl_key);
  return result;
}

void MDL_lock::destroy(MDL_lock *lock) { delete lock; }

/**
  Finalize initialization or re-initialize MDL_lock returned from
  LF_ALLOCATOR's cache to represent object identified by provided key.

  @note All non-static MDL_lock members:
        1) either have to be reinitialized here
           (like IS_DESTROYED flag in MDL_lock::m_fast_path_state).
        2) or need to be initialized in constructor AND returned to their
           pristine state once they are removed from MDL_map container
           (like MDL_lock::m_granted or MDL_lock::m_rwlock).
           Otherwise it is possible that we will end up in situation
           when "new" (actually reused) MDL_lock object inserted in
           LF_HASH will inherit some values from old object.
*/

inline void MDL_lock::reinit(const MDL_key *mdl_key) {
  key.mdl_key_init(mdl_key);
  m_strategy = MDL_lock::get_strategy(*mdl_key);
  m_hog_lock_count = 0;
  m_piglet_lock_count = 0;
  m_current_waiting_incompatible_idx = 0;
  m_fast_path_state = 0;
  /*
    Check that we have clean "m_granted" and "m_waiting" sets/lists in both
    cases when we have fresh and re-used object.
  */
  DBUG_ASSERT(m_granted.is_empty() && m_waiting.is_empty());
  /* The same should be true for "m_obtrusive_locks_granted_waiting_count". */
  DBUG_ASSERT(m_obtrusive_locks_granted_waiting_count == 0);
}

/**
  @returns "Fast path" increment for request for "unobtrusive" type
            of lock, 0 - if it is request for "obtrusive" type of
            lock.

  @sa Description at method declaration for more details.
*/

MDL_lock::fast_path_state_t MDL_lock::get_unobtrusive_lock_increment(
    const MDL_request *request) {
  return MDL_lock::get_strategy(request->key)
      ->m_unobtrusive_lock_increment[request->type];
}

/**
  Indicates whether object belongs to namespace which requires storage engine
  to be notified before acquiring and after releasing exclusive lock.
*/

bool MDL_lock::needs_hton_notification(
    MDL_key::enum_mdl_namespace mdl_namespace) {
  switch (mdl_namespace) {
    case MDL_key::TABLESPACE:
    case MDL_key::SCHEMA:
    case MDL_key::TABLE:
    case MDL_key::FUNCTION:
    case MDL_key::PROCEDURE:
    case MDL_key::TRIGGER:
    case MDL_key::EVENT:
      return true;
    default:
      return false;
  }
}

/**
  Auxiliary functions needed for creation/destruction of MDL_ticket
  objects.

  @todo This naive implementation should be replaced with one that saves
        on memory allocation by reusing released objects.
*/

MDL_ticket *MDL_ticket::create(MDL_context *ctx_arg, enum_mdl_type type_arg
#ifndef DBUG_OFF
                               ,
                               enum_mdl_duration duration_arg
#endif
) {
  return new (std::nothrow) MDL_ticket(ctx_arg, type_arg
#ifndef DBUG_OFF
                                       ,
                                       duration_arg
#endif
  );
}

void MDL_ticket::destroy(MDL_ticket *ticket) {
  mysql_mdl_destroy(ticket->m_psi);
  ticket->m_psi = nullptr;

  delete ticket;
}

/**
  Return the 'weight' of this ticket for the victim selection algorithm.
  Requests with lower weight are preferred to requests with higher weight
  when choosing a victim.

  @note When MDL_context::m_force_dml_deadlock_weight is set, the return value
        of this method is ignored and DEADLOCK_WEIGHT_DML is used for the
        context.
*/

uint MDL_ticket::get_deadlock_weight() const {
  /*
    Waits for user-level locks have lower weight than waits for locks
    typically acquired by DDL, so we don't abort DDL in case of deadlock
    involving user-level locks and DDL. Deadlock errors are not normally
    expected from DDL by users.
    Waits for user-level locks have higher weight than waits for locks
    acquired by DML, so we prefer to abort DML in case of deadlock involving
    user-level locks and DML. User-level locks are explicitly requested by
    user, so they are probably important for them. Plus users expect
    deadlocks from DML transactions and for DML statements executed in
    @@autocommit=1 mode back-off and retry algorithm hides deadlock errors.
  */
  if (m_lock->key.mdl_namespace() == MDL_key::USER_LEVEL_LOCK)
    return DEADLOCK_WEIGHT_ULL;

  /*
    Locks higher or equal to MDL_SHARED_UPGRADABLE:
    *) Are typically acquired for DDL and LOCK TABLES statements.
    *) Are often acquired in a way which doesn't allow simple release of
       locks and restart of lock acquisition process in case of deadlock
       (e.g. through lock_table_names() call).

    To avoid such statements getting aborted with ER_LOCK_DEADLOCK error
    we use the higher DEADLOCK_WEIGHT_DDL weight for them.

    Note that two DDL statements should not typically deadlock with each
    other since they normally acquire locks in the same order, thanks to
    to the fact that lock_table_names() uses MDL_context::acquire_locks()
    method which sorts lock requests before trying to acquire them.

    In cases when "strong" locks can be acquired out-of-order (e.g. for
    LOCK TABLES) we try to use DEADLOCK_WEIGHT_DML instead.

    TODO/FIXME: The below condition needs to be updated. The fact that a
                lock from GLOBAL namespace is requested no longer means
                that this is a DDL statement. There is a bug report about
                this.
  */
  if (m_lock->key.mdl_namespace() == MDL_key::GLOBAL ||
      m_type >= MDL_SHARED_UPGRADABLE)
    return DEADLOCK_WEIGHT_DDL;

  return DEADLOCK_WEIGHT_DML;
}

/** Construct an empty wait slot. */

MDL_wait::MDL_wait() : m_wait_status(WS_EMPTY) {
  mysql_mutex_init(key_MDL_wait_LOCK_wait_status, &m_LOCK_wait_status, nullptr);
  mysql_cond_init(key_MDL_wait_COND_wait_status, &m_COND_wait_status);
}

/** Destroy system resources. */

MDL_wait::~MDL_wait() {
  mysql_mutex_destroy(&m_LOCK_wait_status);
  mysql_cond_destroy(&m_COND_wait_status);
}

/**
  Set the status unless it's already set. Return false if set,
  true otherwise.
*/

bool MDL_wait::set_status(enum_wait_status status_arg) {
  bool was_occupied = true;
  mysql_mutex_lock(&m_LOCK_wait_status);
  if (m_wait_status == WS_EMPTY) {
    was_occupied = false;
    m_wait_status = status_arg;
    mysql_cond_signal(&m_COND_wait_status);
  }
  mysql_mutex_unlock(&m_LOCK_wait_status);
  return was_occupied;
}

/** Query the current value of the wait slot. */

MDL_wait::enum_wait_status MDL_wait::get_status() {
  enum_wait_status result;
  mysql_mutex_lock(&m_LOCK_wait_status);
  result = m_wait_status;
  mysql_mutex_unlock(&m_LOCK_wait_status);
  return result;
}

/** Clear the current value of the wait slot. */

void MDL_wait::reset_status() {
  mysql_mutex_lock(&m_LOCK_wait_status);
  m_wait_status = WS_EMPTY;
  mysql_mutex_unlock(&m_LOCK_wait_status);
}

/**
  Wait for the status to be assigned to this wait slot.

  @param owner           MDL context owner.
  @param abs_timeout     Absolute time after which waiting should stop.
  @param set_status_on_timeout true  - If in case of timeout waiting
                                       context should close the wait slot by
                                       sending TIMEOUT to itself.
                               false - Otherwise.
  @param wait_state_name  Thread state name to be set for duration of wait.

  @returns Signal posted.
*/

MDL_wait::enum_wait_status MDL_wait::timed_wait(
    MDL_context_owner *owner, struct timespec *abs_timeout,
    bool set_status_on_timeout, const PSI_stage_info *wait_state_name) {
  PSI_stage_info old_stage;
  enum_wait_status result;
  int wait_result = 0;

  mysql_mutex_lock(&m_LOCK_wait_status);

  owner->ENTER_COND(&m_COND_wait_status, &m_LOCK_wait_status, wait_state_name,
                    &old_stage);
  thd_wait_begin(owner->get_thd(), THD_WAIT_META_DATA_LOCK);
  while (!m_wait_status && !owner->is_killed() && !is_timeout(wait_result)) {
    wait_result = mysql_cond_timedwait(&m_COND_wait_status, &m_LOCK_wait_status,
                                       abs_timeout);
  }

  if (m_wait_status == WS_EMPTY) {
    /*
      Wait has ended not due to a status being set from another
      thread but due to this connection/statement being killed or a
      time out.
      To avoid races, which may occur if another thread sets
      GRANTED status before the code which calls this method
      processes the abort/timeout, we assign the status under
      protection of the m_LOCK_wait_status, within the critical
      section. An exception is when set_status_on_timeout is
      false, which means that the caller intends to restart the
      wait.
    */
    if (owner->is_killed())
      m_wait_status = KILLED;
    else if (set_status_on_timeout)
      m_wait_status = TIMEOUT;
  }
  result = m_wait_status;

  mysql_mutex_unlock(&m_LOCK_wait_status);
  owner->EXIT_COND(&old_stage);
  thd_wait_end(owner->get_thd());

  return result;
}

/**
  Clear bit corresponding to the type of metadata lock in bitmap representing
  set of such types if list of tickets does not contain ticket with such type.

  @param[in]      type    Type of metadata lock to look up in the list.
*/

void MDL_lock::Ticket_list::clear_bit_if_not_in_list(enum_mdl_type type) {
  MDL_lock::Ticket_iterator it(m_list);
  const MDL_ticket *ticket;

  while ((ticket = it++))
    if (ticket->get_type() == type) return;
  m_bitmap &= ~MDL_BIT(type);
}

/**
  Add ticket to MDL_lock's list of waiting requests and
  update corresponding bitmap of lock types.
*/

void MDL_lock::Ticket_list::add_ticket(MDL_ticket *ticket) {
  /*
    Ticket being added to the list must have MDL_ticket::m_lock set,
    since for such tickets methods accessing this member might be
    called by other threads.
  */
  DBUG_ASSERT(ticket->get_lock());
  /*
    Add ticket to the *back* of the queue to ensure fairness
    among requests with the same priority.
  */
  m_list.push_back(ticket);
  m_bitmap |= MDL_BIT(ticket->get_type());
}

/**
  Remove ticket from MDL_lock's list of requests and
  update corresponding bitmap of lock types.
*/

void MDL_lock::Ticket_list::remove_ticket(MDL_ticket *ticket) {
  m_list.remove(ticket);
  /*
    Check if waiting queue has another ticket with the same type as
    one which was removed. If there is no such ticket, i.e. we have
    removed last ticket of particular type, then we need to update
    bitmap of waiting ticket's types.
    Note that in most common case, i.e. when shared lock is removed
    from waiting queue, we are likely to find ticket of the same
    type early without performing full iteration through the list.
    So this method should not be too expensive.
  */
  clear_bit_if_not_in_list(ticket->get_type());
}

/**
  Determine waiting contexts which requests for the lock can be
  satisfied, grant lock to them and wake them up.

  @note Together with MDL_lock::add_ticket() this method implements
        fair scheduling among requests with the same priority.
        It tries to grant lock from the head of waiters list, while
        add_ticket() adds new requests to the back of this list.

*/

void MDL_lock::reschedule_waiters() {
  MDL_lock::Ticket_iterator it(m_waiting);
  MDL_ticket *ticket;

  /*
    Find the first (and hence the oldest) waiting request which
    can be satisfied (taking into account priority). Grant lock to it.
    Repeat the process for the remainder of waiters.
    Note we don't need to re-start iteration from the head of the
    list after satisfying the first suitable request as in our case
    all compatible types of requests have the same priority.

    TODO/FIXME: We should:
                - Either switch to scheduling without priorities
                  which will allow to stop iteration through the
                  list of waiters once we found the first ticket
                  which can't be  satisfied
                - Or implement some check using bitmaps which will
                  allow to stop iteration in cases when, e.g., we
                  grant SNRW lock and there are no pending S or
                  SH locks.
  */
  while ((ticket = it++)) {
    if (can_grant_lock(ticket->get_type(), ticket->get_ctx())) {
      if (!ticket->get_ctx()->m_wait.set_status(MDL_wait::GRANTED)) {
        /*
          Satisfy the found request by updating lock structures.
          It is OK to do so even after waking up the waiter since any
          session which tries to get any information about the state of
          this lock has to acquire MDL_lock::m_rwlock first and thus,
          when manages to do so, already sees an updated state of the
          MDL_lock object.

          It doesn't matter if we are dealing with "obtrusive" lock here,
          we are moving lock request from waiting to granted lists,
          so m_obtrusive_locks_granted_waiting_count should stay the same.
        */
        m_waiting.remove_ticket(ticket);
        m_granted.add_ticket(ticket);

        if (is_affected_by_max_write_lock_count()) {
          /*
            Increase the counter of successively granted high priority "hog" or
            "piglet" locks, if we have granted one and there are pending
            lower priority locks.
          */
          if (count_piglets_and_hogs(ticket->get_type())) {
            /*
              Switch of priority matrice might have unblocked some lower-prio
              locks which are still compatible with the lock type we just have
              granted (for example, when we grant SNW lock and there are pending
              requests of SR type). Restart iteration to wake them up, otherwise
              we might get deadlocks.
            */
            it.rewind();
            continue;
          }
        }
      }
      /*
        If we could not update the wait slot of the waiter,
        it can be due to fact that its connection/statement was
        killed or it has timed out (i.e. the slot is not empty).
        Since in all such cases the waiter assumes that the lock was
        not been granted, we should keep the request in the waiting
        queue and look for another request to reschedule.
      */
    }
  }

  if (is_affected_by_max_write_lock_count()) {
    /*
      Reset number of successively granted higher-prio "hog"/"piglet" locks
      if there are no pending lower-prio conflicting locks.
      This ensures:
      - That m_hog_lock_count/m_piglet_lock_count is correctly reset after
        a strong lock is released and weak locks are granted (or there are
        no other lock requests).
      - That the situation when SNW lock is granted along with some SR/SRO
        locks, but SW locks are still blocked is handled correctly.
      - That m_hog_lock_count/m_piglet_lock_count is zero in all cases
        when there are no pending weak locks (e.g. when weak locks are
        removed due to deadlock, being killed or timeout).

      Also switch priority matrice accordingly. Note that switch in
      this particular place doesn't require reschedule since:

      1) We never switch to a matrice which prefers lower priority locks
         more than "hog"/"piglet" locks here (this might have happened if
         MDL_lock::switch_incompatible_waiting_types_bitmap_if_needed()
         was used instead and max_write_lock_count was decreased
         concurrently).
      2) When we switch from matrice #1 (which prefers SRO over SW) to
         default matrice #0 only the priority of SW vs SRO requests changes.
         Since the switch happens only when there are no pending SRO
         locks, no reschedule is required.
      3) When we switch from matrice #2 (which prefers all non-"nog" over
         "hog" requests) to default matrice #0 only the priority of "hog" vs
         non-"hog" requests changes. But since this happens when there are
         no non-"hog" requests, no reschedule is required.
      4) When we switch from matrice #3 (which prefers SRO over SW and
         non-"hog"-minus-SW over "hog" locks) to default matrice #0 only
         the priority of non-"hog"-minus-SW vs "hog" and SRO vs SW changes
         (see invariant [INV3]). Since the switch happens only when there
         are no pending non-"hog"-minus-SW/SRO requests, no reschedule is
         required.

      Note that we might be switching priority matrice in a situation when we
      had pending SRO/non-"hog"/non-"hog"-minus-SW requests at the start of
      the call but they were granted during the loop. If some "piglet"/"hog"
      requests are compatible with those lower priority locks, they were
      granted as well. Those which were not compatible were not granted and
      should stay waiting until lower priority locks are released (in other
      words, the fact that a lock moved from pending to granted doesn't unblock
      additional requests, see invariant [INV2]).
    */
    if (m_current_waiting_incompatible_idx == 3) {
      /*
        We can't simply switch from matrice #3 to matrice #2 when there are no
        pending SRO locks, as that would allow a stream of concurrent SW and SRO
        requests to starve out "hog" locks when max_write_lock_count is set
        (there will be always pending SW or/and SRO locks in this case, so no
        switch back to matrice #0 will ever happen).

        So we switch from matrice #3 to #0 directly and ignore pending SW/SWLP
        locks. This is OK as situation when matrice #3 is active can only
        occur when there were max_write_lock_count SW locks granted recently
        (before switch from #0 -> #1 which preceded switch #3 or before switch
        #2 -> #3).
      */
      if ((m_waiting.bitmap() &
           ~(MDL_OBJECT_HOG_LOCK_TYPES | MDL_BIT(MDL_SHARED_WRITE) |
             MDL_BIT(MDL_SHARED_WRITE_LOW_PRIO))) == 0) {
        m_piglet_lock_count = 0;
        m_hog_lock_count = 0;
        m_current_waiting_incompatible_idx = 0;
      }
    } else {
      if ((m_waiting.bitmap() & ~MDL_OBJECT_HOG_LOCK_TYPES) == 0) {
        m_hog_lock_count = 0;
        m_current_waiting_incompatible_idx &= ~2;
      }
      if ((m_waiting.bitmap() & MDL_BIT(MDL_SHARED_READ_ONLY)) == 0) {
        m_piglet_lock_count = 0;
        m_current_waiting_incompatible_idx &= ~1;
      }
    }
  }
}

/**
  Strategy instances to be used with scoped metadata locks (i.e. locks
  from GLOBAL, COMMIT, TABLESPACE, BACKUP_LOCK and SCHEMA namespaces).
  The only locking modes which are supported at the moment are SHARED and
  INTENTION EXCLUSIVE and EXCLUSIVE.
*/

const MDL_lock::MDL_lock_strategy MDL_lock::m_scoped_lock_strategy = {
    /**
      Compatibility (or rather "incompatibility") matrices for scoped metadata
      lock. Arrays of bitmaps which elements specify which granted/waiting locks
      are incompatible with type of lock being requested.

      The first array specifies if particular type of request can be satisfied
      if there is granted scoped lock of certain type.

                 | Type of active   |
         Request |   scoped lock    |
          type   | IS(*)  IX   S  X |
        ---------+------------------+
        IS       |  +      +   +  + |
        IX       |  +      +   -  - |
        S        |  +      -   +  - |
        X        |  +      -   -  - |

      Here: "+" -- means that request can be satisfied
            "-" -- means that request can't be satisfied and should wait

      (*)  Since intention shared scoped locks are compatible with all other
           type of locks we don't even have any accounting for them.

      Note that relation between scoped locks and objects locks requested
      by statement is not straightforward and is therefore fully defined
      by SQL-layer.
      For example, in order to support global read lock implementation
      SQL-layer acquires IX lock in GLOBAL namespace for each statement
      that can modify metadata or data (i.e. for each statement that
      needs SW, SU, SNW, SNRW or X object locks). OTOH, to ensure that
      DROP DATABASE works correctly with concurrent DDL, IX metadata locks
      in SCHEMA namespace are acquired for DDL statements which can update
      metadata in the schema (i.e. which acquire SU, SNW, SNRW and X locks
      on schema objects) and aren't acquired for DML.
    */
    {MDL_BIT(MDL_EXCLUSIVE) | MDL_BIT(MDL_SHARED),
     MDL_BIT(MDL_EXCLUSIVE) | MDL_BIT(MDL_INTENTION_EXCLUSIVE), 0, 0, 0, 0, 0,
     0, 0, 0,
     MDL_BIT(MDL_EXCLUSIVE) | MDL_BIT(MDL_SHARED) |
         MDL_BIT(MDL_INTENTION_EXCLUSIVE)},
    /**
      Each array in the next group specifies if a particular type of request can
      be satisfied if there is already a waiting request for the scoped lock of
      a certain type. I.e. each array specifies a matrice with priorities for
      different lock types.

      Scoped locks only use the first array which represents the "default"
      priority matrix. The remaing 3 matrices are not relevant for them.

                 |    Pending      |
         Request |  scoped lock    |
          type   | IS(*)  IX  S  X |
        ---------+-----------------+
        IS       |  +      +  +  + |
        IX       |  +      +  -  - |
        S        |  +      +  +  - |
        X        |  +      +  +  + |

      Here the meaning of "+", "-" and (*) is the same as above.
    */
    {{MDL_BIT(MDL_EXCLUSIVE) | MDL_BIT(MDL_SHARED), MDL_BIT(MDL_EXCLUSIVE), 0,
      0, 0, 0, 0, 0, 0, 0, 0},
     {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
    /**
      Array of increments for "unobtrusive" types of lock requests for scoped
      locks.

      @sa MDL_lock::get_unobtrusive_lock_increment().

      For scoped locks:
      - "unobtrusive" types: IX
      - "obtrusive" types: X and S

      We encode number of IX locks acquired using "fast path" in bits 0 .. 59
      of MDL_lock::m_fast_path_state.
    */
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /*
      In scoped locks, only IX lock request would starve because of X/S.
      But that is practically a very rare case. So we don't apply the
      max_write_lock_count limit to them.
    */
    false,
#ifndef DBUG_OFF
    // In scoped locks, only IX, SHARED and X is allowed.
    {true, true, false, false, false, false, false, false, false, false, true},
#endif /* not defined DBUG_OFF */

    /*
      Scoped locks doesn't require notification of owners of conflicting
      locks for any type of requests. Hence 'm_needs_notification' is NULL.
    */
    nullptr,
    /*
      For the same reason, 'm_notify_conflicting_locks' is NULL for scoped
      locks.
    */
    nullptr,
    &MDL_lock::scoped_lock_kill_conflicting_locks,
    &MDL_lock::scoped_lock_fast_path_granted_bitmap,
    /* Scoped locks never require connection check. */
    nullptr};

/**
  Strategy instance for per-object locks.
  Supports all locked modes except INTENTION EXCLUSIVE locks.
*/

const MDL_lock::MDL_lock_strategy MDL_lock::m_object_lock_strategy = {
    /**
      Compatibility (or rather "incompatibility") matrices for per-object
      metadata lock. Arrays of bitmaps which elements specify which granted/
      waiting locks are incompatible with type of lock being requested.

      The first array specifies if particular type of request can be satisfied
      if there is granted lock of certain type.

         Request  |  Granted requests for lock            |
          type    | S  SH  SR  SW  SWLP  SU  SRO  SNW  SNRW  X  |
        ----------+---------------------------------------------+
        S         | +   +   +   +    +    +   +    +    +    -  |
        SH        | +   +   +   +    +    +   +    +    +    -  |
        SR        | +   +   +   +    +    +   +    +    -    -  |
        SW        | +   +   +   +    +    +   -    -    -    -  |
        SWLP      | +   +   +   +    +    +   -    -    -    -  |
        SU        | +   +   +   +    +    -   +    -    -    -  |
        SRO       | +   +   +   -    -    +   +    +    -    -  |
        SNW       | +   +   +   -    -    -   +    -    -    -  |
        SNRW      | +   +   -   -    -    -   -    -    -    -  |
        X         | -   -   -   -    -    -   -    -    -    -  |

      Here: "+" -- means that request can be satisfied
            "-" -- means that request can't be satisfied and should wait

      @note In cases then current context already has "stronger" type
            of lock on the object it will be automatically granted
            thanks to usage of the MDL_context::find_ticket() method.

      @note IX locks are excluded since they are not used for per-object
            metadata locks.
    */
    {0, MDL_BIT(MDL_EXCLUSIVE), MDL_BIT(MDL_EXCLUSIVE),
     MDL_BIT(MDL_EXCLUSIVE) | MDL_BIT(MDL_SHARED_NO_READ_WRITE),
     MDL_BIT(MDL_EXCLUSIVE) | MDL_BIT(MDL_SHARED_NO_READ_WRITE) |
         MDL_BIT(MDL_SHARED_NO_WRITE) | MDL_BIT(MDL_SHARED_READ_ONLY),
     MDL_BIT(MDL_EXCLUSIVE) | MDL_BIT(MDL_SHARED_NO_READ_WRITE) |
         MDL_BIT(MDL_SHARED_NO_WRITE) | MDL_BIT(MDL_SHARED_READ_ONLY),
     MDL_BIT(MDL_EXCLUSIVE) | MDL_BIT(MDL_SHARED_NO_READ_WRITE) |
         MDL_BIT(MDL_SHARED_NO_WRITE) | MDL_BIT(MDL_SHARED_UPGRADABLE),
     MDL_BIT(MDL_EXCLUSIVE) | MDL_BIT(MDL_SHARED_NO_READ_WRITE) |
         MDL_BIT(MDL_SHARED_WRITE_LOW_PRIO) | MDL_BIT(MDL_SHARED_WRITE),
     MDL_BIT(MDL_EXCLUSIVE) | MDL_BIT(MDL_SHARED_NO_READ_WRITE) |
         MDL_BIT(MDL_SHARED_NO_WRITE) | MDL_BIT(MDL_SHARED_UPGRADABLE) |
         MDL_BIT(MDL_SHARED_WRITE_LOW_PRIO) | MDL_BIT(MDL_SHARED_WRITE),
     MDL_BIT(MDL_EXCLUSIVE) | MDL_BIT(MDL_SHARED_NO_READ_WRITE) |
         MDL_BIT(MDL_SHARED_NO_WRITE) | MDL_BIT(MDL_SHARED_READ_ONLY) |
         MDL_BIT(MDL_SHARED_UPGRADABLE) | MDL_BIT(MDL_SHARED_WRITE_LOW_PRIO) |
         MDL_BIT(MDL_SHARED_WRITE) | MDL_BIT(MDL_SHARED_READ),
     MDL_BIT(MDL_EXCLUSIVE) | MDL_BIT(MDL_SHARED_NO_READ_WRITE) |
         MDL_BIT(MDL_SHARED_NO_WRITE) | MDL_BIT(MDL_SHARED_READ_ONLY) |
         MDL_BIT(MDL_SHARED_UPGRADABLE) | MDL_BIT(MDL_SHARED_WRITE_LOW_PRIO) |
         MDL_BIT(MDL_SHARED_WRITE) | MDL_BIT(MDL_SHARED_READ) |
         MDL_BIT(MDL_SHARED_HIGH_PRIO) | MDL_BIT(MDL_SHARED)},
    /**
      Each array in the next group specifies if a particular type of request can
      be satisfied if there is a waiting request for the same lock of a certain
      type. In other words each array specifies a priority matrix for different
      lock types.

      We use each of the arrays depending on whether the number of successively
      granted "piglet" and "hog" lock requests exceed the max_write_lock_count
      threshold. This is necessary to avoid high priority lock requests starving
      out requests with lower priority.

      The first array in the group is used in default situation when both
      MDL_lock::m_piglet_lock_count and MDL_lock::m_hog_lock_count don't exceed
      the threshold.

      A priority matrice specified by it looks like:

         Request  |         Pending requests for lock          |
          type    | S  SH  SR  SW  SWLP  SU  SRO  SNW  SNRW  X |
        ----------+--------------------------------------------+
        S         | +   +   +   +    +    +   +    +     +   - |
        SH        | +   +   +   +    +    +   +    +     +   + |
        SR        | +   +   +   +    +    +   +    +     -   - |
        SW        | +   +   +   +    +    +   +    -     -   - |
        SWLP      | +   +   +   +    +    +   -    -     -   - |
        SU        | +   +   +   +    +    +   +    +     +   - |
        SRO       | +   +   +   -    +    +   +    +     -   - |
        SNW       | +   +   +   +    +    +   +    +     +   - |
        SNRW      | +   +   +   +    +    +   +    +     +   - |
        X         | +   +   +   +    +    +   +    +     +   + |

      Invariant [INV2]: for all priority matrices, if A is the set of
      incompatible waiting requests for a given request and B is the set
      of incompatible granted requests for the same request, then A will
      always be a subset of B. This means that moving a lock from waiting
      to granted state doesn't unblock additional requests.
      MDL_lock::reschedule_waiters() code relies on this.
    */
    {{0, MDL_BIT(MDL_EXCLUSIVE), 0,
      MDL_BIT(MDL_EXCLUSIVE) | MDL_BIT(MDL_SHARED_NO_READ_WRITE),
      MDL_BIT(MDL_EXCLUSIVE) | MDL_BIT(MDL_SHARED_NO_READ_WRITE) |
          MDL_BIT(MDL_SHARED_NO_WRITE),
      MDL_BIT(MDL_EXCLUSIVE) | MDL_BIT(MDL_SHARED_NO_READ_WRITE) |
          MDL_BIT(MDL_SHARED_NO_WRITE) | MDL_BIT(MDL_SHARED_READ_ONLY),
      MDL_BIT(MDL_EXCLUSIVE),
      MDL_BIT(MDL_EXCLUSIVE) | MDL_BIT(MDL_SHARED_NO_READ_WRITE) |
          MDL_BIT(MDL_SHARED_WRITE),
      MDL_BIT(MDL_EXCLUSIVE), MDL_BIT(MDL_EXCLUSIVE), 0},
     /**
       The second array in the group is used when the number of successively
       granted "piglet" (SW) locks exceeds max_write_lock_count.

       It is the same matrix as in the first case but with the SW lock type
       having lower priority than the SRO lock type.
     */
     {0, MDL_BIT(MDL_EXCLUSIVE), 0,
      MDL_BIT(MDL_EXCLUSIVE) | MDL_BIT(MDL_SHARED_NO_READ_WRITE),
      MDL_BIT(MDL_EXCLUSIVE) | MDL_BIT(MDL_SHARED_NO_READ_WRITE) |
          MDL_BIT(MDL_SHARED_NO_WRITE) | MDL_BIT(MDL_SHARED_READ_ONLY),
      MDL_BIT(MDL_EXCLUSIVE) | MDL_BIT(MDL_SHARED_NO_READ_WRITE) |
          MDL_BIT(MDL_SHARED_NO_WRITE) | MDL_BIT(MDL_SHARED_READ_ONLY),
      MDL_BIT(MDL_EXCLUSIVE),
      MDL_BIT(MDL_EXCLUSIVE) | MDL_BIT(MDL_SHARED_NO_READ_WRITE),
      MDL_BIT(MDL_EXCLUSIVE), MDL_BIT(MDL_EXCLUSIVE), 0},
     /**
       The third array in the group is used when the number of successively
       granted "hog" (SNW, SNRW, X) locks exceeds max_write_lock_count.

       In this case S, SH, SR, SW, SNRW, SRO and SU locks types have
       priority over all "hog" types.
     */
     {0, 0, 0, 0, 0, MDL_BIT(MDL_SHARED_READ_ONLY), 0,
      MDL_BIT(MDL_SHARED_WRITE),
      MDL_BIT(MDL_EXCLUSIVE) | MDL_BIT(MDL_SHARED_UPGRADABLE) |
          MDL_BIT(MDL_SHARED_WRITE_LOW_PRIO) | MDL_BIT(MDL_SHARED_WRITE),
      MDL_BIT(MDL_EXCLUSIVE) | MDL_BIT(MDL_SHARED_READ_ONLY) |
          MDL_BIT(MDL_SHARED_UPGRADABLE) | MDL_BIT(MDL_SHARED_WRITE_LOW_PRIO) |
          MDL_BIT(MDL_SHARED_WRITE) | MDL_BIT(MDL_SHARED_READ),
      static_cast<bitmap_t>(~MDL_OBJECT_HOG_LOCK_TYPES)},
     /**
       The fourth array in the group is used when both the number of
       successively granted "piglet" (SW) and the number of successively granted
       "hog" (SNW, SNRW, X) locks exceed max_write_lock_count.

       This matrice prefers SRO locks over SW/SWLP locks. And non-"hog" locks
       other than SW/SWLP over "hog" locks.

       Note that the fact that "hog" locks have the same priority vs SW/SWLP
       locks as in the default matrice (#0) is important and is relied upon in
       MDL_lock::reschedule_waiters(). This is invariant [INV3].
     */
     {0, 0, 0, 0,
      MDL_BIT(MDL_EXCLUSIVE) | MDL_BIT(MDL_SHARED_NO_READ_WRITE) |
          MDL_BIT(MDL_SHARED_NO_WRITE) | MDL_BIT(MDL_SHARED_READ_ONLY),
      MDL_BIT(MDL_EXCLUSIVE) | MDL_BIT(MDL_SHARED_NO_READ_WRITE) |
          MDL_BIT(MDL_SHARED_NO_WRITE) | MDL_BIT(MDL_SHARED_READ_ONLY),
      0, 0, MDL_BIT(MDL_EXCLUSIVE) | MDL_BIT(MDL_SHARED_UPGRADABLE),
      MDL_BIT(MDL_EXCLUSIVE) | MDL_BIT(MDL_SHARED_READ_ONLY) |
          MDL_BIT(MDL_SHARED_UPGRADABLE) | MDL_BIT(MDL_SHARED_READ),
      static_cast<bitmap_t>(~(MDL_OBJECT_HOG_LOCK_TYPES |
                              MDL_BIT(MDL_SHARED_WRITE) |
                              MDL_BIT(MDL_SHARED_WRITE_LOW_PRIO)))}},
    /**
      Array of increments for "unobtrusive" types of lock requests for
      per-object locks.

      @sa MDL_lock::get_unobtrusive_lock_increment().

      For per-object locks:
      - "unobtrusive" types: S, SH, SR and SW
      - "obtrusive" types: SU, SRO, SNW, SNRW, X

      Number of locks acquired using "fast path" are encoded in the following
      bits of MDL_lock::m_fast_path_state:

      - bits 0 .. 19  - S and SH (we don't differentiate them once acquired)
      - bits 20 .. 39 - SR
      - bits 40 .. 59 - SW and SWLP (we don't differentiate them once acquired)

      Overflow is not an issue as we are unlikely to support more than 2^20 - 1
      concurrent connections in foreseeable future.

      This encoding defines the below contents of increment array.
    */
    {0, 1, 1, 1ULL << 20, 1ULL << 40, 1ULL << 40, 0, 0, 0, 0, 0},
    /*
      To prevent starvation, "hog" and "piglet" lock types are only granted
      max_write_lock_count times in a row while conflicting lock types are
      waiting.
    */
    true,
#ifndef DBUG_OFF
    // For object locks all types, except IX, are permitted
    {false, true, true, true, true, true, true, true, true, true, true},
#endif /* not defined DBUG_OFF */

    &MDL_lock::object_lock_needs_notification,
    &MDL_lock::object_lock_notify_conflicting_locks,
    &MDL_lock::object_lock_kill_conflicting_locks,
    &MDL_lock::object_lock_fast_path_granted_bitmap,
    &MDL_lock::object_lock_needs_connection_check};

/**
  Check if request for the metadata lock can be satisfied given its
  current state.

  @param  type_arg             The requested lock type.
  @param  requestor_ctx        The MDL context of the requestor.

  @retval true   Lock request can be satisfied
  @retval false  There is some conflicting lock.

  @note In cases then current context already has "stronger" type
        of lock on the object it will be automatically granted
        thanks to usage of the MDL_context::find_ticket() method.
*/

bool MDL_lock::can_grant_lock(enum_mdl_type type_arg,
                              const MDL_context *requestor_ctx) const {
  bool can_grant = false;
  bitmap_t waiting_incompat_map = incompatible_waiting_types_bitmap()[type_arg];
  bitmap_t granted_incompat_map = incompatible_granted_types_bitmap()[type_arg];

  /*
    New lock request can be satisfied iff:
    - There are no incompatible types of satisfied requests
    in other contexts
    - There are no waiting requests which have higher priority
    than this request.
  */
  if (!(m_waiting.bitmap() & waiting_incompat_map)) {
    if (!(fast_path_granted_bitmap() & granted_incompat_map)) {
      if (!(m_granted.bitmap() & granted_incompat_map))
        can_grant = true;
      else {
        Ticket_iterator it(m_granted);
        MDL_ticket *ticket;

        /*
          There is an incompatible lock. Check that it belongs to some
          other context.

          If we are trying to acquire "unobtrusive" type of lock then the
          confliciting lock must be from "obtrusive" set, therefore it should
          have been acquired using "slow path" and should be present in
          m_granted list.

          If we are trying to acquire "obtrusive" type of lock then it can be
          either another "obtrusive" lock or "unobtrusive" type of lock
          acquired on "slow path" (can't be "unobtrusive" lock on fast path
          because of surrounding if-statement). In either case it should be
          present in m_granted list.
        */
        while ((ticket = it++)) {
          if (ticket->get_ctx() != requestor_ctx &&
              ticket->is_incompatible_when_granted(type_arg))
            break;
        }
        if (ticket == nullptr) /* Incompatible locks are our own. */
          can_grant = true;
      }
    } else {
      /*
        Our lock request conflicts with one of granted "fast path" locks:

        This means that we are trying to acquire "obtrusive" lock and:
        a) Either we are called from MDL_context::try_acquire_lock_impl()
           and then all "fast path" locks belonging to this context were
           materialized (as we do for "obtrusive" locks).
        b) Or we are called from MDL_lock::reschedule_waiters() then
           this context is waiting for this request and all its "fast
           path" locks were materialized before the wait.

        The above means that conflicting granted "fast path" lock cannot
        belong to us and our request cannot be satisfied.
      */
    }
  }
  return can_grant;
}

/**
  Return the first MDL_context which owns the lock.

  @return Pointer to the first MDL_context which has acquired the lock
          NULL if there are no such contexts.

  @note This method works properly only for locks acquired using
        "slow" path. It won't return context if it has used "fast"
        path to acquire the lock.
*/

inline MDL_context *MDL_lock::get_lock_owner() const {
  Ticket_iterator it(m_granted);
  MDL_ticket *ticket;

  if ((ticket = it++)) return ticket->get_ctx();
  return nullptr;
}

/** Remove a ticket from waiting or pending queue and wakeup up waiters. */

void MDL_lock::remove_ticket(MDL_context *ctx, LF_PINS *pins,
                             Ticket_list MDL_lock::*list, MDL_ticket *ticket) {
  bool is_obtrusive = is_obtrusive_lock(ticket->get_type());
  bool is_singleton = mdl_locks.is_lock_object_singleton(&key);

  mysql_prlock_wrlock(&m_rwlock);
  (this->*list).remove_ticket(ticket);

  /*
    If we are removing "obtrusive" type of request either from granted or
    waiting lists we need to decrement "obtrusive" requests counter.
    Once last ticket for "obtrusive" lock is removed we should clear
    HAS_OBTRUSIVE flag in m_fast_path_state as well.
  */
  bool last_obtrusive =
      is_obtrusive && ((--m_obtrusive_locks_granted_waiting_count) == 0);
  /*
    If both m_granted and m_waiting lists become empty as result we also
    need to clear HAS_SLOW_PATH flag in m_fast_path_state.
  */
  bool last_slow_path = m_granted.is_empty() && m_waiting.is_empty();
  bool last_use = false;

  if (last_slow_path || last_obtrusive) {
    fast_path_state_t old_state = m_fast_path_state;
    fast_path_state_t new_state;
    do {
      new_state = old_state;
      if (last_slow_path) new_state &= ~MDL_lock::HAS_SLOW_PATH;
      if (last_obtrusive) new_state &= ~MDL_lock::HAS_OBTRUSIVE;
    } while (!fast_path_state_cas(&old_state, new_state));

    /*
      We don't have any "fast" or "slow" path locks. MDL_lock object becomes
      unused so unused objects counter needs to be incremented.
    */
    if (new_state == 0) last_use = true;
  }

  if (last_slow_path) {
    /*
      We might end-up with the last waiting ticket being removed and non-0
      m_hog_lock_count/m_piglet_lock_count in the following situation:

      1) There is a granted "hog"/"piglet" lock blocking a lower priority lock
         request.
      2) The lower priority lock request is timed out or killed. It is not yet
         removed from waiters list and bitmap.
      3) The "Hog"/"piglet" lock is released. Its reschedule_waiters() call
         will still see the pending lower priority lock so it won't reset
         the m_hog_lock_count/m_piglet_lock_count counters.
      4) MDL_lock::remove_ticket() is called for the timed out/killed
         lower priority ticket. Which turns out to be the last ticket
         for this lock.

      Hence we need to reset these counters here.
    */
    m_hog_lock_count = 0;
    m_piglet_lock_count = 0;
    m_current_waiting_incompatible_idx = 0;
  } else {
    /*
      There can be some contexts waiting to acquire a lock
      which now might be able to do it. Grant the lock to
      them and wake them up!

      We always try to reschedule locks, since there is no easy way
      (i.e. by looking at the bitmaps) to find out whether it is
      required or not.
      In a general case, even when the queue's bitmap is not changed
      after removal of the ticket, there is a chance that some request
      can be satisfied (due to the fact that a granted request
      reflected in the bitmap might belong to the same context as a
      pending request).
    */
    reschedule_waiters();
  }
  mysql_prlock_unlock(&m_rwlock);

  /* Don't count singleton MDL_lock objects as unused. */
  if (last_use && !is_singleton) mdl_locks.lock_object_unused(ctx, pins);
}

/**
  Check if we have any pending locks which conflict with existing
  shared lock.

  @pre The ticket must match an acquired lock.

  @return true if there is a conflicting lock request, false otherwise.
*/

bool MDL_lock::has_pending_conflicting_lock(enum_mdl_type type) {
  bool result;

  mysql_mutex_assert_not_owner(&LOCK_open);

  mysql_prlock_rdlock(&m_rwlock);
  result = (m_waiting.bitmap() & incompatible_granted_types_bitmap()[type]);
  mysql_prlock_unlock(&m_rwlock);
  return result;
}

MDL_wait_for_graph_visitor::~MDL_wait_for_graph_visitor() {}

MDL_wait_for_subgraph::~MDL_wait_for_subgraph() {}

/**
  Check if ticket represents metadata lock of "stronger" or equal type
  than specified one. I.e. if metadata lock represented by ticket won't
  allow any of locks which are not allowed by specified type of lock.

  @return true  if ticket has stronger or equal type
          false otherwise.
*/

bool MDL_ticket::has_stronger_or_equal_type(enum_mdl_type type) const {
  const MDL_lock::bitmap_t *granted_incompat_map =
      m_lock->incompatible_granted_types_bitmap();

  return !(granted_incompat_map[type] & ~(granted_incompat_map[m_type]));
}

bool MDL_ticket::is_incompatible_when_granted(enum_mdl_type type) const {
  return (MDL_BIT(m_type) & m_lock->incompatible_granted_types_bitmap()[type]);
}

bool MDL_ticket::is_incompatible_when_waiting(enum_mdl_type type) const {
  return (MDL_BIT(m_type) & m_lock->incompatible_waiting_types_bitmap()[type]);
}

#ifndef DBUG_OFF
bool equivalent(const MDL_ticket *a, const MDL_ticket *b,
                enum_mdl_duration target_duration) {
  if (a == b) {
    // Same pointer (incl. nullptr) are equivalent
    return true;
  }

  if (a->get_lock() != b->get_lock()) {
    // If they refer to different locks, they are never equivalent
    return false;
  }
  if (a->get_duration() == target_duration &&
      b->get_duration() == target_duration) {
    // Different objects, but which has the same lock, AND both refer to the
    // target duration are equivalent
    return true;
  }
  if (a->get_duration() != target_duration &&
      b->get_duration() != target_duration) {
    // Different objects with same lock are still equivalent, if neither has
    // the target duration
    return true;
  }
  return false;
}
#endif /* not defined DBUG_OFF */

/**
  Check whether the context already holds a compatible lock ticket
  on an object.
  Start searching from list of locks for the same duration as lock
  being requested. If not look at lists for other durations.

  @param mdl_request  Lock request object for lock to be acquired
  @param[out] result_duration  Duration of lock which was found.

  @note Tickets which correspond to lock types "stronger" than one
        being requested are also considered compatible.

  @return A pointer to the lock ticket for the object or NULL otherwise.
*/

MDL_ticket *MDL_context::find_ticket(MDL_request *mdl_request,
                                     enum_mdl_duration *result_duration) {
  auto h = m_ticket_store.find(*mdl_request);
  *result_duration = h.m_dur;
  return h.m_ticket;
}

/**
  Try to acquire one lock.

  Unlike exclusive locks, shared locks are acquired one by
  one. This is interface is chosen to simplify introduction of
  the new locking API to the system. MDL_context::try_acquire_lock()
  is currently used from open_table(), and there we have only one
  table to work with.

  This function may also be used to try to acquire an exclusive
  lock on a destination table, by ALTER TABLE ... RENAME.

  Returns immediately without any side effect if encounters a lock
  conflict. Otherwise takes the lock.

  @param [in,out] mdl_request Lock request object for lock to be acquired

  @retval  false   Success. The lock may have not been acquired.
                   Check the ticket, if it's NULL, a conflicting lock
                   exists.
  @retval  true    Out of resources, an error has been reported.
*/

bool MDL_context::try_acquire_lock(MDL_request *mdl_request) {
  MDL_ticket *ticket = nullptr;

  if (try_acquire_lock_impl(mdl_request, &ticket)) return true;

  if (!mdl_request->ticket) {
    /*
      Our attempt to acquire lock without waiting has failed.
      Let us release resources which were acquired in the process.

      We don't need to count MDL_lock object as unused and possibly
      delete it here because:
      - Either there was a conflicting ticket in MDL_lock::m_granted/
        m_waiting lists during our call to MDL_lock::can_grant_lock().
        This ticket can't go away while MDL_lock::m_rwlock is held.
      - Or we have tried to acquire an "obtrusive" lock and there was
        a conflicting "fast path" lock in MDL_lock::m_fast_path_state
        counter during our call to MDL_lock::can_grant_lock().
        In this case HAS_OBTRUSIVE lock flag should have been set before
        call to MDL_lock::can_grant_lock() so release of this "fast path"
        lock will have to take slow path (see release_lock() and invariant
        [INV1]). This means that conflicting "fast path" lock can't go
        away until MDL_lock::m_rwlock is released or HAS_OBSTRUSIVE flag
        is cleared. In the latter case counting MDL_lock object as unused
        is responsibility of thread which is decrementing "fast path" lock
        counter. MDL_lock object can't be deleted under out feet since
        thread doing deletion needs to acquire MDL_lock::m_rwlock first.
    */
    MDL_lock *lock = ticket->m_lock;

    bool last_obtrusive =
        lock->is_obtrusive_lock(mdl_request->type) &&
        ((--lock->m_obtrusive_locks_granted_waiting_count) == 0);
    bool last_slow_path =
        lock->m_granted.is_empty() && lock->m_waiting.is_empty();

    if (last_slow_path || last_obtrusive) {
      MDL_lock::fast_path_state_t old_state = lock->m_fast_path_state;
      MDL_lock::fast_path_state_t new_state;
      do {
        new_state = old_state;
        if (last_slow_path) new_state &= ~MDL_lock::HAS_SLOW_PATH;
        if (last_obtrusive) new_state &= ~MDL_lock::HAS_OBTRUSIVE;
      } while (!lock->fast_path_state_cas(&old_state, new_state));
    }

    mysql_prlock_unlock(&lock->m_rwlock);

    /*
      If SEs were notified about impending lock acquisition, the failure
      to acquire it requires the same notification as lock release.
    */
    if (ticket->m_hton_notified) {
      mysql_mdl_set_status(ticket->m_psi, MDL_ticket::POST_RELEASE_NOTIFY);
      m_owner->notify_hton_post_release_exclusive(&mdl_request->key);
    }

    MDL_ticket::destroy(ticket);
  }

  return false;
}

/**
  "Materialize" requests for locks which were satisfied using
  "fast path" by properly including them into corresponding
  MDL_lock::m_granted bitmaps/lists and removing it from
  packed counter in MDL_lock::m_fast_path_state.

  @note In future we might optimize this method if necessary,
        for example, by keeping pointer to first "fast path"
        ticket.
*/

void MDL_context::materialize_fast_path_locks() {
  int i;

  for (i = 0; i < MDL_DURATION_END; i++) {
    MDL_ticket_store::List_iterator it = m_ticket_store.list_iterator(i);

    MDL_ticket *matf = m_ticket_store.materialized_front(i);
    for (MDL_ticket *ticket = it++; ticket != matf; ticket = it++) {
      if (ticket->m_is_fast_path) {
        MDL_lock *lock = ticket->m_lock;
        MDL_lock::fast_path_state_t unobtrusive_lock_increment =
            lock->get_unobtrusive_lock_increment(ticket->get_type());
        ticket->m_is_fast_path = false;
        mysql_prlock_wrlock(&lock->m_rwlock);
        lock->m_granted.add_ticket(ticket);
        /*
          Atomically decrement counter in MDL_lock::m_fast_path_state.
          This needs to happen under protection of MDL_lock::m_rwlock to make
          it atomic with addition of ticket to MDL_lock::m_granted list and
          to enforce invariant [INV1].
        */
        MDL_lock::fast_path_state_t old_state = lock->m_fast_path_state;
        while (!lock->fast_path_state_cas(
            &old_state, ((old_state - unobtrusive_lock_increment) |
                         MDL_lock::HAS_SLOW_PATH))) {
        }
        mysql_prlock_unlock(&lock->m_rwlock);
      }
    }
  }
  m_ticket_store.set_materialized();
}

/**
  Auxiliary method for acquiring lock without waiting.

  @param [in,out] mdl_request Lock request object for lock to be acquired
  @param [out] out_ticket     Ticket for the request in case when lock
                              has not been acquired.

  @retval  false   Success. The lock may have not been acquired.
                   Check MDL_request::ticket, if it's NULL, a conflicting
                   lock exists. In this case "out_ticket" out parameter
                   points to ticket which was constructed for the request.
                   MDL_ticket::m_lock points to the corresponding MDL_lock
                   object and MDL_lock::m_rwlock write-locked.
  @retval  true    Out of resources, an error has been reported.
*/

bool MDL_context::try_acquire_lock_impl(MDL_request *mdl_request,
                                        MDL_ticket **out_ticket) {
  MDL_lock *lock;
  MDL_key *key = &mdl_request->key;
  MDL_ticket *ticket;
  enum_mdl_duration found_duration;
  MDL_lock::fast_path_state_t unobtrusive_lock_increment;
  bool force_slow;
  bool pinned;

  DBUG_ASSERT(mdl_request->ticket == nullptr);

  /* Don't take chances in production. */
  mdl_request->ticket = nullptr;
  mysql_mutex_assert_not_owner(&LOCK_open);

  /*
    Check whether the context already holds a shared lock on the object,
    and if so, grant the request.
  */
  if ((ticket = find_ticket(mdl_request, &found_duration))) {
    DBUG_ASSERT(ticket->m_lock);
    DBUG_ASSERT(ticket->has_stronger_or_equal_type(mdl_request->type));
    /*
      If the request is for a transactional lock, and we found
      a transactional lock, just reuse the found ticket.

      It's possible that we found a transactional lock,
      but the request is for a HANDLER lock. In that case HANDLER
      code will clone the ticket (see below why it's needed).

      If the request is for a transactional lock, and we found
      a HANDLER lock, create a copy, to make sure that when user
      does HANDLER CLOSE, the transactional lock is not released.

      If the request is for a handler lock, and we found a
      HANDLER lock, also do the clone. HANDLER CLOSE for one alias
      should not release the lock on the table HANDLER opened through
      a different alias.
    */
    mdl_request->ticket = ticket;
    if ((found_duration != mdl_request->duration ||
         mdl_request->duration == MDL_EXPLICIT) &&
        clone_ticket(mdl_request)) {
      /* Clone failed. */
      mdl_request->ticket = nullptr;
      return true;
    }
    return false;
  }

  /*
    Prepare context for lookup in MDL_map container by allocating pins
    if necessary. This also ensures that this MDL_context has pins allocated
    and ready for future attempts elements from MDL_map container (which
    might happen during lock release).
  */
  if (fix_pins()) return true;

  if (!(ticket = MDL_ticket::create(this, mdl_request->type
#ifndef DBUG_OFF
                                    ,
                                    mdl_request->duration
#endif
                                    )))
    return true;

  /*
    Get increment for "fast path" or indication that this is
    request for "obtrusive" type of lock outside of critical section.
  */
  unobtrusive_lock_increment =
      MDL_lock::get_unobtrusive_lock_increment(mdl_request);

  /*
    If this "obtrusive" type we have to take "slow path".
    If this context has open HANDLERs we have to take "slow path"
    as well for MDL_object_lock::notify_conflicting_locks() to work
    properly.
  */
  force_slow = !unobtrusive_lock_increment || m_needs_thr_lock_abort;

  /*
    If "obtrusive" lock is requested we need to "materialize" all fast
    path tickets, so MDL_lock::can_grant_lock() can safely assume
    that all granted "fast path" locks belong to different context.
  */
  if (!unobtrusive_lock_increment) materialize_fast_path_locks();

  DBUG_ASSERT(ticket->m_psi == nullptr);
  ticket->m_psi = mysql_mdl_create(
      ticket, key, mdl_request->type, mdl_request->duration,
      MDL_ticket::PENDING, mdl_request->m_src_file, mdl_request->m_src_line);

  /*
    We need to notify/get permission from storage engines before acquiring
    X lock if it is requested in one of namespaces interesting for SEs.
  */
  if (mdl_request->type == MDL_EXCLUSIVE &&
      MDL_lock::needs_hton_notification(key->mdl_namespace())) {
    mysql_mdl_set_status(ticket->m_psi, MDL_ticket::PRE_ACQUIRE_NOTIFY);

    bool victimized;
    if (m_owner->notify_hton_pre_acquire_exclusive(key, &victimized)) {
      MDL_ticket::destroy(ticket);
      my_error(victimized ? ER_LOCK_DEADLOCK : ER_LOCK_REFUSED_BY_ENGINE,
               MYF(0));
      return true;
    }
    ticket->m_hton_notified = true;

    mysql_mdl_set_status(ticket->m_psi, MDL_ticket::PENDING);
  }

retry:
  /*
    The below call pins pointer to returned MDL_lock object (unless
    it is the singleton object for GLOBAL, COMMIT or ACL_CACHE namespaces).
  */
  if (!(lock = mdl_locks.find_or_insert(m_pins, key, &pinned))) {
    /*
      If SEs were notified about impending lock acquisition, the failure
      to acquire it requires the same notification as lock release.
    */
    if (ticket->m_hton_notified) {
      mysql_mdl_set_status(ticket->m_psi, MDL_ticket::POST_RELEASE_NOTIFY);
      m_owner->notify_hton_post_release_exclusive(key);
    }
    MDL_ticket::destroy(ticket);
    return true;
  }

  /*
    Code counting unused MDL_lock objects below assumes that object is not
    pinned iff it is a singleton.
  */
  DBUG_ASSERT(mdl_locks.is_lock_object_singleton(key) == !pinned);

  if (!force_slow) {
    /*
      "Fast path".

      Hurray! We are acquring "unobtrusive" type of lock and not forced
      to take "slow path" because of open HANDLERs.

      Let us do a few checks first to figure out if we really can acquire
      lock using "fast path".

      Since the MDL_lock object is pinned at this point (or it is the
      singleton) we can access its members without risk of it getting deleted
      under out feet.

      Ordinary read of MDL_lock::m_fast_path_state which we do here is OK as
      correctness of value returned by it will be anyway validated by atomic
      compare-and-swap which happens later.
      In theory, this algorithm will work correctly (but not very efficiently)
      if the read will return random values.
      In practice, it won't return values which are too out-of-date as the
      above call to MDL_map::find_or_insert() contains memory barrier.
    */
    MDL_lock::fast_path_state_t old_state = lock->m_fast_path_state;
    bool first_use;

    do {
      /*
        Check if hash look-up returned object marked as destroyed or
        it was marked as such while it was pinned by us. If yes we
        need to unpin it and retry look-up.
      */
      if (old_state & MDL_lock::IS_DESTROYED) {
        if (pinned) lf_hash_search_unpin(m_pins);
        DEBUG_SYNC(get_thd(), "mdl_acquire_lock_is_destroyed_fast_path");
        goto retry;
      }

      /*
        Check that there are no granted/pending "obtrusive" locks and nobody
        even is about to try to check if such lock can be acquired.

        In these cases we need to take "slow path".
      */
      if (old_state & MDL_lock::HAS_OBTRUSIVE) goto slow_path;

      /*
        If m_fast_path_state doesn't have HAS_SLOW_PATH set and all "fast"
        path counters are 0 then we are about to use an unused MDL_lock
        object. We need to decrement unused objects counter eventually.
      */
      first_use = (old_state == 0);

      /*
        Now we simply need to increment m_fast_path_state with a value which
        corresponds to type of our request (i.e. increment part this member
        which contains counter which corresponds to this type).

        This needs to be done as atomical operation with the above checks,
        which is achieved by using atomic compare-and-swap.

        @sa MDL_object_lock::m_unobtrusive_lock_increment for explanation
        why overflow is not an issue here.
      */
    } while (!lock->fast_path_state_cas(
        &old_state, old_state + unobtrusive_lock_increment));

    /*
      Lock has been acquired. Since this can only be an "unobtrusive" lock and
      there were no active/pending requests for "obtrusive" locks, we don't need
      to care about "hog" and "piglet" locks and the max_write_lock_count
      threshold.
    */

    if (pinned) lf_hash_search_unpin(m_pins);

    /*
      Don't count singleton MDL_lock objects as used, use "pinned == false"
      as an indication of such objects.
    */
    if (first_use && pinned) mdl_locks.lock_object_used();

    /*
      Since this MDL_ticket is not visible to any threads other than
      the current one, we can set MDL_ticket::m_lock member without
      protect of MDL_lock::m_rwlock. MDL_lock won't be deleted
      underneath our feet as MDL_lock::m_fast_path_state serves as
      reference counter in this case.
    */
    ticket->m_lock = lock;
    ticket->m_is_fast_path = true;
    m_ticket_store.push_front(mdl_request->duration, ticket);
    mdl_request->ticket = ticket;

    mysql_mdl_set_status(ticket->m_psi, MDL_ticket::GRANTED);
    materialize_fast_path_locks();
    return false;
  }

slow_path:

  /*
   "Slow path".

    Do full-blown check and list manipulation if necessary.
  */
  mysql_prlock_wrlock(&lock->m_rwlock);

  /*
    First of all, let us check if hash look-up returned MDL_lock object which
    is marked as destroyed or was marked as such while it was pinned by us.
    If we have got such object we need to retry look-up.

    We can use ordinary non-atomic read in this case as this flag is set under
    protection of MDL_lock::m_rwlock (we can get inconsistent data for other
    parts of m_fast_path_state due to concurrent atomic updates, but we don't
    care about them yet).
  */
  MDL_lock::fast_path_state_t state = lock->m_fast_path_state;

  if (state & MDL_lock::IS_DESTROYED) {
    mysql_prlock_unlock(&lock->m_rwlock);
    /*
      We can't unpin object earlier as lf_hash_delete() might have been
      called for it already and so LF_ALLOCATOR is free to deallocate it
      once unpinned.
    */
    if (pinned) lf_hash_search_unpin(m_pins);
    DEBUG_SYNC(get_thd(), "mdl_acquire_lock_is_destroyed_slow_path");
    goto retry;
  }

  /*
    Object was not marked as destroyed. Since it can't be deleted from hash
    and deallocated until this happens we can unpin it and work with it safely
    while MDL_lock::m_rwlock is held.
  */
  if (pinned) lf_hash_search_unpin(m_pins);

  /*
    When we try to acquire the first obtrusive lock for MDL_lock object we
    need to atomically set the HAS_OBTRUSIVE flag in m_fast_path_state before
    we call the MDL_lock::can_grant_lock() method.
    This is necessary to prevent concurrent fast path acquisitions from
    invalidating the results of this method.
  */
  bool first_obtrusive_lock =
      (unobtrusive_lock_increment == 0) &&
      ((lock->m_obtrusive_locks_granted_waiting_count++) == 0);
  bool first_use = false;

  /*
    When we try to acquire the first "slow" path lock for MDL_lock object
    we also need to atomically set HAS_SLOW_PATH flag. It is OK to read
    HAS_SLOW_PATH non-atomically here since it can be only set under
    protection of MDL_lock::m_rwlock lock.
  */
  if (!(state & MDL_lock::HAS_SLOW_PATH) || first_obtrusive_lock) {
    do {
      /*
        If HAS_SLOW_PATH flag is not set and all "fast" path counters
        are zero we are about to use previously unused MDL_lock object.
        MDL_map::m_unused_lock_objects counter needs to be decremented
        eventually.
      */
      first_use = (state == 0);
    } while (!lock->fast_path_state_cas(
        &state, state | MDL_lock::HAS_SLOW_PATH |
                    (first_obtrusive_lock ? MDL_lock::HAS_OBTRUSIVE : 0)));
  }

  /*
    Don't count singleton MDL_lock objects as used, use "pinned == false"
    as an indication of such objects.

    If the fact that we do this atomic decrement under MDL_lock::m_rwlock
    ever becomes bottleneck (which is unlikely) it can be safely moved
    outside of critical section.
  */
  if (first_use && pinned) mdl_locks.lock_object_used();

  ticket->m_lock = lock;

  if (lock->can_grant_lock(mdl_request->type, this)) {
    lock->m_granted.add_ticket(ticket);

    if (lock->is_affected_by_max_write_lock_count()) {
      /*
        If we have acquired an higher-prio "piglet" or "hog" lock while there
        are pending lower priority locks, we need to increment the appropriate
        counter.

        If one or both of these counters exceed the max_write_lock_count
        threshold, change priority matrice for this MDL_lock object.
        Reschedule waiters to avoid problems when the change of matrice
        happens concurrently to other thread acquiring a lower priority
        lock between its call to MDL_lock::can_grant_lock() and
        MDL_context::find_deadlock().
      */
      if (lock->count_piglets_and_hogs(mdl_request->type))
        lock->reschedule_waiters();
    }

    mysql_prlock_unlock(&lock->m_rwlock);

    m_ticket_store.push_front(mdl_request->duration, ticket);
    mdl_request->ticket = ticket;

    mysql_mdl_set_status(ticket->m_psi, MDL_ticket::GRANTED);
  } else
    *out_ticket = ticket;

  return false;
}

/**
  Create a copy of a granted ticket.
  This is used to make sure that HANDLER ticket
  is never shared with a ticket that belongs to
  a transaction, so that when we HANDLER CLOSE,
  we don't release a transactional ticket, and
  vice versa -- when we COMMIT, we don't mistakenly
  release a ticket for an open HANDLER.

  @retval true   Out of memory.
  @retval false  Success.
*/

bool MDL_context::clone_ticket(MDL_request *mdl_request) {
  MDL_ticket *ticket;

  mysql_mutex_assert_not_owner(&LOCK_open);

  /*
    Since in theory we can clone ticket belonging to a different context
    we need to prepare target context for possible attempts to release
    lock and thus possible removal of MDL_lock from MDL_map container.
    So we allocate pins to be able to work with this container if they
    are not allocated already.
  */
  if (fix_pins()) return true;

  /*
    By submitting mdl_request->type to MDL_ticket::create()
    we effectively downgrade the cloned lock to the level of
    the request.
  */
  if (!(ticket = MDL_ticket::create(this, mdl_request->type
#ifndef DBUG_OFF
                                    ,
                                    mdl_request->duration
#endif
                                    )))
    return true;

  DBUG_ASSERT(ticket->m_psi == nullptr);
  ticket->m_psi = mysql_mdl_create(
      ticket, &mdl_request->key, mdl_request->type, mdl_request->duration,
      MDL_ticket::PENDING, mdl_request->m_src_file, mdl_request->m_src_line);

  /* clone() is not supposed to be used to get a stronger lock. */
  DBUG_ASSERT(mdl_request->ticket->has_stronger_or_equal_type(ticket->m_type));

  /*
    If we are to clone exclusive lock in namespace requiring notification
    of storage engines we need to notify/get permission from SEs similarly
    to situation when lock acquired.
  */
  if (mdl_request->type == MDL_EXCLUSIVE &&
      MDL_lock::needs_hton_notification(mdl_request->key.mdl_namespace())) {
    DBUG_ASSERT(mdl_request->ticket->m_hton_notified);

    mysql_mdl_set_status(ticket->m_psi, MDL_ticket::PRE_ACQUIRE_NOTIFY);

    bool victimized;
    if (m_owner->notify_hton_pre_acquire_exclusive(&mdl_request->key,
                                                   &victimized)) {
      MDL_ticket::destroy(ticket);
      my_error(victimized ? ER_LOCK_DEADLOCK : ER_LOCK_REFUSED_BY_ENGINE,
               MYF(0));
      return true;
    }
    ticket->m_hton_notified = true;

    mysql_mdl_set_status(ticket->m_psi, MDL_ticket::PENDING);
  }

  ticket->m_lock = mdl_request->ticket->m_lock;

  if (mdl_request->ticket->m_is_fast_path) {
    /*
      We are cloning ticket which was acquired on "fast path".
      Let us use "fast path" to create clone as well.
    */
    MDL_lock::fast_path_state_t unobtrusive_lock_increment =
        ticket->m_lock->get_unobtrusive_lock_increment(ticket->get_type());

    /*
      "Obtrusive" type of lock can't be cloned from weaker, "unobtrusive"
      type of lock.
    */
    DBUG_ASSERT(unobtrusive_lock_increment != 0);

    /*
      Increment of counter in MDL_lock::m_fast_path_state needs to happen here
      atomically and under protection of MDL_lock::m_rwlock in order to enforce
      invariant [INV1].
    */
    mysql_prlock_wrlock(&ticket->m_lock->m_rwlock);
    ticket->m_lock->fast_path_state_add(unobtrusive_lock_increment);
    mysql_prlock_unlock(&ticket->m_lock->m_rwlock);
    ticket->m_is_fast_path = true;
  } else {
    /*
      We are cloning ticket which was acquired on "slow path".
      We will use "slow path" for new ticket as well. We also
      need to take into account if new ticket corresponds to
      "obtrusive" lock.
    */
    bool is_obtrusive = ticket->m_lock->is_obtrusive_lock(ticket->m_type);
    mysql_prlock_wrlock(&ticket->m_lock->m_rwlock);
    ticket->m_lock->m_granted.add_ticket(ticket);
    if (is_obtrusive) {
      /*
        We don't need to set HAS_OBTRUSIVE flag in MDL_lock::m_fast_path_state
        here as it is already set since the ticket being cloned already
        represents "obtrusive" lock for this MDL_lock object.
      */
      DBUG_ASSERT(ticket->m_lock->m_obtrusive_locks_granted_waiting_count != 0);
      ++ticket->m_lock->m_obtrusive_locks_granted_waiting_count;
    }
    mysql_prlock_unlock(&ticket->m_lock->m_rwlock);
  }

  mdl_request->ticket = ticket;
  m_ticket_store.push_front(mdl_request->duration, ticket);
  mysql_mdl_set_status(ticket->m_psi, MDL_ticket::GRANTED);

  return false;
}

bool MDL_lock::scoped_lock_kill_conflicting_locks(
    MDL_context *ctx, MDL_lock *lock, enum_mdl_type kill_lower_than) {
  // do not kill connection for scoped lock conflicts for hi-pri ddl
  // except for alter/drop database.
  // SCHEMA namespace locks are treated as object locks for the purposes
  // of killing conflicting connections.
  if (ctx->get_owner()->get_thd()->variables.kill_conflicting_connections ||
      lock->key.mdl_namespace() == MDL_key::SCHEMA) {
    return MDL_lock::object_lock_kill_conflicting_locks(ctx, lock,
                                                        kill_lower_than);
  }
  return false;
}

/**
  Notify threads holding S/SH metadata locks on an object, which conflict
  with a pending X lock.

  @note Currently this method is guaranteed to notify shared lock
        owners which have MDL_context::m_needs_thr_lock_abort flag
        set (as for others conficting locks might have been acquired
        on "fast path" and thus might be absent from list of granted
        locks).
        This is OK as notification for other contexts is anyway
        no-op now.

  @note We don't notify threads holding other than S/SH types of
        conflicting locks on the object since notification should
        not be needed and anyway will be no-op for them (unless
        they also hold S/SH locks on the object).

  @param  ctx  MDL_context for current thread.
  @param  lock MDL_lock object representing lock which is to be
               acquired.
*/

void MDL_lock::object_lock_notify_conflicting_locks(MDL_context *ctx,
                                                    MDL_lock *lock) {
  Ticket_iterator it(lock->m_granted);
  MDL_ticket *conflicting_ticket;

  while ((conflicting_ticket = it++)) {
    if (conflicting_ticket->get_ctx() != ctx &&
        (conflicting_ticket->get_type() == MDL_SHARED ||
         conflicting_ticket->get_type() == MDL_SHARED_HIGH_PRIO) &&
        (conflicting_ticket->get_ctx()->get_owner()->get_thd() != nullptr ||
         ctx->get_ignore_owner_thd())) {
      MDL_context *conflicting_ctx = conflicting_ticket->get_ctx();

      /*
        If thread which holds conflicting lock is waiting on table-level
        lock or some other non-MDL resource we might need to wake it up
        by calling code outside of MDL.

        The only scenario in which it is important now looks like:
        Thread 1: HANDLER t1 OPEN, acquires S metadata lock on t1.
        Thread 2: LOCK TABLES t1 WRITE, t2 READ LOCAL, acquire
                  SNRW lock on t1, SR lock on t2 and TL_READ THR_LOCK
                  lock on t2.
        Thread 1: Executes UPDATE t2 SET i = 1 or some other DML which
                  will directly or indirectly block on THR_LOCK for t2.
        Thread 2: Does ALTER TABLE t1 ... which tries to upgrade SNRW
                  metadata lock to X lock and blocks because of S lock
                  from open HANDLER.

        A similar scenario is possible when MERGE tables are used instead
        of the READ LOCAL clause.
        Once we will get rid of the support for READ LOCAL and MERGE clauses
        this code can be removed.
      */
      ctx->get_owner()->notify_shared_lock(
          conflicting_ctx->get_owner(),
          conflicting_ctx->get_needs_thr_lock_abort());
    }
  }
}

bool MDL_lock::object_lock_kill_conflicting_locks(
    MDL_context *ctx, MDL_lock *lock, enum_mdl_type kill_lower_than) {
  Ticket_iterator it(lock->m_granted);
  MDL_ticket *conflicting_ticket;

  THD *thd = ctx->get_thd();
  while ((conflicting_ticket = it++)) {
    if (conflicting_ticket->get_ctx() != ctx &&
        conflicting_ticket->get_type() < kill_lower_than) {
      MDL_context *conflicting_ctx = conflicting_ticket->get_ctx();
      // if any conflicting thread is not killed, stop and just return false
      if (!ctx->get_owner()->kill_shared_lock(conflicting_ctx->get_owner()))
        return false;
      if (thd->slave_thread) slave_high_priority_ddl_killed_connections++;
    }
  }
  return true;
}

/**
  Acquire one lock with waiting for conflicting locks to go away if needed.

  @param [in,out] mdl_request Lock request object for lock to be acquired

  @param lock_wait_timeout_nsec Nanoseconds to wait before timeout.

  @retval  false   Success. MDL_request::ticket points to the ticket
                   for the lock.
  @retval  true    Failure (Out of resources or waiting is aborted),
*/

bool MDL_context::acquire_lock_nsec(MDL_request *mdl_request,
                                    Timeout_type lock_wait_timeout_nsec) {
  THD *thd = get_thd();
  bool is_high_priority_ddl = false;
  if (thd != nullptr) {
    if (thd->variables.high_priority_ddl) {
      // if this is a high priority command, use the
      // high_priority_lock_wait_timeout_nsec
      lock_wait_timeout_nsec =
          thd->variables.high_priority_lock_wait_timeout_nsec;
      is_high_priority_ddl =
          thd->lex != nullptr && support_high_priority(thd->lex->sql_command);
    } else if (thd->slave_thread && slave_high_priority_ddl) {
      lock_wait_timeout_nsec = slave_high_priority_lock_wait_timeout_nsec;
      is_high_priority_ddl =
          thd->lex != nullptr && support_high_priority(thd->lex->sql_command);
    }
  }

  if (lock_wait_timeout_nsec == 0) {
    /*
      Resort to try_acquire_lock() in case of zero timeout.

      This allows to avoid unnecessary deadlock detection attempt and "fake"
      deadlocks which might result from it.
      In case of failure to acquire lock, try_acquire_lock() preserves
      invariants by updating MDL_lock::fast_path_state and obtrusive locks
      count. It also performs SE notification if needed.
    */
    if (try_acquire_lock(mdl_request)) return true;

    if (!mdl_request->ticket) {
      /* We have failed to acquire lock instantly. */
      DEBUG_SYNC(get_thd(), "mdl_acquire_lock_wait");
      my_error(
          ER_LOCK_WAIT_TIMEOUT, MYF(0),
          timeout_message(mdl_request->key.get_namespace_name(),
                          mdl_request->key.db_name(), mdl_request->key.name())
              .c_ptr_safe());
      return true;
    }
    return false;
  }

  /* Normal, non-zero timeout case. */
  MDL_lock *lock;
  MDL_ticket *ticket = nullptr;
  struct timespec abs_timeout;
  MDL_wait::enum_wait_status wait_status;
  /* Do some work outside the critical section. */
  set_timespec_nsec(&abs_timeout, lock_wait_timeout_nsec);

  if (try_acquire_lock_impl(mdl_request, &ticket)) return true;

  if (mdl_request->ticket) {
    /*
      We have managed to acquire lock without waiting.
      MDL_lock, MDL_context and MDL_request were updated
      accordingly, so we can simply return success.
    */
    return false;
  }

  /*
    Our attempt to acquire lock without waiting has failed.
    As a result of this attempt we got MDL_ticket with m_lock
    member pointing to the corresponding MDL_lock object which
    has MDL_lock::m_rwlock write-locked.
  */
  lock = ticket->m_lock;

  lock->m_waiting.add_ticket(ticket);

  /*
    Once we added a pending ticket to the waiting queue,
    we must ensure that our wait slot is empty, so
    that our lock request can be scheduled. Do that in the
    critical section formed by the acquired write lock on MDL_lock.
  */
  m_wait.reset_status();

  if (lock->needs_notification(ticket)) lock->notify_conflicting_locks(this);

  mysql_prlock_unlock(&lock->m_rwlock);

#ifdef HAVE_PSI_METADATA_INTERFACE
  PSI_metadata_locker_state state;
  PSI_metadata_locker *locker = nullptr;

  if (ticket->m_psi != nullptr) {
    locker = PSI_METADATA_CALL(start_metadata_wait)(&state, ticket->m_psi,
                                                    __FILE__, __LINE__);
  }
#endif

  will_wait_for(ticket);

  /* There is a shared or exclusive lock on the object. */
  DEBUG_SYNC(get_thd(), "mdl_acquire_lock_wait");

  find_deadlock();

  /*
    For high priority ddl, if this lock is upgradable, the
    final timed_wait happens after connection kill. For other
    requests, connections will not be killed only if
    kill_conflicting_connections is set.
  */
  /*
    there are no locks lower than MDL_INTENTION_EXCLUSIVE so initial value
    indicates that no connections will be killed
  */
  enum_mdl_type kill_conflicting_locks_lower_than = MDL_INTENTION_EXCLUSIVE;
  bool kill_conflicting_connections_after_timeout_and_retry = false;
  if (thd != nullptr) {
    if ((thd->variables.high_priority_ddl ||
         (thd->slave_thread && slave_high_priority_ddl)) &&
        ticket->get_type() >= MDL_SHARED_NO_WRITE) {
      kill_conflicting_connections_after_timeout_and_retry = true;
      /* Use MDL_SHARED_NO_WRITE to kill "lock tables read" connection */
      kill_conflicting_locks_lower_than = MDL_SHARED_NO_WRITE;
    }
    if (thd->variables.kill_conflicting_connections) {
      kill_conflicting_connections_after_timeout_and_retry = true;
      kill_conflicting_locks_lower_than = MDL_TYPE_END;
    }
  }
  /* do not set status on timeout if we are going to retry */
  bool set_status_on_timeout =
      !kill_conflicting_connections_after_timeout_and_retry;

  if (lock->needs_notification(ticket) || lock->needs_connection_check()) {
    struct timespec abs_shortwait;
    set_timespec(&abs_shortwait, 1);
    wait_status = MDL_wait::WS_EMPTY;

    while (!is_high_priority_ddl &&
           cmp_timespec(&abs_shortwait, &abs_timeout) <= 0) {
      /* abs_timeout is far away. Wait a short while and notify locks. */
      wait_status = m_wait.timed_wait(m_owner, &abs_shortwait, false,
                                      mdl_request->key.get_wait_state_name());

      if (wait_status != MDL_wait::WS_EMPTY) break;

      if (lock->needs_connection_check() && !m_owner->is_connected()) {
        /*
          If this is user-level lock and the client is disconnected don't wait
          forever: assume it's the same as statement being killed (this differs
          from pre-5.7 where we treat it as timeout, but is more logical).
          Using MDL_wait::set_status() to set status atomically wastes one
          condition variable wake up but should happen rarely.
          We don't want to do this check for all types of metadata locks since
          in general case we may want to complete wait/operation even when
          connection is lost (e.g. in case of logging into slow/general log).
        */
        if (!m_wait.set_status(MDL_wait::KILLED))
          wait_status = MDL_wait::KILLED;
        break;
      }

      if (lock->needs_notification(ticket)) {
        mysql_prlock_wrlock(&lock->m_rwlock);
        lock->notify_conflicting_locks(this);
        mysql_prlock_unlock(&lock->m_rwlock);
      }

      set_timespec(&abs_shortwait, 1);
    }
    if (wait_status == MDL_wait::WS_EMPTY)
      wait_status =
          m_wait.timed_wait(m_owner, &abs_timeout, set_status_on_timeout,
                            mdl_request->key.get_wait_state_name());
  } else {
    wait_status =
        m_wait.timed_wait(m_owner, &abs_timeout, set_status_on_timeout,
                          mdl_request->key.get_wait_state_name());
  }

  /*
   * If DDL request is blocked and timed out, we may be able to kill
   * the blocking connections, and then retry a short wait.
   * NOTE: Only allow super user with ddl command to kill blocking threads
   */
  if ((wait_status == MDL_wait::TIMEOUT || wait_status == MDL_wait::WS_EMPTY) &&
      kill_conflicting_connections_after_timeout_and_retry) {
    if (wait_status != MDL_wait::WS_EMPTY) {
      // reset MDL_wait status
      m_wait.reset_status();
    }

    if (thd->slave_thread) {
      slave_high_priority_ddl_executed++;
    }
    mysql_prlock_wrlock(&lock->m_rwlock);
    lock->kill_conflicting_locks(this, kill_conflicting_locks_lower_than);
    mysql_prlock_unlock(&lock->m_rwlock);

    DEBUG_SYNC(get_thd(), "mdl_high_priority_kill_conflicting_locks");

    // retry after kill_conflicting_connections_timeout seconds as
    // kill command is asynchronous, and MDL is held until killed
    // connections complete rollback that take some time.
    // And only sessions with granted lock are killed but there
    // could be more sessions in the queue before this one.
    set_timespec(&abs_timeout,
                 thd->variables.kill_conflicting_connections_timeout);
    wait_status = m_wait.timed_wait(m_owner, &abs_timeout, true,
                                    mdl_request->key.get_wait_state_name());
  }

  done_waiting_for();

#ifdef HAVE_PSI_METADATA_INTERFACE
  if (locker != nullptr) {
    PSI_METADATA_CALL(end_metadata_wait)(locker, 0);
  }
#endif

  if (wait_status != MDL_wait::GRANTED) {
    lock->remove_ticket(this, m_pins, &MDL_lock::m_waiting, ticket);

    /*
      If SEs were notified about impending lock acquisition, the failure
      to acquire it requires the same notification as lock release.
    */
    if (ticket->m_hton_notified) {
      mysql_mdl_set_status(ticket->m_psi, MDL_ticket::POST_RELEASE_NOTIFY);
      m_owner->notify_hton_post_release_exclusive(&mdl_request->key);
    }

    MDL_ticket::destroy(ticket);
    switch (wait_status) {
      case MDL_wait::VICTIM:
        my_error(ER_LOCK_DEADLOCK, MYF(0));
        break;
      case MDL_wait::TIMEOUT:
        my_error(
            ER_LOCK_WAIT_TIMEOUT, MYF(0),
            timeout_message(mdl_request->key.get_namespace_name(),
                            mdl_request->key.db_name(), mdl_request->key.name())
                .c_ptr_safe());
        break;
      case MDL_wait::KILLED:
        if (get_owner()->is_killed() == ER_QUERY_TIMEOUT)
          my_error(ER_QUERY_TIMEOUT, MYF(0));
        else
          my_error(ER_QUERY_INTERRUPTED, MYF(0));
        break;
      default:
        DBUG_ASSERT(0);
        break;
    }
    return true;
  }

  /*
    We have been granted our request.
    State of MDL_lock object is already being appropriately updated by a
    concurrent thread (@sa MDL_lock:reschedule_waiters()).
    So all we need to do is to update MDL_context and MDL_request objects.
  */
  DBUG_ASSERT(wait_status == MDL_wait::GRANTED);

  m_ticket_store.push_front(mdl_request->duration, ticket);
  mdl_request->ticket = ticket;

  mysql_mdl_set_status(ticket->m_psi, MDL_ticket::GRANTED);

  return false;
}

class MDL_request_cmp {
 public:
  bool operator()(const MDL_request *req1, const MDL_request *req2) {
    int rc = req1->key.cmp(&req2->key);
    /*
      In cases when both requests correspond to the same key, we need to put
      the request for the stronger lock type first, to avoid extra deadlocks.
      We do this by simply comparing types of lock requests.
      This works OK since it is mostly needed to avoid the scenario when
      LOCK TABLES t1 WRITE, t1 READ acquires SRO lock before SNRW lock.
      It won't work in the general case (e.g. SRO is not stronger than SW).
    */
    if (rc == 0)
      rc = static_cast<int>(req2->type) - static_cast<int>(req1->type);
    return rc < 0;
  }
};

/**
  Acquire exclusive locks. There must be no granted locks in the
  context.

  This is a replacement of lock_table_names(). It is used in
  RENAME, DROP and other DDL SQL statements.

  @param  mdl_requests  List of requests for locks to be acquired.

  @param lock_wait_timeout_nsec  Nanoeconds to wait before timeout.

  @note The list of requests should not contain non-exclusive lock requests.
        There should not be any acquired locks in the context.

  @note Assumes that one already owns scoped intention exclusive lock.

  @note If acquisition fails any locks with MDL_EXPLICIT duration that had
        already been taken, are released. Not just locks with MDL_STATEMENT
        and MDL_TRANSACTION duration. This makes acquition of MDL_EXPLICIT
        locks atomic (all or nothing). This is needed for the locking
        service plugin API.

  @retval false  Success
  @retval true   Failure
*/

bool MDL_context::acquire_locks_nsec(MDL_request_list *mdl_requests,
                                     Timeout_type lock_wait_timeout_nsec) {
  MDL_request_list::Iterator it(*mdl_requests);
  MDL_request **p_req;
  MDL_savepoint mdl_svp = mdl_savepoint();
  /*
    Remember the first MDL_EXPLICIT ticket so that we can release
    any new such locks taken if acquisition fails.
  */
  MDL_ticket *explicit_front = m_ticket_store.front(MDL_EXPLICIT);
  const size_t req_count = mdl_requests->elements();

  if (req_count == 0) return false;

  /* Sort requests according to MDL_key. */
  Prealloced_array<MDL_request *, 16> sort_buf(
      key_memory_MDL_context_acquire_locks);
  if (sort_buf.reserve(req_count)) return true;

  for (size_t ii = 0; ii < req_count; ++ii) {
    sort_buf.push_back(it++);
  }

  std::sort(sort_buf.begin(), sort_buf.end(), MDL_request_cmp());

  size_t num_acquired = 0;
  for (p_req = sort_buf.begin(); p_req != sort_buf.end(); p_req++) {
    if (acquire_lock_nsec(*p_req, lock_wait_timeout_nsec)) goto err;
    ++num_acquired;
  }
  return false;

err:
  /*
    Release locks we have managed to acquire so far.
    Use rollback_to_savepoint() since there may be duplicate
    requests that got assigned the same ticket.
  */
  rollback_to_savepoint(mdl_svp);
  /*
    Also release the MDL_EXPLICIT locks taken in this call.
    The locking service plugin API needs acquisition of such
    locks to be atomic as well.
  */
  release_locks_stored_before(MDL_EXPLICIT, explicit_front);

  /* Reset lock requests back to its initial state. */
  for (p_req = sort_buf.begin(); p_req != sort_buf.begin() + num_acquired;
       p_req++) {
    (*p_req)->ticket = nullptr;
  }
  return true;
}

bool MDL_context::clone_tickets(const MDL_context *ticket_owner,
                                enum_mdl_duration duration) {
  MDL_ticket *ticket;
  MDL_ticket_store::List_iterator it_ticket =
      ticket_owner->m_ticket_store.list_iterator(duration);
  bool ret = false;
  MDL_request request;

  while ((ticket = it_ticket++)) {
    DBUG_ASSERT(ticket->m_lock);

    MDL_REQUEST_INIT_BY_KEY(&request, ticket->get_key(), ticket->get_type(),
                            duration);
    request.ticket = ticket;
    ret = clone_ticket(&request);
    if (ret) break;
  }

  if (ret) {
    Ticket_iterator it_ticket1 = m_ticket_store.list_iterator(duration);
    while ((ticket = it_ticket1++)) {
      release_lock(ticket);
    }
  }

  return ret;
}

/**
  Upgrade a shared metadata lock.

  Used in ALTER TABLE and CREATE TABLE.

  @param mdl_ticket         Lock to upgrade.
  @param new_type           Lock type to upgrade to.
  @param lock_wait_timeout_nsec  Nanoeconds to wait before timeout.

  @note In case of failure to upgrade lock (e.g. because upgrader
        was killed) leaves lock in its original state (locked in
        shared mode).

  @note There can be only one upgrader for a lock or we will have deadlock.
        This invariant is ensured by the fact that upgradeable locks SU, SNW
        and SNRW are not compatible with each other and themselves in case
        of ALTER TABLE operation.
        In case of CREATE TABLE operation there is chance of deadlock as 'S'
        is compatible with 'S'. But the deadlock is recovered by backoff and
        retry mechanism.

  @retval false  Success
  @retval true   Failure (thread was killed)
*/

bool MDL_context::upgrade_shared_lock_nsec(
    MDL_ticket *mdl_ticket, enum_mdl_type new_type,
    Timeout_type lock_wait_timeout_nsec) {
  MDL_request mdl_new_lock_request;
  MDL_savepoint mdl_svp = mdl_savepoint();
  bool is_new_ticket;
  MDL_lock *lock;

  DBUG_TRACE;
  DEBUG_SYNC(get_thd(), "mdl_upgrade_lock");

  /*
    Do nothing if already upgraded. Used when we FLUSH TABLE under
    LOCK TABLES and a table is listed twice in LOCK TABLES list.
  */
  if (mdl_ticket->has_stronger_or_equal_type(new_type)) return false;

  MDL_REQUEST_INIT_BY_KEY(&mdl_new_lock_request, &mdl_ticket->m_lock->key,
                          new_type, MDL_TRANSACTION);

  if (acquire_lock_nsec(&mdl_new_lock_request, lock_wait_timeout_nsec))
    return true;

  is_new_ticket = !has_lock(mdl_svp, mdl_new_lock_request.ticket);

  lock = mdl_ticket->m_lock;

  /* Code below assumes that we were upgrading to "obtrusive" type of lock. */
  DBUG_ASSERT(lock->is_obtrusive_lock(new_type));

  /* Merge the acquired and the original lock. @todo: move to a method. */
  mysql_prlock_wrlock(&lock->m_rwlock);
  if (is_new_ticket) {
    lock->m_granted.remove_ticket(mdl_new_lock_request.ticket);
    /*
      We should not clear HAS_OBTRUSIVE flag in this case as we will
      get "obtrusive' lock as result in any case.
    */
    --lock->m_obtrusive_locks_granted_waiting_count;
  }
  /*
    Set the new type of lock in the ticket. To update state of
    MDL_lock object correctly we need to temporarily exclude
    ticket from the granted queue or "fast path" counter and
    then include lock back into granted queue.
    Note that with current code at this point we can't have
    "fast path" tickets as upgrade to "obtrusive" locks
    materializes tickets normally. Still we cover this case
    for completeness.
  */
  if (mdl_ticket->m_is_fast_path) {
    /*
      Decrement of counter in MDL_lock::m_fast_path_state needs to be done
      under protection of MDL_lock::m_rwlock to ensure that it is atomic with
      changes to MDL_lock::m_granted list and to enforce invariant [INV1].
      Note that since we have HAS_OBTRUSIVE flag set at this point all
      concurrent lock acquisitions and releases will have to acquire
      MDL_lock::m_rwlock, so nobody will see results of this decrement until
      m_rwlock is released.
    */
    lock->fast_path_state_add(
        -lock->get_unobtrusive_lock_increment(mdl_ticket->m_type));
    mdl_ticket->m_is_fast_path = false;
  } else {
    lock->m_granted.remove_ticket(mdl_ticket);
    /*
      Also if we are upgrading from "obtrusive" lock we need to temporarily
      decrement m_obtrusive_locks_granted_waiting_count counter.
      We should not clear HAS_OBTRUSIVE flag in this case as we will get
      "obtrusive' lock as result in any case.
    */
    if (lock->is_obtrusive_lock(mdl_ticket->m_type))
      --lock->m_obtrusive_locks_granted_waiting_count;
  }

  mdl_ticket->m_type = new_type;

  lock->m_granted.add_ticket(mdl_ticket);
  /*
    Since we always upgrade to "obtrusive" type of lock we need to
    increment m_obtrusive_locks_granted_waiting_count counter.

    HAS_OBTRUSIVE flag has been already set by acquire_lock()
    and should not have been cleared since then.
  */
  DBUG_ASSERT(lock->m_fast_path_state & MDL_lock::HAS_OBTRUSIVE);
  ++lock->m_obtrusive_locks_granted_waiting_count;

  mysql_prlock_unlock(&lock->m_rwlock);

  /*
    The below code can't handle situation when we upgrade to lock requiring
    SE notification and it turns out that we already have lock of this type
    associated with different ticket.
  */
  DBUG_ASSERT(is_new_ticket || !mdl_new_lock_request.ticket->m_hton_notified);

  mdl_ticket->m_hton_notified = mdl_new_lock_request.ticket->m_hton_notified;

  if (is_new_ticket) {
    m_ticket_store.remove(MDL_TRANSACTION, mdl_new_lock_request.ticket);
    MDL_ticket::destroy(mdl_new_lock_request.ticket);
  }

  return false;
}

/**
  A fragment of recursive traversal of the wait-for graph
  in search for deadlocks. Direct the deadlock visitor to all
  contexts that own the lock the current node in the wait-for
  graph is waiting for.
  As long as the initial node is remembered in the visitor,
  a deadlock is found when the same node is seen twice.
*/

bool MDL_lock::visit_subgraph(MDL_ticket *waiting_ticket,
                              MDL_wait_for_graph_visitor *gvisitor) {
  MDL_ticket *ticket;
  MDL_context *src_ctx = waiting_ticket->get_ctx();
  bool result = true;

#ifndef EXTRA_CODE_FOR_UNIT_TESTING
  if (!enable_acl_cache_deadlock_detection &&
      key.mdl_namespace() == MDL_key::ACL_CACHE) {
    /*
      Deadlock detection for ACL_CACHE may lead to bad contention problems
      in the connection path. In particular taking rdlock on m_rwlock
      while doing deadlock traversal takes longer with more granted and
      waiting tickets, but the releasing of these granted tickets are
      themselves blocked on taking wrlock on m_rwlock, meanwhile more
      connections are trying to come in and grant more tickets and
      taking wrlock as well, so this becomes a serious lock contention
      that you can't get out of if you keep new connection come in at
      a fast pace. A easy way to trigger this is to run FLUSH
      PRIVILEGES on a server that has lots of incoming connections with
      lots of connection from acl_check_host / has_global_grant.

      For ACL_CACHE, the deadlock detection in most cases doesn't provide
      a ton of value in production so this allows turning it off for
      ACL_CACHE only.
     */
    return false;
  }
#endif

  mysql_prlock_rdlock(&m_rwlock);

  /*
    Iterators must be initialized after taking a read lock.

    Note that MDL_ticket's which correspond to lock requests satisfied
    on "fast path" are not present in m_granted list and thus
    corresponding edges are missing from wait-for graph.
    It is OK since contexts with "fast path" tickets are not allowed to
    wait for any resource (they have to convert "fast path" tickets to
    normal tickets first) and thus cannot participate in deadlock.
    @sa MDL_contex::will_wait_for().
  */
  Ticket_iterator granted_it(m_granted);
  Ticket_iterator waiting_it(m_waiting);

  /*
    MDL_lock's waiting and granted queues and MDL_context::m_waiting_for
    member are updated by different threads when the lock is granted
    (see MDL_context::acquire_lock() and MDL_lock::reschedule_waiters()).
    As a result, here we may encounter a situation when MDL_lock data
    already reflects the fact that the lock was granted but
    m_waiting_for member has not been updated yet.

    For example, imagine that:

    thread1: Owns SNW lock on table t1.
    thread2: Attempts to acquire SW lock on t1,
             but sees an active SNW lock.
             Thus adds the ticket to the waiting queue and
             sets m_waiting_for to point to the ticket.
    thread1: Releases SNW lock, updates MDL_lock object to
             grant SW lock to thread2 (moves the ticket for
             SW from waiting to the active queue).
             Attempts to acquire a new SNW lock on t1,
             sees an active SW lock (since it is present in the
             active queue), adds ticket for SNW lock to the waiting
             queue, sets m_waiting_for to point to this ticket.

    At this point deadlock detection algorithm run by thread1 will see that:
    - Thread1 waits for SNW lock on t1 (since m_waiting_for is set).
    - SNW lock is not granted, because it conflicts with active SW lock
      owned by thread 2 (since ticket for SW is present in granted queue).
    - Thread2 waits for SW lock (since its m_waiting_for has not been
      updated yet!).
    - SW lock is not granted because there is pending SNW lock from thread1.
      Therefore deadlock should exist [sic!].

    To avoid detection of such false deadlocks we need to check the "actual"
    status of the ticket being waited for, before analyzing its blockers.
    We do this by checking the wait status of the context which is waiting
    for it. To avoid races this has to be done under protection of
    MDL_lock::m_rwlock lock.
  */
  if (src_ctx->m_wait.get_status() != MDL_wait::WS_EMPTY) {
    result = false;
    goto end;
  }

  /*
    To avoid visiting nodes which were already marked as victims of
    deadlock detection (or whose requests were already satisfied) we
    enter the node only after peeking at its wait status.
    This is necessary to avoid active waiting in a situation
    when previous searches for a deadlock already selected the
    node we're about to enter as a victim (see the comment
    in MDL_context::find_deadlock() for explanation why several searches
    can be performed for the same wait).
    There is no guarantee that the node isn't chosen a victim while we
    are visiting it but this is OK: in the worst case we might do some
    extra work and one more context might be chosen as a victim.
  */
  if (gvisitor->enter_node(src_ctx)) goto end;

  /*
    We do a breadth-first search first -- that is, inspect all
    edges of the current node, and only then follow up to the next
    node. In workloads that involve wait-for graph loops this
    has proven to be a more efficient strategy [citation missing].
  */
  while ((ticket = granted_it++)) {
    /* Filter out edges that point to the same node. */
    if (ticket->get_ctx() != src_ctx &&
        ticket->is_incompatible_when_granted(waiting_ticket->get_type()) &&
        gvisitor->inspect_edge(ticket->get_ctx())) {
      goto end_leave_node;
    }
  }

  while ((ticket = waiting_it++)) {
    /* Filter out edges that point to the same node. */
    if (ticket->get_ctx() != src_ctx &&
        ticket->is_incompatible_when_waiting(waiting_ticket->get_type()) &&
        gvisitor->inspect_edge(ticket->get_ctx())) {
      goto end_leave_node;
    }
  }

  /* Recurse and inspect all adjacent nodes. */
  granted_it.rewind();
  while ((ticket = granted_it++)) {
    if (ticket->get_ctx() != src_ctx &&
        ticket->is_incompatible_when_granted(waiting_ticket->get_type()) &&
        ticket->get_ctx()->visit_subgraph(gvisitor)) {
      goto end_leave_node;
    }
  }

  waiting_it.rewind();
  while ((ticket = waiting_it++)) {
    if (ticket->get_ctx() != src_ctx &&
        ticket->is_incompatible_when_waiting(waiting_ticket->get_type()) &&
        ticket->get_ctx()->visit_subgraph(gvisitor)) {
      goto end_leave_node;
    }
  }

  result = false;

end_leave_node:
  gvisitor->leave_node(src_ctx);

end:
  mysql_prlock_unlock(&m_rwlock);
  return result;
}

/**
  Traverse a portion of wait-for graph which is reachable
  through the edge represented by this ticket and search
  for deadlocks.

  @retval true  A deadlock is found. A pointer to deadlock
                 victim is saved in the visitor.
  @retval false
*/

bool MDL_ticket::accept_visitor(MDL_wait_for_graph_visitor *gvisitor) {
  return m_lock->visit_subgraph(this, gvisitor);
}

/**
  A fragment of recursive traversal of the wait-for graph of
  MDL contexts in the server in search for deadlocks.
  Assume this MDL context is a node in the wait-for graph,
  and direct the visitor to all adjacent nodes. As long
  as the starting node is remembered in the visitor, a
  deadlock is found when the same node is visited twice.
  One MDL context is connected to another in the wait-for
  graph if it waits on a resource that is held by the other
  context.

  @retval true  A deadlock is found. A pointer to deadlock
                victim is saved in the visitor.
  @retval false
*/

bool MDL_context::visit_subgraph(MDL_wait_for_graph_visitor *gvisitor) {
  bool result = false;

  mysql_prlock_rdlock(&m_LOCK_waiting_for);

  if (m_waiting_for) result = m_waiting_for->accept_visitor(gvisitor);

  mysql_prlock_unlock(&m_LOCK_waiting_for);

  return result;
}

/**
  Try to find a deadlock. This function produces no errors.

  @note If during deadlock resolution context which performs deadlock
        detection is chosen as a victim it will be informed about the
        fact by setting VICTIM status to its wait slot.
*/

void MDL_context::find_deadlock() {
  while (true) {
    /*
      The fact that we use fresh instance of gvisitor for each
      search performed by find_deadlock() below is important,
      the code responsible for victim selection relies on this.
    */
    Deadlock_detection_visitor dvisitor(this);
    MDL_context *victim;

    if (!visit_subgraph(&dvisitor)) {
      /* No deadlocks are found! */
      break;
    }

    victim = dvisitor.get_victim();

    /*
      Failure to change status of the victim is OK as it means
      that the victim has received some other message and is
      about to stop its waiting/to break deadlock loop.
      Even when the initiator of the deadlock search is
      chosen the victim, we need to set the respective wait
      result in order to "close" it for any attempt to
      schedule the request.
      This is needed to avoid a possible race during
      cleanup in case when the lock request on which the
      context was waiting is concurrently satisfied.
    */
    (void)victim->m_wait.set_status(MDL_wait::VICTIM);
    victim->unlock_deadlock_victim();

    if (victim == this) break;
    /*
      After adding a new edge to the waiting graph we found that it
      creates a loop (i.e. there is a deadlock). We decided to destroy
      this loop by removing an edge, but not the one that we added.
      Since this doesn't guarantee that all loops created by addition
      of the new edge are destroyed, we have to repeat the search.
    */
  }
}

/**
  Release lock.

  @param duration Lock duration.
  @param ticket   Ticket for lock to be released.

*/

void MDL_context::release_lock(enum_mdl_duration duration, MDL_ticket *ticket) {
  MDL_lock *lock = ticket->m_lock;
  MDL_key key_for_hton;
  DBUG_TRACE;
  DBUG_PRINT("enter", ("db=%s name=%s", lock->key.db_name(), lock->key.name()));

  DBUG_ASSERT(this == ticket->get_ctx());
  mysql_mutex_assert_not_owner(&LOCK_open);

  // Remove ticket from the Ticket_store before actually releasing the lock,
  // so this removal process can safely reference MDL_lock::m_key in cases
  // when Ticket_store uses hash-based secondary index.
  m_ticket_store.remove(duration, ticket);

  /*
    If lock we are about to release requires post-release notification
    of SEs, we need to save its MDL_key on stack. This is necessary to
    be able to pass this key to SEs after corresponding MDL_lock object
    might be freed as result of lock release.
  */
  if (ticket->m_hton_notified) key_for_hton.mdl_key_init(&lock->key);

  if (ticket->m_is_fast_path) {
    /*
      We are releasing ticket which represents lock request which was
      satisfied using "fast path". We can use "fast path" release
      algorithm of release for it as well.
    */
    MDL_lock::fast_path_state_t unobtrusive_lock_increment =
        lock->get_unobtrusive_lock_increment(ticket->get_type());
    bool is_singleton = mdl_locks.is_lock_object_singleton(&lock->key);

    /* We should not have "fast path" tickets for "obtrusive" lock types. */
    DBUG_ASSERT(unobtrusive_lock_increment != 0);

    /*
      We need decrement part of m_fast_path_state which holds number of
      acquired "fast path" locks of this type. This needs to be done
      by atomic compare-and-swap.

      The same atomic compare-and-swap needs to check:

      *) If HAS_OBSTRUSIVE flag is set. In this case we need to acquire
         MDL_lock::m_rwlock before changing m_fast_path_state. This is
         needed to enforce invariant [INV1] and also because we might
         have to atomically wake-up some waiters for our "unobtrusive"
         lock to go away.
      *) If we are about to release last "fast path" lock and there
         are no "slow path" locks. In this case we need to count
         MDL_lock object as unused and maybe even delete some
         unused MDL_lock objects eventually.

      Similarly to the case with "fast path" acquisition it is OK to
      perform ordinary read of MDL_lock::m_fast_path_state as correctness
      of value returned by it will be validated by atomic compare-and-swap.
      Again, in theory, this algorithm will work correctly if the read will
      return random values.
    */
    MDL_lock::fast_path_state_t old_state = lock->m_fast_path_state;
    bool last_use;

    do {
      if (old_state & MDL_lock::HAS_OBTRUSIVE) {
        mysql_prlock_wrlock(&lock->m_rwlock);
        /*
          It is possible that obtrusive lock has gone away since we have
          read m_fast_path_state value. This means that there is possibility
          that there are no "slow path" locks (HAS_SLOW_PATH is not set) and
          we are about to release last "fast path" lock. In this case MDL_lock
          will become unused and needs to be counted as such eventually.
        */
        last_use = (lock->fast_path_state_add(-unobtrusive_lock_increment) ==
                    unobtrusive_lock_increment);
        /*
          There might be some lock requests waiting for ticket being released
          to go away. Since this is "fast path" ticket it represents
          "unobtrusive" type of lock. In this case if there are any waiters
          for it there should be "obtrusive" type of request among them.
        */
        if (lock->m_obtrusive_locks_granted_waiting_count)
          lock->reschedule_waiters();
        mysql_prlock_unlock(&lock->m_rwlock);
        goto end_fast_path;
      }
      /*
        If there are no "slow path" locks (HAS_SLOW_PATH is not set) and
        we are about to release last "fast path" lock - MDL_lock object
        will become unused and needs to be counted as such.
      */
      last_use = (old_state == unobtrusive_lock_increment);
    } while (!lock->fast_path_state_cas(
        &old_state, old_state - unobtrusive_lock_increment));

  end_fast_path:
    /* Don't count singleton MDL_lock objects as unused. */
    if (last_use && !is_singleton) mdl_locks.lock_object_unused(this, m_pins);
  } else {
    /*
      Lock request represented by ticket was acquired using "slow path"
      or ticket was materialized later. We need to use "slow path" release.
    */
    lock->remove_ticket(this, m_pins, &MDL_lock::m_granted, ticket);
  }
  if (ticket->m_hton_notified) {
    mysql_mdl_set_status(ticket->m_psi, MDL_ticket::POST_RELEASE_NOTIFY);
    m_owner->notify_hton_post_release_exclusive(&key_for_hton);
  }

  MDL_ticket::destroy(ticket);
}

/**
  Release lock with explicit duration.

  @param ticket   Ticket for lock to be released.

*/

void MDL_context::release_lock(MDL_ticket *ticket) {
  DBUG_ASSERT(ticket->m_duration == MDL_EXPLICIT);

  release_lock(MDL_EXPLICIT, ticket);
}

/**
  Release all locks associated with the context. If the sentinel
  is not NULL, do not release locks stored in the list after and
  including the sentinel.

  Statement and transactional locks are added to the beginning of
  the corresponding lists, i.e. stored in reverse temporal order.
  This allows to employ this function to:
  - back off in case of a lock conflict.
  - release all locks in the end of a statement or transaction
  - rollback to a savepoint.
*/

void MDL_context::release_locks_stored_before(enum_mdl_duration duration,
                                              MDL_ticket *sentinel) {
  DBUG_TRACE;

  MDL_ticket *ticket;
  MDL_ticket_store::List_iterator it = m_ticket_store.list_iterator(duration);
  if (m_ticket_store.is_empty(duration)) {
    return;
  }

  while ((ticket = it++) && ticket != sentinel) {
    DBUG_PRINT("info", ("found lock to release ticket=%p", ticket));
    release_lock(duration, ticket);
  }
}

/**
  Release all explicit locks in the context which correspond to the
  same name/object as this lock request.

  @param name      One of the locks for the name/object for which all
                   locks should be released.
*/

void MDL_context::release_all_locks_for_name(MDL_ticket *name) {
  /* Use MDL_ticket::m_lock to identify other locks for the same object. */
  MDL_lock *lock = name->m_lock;

  /* Remove matching lock tickets from the context. */
  MDL_ticket *ticket;
  MDL_ticket_store::List_iterator it_ticket =
      m_ticket_store.list_iterator(MDL_EXPLICIT);

  while ((ticket = it_ticket++)) {
    DBUG_ASSERT(ticket->m_lock);
    if (ticket->m_lock == lock) release_lock(MDL_EXPLICIT, ticket);
  }
}

/**
  Release all explicit locks in the context for which the release()
  method of the provided visitor evalates to true.

  @param visitor   Object to ask if the lock should be released.
*/

void MDL_context::release_locks(MDL_release_locks_visitor *visitor) {
  /* Remove matching lock tickets from the context. */
  MDL_ticket *ticket;
  MDL_ticket_store::List_iterator it_ticket =
      m_ticket_store.list_iterator(MDL_EXPLICIT);

  while ((ticket = it_ticket++)) {
    DBUG_ASSERT(ticket->m_lock);
    if (visitor->release(ticket)) release_lock(MDL_EXPLICIT, ticket);
  }
}

/**
  Downgrade an EXCLUSIVE or SHARED_NO_WRITE lock to shared metadata lock.

  @param new_type  Type of lock to which exclusive lock should be downgraded.
*/

void MDL_ticket::downgrade_lock(enum_mdl_type new_type) {
  bool new_type_is_unobtrusive;
  mysql_mutex_assert_not_owner(&LOCK_open);
  DBUG_ASSERT(m_lock->m_strategy->legal_type[new_type]);

  /*
    Do nothing if already downgraded. Used when we FLUSH TABLE under
    LOCK TABLES and a table is listed twice in LOCK TABLES list.
    Note that this code might even try to "downgrade" a weak lock
    (e.g. SW) to a stronger one (e.g SNRW). So we can't even assert
    here that target lock is weaker than existing lock.
  */
  if (m_type == new_type || !has_stronger_or_equal_type(new_type)) return;

  /* Only allow downgrade from EXCLUSIVE and SHARED_NO_WRITE. */
  DBUG_ASSERT(m_type == MDL_EXCLUSIVE || m_type == MDL_SHARED_NO_WRITE);

  /* Below we assume that we always downgrade "obtrusive" locks. */
  DBUG_ASSERT(m_lock->is_obtrusive_lock(m_type));

  new_type_is_unobtrusive = !m_lock->is_obtrusive_lock(new_type);

  mysql_prlock_wrlock(&m_lock->m_rwlock);
  /*
    To update state of MDL_lock object correctly we need to temporarily
    exclude ticket from the granted queue and then include it back.

    Since we downgrade only "obtrusive" locks we can always assume that the
    ticket for the lock being downgraded is a "slow path" ticket.
    If we are downgrading to non-"obtrusive" lock we also should decrement
    counter of waiting and granted "obtrusive" locks.
  */
  m_lock->m_granted.remove_ticket(this);
  if (new_type_is_unobtrusive) {
    if ((--m_lock->m_obtrusive_locks_granted_waiting_count) == 0) {
      /*
        We are downgrading the last "obtrusive" lock. So we need to clear
        HAS_OBTRUSIVE flag.
        Note that it doesn't matter that we do this before including ticket
        to MDL_lock::m_granted list. Threads requesting "obtrusive" locks
        won't see this until MDL_lock::m_rwlock is released. And threads
        requesting "unobtrusive" locks don't care about this ticket.
      */
      MDL_lock::fast_path_state_t old_state = m_lock->m_fast_path_state;
      while (!m_lock->fast_path_state_cas(
          &old_state, old_state & ~MDL_lock::HAS_OBTRUSIVE)) {
      }
    }
  }
  m_type = new_type;
  m_lock->m_granted.add_ticket(this);
  m_lock->reschedule_waiters();
  mysql_prlock_unlock(&m_lock->m_rwlock);

  /*
    When we downgrade X lock for which SEs were notified about lock
    acquisition we treat situation in the same way as lock release
    (from SEs notification point of view).
  */
  if (m_hton_notified) {
    mysql_mdl_set_status(m_psi, MDL_ticket::POST_RELEASE_NOTIFY);
    m_ctx->get_owner()->notify_hton_post_release_exclusive(&m_lock->key);
    m_hton_notified = false;
    mysql_mdl_set_status(m_psi, MDL_ticket::GRANTED);
  }
}

/**
  Auxiliary function which allows to check if we have some kind of lock on
  a object. Returns true if we have a lock of an equal to given or stronger
  type.

  @param mdl_key       Key to check for ownership
  @param mdl_type      Lock type. Pass in the weakest type to find
                       out if there is at least some lock.

  @return TRUE if current context contains satisfied lock for the object,
          FALSE otherwise.
*/

bool MDL_context::owns_equal_or_stronger_lock(const MDL_key *mdl_key,
                                              enum_mdl_type mdl_type) {
  MDL_request mdl_request;
  enum_mdl_duration not_used;
  /* We don't care about exact duration of lock here. */
  MDL_REQUEST_INIT_BY_KEY(&mdl_request, mdl_key, mdl_type, MDL_TRANSACTION);
  MDL_ticket *ticket = find_ticket(&mdl_request, &not_used);

  DBUG_ASSERT(ticket == nullptr || ticket->m_lock);

  return ticket;
}

/**
  Auxiliary function which allows to check if we have some kind of lock on
  a object. Returns TRUE if we have a lock of an equal to given or stronger
  type.

  @param mdl_namespace Id of object namespace
  @param db            Name of the database
  @param name          Name of the object
  @param mdl_type      Lock type. Pass in the weakest type to find
                       out if there is at least some lock.

  @return true if current context contains satisfied lock for the object,
          false otherwise.
*/

bool MDL_context::owns_equal_or_stronger_lock(
    MDL_key::enum_mdl_namespace mdl_namespace, const char *db, const char *name,
    enum_mdl_type mdl_type) {
  MDL_request mdl_request;
  enum_mdl_duration not_used;
  /* We don't care about exact duration of lock here. */
  MDL_REQUEST_INIT(&mdl_request, mdl_namespace, db, name, mdl_type,
                   MDL_TRANSACTION);
  MDL_ticket *ticket = find_ticket(&mdl_request, &not_used);

  DBUG_ASSERT(ticket == nullptr || ticket->m_lock);

  return ticket;
}

/**
  Find the first context which owns the lock and inspect it by
  calling MDL_context_visitor::visit_context() method.

  @return True in case error (e.g. OOM). False otherwise. There
          is no guarantee that owner was found in either case.
  @note This method only works properly for locks which were
        acquired using "slow" path.
*/

bool MDL_context::find_lock_owner(const MDL_key *mdl_key,
                                  MDL_context_visitor *visitor) {
  MDL_lock *lock = nullptr;
  MDL_context *owner;
  bool pinned;

  if (fix_pins()) return true;

retry:
  if ((lock = mdl_locks.find(m_pins, mdl_key, &pinned)) == MY_LF_ERRPTR)
    return true;

  /* No MDL_lock object, no owner, nothing to visit. */
  if (lock == nullptr) return false;

  mysql_prlock_rdlock(&lock->m_rwlock);

  if (lock->m_fast_path_state & MDL_lock::IS_DESTROYED) {
    mysql_prlock_unlock(&lock->m_rwlock);
    if (pinned) lf_hash_search_unpin(m_pins);
    DEBUG_SYNC(get_thd(), "mdl_find_lock_owner_is_destroyed");
    goto retry;
  }

  if (pinned) lf_hash_search_unpin(m_pins);

  if ((owner = lock->get_lock_owner())) visitor->visit_context(owner);

  mysql_prlock_unlock(&lock->m_rwlock);

  return false;
}

/**
  Check if we have any pending locks which conflict with existing shared lock.

  @pre The ticket must match an acquired lock.

  @return true if there is a conflicting lock request, false otherwise.
*/

bool MDL_ticket::has_pending_conflicting_lock() const {
  return m_lock->has_pending_conflicting_lock(m_type);
}

/** Return a key identifying this lock. */

const MDL_key *MDL_ticket::get_key() const { return &m_lock->key; }

/**
  Releases metadata locks that were acquired after a specific savepoint.

  @note Used to release tickets acquired during a savepoint unit.
  @note It's safe to iterate and unlock any locks after taken after this
        savepoint because other statements that take other special locks
        cause a implicit commit (ie LOCK TABLES).
*/

void MDL_context::rollback_to_savepoint(const MDL_savepoint &mdl_savepoint) {
  DBUG_TRACE;

  /* If savepoint is NULL, it is from the start of the transaction. */
  release_locks_stored_before(MDL_STATEMENT, mdl_savepoint.m_stmt_ticket);
  release_locks_stored_before(MDL_TRANSACTION, mdl_savepoint.m_trans_ticket);
}

/**
  Release locks acquired by normal statements (SELECT, UPDATE,
  DELETE, etc) in the course of a transaction. Do not release
  HANDLER locks, if there are any.

  This method is used at the end of a transaction, in
  implementation of COMMIT (implicit or explicit) and ROLLBACK.
*/

void MDL_context::release_transactional_locks() {
  DBUG_TRACE;
  release_locks_stored_before(MDL_STATEMENT, nullptr);
  release_locks_stored_before(MDL_TRANSACTION, nullptr);
}

void MDL_context::release_statement_locks() {
  DBUG_TRACE;
  release_locks_stored_before(MDL_STATEMENT, nullptr);
}

/**
  Does this savepoint have this lock?

  @retval true  The ticket is older than the savepoint or
                is an LT, HA or GLR ticket. Thus it belongs
                to the savepoint or has explicit duration.
  @retval false The ticket is newer than the savepoint.
                and is not an LT, HA or GLR ticket.
*/

bool MDL_context::has_lock(const MDL_savepoint &mdl_savepoint,
                           MDL_ticket *mdl_ticket) {
  MDL_ticket *ticket;
  /* Start from the beginning, most likely mdl_ticket's been just acquired. */

  MDL_ticket_store::List_iterator s_it =
      m_ticket_store.list_iterator(MDL_STATEMENT);
  MDL_ticket_store::List_iterator t_it =
      m_ticket_store.list_iterator(MDL_TRANSACTION);

  while ((ticket = s_it++) && ticket != mdl_savepoint.m_stmt_ticket) {
    if (ticket == mdl_ticket) return false;
  }

  while ((ticket = t_it++) && ticket != mdl_savepoint.m_trans_ticket) {
    if (ticket == mdl_ticket) return false;
  }
  return true;
}

/**
  Does this context have an lock of the given namespace?

  @retval true    At least one lock of given namespace held
  @retval false   No locks of the given namespace held
*/

bool MDL_context::has_locks(MDL_key::enum_mdl_namespace mdl_namespace) const {
  MDL_ticket *ticket;

  for (int i = 0; i < MDL_DURATION_END; i++) {
    enum_mdl_duration duration = static_cast<enum_mdl_duration>(i);

    MDL_ticket_store::List_iterator it = m_ticket_store.list_iterator(duration);
    while ((ticket = it++)) {
      if (ticket->get_key()->mdl_namespace() == mdl_namespace) return true;
    }
  }
  return false;
}

/**
  Do we hold any locks which are possibly being waited
  for by another MDL_context?

  @retval true  A lock being 'waited_for' was found.
  @retval false No one waits for the lock(s) we hold.

  @note Should only be called from the thread which
        owns the MDL_context
*/

bool MDL_context::has_locks_waited_for() const {
  MDL_ticket *ticket;

  for (int i = 0; i < MDL_DURATION_END; i++) {
    const enum_mdl_duration duration = static_cast<enum_mdl_duration>(i);

    MDL_ticket_store::List_iterator it = m_ticket_store.list_iterator(duration);
    while ((ticket = it++)) {
      MDL_lock *const lock = ticket->m_lock;

      mysql_prlock_rdlock(&lock->m_rwlock);
      const bool has_waiters = !lock->m_waiting.is_empty();
      mysql_prlock_unlock(&lock->m_rwlock);

      if (!has_waiters) return true;
    }
  }
  return false;
}

/**
  Change lock duration for transactional lock.

  @param mdl_ticket Ticket representing lock.
  @param duration Lock duration to be set.

  @note This method only supports changing duration of
        transactional lock to some other duration.
*/

void MDL_context::set_lock_duration(MDL_ticket *mdl_ticket,
                                    enum_mdl_duration duration) {
  DBUG_ASSERT(mdl_ticket->m_duration == MDL_TRANSACTION &&
              duration != MDL_TRANSACTION);
  m_ticket_store.remove(MDL_TRANSACTION, mdl_ticket);
  m_ticket_store.push_front(duration, mdl_ticket);

#ifndef DBUG_OFF
  mdl_ticket->m_duration = duration;
#endif
}

/**
  Set explicit duration for all locks in the context.
*/

void MDL_context::set_explicit_duration_for_all_locks() {
  m_ticket_store.move_all_to_explicit_duration();
}

/**
  Set transactional duration for all locks in the context.
*/

void MDL_context::set_transaction_duration_for_all_locks() {
  m_ticket_store.move_explicit_to_transaction_duration();
}

void MDL_context::get_locked_object_db_names(MDL_DB_Name_List &list) {
  DBUG_ENTER("MDL_context::get_locked_object_db_names");
  MDL_ticket_store::List_iterator it =
      m_ticket_store.list_iterator(MDL_TRANSACTION);

  for (MDL_ticket *ticket = it++; ticket != nullptr; ticket = it++) {
    MDL_lock *lock = ticket->m_lock;
    DBUG_PRINT("enter",
               ("db=%s name=%s", lock->key.db_name(), lock->key.name()));
    list.insert(std::string(lock->key.db_name()));
  }

  DBUG_VOID_RETURN;
}

size_t MDL_ticket_store::Hash::operator()(const MDL_key *k) const {
  return static_cast<size_t>(murmur3_32(k->ptr(), k->length(), 0));
}

MDL_ticket_store::MDL_ticket_handle MDL_ticket_store::find_in_lists(
    const MDL_request &req) const {
  for (int i = 0; i < MDL_DURATION_END; i++) {
    int di = (req.duration + i) % MDL_DURATION_END;

    List_iterator it(m_durations[di].m_ticket_list);

    for (MDL_ticket *ticket = it++; ticket != nullptr; ticket = it++) {
      if (req.key.is_equal(ticket->get_key()) &&
          ticket->has_stronger_or_equal_type(req.type)) {
        return {ticket, static_cast<enum_mdl_duration>(di)};
      }
    }
  }
  return {nullptr, MDL_DURATION_END};
}

MDL_ticket_store::MDL_ticket_handle MDL_ticket_store::find_in_hash(
    const MDL_request &req) const {
  auto foundrng = m_map->equal_range(&req.key);

  const MDL_ticket_handle *found_handle = nullptr;
  std::find_if(foundrng.first, foundrng.second,
               [&](const Ticket_map::value_type &vt) {
                 auto &th = vt.second;
                 if (!th.m_ticket->has_stronger_or_equal_type(req.type)) {
                   return false;
                 }
                 found_handle = &th;
                 return (th.m_dur == req.duration);
               });

  if (found_handle != nullptr) {
    return *found_handle;
  }
  return {nullptr, MDL_DURATION_END};
}

bool MDL_ticket_store::is_empty() const {
  return (std::all_of(
      std::begin(m_durations), std::end(m_durations), [](const Duration &d) {
        DBUG_ASSERT(!d.m_ticket_list.is_empty() || d.m_mat_front == nullptr);
        return d.m_ticket_list.is_empty();
      }));
}

bool MDL_ticket_store::is_empty(int di) const {
  return m_durations[di].m_ticket_list.is_empty();
}

MDL_ticket *MDL_ticket_store::front(int di) {
  return m_durations[di].m_ticket_list.front();
}

void MDL_ticket_store::push_front(enum_mdl_duration dur, MDL_ticket *ticket) {
  ++m_count;
  m_durations[dur].m_ticket_list.push_front(ticket);

  /*
    Note that we do not update m_durations[dur].m_mat_front here. That
    is ok, since it is only to be used as an optimization for
    MDL_context::materialize_fast_path_locks(). So if the ticket being
    added is already materialized it does not break an invariant, and
    m_mat_front will be updated the next time
    MDL_context::materialize_fast_path_locks() runs.
  */

  if (m_count < THRESHOLD) {
    return;
  }

  if (m_count == THRESHOLD) {
    // If this is the first time we cross the threshold, the map must
    // be allocated
    if (m_map == nullptr) {
      m_map.reset(new Ticket_map{INITIAL_BUCKET_COUNT, Hash{}, Key_equal{}});
    }
    // In any event, it should now be empty
    DBUG_ASSERT(m_map->empty());

    /*
       When the THRESHOLD value is reached, the unordered map must be
       populated with all the tickets added before reaching the
       threshold.
    */
    for_each_ticket_in_ticket_lists(
        [this](MDL_ticket *t, enum_mdl_duration da) {
          m_map->emplace(t->get_key(), MDL_ticket_handle{t, da});
        });
    DBUG_ASSERT(m_map->size() == m_count);
    return;
  }

  m_map->emplace(ticket->get_key(), MDL_ticket_handle{ticket, dur});
  DBUG_ASSERT(m_map->size() == m_count);
}

void MDL_ticket_store::remove(enum_mdl_duration dur, MDL_ticket *ticket) {
  --m_count;
  m_durations[dur].m_ticket_list.remove(ticket);

  if (m_durations[dur].m_mat_front == ticket) {
    m_durations[dur].m_mat_front = ticket->next_in_context;
  }

  if (m_count < THRESHOLD - 1) {
    DBUG_ASSERT(m_map == nullptr || m_map->empty());
    return;
  }
  DBUG_ASSERT(m_map != nullptr && m_map->size() == m_count + 1);

  if (m_count == THRESHOLD - 1) {
    // This remove dipped us below threshold
    m_map->clear();
    return;
  }
  DBUG_ASSERT(m_count >= THRESHOLD);

  auto foundrng = m_map->equal_range(ticket->get_key());

  auto foundit = std::find_if(
      foundrng.first, foundrng.second, [&](const Ticket_map::value_type &vt) {
        auto &th = vt.second;
        DBUG_ASSERT(th.m_ticket != ticket || th.m_dur == dur);
        return (th.m_ticket == ticket);
      });
  DBUG_ASSERT(foundit != foundrng.second);
  if (foundit != foundrng.second) {
    m_map->erase(foundit);
  }
  DBUG_ASSERT(m_map->size() == m_count);
}

void MDL_ticket_store::move_all_to_explicit_duration() {
  int i;
  MDL_ticket *ticket;

  /*
    It is assumed that at this point all the locks in the context are
    materialized. So the next call to MDL_context::materialize_fast_path_locks()
    will not be considering these locks. m_mat_front is valid for the current
    list of tickets with explicit duration and if we had added all the tickets
    which previously had transaction duration at the front, it would still be
    valid. But since we are using the swap-trick below, all the previously
    transactional tickets end up at the tail end of the list, and the order of
    the already explicit tickets is reversed when they are moved (remove pops
    from the front and push_front adds to the front). So unless all the tickets
    were already materialized we're guaranteed that m_mat_front is wrong after
    the swapping and moving. Hence we set m_mat_front of tickets with explicit
    duration to null so that the next call to
    MDL_context::materialize_fast_path_locks() will set it appropriately.
  */
  m_durations[MDL_EXPLICIT].m_mat_front = nullptr;

  /*
    In the most common case when this function is called list
    of transactional locks is bigger than list of locks with
    explicit duration. So we start by swapping these two lists
    and then move elements from new list of transactional
    locks and list of statement locks to list of locks with
    explicit duration.
  */

  m_durations[MDL_EXPLICIT].m_ticket_list.swap(
      m_durations[MDL_TRANSACTION].m_ticket_list);

  for (i = 0; i < MDL_EXPLICIT; i++) {
    m_durations[i].m_mat_front = nullptr;
    List_iterator it_ticket(m_durations[i].m_ticket_list);

    while ((ticket = it_ticket++)) {
      m_durations[i].m_ticket_list.remove(ticket);
      m_durations[MDL_EXPLICIT].m_ticket_list.push_front(ticket);
    }
  }

#ifndef DBUG_OFF
  List_iterator exp_it(m_durations[MDL_EXPLICIT].m_ticket_list);

  while ((ticket = exp_it++)) {
    ticket->set_duration(MDL_EXPLICIT);
  }
#endif

  // Must also change duration in map
  if (m_map != nullptr) {
    for (auto &p : *m_map) {
      p.second.m_dur = MDL_EXPLICIT;
    }
  }
}

void MDL_ticket_store::move_explicit_to_transaction_duration() {
  MDL_ticket *ticket;

  /*
    In the most common case when this function is called list
    of explicit locks is bigger than two other lists (in fact,
    list of statement locks is always empty). So we start by
    swapping list of explicit and transactional locks and then
    move contents of new list of explicit locks to list of
    locks with transactional duration.
  */

  DBUG_ASSERT(m_durations[MDL_STATEMENT].m_ticket_list.is_empty());
  DBUG_ASSERT(m_durations[MDL_STATEMENT].m_mat_front == nullptr);

  m_durations[MDL_TRANSACTION].m_ticket_list.swap(
      m_durations[MDL_EXPLICIT].m_ticket_list);

  m_durations[MDL_EXPLICIT].m_mat_front = nullptr;
  List_iterator it_ticket(m_durations[MDL_EXPLICIT].m_ticket_list);

  while ((ticket = it_ticket++)) {
    m_durations[MDL_EXPLICIT].m_ticket_list.remove(ticket);
    m_durations[MDL_TRANSACTION].m_ticket_list.push_front(ticket);
  }
  /*
    Note that we do not update
    m_durations[MDL_TRANSACTION].m_mat_front here. That is ok, since
    it is only to be used as an optimization for
    MDL_context::materialize_fast_path_locks(). So if the tickets
    being added are already materialized it does not break an
    invariant, and m_mat_front will be updated the next time
    MDL_context::materialize_fast_path_locks() runs.
  */

#ifndef DBUG_OFF
  List_iterator trans_it(m_durations[MDL_TRANSACTION].m_ticket_list);

  while ((ticket = trans_it++)) {
    ticket->set_duration(MDL_TRANSACTION);
  }
#endif

  // Must also change duration in map
  if (m_map != nullptr) {
    for (auto &p : *m_map) {
      p.second.m_dur = MDL_TRANSACTION;
    }
  }
}

MDL_ticket_store::MDL_ticket_handle MDL_ticket_store::find(
    const MDL_request &req) const {
#ifndef DBUG_OFF
  if (m_count >= THRESHOLD) {
    MDL_ticket_handle list_h = find_in_lists(req);
    MDL_ticket_handle hash_h = find_in_hash(req);

    DBUG_ASSERT(equivalent(list_h.m_ticket, hash_h.m_ticket, req.duration));
  }
#endif /*! DBUG_OFF */
  return (m_map == nullptr || m_count < THRESHOLD) ? find_in_lists(req)
                                                   : find_in_hash(req);
}

void MDL_ticket_store::set_materialized() {
  for (auto &d : m_durations) {
    d.m_mat_front = d.m_ticket_list.front();
  }
}

MDL_ticket *MDL_ticket_store::materialized_front(int di) {
  return m_durations[di].m_mat_front;
}

String timeout_message(const char *command, const char *name1,
                       const char *name2) {
  String msg;
  msg.append("Timeout on ");
  msg.append(command);
  if ((name1 && name1[0]) || (name2 && name2[0])) {
    msg.append(": ");
    msg.append(name1);
  }
  if (name2 && name2[0]) {
    msg.append(".");
    msg.append(name2);
  }
  return msg;
}
