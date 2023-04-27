/* Copyright (c) 2017, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef LOG_BUILTINS_FILTER_H
#define LOG_BUILTINS_FILTER_H

#include <mysql/components/component_implementation.h>
#include <mysql/components/my_service.h>
#include <mysql/components/service_implementation.h>
#include <mysql/components/services/log_shared.h>

#include "rwlock_scoped_lock.h"

/*
  services: log item filters
*/

typedef enum enum_log_filter_verb {
  LOG_FILTER_UNDEF = 0,    /**< not set */
  LOG_FILTER_DROP = 1,     /**< drop this log event */
  LOG_FILTER_THROTTLE = 2, /**< rate-limit this line class */
  LOG_FILTER_ITEM_SET = 3, /**< add field */
  LOG_FILTER_ITEM_DEL = 4, /**< remove field */
  LOG_FILTER_RETURN = 8,   /**< stop filtering, run (next filter or) sinks */

  LOG_FILTER_CHAIN_AND = 4096, /**< no verb yet, part of a condition chain */
  LOG_FILTER_CHAIN_OR = 4097   /**< no verb yet, part of a condition chain */
} log_filter_verb;

typedef enum enum_log_filter_cond {
  LOG_FILTER_COND_NONE = 0,    /**< not set / unconditional */
  LOG_FILTER_COND_EQ = 1,      /**< equal */
  LOG_FILTER_COND_NE = 2,      /**< not equal */
  LOG_FILTER_COND_LT = 3,      /**< less than */
  LOG_FILTER_COND_LE = 4,      /**< less or equal */
  LOG_FILTER_COND_GE = 5,      /**< greater or equal */
  LOG_FILTER_COND_GT = 6,      /**< greater than */
  LOG_FILTER_COND_PRESENT = 7, /**< field present */
  LOG_FILTER_COND_ABSENT = 8,  /**< field absent */
} log_filter_cond;

/**
  Note that if our condition requires absence of the key, and the
  key does not exist, that constitutes success!
*/
typedef enum enum_log_filter_match {
  /** success */
  LOG_FILTER_MATCH_SUCCESS = 0,
  /** failure */
  LOG_FILTER_MATCH_UNSATISFIED = 1,
  /** don't know yet */
  LOG_FILTER_MATCH_UNCOMPARED = 2,
  /** invalid value */
  LOG_FILTER_MATCH_COMPARATOR_UNKNOWN = 4,
  /** either both or neither operands must be strings */
  LOG_FILTER_MATCH_CLASSES_DIFFER = 8,
  /** comparator does not exist for this type (yet) */
  LOG_FILTER_MATCH_UNSUPPORTED_FOR_TYPE = 16
} log_filter_match;

typedef enum enum_log_filter_apply {
  /** all's well that applies well */
  LOG_FILTER_APPLY_SUCCESS = 0,
  /** invalid argument, e.g. "keep -3 characters of this string" */
  LOG_FILTER_APPLY_ARGUMENT_OUT_OF_RANGE = 1,
  /** log line does not accept further items */
  LOG_FILTER_APPLY_ITEM_BAG_FULL = 2,
  /** variant of malloc() failed */
  LOG_FILTER_APPLY_OUT_OF_MEMORY = 4,
  /** action/verb not known/implement by this filter */
  LOG_FILTER_APPLY_UNKNOWN_OPERATION = 8,
  /** had a match for the condition/comparator, but not for the action/verb */
  LOG_FILTER_APPLY_TARGET_NOT_IN_LOG_LINE = 16
} log_filter_apply;

typedef enum enum_log_filter_flags {
  /** normal state */
  LOG_FILTER_FLAG_NONE = 0,
  /** used to identify rules that don't come from the rule engine, but were
      injected by the server for emulation of legacy log_error_verbosity etc. */
  LOG_FILTER_FLAG_SYNTHETIC = 1,
  /** rule temporarily disabled */
  LOG_FILTER_FLAG_DISABLED = 2
} log_filter_flags;

typedef struct _log_filter_tag {
  const char *filter_name; /**< name of the service that created this rule */
  void *filter_data;       /**< for ad lib use by said service. instance etc. */
} log_filter_tag;

typedef struct _log_filter_rule {
  ulong id;             /**< index may change; this will not */
  ulong jump;           /**< default: 0.
                             action of an IF/ELSEIF/ELSE branch:
                               points behind last branch of this
                               IF-construct (we successfully matched
                               a condition, and executed its statement,
                               so leave this IF-construct as nothing
                               else will apply).
                             condition of an IF/ELSEIF:
                               points to next ELSEIF/ELSE, so we can
                               jump there if the condition fails.
                        */
  log_item match;       /**< compare to this item type/class/key/value etc */
  log_filter_cond cond; /**< how to compare: < > == regex etc */
  log_filter_verb verb; /**< what to do: drop, upvote, etc */
  log_item aux;         /**< aux: item to add/prio to set/throttle rate */

  // private state keeping

  /** for rate-limiting: at what time will current window end? */
  ulonglong throttle_window_end;
  /** for rate-limiting: window-size (in seconds) */
  ulong throttle_window_size;
  /** how many lines in this window matched? (both suppressed and not) */
  ulong throttle_matches;

  /** log_filter_flags (fix storage size) */
  ulong flags;

  /** how often did this rule match? */
  volatile int32 match_count;

  /** lock an individual rule (to update state keeping) */
  mysql_rwlock_t rule_lock;
} log_filter_rule;

#define LOG_FILTER_RULE_MAX 512

typedef struct _log_filter_ruleset {
  /** creator of this rule */
  log_filter_tag *tag;

  /** number of rules currently in ruleset */
  uint32 count;
  /** maximum number of rules in this ruleset */
  uint32 alloc;
  /** rules in this ruleset */
  log_filter_rule rule[LOG_FILTER_RULE_MAX];
  /**
    lock for whole ruleset.

    - get a read lock to apply filter rules to event
      - if a rule changes rule-internal state, like the occurrence count
        for throttle, it must upgrade to write lock
        => technically, internal state could use a per-rule lock,
           but let's keep things simple for now
    - changing the filter ruleset obviously also needs a write lock
  */
  mysql_rwlock_t ruleset_lock;
} log_filter_ruleset;

typedef enum enum_log_builtins_lock {
  LOG_BUILTINS_LOCK_NONE = 0,     /**< undefined */
  LOG_BUILTINS_LOCK_SHARED = 1,   /**< read-only lock */
  LOG_BUILTINS_LOCK_EXCLUSIVE = 2 /**< read-write lock */
} log_builtins_filter_lock;

BEGIN_SERVICE_DEFINITION(log_builtins_filter)
// run built-in filter, get/set its configuration

/**
  Create a new set of filter rules.

  @param   tag       tag for this ruleset
  @param   count     number of rules to allocate

  @retval            a pointer to a ruleset structure, or nullptr on failure
*/
DECLARE_METHOD(log_filter_ruleset *, filter_ruleset_new,
               (log_filter_tag * tag, size_t count));

/**
  Lock and get the filter rules.

  @param  ruleset    a ruleset (usually allocated with filter_ruleset_new())
  @param  locktype   LOG_BUILTINS_LOCK_SHARED     lock for reading
                     LOG_BUILTINS_LOCK_EXCLUSIVE  lock for writing

  @retval  0         lock acquired
  @retval !0         failed to acquire lock
*/
DECLARE_METHOD(int, filter_ruleset_lock,
               (log_filter_ruleset * ruleset,
                log_builtins_filter_lock locktype));

/**
  Release lock on filter rules.

  @param  ruleset    a ruleset (usually allocated with filter_ruleset_new())
*/
DECLARE_METHOD(void, filter_ruleset_unlock, (log_filter_ruleset * ruleset));

/**
  Drop an entire filter rule-set. Must hold lock.

  @param  ruleset    a ruleset * (usually allocated with filter_ruleset_new())
*/
DECLARE_METHOD(void, filter_ruleset_drop, (log_filter_ruleset * ruleset));

/**
  Free an entire filter rule-set. Must hold lock. Lock will be destroyed.

  @param  ruleset    a ruleset * (usually allocated with filter_ruleset_new())
                     the pointer pointed to will be a nullptr on return.
*/
DECLARE_METHOD(void, filter_ruleset_free, (log_filter_ruleset * *ruleset));

/**
  Move rules from one ruleset to another. Origin will be empty afterwards.

  @param  from   source      ruleset
  @param  to     destination ruleset
*/
DECLARE_METHOD(int, filter_ruleset_move,
               (log_filter_ruleset * from, log_filter_ruleset *to));

/**
  Initialize a new rule.
  This clears the first unused rule. It does not update the rules
  count; this is for the caller to do if it succeeds in setting up
  the rule to its satisfaction. If the caller fails, it should
  log_builtins_filter_rule_free() the incomplete rule.

  @param   ruleset  a ruleset (usually allocated with filter_ruleset_new())

  @return  nullptr: could not initialize rule. Do not call rule_free.
          !nullptr: the address of the rule. fill in. on success,
                    caller must increase rule count.  on failure,
                    it must call rule_free.
*/
DECLARE_METHOD(void *, filter_rule_init, (log_filter_ruleset * ruleset));

/**
  Apply all matching rules from a filter rule set to a given log line.

  @param           ll                   the current log line

  @retval          int                  number of matched rules
*/
DECLARE_METHOD(int, filter_run, (log_filter_ruleset * ruleset, log_line *ll));

END_SERVICE_DEFINITION(log_builtins_filter)

/**
  Temporary primitives for logging services.
*/
BEGIN_SERVICE_DEFINITION(log_builtins_filter_debug)
DECLARE_METHOD(log_filter_ruleset *, filter_debug_ruleset_get, (void));
END_SERVICE_DEFINITION(log_builtins_filter_debug)

#endif
