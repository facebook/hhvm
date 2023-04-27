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

/**
  services: log filter: basic filtering

  This implementation, "dragnet" is currently the default filter
  and therefore built-in.  Basic configuration is built into the
  server proper (via log_error_verbosity etc.); for advanced configuration,
  load the service log_filter_dragnet which implements a configuration
  language for this engine.  See there for details about the Configuration
  Stage.  Some of the code-paths are only available via the configuration
  language or an equivalent service (but not without any such service loaded).

  At present, the design is such that multiple threads can call the
  filter concurrently; the ruleset is global and shared between all
  users.


  FILTERING STAGE

  At run time, the filter iterates over its rule-set.  For each
  rule, if the condition contains a well-known item, it looks for
  an item of that type in the event.  If the condition contains
  an ad hoc-item, it looks for an item of any ad hoc-type with
  the given key within the event.

  If there is a match, the filter will verify whether the storage
  class of the value in the event and that in the condition are
  either both strings, or both not.  If the classes do not match,
  it flags an error.  Otherwise, it now compares both values using
  the requested comparator, and reports the result.

  If a log event matches a rule, an action ("suppress log line",
  "delete field", etc.) will be applied to that event.


  LOCKING

  During the filtering stage, a shared lock on the ruleset is held.
  An exclusive lock on the ruleset is only taken as response to the
  user's changing of the filter configuration, which should be rare.

  For debugging puroposes, rules feature a counter of how often events
  matched them; this counter is updated atomically.

  Rate-limiting ("throttle") needs some bookkeeping data (when does
  the current window expire? how many matches have we had so far
  within the current window? etc.). A write-lock is taken on the
  individual rule (not the entire ruleset) to update this information;
  any throttling-related actions taken on the event happen after this
  lock has been released.

  The event itself is not locked.
*/

#include <mysqld_error.h>

#include "log_builtins_filter_imp.h"
#include "log_builtins_imp.h"
#include "my_atomic.h"
#include "my_systime.h"  // my_micro_time()
#include "mysys_err.h"   // EE_ERROR_LAST for globerrs
#include "sql/derror.h"
#include "sql/log.h"
// for the default rules
#include "sql/mysqld.h"

#define THROTTLE_DEFAULT_WINDOW_SIZE_IN_SECONDS 60
#define THROTTLE_MICROSECOND_MULTIPLIER 1000000

static bool filter_inited = false;
static ulong filter_rule_uuid = 0;

log_filter_ruleset *log_filter_builtin_rules = nullptr;
log_filter_tag rule_tag_builtin = {"log_filter_builtin", nullptr};

/**
  Predicate: can we add any more rules?

  @param  rs     the ruleset to check

  @retval true   full, no more rules can be added
  @retval false  not full, further rules can be added
*/
static bool log_filter_ruleset_full(log_filter_ruleset *rs) {
  return (rs->count >= LOG_FILTER_RULE_MAX);
}

/**
  Initialize a new rule.

  This clears the first unused rule. It does not update the rules
  count; this is for the caller to do if it succeeds in setting up
  the rule to its satisfaction. If the caller fails, it should
  log_builtins_filter_rule_free() the incomplete rule.

  @retval  nullptr: could not initialize rule. Do not call rule_free.
  @retval !nullptr: the address of the rule. fill in. on success,
                    caller must increase rule count.  on failure,
                    it must call rule_free.
*/
static log_filter_rule *log_builtins_filter_rule_init(
    log_filter_ruleset *ruleset) {
  log_filter_rule *r = &ruleset->rule[ruleset->count];

  memset(r, 0, sizeof(log_filter_rule));

  r->id = ++filter_rule_uuid;
  r->throttle_window_size =
      THROTTLE_DEFAULT_WINDOW_SIZE_IN_SECONDS;  // 1 minute

  if (mysql_rwlock_init(0, &(r->rule_lock))) return nullptr;

  return r;
}

/**
  Release all resources associated with a filter rule.
  Leaves a "gap" (an uninitialized rule) for immediate re-filling;
  if this is undesired, use log_builtins_filter_rule_remove() (see there).
  Must hold rule-set lock.

  @param  ri  the rule to release

  @retval     the return value from mysql_rwlock_destroy()
*/
static int log_builtins_filter_rule_free(log_filter_rule *ri) {
  ri->cond = LOG_FILTER_COND_NONE;
  ri->verb = LOG_FILTER_UNDEF;

  // release memory if needed
  log_item_free(&(ri->match));
  log_item_free(&(ri->aux));

  return mysql_rwlock_destroy(&(ri->rule_lock));
}

/**
  Release filter rule (key/value pair) with the index "elem" in "ruleset".
  This frees whichever of key and value were dynamically allocated.
  It then moves any trailing items to fill the "gap" and decreases the
  counter of elements in the rule-set.

  If the intention is to leave a "gap" in the bag that may immediately be
  overwritten with an updated element, use log_builtins_filter_rule_free()
  instead.

  Caller must hold rule-set lock.

  @param         ruleset   filter rule-set
  @param         elem      index of the filter rule to release
*/
static void log_builtins_filter_rule_remove(log_filter_ruleset *ruleset,
                                            int elem) {
  size_t rn;

  DBUG_ASSERT(ruleset->count > 0);

  log_builtins_filter_rule_free(&ruleset->rule[elem]);

  for (rn = elem; rn < (ruleset->count - 1); rn++) {
    ruleset->rule[rn] = ruleset->rule[rn + 1];
  }

  ruleset->count--;
}

/**
   Create a new set of filter rules.

   @param   tag       tag for this ruleset
   @param   count     number of rules to allocate, 0 for default

   @retval            a pointer to a ruleset structure, or nullptr on failure
*/
static log_filter_ruleset *log_builtins_filter_ruleset_new(log_filter_tag *tag,
                                                           size_t count) {
  log_filter_ruleset *ruleset = nullptr;

  if ((tag == nullptr) || (tag->filter_name == nullptr))
    return nullptr; /* purecov: inspected */

  ruleset =
      (log_filter_ruleset *)my_malloc(0, sizeof(log_filter_ruleset), MYF(0));

  if (ruleset != nullptr) {
    memset(ruleset, 0, sizeof(log_filter_ruleset));
    ruleset->tag = tag;
    ruleset->alloc = (count < 1) ? LOG_FILTER_RULE_MAX : count;

    if (mysql_rwlock_init(0, &ruleset->ruleset_lock)) {
      my_free((void *)ruleset); /* purecov: inspected */
      ruleset = nullptr;        /* purecov: inspected */
    }
  }

  return ruleset;
}

/**
  Lock and get the filter rules.

  @param  ruleset  the ruleset to lock
  @param  lt       LOG_BUILTINS_LOCK_SHARED     lock for reading
                   LOG_BUILTINS_LOCK_EXCLUSIVE  lock for writing

  @retval  0       lock acquired
  @retval !0       failed to acquire lock
*/
static int log_builtins_filter_ruleset_lock(log_filter_ruleset *ruleset,
                                            log_builtins_filter_lock lt) {
  if ((!filter_inited) || (ruleset == nullptr)) return -1;

  switch (lt) {
    case LOG_BUILTINS_LOCK_SHARED:
      mysql_rwlock_rdlock(&ruleset->ruleset_lock);
      break;
    case LOG_BUILTINS_LOCK_EXCLUSIVE:
      mysql_rwlock_wrlock(&ruleset->ruleset_lock);
      break;
    default:
      return -2; /* purecov: inspected */
  }

  return 0;
}

/**
  Drop an entire filter rule-set. Must hold lock.
*/
static void log_builtins_filter_ruleset_drop(log_filter_ruleset *ruleset) {
  log_filter_rule *ri;

  while (ruleset->count > 0) {
    ruleset->count--;
    ri = &ruleset->rule[ruleset->count];
    log_builtins_filter_rule_free(ri);
  }
}

/**
  Unlock filter ruleset.
*/
static void log_builtins_filter_ruleset_unlock(log_filter_ruleset *ruleset) {
  if (ruleset != nullptr) {
    mysql_rwlock_unlock(&(ruleset->ruleset_lock));
  }
}

/**
  Free filter ruleset.
*/
static void log_builtins_filter_ruleset_free(log_filter_ruleset **ruleset) {
  if (ruleset != nullptr) {
    log_filter_ruleset *rs = *ruleset;
    if (rs != nullptr) {
      *ruleset = nullptr;

      log_builtins_filter_ruleset_drop(rs);
      log_builtins_filter_ruleset_unlock(rs);

      mysql_rwlock_destroy(&(rs->ruleset_lock));

      my_free((void *)rs);
    }
  }
}

/**
  Defaults for when the configuration engine isn't loaded;
  aim for 5.7 compatibilty.
*/
static void log_builtins_filter_set_defaults(log_filter_ruleset *ruleset) {
  log_filter_rule *r;

  DBUG_ASSERT(ruleset != nullptr);

  DBUG_ASSERT(!log_filter_ruleset_full(ruleset));

  // failsafe: simple built-in filter may not drop "System" or "Error" messages
  r = log_builtins_filter_rule_init(ruleset);
  log_item_set_with_key(&r->match, LOG_ITEM_LOG_PRIO, nullptr,
                        LOG_ITEM_FREE_NONE)
      ->data_integer = WARNING_LEVEL;
  r->cond = LOG_FILTER_COND_LT;
  r->verb = LOG_FILTER_RETURN;

  ruleset->count++;

  // sys_var: log_error_verbosity
  r = log_builtins_filter_rule_init(ruleset);
  log_item_set_with_key(&r->match, LOG_ITEM_LOG_PRIO, nullptr,
                        LOG_ITEM_FREE_NONE)
      ->data_integer = log_error_verbosity;
  r->cond = LOG_FILTER_COND_GT;
  r->verb = LOG_FILTER_DROP;

  ruleset->count++;

  // example: remove all source-line log items
  // these are not desirable by default, only while debugging.
  r = log_builtins_filter_rule_init(ruleset);
  log_item_set(&r->match, LOG_ITEM_SRC_LINE);
  r->cond = LOG_FILTER_COND_PRESENT;
  r->verb = LOG_FILTER_ITEM_DEL;
  // aux optional

  ruleset->count++;
}

/**
  Deinitialize filtering engine.

  @retval  0   Success!
  @retval -1   De-initialize? Filter wasn't even initialized!
*/
int log_builtins_filter_exit() {
  if (!filter_inited) return -1;

  /*
    Nobody else should run at this point anyway, but
    since we made a big song and dance about having
    to hold this lock above ...
  */
  if (log_filter_builtin_rules != nullptr) {
    mysql_rwlock_wrlock(&log_filter_builtin_rules->ruleset_lock);
    filter_inited = false;
    log_builtins_filter_ruleset_free(&log_filter_builtin_rules);
  } else
    filter_inited = false; /* purecov: inspected */

  return 0;
}

/**
  Initialize filtering engine.
  We need to do this early, before the component system is up.

  @retval  0   Success!
  @retval -1   Couldn't initialize ruleset
  @retval -2   Filter was already initialized?
*/
int log_builtins_filter_init() {
  if (!filter_inited) {
    log_filter_builtin_rules =
        log_builtins_filter_ruleset_new(&rule_tag_builtin, 0);
    if (log_filter_builtin_rules == nullptr) return -1;

    log_builtins_filter_set_defaults(log_filter_builtin_rules);
    filter_inited = true;

    return 0;
  } else
    return -2;
}

/**
  Apply the action of an individual rule to an individual log line (or
  a part thereof, i.e. a "field"). At this point, we already know that
  the current log line matches the condition.

  @param[in,out]   ll                   the current log line
  @param           ln                   index of the matching field,
                                        -1 for none (when a test for absence
                                        matched)
  @param           r                    the rule to apply. internal state may
                                        be changed (i.e. the number of seen
                                        matches for a throttle rule)

  @retval          log_filter_apply     0 on success, an error-code otherwise
*/

static log_filter_apply log_filter_try_apply(log_line *ll, int ln,
                                             log_filter_rule *r) {
  switch (r->verb) {
    case LOG_FILTER_RETURN:
      break;

    case LOG_FILTER_DROP:
      log_line_item_free_all(ll);
      break;

    case LOG_FILTER_THROTTLE: {
      ulonglong now = my_micro_time();
      ulong rate = (ulong)(
          (r->aux.data.data_integer < 0) ? 0 : r->aux.data.data_integer);
      ulong suppressed = 0;
      ulong matches;

      /*
        Check whether we're still in the current window. (If not, we
        will want to print a summary (if the logging of any lines was
        suppressed) and start a new window.)
      */
      mysql_rwlock_wrlock(&(r->rule_lock));

      if (now >= r->throttle_window_end) {
        suppressed =
            (r->throttle_matches > rate) ? (r->throttle_matches - rate) : 0;

        // new window
        r->throttle_matches = 0;
        r->throttle_window_end =
            now + (r->throttle_window_size * THROTTLE_MICROSECOND_MULTIPLIER);
      }

      matches = ++r->throttle_matches;

      mysql_rwlock_unlock(&(r->rule_lock));

      /*
        If it's over the limit for the current window, discard the whole
        log line.  A rate of 0 ("discard all") is allowed for convenience,
        but the DROP verb should be used instead.
      */
      if (matches > rate) {
        log_line_item_free_all(ll);
      }

      // if we actually suppressed any lines, add info declaring that
      else if ((suppressed > 0) && !log_line_full(ll)) {
        log_line_item_set(ll, LOG_ITEM_LOG_SUPPRESSED)->data_integer =
            suppressed;
      }
    } break;

    case LOG_FILTER_ITEM_SET:
      if (r->aux.key == nullptr) return LOG_FILTER_APPLY_TARGET_NOT_IN_LOG_LINE;

      // if not same field as in match, look for it
      if ((ln < 0) || (0 != native_strcasecmp(r->aux.key, ll->item[ln].key)))
        ln = log_line_index_by_item(ll, &r->aux);

      if (ln >= 0)  // found? release for over-write!
        log_item_free(&ll->item[ln]);
      else if (log_line_full(ll))  // otherwise, add it if there's still space
        return LOG_FILTER_APPLY_OUT_OF_MEMORY;
      else
        ln = ll->count++;

      ll->item[ln] = r->aux;
      /*
        It's a shallow copy, don't try to free it.
        As a downside, the filter rules must remain shared-locked until
        the line is logged. The assumption is that logging happens vastly
        more than changing of the ruleset.
      */
      ll->item[ln].alloc = LOG_ITEM_FREE_NONE;
      ll->seen |= r->aux.type;
      break;

    case LOG_FILTER_ITEM_DEL:
      // might want to delete a field other than that from the cond:
      if ((r->aux.key != nullptr) &&
          ((ln < 0) ||
           (0 != native_strcasecmp(r->aux.key, ll->item[ln].key)))) {
        ln = log_line_index_by_item(ll, &r->aux);
      }

      if (ln < 0) return LOG_FILTER_APPLY_TARGET_NOT_IN_LOG_LINE;

      {
        log_item_type t = ll->item[ln].type;

        log_line_item_remove(ll, ln);

        /*
          If it's a well-known type (and therefore unique), or if it's
          the last one of a generic type, unflag the presence of that type.
        */
        if (!log_item_generic_type(t) || (log_line_index_by_type(ll, t) < 0))
          ll->seen &= ~t;
      }

      break;

    default:
      return LOG_FILTER_APPLY_UNKNOWN_OPERATION;
  }

  return LOG_FILTER_APPLY_SUCCESS;
}

/**
  Try to match an individual log line-field against an individual
  rule's condition

  @param           li                   the log item we try to match
  @param           ri                   the rule containing the condition

  @retval          log_filter_match     0 (LOG_FILTER_MATCH_SUCCESS) on match,
                                        1 (LOG_FILTER_MATCH_UNSATISFIED) or
                                        an error-code otherwise
*/
static log_filter_match log_filter_try_match(log_item *li,
                                             log_filter_rule *ri) {
  bool rc, lc;
  log_filter_match e = LOG_FILTER_MATCH_UNCOMPARED;

  /*
    If a condition is e.g.  prio > 4,
    - the "4" is part of the filter-rule (rule-item "ri")
    - the "prio" will be substitute with that field's value
      in the log-event (the corresponding log-item, "li")

    I.e. we're lucky in that as mnemonics go, the "l" in
    "lc", "lf", "li" can stand for both "log-event" and
    "left in the comparison", and as can the "r" for both
    "rule" and "right in the comparison."
  */

  DBUG_ASSERT(ri != nullptr);

  /*
    If there is no match, the only valid scenarios are "success"
    (if we tested for absence), or failure (otherwise).  Handle
    them here to make any derefs of li beyond this point safe.
  */
  if (li == nullptr)
    return (ri->cond == LOG_FILTER_COND_ABSENT) ? LOG_FILTER_MATCH_SUCCESS
                                                : LOG_FILTER_MATCH_UNSATISFIED;

  else if (ri->cond == LOG_FILTER_COND_PRESENT)
    return LOG_FILTER_MATCH_SUCCESS;

  else if (ri->cond == LOG_FILTER_COND_ABSENT)
    return LOG_FILTER_MATCH_UNSATISFIED;

  // item class on left hand side / right hand side
  rc = log_item_string_class(ri->match.item_class);
  lc = log_item_string_class(li->item_class);

  // if one's a string and the other isn't, fail for now
  if (rc != lc)
    e = LOG_FILTER_MATCH_CLASSES_DIFFER;

  else {
    e = LOG_FILTER_MATCH_UNSATISFIED;
    double rf, lf;

    // we're comparing two strings
    if (rc) {
      /*
        We're setting up lf to be the result of our string-comparison
        (<0, 0, >0), and rf to be 0.  This allows us to use all the
        numerical comparators below that compare lf and rf.
      */

      rf = 0;

      // some ado in case the strings aren't \0 terminated
      size_t len;  // length of the shorter string
      len = (ri->match.data.data_string.length < li->data.data_string.length)
                ? ri->match.data.data_string.length
                : li->data.data_string.length;

      // compare for the shorter of the given lengths
      lf = log_string_compare(li->data.data_string.str,
                              ri->match.data.data_string.str, len,
                              false);  // case-sensitive

      /*
        If strings are the same for the shared length,
        but are of different length, longer string "wins"
      */
      if ((lf == 0) &&
          (ri->match.data.data_string.length != li->data.data_string.length))
        lf = (li->data.data_string.length > len) ? 1 : -1;
    }

    // we're comparing numerically
    else {
      // get values as floats (works for integer items as well)
      log_item_get_float(&ri->match, &rf);
      log_item_get_float(li, &lf);
    }

    // do the actual comparison
    switch (ri->cond) {
      case LOG_FILTER_COND_EQ:  // ==  Equal
        if (lf == rf) e = LOG_FILTER_MATCH_SUCCESS;
        break;

      case LOG_FILTER_COND_NE:  // !=  Not Equal
        if (lf != rf) e = LOG_FILTER_MATCH_SUCCESS;
        break;

      case LOG_FILTER_COND_GE:  // >=  Greater or Equal
        if (lf >= rf) e = LOG_FILTER_MATCH_SUCCESS;
        break;

      case LOG_FILTER_COND_LT:  // <   Less Than
        if (lf < rf) e = LOG_FILTER_MATCH_SUCCESS;
        break;

      case LOG_FILTER_COND_LE:  // <=  Less or Equal
        if (lf <= rf) e = LOG_FILTER_MATCH_SUCCESS;
        break;

      case LOG_FILTER_COND_GT:  // >   Greater than
        if (lf > rf) e = LOG_FILTER_MATCH_SUCCESS;
        break;

        // unknown comparison type
      default:
        e = LOG_FILTER_MATCH_COMPARATOR_UNKNOWN; /* purecov: inspected */
        DBUG_ASSERT(false);
    }  // comparator switch
  }    // class mismatch?

  return e;
}

/**
  Apply all matching rules from a filter rule set to a given log line.

  @param           ruleset              rule-set to apply
  @param           ll                   the current log line

  @retval          int                  number of matched rules
*/
int log_builtins_filter_run(log_filter_ruleset *ruleset, log_line *ll) {
  size_t rn;           // rule     number
  int ln = -1;         // log-item number
  log_filter_rule *r;  // current  rule
  int processed = 0;
  log_filter_match cond_result;

  DBUG_ASSERT(filter_inited);

  if (ruleset == nullptr) return 0; /* purecov: inspected */

  mysql_rwlock_rdlock(&ruleset->ruleset_lock);

  for (rn = 0; ((rn < ruleset->count) && (ll->seen != LOG_ITEM_END)); rn++) {
    r = &ruleset->rule[rn];

    /*
      If the rule is temporarily disabled, skip over it.
      If and when LOG_FILTER_CHAIN_AND/LOG_FILTER_CHAIN_OR
      are added, those chained conditions must be muted/unmuted
      along with the first one, i.e. as a group.
    */
    if (r->flags & LOG_FILTER_FLAG_DISABLED) continue;

    /*
      Look for a matching field in the event!
    */

    /*
      Currently applies to 0 or 1 match, there is no multi-match.
    */

    if (r->cond == LOG_FILTER_COND_NONE)  // in ELSE etc. there is no condition
    {
      /*
        We could just initialize this at a top and then chain this
        (i.e. rules with conditions will overwrite this; while rules
        without (i.e. ELSE) will "inherit" the previous value, but we
        don't currently have to as implicit-DELETE-field is set up at
        parse-time, not at execution time. For that reason, we always
        reset this for the time being so people will find the code
        easier to understand.
      */
      ln = -1;
      goto apply_action;
    }

    ln = log_line_index_by_item(ll, &r->match);

    /*
      If we found a suitable field, see whether its value satisfies
      the condition given in the rule.  If so, apply the action.

      ln == -1  would indicate "not found", which we can actually
      match against for cases like, "if one doesn't exist, create one now."
    */
    cond_result = log_filter_try_match((ln >= 0) ? &ll->item[ln] : nullptr, r);

    if (cond_result == LOG_FILTER_MATCH_SUCCESS) {
      /*
        Protect match_count in case of non-atomic updates.
      */
      mysql_rwlock_wrlock(&(r->rule_lock));
      ++r->match_count;
      mysql_rwlock_unlock(&(r->rule_lock));

      if (r->verb == LOG_FILTER_CHAIN_AND)  // AND -- test next condition
        continue;                           // proceed with next cond

      else if (r->verb == LOG_FILTER_CHAIN_OR)  // OR -- one match is enough
      {  // skip any other conditions in OR
        while (ruleset->rule[rn].verb == LOG_FILTER_CHAIN_OR) rn++;
        r = &ruleset->rule[rn];
      }
      /*
        If we're here, we either had a single-condition IF match,
        or one condition in an OR-chain matched.
        In either case, it's time to apply the verb now.
      */
    } else if (cond_result == LOG_FILTER_MATCH_UNSATISFIED) {
      if (r->verb ==
          LOG_FILTER_CHAIN_AND) {  // jump to next branch (ELSEIF/ELSE/ENDIF)
        while (ruleset->rule[++rn].verb == LOG_FILTER_CHAIN_AND)
          ;  // skip over all AND conditions
      }
      /*
        skip over rule:
        - if this was an AND-chain,  it'll skip over last cond and the action
        - if this was an OR-chain,   it'll proceed to the next condition
        - if this was a solitary IF, it'll skip the action
      */
      continue;  // skip over last condition (the one with the actual action)
    } else       // misc. failures (type mismatch etc.)
      continue;

  apply_action:
    if (log_filter_try_apply(ll, ln, r) == LOG_FILTER_APPLY_SUCCESS) {
      processed++;

      if (r->verb == LOG_FILTER_RETURN) goto done;
    }

    if (r->jump != 0)  // we're at the end of a block; jump to ENDIF
      rn += r->jump - 1;
  }

done:
  mysql_rwlock_unlock(&ruleset->ruleset_lock);

  return processed;
}

/**
  This is part of the 5.7 emulation:
  If --log_error_verbosity is changed, we generate an
  artificial filter rule from it here.

  For this filtering to be active, @@global.log_error_services
  has to feature "log_filter_internal", as it does by default.
  When that is the case, one or both of log_error_verbosity and
  log_error_suppression_list (see below) may be used.
  Only one of "log_filter_internal" and "log_filter_dragnet"
  should be used at a time.

  @param verbosity  log_error_verbosity style, range(1,4)
                    1:errors,   2:+=warnings,  3:+=notes,  4:+=debug

  @retval            0: success
  @retval           !0: failure
*/
int log_builtins_filter_update_verbosity(int verbosity) {
  size_t rn;
  log_filter_rule *r;
  int rr = -99;

  if (log_builtins_filter_ruleset_lock(log_filter_builtin_rules,
                                       LOG_BUILTINS_LOCK_EXCLUSIVE) < 0)
    return -1;

  /*
    If a log_error_verbosity item already exists, update it.
    This should always be the case now since we create an item on start-up
    and different filter components can keep their rule-sets separate.
  */
  for (rn = 0; (rn < log_filter_builtin_rules->count); rn++) {
    r = &log_filter_builtin_rules->rule[rn];

    if ((r->match.type == LOG_ITEM_LOG_PRIO) && (r->verb == LOG_FILTER_DROP) &&
        (r->cond == LOG_FILTER_COND_GT)) {
      r->match.data.data_integer = verbosity;
      r->flags &= ~LOG_FILTER_FLAG_DISABLED;
      rr = 0;
      goto done;
    }
  }

  /* purecov: begin deadcode */

  // if no log_error_verbosity item already exists, create one
  if (log_filter_ruleset_full(
          log_filter_builtin_rules)) { /* since this is a private rule-set, this
                                          should never happen */
    rr = -2;
    goto done;
  }

  r = log_builtins_filter_rule_init(log_filter_builtin_rules);

  log_item_set_with_key(&r->match, LOG_ITEM_LOG_PRIO, nullptr,
                        LOG_ITEM_FREE_NONE)
      ->data_integer = verbosity;
  r->cond = LOG_FILTER_COND_GT;
  r->verb = LOG_FILTER_DROP;

  log_filter_builtin_rules->count++;

  rr = 1; /* purecov: end */

done:
  log_builtins_filter_ruleset_unlock(log_filter_builtin_rules);

  return rr;
}

/**
  @@global.log_error_suppression_list accepts a comma-separated
  list of error-codes that should not be included in the error-log.
  Events with a severity of System or Error can not be filtered
  in this way and will always be forwarded to the log-sinks.

  This provides simple filtering for cases where the flexibility
  of the loadable filter-language is not needed. (The same engine
  is used however, just with a more limited interface.)

  For this filtering to be active, @@global.log_error_services has
  to feature "log_filter_internal", as it does by default. When that
  is the case, one or both of log_error_verbosity and this variable
  may be used. Only one of "log_filter_internal" and "log_filter_dragnet"
  should be used at a time.

  The semantics follow that of our system variables; that is to say,
  when called with update==false, the function acts as a check-function
  that validates the entire list given to it; when called with
  update==true, it creates filter-rules from the list items. This way,
  we either create all rules, or no rules, rather than ending up with
  an incomplete rule-set when we encounter a problem in the input.

  The return value encodes the location in the argument where the
  failure occurred, like so:
  - if 0 is returned, no issues were detected
  - if a value less than zero is returned, -(retval + 1) is the
    byte position (counting from 0) in the list argument at
    which point the failure was detected

  @param   list       list of error-codes that should not appear
                      in the error-log
  @param   update     false: verify list only
                      true:  create filtering rules from suppression list

  @retval              0: success
  @retval             !0: failure (see above)
*/
int log_builtins_filter_parse_suppression_list(char *list, bool update) {
  char symbol[80];      // error to suppress ("1234" or "MY-001234")
  char *start = list;   // start of token
  char *end = nullptr;  // end of token
  size_t len;           // length of token
  int errcode;          // error-code of token
  int commas = -1;      // reject two separators in a row
  int rr = 0;           // return value
  size_t rn;            // rule number
  log_filter_rule *r;   // rule
  uint list_len = 0;    // number of error-codes in input

  // in update-mode, discard old suppress rules for filter rule-set
  if (update) {
    // lock rule-set
    if (log_builtins_filter_ruleset_lock(log_filter_builtin_rules,
                                         LOG_BUILTINS_LOCK_EXCLUSIVE) < 0)
      return -1;  // bail directly (without trying to unlock)

    for (rn = 0; (rn < log_filter_builtin_rules->count);) {
      r = &log_filter_builtin_rules->rule[rn];

      if ((r->match.type == LOG_ITEM_SQL_ERRCODE) &&
          (r->verb == LOG_FILTER_DROP) &&
          (r->cond == LOG_FILTER_COND_EQ))  // match "drop by errcode"
        log_builtins_filter_rule_remove(log_filter_builtin_rules, rn);
      else
        rn++;
    }
  }

  if (list == nullptr) goto success;

  while (true) {
    // find start of token
    while ((*start != '\0') && (isspace(*start) || (*start == ','))) {
      if (*start == ',') {
        /*
          commas at this point:
          -1  We're looking for the first element. No comma allowed yet!
           0  We're looking for element 2+, no comma yet, one is allowed.
           1  We're looking for element 2+ and already have a comma!
        */
        if (commas != 0) goto fail;
        commas++;
      }
      start++;
    }
    end = start;

    // find first non-token character (i.e. first character after token)
    while ((*end != '\0') && !(isspace(*end) || (*end == ','))) end++;

    // no token found, end loop
    if ((len = (end - start)) == 0) {
      if (commas < 1)  // list may not end in a comma
        goto success;
      goto fail;
    }

    if (commas == 0)  // element needs to be first, or follow a comma
      goto fail;
    else
      commas = 0;

    // token too long, error
    if (len >= sizeof(symbol) - 1) goto fail;
    strncpy(symbol, start, len);
    symbol[len] = '\0';

    // numeric token "1234"
    if (isdigit(symbol[0])) {
      char *last;
      errcode = (int)strtol(symbol, &last, 10);
      if (*last != '\0') errcode = -1;
    }
    // alphanumeric token ("MY-1234" or "ER_FOO")
    else
      errcode = mysql_symbol_to_errno(symbol);

    if (!(((errcode >= EE_ERROR_FIRST) && (errcode <= EE_ERROR_LAST)) ||
          ((errcode >= ER_SERVER_RANGE_START) &&
           (mysql_errno_to_builtin(errcode) >= 0))))
      goto fail;

    if (update) {
      // make sure we have the space
      if (log_filter_ruleset_full(log_filter_builtin_rules)) goto fail;

      /*
        This would require the mutex-init to fail,
        which we can't know beforehand.
      */
      if ((r = log_builtins_filter_rule_init(log_filter_builtin_rules)) ==
          nullptr)
        goto fail; /* purecov: inspected */

      log_item_set_with_key(&r->match, LOG_ITEM_SQL_ERRCODE, nullptr,
                            LOG_ITEM_FREE_NONE)
          ->data_integer = errcode;
      r->cond = LOG_FILTER_COND_EQ;
      r->verb = LOG_FILTER_DROP;

      log_filter_builtin_rules->count++;
    }

    /*
      During check-phase, make sure the requested number of error-codes
      (and therefore, the requested number of DROP rules) will fit into
      the built-in rule-set.  Reserve one rule for --log-error-verbosity
      and one for our "ERROR and SYSTEM always pass" shortcut.
      Without this check, we'd still catch the (attempted) overflow during
      assignment, but if we do it during the check phase, we protect the
      integrity of both the current rule-set and the variable's value.
    */
    else if (++list_len >= (log_filter_builtin_rules->alloc - 2))
      goto fail;

    start = end;
  }

fail:
  rr = -(start - list + 1);
success:
  if (update) log_builtins_filter_ruleset_unlock(log_filter_builtin_rules);

  return rr;
}

/*
  Service: built-in filter
*/

DEFINE_METHOD(log_filter_ruleset *, log_builtins_filter_imp::filter_ruleset_new,
              (log_filter_tag * tag, size_t count)) {
  return log_builtins_filter_ruleset_new(tag, count);
}

DEFINE_METHOD(int, log_builtins_filter_imp::filter_ruleset_lock,
              (log_filter_ruleset * ruleset,
               log_builtins_filter_lock locktype)) {
  return log_builtins_filter_ruleset_lock(ruleset, locktype);
}

DEFINE_METHOD(void, log_builtins_filter_imp::filter_ruleset_unlock,
              (log_filter_ruleset * ruleset)) {
  log_builtins_filter_ruleset_unlock(ruleset);
}

DEFINE_METHOD(void, log_builtins_filter_imp::filter_ruleset_drop,
              (log_filter_ruleset * ruleset)) {
  log_builtins_filter_ruleset_drop(ruleset);
}

DEFINE_METHOD(void, log_builtins_filter_imp::filter_ruleset_free,
              (log_filter_ruleset * *ruleset)) {
  log_builtins_filter_ruleset_free(ruleset);
}

DEFINE_METHOD(int, log_builtins_filter_imp::filter_ruleset_move,
              (log_filter_ruleset * from, log_filter_ruleset *to)) {
  uint32 rule_index;

  if (from->count > to->alloc)  // do we have enough space in target?
    return -1;                  /* purecov: inspected */

  log_builtins_filter_ruleset_drop(to);  // clear destination

  to->tag = from->tag;

  for (rule_index = 0; rule_index < from->count; rule_index++) {
    to->rule[rule_index] = from->rule[rule_index];
    memset(&from->rule[rule_index], 0, sizeof(log_filter_rule));
  }

  to->count = from->count;
  from->count = 0;

  return 0;
}

DEFINE_METHOD(void *, log_builtins_filter_imp::filter_rule_init,
              (log_filter_ruleset * ruleset)) {
  if (log_filter_ruleset_full(ruleset)) return nullptr;
  return (void *)log_builtins_filter_rule_init(ruleset);
}

DEFINE_METHOD(int, log_builtins_filter_imp::filter_run,
              (log_filter_ruleset * ruleset, log_line *ll)) {
  return log_builtins_filter_run(ruleset, ll);
}

DEFINE_METHOD(log_filter_ruleset *,
              log_builtins_filter_debug_imp::filter_debug_ruleset_get, (void)) {
  return log_filter_builtin_rules;
}
