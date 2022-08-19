#ifndef DEBUG_SYNC_INCLUDED
#define DEBUG_SYNC_INCLUDED

/* Copyright (c) 2009, 2019, Oracle and/or its affiliates. All rights reserved.

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
  @file

  Declarations for the Debug Sync Facility. See debug_sync.cc for details.
*/

#include <stddef.h>
#include <sys/types.h>

#include "m_string.h"
#include "my_compiler.h"
#include "my_inttypes.h"
#include "my_sharedlib.h"

class THD;

#if defined(ENABLED_DEBUG_SYNC)

/* Macro to be put in the code at synchronization points. */
#define DEBUG_SYNC(_thd_, _sync_point_name_)                 \
  do {                                                       \
    if (unlikely(opt_debug_sync_timeout))                    \
      debug_sync(_thd_, STRING_WITH_LEN(_sync_point_name_)); \
  } while (0)

/* Command line option --debug-sync-timeout. See mysqld.cc. */
extern MYSQL_PLUGIN_IMPORT uint opt_debug_sync_timeout;

/* Default WAIT_FOR timeout if command line option is given without argument. */
#define DEBUG_SYNC_DEFAULT_WAIT_TIMEOUT 300

/* Debug Sync prototypes. See debug_sync.cc. */
extern int debug_sync_init(void);
extern void debug_sync_end(void);
extern void debug_sync_init_thread(THD *thd);
extern void debug_sync_claim_memory_ownership(THD *thd);
extern void debug_sync_end_thread(THD *thd);
extern void debug_sync(THD *thd, const char *sync_point_name, size_t name_len);
extern bool debug_sync_set_action(THD *thd, const char *action_str, size_t len);
extern bool debug_sync_update(THD *thd, char *val_str);
extern uchar *debug_sync_value_ptr(THD *thd);
extern void conditional_sync_point_for_timestamp(std::string name);
extern void conditional_sync_point(std::string name);

/**
  This macro simplifies when a DBUG_EXECUTE_IF will generate a given
  signal and then will wait for another signal to continue.
*/
#define DBUG_SIGNAL_WAIT_FOR(T, A, B, C)                          \
  DBUG_EXECUTE_IF(A, {                                            \
    const char act[] = "now SIGNAL " B " WAIT_FOR " C;            \
    DBUG_ASSERT(!debug_sync_set_action(T, STRING_WITH_LEN(act))); \
  };)

/**
  Set a sync point that is activated by setting
  @@debug='d,syncpoint_NAME', and which will emit the signal
  "reached_NAME" and wait for the signal "continue_NAME"

  @param[in] NAME The name of the debug symbol. This should indicate
    the logical point in time in the code flow where it
    appears. Usually it should have the form "before_EVENT" or
    "after_EVENT", where EVENT identifies something that the code
    does. EVENT might for instance be the name of a function in the
    source code.  The sync point might be reused by multiple tests, so
    the name should relate to what the server does and not the test
    scenario.
  */
#define CONDITIONAL_SYNC_POINT(NAME) conditional_sync_point(NAME)

/**
   Set a sync point that is activated by setting
   @@debug='d,syncpoint_NAME_TIMESTAMP', where NAME is given as an
   argument and TIMESTAMP must match the value of @@session.timestamp
   for the thread.  When activated, the sync point will emit the
   signal "reached_NAME_TIMESTAMP", and wait for the signal
   "continue_NAME_TIMESTAMP".

   @param[in] NAME The name of the debug symbol. This should indicate
   the logical point in time in the code flow where it
   appears. Usually it should have the form "before_EVENT" or
   "after_EVENT", where EVENT identifies something that the code
   does. EVENT might for instance be the name of a function in the
   source code.  The sync point might be reused by multiple tests, so
   the name should relate to what the server does and not the test
   scenario.

   @param[in] TIMESTAMP The timestamp. Only threads where the session
   variable @@session.timestamp is set to TIMESTAMP will be
   affected. TIMESTAMP will be appended to the debug symbol, to the
   signals that the sync point emits and waits for.
*/
#define CONDITIONAL_SYNC_POINT_FOR_TIMESTAMP(NAME) \
  conditional_sync_point_for_timestamp(NAME)

#else /* defined(ENABLED_DEBUG_SYNC) */

#define DEBUG_SYNC(_thd_, _sync_point_name_) /* disabled DEBUG_SYNC */
#define DBUG_SIGNAL_WAIT_FOR(T, A, B, C) \
  do {                                   \
  } while (0)
#define CONDITIONAL_SYNC_POINT(NAME)
#define CONDITIONAL_SYNC_POINT_FOR_TIMESTAMP(NAME)

#endif /* defined(ENABLED_DEBUG_SYNC) */

#endif /* DEBUG_SYNC_INCLUDED */
