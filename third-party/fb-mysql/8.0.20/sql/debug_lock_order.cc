/* Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include <string.h>

#include <my_stacktrace.h>
#include "my_alloc.h"
#include "my_psi_config.h"
#include "my_thread.h"
#include "mysql/psi/psi_cond.h"
#include "mysql/psi/psi_file.h"
#include "mysql/psi/psi_idle.h"
#include "mysql/psi/psi_mutex.h"
#include "mysql/psi/psi_rwlock.h"
#include "mysql/psi/psi_statement.h"
#include "mysql/psi/psi_thread.h"
#include "mysqld.h"
#include "mysqld_error.h"
#include "sql/debug_lock_order.h"

#include "sql/debug_lo_misc.h"
#include "sql/debug_lo_parser.h"
#include "sql/debug_lo_scanner.h"

#include "mysql/components/services/log_builtins.h"

/* Old versions of bison forget to declare this in sql/debug_lo_parser.h */
int LOCK_ORDER_parse(struct LO_parser_param *param);

#ifdef HAVE_EXECINFO_H
#include <execinfo.h>
#endif

#ifdef _WIN32
#include <crtdbg.h>
#include <process.h>
#endif

#include <algorithm>
#include <list>
#include <map>
#include <set>
#include <stack>
#include <vector>

/* Sanity check for the makefile. */
#ifndef WITH_LOCK_ORDER
#error "WITH_LOCK_ORDER is not defined, this code should not be built."
#endif /* WITH_LOCK_ORDER */

/* Additional traces, to debug lock order itself. */
static bool g_internal_debug = false;

/* Helper to exclude rwlock, to debug lock order itself. */
static bool g_with_rwlock = true;

/*
** ========================================================================
** SECTION 1: DOXYGEN DOCUMENTATION
** ========================================================================
*/

/* clang-format off */
/**
  @page PAGE_LOCK_ORDER Lock Order
  MySQL LOCK ORDER

  @section LO_MAIN_INTRO Introduction

  LOCK ORDER is a debugging tool used to enforce
  that no deadlocks can happen in the server.

  The general idea is as follows.

  Every mutex is instrumented by the performance schema,
  and is assigned a name already.

  There is a global, unique, oriented graph,
  that describes the lock sequences allowed during runtime.

  Instrumented mutexes are nodes in the graph.

  When some code locks mutex A then B, there is an explicit
  A -> B edge in the graph.

  During runtime execution, the LOCK ORDER tool checks
  whether the actual locks taken comply with the graph.

  Should any test not comply,
  - either the arc is considered valid, so the graph is incomplete,
    and it should be amended (provided there are no cycles),
    to describe more accurately actual dependencies,
  - or the lock sequence is deemed illegal,
    in which case the code should be changed to avoid it.

  Once a state is achieved where:
  - the graph has no cycle,
  - the entire test suite executes within the graph constraints,

  then the server is guaranteed to have no dead locks.

  The beauty of this approach is that there is no need to
  run tests concurrently to expose deadlocks.

  When statements are executed, even alone in a single session,
  any statement that causes an execution path to be not compliant
  will be detected, and reported.

  @section LO_BUILDING Building the code

  LOCK ORDER is a debugging tool only.
  It should not be used in production.

  @c CMAKE contains a new build option @c WITH_LOCK_ORDER,
  to build with the tool.

  @section LO_RUNNING Running MySQL test with LOCK ORDER

  To run a test without LOCK ORDER,
  use any of the following:

  @verbatim
  ./mtr --lock-order=0 main.parser
  @endverbatim

  @verbatim
  export MTR_LOCK_ORDER=0
  ./mtr main.parser
  @endverbatim

  To run a test with LOCK_ORDER,
  use any of the following:

  @verbatim
  ./mtr --lock-order=1 main.parser
  @endverbatim

  @verbatim
  export MTR_LOCK_ORDER=1
  ./mtr main.parser
  @endverbatim

  By default, LOCK ORDER is disabled in mysql-test-run.

  Executing a test with LOCK_ORDER enabled will have the following effects:

  - the file @c lock_order_dependencies.txt is read
  by the tool, to load the graph to enforce.

  - the file @c lock_order-(timestamp)-(pid).log is written
  by the tool, and contains various messages.

  - optionally, the file @c lock_order.txt is written,
  and contains a textual representation of the graph,
  with some analysis.

  @section LO_FILE_FORMATS Lock order file formats

  @subsection LO_DEPENDENCIES lock_order_dependencies file

  The default dependencies file is
  @c (root)/mysql-test/lock_order_dependencies.txt.

  To use a different dependencies file,
  use the @c lock_order_dependencies system variable.

  The file format is text, with one line per declaration.

  @subsubsection LO_DEP_MUTEX Mutex nodes

  Every node is named from the performance schema instrumentation,
  with a shorter prefix.

  For example, the mutex @c mysql_mutex_t @c LOCK_open is named
  "wait/synch/mutex/sql/LOCK_open" in the performance schema,
  simplified as "mutex/sql/LOCK_open" in LOCK ORDER.

  When the code locks A then B,
  this arc is declared in the graph as:

  @verbatim
  ARC FROM "A" TO "B"
  @endverbatim

  For example:

  @verbatim
  ARC FROM "mutex/sql/LOCK_open" TO "mutex/sql/TABLE_SHARE::LOCK_ha_data"
  @endverbatim

  Note that arcs are not transitive in the graph.

  For example, if the graph has the following arcs:

  @verbatim
  ARC FROM "A" TO "B"
  ARC FROM "B" TO "C"
  @endverbatim

  Then the following code will comply:

  @code
  mysql_mutex_lock(A);
  mysql_mutex_lock(B);
  mysql_mutex_unlock(A);
  mysql_mutex_lock(C);
  mysql_mutex_unlock(B);
  mysql_mutex_unlock(C);
  @endcode

  But the following code will not comply:

  @code
  mysql_mutex_lock(A);
  mysql_mutex_lock(B);
  mysql_mutex_lock(C); // <- this will raise an error.
  mysql_mutex_unlock(A);
  mysql_mutex_unlock(B);
  mysql_mutex_unlock(C);
  @endcode

  This happens because the "A" -> "C" transition is not declared.

  This is a desired feature: to understand contention in the server,
  the real dependencies (the paths actually taken) must be documented explicitly.
  The tool could, but does not, infer more arcs by transitivity.

  Additional metadata can be associated with an ARC,
  by adding flags to the arc declaration.

  The format is

  @verbatim
  ARC FROM "A" TO "B" FLAGS <flag1> <flag2> ... <flagN>
  @endverbatim

  Individual flags are separated by spaces.

  Supported arc flags are:
  - TRACE
  - DEBUG
  - LOOP
  - IGNORED

  @subsubsection LO_DEP_RWLOCK Rwlock nodes

  Read write locks are named from the performance schema,
  with a shorter prefix.

  There are three kinds of read write locks instrumented:
  - read write locks (@c mysql_rwlock_t)
  - priority read write lock (@c mysql_prlock_t)
  - shared exclusive locks (@c rw_lock_t in innodb)

  The lock @c mysql_rwlock_t @c LOCK_system_variables_hash, which is a read write lock,
  is named "wait/synch/rwlock/sql/LOCK_system_variables_hash" in the performance schema,
  simplified as "rwlock/sql/LOCK_system_variables_hash" in LOCK ORDER.

  Read write locks are recursive, but only on read operations.
  Due to the scheduling behavior in the underlying implementation,
  a reader might block indirectly another reader, if a write request is present.

  When a lock is held on a read write lock, it can be in two states:
  - either READ,
  - or WRITE.

  These states are exclusive.

  When a thread holds a read write lock on "L" and locks "B",
  arcs are noted as follows:

  @verbatim
  ARC FROM "rwlock/sql/L" STATE "R" TO "B" ...
  ARC FROM "rwlock/sql/L" STATE "W" TO "B" ...
  @endverbatim

  Operations on read write locks are:
  - READ
  - TRY READ
  - WRITE
  - TRY WRITE

  When a thread holds a lock on "A" and then locks a read write lock "L",
  arcs are noted as follows:

  @verbatim
  ARC FROM "A" ... TO "rwlock/sql/L" OP "R"
  ARC FROM "A" ... TO "rwlock/sql/L" OP "TRY R"
  ARC FROM "A" ... TO "rwlock/sql/L" OP "W"
  ARC FROM "A" ... TO "rwlock/sql/L" OP "TRY W"
  @endverbatim

  Recursive locks are noted as follows:

  @verbatim
  ARC FROM "rwlock/sql/L" STATE "..." TO "rwlock/sql/L" RECURSIVE OP "R"
  ARC FROM "rwlock/sql/L" STATE "..." TO "rwlock/sql/L" RECURSIVE OP "TRY R"
  @endverbatim

  The lock @c mysql_prlock_t @c MDL_context::m_LOCK_waiting_for, which is a priority read write lock,
  is named "wait/synch/prlock/sql/MDL_context::LOCK_waiting_for" in the performance schema,
  simplified as "prlock/sql/MDL_context::LOCK_waiting_for" in LOCK ORDER.

  Priority locks are recursive.
  A reader will never block another reader, even if a write request is present.

  When a lock is held on a priority lock, it can be in two states:
  - either READ,
  - or WRITE.

  These states are exclusive.

  When a thread holds a priority lock on "L" and locks "B",
  arcs are noted as follows:

  @verbatim
  ARC FROM "prlock/sql/L" STATE "R" TO "B" ...
  ARC FROM "prlock/sql/L" STATE "W" TO "B" ...
  @endverbatim

  Operations on priority locks are:
  - READ
  - WRITE

  Note that the READ state can be acquired recursively on the same priority lock.

  When a thread holds a lock on "A" and then locks a priority lock "L",
  arcs are noted as follows:

  @verbatim
  ARC FROM "A" ... TO "prlock/sql/L" OP "R"
  ARC FROM "A" ... TO "prlock/sql/L" OP "W"
  @endverbatim

  Recursive locks are noted as follows:

  @verbatim
  ARC FROM "prlock/sql/L" STATE "..." TO "prlock/sql/L" RECURSIVE OP "R"
  @endverbatim

  The lock @c rw_lock_t @c dict_operation_lock, which is a shared exclusive lock,
  is named "wait/synch/sxlock/innodb/dict_operation_lock" in the performance schema,
  simplified as "sxlock/innodb/dict_operation_lock" in LOCK ORDER.

  Shared exclusive locks are recursive.
  Shared exclusive locks are implemented as spin locks, with fallback on condition variables.

  When a lock is held on a shared exclusive lock, it can be in three states:
  - SHARED,
  - or SHARED EXCLUSIVE,
  - or EXCLUSIVE.

  Because the same lock can be acquired recursively,
  when multiple locks are taken by the same thread on the same object,
  the overall equivalent state is computed (for example, SX + X counts as X).

  When a thread holds a shared exclusive lock on "L" and locks "B",
  arcs are noted as follows:

  @verbatim
  ARC FROM "sxlock/innodb/L" STATE "S" TO "B" ...
  ARC FROM "sxlock/innodb/L" STATE "SX" TO "B" ...
  ARC FROM "sxlock/innodb/L" STATE "X" TO "B" ...
  @endverbatim

  Operations on shared exclusive locks are:
  - SHARED
  - TRY SHARED
  - SHARED EXCLUSIVE
  - TRY SHARED EXCLUSIVE
  - EXCLUSIVE
  - TRY EXCLUSIVE

  Note that some states can be acquired recursively on the same shared exclusive lock.

  When a thread holds a lock on "A" and then locks a shared exclusive lock "L",
  arcs are noted as follows:

  @verbatim
  ARC FROM "A" ... TO "sxlock/innodb/L" OP "S"
  ARC FROM "A" ... TO "sxlock/innodb/L" OP "TRY S"
  ARC FROM "A" ... TO "sxlock/innodb/L" OP "SX"
  ARC FROM "A" ... TO "sxlock/innodb/L" OP "TRY SX"
  ARC FROM "A" ... TO "sxlock/innodb/L" OP "X"
  ARC FROM "A" ... TO "sxlock/innodb/L" OP "TRY X"
  @endverbatim

  Recursive locks are noted as follows:

  @verbatim
  ARC FROM "sxlock/innodb/L" STATE "..." TO "sxlock/innodb/L" RECURSIVE OP "S"
  ARC FROM "sxlock/innodb/L" STATE "..." TO "sxlock/innodb/L" RECURSIVE OP "SX"
  ARC FROM "sxlock/innodb/L" STATE "..." TO "sxlock/innodb/L" RECURSIVE OP "X"
  @endverbatim

  @subsubsection LO_DEP_COND Cond nodes

  Conditions are named from the performance schema,
  with a shorter prefix.

  For example, the condition @c mysql_cond_t @c COND_open is named
  "wait/synch/cond/sql/COND_open" in the performance schema,
  simplified as "cond/sql/COND_open" in LOCK ORDER.

  Explicit arcs are declared between mutexes and associated conditions,
  so that the tool also enforces that the same mutex is consistently
  used for the same condition, to comply with the posix APIs.

  When a condition C is associated with a mutex M,
  this arc is declared in the graph as:

  @verbatim
  BIND "C" TO "M"
  @endverbatim

  For example:

  @verbatim
  BIND "cond/sql/COND_open" TO "mutex/sql/LOCK_open"
  @endverbatim

  In the following sequence of code:

  @verbatim
  mysql_mutex_lock(M);
  mysql_cond_signal(C);
  mysql_mutex_unlock(M);
  @endverbatim

  The tool will verify, when calling mysql_cond_signal, that condition C is bound with M, and that M is locked.

  Note that holding a mutex when using signal or broadcast is recommended, but not mandatory.

  To allow the following code:

  @verbatim
  mysql_cond_signal(C); // mutex M not locked
  @endverbatim

  The condition must be flagged explicitly as using 'UNFAIR' scheduling,
  as in:

  @verbatim
  BIND "C" TO "M" FLAGS UNFAIR
  @endverbatim

  For example:

  @verbatim
  BIND "cond/sql/Master_info::start_cond" TO "mutex/sql/Master_info::run_lock" FLAGS UNFAIR
  @endverbatim

  @subsubsection LO_DEP_FILE File nodes

  Files are named from the performance schema,
  with a shorter prefix.

  For example, the relay log file is named
  "wait/io/file/sql/relaylog" in the performance schema,
  simplified as "file/sql/relaylog" in LOCK ORDER.

  File io operations (read, write, etc) on the file are not documented.
  When any file io is performed while holding a lock,
  the dependency is documented, for example:

  @verbatim
  ARC FROM "rwlock/sql/channel_lock" STATE "W" TO "file/sql/relaylog"
  ARC FROM "sxlock/innodb/dict_operation_lock" STATE "X" TO "file/innodb/innodb_data_file"
  @endverbatim

  @subsection LO_LOG lock_order.log file

  During execution, the server writes to a log file
  located under the build directory,
  in a sub directory named lock-order,
  and named
  @c lock_order-(timestamp)-(pid).log
  where (pid) is the process id for mysqld.

  The log file contains various messages printed
  by LOCK ORDER.

  @subsection LO_GRAPH_TEXT lock_order.txt file

  This file is an optional output.

  To print the lock_order.txt file,
  two actions are required:
  - enable the lock_order_print_txt system variable,
  - send a COM_DEBUG command to the server, using mysqladmin.

  The COM_DEBUG causes the file to be printed.
  It is desirable to load dynamic plugins and components before dumping
  this report, so that lock names instrumented inside the loaded code
  are checked against the dependencies graph.

  A helper test, lock_order.cycle, performs all the steps
  required to dump the lock_order.txt report.

  This command:

  @verbatim
  export MTR_LOCK_ORDER=1
  ./mtr lock_order.cycle
  @endverbatim

  will generate the graph dump as a side effect.

  The generated file contains the following sections:
  - DEPENDENCY GRAPH
  - SCC ANALYSIS (full graph)
  - IGNORED NODES
  - LOOP ARC
  - SCC ANALYSIS (revised graph)
  - UNRESOLVED ARCS


  The section "DEPENDENCY GRAPH" is a textual representation of the graph,
  to facilitate investigations.

  Each node is dumped with incoming and outgoing arcs, for example:

  @verbatim
NODE: mutex/sql/LOCK_open
  16 incoming arcs:
    FROM: mutex/sql/tz_LOCK
    FROM: mutex/p_dyn_loader/key_component_id_by_urn_mutex
    FROM: mutex/sql/LOCK_plugin_install
    FROM: mutex/sql/LOCK_table_cache
    FROM: mutex/sql/key_mts_temp_table_LOCK
    FROM: mutex/sql/LOCK_event_queue
    FROM: mutex/sql/LOCK_global_system_variables
    FROM: mutex/sql/LOCK_reset_gtid_table
    FROM: mutex/sql/Master_info::data_lock
    FROM: mutex/sql/Master_info::run_lock
    FROM: mutex/sql/MYSQL_BIN_LOG::LOCK_index
    FROM: mutex/sql/MYSQL_BIN_LOG::LOCK_log
    FROM: mutex/sql/MYSQL_RELAY_LOG::LOCK_log
    FROM: mutex/sql/Relay_log_info::data_lock
    FROM: mutex/sql/Relay_log_info::run_lock
    FROM: mutex/sql/TABLE_SHARE::LOCK_ha_data -- LOOP FLAG
  22 outgoing arcs:
    TO: mutex/blackhole/blackhole
    TO: mutex/archive/Archive_share::mutex
    TO: mutex/myisam/MYISAM_SHARE::intern_lock
    TO: mutex/innodb/sync_array_mutex
    TO: mutex/innodb/rtr_active_mutex
    TO: mutex/innodb/srv_sys_mutex
    TO: mutex/innodb/rw_lock_list_mutex
    TO: mutex/innodb/rw_lock_debug_mutex
    TO: mutex/innodb/dict_table_mutex
    TO: mutex/innodb/dict_sys_mutex
    TO: mutex/innodb/innobase_share_mutex
    TO: mutex/csv/tina
    TO: mutex/sql/LOCK_plugin
    TO: cond/sql/COND_open
    TO: mutex/mysys/KEY_CACHE::cache_lock
    TO: mutex/mysys/THR_LOCK_heap
    TO: mutex/mysys/THR_LOCK_myisam
    TO: mutex/mysys/THR_LOCK_open
    TO: mutex/sql/DEBUG_SYNC::mutex
    TO: mutex/sql/MDL_wait::LOCK_wait_status
    TO: mutex/sql/TABLE_SHARE::LOCK_ha_data
    TO: mutex/sql/THD::LOCK_current_cond -- LOOP FLAG
  @endverbatim

  The section "SCC ANALYSIS (full graph)" is a "Strongly Connected Component"
  (SCC) analysis of the entire graph.

  See https://en.wikipedia.org/wiki/Strongly_connected_component

  For each SCC, the report prints the nodes part of the SCCs:

  @verbatim
Found SCC number 1 of size 26:
mutex/innodb/dict_sys_mutex
...
mutex/sql/LOCK_offline_mode

Found SCC number 2 of size 2:
mutex/sql/Relay_log_info::run_lock
mutex/sql/Master_info::run_lock

Number of SCC found: 2
  @endverbatim

  Then the arcs internal to each SCC are printed.

  @verbatim
Dumping arcs for SCC 1:
SCC ARC FROM "mutex/myisam/MYISAM_SHARE::intern_lock" TO "mutex/sql/LOCK_plugin"
SCC ARC FROM "mutex/innodb/parser_mutex" TO "mutex/innodb/dict_sys_mutex"
SCC ARC FROM "mutex/innodb/dict_sys_mutex" TO "mutex/sql/LOCK_plugin"
SCC ARC FROM "mutex/sql/LOCK_plugin" TO "mutex/sql/LOCK_global_system_variables"
SCC ARC FROM "mutex/sql/LOCK_plugin" TO "mutex/sql/THD::LOCK_current_cond"
SCC ARC FROM "mutex/sql/LOCK_table_cache" TO "mutex/myisam/MYISAM_SHARE::intern_lock"
SCC ARC FROM "mutex/sql/LOCK_table_cache" TO "mutex/innodb/dict_sys_mutex"
SCC ARC FROM "mutex/sql/LOCK_table_cache" TO "mutex/sql/LOCK_plugin"
...
SCC ARC FROM "mutex/sql/MYSQL_BIN_LOG::LOCK_commit" TO "mutex/sql/LOCK_plugin"
SCC ARC FROM "mutex/sql/MYSQL_BIN_LOG::LOCK_commit" TO "mutex/sql/THD::LOCK_current_cond"

Dumping arcs for SCC 2:
SCC ARC FROM "mutex/sql/Relay_log_info::run_lock" TO "mutex/sql/Master_info::run_lock"
SCC ARC FROM "mutex/sql/Master_info::run_lock" TO "mutex/sql/Relay_log_info::run_lock"
  @endverbatim

  The section "IGNORED NODES" prints the nodes flagged as IGNORED
  in the dependencies file:

  @verbatim
IGNORED NODE: mutex/sql/LOCK_thd_list
IGNORED NODE: mutex/sql/LOCK_event_queue
IGNORED NODE: mutex/sql/LOCK_offline_mode
IGNORED NODE: mutex/sql/LOCK_global_system_variables
  @endverbatim

  The section "LOOP ARC" prints the arcs flagged as LOOP
  in the dependencies file:

  @verbatim
LOOP ARC FROM mutex/myisam/MYISAM_SHARE::intern_lock TO mutex/sql/LOCK_plugin
LOOP ARC FROM mutex/innodb/dict_sys_mutex TO mutex/sql/LOCK_plugin
LOOP ARC FROM mutex/mysys/THR_LOCK_myisam TO mutex/sql/LOCK_plugin
LOOP ARC FROM mutex/sql/LOCK_plugin TO mutex/sql/LOCK_global_system_variables
LOOP ARC FROM mutex/sql/LOCK_plugin TO mutex/sql/THD::LOCK_current_cond
LOOP ARC FROM mutex/sql/TABLE_SHARE::LOCK_ha_data TO mutex/sql/LOCK_table_cache
LOOP ARC FROM mutex/sql/LOCK_open TO mutex/sql/THD::LOCK_current_cond
LOOP ARC FROM mutex/sql/TABLE_SHARE::LOCK_ha_data TO mutex/sql/LOCK_open
LOOP ARC FROM mutex/sql/LOCK_global_system_variables TO mutex/sql/LOCK_thd_list
LOOP ARC FROM mutex/sql/TABLE_SHARE::LOCK_ha_data TO mutex/sql/THD::LOCK_thd_data
  @endverbatim

  The section "SCC ANALYSIS (revised graph)" prints the strongly connected
  components of a sub graph, obtained by:
  - ignoring nodes tagged IGNORED,
  - ignoring arcs tagged LOOP.

  @verbatim
Found SCC number 1 of size 14:
mutex/sql/THD::LOCK_current_cond
mutex/sql/MYSQL_RELAY_LOG::LOCK_log_end_pos
mutex/sql/Relay_log_info::log_space_lock
mutex/sql/MYSQL_RELAY_LOG::LOCK_index
mutex/sql/THD::LOCK_thd_data
mutex/sql/MYSQL_BIN_LOG::LOCK_log
mutex/sql/LOCK_reset_gtid_table
mutex/sql/MYSQL_BIN_LOG::LOCK_sync
mutex/sql/MYSQL_BIN_LOG::LOCK_commit
mutex/sql/MYSQL_BIN_LOG::LOCK_index
mutex/sql/MYSQL_RELAY_LOG::LOCK_log
mutex/sql/Relay_log_info::data_lock
mutex/sql/key_mts_temp_table_LOCK
mutex/sql/Master_info::data_lock

Found SCC number 2 of size 2:
mutex/sql/Relay_log_info::run_lock
mutex/sql/Master_info::run_lock

Number of SCC found: 2

Dumping arcs for SCC 1:
...

Dumping arcs for SCC 2:
...
  @endverbatim

  Finally, the section "UNRESOLVED ARCS" lists arcs found in the dependency graph
  that could not be matched with actual nodes from the code.

  @verbatim
UNRESOLVED ARC FROM "mutex/sql/MYSQL_BIN_LOG::LOCK_log" TO "mutex/semisync/LOCK_binlog_"
UNRESOLVED ARC FROM "mutex/sql/MYSQL_BIN_LOG::LOCK_commit" TO "mutex/semisync/LOCK_binlog_"
UNRESOLVED ARC FROM "mutex/sql/LOCK_plugin" TO "mutex/semisync/LOCK_binlog_"
UNRESOLVED ARC FROM "mutex/sql/LOCK_plugin_install" TO "mutex/semisync/LOCK_binlog_"
UNRESOLVED ARC FROM "mutex/sql/LOCK_plugin_install" TO "mutex/semisync/Ack_receiver::m_mutex"
UNRESOLVED ARC FROM "mutex/semisync/LOCK_binlog_" TO "cond/semisync/COND_binlog_send_"
UNRESOLVED ARC FROM "mutex/semisync/Ack_receiver::m_mutex" TO "cond/semisync/Ack_receiver::m_cond"
  @endverbatim

  Arcs can be unresolved for two reasons:
  - The code changed, and a node was either removed or renamed.
    The fix is to correct the dependency file accordingly.
  - A node is defined in a plugin (in this example, semisync), but the plugin is not loaded
    in the lock_order.cycle test script. The fix is to add the missing plugin.

  @section LO_METHODOLOGY Methodology

  First, the graph defining valid lock sequences should be
  unique for the server.
  Any attempt to use different graphs for different tests
  is fundamentally flawed.

  Secondly, documenting the dependencies graph helps to
  document the design, as the order of locks does not happen by accident
  in the code but as a result of design decisions by development.

  In an ideal world, the dependency graph should be documented
  up front when writing the code, and testing should only verify
  that the code complies with the graph.

  In practice, such a graph is -- not -- documented,
  only fragments of it are "common knowledge".

  As a result, the complete graph must be discovered from the code,
  by performing reverse engineering, to be documented.

  The LOCK_ORDER tool support both:
  - discovery, to rebuild a graph from runtime execution
  - enforcement, to ensure runtime execution complies with constraints

  @subsection LO_PROCESS_COLLECT Collect missing arcs

  Start with an empty lock_order_dependencies.txt file,
  and run a test.

  For example,
  @verbatim
  ./mtr main.parser
  @endverbatim

  The resulting @c lock-order-(timestamp)-(pid).log file will contain numerous
  messages, like:

@verbatim
MISSING: ARC FROM "mutex/sql/LOCK_table_cache" TO "mutex/sql/LOCK_open"
Trace: using missing arc mutex/sql/LOCK_table_cache (/home/malff/GIT_LOCK_ORDER/sql/table_cache.h:140)
 -> mutex/sql/LOCK_open (/home/malff/GIT_LOCK_ORDER/sql/table_cache.h:319)
@endverbatim

  Here, the tool detected a lock sequence (@c LOCK_table_cache then @c LOCK_open)
  that is not declared in the graph.
  The "MISSING:" line prints the arc definition that should be added
  in the lock_order_dependencies.txt file, for easier copy and paste.
  The "Trace:" message gives details about where each lock was taken in the source code.

  An efficient way to add at once all the missing arcs found while running tests
  is as follows:

  @verbatim
  cat lock_order*.log | grep MISSING | sort -u
  @endverbatim

  Run this script, remove the MISSING: prefix,
  and add the result to the dependency graph.
  Then run tests again, with the new graph,
  and repeat until no more missing arcs are found.

  @subsection LO_PROCESS_ANALYSIS Perform graph analysis

  The tool processes the dependency graph to detect
  "Strongly Connected Components".
  See https://en.wikipedia.org/wiki/Strongly_connected_component

  Strictly speaking, a Strongly Connected Component can be a single
  node. In the context of LOCK_ORDER, "SCC" refers to Strongly Connected Components
  of size greater or equal to 2, that is, an actual cluster of _several_ nodes.

  This computation is done when dumping the lock_order.txt file.

  A dedicated mtr test is written as a helper:

  @verbatim
  ./mtr lock_order.cycle
  @endverbatim

  Then read the section named "SCC ANALYSIS (full graph)" in file lock_order.txt

  At time of writing, it reads as:

  @verbatim
SCC ANALYSIS (full graph):
==========================

Found SCC number 1 of size 26:
mutex/innodb/dict_sys_mutex
mutex/sql/LOCK_plugin
mutex/sql/LOCK_global_system_variables
mutex/innodb/parser_mutex
mutex/sql/LOCK_table_cache
mutex/myisam/MYISAM_SHARE::intern_lock
mutex/mysys/THR_LOCK_myisam
mutex/sql/LOCK_open
mutex/sql/TABLE_SHARE::LOCK_ha_data
mutex/sql/THD::LOCK_thd_data
mutex/sql/LOCK_event_queue
mutex/sql/MYSQL_BIN_LOG::LOCK_commit
mutex/sql/THD::LOCK_current_cond
mutex/sql/MYSQL_RELAY_LOG::LOCK_log_end_pos
mutex/sql/Relay_log_info::log_space_lock
mutex/sql/LOCK_thd_list
mutex/sql/MYSQL_RELAY_LOG::LOCK_index
mutex/sql/MYSQL_RELAY_LOG::LOCK_log
mutex/sql/Relay_log_info::data_lock
mutex/sql/key_mts_temp_table_LOCK
mutex/sql/Master_info::data_lock
mutex/sql/MYSQL_BIN_LOG::LOCK_log
mutex/sql/LOCK_reset_gtid_table
mutex/sql/MYSQL_BIN_LOG::LOCK_sync
mutex/sql/MYSQL_BIN_LOG::LOCK_index
mutex/sql/LOCK_offline_mode

Found SCC number 2 of size 2:
mutex/sql/Relay_log_info::run_lock
mutex/sql/Master_info::run_lock

Number of SCC found: 2
  @endverbatim

  The tool found two Strongly Connected Components (SCC).

  The details about each arcs are printed:

  @verbatim
Dumping arcs for SCC 1:
SCC ARC FROM "mutex/myisam/MYISAM_SHARE::intern_lock" TO "mutex/sql/LOCK_plugin"
SCC ARC FROM "mutex/innodb/parser_mutex" TO "mutex/innodb/dict_sys_mutex"
SCC ARC FROM "mutex/innodb/dict_sys_mutex" TO "mutex/sql/LOCK_plugin"
SCC ARC FROM "mutex/sql/LOCK_plugin" TO "mutex/sql/LOCK_global_system_variables"
SCC ARC FROM "mutex/sql/LOCK_plugin" TO "mutex/sql/THD::LOCK_current_cond"
SCC ARC FROM "mutex/sql/LOCK_table_cache" TO "mutex/myisam/MYISAM_SHARE::intern_lock"
SCC ARC FROM "mutex/sql/LOCK_table_cache" TO "mutex/innodb/dict_sys_mutex"
SCC ARC FROM "mutex/sql/LOCK_table_cache" TO "mutex/sql/LOCK_plugin"
...
SCC ARC FROM "mutex/sql/MYSQL_BIN_LOG::LOCK_commit" TO "mutex/sql/LOCK_plugin"
SCC ARC FROM "mutex/sql/MYSQL_BIN_LOG::LOCK_commit" TO "mutex/sql/THD::LOCK_current_cond"

Dumping arcs for SCC 2:
SCC ARC FROM "mutex/sql/Relay_log_info::run_lock" TO "mutex/sql/Master_info::run_lock"
SCC ARC FROM "mutex/sql/Master_info::run_lock" TO "mutex/sql/Relay_log_info::run_lock"
  @endverbatim

  Note that only arcs within the same SCC are printed here,
  as they are the ones that need closer investigation.
  Arcs coming in or out of this SCC are omitted, to reduce noise.

  @subsection LO_PROCESS_BREAK Break Strongly Connected Components

  By its very own definition, a SCC of size greater than one
  is a cluster of nodes where any node is reachable, using locks,
  from any other node.
  Hence, SCC should not exist, and the code must be re-factored to avoid them.

  In the list of SCC arcs printed, some (human) analysis is required
  to decide:
  - which arc is valid, and compliant with the MySQL server design
  - which arc is invalid, which points to a flaw in the code.

  A bug fix is required to change the server code affected,
  to avoid locks, or take locks in the proper order.

  Now, this is a sensitive task, for the following reasons:
  - deciding which arc to remove is by itself difficult
  - changing the server code and / or design to change lock order
    can be even more difficult
  - removing an arc alone might not even fix anything, if there is
    another path in the graph that closes the same loop.

  For this reason, the tool supports ways to simulate "what-if" scenarios,
  and see in practice what the overall graph would look like if such and such
  arc were to be removed.

  First, some nodes can (temporarily) be ignored entirely,
  to simplify the graph analysis, and identify smaller sub graphs in a big SCC.

  By ignoring some nodes,
  a big SCC can be broken down into smaller,
  independent, sub graphs,
  which helps to investigate, identify, and resolve
  several dead lock root causes in isolation.

  To ignore a node "A", use the following syntax in the dependency graph:

  @verbatim
NODE "A" IGNORED
  @endverbatim

  For example, using

  @verbatim
NODE "mutex/sql/LOCK_event_queue" IGNORED
NODE "mutex/sql/LOCK_global_system_variables" IGNORED
NODE "mutex/sql/LOCK_offline_mode" IGNORED
NODE "mutex/sql/LOCK_thd_list" IGNORED
  @endverbatim

  will produce a graph without these nodes,
  also ignoring arcs from and to these nodes.

  Secondly, arcs that are considered loops to fix in the code can be marked explicitly,
  like this:

  @verbatim
ARC FROM "mutex/sql/TABLE_SHARE::LOCK_ha_data" TO "mutex/sql/LOCK_open" FLAGS LOOP
ARC FROM "mutex/sql/TABLE_SHARE::LOCK_ha_data" TO "mutex/sql/LOCK_table_cache" FLAGS LOOP
ARC FROM "mutex/sql/TABLE_SHARE::LOCK_ha_data" TO "mutex/sql/THD::LOCK_thd_data" FLAGS LOOP
  @endverbatim

  After tagging some IGNORED nodes or LOOP arcs, generate the lock_order.txt
  report again, to perform some analysis again.

  The section "SCC ANALYSIS (full graph)" will be identical, as the real graph did not change.

  The section "SCC ANALYSIS (revised graph)" will show what the graph would look like, with the loops fixed.

  The goal is to iteratively tweak the LOOP flags in the graph,
  and perform analysis on the revised graph, until:
  - the list of LOOP arcs can be reasonably fixed in the code,
    without causing too much re-engineering effort.
  - the resulting revised graph has less complicated SCC (ideally, there should be none left),
    showing progress towards the resolution of dead locks.

  Once a viable set of LOOP arcs to remove is identified,
  file a bug report to address the issue found.

  @subsection LO_PROCESS_FIX Get reported bugs fixed

  Each time a dependency is flagged as a LOOP,
  a matching bug report should be filed,
  and that bug should be eventually resolved.

  Marking nodes are IGNORED or arcs as LOOP is equivalent
  to using suppressions in valgrind to avoid error messages.

  This can be convenient to investigate further other areas,
  but it is by no means a satisfactory resolution in itself.

  To achieve a state where the server can be declared as deadlock free
  with reasonable confidence, all the following conditions are required:
  - the GCOV code coverage is satisfactory,
    and in particular covers all lines of code taking locks.
  - all mutexes in the code are instrumented with the performance schema
  - the dependency graph contains no SCC
  - the dependency graph contains no LOOP arcs
  - the dependency graph contains no IGNORED nodes
  - the test suite passes under LOCK_ORDER without any complaints from the tool.

  @section LO_TOOLS Advanced tooling

  To facilitate investigations and debugging, the following features are available:
  - Tracing
  - Debugging
  - Simulating loop arcs
  - Simulating ignored nodes

  @subsection LO_TOOL_TRACE Tracing dependencies

  When an arc from A to B exists in the graph,
  it might be sometime necessary to understand where
  in the source code the A -> B dependency is actually taken.

  By declaring the arc with the TRACE flag, as in

  @verbatim
  ARC FROM "mutex/sql/LOCK_open" TO "mutex/sql/TABLE_SHARE::LOCK_ha_data" FLAGS TRACE
  @endverbatim

  the tool will:
  - capture the current statement text,
  - capture the source code location, and the call stack,
    when the first lock is taken,
  - capture the source code location, and the call stack,
    when the second lock is taken,
  and print all the details in the log file when this arc is found
  during runtime execution.

  An example of trace:

  @verbatim
Trace: using arc mutex/sql/LOCK_open (/home/malff/GIT_LOCK_ORDER/sql/sql_base.cc:1704) -> mutex/sql/TABLE_SHARE::LOCK_ha_data (/home/malff/GIT_LOCK_ORDER/sql/handler.cc:7764)
statement when first lock was acquired:
ALTER TABLE t1 ADD PARTITION
stack when the first lock was acquired:
[0] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(_ZN14LO_stack_traceC2Ev+0x28) [0x2e9f654]
[1] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(_ZN7LO_lock18record_stack_traceEv+0x24) [0x2e9fe52]
[2] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(_ZN9LO_thread14add_mutex_lockEP8LO_mutexPKci+0x18c) [0x2e9aeb4]
[3] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(_ZN15LO_mutex_locker3endEv+0x3e) [0x2ea02dc]
[4] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld() [0x2e9d834]
[5] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld() [0x2b30112]
[6] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld() [0x2b33f39]
[7] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(_Z18close_thread_tableP3THDPP5TABLE+0x20e) [0x2b34196]
[8] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld() [0x2b336db]
[9] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(_Z19close_thread_tablesP3THD+0x3fd) [0x2b33e83]
[10] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(_Z21mysql_execute_commandP3THDb+0x5dca) [0x2be875c]
[11] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(_Z11mysql_parseP3THDP12Parser_stateb+0x672) [0x2bea1bb]
[12] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(_Z16dispatch_commandP3THDPK8COM_DATA19enum_server_command+0x1496) [0x2be0291]
[13] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(_Z10do_commandP3THD+0x448) [0x2bde867]
[14] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld() [0x2d799ba]
[15] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(lo_spawn_thread+0xda) [0x2e9cb5c]
[16] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld() [0x466465e]
[17] /lib64/libpthread.so.0(+0x80a4) [0x7f094120a0a4]
[18] /lib64/libc.so.6(clone+0x6d) [0x7f093f75102d]
stack when the second lock was acquired:
[0] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(_ZN14LO_stack_traceC2Ev+0x28) [0x2e9f654]
[1] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(_ZN8LO_graph5checkEPK7LO_lockS2_+0x447) [0x2e99d25]
[2] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(_ZN9LO_thread11check_locksEPK7LO_lock+0xf1) [0x2e9aa99]
[3] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(_ZN9LO_thread14add_mutex_lockEP8LO_mutexPKci+0x1c9) [0x2e9aef1]
[4] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(_ZN15LO_mutex_locker3endEv+0x3e) [0x2ea02dc]
[5] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld() [0x2e9d834]
[6] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld() [0x2f04334]
[7] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(_ZN7handler19lock_shared_ha_dataEv+0x6c) [0x2f18e10]
[8] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(_ZN11ha_innopart5closeEv+0xec) [0x4125be8]
[9] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(_ZN7handler8ha_closeEv+0x173) [0x2f0aa1d]
[10] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(_Z8closefrmP5TABLEb+0x8c) [0x2d1e118]
[11] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(_Z18intern_close_tableP5TABLE+0xe5) [0x2b32d42]
[12] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld() [0x2b33f45]
[13] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(_Z18close_thread_tableP3THDPP5TABLE+0x20e) [0x2b34196]
[14] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld() [0x2b336db]
[15] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(_Z19close_thread_tablesP3THD+0x3fd) [0x2b33e83]
[16] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(_Z21mysql_execute_commandP3THDb+0x5dca) [0x2be875c]
[17] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(_Z11mysql_parseP3THDP12Parser_stateb+0x672) [0x2bea1bb]
[18] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(_Z16dispatch_commandP3THDPK8COM_DATA19enum_server_command+0x1496) [0x2be0291]
[19] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(_Z10do_commandP3THD+0x448) [0x2bde867]
[20] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld() [0x2d799ba]
[21] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(lo_spawn_thread+0xda) [0x2e9cb5c]
[22] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld() [0x466465e]
[23] /lib64/libpthread.so.0(+0x80a4) [0x7f094120a0a4]
[24] /lib64/libc.so.6(clone+0x6d) [0x7f093f75102d]
  @endverbatim

  Note that C++ symbol names are mangled.
  Using @c c++filt on the log produces the following output:

  @verbatim
Trace: using arc mutex/sql/LOCK_open (/home/malff/GIT_LOCK_ORDER/sql/sql_base.cc:1704) -> mutex/sql/TABLE_SHARE::LOCK_ha_data (/home/malff/GIT_LOCK_ORDER/sql/handler.cc:7764)
statement when first lock was acquired:
ALTER TABLE t1 ADD PARTITION
stack when the first lock was acquired:
[0] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(LO_stack_trace::LO_stack_trace()+0x28) [0x2e9f654]
[1] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(LO_lock::record_stack_trace()+0x24) [0x2e9fe52]
[2] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(LO_thread::add_mutex_lock(LO_mutex*, char const*, int)+0x18c) [0x2e9aeb4]
[3] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(LO_mutex_locker::end()+0x3e) [0x2ea02dc]
[4] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld() [0x2e9d834]
[5] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld() [0x2b30112]
[6] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld() [0x2b33f39]
[7] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(close_thread_table(THD*, TABLE**)+0x20e) [0x2b34196]
[8] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld() [0x2b336db]
[9] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(close_thread_tables(THD*)+0x3fd) [0x2b33e83]
[10] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(mysql_execute_command(THD*, bool)+0x5dca) [0x2be875c]
[11] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(mysql_parse(THD*, Parser_state*, bool)+0x672) [0x2bea1bb]
[12] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(dispatch_command(THD*, COM_DATA const*, enum_server_command)+0x1496) [0x2be0291]
[13] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(do_command(THD*)+0x448) [0x2bde867]
[14] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld() [0x2d799ba]
[15] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(lo_spawn_thread+0xda) [0x2e9cb5c]
[16] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld() [0x466465e]
[17] /lib64/libpthread.so.0(+0x80a4) [0x7f094120a0a4]
[18] /lib64/libc.so.6(clone+0x6d) [0x7f093f75102d]
stack when the second lock was acquired:
[0] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(LO_stack_trace::LO_stack_trace()+0x28) [0x2e9f654]
[1] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(LO_graph::check(LO_lock const*, LO_lock const*)+0x447) [0x2e99d25]
[2] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(LO_thread::check_locks(LO_lock const*)+0xf1) [0x2e9aa99]
[3] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(LO_thread::add_mutex_lock(LO_mutex*, char const*, int)+0x1c9) [0x2e9aef1]
[4] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(LO_mutex_locker::end()+0x3e) [0x2ea02dc]
[5] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld() [0x2e9d834]
[6] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld() [0x2f04334]
[7] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(handler::lock_shared_ha_data()+0x6c) [0x2f18e10]
[8] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(ha_innopart::close()+0xec) [0x4125be8]
[9] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(handler::ha_close()+0x173) [0x2f0aa1d]
[10] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(closefrm(TABLE*, bool)+0x8c) [0x2d1e118]
[11] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(intern_close_table(TABLE*)+0xe5) [0x2b32d42]
[12] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld() [0x2b33f45]
[13] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(close_thread_table(THD*, TABLE**)+0x20e) [0x2b34196]
[14] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld() [0x2b336db]
[15] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(close_thread_tables(THD*)+0x3fd) [0x2b33e83]
[16] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(mysql_execute_command(THD*, bool)+0x5dca) [0x2be875c]
[17] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(mysql_parse(THD*, Parser_state*, bool)+0x672) [0x2bea1bb]
[18] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(dispatch_command(THD*, COM_DATA const*, enum_server_command)+0x1496) [0x2be0291]
[19] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(do_command(THD*)+0x448) [0x2bde867]
[20] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld() [0x2d799ba]
[21] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld(lo_spawn_thread+0xda) [0x2e9cb5c]
[22] /home/malff/GIT_LOCK_ORDER/build/runtime_output_directory/mysqld() [0x466465e]
[23] /lib64/libpthread.so.0(+0x80a4) [0x7f094120a0a4]
[24] /lib64/libc.so.6(clone+0x6d) [0x7f093f75102d]
  @endverbatim

  @subsection LO_TOOL_DEBUG Debugging dependencies

  When tracing is not enough, the next step is to break the server
  execution in a debugger to understand the context, to investigate.

  The tool allows to put breakpoints on dependencies (not just functions),
  using the DEBUG flag on an arc, as in:

  @verbatim
  ARC FROM "mutex/sql/LOCK_open" TO "mutex/sql/TABLE_SHARE::LOCK_ha_data" FLAGS DEBUG
  @endverbatim

  When this arc is found at runtime, a @c DBUG_ASSERT will fail,
  that can be caught.

  To help diagnostics, the tool will construct a string that details
  the reason for the failed assert.

  In a debugger, a good place to put breakpoints to debug specific arcs
  is the function @c debug_lock_order_break_here().

  @subsection LO_TOOL_LOOP Simulating loops

  Loops can be declared explicitly in the dependency graph, using the LOOP flag:

  @verbatim
  ARC FROM "mutex/sql/TABLE_SHARE::LOCK_ha_data" TO "mutex/sql/LOCK_open" FLAGS LOOP
  @endverbatim

  @subsection LO_TOOL_IGNORED Simulating ignored nodes

  To facilitate investigation, nodes can be ignored to produce a smaller
  graph:

  @verbatim
  NODE "mutex/sql/LOCK_thd_list" IGNORED
  @endverbatim

  @section LO_NOTATION Understanding the notation

  This is a mini tutorial with examples,
  to better understand the notations used in file lock_order.txt.

  @subsection LO_TUT_1 Basic mutex loop

  @subsubsection LO_TUT_1_CODE Sample code

  Assume some server code as follows:

  @code
  mysql_mutex_t mutex_A;
  mysql_mutex_t mutex_B;
  mysql_mutex_t mutex_C;

  void func_red() {
    mysql_mutex_lock(&mutex_A);
    mysql_mutex_lock(&mutex_B);
    do_something_red();
    mysql_mutex_unlock(&mutex_B);
    mysql_mutex_unlock(&mutex_A);
  }

  void func_green() {
    mysql_mutex_lock(&mutex_B);
    mysql_mutex_lock(&mutex_C);
    do_something_green();
    mysql_mutex_unlock(&mutex_C);
    mysql_mutex_unlock(&mutex_B);
  }

  void func_blue() {
    mysql_mutex_lock(&mutex_C);
    mysql_mutex_lock(&mutex_A);
    do_something_blue();
    mysql_mutex_unlock(&mutex_A);
    mysql_mutex_unlock(&mutex_C);
  }
  @endcode

  Then, assume three different threads (red, green and blue)
  execute the corresponding functions in the server.

  @subsubsection LO_TUT_1_GRAPH Dependency graph

  When running tests, the lock_order tool will find the following dependencies,
  to add in the lock_order_dependencies.txt file:

  @verbatim
  ARC FROM "mutex/sql/A" TO "mutex/sql/B"
  ARC FROM "mutex/sql/B" TO "mutex/sql/C"
  ARC FROM "mutex/sql/C" TO "mutex/sql/A"
  @endverbatim

  @subsubsection LO_TUT_1_REPORT Lock order report

  Executing the lock_order.cycle test, to get the lock_order.txt report,
  will show the following SCC:

  @verbatim
  Dumping classes for SCC 1:
  SCC Class mutex/sql/A
  SCC Class mutex/sql/B
  SCC Class mutex/sql/C

  Dumping nodes for SCC 1:
  SCC Node mutex/sql/A Girth 3
  SCC Node mutex/sql/B Girth 3
  SCC Node mutex/sql/C Girth 3

  Dumping arcs for SCC 1:
  SCC ARC FROM "mutex/sql/A" TO "mutex/sql/B"
  SCC ARC FROM "mutex/sql/B" TO "mutex/sql/C"
  SCC ARC FROM "mutex/sql/C" TO "mutex/sql/A"

  SCC 1/1 summary:
   - Class Size 3
   - Node Size 3
   - Arc Count 3
   - Girth 3
   - Circumference 3
  @endverbatim

  In other words, the tool found a cycle in the code, illustrated below.

  @dot
  digraph LockOrder {
    graph [ label="Lock Order SCC cycle" ]

    mutex_A -> mutex_B [label="func_red", color=red, penwidth=3];
    mutex_B -> mutex_C [label="func_green", color=green, penwidth=3];
    mutex_C -> mutex_A [label="func_blue", color=blue, penwidth=3];
  }
  @enddot

  This cycle involves three nodes, so the cycle girth is 3.

  A possible fix is to change the implementation of func_blue()
  to take locks on A then C, in that order.

  Fixing the code is not enought, as the lock_order_dependencies.txt file
  now contains an arc (C -> A) that is never taken in the code.

  Once both:
  - the code is fixed
  - the C -> A arc is replaced by an A -> C arc in lock_order_dependencies.txt

  the tool will then detect no more deadlocks involving these nodes.

  @subsection LO_TUT_2 With read write lock

  Building on the previous example, let's now change B and C to read write locks.

  @subsubsection LO_TUT_2_CODE Sample code

  @code
  mysql_mutex_t mutex_A;
  mysql_rwlock_t rwlock_B;
  mysql_rwlock_t rwlock_C;

  void func_red() {
    mysql_mutex_lock(&mutex_A);
    mysql_rwlock_rdlock(&rwlock_B);
    do_something_red();
    mysql_rwlock_unlock(&rwlock_B);
    mysql_mutex_unlock(&mutex_A);
  }

  void func_green() {
    mysql_rwlock_rdlock(&rwlock_B);
    mysql_rwlock_wrlock(&rwlock_C);
    do_something_green();
    mysql_rwlock_unlock(&rwlock_C);
    mysql_rwlock_unlock(&rwlock_B);
  }

  void func_blue() {
    mysql_rwlock_rdlock(&rwlock_C);
    mysql_mutex_lock(&mutex_A);
    do_something_blue();
    mysql_mutex_unlock(&mutex_A);
    mysql_mutex_unlock(&mutex_C);
  }
  @endcode

  @subsubsection LO_TUT_2_GRAPH Dependency graph

  The dependencies found,
  to add in the lock_order_dependencies.txt file, are:

  @verbatim
  ARC FROM "mutex/sql/A" TO "rwlock/sql/B" OP "R"
  ARC FROM "rwlock/sql/B" STATE "R" TO "rwlock/sql/C" OP "W"
  ARC FROM "rwlock/sql/C" STATE "R" TO "mutex/sql/A"
  @endverbatim

  @subsubsection LO_TUT_2_REPORT Lock order report

  File lock_order.txt indicates a SCC, so there is a cycle.

  @verbatim
  Dumping classes for SCC 1:
  SCC Class mutex/sql/A
  SCC Class rwlock/sql/B
  SCC Class rwlock/sql/C

  Dumping nodes for SCC 1:
  SCC Node mutex/sql/A
  SCC Node rwlock/sql/B:-R
  SCC Node rwlock/sql/B:+R
  SCC Node rwlock/sql/C:-R
  SCC Node rwlock/sql/C:+W

  -- Note that these nodes exist, but are not part of the SCC: --
  Node rwlock/sql/B:+W
  Node rwlock/sql/B:-W
  Node rwlock/sql/C:+R
  Node rwlock/sql/C:-W

  Dumping arcs for SCC 1:
  SCC ARC FROM "mutex/sql/A" TO "rwlock/sql/B:+R"
  SCC ARC FROM "rwlock/sql/B:+R" TO "rwlok/sql/B:-R" -- MICRO ARC
  SCC ARC FROM "rwlock/sql/B:-R" TO "rwlock/sql/C:+W"
  SCC ARC FROM "rwlock/sql/C:+W" TO "rwlock/sql/C:-R" -- MICRO ARC
  SCC ARC FROM "rwlock/sql/C:-R" TO "rwlock/sql/A"

  SCC 1/1 summary:
   - Class Size 3
   - Node Size 5
   - Arc Count 5
   - Girth 5
   - Circumference 5
  @endverbatim

  First, notice how the read write lock class "rwlock/sql/B"
  is represented by four nodes, named:
  - "rwlock/sql/B:+R", incomming read lock
  - "rwlock/sql/B:-R", outgoing read lock
  - "rwlock/sql/B:+W", incomming write lock
  - "rwlock/sql/B:-W", outgoing write lock

  A practical way to represent this graphically is to draw a box
  for the lock, that contains four ports.
  Internal connections between ports represent the logic table
  for the lock: an arc from "+R" to "-W" means that an incomming request
  for a read lock ("+R") is blocked by a write lock request already given ("-W").

  Micro arcs represent the wired logic of the lock itself, these can not be changed.

  @dot
  digraph LockOrder {
    graph [ label="Lock Order rwlock block" ]

    subgraph cluster_rwlock_B {
      graph [ label="mysql_rwlock_t", rankdir=TB ]

      node [ label="+R" ] p_r;
      node [ label="-R" ] m_r;
      node [ label="+W" ] p_w;
      node [ label="-W" ] m_w;
      { rank=same; p_r -> m_r [style=dashed]; }
      { rank=same; p_w -> m_w [style=dashed]; }
      p_r -> m_w [style=dashed];
      p_w -> m_r [style=dashed];
    }
  }
  @enddot

  The complete SCC report can be represented graphically as follows:

  @dot
  digraph LockOrder {
    graph [ label="Lock Order SCC cycle", rankdir=LR ]

    mutex_A;

    subgraph cluster_rwlock_B {
      graph [ label="rwlock_B", rankdir=TB ]

      node [ label="+R" ] B_p_r;
      node [ label="-R" ] B_m_r;
      node [ label="+W" ] B_p_w;
      node [ label="-W" ] B_m_w;
      B_p_r -> B_m_r [style=dashed, penwidth=3];
      B_p_w -> B_m_w [style=dashed];
      B_p_r -> B_m_w [style=dashed];
      B_p_w -> B_m_r [style=dashed];
    }

    subgraph cluster_rwlock_C {
      graph [ label="rwlock_C", rankdir=TB ]

      node [ label="+R" ] C_p_r;
      node [ label="-R" ] C_m_r;
      node [ label="+W" ] C_p_w;
      node [ label="-W" ] C_m_w;
      C_p_r -> C_m_r [style=dashed];
      C_p_w -> C_m_w [style=dashed];
      C_p_r -> C_m_w [style=dashed];
      C_p_w -> C_m_r [style=dashed, penwidth=3];
    }

    mutex_A -> B_p_r [ label="func_red", color=red, penwidth=3 ];
    B_m_r -> C_p_w [ label="func_green", color = green, penwidth=3 ];
    C_m_r -> mutex_A [ label="func_blue", color=blue, penwidth=3 ];
  }
  @enddot

  Indeed in this graph, there is a cycle of girth 5 involving the following 3 objects:
  - mutex_A
  - rwlock_B
  - rwlock_C

  The lock order tool indicates that this is a dead lock ... let's verify this with
  a test scenario:
  - t1: thread red executes function func_red
  - t2: thread red locks mutex_A
  - t3: thread red is interrupted, and sleeps for a while
  - t4: thread blue executes function func_blue
  - t5: thread blue locks rwlock_C in read mode
  - t6: thread blue attempts to lock mutex_A and is blocked.
  - t7: thread green executes function func_green
  - t8: thread green locks rwlock_B in read mode
  - t9: thread green attempts to lock rwlock_C in write mode and is blocked.
  - t20: thread red awakes
  - t21: thread red attempts to lock rwlock_B in read mode ...

  Now, at t21, rwlock_B is locked in read mode by thread green,
  so a request for a read lock should be granted, right ?

  Lock order claims there is a dead lock, because a read can block indirectly a read:

  Consider the folowing code:

  @code
  func_grey() {
    mysql_rwlock_wrlock(&rwlock_B);
    mysql_rwlock_unlock(&rwlock_B);
  }
  @endcode

  Executing func_grey() in thread grey any time after t8 will cause
  thread grey to block, with a write lock request in the queue in rwlock_B.

  Such a request will in turn block thread red at t21.

  The system is in deadlock:
  - thread red holds mutex_A, waits for rwlock_B:+R
  - thread blue holds rwlock_C:-R, waits for mutex_A
  - thread gren holds rwlock_B:-R, waits for rwlock_C:+W
  - thread grey holds nothing, waits for rwlock_B:+W

  A possible fix is to replace rwlock_B by a priority lock, prlock_B.
  Priority locks differ from read write locks precisely on this scheduling policy,
  as a read lock will never block another read lock, even when write lock requests
  are present.

  The internal representation of a priority lock is as follows,
  note how the "+R" -> "-R" arc is missing, allowing parallel processing for reads.

  @dot
  digraph LockOrder {
    graph [ label="Lock Order prlock block" ]

    subgraph cluster_prlock {
      graph [ label="mysql_prlock_t", rankdir=TB ]

      node [ label="+R" ] p_r;
      node [ label="-R" ] m_r;
      node [ label="+W" ] p_w;
      node [ label="-W" ] m_w;
      p_w -> m_w [style=dashed];
      p_r -> m_w [style=dashed];
      p_w -> m_r [style=dashed];
    }
  }
  @enddot

  @subsection LO_TUT_3 With shared exclusive locks

  Let's now look at innodb @c rw_lock_t,
  which is a shared exclusive read write lock.

  @subsubsection LO_TUT_3_CODE Sample code

  Consider the following code:

  @code
  mysql_mutex_t mutex_A;
  -- these are innodb locks, see storage/innobase/include/sync0rw.h --
  rw_lock_t latch_B;
  rw_lock_t latch_C;

  void func_red() {
    mysql_mutex_lock(&mutex_A);
    rw_lock_s_lock(&latch_B);
    do_something_red();
    rw_lock_s_unlock(&latch_B);
    mysql_mutex_unlock(&mutex_A);
  }

  void func_green() {
    rw_lock_sx_lock(&latch_B); -- Holds a SX lock.
    rw_lock_x_lock(&latch_B); -- Recursive, now holds a X lock.

    rw_lock_x_lock(&latch_C);

    do_something_green();

    rw_lock_x_unlock(&latch_C);

    rw_lock_x_unlock(&latch_B);
    rw_lock_sx_unlock(&latch_B);
  }

  void func_blue() {
    rw_lock_sx_lock(&latch_C);
    mysql_mutex_lock(&mutex_A);
    do_something_blue();
    mysql_mutex_unlock(&mutex_A);
    rw_lock_sx_unlock(&latch_C);
  }
  @endcode

  @subsubsection LO_TUT_3_GRAPH Dependency graph

  When executing func_red(), the tool detects the following dependencies:

  @verbatim
  ARC FROM "mutex/tutorial/mutex_A" TO "sxlock/tutorial/latch_B" OP "S"
  @endverbatim

  Likewise for func_green(), the dependencies found are:

  @verbatim
  ARC FROM "sxlock/tutorial/latch_B" STATE "SX" TO "sxlock/tutorial/latch_B" RECURSIVE OP "X"
  ARC FROM "sxlock/tutorial/latch_B" STATE "X" TO "sxlock/tutorial/latch_C" OP "X"
  @endverbatim

  Notice how the tool:
  - detects that some locks are recursive
  - infere the overall STATE of a lock based on all lock calls made.

  And finally for func_blue(), the tool detects:

  @verbatim
  ARC FROM "sxlock/tutorial/latch_C" STATE "SX" TO "mutex/tutorial/mutex_A"
  @endverbatim

  @subsubsection LO_TUT_3_REPORT Lock order report

  @verbatim
  Dumping classes for SCC 1:
  SCC Class sxlock/tutorial/latch_C
  SCC Class sxlock/tutorial/latch_B
  SCC Class mutex/tutorial/mutex_A

  Dumping nodes for SCC 1:
  SCC Node sxlock/tutorial/latch_C:+X Girth 5
  SCC Node sxlock/tutorial/latch_C:-SX Girth 5
  SCC Node sxlock/tutorial/latch_B:-X Girth 5
  SCC Node sxlock/tutorial/latch_B:+S Girth 5
  SCC Node mutex/tutorial/mutex_A Girth 5

  -- Note that these nodes exist, but are not part of the SCC: --
  Node sxlock/tutorial/latch_C:-X
  Node sxlock/tutorial/latch_C:+SX
  Node sxlock/tutorial/latch_C:+S
  Node sxlock/tutorial/latch_C:-S
  Node sxlock/tutorial/latch_B:+X
  Node sxlock/tutorial/latch_B:+SX
  Node sxlock/tutorial/latch_B:-SX
  Node sxlock/tutorial/latch_B:-S

  Dumping arcs for SCC 1:
  SCC ARC FROM "sxlock/tutorial/latch_C:+X" TO "sxlock/tutorial/latch_C:-SX" -- MICRO ARC
  SCC ARC FROM "sxlock/tutorial/latch_C:-SX" TO "mutex/tutorial/mutex_A"
  SCC ARC FROM "sxlock/tutorial/latch_B:-X" TO "sxlock/tutorial/latch_C:+X"
  SCC ARC FROM "sxlock/tutorial/latch_B:+S" TO "sxlock/tutorial/latch_B:-X" -- MICRO ARC
  SCC ARC FROM "mutex/tutorial/mutex_A" TO "sxlock/tutorial/latch_B:+S"

  -- Note that these arcs exist, but are not part of the SCC: --
  ARC FROM "sxlock/tutorial/latch_B:-SX" TO "sxlock/tutorial/latch_B:-X"

  SCC 1/1 summary:
   - Class Size 3
   - Node Size 5
   - Arc Count 3
   - Girth 5
   - Circumference 5

  @endverbatim

  The graphical representation of a shared exclusive lock is as follows:

  @dot
  digraph LockOrder {
    graph [ label="Lock Order sxlock block" ]

    subgraph cluster_latch_B {
      graph [ label="innodb rw_lock_t", rankdir=TB ]

      node [ label="+S" ] p_s;
      node [ label="-S" ] m_s;
      node [ label="+SX" ] p_sx;
      node [ label="-SX" ] m_sx;
      node [ label="+X" ] p_x;
      node [ label="-X" ] m_x;
      p_s -> m_x [style=dashed];
      p_sx -> m_x [style=dashed];
      p_x -> m_x [style=dashed];
      p_sx -> m_sx [style=dashed];
      p_x -> m_sx [style=dashed];
      p_x -> m_s [style=dashed];
    }
  }
  @enddot

  Using this graphical representation, the SCC reported can be represented as:

  @dot
  digraph LockOrder {
    graph [ label="Lock Order SCC", rankdir=LR ]

    mutex_A;

    subgraph cluster_latch_B {
      graph [ label="latch_B", rankdir=TB ]

      node [ label="+S" ] B_p_s;
      node [ label="-S" ] B_m_s;
      node [ label="+SX" ] B_p_sx;
      node [ label="-SX" ] B_m_sx;
      node [ label="+X" ] B_p_x;
      node [ label="-X" ] B_m_x;
      B_p_s -> B_m_x [style=dashed, penwidth=3];
      B_p_sx -> B_m_x [style=dashed];
      B_p_x -> B_m_x [style=dashed];
      B_p_sx -> B_m_sx [style=dashed];
      B_p_x -> B_m_sx [style=dashed];
      B_p_x -> B_m_s [style=dashed];
    }

    subgraph cluster_latch_C {
      graph [ label="latch_C", rankdir=TB ]

      node [ label="+S" ] C_p_s;
      node [ label="-S" ] C_m_s;
      node [ label="+SX" ] C_p_sx;
      node [ label="-SX" ] C_m_sx;
      node [ label="+X" ] C_p_x;
      node [ label="-X" ] C_m_x;
      C_p_s -> C_m_x [style=dashed];
      C_p_sx -> C_m_x [style=dashed];
      C_p_x -> C_m_x [style=dashed];
      C_p_sx -> C_m_sx [style=dashed];
      C_p_x -> C_m_sx [style=dashed, penwidth=3];
      C_p_x -> C_m_s [style=dashed];
    }

    mutex_A -> B_p_s [penwidth=3, label="func_red", color=red];
    B_m_sx -> B_m_x [label="func_green, recursive", color=green];
    B_m_x -> C_p_x [penwidth=3, label="func_green", color=green];
    C_m_sx -> mutex_A [penwidth=3, label="func_blue", color=blue];
  }
  @enddot

  There is a cycle of girth five involving the three nodes:
  - mutex_A
  - latch_B
  - latch_C

  By code review, this is correct:
  - func_red() will hold mutex_A, blocking func_blue()
  - func_blue() will hold a SX lock on latch_C, blocking func_green()
  - func_green() will hold a X lock on latch_B, blocking func_red()

  As can be found by reading the diagram, there are several options to break the deadlock:
  - the order of locks between mutex_A and latch_C can be reversed, breaking the loop,
  - func_green() could avoid taking a X lock on latch_B, and use a SX lock only.
  This breaks the cycle because a S lock (from func_red()) and a SX lock (from modified func_green())
  are not blocking each others, making the graph loop free.

  As a site note about the notation used,
  see how the recursive lock taken by func_green() in latch_B is represented:
  An "-SX" -> "-X" arc means that the already given SX lock ("-SX")
  is promoted to a given X lock ("-X").
*/
/* clang-format on */

/*
** ========================================================================
** SECTION 2: GENERAL DECLARATIONS
** ========================================================================
*/

#define safe_snprintf(B, S, F, ...) \
  {                                 \
    snprintf(B, S, F, __VA_ARGS__); \
    B[S - 1] = '\0';                \
  }

#define LO_MAX_THREAD_CLASS 100
#define LO_MAX_MUTEX_CLASS 300
#define LO_MAX_RWLOCK_CLASS 100
#define LO_MAX_COND_CLASS 100
#define LO_MAX_FILE_CLASS 100

/*
  Mutex class : 1 node
  Rwlock class :
   - prlock : 4 nodes
   - rwlock : 4 nodes
   - sxlock : 6 nodes
  Cond class : 1 node
  File class : 1 node
*/
#define LO_MAX_NODE_NUMBER                                            \
  (LO_MAX_MUTEX_CLASS + 6 * LO_MAX_RWLOCK_CLASS + LO_MAX_COND_CLASS + \
   LO_MAX_FILE_CLASS)

#define LO_THREAD_RANGE 0
#define LO_MUTEX_RANGE 1000
#define LO_RWLOCK_RANGE 2000
#define LO_COND_RANGE 3000
#define LO_FILE_RANGE 4000

#define LO_MAX_QNAME_LENGTH 128

/** Buffer size to safely print " STATE <state>" */
#define STATE_NAME_LENGTH 80
/** Buffer size to safely print " RECURSIVE OP <operation>" */
#define OPERATION_NAME_LENGTH 80

class LO_class;
struct tarjan_scc_state;
struct graph_search_state;
class LO_node;
class LO_node_finder;
class LO_arc;
class LO_graph;
class LO_lock;
class LO_thread_class;
class LO_thread;
class LO_mutex_class;
class LO_mutex;
class LO_mutex_locker;
class LO_mutex_lock;
class LO_rwlock_class;
class LO_rwlock_class_pr;
class LO_rwlock_class_rw;
class LO_rwlock_class_sx;
class LO_rwlock;
class LO_rwlock_pr;
class LO_rwlock_rw;
class LO_rwlock_sx;
class LO_rwlock_locker;
class LO_rwlock_lock;
class LO_rwlock_lock_pr;
class LO_rwlock_lock_rw;
class LO_rwlock_lock_sx;
class LO_cond_class;
class LO_cond;
class LO_cond_locker;
class LO_cond_wait;
class LO_file_class;
class LO_file;
class LO_file_locker;
class SCC_visitor;
class SCC_all;
class SCC_filter;
class LO_stack_trace;

typedef std::list<LO_mutex_lock *> LO_mutex_lock_list;
typedef std::list<LO_rwlock_lock *> LO_rwlock_lock_list;
typedef std::list<LO_node *> LO_node_list;
typedef std::list<LO_arc *> LO_arc_list;
typedef std::list<LO_authorised_arc *> LO_authorised_arc_list;
typedef std::list<LO_thread *> LO_thread_list;

/* Have to use a forward declaration to add MY_ATTRIBUTE */
static void print_file(FILE *file, const char *format, ...)
    MY_ATTRIBUTE((format(printf, 2, 3)));

static void debug_lock_order_break_here(const char *why);

static char *deep_copy_string(const char *src);
static void destroy_string(char *src);
static LO_authorised_arc *deep_copy_arc(const LO_authorised_arc *src);
static void destroy_arc(LO_authorised_arc *arc);

/*
** ========================================================================
** SECTION 3: GRAPH CLASSES DECLARATIONS
** ========================================================================
*/

enum LO_node_type {
  NODE_TYPE_MUTEX,
  NODE_TYPE_RWLOCK,
  NODE_TYPE_COND,
  NODE_TYPE_FILE
};

class LO_class {
 public:
  LO_class(const char *prefix, const char *category, const char *name);
  virtual ~LO_class() {}

  virtual const char *get_qname() const { return m_class_name; }

  virtual unsigned int get_unified_key() const = 0;

  unsigned int get_key() const { return m_key; }
  unsigned int get_chain_key() const { return m_chain_key; }
  void set_chain_key(unsigned int chain) { m_chain_key = chain; }

  virtual void add_to_graph(LO_graph *g) const = 0;
  virtual LO_node *get_state_node_by_name(const char *name) const = 0;
  virtual LO_node *get_operation_node_by_name(bool recursive, const char *state,
                                              const char *operation) const = 0;

  bool has_trace() const { return m_lo_flags & LO_FLAG_TRACE; }

  bool has_debug() const { return m_lo_flags & LO_FLAG_DEBUG; }

  void set_trace() { m_lo_flags |= LO_FLAG_TRACE; }

  void set_debug() { m_lo_flags |= LO_FLAG_DEBUG; }

 protected:
  unsigned int m_key;
  unsigned int m_chain_key;
  char m_class_name[LO_MAX_QNAME_LENGTH];

 private:
  int m_lo_flags;
};

struct tarjan_scc_state {
 public:
  tarjan_scc_state()
      : m_index(-1), m_low_index(-1), m_on_stack(false), m_scc_number(0) {}

  void clear() {
    m_index = -1;
    m_low_index = -1;
    m_on_stack = false;
    m_scc_number = 0;
    m_scc_girth = 0;
  }

  int m_index;
  int m_low_index;
  bool m_on_stack;

  /** Track which node is part of which SCC. */
  int m_scc_number;
  /** Cycle girth for this node. */
  int m_scc_girth;
};

struct graph_search_state {
 public:
  graph_search_state() : m_visited(false), m_value(0) {}

  void clear() {
    m_visited = false;
    m_value = 0;
  }

  bool m_visited;
  int m_value;
};

class LO_node {
 public:
  LO_node(LO_class *klass, LO_node_type node_type, const char *instrument,
          const char *category, const char *name, const char *substate,
          int flags);

  unsigned int get_node_index() const { return m_node_index; }

  LO_node_type get_node_type() const { return m_node_type; }

  virtual ~LO_node() {}

  const char *get_qname() const { return m_qname; }

  const char *get_short_name() const { return m_short_name; }

  void add_out(LO_arc *arc) { m_arcs_out.push_front(arc); }

  void add_in(LO_arc *arc) { m_arcs_in.push_front(arc); }

  const LO_arc_list &get_arcs_in() const { return m_arcs_in; }

  const LO_arc_list &get_arcs_out() const { return m_arcs_out; }

  void clear_arcs() {
    m_arcs_in.clear();
    m_arcs_out.clear();
  }

  LO_arc *find_edge_to(const LO_node *to) const;

  bool is_shared_exclusive() const { return m_flags & PSI_FLAG_RWLOCK_SX; }

  bool is_sink() const { return m_is_sink; }

  bool is_ignored() const { return m_is_ignored; }

  void set_sink() { m_is_sink = true; }

  void set_ignored() { m_is_ignored = true; }

  bool has_trace() const { return m_lo_flags & LO_FLAG_TRACE; }

  bool has_debug() const { return m_lo_flags & LO_FLAG_DEBUG; }

  void set_trace() { m_lo_flags |= LO_FLAG_TRACE; }

  void set_debug() { m_lo_flags |= LO_FLAG_DEBUG; }

  LO_class *get_class() const { return m_class; }

 public:
  tarjan_scc_state m_scc;
  graph_search_state m_search;

 private:
  unsigned int m_node_index;

  LO_node_type m_node_type;
  char m_qname[LO_MAX_QNAME_LENGTH];
  char m_short_name[LO_MAX_QNAME_LENGTH];
  LO_arc_list m_arcs_in;
  LO_arc_list m_arcs_out;
  /* Flags from the instrumentation (pfs) */
  int m_flags;
  /* Flags from lock order */
  int m_lo_flags;
  bool m_is_sink;
  bool m_is_ignored;
  LO_class *m_class;

  static unsigned int g_node_index_counter;
};

class LO_node_finder {
 public:
  static LO_node *find_state_node(const char *qname, const char *state);
  static LO_node *find_operation_node(const char *qname, bool recursive,
                                      const char *state, const char *operation);

 private:
  static LO_class *find_class(const char *qname);
};

class LO_arc {
 public:
  LO_arc(LO_node *from, LO_node *to, int flags, const char *constraint,
         const char *comment);

  ~LO_arc();

  LO_node *get_from() const { return m_from; }

  LO_node *get_to() const { return m_to; }

  bool is_micro() const { return m_lo_flags & LO_FLAG_MICRO; }

  bool has_trace() const { return m_lo_flags & LO_FLAG_TRACE; }

  bool has_debug() const { return m_lo_flags & LO_FLAG_DEBUG; }

  bool has_loop() const { return m_lo_flags & LO_FLAG_LOOP; }

  void merge_flags(int flags) { m_lo_flags |= flags; }

  bool has_self() const { return m_self; }

  const char *get_constraint() const { return m_constraint; }

  const char *get_comment() const { return m_comment; }

 private:
  LO_node *m_from;
  LO_node *m_to;
  int m_lo_flags;
  bool m_self;
  char *m_constraint;
  char *m_comment;
};

class LO_graph {
 public:
  LO_graph();
  ~LO_graph();

  void check_mutex(LO_thread *thread, const LO_lock *old_lock,
                   const LO_mutex_lock *new_lock);

  void check_rwlock(LO_thread *thread, const LO_lock *old_lock,
                    const LO_rwlock_lock *new_lock, PSI_rwlock_operation op);

  void check_file(LO_thread *thread, const LO_lock *old_lock,
                  const LO_file_class *new_file);

  void check_cond(LO_thread *thread, const LO_lock *old_lock,
                  const LO_cond_wait *new_cond);

  void check_common(LO_thread *thread, const char *from_class_name,
                    const char *from_state_name, const LO_node *from_node,
                    const LO_lock *old_lock, const char *to_class_name,
                    bool recursive, const char *to_operation_name,
                    const LO_node *to_node, const LO_lock *new_lock);

  LO_node *find_state_node(const char *qname, const char *state);
  LO_node *find_operation_node(const char *qname, bool recursive,
                               const char *state, const char *operation);
  LO_node *find_node(const char *qname);
  void add_node(LO_node *node);
  void add_class(const char *class_name);

  void add_arc(const LO_authorised_arc *arc);
  void add_arc(LO_node *from, LO_node *to, bool recursive, int flags,
               const char *constraint, const char *comment);
  void add_unresolved_arc(LO_authorised_arc *arc);

  void dump_txt();

  void scc_util(const SCC_visitor *v, int *discovery_time, int *scc_count,
                LO_node *n, std::stack<LO_node *> *st);
  int compute_scc(const SCC_visitor *v);
  void compute_scc_girth(int iter_scc, const SCC_visitor *v);
  void compute_node_girth(int iter_scc, const SCC_visitor *v, LO_node *start);
  void dump_scc(FILE *out, int number_of_scc, bool print_loop_flag);
  void dump_one_scc(FILE *out, int scc, int number_of_scc,
                    bool print_loop_flag);

 private:
  LO_node_list m_nodes;
  LO_arc_list m_arcs;
  LO_authorised_arc_list m_unresolved_arcs;

  LO_arc *m_arc_matrix[LO_MAX_NODE_NUMBER][LO_MAX_NODE_NUMBER];
};

class LO_lock {
 public:
  LO_lock(const char *src_file, int src_line, size_t event_id);
  virtual ~LO_lock();

  virtual const char *get_class_name() const = 0;
  virtual LO_node *get_state_node() const = 0;
  virtual const char *get_state_name() const = 0;

  const char *get_locking_src_file() const { return m_locking_src_file; }

  int get_locking_src_line() const { return m_locking_src_line; }

  size_t get_event_id() const { return m_event_id; }

  const LO_stack_trace *get_stack_trace() const { return m_stack; }

  void record_stack_trace();

  void record_statement_text(const char *text, int length);

  const char *get_statement_text(int *length) const {
    *length = m_locking_statement_text_length;
    return m_locking_statement_text;
  }

 private:
  const char *m_locking_src_file;
  int m_locking_src_line;
  size_t m_event_id;
  LO_stack_trace *m_stack;
  const char *m_locking_statement_text;
  int m_locking_statement_text_length;

 protected:
  my_thread_t m_locking_pthread;
};

class LO_thread_class {
 public:
  static LO_thread_class *find(int key);
  static LO_thread_class *find_by_name(const char *category, const char *name);
  static void destroy_all();

  LO_thread_class(const char *category, const char *name);

  virtual ~LO_thread_class();

  virtual unsigned int get_unified_key() const {
    return m_key + LO_THREAD_RANGE;
  }

  unsigned int get_key() const { return m_key; }
  unsigned int get_chain_key() const { return m_chain_key; }
  void set_chain_key(unsigned int chain) { m_chain_key = chain; }

  const char *get_qname() const { return m_qname; }

 private:
  static unsigned int m_counter;
  static LO_thread_class *m_array[LO_MAX_THREAD_CLASS];

  unsigned int m_key;
  unsigned int m_chain_key;
  char m_qname[LO_MAX_QNAME_LENGTH];
};

class LO_thread {
 public:
  static LO_thread_list g_threads;

  LO_thread(const LO_thread_class *klass)
      : m_runaway(false),
        m_class(klass),
        m_statement_text(nullptr),
        m_statement_text_length(0),
        m_event_counter(0),
        m_chain(nullptr) {}

  ~LO_thread();

  void check_locks(const LO_mutex_lock *new_lock);
  void check_locks(const LO_rwlock_lock *new_lock, PSI_rwlock_operation op);
  void check_locks(const LO_file_class *new_file);
  void check_cond_wait(const LO_cond_wait *new_lock);
  void check_signal_broadcast(const char *operation, LO_cond *cond);
  void check_destroy();
  void print_all_locks(FILE *out);
  void print_mutex_locks(FILE *out);
  void print_rwlock_locks(FILE *out);

  static void dump(FILE *out, LO_thread *thread);
  void dump(FILE *out) const;

  bool satisfy_constraint(const char *constraint);
  bool satisfy_mutex_constraint(const char *constraint);
  bool satisfy_rwlock_constraint(const char *constraint);

  static void add_mutex_lock(LO_thread *thread, LO_mutex *mutex,
                             const char *src_file, int src_line);
  static void remove_mutex_lock(LO_thread *thread, LO_mutex *mutex);

  static void add_rwlock_lock(LO_thread *thread, LO_rwlock *rwlock,
                              PSI_rwlock_operation op, const char *src_file,
                              int src_line);
  static void remove_rwlock_lock(LO_thread *thread, LO_rwlock *rwlock,
                                 PSI_rwlock_operation unlock_op);

  const LO_thread_class *get_class() const { return m_class; }

  void set_statement_text(const char *text, int length) {
    m_statement_text = text;
    m_statement_text_length = length;
  }

  const char *get_statement_text(int *length) {
    *length = m_statement_text_length;
    return m_statement_text;
  }

  size_t new_event_id() { return ++m_event_counter; }

  void clear_all_locks();
  void set_runaway();

 private:
  bool m_runaway;
  const LO_thread_class *m_class;
  LO_mutex_lock_list m_mutex_locks;
  LO_rwlock_lock_list m_rwlock_locks;
  const char *m_statement_text;
  int m_statement_text_length;
  size_t m_event_counter;

 public:
  PSI_thread *m_chain;
};

class LO_mutex_class : public LO_class {
 public:
  static LO_mutex_class *find_by_key(int key);
  static LO_mutex_class *find_by_name(const char *category, const char *name);
  static LO_mutex_class *find_by_qname(const char *qname);
  static void destroy_all();

  LO_mutex_class(const char *category, const char *name, int flags);
  virtual ~LO_mutex_class() override;

  virtual unsigned int get_unified_key() const override {
    return m_key + LO_MUTEX_RANGE;
  }

  virtual void add_to_graph(LO_graph *g) const override;

  virtual LO_node *get_state_node_by_name(const char *name) const override;

  virtual LO_node *get_operation_node_by_name(
      bool recursive, const char *state, const char *operation) const override;

  LO_node *get_node() const { return m_node; }

 private:
  static unsigned int m_counter;
  static LO_mutex_class *m_array[LO_MAX_MUTEX_CLASS];

  LO_node *m_node;
};

class LO_mutex {
 public:
  LO_mutex(const LO_mutex_class *klass)
      : m_class(klass), m_lock(nullptr), m_chain(nullptr) {}

  ~LO_mutex() {}

  const LO_mutex_class *get_class() const { return m_class; }

  LO_mutex_lock *get_lock() const { return m_lock; }

  void set_lock(LO_mutex_lock *lock);

  void set_unlocked() { m_lock = nullptr; }

 private:
  const LO_mutex_class *m_class;
  LO_mutex_lock *m_lock;

 public:
  PSI_mutex *m_chain;
};

class LO_mutex_locker {
 public:
  LO_mutex_locker(LO_thread *thread, LO_mutex *mutex)
      : m_thread(thread), m_mutex(mutex), m_chain(nullptr) {}

  void start(const char *src_file, int src_line);

  void end();

  ~LO_mutex_locker() {}

  const char *get_src_file() const { return m_src_file; }

  int get_src_line() const { return m_src_line; }

 private:
  LO_thread *m_thread;
  LO_mutex *m_mutex;
  const char *m_src_file;
  int m_src_line;

 public:
  PSI_mutex_locker *m_chain;
};

class LO_mutex_lock : public LO_lock {
 public:
  LO_mutex_lock(LO_mutex *mutex, const char *src_file, int src_line,
                LO_thread *thread);
  ~LO_mutex_lock() override {}

  virtual const char *get_class_name() const override;

  virtual LO_node *get_state_node() const override;

  virtual const char *get_state_name() const override;

  LO_node *get_node() const;

  LO_mutex *get_mutex() const { return m_mutex; }

  LO_thread *get_thread() { return m_thread; }

  virtual void dump(FILE *out) const;

 private:
  LO_mutex *m_mutex;
  /** Owning thread. Could be null for uninstrumented threads. */
  LO_thread *m_thread;
};

class LO_rwlock_class : public LO_class {
 public:
  static LO_rwlock_class *find_by_key(int key);
  static LO_rwlock_class *find_by_name(const char *category, const char *name,
                                       int flags);
  static LO_rwlock_class *find_by_qname(const char *qname);

  static LO_rwlock_class *create(const char *category, const char *name,
                                 int flags);
  static void destroy_all();

  static bool get_state_by_name(const char *name, PSI_rwlock_operation *state);
  static bool get_operation_by_name(const char *name, PSI_rwlock_operation *op);

  virtual ~LO_rwlock_class() override;

  virtual unsigned int get_unified_key() const override {
    return m_key + LO_RWLOCK_RANGE;
  }

  virtual LO_node *get_state_node_by_name(const char *name) const override;
  virtual LO_node *get_operation_node_by_name(
      bool recursive, const char *state, const char *operation) const override;
  virtual LO_node *get_state_node(PSI_rwlock_operation state) const = 0;
  virtual LO_node *get_operation_node(bool recursive,
                                      PSI_rwlock_operation state,
                                      PSI_rwlock_operation op) const = 0;
  virtual const char *get_operation_name(PSI_rwlock_operation op) const = 0;

  virtual void add_to_graph(LO_graph *g) const override = 0;

  virtual LO_rwlock *build_instance() = 0;

 protected:
  LO_rwlock_class(const char *prefix, const char *category, const char *name,
                  int flags);

 private:
  static unsigned int m_counter;
  static LO_rwlock_class *m_array[LO_MAX_RWLOCK_CLASS];
};

class LO_rwlock_class_pr : public LO_rwlock_class {
 public:
  LO_rwlock_class_pr(const char *category, const char *name, int flags);
  ~LO_rwlock_class_pr() override;

  virtual LO_node *get_state_node(PSI_rwlock_operation state) const override;

  virtual LO_node *get_operation_node(bool recursive,
                                      PSI_rwlock_operation state,
                                      PSI_rwlock_operation op) const override;

  virtual const char *get_operation_name(
      PSI_rwlock_operation op) const override;

  virtual void add_to_graph(LO_graph *g) const override;

  virtual LO_rwlock *build_instance() override;

 private:
  /** Node "+R". */
  LO_node *m_node_p_r;
  /** Node "-R". */
  LO_node *m_node_m_r;
  /** Node "+W". */
  LO_node *m_node_p_w;
  /** Node "-W". */
  LO_node *m_node_m_w;
};

class LO_rwlock_class_rw : public LO_rwlock_class {
 public:
  LO_rwlock_class_rw(const char *category, const char *name, int flags);
  ~LO_rwlock_class_rw() override;

  virtual LO_node *get_state_node(PSI_rwlock_operation state) const override;

  virtual LO_node *get_operation_node(bool recursive,
                                      PSI_rwlock_operation state,
                                      PSI_rwlock_operation op) const override;

  virtual const char *get_operation_name(
      PSI_rwlock_operation op) const override;

  virtual void add_to_graph(LO_graph *g) const override;

  virtual LO_rwlock *build_instance() override;

 private:
  /** Node "+R". */
  LO_node *m_node_p_r;
  /** Node "-R". */
  LO_node *m_node_m_r;
  /** Node "+W". */
  LO_node *m_node_p_w;
  /** Node "-W". */
  LO_node *m_node_m_w;
};

class LO_rwlock_class_sx : public LO_rwlock_class {
 public:
  LO_rwlock_class_sx(const char *category, const char *name, int flags);
  ~LO_rwlock_class_sx() override;

  virtual LO_node *get_state_node(PSI_rwlock_operation state) const override;

  virtual LO_node *get_operation_node(bool recursive,
                                      PSI_rwlock_operation state,
                                      PSI_rwlock_operation op) const override;

  virtual const char *get_operation_name(
      PSI_rwlock_operation op) const override;

  virtual void add_to_graph(LO_graph *g) const override;

  virtual LO_rwlock *build_instance() override;

 private:
  /** Node "+S". */
  LO_node *m_node_p_s;
  /** Node "-S". */
  LO_node *m_node_m_s;
  /** Node "+SX". */
  LO_node *m_node_p_sx;
  /** Node "-SX". */
  LO_node *m_node_m_sx;
  /** Node "+X". */
  LO_node *m_node_p_x;
  /** Node "-X". */
  LO_node *m_node_m_x;
};

class LO_rwlock {
 public:
  LO_rwlock(const LO_rwlock_class *klass) : m_class(klass), m_chain(nullptr) {}

  virtual ~LO_rwlock() {}

  virtual LO_rwlock_lock *build_lock(const char *src_file, int src_line,
                                     LO_thread *thread) = 0;

  const LO_rwlock_class *get_class() const { return m_class; }

  void add_lock(LO_rwlock_lock *lock);

  void remove_lock(LO_rwlock_lock *lock);

 private:
  const LO_rwlock_class *m_class;
  LO_rwlock_lock_list m_rwlock_locks;

 public:
  PSI_rwlock *m_chain;
};

class LO_rwlock_pr : public LO_rwlock {
 public:
  LO_rwlock_pr(const LO_rwlock_class_pr *klass) : LO_rwlock(klass) {}
  ~LO_rwlock_pr() {}

  virtual LO_rwlock_lock *build_lock(const char *src_file, int src_line,
                                     LO_thread *thread);
};

class LO_rwlock_rw : public LO_rwlock {
 public:
  LO_rwlock_rw(const LO_rwlock_class_rw *klass) : LO_rwlock(klass) {}
  ~LO_rwlock_rw() {}

  virtual LO_rwlock_lock *build_lock(const char *src_file, int src_line,
                                     LO_thread *thread);
};

class LO_rwlock_sx : public LO_rwlock {
 public:
  LO_rwlock_sx(const LO_rwlock_class_sx *klass) : LO_rwlock(klass) {}
  ~LO_rwlock_sx() {}

  virtual LO_rwlock_lock *build_lock(const char *src_file, int src_line,
                                     LO_thread *thread);
};

class LO_rwlock_locker {
 public:
  LO_rwlock_locker(LO_thread *thread, LO_rwlock *rwlock)
      : m_thread(thread), m_rwlock(rwlock), m_chain(nullptr) {}

  void start(PSI_rwlock_operation op, const char *src_file, int src_line);

  void end();

  ~LO_rwlock_locker() {}

  const char *get_src_file() const { return m_src_file; }

  int get_src_line() const { return m_src_line; }

 private:
  LO_thread *m_thread;
  LO_rwlock *m_rwlock;
  PSI_rwlock_operation m_op;
  const char *m_src_file;
  int m_src_line;

 public:
  PSI_rwlock_locker *m_chain;
};

class LO_rwlock_lock : public LO_lock {
 public:
  LO_rwlock_lock(LO_rwlock *rwlock, const char *src_file, int src_line,
                 LO_thread *thread);
  virtual ~LO_rwlock_lock() override {}

  virtual const char *get_class_name() const override;

  virtual LO_node *get_state_node() const override;

  LO_node *get_operation_node(bool recursive, PSI_rwlock_operation op) const;

  const char *get_operation_name(PSI_rwlock_operation op) const;

  const LO_thread *get_thread() const { return m_thread; }
  LO_rwlock *get_rwlock() { return m_rwlock; }

  virtual void set_locked(PSI_rwlock_operation op, const char *src_file,
                          int src_line) = 0;
  virtual void merge_lock(PSI_rwlock_operation op, const char *src_file,
                          int src_line) = 0;
  virtual bool set_unlocked(PSI_rwlock_operation op) = 0;
  virtual PSI_rwlock_operation get_state() const = 0;

  const char *get_op() const;

  virtual void dump(FILE *out) const;

 private:
  LO_thread *m_thread;
  LO_rwlock *m_rwlock;
};

class LO_rwlock_lock_pr : public LO_rwlock_lock {
 public:
  LO_rwlock_lock_pr(LO_rwlock *rwlock, const char *src_file, int src_line,
                    LO_thread *thread);
  ~LO_rwlock_lock_pr() {}

  virtual void set_locked(PSI_rwlock_operation op, const char *src_file,
                          int src_line);

  virtual void merge_lock(PSI_rwlock_operation op, const char *src_file,
                          int src_line);

  virtual bool set_unlocked(PSI_rwlock_operation op);

  virtual PSI_rwlock_operation get_state() const;

  virtual const char *get_state_name() const;

 private:
  int m_read_count;
  int m_write_count;
  const char *m_write_src_file;
  int m_write_src_line;
  const char *m_read_src_file;
  int m_read_src_line;
};

class LO_rwlock_lock_rw : public LO_rwlock_lock {
 public:
  LO_rwlock_lock_rw(LO_rwlock *rwlock, const char *src_file, int src_line,
                    LO_thread *thread);
  ~LO_rwlock_lock_rw() {}

  virtual void set_locked(PSI_rwlock_operation op, const char *src_file,
                          int src_line);

  virtual void merge_lock(PSI_rwlock_operation op, const char *src_file,
                          int src_line);

  virtual bool set_unlocked(PSI_rwlock_operation op);

  virtual PSI_rwlock_operation get_state() const;

  virtual const char *get_state_name() const;

 private:
  int m_read_count;
  int m_write_count;
  const char *m_write_src_file;
  int m_write_src_line;
  const char *m_read_src_file;
  int m_read_src_line;
};

class LO_rwlock_lock_sx : public LO_rwlock_lock {
 public:
  LO_rwlock_lock_sx(LO_rwlock *rwlock, const char *src_file, int src_line,
                    LO_thread *thread);
  ~LO_rwlock_lock_sx() {}

  virtual void set_locked(PSI_rwlock_operation op, const char *src_file,
                          int src_line);

  virtual void merge_lock(PSI_rwlock_operation op, const char *src_file,
                          int src_line);

  virtual bool set_unlocked(PSI_rwlock_operation op);

  virtual PSI_rwlock_operation get_state() const;

  virtual const char *get_state_name() const;

 private:
  int m_s_count;
  int m_sx_count;
  int m_x_count;
  const char *m_s_src_file;
  int m_s_src_line;
  const char *m_sx_src_file;
  int m_sx_src_line;
  const char *m_x_src_file;
  int m_x_src_line;
};

class LO_cond_class : public LO_class {
 public:
  static LO_cond_class *find_by_key(int key);
  static LO_cond_class *find_by_name(const char *category, const char *name);
  static LO_cond_class *find_by_qname(const char *qname);
  static void destroy_all();

  LO_cond_class(const char *category, const char *name, int flags);
  virtual ~LO_cond_class() override;

  virtual unsigned int get_unified_key() const override {
    return m_key + LO_COND_RANGE;
  }

  LO_node *get_node() const { return m_node; }

  virtual void add_to_graph(LO_graph *g) const override;

  virtual LO_node *get_state_node_by_name(const char *name) const override;

  virtual LO_node *get_operation_node_by_name(
      bool recursive, const char *state, const char *operation) const override;

  const LO_mutex_class *get_mutex_class() const { return m_mutex_class; }

  void set_mutex_class(const LO_mutex_class *mutex_class);

  void set_unfair() { m_unfair = true; }
  bool is_unfair() const { return m_unfair; }

 private:
  static unsigned int m_counter;
  static LO_cond_class *m_array[LO_MAX_COND_CLASS];

  const LO_mutex_class *m_mutex_class;
  bool m_unfair;

  LO_node *m_node;
};

class LO_cond {
 public:
  LO_cond(const LO_cond_class *klass);

  ~LO_cond() {}

  const LO_cond_class *get_class() const { return m_class; }

 private:
  const LO_cond_class *m_class;

 public:
  PSI_cond *m_chain;
};

class LO_cond_locker {
 public:
  LO_cond_locker(LO_thread *thread, LO_cond *cond, LO_mutex *mutex)
      : m_thread(thread), m_cond(cond), m_mutex(mutex), m_chain(nullptr) {}

  void start(const char *src_file, int src_line);

  void end();

  LO_mutex *get_mutex() { return m_mutex; }

  ~LO_cond_locker() {}

  const LO_cond *get_cond() const { return m_cond; }

  const char *get_src_file() const { return m_src_file; }

  int get_src_line() const { return m_src_line; }

 private:
  LO_thread *m_thread;
  LO_cond *m_cond;
  LO_mutex *m_mutex;
  const char *m_src_file;
  int m_src_line;

 public:
  PSI_cond_locker *m_chain;
};

class LO_cond_wait : public LO_lock {
 public:
  LO_cond_wait(LO_mutex *mutex, LO_cond *cond, const char *src_file,
               int src_line, LO_thread *thread);

  virtual const char *get_class_name() const override;

  virtual LO_node *get_state_node() const override;

  virtual const char *get_state_name() const override;

  virtual LO_node *get_node() const;

  const LO_cond *get_cond() const { return m_cond; }

  const LO_thread *get_thread() const { return m_thread; }

  virtual void dump(FILE *out) const;

  ~LO_cond_wait() override {}

 private:
  LO_mutex *m_mutex;
  LO_cond *m_cond;
  LO_thread *m_thread;
};

class LO_file_class : public LO_class {
 public:
  static LO_file_class *find_by_key(int key);
  static LO_file_class *find_by_name(const char *category, const char *name);
  static LO_file_class *find_by_qname(const char *qname);
  static void destroy_all();

  LO_file_class(const char *category, const char *name, int flags);
  virtual ~LO_file_class() override;

  virtual unsigned int get_unified_key() const override {
    return m_key + LO_FILE_RANGE;
  }

  LO_node *get_node() const { return m_node; }

  virtual void add_to_graph(LO_graph *g) const override;

  virtual LO_node *get_state_node_by_name(const char *name) const override;

  virtual LO_node *get_operation_node_by_name(
      bool recursive, const char *state, const char *operation) const override;

 private:
  static unsigned int m_counter;
  static LO_file_class *m_array[LO_MAX_FILE_CLASS];

  LO_node *m_node;
};

class LO_file {
 public:
  LO_file(const LO_file_class *klass)
      : m_class(klass), m_chain(nullptr), m_bound(false) {}

  ~LO_file() { DBUG_ASSERT(!m_bound); }

  const LO_file_class *get_class() const { return m_class; }

 private:
  const LO_file_class *m_class;

 public:
  PSI_file *m_chain;
  bool m_bound;
};

class LO_file_locker {
 public:
  LO_file_locker() {}
  ~LO_file_locker() {}

  LO_thread *m_thread;
  const LO_file_class *m_class;
  LO_file *m_file;
  const char *m_name;
  PSI_file_operation m_operation;
  PSI_file_locker_state *m_chain_state;
  PSI_file_locker *m_chain_locker;
};

class SCC_visitor {
 public:
  SCC_visitor() {}
  virtual ~SCC_visitor() {}

  virtual bool accept_node(LO_node *) const = 0;
  virtual bool accept_arc(LO_arc *) const = 0;
};

class SCC_all : public SCC_visitor {
 public:
  SCC_all() {}
  virtual ~SCC_all() override {}

  virtual bool accept_node(LO_node *) const override { return true; }
  virtual bool accept_arc(LO_arc *) const override { return true; }
};

class SCC_filter : public SCC_visitor {
 public:
  SCC_filter() {}
  virtual ~SCC_filter() override {}

  virtual bool accept_node(LO_node *node) const override;
  virtual bool accept_arc(LO_arc *arc) const override;
};

class LO_stack_trace {
 public:
  LO_stack_trace();

  ~LO_stack_trace() {}

  void print(FILE *out) const;

 private:
  void *m_addrs_array[128];
  int m_array_size;
};

/*
** ========================================================================
** SECTION 4: GLOBAL VARIABLES
** ========================================================================
*/

FILE *out_log = nullptr;
FILE *out_txt = nullptr;

native_mutex_t serialize;
native_mutex_t serialize_logs;

bool check_activated = false;

static int debugger_calls = 0;
static uint sanity_mutex_lock_limit = 100;
// static uint sanity_rwlock_lock_limit= 100;

LO_graph *global_graph = nullptr;

/*
** ========================================================================
** SECTION 5: GRAPH CLASSES IMPLEMENTATION
** ========================================================================
*/

LO_class::LO_class(const char *prefix, const char *category, const char *name)
    : m_key(0), m_chain_key(0), m_lo_flags(0) {
  safe_snprintf(m_class_name, sizeof(m_class_name), "%s/%s/%s", prefix,
                category, name);
}

unsigned int LO_node::g_node_index_counter = 0;

LO_node::LO_node(LO_class *klass, LO_node_type node_type,
                 const char *instrument, const char *category, const char *name,
                 const char *substate, int flags) {
  m_node_type = node_type;
  if (substate != nullptr) {
    safe_snprintf(m_qname, sizeof(m_qname), "%s/%s/%s:%s", instrument, category,
                  name, substate);
  } else {
    safe_snprintf(m_qname, sizeof(m_qname), "%s/%s/%s", instrument, category,
                  name);
  }

  safe_snprintf(m_short_name, sizeof(m_short_name), "%s/%s", category, name);

  m_class = klass;
  m_flags = flags;
  m_lo_flags = 0;
  m_is_sink = false;
  m_is_ignored = false;
  m_node_index = g_node_index_counter++;
}

LO_arc *LO_node::find_edge_to(const LO_node *to) const {
  LO_arc_list::const_iterator it;
  LO_arc *arc;
  LO_node *n;

  DBUG_ASSERT(to != nullptr);

  for (it = m_arcs_out.begin(); it != m_arcs_out.end(); it++) {
    arc = *it;
    n = arc->get_to();
    DBUG_ASSERT(n != nullptr);
    if (strcmp(to->get_qname(), n->get_qname()) == 0) {
      return arc;
    }
  }

  return nullptr;
}

LO_class *LO_node_finder::find_class(const char *qname) {
  LO_class *klass = nullptr;

  // TODO: use a hash instead.
  if (strncmp("mutex/", qname, 6) == 0) {
    klass = LO_mutex_class::find_by_qname(qname);
  } else if (strncmp("prlock/", qname, 7) == 0) {
    klass = LO_rwlock_class::find_by_qname(qname);
  } else if (strncmp("rwlock/", qname, 7) == 0) {
    klass = LO_rwlock_class::find_by_qname(qname);
  } else if (strncmp("sxlock/", qname, 7) == 0) {
    klass = LO_rwlock_class::find_by_qname(qname);
  } else if (strncmp("cond/", qname, 5) == 0) {
    klass = LO_cond_class::find_by_qname(qname);
  } else if (strncmp("file/", qname, 5) == 0) {
    klass = LO_file_class::find_by_qname(qname);
  }

  return klass;
}

LO_node *LO_node_finder::find_state_node(const char *qname, const char *state) {
  LO_node *n = nullptr;
  const LO_class *klass = find_class(qname);

  if (klass != nullptr) {
    n = klass->get_state_node_by_name(state);
    if (n == nullptr) {
      print_file(out_log, "Error: malformed from node, class %s, state %s\n",
                 qname, state);
    }
  }
  return n;
}

LO_node *LO_node_finder::find_operation_node(const char *qname, bool recursive,
                                             const char *state,
                                             const char *operation) {
  LO_node *n = nullptr;
  const LO_class *klass = find_class(qname);

  if (klass != nullptr) {
    n = klass->get_operation_node_by_name(recursive, state, operation);
    if (n == nullptr) {
      print_file(out_log, "Error: malformed to node, class %s, operation %s\n",
                 qname, operation);
    }
  }
  return n;
}

LO_arc::LO_arc(LO_node *from, LO_node *to, int flags, const char *constraint,
               const char *comment)
    : m_from(from), m_to(to), m_lo_flags(flags) {
  m_self = (from == to);
  m_constraint = deep_copy_string(constraint);
  m_comment = deep_copy_string(comment);
}

LO_arc::~LO_arc() {
  destroy_string(m_constraint);
  destroy_string(m_comment);
}

LO_graph::LO_graph() {
  unsigned int i;
  unsigned int j;

  for (i = 0; i < LO_MAX_NODE_NUMBER; i++) {
    for (j = 0; j < LO_MAX_NODE_NUMBER; j++) {
      m_arc_matrix[i][j] = nullptr;
    }
  }
}

LO_graph::~LO_graph() {
  // 1: Destroy unresolved arcs.

  LO_authorised_arc_list::iterator it;
  LO_authorised_arc *unresolved_arc;

  for (it = m_unresolved_arcs.begin(); it != m_unresolved_arcs.end();
       /* nothing */) {
    unresolved_arc = *it;
    it = m_unresolved_arcs.erase(it);
    destroy_arc(unresolved_arc);
  }

  // 2: Disconnect arcs from nodes.

  LO_node_list::const_iterator node_it;
  LO_node *n;

  for (node_it = m_nodes.begin(); node_it != m_nodes.end(); node_it++) {
    n = *node_it;
    n->clear_arcs();
  }

  // 3: Destroy arcs.

  LO_arc_list::iterator arc_it;
  LO_arc *arc;

  for (arc_it = m_arcs.begin(); arc_it != m_arcs.end(); /* nothing */) {
    arc = *arc_it;
    arc_it = m_arcs.erase(arc_it);
    delete arc;
  }
}

void LO_graph::check_mutex(LO_thread *thread, const LO_lock *old_lock,
                           const LO_mutex_lock *new_lock) {
  bool recursive = (old_lock == new_lock) ? true : false;
  DBUG_ASSERT(!recursive);
  const char *from_class_name = old_lock->get_class_name();
  const char *from_class_state = old_lock->get_state_name();
  const LO_node *from_node = old_lock->get_state_node();
  const char *to_class_name = new_lock->get_node()->get_qname();
  const LO_node *to_node = new_lock->get_node();

  check_common(thread, from_class_name, from_class_state, from_node, old_lock,
               to_class_name, recursive, nullptr, to_node, new_lock);
}

void LO_graph::check_rwlock(LO_thread *thread, const LO_lock *old_lock,
                            const LO_rwlock_lock *new_lock,
                            PSI_rwlock_operation op) {
  bool recursive = (old_lock == new_lock) ? true : false;
  const char *from_class_name = old_lock->get_class_name();
  const char *from_class_state = old_lock->get_state_name();
  const LO_node *from_node = old_lock->get_state_node();
  const char *to_class_name = new_lock->get_class_name();
  const char *to_class_operation = new_lock->get_operation_name(op);
  const LO_node *to_node = new_lock->get_operation_node(recursive, op);

  check_common(thread, from_class_name, from_class_state, from_node, old_lock,
               to_class_name, recursive, to_class_operation, to_node, new_lock);
}

void LO_graph::check_file(LO_thread *thread, const LO_lock *old_lock,
                          const LO_file_class *new_file) {
  const char *from_class_name = old_lock->get_class_name();
  const char *from_class_state = old_lock->get_state_name();
  const LO_node *from_node = old_lock->get_state_node();
  const char *to_class_name = new_file->get_qname();
  const LO_node *to_node = new_file->get_node();

  check_common(thread, from_class_name, from_class_state, from_node, old_lock,
               to_class_name, false, nullptr, to_node, nullptr);
}

void LO_graph::check_cond(LO_thread *thread, const LO_lock *old_lock,
                          const LO_cond_wait *new_lock) {
  const char *from_class_name = old_lock->get_class_name();
  const char *from_class_state = old_lock->get_state_name();
  const LO_node *from_node = old_lock->get_state_node();
  const char *to_class_name = new_lock->get_class_name();
  const LO_node *to_node = new_lock->get_node();

  check_common(thread, from_class_name, from_class_state, from_node, old_lock,
               to_class_name, false, nullptr, to_node, nullptr);
}

void LO_graph::check_common(LO_thread *thread, const char *from_class_name,
                            const char *from_state_name,
                            const LO_node *from_node, const LO_lock *old_lock,
                            const char *to_class_name, bool recursive,
                            const char *to_operation_name,
                            const LO_node *to_node,
                            const LO_lock *new_lock MY_ATTRIBUTE((unused))) {
  DBUG_ASSERT(thread != nullptr);
  DBUG_ASSERT(from_class_name != nullptr);
  /* from_state_name can be null. */
  DBUG_ASSERT(old_lock != nullptr);
  DBUG_ASSERT(to_class_name != nullptr);
  /* to_operation_name can be null. */

  if (from_node == nullptr) {
    print_file(
        out_log,
        "Error: FROM node does not fit into the model, illegal state.\n");
    print_file(out_log, "- node class : %s\n", from_class_name);
    print_file(out_log, "- node state : %s\n", from_state_name);
    print_file(out_log, "- recursive op : %s\n", recursive ? "true" : "false");
    return;
  }

  if (to_node == nullptr) {
    print_file(
        out_log,
        "Error: TO node does not fit into the model, illegal operation.\n");
    print_file(out_log, "- node class : %s\n", to_class_name);
    print_file(out_log, "- operation : %s\n", to_operation_name);
    return;
  }

  // DBUG_ASSERT(new_lock != nullptr);

  if (to_node->is_sink()) {
    return;
  }

  if (to_node->is_ignored()) {
    return;
  }

  LO_arc *arc;
  unsigned int from_index = from_node->get_node_index();
  unsigned int to_index = to_node->get_node_index();

  DBUG_ASSERT(from_index < LO_MAX_NODE_NUMBER);
  DBUG_ASSERT(to_index < LO_MAX_NODE_NUMBER);
  arc = m_arc_matrix[from_index][to_index];

  if (arc == nullptr) {
    char buff_state[STATE_NAME_LENGTH];
    char buff_operation[OPERATION_NAME_LENGTH];
    char buff[1024];

    if (from_state_name != nullptr) {
      safe_snprintf(buff_state, sizeof(buff_state), " STATE \"%s\"",
                    from_state_name);
    } else {
      buff_state[0] = '\0';
    }

    if (to_operation_name != nullptr) {
      if (recursive) {
        safe_snprintf(buff_operation, sizeof(buff_operation),
                      " RECURSIVE OP \"%s\"", to_operation_name);
      } else {
        safe_snprintf(buff_operation, sizeof(buff_operation), " OP \"%s\"",
                      to_operation_name);
      }
    } else {
      buff_operation[0] = '\0';
    }

    /* Print in a friendly format, to add the declaration back. */
    safe_snprintf(buff, sizeof(buff),
                  "MISSING: ARC FROM \"%s\"%s TO \"%s\"%s\n", from_class_name,
                  buff_state, to_class_name, buff_operation);

    print_file(out_log, "%s", buff);

#ifdef LATER
    if (lo_param.m_trace_missing_arc) {
      print_file(out_log, "Trace: using missing arc %s (%s:%d) -> %s (%s:%d)\n",
                 old_node->get_qname(), old_lock->get_locking_src_file(),
                 old_lock->get_locking_src_line(), new_node->get_qname(),
                 new_lock->get_locking_src_file(),
                 new_lock->get_locking_src_line());
    }
#endif

    if (lo_param.m_debug_missing_arc) {
      debug_lock_order_break_here(buff);
    }

    /* Add the missing arc, to avoid reporting errors all the time. */

    /* find_node() is to avoid a const_cast */
    LO_node *from = find_node(from_node->get_qname());
    DBUG_ASSERT(from != nullptr);
    LO_node *to = find_node(to_node->get_qname());
    DBUG_ASSERT(to != nullptr);

    add_arc(from, to, false, 0, nullptr, "MISSING");
    return;
  }

  if (arc->has_trace()) {
    const LO_stack_trace *stack;
    if (new_lock != nullptr) {
      // lock -> lock arc
      print_file(out_log, "Trace: using arc %s (%s:%d) -> %s (%s:%d)\n",
                 from_node->get_qname(), old_lock->get_locking_src_file(),
                 old_lock->get_locking_src_line(), to_node->get_qname(),
                 new_lock->get_locking_src_file(),
                 new_lock->get_locking_src_line());
    } else {
      // lock -> file arc
      print_file(out_log, "Trace: using arc %s (%s:%d) -> %s\n",
                 from_node->get_qname(), old_lock->get_locking_src_file(),
                 old_lock->get_locking_src_line(), to_node->get_qname());
    }

    int length;
    const char *text;

    text = old_lock->get_statement_text(&length);
    if (length > 0) {
      print_file(out_log, "statement when first lock was acquired:\n");
      print_file(out_log, "%s\n", text);
    }

    stack = old_lock->get_stack_trace();
    if (stack != nullptr) {
      print_file(out_log, "stack when the first lock was acquired:\n");
      stack->print(out_log);
    }

    LO_stack_trace new_stack;
    print_file(out_log, "stack when the second lock was acquired:\n");
    new_stack.print(out_log);

    print_file(out_log, "locks held by this thread:\n");
    thread->print_all_locks(out_log);
  }

  const char *constraint = arc->get_constraint();
  if (constraint != nullptr) {
    if (thread->satisfy_constraint(constraint)) {
      return;
    }
    debug_lock_order_break_here("todo: constraint failed");
  }

  if (arc->has_debug()) {
    char buff[1024];
    if (new_lock != nullptr) {
      safe_snprintf(buff, sizeof(buff),
                    "Debug flag set on arc %s (%s:%d) -> %s (%s:%d)\n",
                    from_node->get_qname(), old_lock->get_locking_src_file(),
                    old_lock->get_locking_src_line(), to_node->get_qname(),
                    new_lock->get_locking_src_file(),
                    new_lock->get_locking_src_line());
    } else {
      safe_snprintf(buff, sizeof(buff),
                    "Debug flag set on arc %s (%s:%d) -> %s\n",
                    from_node->get_qname(), old_lock->get_locking_src_file(),
                    old_lock->get_locking_src_line(), to_node->get_qname());
    }

    debug_lock_order_break_here(buff);
  }
}

LO_node *LO_graph::find_state_node(const char *qname, const char *state) {
  LO_node *n = LO_node_finder::find_state_node(qname, state);
  return n;
}

LO_node *LO_graph::find_operation_node(const char *qname, bool recursive,
                                       const char *state,
                                       const char *operation) {
  LO_node *n =
      LO_node_finder::find_operation_node(qname, recursive, state, operation);
  return n;
}

LO_node *LO_graph::find_node(const char *qname) {
  LO_node_list::const_iterator it;
  LO_node *n;

  for (it = m_nodes.begin(); it != m_nodes.end(); it++) {
    n = *it;
    if (strcmp(qname, n->get_qname()) == 0) {
      return n;
    }
  }

  return nullptr;
}

void LO_graph::add_node(LO_node *node) { m_nodes.push_front(node); }

void LO_graph::add_class(const char *class_name) {
  if (g_internal_debug) {
    print_file(out_log, "Adding class %s\n", class_name);
  }

  LO_authorised_arc_list::iterator it;
  LO_authorised_arc *unresolved_arc;
  LO_node *from;
  LO_node *to;

  for (it = m_unresolved_arcs.begin(); it != m_unresolved_arcs.end();
       /* nothing */) {
    unresolved_arc = *it;

    if (strcmp(unresolved_arc->m_from_name, class_name) == 0) {
      from = find_state_node(unresolved_arc->m_from_name,
                             unresolved_arc->m_from_state);
      to = find_operation_node(
          unresolved_arc->m_to_name, unresolved_arc->m_op_recursive,
          unresolved_arc->m_from_state, unresolved_arc->m_to_operation);
      if ((from != nullptr) && (to != nullptr)) {
        /* Unresolved Named FROM [= node] --> Named TO */
        add_arc(from, to, unresolved_arc->m_op_recursive,
                unresolved_arc->m_flags, unresolved_arc->m_constraint,
                unresolved_arc->m_comment);
        it = m_unresolved_arcs.erase(it);
        destroy_arc(unresolved_arc);
        continue;
      }
    }

    if (strcmp(unresolved_arc->m_to_name, class_name) == 0) {
      if (strcmp(unresolved_arc->m_from_name, "*") != 0) {
        from = find_state_node(unresolved_arc->m_from_name,
                               unresolved_arc->m_from_state);
        to = find_operation_node(
            unresolved_arc->m_to_name, unresolved_arc->m_op_recursive,
            unresolved_arc->m_from_state, unresolved_arc->m_to_operation);

        if ((from != nullptr) && (to != nullptr)) {
          /* Unresolved Named FROM --> Named TO [= node] */
          add_arc(from, to, unresolved_arc->m_op_recursive,
                  unresolved_arc->m_flags, unresolved_arc->m_constraint,
                  unresolved_arc->m_comment);
          it = m_unresolved_arcs.erase(it);
          destroy_arc(unresolved_arc);
          continue;
        }
      } else {
        /* Wildcard "*" --> Named TO [= node] */
        to = find_operation_node(unresolved_arc->m_to_name, false, nullptr,
                                 unresolved_arc->m_to_operation);

        if (to != nullptr) {
          if (unresolved_arc->m_flags & LO_FLAG_IGNORED) {
            to->set_ignored();
          }

#ifdef LATER
          if (unresolved_arc->m_flags & LO_FLAG_TRACE) {
            to->set_trace();
          }
#endif

          it = m_unresolved_arcs.erase(it);
          destroy_arc(unresolved_arc);
          continue;
        }
      }
    }

    it++;
  }
}

void LO_graph::add_arc(const LO_authorised_arc *arc) {
  LO_node *from;
  LO_node *to;

  from = find_state_node(arc->m_from_name, arc->m_from_state);
  to = find_operation_node(arc->m_to_name, arc->m_op_recursive,
                           arc->m_from_state, arc->m_to_operation);

  if ((from != nullptr) && (to != nullptr)) {
    add_arc(from, to, arc->m_op_recursive, arc->m_flags, arc->m_constraint,
            arc->m_comment);
  } else {
    /* arc is owned by the parser, and is temporary. */
    LO_authorised_arc *arc_copy = deep_copy_arc(arc);
    add_unresolved_arc(arc_copy);
  }
}

void LO_graph::add_arc(LO_node *from, LO_node *to, bool recursive, int flags,
                       const char *constraint, const char *comment) {
  /* BIND condition TO mutex */
  if (flags & LO_FLAG_BIND) {
    if ((from->get_node_type() == NODE_TYPE_COND) &&
        (to->get_node_type() == NODE_TYPE_MUTEX)) {
      LO_cond_class *cond;
      LO_mutex_class *mutex;

      cond = LO_cond_class::find_by_qname(from->get_qname());
      mutex = LO_mutex_class::find_by_qname(to->get_qname());

      DBUG_ASSERT(cond != nullptr);
      DBUG_ASSERT(mutex != nullptr);

      cond->set_mutex_class(mutex);
      if (flags & LO_FLAG_UNFAIR) {
        cond->set_unfair();
      }
      return;
    }

    char message[1024];
    safe_snprintf(message, sizeof(message),
                  "Format Error: invalid bind, %s -> %s flags %d\n",
                  from->get_qname(), to->get_qname(), flags);
    print_file(out_log, "%s", message);
    debug_lock_order_break_here(message);
  }

  if (!g_with_rwlock) {
    if ((from->get_node_type() == NODE_TYPE_RWLOCK) ||
        (to->get_node_type() == NODE_TYPE_RWLOCK)) {
      return;
    }
  }

  LO_node_list cycle;

  bool is_loop = ((flags & LO_FLAG_LOOP) == LO_FLAG_LOOP);

  if (to->is_sink()) {
    return;
  }

  /* Instantly trace all loops */
  if (is_loop && lo_param.m_trace_loop) {
    flags |= LO_FLAG_TRACE;
  }

  /* Instantly debug all loops */
  if (is_loop && lo_param.m_debug_loop) {
    flags |= LO_FLAG_DEBUG;
  }

  DBUG_ASSERT(from != nullptr);
  DBUG_ASSERT(to != nullptr);

  /* Check for duplicates */
  const LO_arc_list &arcs_out = from->get_arcs_out();
  LO_arc_list::const_iterator it;
  LO_arc *arc;
  LO_node *n;

  for (it = arcs_out.begin(); it != arcs_out.end(); it++) {
    arc = *it;
    n = arc->get_to();
    if (strcmp(to->get_qname(), n->get_qname()) == 0) {
      /*
        For recursive arcs, the to node name can be adjusted,
        leading to confusion here.
        For example:
          STATE -X + RECURSIVE OP SX -> -X
          STATE -X + RECURSIVE OP X -> -X
        These are two distinct arc declarations,
        leading to the same arc in the graph.
        For now, do not print (possibly spurious) duplicate messages for
        recursive arcs.
      */
      if (!recursive) {
        print_file(out_log, "Format Error: duplicate arc, %s -> %s flags %d\n",
                   from->get_qname(), to->get_qname(), flags);
      }
      return;
    }
  }

  arc = new LO_arc(from, to, flags, constraint, comment);
  from->add_out(arc);
  to->add_in(arc);
  m_arcs.push_front(arc);

  if (arc->has_trace()) {
    from->set_trace();
  }

  if (arc->has_debug()) {
    from->set_debug();
  }

  unsigned int from_index = from->get_node_index();
  unsigned int to_index = to->get_node_index();

  DBUG_ASSERT(from_index < LO_MAX_NODE_NUMBER);
  DBUG_ASSERT(to_index < LO_MAX_NODE_NUMBER);
  m_arc_matrix[from_index][to_index] = arc;
}

void LO_graph::add_unresolved_arc(LO_authorised_arc *arc) {
  if (g_internal_debug) {
    char buff_state[STATE_NAME_LENGTH];
    char buff_op[OPERATION_NAME_LENGTH];

    if (arc->m_from_state != nullptr) {
      safe_snprintf(buff_state, sizeof(buff_state), " STATE %s",
                    arc->m_from_state);
    } else {
      buff_state[0] = '\0';
    }

    if (arc->m_to_operation != nullptr) {
      safe_snprintf(buff_op, sizeof(buff_op), " %sOP %s",
                    arc->m_op_recursive ? "RECURSIVE " : "",
                    arc->m_to_operation);
    } else {
      buff_op[0] = '\0';
    }

    print_file(out_log, "Unresolved arc: FROM \"%s\"%s TO \"%s\"%s\n",
               arc->m_from_name, buff_state, arc->m_to_name, buff_op);
  }
  m_unresolved_arcs.push_front(arc);
}

void LO_graph::dump_txt() {
  if (out_txt == nullptr) {
    return;
  }

  print_file(out_txt, "# Automatically generated, do not edit\n");
  print_file(out_txt, "\n");
  print_file(out_txt, "total number of nodes: %d\n", (int)m_nodes.size());
  print_file(out_txt, "total number of arcs: %d\n", (int)m_arcs.size());

  print_file(out_txt, "\n");
  print_file(out_txt, "DEPENDENCY GRAPH:\n");
  print_file(out_txt, "=================\n");
  print_file(out_txt, "\n");

  LO_node_list::const_iterator node_it;
  LO_node *n;
  LO_arc_list::const_iterator arc_it;
  LO_arc *a;

  for (node_it = m_nodes.begin(); node_it != m_nodes.end(); node_it++) {
    n = *node_it;

    print_file(out_txt, "NODE: %s\n", n->get_qname());

    const LO_arc_list &ali = n->get_arcs_in();
    if (ali.size() > 0) {
      print_file(out_txt, "  %d incoming arcs:\n", (int)ali.size());

      for (arc_it = ali.begin(); arc_it != ali.end(); arc_it++) {
        a = *arc_it;

        if (a->is_micro()) {
          print_file(out_txt, "    FROM: %s -- MICRO ARC\n",
                     a->get_from()->get_qname());
        } else if (a->has_loop()) {
          print_file(out_txt, "    FROM: %s -- LOOP FLAG\n",
                     a->get_from()->get_qname());
        } else {
          print_file(out_txt, "    FROM: %s\n", a->get_from()->get_qname());
        }
      }
    }

    const LO_arc_list &alo = n->get_arcs_out();
    if (alo.size() > 0) {
      print_file(out_txt, "  %d outgoing arcs:\n", (int)alo.size());

      for (arc_it = alo.begin(); arc_it != alo.end(); arc_it++) {
        a = *arc_it;

        if (a->is_micro()) {
          print_file(out_txt, "    TO: %s -- MICRO ARC\n",
                     a->get_to()->get_qname());
        } else if (a->has_loop()) {
          print_file(out_txt, "    TO: %s -- LOOP FLAG\n",
                     a->get_to()->get_qname());
        } else {
          print_file(out_txt, "    TO: %s\n", a->get_to()->get_qname());
        }
      }
    }
  }

  print_file(out_txt, "\n");
  print_file(out_txt, "SCC ANALYSIS (full graph):\n");
  print_file(out_txt, "==========================\n");
  print_file(out_txt, "\n");

  SCC_all full_graph;
  int number_of_scc = compute_scc(&full_graph);
  compute_scc_girth(number_of_scc, &full_graph);
  dump_scc(out_txt, number_of_scc, false);

  print_file(out_txt, "\n");
  print_file(out_txt, "IGNORED NODES:\n");
  print_file(out_txt, "==============\n");
  print_file(out_txt, "\n");

  for (node_it = m_nodes.begin(); node_it != m_nodes.end(); node_it++) {
    n = *node_it;

    if (n->is_ignored()) {
      print_file(out_txt, "IGNORED NODE: %s\n", n->get_qname());
    }
  }

  print_file(out_txt, "\n");
  print_file(out_txt, "CONSTRAINT ARC:\n");
  print_file(out_txt, "===============\n");
  print_file(out_txt, "\n");

  const char *constraint;
  for (arc_it = m_arcs.begin(); arc_it != m_arcs.end(); arc_it++) {
    a = *arc_it;
    constraint = a->get_constraint();

    if (constraint != nullptr) {
      LO_node *from = a->get_from();
      LO_node *to = a->get_to();
      print_file(out_txt, "CONSTRAINT ARC FROM %s TO %s\n", from->get_qname(),
                 to->get_qname());
      print_file(out_txt, "  ARC CONSTRAINT: %s\n", constraint);
      if (from->m_scc.m_scc_number == to->m_scc.m_scc_number) {
        print_file(out_txt, "  SAME SCC\n");
      }
      if (from->m_scc.m_scc_number < to->m_scc.m_scc_number) {
        print_file(out_txt, "  FORWARD SCC\n");
      }
      if (from->m_scc.m_scc_number < to->m_scc.m_scc_number) {
        print_file(out_txt, "  BACKWARD SCC\n");
      }
      const char *comment = a->get_comment();
      if (comment != nullptr) {
        print_file(out_txt, "  ARC COMMENT: %s\n", comment);
      }
    }
  }

  print_file(out_txt, "\n");
  print_file(out_txt, "LOOP ARC:\n");
  print_file(out_txt, "=========\n");
  print_file(out_txt, "\n");

  for (arc_it = m_arcs.begin(); arc_it != m_arcs.end(); arc_it++) {
    a = *arc_it;

    if (a->has_loop()) {
      LO_node *from = a->get_from();
      LO_node *to = a->get_to();
      print_file(out_txt, "LOOP ARC FROM %s TO %s\n", from->get_qname(),
                 to->get_qname());
      const char *comment = a->get_comment();
      if (comment != nullptr) {
        print_file(out_txt, "  ARC COMMENT: %s\n", comment);
      }
      if (from->m_scc.m_scc_number != to->m_scc.m_scc_number) {
        print_file(out_txt, "  LOOP FLAG NOT NEEDED\n");
      }
    }
  }

  print_file(out_txt, "\n");
  print_file(out_txt, "SCC ANALYSIS (revised graph):\n");
  print_file(out_txt, "=============================\n");
  print_file(out_txt, "\n");

  SCC_filter partial_graph;
  number_of_scc = compute_scc(&partial_graph);
  compute_scc_girth(number_of_scc, &partial_graph);
  dump_scc(out_txt, number_of_scc, true);

  print_file(out_txt, "\n");
  print_file(out_txt, "UNRESOLVED ARCS:\n");
  print_file(out_txt, "================\n");
  print_file(out_txt, "\n");

  LO_authorised_arc_list::iterator it;
  LO_authorised_arc *unresolved_arc;
  char buff_state[STATE_NAME_LENGTH];
  char buff_operation[OPERATION_NAME_LENGTH];

  for (it = m_unresolved_arcs.begin(); it != m_unresolved_arcs.end(); it++) {
    unresolved_arc = *it;
    const char *from_name = unresolved_arc->m_from_name;
    const char *from_state = unresolved_arc->m_from_state;
    const char *to_name = unresolved_arc->m_to_name;
    const char *to_operation = unresolved_arc->m_to_operation;

    if (from_state != nullptr) {
      safe_snprintf(buff_state, sizeof(buff_state), " STATE \"%s\"",
                    from_state);
    } else {
      buff_state[0] = '\0';
    }

    if (to_operation != nullptr) {
      safe_snprintf(buff_operation, sizeof(buff_operation), " OP \"%s\"",
                    to_operation);
    } else {
      buff_operation[0] = '\0';
    }

    print_file(out_txt, "UNRESOLVED ARC FROM \"%s\"%s TO \"%s\"%s\n", from_name,
               buff_state, to_name, buff_operation);
  }

  fclose(out_txt);
  out_txt = nullptr;
}

void LO_graph::scc_util(const SCC_visitor *v, int *discovery_time,
                        int *scc_count, LO_node *n, std::stack<LO_node *> *st) {
  int discovered = (*discovery_time)++;

  n->m_scc.m_index = discovered;
  n->m_scc.m_low_index = discovered;
  st->push(n);
  n->m_scc.m_on_stack = true;

  const LO_arc_list &arcs_out = n->get_arcs_out();
  LO_arc_list::const_iterator it;
  LO_arc *arc;
  LO_node *n2;

  /* If the node is IGNORED, pretend arcs do not exist */
  if (v->accept_node(n)) {
    for (it = arcs_out.begin(); it != arcs_out.end(); it++) {
      arc = *it;

      /* If the arc is LOOP, pretend it does not exist */
      if (v->accept_arc(arc)) {
        n2 = arc->get_to();

        if (n2->m_scc.m_index == -1) {
          scc_util(v, discovery_time, scc_count, n2, st);

          n->m_scc.m_low_index =
              std::min(n->m_scc.m_low_index, n2->m_scc.m_low_index);
        } else {
          if (n2->m_scc.m_on_stack == true) {
            n->m_scc.m_low_index =
                std::min(n->m_scc.m_low_index, n2->m_scc.m_index);
          }
        }
      }
    }
  }

  if (n->m_scc.m_low_index == n->m_scc.m_index) {
    LO_node *n3;
    std::stack<LO_node *> scc;
    while (st->top() != n) {
      n3 = st->top();
      n3->m_scc.m_on_stack = false;
      st->pop();
      scc.push(n3);
    }
    n3 = st->top();
    n3->m_scc.m_on_stack = false;
    st->pop();
    scc.push(n3);

    if (scc.size() > 1) {
      LO_node *c;
      (*scc_count)++;
      while (scc.size() != 0) {
        c = scc.top();
        c->m_scc.m_scc_number = *scc_count;
        scc.pop();
      }
    }
  }
}

int LO_graph::compute_scc(const SCC_visitor *v) {
  int discovery_time = 0;
  int scc_count = 0;

  std::stack<LO_node *> node_stack;

  LO_node_list::const_iterator node_it;
  LO_node *n;

  for (node_it = m_nodes.begin(); node_it != m_nodes.end(); node_it++) {
    n = *node_it;
    n->m_scc.clear();
  }

  for (node_it = m_nodes.begin(); node_it != m_nodes.end(); node_it++) {
    n = *node_it;
    if (n->m_scc.m_index == -1) {
      scc_util(v, &discovery_time, &scc_count, n, &node_stack);
    }
  }

  return scc_count;
}

void LO_graph::compute_scc_girth(int number_of_scc, const SCC_visitor *v) {
  int scc;
  LO_node_list::const_iterator node_it;
  LO_node *n;

  /* For each scc, compute nodes girth. */
  for (scc = 1; scc <= number_of_scc; scc++) {
    for (node_it = m_nodes.begin(); node_it != m_nodes.end(); node_it++) {
      n = *node_it;
      if (n->m_scc.m_scc_number == scc) {
        compute_node_girth(scc, v, n);
      }
    }
  }
}

void LO_graph::compute_node_girth(int iter_scc, const SCC_visitor *v,
                                  LO_node *start) {
  /** List of nodes at @c girth distance from the start node. */
  LO_node_list successors;
  /** List of nodes at @c girth+1 distance from the start node. */
  LO_node_list next_successors;
  LO_node_list::const_iterator node_it;
  LO_node *search;
  LO_node *n;

  LO_arc_list::const_iterator arc_it;
  LO_arc *arc;
  LO_node *to;
  int girth;

  /* Initialize state for a breadth first search. */
  for (node_it = m_nodes.begin(); node_it != m_nodes.end(); node_it++) {
    n = *node_it;
    n->m_search.clear();
  }

  successors.push_front(start);
  girth = 0;

  for (;;) {
    if (successors.empty()) {
      if (next_successors.empty()) {
        return;
      }
      successors.swap(next_successors);
      girth++;
      continue;
    }

    // Remove a node from successors
    node_it = successors.begin();
    search = *node_it;
    successors.erase(node_it);

    /* Do not visit the starting node at girth 0. */
    if (girth > 0) {
      search->m_search.m_visited = true;

      if (search == start) {
        /* Found a cycle */
        start->m_scc.m_scc_girth = girth;
        return;
      }
    }

    /* Continue the breath first search */
    const LO_arc_list &arcs_out = search->get_arcs_out();
    for (arc_it = arcs_out.begin(); arc_it != arcs_out.end(); arc_it++) {
      arc = *arc_it;
      to = arc->get_to();
      DBUG_ASSERT(to != nullptr);
      if ((to != search) && (to->m_scc.m_scc_number == iter_scc) &&
          v->accept_arc(arc)) {
        if (!to->m_search.m_visited) {
          next_successors.push_front(to);
        }
      }
    }
  }
}

void LO_graph::dump_scc(FILE *out, int number_of_scc, bool print_loop_flag) {
  print_file(out, "Number of SCC found: %u\n\n", number_of_scc);
  for (int scc = 1; scc <= number_of_scc; scc++) {
    dump_one_scc(out, scc, number_of_scc, print_loop_flag);
  }
}

void LO_graph::dump_one_scc(FILE *out, int scc, int number_of_scc,
                            bool print_loop_flag) {
  LO_node_list::const_iterator node_it;
  LO_node *n;
  LO_class *klass;
  int scc_girth = 0;
  int scc_circumference = 0;
  int scc_class_size = 0;
  int scc_node_size = 0;
  int arc_count = 0;

  std::set<LO_class *> classes_seen;
  std::set<LO_class *>::iterator it_seen;

  print_file(out, "Dumping classes for SCC %u:\n", scc);
  for (node_it = m_nodes.begin(); node_it != m_nodes.end(); node_it++) {
    n = *node_it;
    if (n->m_scc.m_scc_number == scc) {
      klass = n->get_class();

      it_seen = classes_seen.find(klass);
      if (it_seen == classes_seen.end()) {
        scc_class_size++;
        print_file(out, "SCC Class %s\n", klass->get_qname());
        classes_seen.insert(klass);
      }
    }
  }

  print_file(out, "\nDumping nodes for SCC %u:\n", scc);
  for (node_it = m_nodes.begin(); node_it != m_nodes.end(); node_it++) {
    n = *node_it;
    if (n->m_scc.m_scc_number == scc) {
      scc_node_size++;
      int girth = n->m_scc.m_scc_girth;
      print_file(out, "SCC Node %s Girth %d\n", n->get_qname(), girth);

      if (girth > scc_circumference) {
        scc_circumference = girth;
      }

      if (scc_girth == 0) {
        scc_girth = girth;
      } else if (girth < scc_girth) {
        scc_girth = girth;
      }
    }
  }

  print_file(out, "\nDumping arcs for SCC %u:\n", scc);

  for (node_it = m_nodes.begin(); node_it != m_nodes.end(); node_it++) {
    n = *node_it;
    if (n->m_scc.m_scc_number == scc) {
      const LO_arc_list &arcs_out = n->get_arcs_out();
      LO_arc_list::const_iterator it;
      LO_arc *arc;
      LO_node *to;

      for (it = arcs_out.begin(); it != arcs_out.end(); it++) {
        arc = *it;
        to = arc->get_to();
        DBUG_ASSERT(to != nullptr);
        if (to->m_scc.m_scc_number == scc) {
          if (arc->is_micro()) {
            print_file(out, "SCC ARC FROM \"%s\" TO \"%s\" -- MICRO ARC\n",
                       n->get_qname(), to->get_qname());
          } else {
            // Only count real arcs from the application, not micro arcs.
            arc_count++;
            if (print_loop_flag && arc->has_loop()) {
              print_file(out, "SCC ARC FROM \"%s\" TO \"%s\" -- FLAG LOOP\n",
                         n->get_qname(), to->get_qname());
            } else {
              print_file(out, "SCC ARC FROM \"%s\" TO \"%s\"\n", n->get_qname(),
                         to->get_qname());
            }
          }
        }
      }
    }
  }

  print_file(out, "\nSCC %d/%d summary:\n", scc, number_of_scc);
  print_file(out, " - Class Size %d\n", scc_class_size);
  print_file(out, " - Node Size %d\n", scc_node_size);
  print_file(out, " - Arc Count %d\n", arc_count);
  print_file(out, " - Girth %d\n", scc_girth);
  print_file(out, " - Circumference %d\n\n", scc_circumference);
}

LO_lock::LO_lock(const char *src_file, int src_line, size_t event_id)
    : m_locking_src_file(src_file),
      m_locking_src_line(src_line),
      m_event_id(event_id),
      m_stack(nullptr),
      m_locking_statement_text(nullptr),
      m_locking_statement_text_length(0),
      m_locking_pthread(my_thread_self()) {}

LO_lock::~LO_lock() {
  if (m_locking_statement_text != nullptr) {
    delete[] m_locking_statement_text;
  }
}

void LO_lock::record_stack_trace() { m_stack = new LO_stack_trace(); }

void LO_lock::record_statement_text(const char *text, int length) {
  if ((text != nullptr) && (length > 0)) {
    char *copy = new char[length + 1];
    if (copy != nullptr) {
      strncpy(copy, text, length + 1);
      copy[length] = '\0';
      m_locking_statement_text = copy;
      m_locking_statement_text_length = length;
    } else {
      m_locking_statement_text = nullptr;
      m_locking_statement_text_length = 0;
    }
  } else {
    m_locking_statement_text = nullptr;
    m_locking_statement_text_length = 0;
  }
}

unsigned int LO_thread_class::m_counter =
    1; /* thread/performance_schema/setup */

LO_thread_class *LO_thread_class::m_array[LO_MAX_THREAD_CLASS];

LO_thread_class *LO_thread_class::find(int key) {
  if (key == 0) {
    return nullptr;
  }
  DBUG_ASSERT(key >= 1);
  DBUG_ASSERT(key <= LO_MAX_THREAD_CLASS);
  DBUG_ASSERT(m_array[key - 1] != nullptr);
  return m_array[key - 1];
}

LO_thread_class *LO_thread_class::find_by_name(const char *category,
                                               const char *name) {
  char qname[LO_MAX_QNAME_LENGTH];

  safe_snprintf(qname, sizeof(qname), "thread/%s/%s", category, name);

  for (unsigned int i = 1; i < m_counter; i++) {
    LO_thread_class *klass = m_array[i];

    if (strcmp(qname, klass->get_qname()) == 0) {
      return klass;
    }
  }

  return nullptr;
}

void LO_thread_class::destroy_all() {
  for (unsigned int i = 0; i < m_counter; i++) {
    LO_thread_class *klass = m_array[i];
    delete klass;
  }
}

LO_thread_class::LO_thread_class(const char *category, const char *name) {
  m_counter++;
  m_key = m_counter;
  m_chain_key = 0;
  DBUG_ASSERT(m_key <= LO_MAX_THREAD_CLASS);
  m_array[m_key - 1] = this;
  safe_snprintf(m_qname, sizeof(m_qname), "thread/%s/%s", category, name);
}

LO_thread_class::~LO_thread_class() {
  DBUG_ASSERT(m_key <= LO_MAX_THREAD_CLASS);
  DBUG_ASSERT(m_array[m_key - 1] == this);
  m_array[m_key - 1] = nullptr;
}

LO_thread_list LO_thread::g_threads;

LO_thread::~LO_thread() {
  LO_mutex *mutex;
  LO_mutex_lock *old_lock;
  LO_mutex_lock_list::iterator it;

  for (it = m_mutex_locks.begin(); it != m_mutex_locks.end(); it++) {
    old_lock = *it;
    char buff[1024];
    mutex = old_lock->get_mutex();
    safe_snprintf(
        buff, sizeof(buff),
        "Deleting a thread %s with active mutex lock on %s, acquired in "
        "%s:%d\n",
        m_class->get_qname(), mutex->get_class()->get_qname(),
        old_lock->get_locking_src_file(), old_lock->get_locking_src_line());

    if (lo_param.m_trace_missing_unlock) {
      print_file(out_log, "%s", buff);
    }

    if (lo_param.m_debug_missing_unlock) {
      debug_lock_order_break_here(buff);
    }
  }

  LO_rwlock_lock *old_rwlock_lock;
  LO_rwlock_lock_list::iterator it_rwlock;

  for (it_rwlock = m_rwlock_locks.begin(); it_rwlock != m_rwlock_locks.end();
       it_rwlock++) {
    old_rwlock_lock = *it_rwlock;

    char buff[1024];
    safe_snprintf(
        buff, sizeof(buff),
        "Deleting a thread %s with active rwlock lock %s, acquired in "
        "%s:%d\n",
        m_class->get_qname(), old_rwlock_lock->get_state_node()->get_qname(),
        old_rwlock_lock->get_locking_src_file(),
        old_rwlock_lock->get_locking_src_line());

    if (lo_param.m_trace_missing_unlock) {
      print_file(out_log, "%s", buff);
    }

    if (lo_param.m_debug_missing_unlock) {
      debug_lock_order_break_here(buff);
    }
  }
}

void LO_thread::check_locks(const LO_mutex_lock *new_lock) {
  DBUG_ASSERT(new_lock != nullptr);

  /* Debug the non bootstrap code first. */
  if (!check_activated) {
    return;
  }

  DBUG_ASSERT(global_graph != nullptr);

  const LO_mutex_lock *old_mutex_lock;
  LO_mutex_lock_list::const_iterator it_mutex;

  for (it_mutex = m_mutex_locks.begin(); it_mutex != m_mutex_locks.end();
       it_mutex++) {
    old_mutex_lock = *it_mutex;
    global_graph->check_mutex(this, old_mutex_lock, new_lock);
  }

  const LO_rwlock_lock *old_rwlock_lock;
  LO_rwlock_lock_list::const_iterator it_rwlock;

  for (it_rwlock = m_rwlock_locks.begin(); it_rwlock != m_rwlock_locks.end();
       it_rwlock++) {
    old_rwlock_lock = *it_rwlock;
    global_graph->check_mutex(this, old_rwlock_lock, new_lock);
  }
}

void LO_thread::check_locks(const LO_rwlock_lock *new_lock,
                            PSI_rwlock_operation op) {
  DBUG_ASSERT(new_lock != nullptr);

  /* Debug the non bootstrap code first. */
  if (!check_activated) {
    return;
  }

  DBUG_ASSERT(global_graph != nullptr);

  const LO_mutex_lock *old_mutex_lock;
  LO_mutex_lock_list::const_iterator it_mutex;

  for (it_mutex = m_mutex_locks.begin(); it_mutex != m_mutex_locks.end();
       it_mutex++) {
    old_mutex_lock = *it_mutex;
    global_graph->check_rwlock(this, old_mutex_lock, new_lock, op);
  }

  const LO_rwlock_lock *old_rwlock_lock;
  LO_rwlock_lock_list::const_iterator it_rwlock;

  for (it_rwlock = m_rwlock_locks.begin(); it_rwlock != m_rwlock_locks.end();
       it_rwlock++) {
    old_rwlock_lock = *it_rwlock;
    global_graph->check_rwlock(this, old_rwlock_lock, new_lock, op);
  }
}

void LO_thread::check_locks(const LO_file_class *new_file) {
  DBUG_ASSERT(new_file != nullptr);

  /* Debug the non bootstrap code first. */
  if (!check_activated) {
    return;
  }

  DBUG_ASSERT(global_graph != nullptr);

  const LO_mutex_lock *old_mutex_lock;
  LO_mutex_lock_list::const_iterator it_mutex;

  for (it_mutex = m_mutex_locks.begin(); it_mutex != m_mutex_locks.end();
       it_mutex++) {
    old_mutex_lock = *it_mutex;
    global_graph->check_file(this, old_mutex_lock, new_file);
  }

  const LO_rwlock_lock *old_rwlock_lock;
  LO_rwlock_lock_list::const_iterator it_rwlock;

  for (it_rwlock = m_rwlock_locks.begin(); it_rwlock != m_rwlock_locks.end();
       it_rwlock++) {
    old_rwlock_lock = *it_rwlock;
    global_graph->check_file(this, old_rwlock_lock, new_file);
  }
}

void LO_thread::check_cond_wait(const LO_cond_wait *new_lock) {
  DBUG_ASSERT(new_lock != nullptr);

  /* Debug the non bootstrap code first. */
  if (!check_activated) {
    return;
  }

  DBUG_ASSERT(global_graph != nullptr);

  const LO_mutex_lock *old_mutex_lock;
  LO_mutex_lock_list::const_iterator it_mutex;

  for (it_mutex = m_mutex_locks.begin(); it_mutex != m_mutex_locks.end();
       it_mutex++) {
    old_mutex_lock = *it_mutex;
    global_graph->check_cond(this, old_mutex_lock, new_lock);
  }

  const LO_rwlock_lock *old_rwlock_lock;
  LO_rwlock_lock_list::const_iterator it_rwlock;

  for (it_rwlock = m_rwlock_locks.begin(); it_rwlock != m_rwlock_locks.end();
       it_rwlock++) {
    old_rwlock_lock = *it_rwlock;
    global_graph->check_cond(this, old_rwlock_lock, new_lock);
  }
}

void LO_thread::check_signal_broadcast(const char *operation, LO_cond *cond) {
  DBUG_ASSERT(cond != nullptr);

  const LO_cond_class *cond_class = cond->get_class();
  const LO_mutex_class *bound_mutex_class = cond_class->get_mutex_class();
  const LO_mutex_lock *old_mutex_lock;
  const LO_mutex *mutex;
  const LO_mutex_class *mutex_class;
  LO_mutex_lock_list::const_iterator it_mutex;
  char message[1024];

  if (bound_mutex_class != nullptr) {
    bool bound_mutex_locked = false;

    for (it_mutex = m_mutex_locks.begin(); it_mutex != m_mutex_locks.end();
         it_mutex++) {
      old_mutex_lock = *it_mutex;
      mutex = old_mutex_lock->get_mutex();
      mutex_class = mutex->get_class();

      if (mutex_class == bound_mutex_class) {
        bound_mutex_locked = true;
        break;
      }
    }

    if (!bound_mutex_locked) {
      if (!cond_class->is_unfair()) {
        safe_snprintf(message, sizeof(message),
                      "Error: %s on condition %s without holding "
                      "mutex %s, no UNFAIR flag\n",
                      operation, cond_class->get_qname(),
                      bound_mutex_class->get_qname());

        print_file(out_log, "%s", message);
        debug_lock_order_break_here(message);
      }
    }
  } else {
    for (it_mutex = m_mutex_locks.begin(); it_mutex != m_mutex_locks.end();
         it_mutex++) {
      old_mutex_lock = *it_mutex;
      mutex = old_mutex_lock->get_mutex();
      mutex_class = mutex->get_class();

      safe_snprintf(message, sizeof(message),
                    "Warning: condition %s without bound mutex, candidate %s\n",
                    cond_class->get_qname(), mutex_class->get_qname());

      print_file(out_log, "%s", message);
    }
  }
}

void LO_thread::check_destroy() {
  int size = m_mutex_locks.size() + m_rwlock_locks.size();
  if (size > 0) {
    print_file(out_log,
               "Error: LO_thread::check_destroy on %s %p with active locks\n",
               m_class->get_qname(), this);
    print_all_locks(out_log);
  }
}

bool LO_thread::satisfy_constraint(const char *constraint) {
  DBUG_ASSERT(constraint != nullptr);

  if (strncmp(constraint, "mutex/", 6) == 0) {
    return satisfy_mutex_constraint(constraint);
  }

  if (strncmp(constraint, "rwlock/", 7) == 0) {
    return satisfy_rwlock_constraint(constraint);
  }

  return false;
}

bool LO_thread::satisfy_mutex_constraint(const char *constraint) {
  const LO_mutex_lock *lock;
  LO_mutex_lock_list::const_iterator it_mutex;
  const char *qname;

  for (it_mutex = m_mutex_locks.begin(); it_mutex != m_mutex_locks.end();
       it_mutex++) {
    lock = *it_mutex;
    qname = lock->get_node()->get_qname();
    if (strcmp(qname, constraint) == 0) {
      return true;
    }
  }

  return false;
}

bool LO_thread::satisfy_rwlock_constraint(const char *constraint) {
  const LO_rwlock_lock *lock;
  LO_rwlock_lock_list::const_iterator it_rwlock;
  const char *qname;

  for (it_rwlock = m_rwlock_locks.begin(); it_rwlock != m_rwlock_locks.end();
       it_rwlock++) {
    lock = *it_rwlock;
    qname = lock->get_state_node()->get_qname();
    if (strcmp(qname, constraint) == 0) {
      // TODO: check read / write state as well.
      return true;
    }
  }

  return false;
}

void LO_thread::print_all_locks(FILE *out) {
  print_mutex_locks(out);
  print_rwlock_locks(out);
}

void LO_thread::print_mutex_locks(FILE *out) {
  const LO_mutex_lock *lock;
  LO_mutex_lock_list::const_iterator it_mutex;

  for (it_mutex = m_mutex_locks.begin(); it_mutex != m_mutex_locks.end();
       it_mutex++) {
    lock = *it_mutex;
    lock->dump(out);
  }
}

void LO_thread::print_rwlock_locks(FILE *out) {
  const LO_rwlock_lock *lock;
  LO_rwlock_lock_list::const_iterator it_rwlock;

  for (it_rwlock = m_rwlock_locks.begin(); it_rwlock != m_rwlock_locks.end();
       it_rwlock++) {
    lock = *it_rwlock;
    lock->dump(out);
  }
}

void LO_thread::dump(FILE *out, LO_thread *thread) {
  if (thread == nullptr) {
    print_file(out, "UNINSTRUMENTED\n");
  } else {
    thread->dump(out);
  }
}

void LO_thread::dump(FILE *out) const {
  print_file(out, "LO_thread class %s, object %p\n", m_class->get_qname(),
             this);
}

void LO_thread::add_mutex_lock(LO_thread *thread, LO_mutex *mutex,
                               const char *src_file, int src_line) {
  const char *thread_class_name;
  if (thread != nullptr) {
    thread_class_name = thread->get_class()->get_qname();
  } else {
    thread_class_name = "UNINSTRUMENTED";
  }

  if (mutex->get_class()->has_trace()) {
    print_file(
        out_log, "Trace: LO_thread::add_mutex_lock on %s %p from %s %p\n",
        mutex->get_class()->get_qname(), mutex, thread_class_name, thread);
  }

  LO_mutex_lock *lock = mutex->get_lock();
  if (lock != nullptr) {
    char buff[1024];
    safe_snprintf(
        buff, sizeof(buff),
        "Error: Thread %s %p adding a lock on mutex already locked: %s\n",
        thread_class_name, thread, mutex->get_class()->get_qname());

    print_file(out_log, "%s", buff);

    print_file(out_log, "Existing lock:\n");
    lock->dump(out_log);
    print_file(out_log, "Current thread:\n");
    LO_thread::dump(out_log, thread);

#ifdef LATER
    debug_lock_order_break_here(buff);
#endif
  }

  lock = new LO_mutex_lock(mutex, src_file, src_line, thread);
  mutex->set_lock(lock);

  const LO_mutex_class *mutex_class = mutex->get_class();
  const LO_node *node = mutex_class->get_node();
  if (node->has_trace()) {
    lock->record_stack_trace();

    if (thread != nullptr) {
      lock->record_statement_text(thread->m_statement_text,
                                  thread->m_statement_text_length);
    }
  }

  if (thread != nullptr) {
    thread->check_locks(lock);

    thread->m_mutex_locks.push_front(lock);

    uint size = thread->m_mutex_locks.size();

    if (size >= sanity_mutex_lock_limit && out_log != nullptr) {
      LO_mutex_lock *old_lock;
      LO_mutex_lock_list::iterator it;

      print_file(out_log, "Too many mutex lock found, dumping all locks\n");

      for (it = thread->m_mutex_locks.begin();
           it != thread->m_mutex_locks.end(); it++) {
        old_lock = *it;
        old_lock->dump(out_log);
      }
      print_file(out_log, "\n");

      debug_lock_order_break_here("sanity_mutex_lock_limit");
    }
  }
}

void LO_thread::remove_mutex_lock(LO_thread *thread, LO_mutex *mutex) {
  const char *thread_class_name;
  if (thread != nullptr) {
    thread_class_name = thread->get_class()->get_qname();
  } else {
    thread_class_name = "UNINSTRUMENTED";
  }

  if (mutex->get_class()->has_trace()) {
    print_file(
        out_log, "Trace: LO_thread::remove_mutex_lock on %s %p from %s %p\n",
        mutex->get_class()->get_qname(), mutex, thread_class_name, thread);
  }

  LO_mutex_lock *old_lock;
  LO_mutex_lock_list::iterator it;

  old_lock = mutex->get_lock();

  if (old_lock == nullptr) {
    char buff[1024];
    safe_snprintf(buff, sizeof(buff),
                  "Unlocking a mutex that is not locked: %s\n",
                  mutex->get_class()->get_qname());

    print_file(out_log, "%s", buff);

    debug_lock_order_break_here(buff);
    return;
  }

  LO_thread *old_thread = old_lock->get_thread();

  if ((old_thread == nullptr) && (thread == nullptr)) {
    mutex->set_unlocked();
    delete old_lock;
    return;
  }

  if (((old_thread == nullptr) && (thread != nullptr)) ||
      ((old_thread != nullptr) && (thread == nullptr))) {
    print_file(out_log, "Mutex lock/unlock across thread instrumentation:\n");
    print_file(out_log, "Mutex lock:\n");
    old_lock->dump(out_log);
    print_file(out_log, "Locking thread:\n");
    LO_thread::dump(out_log, old_thread);
    print_file(out_log, "Unlocking thread:\n");
    LO_thread::dump(out_log, thread);
    print_file(out_log, "my_thread_self: %lld\n", (long long)my_thread_self());
  }

  if ((old_thread != thread) && (old_thread != nullptr) &&
      (thread != nullptr)) {
    char buff[1024];
    safe_snprintf(buff, sizeof(buff),
                  "Unlocking a mutex locked by another thread: %s\n",
                  mutex->get_class()->get_qname());

    print_file(out_log, "%s", buff);

    print_file(out_log, "Mutex lock:\n");
    old_lock->dump(out_log);
    print_file(out_log, "Locking thread:\n");
    LO_thread::dump(out_log, old_thread);
    print_file(out_log, "Unlocking thread:\n");
    LO_thread::dump(out_log, thread);
    print_file(out_log, "my_thread_self: %lld\n", (long long)my_thread_self());

    debug_lock_order_break_here(buff);
    return;
  }

  if (thread != nullptr) {
    for (it = thread->m_mutex_locks.begin(); it != thread->m_mutex_locks.end();
         it++) {
      old_lock = *it;
      if (old_lock->get_mutex() == mutex) {
        it = thread->m_mutex_locks.erase(it);
        mutex->set_unlocked();
        delete old_lock;
        return;
      }
    }
  }

  mutex->set_unlocked();
  delete old_lock;
}

void LO_thread::add_rwlock_lock(LO_thread *thread, LO_rwlock *rwlock,
                                PSI_rwlock_operation op, const char *src_file,
                                int src_line) {
  if (g_with_rwlock) {
    if (thread != nullptr) {
      LO_rwlock_lock *old_lock;
      LO_rwlock_lock_list::iterator it;

      for (it = thread->m_rwlock_locks.begin();
           it != thread->m_rwlock_locks.end(); it++) {
        old_lock = *it;
        if (old_lock->get_rwlock() == rwlock) {
          thread->check_locks(old_lock, op);
          old_lock->merge_lock(op, src_file, src_line);
          return;
        }
      }

      // TODO: support uninstrumented threads.
      LO_rwlock_lock *new_lock = rwlock->build_lock(src_file, src_line, thread);
      new_lock->set_locked(op, src_file, src_line);

      thread->check_locks(new_lock, op);
      rwlock->add_lock(new_lock);

      thread->m_rwlock_locks.push_front(new_lock);
    }
  }
}

void LO_thread::remove_rwlock_lock(LO_thread *thread, LO_rwlock *rwlock,
                                   PSI_rwlock_operation unlock_op) {
  if (g_with_rwlock) {
    LO_rwlock_lock *old_lock;
    LO_rwlock_lock_list::iterator it;

    if (thread != nullptr) {
      for (it = thread->m_rwlock_locks.begin();
           it != thread->m_rwlock_locks.end(); it++) {
        old_lock = *it;
        if (old_lock->get_rwlock() == rwlock) {
          if (old_lock->set_unlocked(unlock_op)) {
            rwlock->remove_lock(old_lock);
            it = thread->m_rwlock_locks.erase(it);
            delete old_lock;
            return;
          }
        }
      }
    }

#ifdef LATER
    print_file(out_log,
               "Unlocking a rwlock that this thread did not lock: %s\n",
               rwlock->get_node()->get_qname());
#endif

#ifdef LATER
    debug_lock_order_break_here("bad unlock");
#endif
  }
}

void LO_thread::clear_all_locks() {
  int size = m_mutex_locks.size() + m_rwlock_locks.size();
  if (size > 0) {
    print_file(out_log, "Warning: thread still holds locks during shutdown:\n");
    print_all_locks(out_log);
  }
  // TODO: delete locks
  m_mutex_locks.clear();
  m_rwlock_locks.clear();
}

void LO_thread::set_runaway() {
  m_runaway = true;
  m_class = nullptr;
}

unsigned int LO_mutex_class::m_counter = 0;

LO_mutex_class *LO_mutex_class::m_array[LO_MAX_MUTEX_CLASS];

LO_mutex_class *LO_mutex_class::find_by_key(int key) {
  if (key == 0) {
    return nullptr;
  }
  DBUG_ASSERT(key >= 1);
  DBUG_ASSERT(key <= LO_MAX_MUTEX_CLASS);
  DBUG_ASSERT(m_array[key - 1] != nullptr);
  return m_array[key - 1];
}

LO_mutex_class *LO_mutex_class::find_by_name(const char *category,
                                             const char *name) {
  char qname[LO_MAX_QNAME_LENGTH];
  safe_snprintf(qname, sizeof(qname), "mutex/%s/%s", category, name);
  LO_mutex_class *klass = find_by_qname(qname);
  return klass;
}

LO_mutex_class *LO_mutex_class::find_by_qname(const char *qname) {
  for (unsigned int i = 0; i < m_counter; i++) {
    LO_mutex_class *klass = m_array[i];

    if (strcmp(qname, klass->get_qname()) == 0) {
      return klass;
    }
  }

  return nullptr;
}

void LO_mutex_class::destroy_all() {
  for (unsigned int i = 0; i < m_counter; i++) {
    LO_mutex_class *klass = m_array[i];
    delete klass;
  }
}

LO_mutex_class::LO_mutex_class(const char *category, const char *name,
                               int flags)
    : LO_class("mutex", category, name) {
  m_counter++;
  m_key = m_counter;
  DBUG_ASSERT(m_key <= LO_MAX_MUTEX_CLASS);
  m_array[m_key - 1] = this;
  m_node = new LO_node(this, NODE_TYPE_MUTEX, "mutex", category, name, nullptr,
                       flags);

  if (g_internal_debug) {
    print_file(out_log, "LO_mutex_class %s key %d\n", m_class_name, m_key);
  }
}

LO_mutex_class::~LO_mutex_class() {
  DBUG_ASSERT(m_key <= LO_MAX_MUTEX_CLASS);
  DBUG_ASSERT(m_array[m_key - 1] == this);
  m_array[m_key - 1] = nullptr;
  delete m_node;
}

void LO_mutex_class::add_to_graph(LO_graph *g) const {
  g->add_node(m_node);
  g->add_class(get_qname());
}

LO_node *LO_mutex_class::get_state_node_by_name(const char *name) const {
  if (name == nullptr) {
    return m_node;
  }
  return nullptr;
}

LO_node *LO_mutex_class::get_operation_node_by_name(
    bool recursive MY_ATTRIBUTE((unused)),
    const char *state MY_ATTRIBUTE((unused)), const char *operation) const {
  if (operation == nullptr) {
    return m_node;
  }
  return nullptr;
}

void LO_mutex::set_lock(LO_mutex_lock *lock) {
  if (m_lock != nullptr) {
    char buff[1024];
    safe_snprintf(
        buff, sizeof(buff),
        "Mutex %s already locked at file %s line %d, re locking from file "
        "%s line %d\n",
        m_class->get_qname(), m_lock->get_locking_src_file(),
        m_lock->get_locking_src_line(), lock->get_locking_src_file(),
        lock->get_locking_src_line());

    print_file(out_log, "%s", buff);
    debug_lock_order_break_here(buff);
  }

  m_lock = lock;
}

void LO_mutex_locker::start(const char *src_file, int src_line) {
  m_src_file = src_file;
  m_src_line = src_line;
}

void LO_mutex_locker::end() {
  LO_thread::add_mutex_lock(m_thread, m_mutex, m_src_file, m_src_line);
}

LO_mutex_lock::LO_mutex_lock(LO_mutex *mutex, const char *src_file,
                             int src_line, LO_thread *thread)
    : LO_lock(src_file, src_line, thread ? thread->new_event_id() : 0),
      m_mutex(mutex),
      m_thread(thread) {
  DBUG_ASSERT(mutex != nullptr);
}

const char *LO_mutex_lock::get_class_name() const {
  return m_mutex->get_class()->get_qname();
}

LO_node *LO_mutex_lock::get_state_node() const { return get_node(); }

const char *LO_mutex_lock::get_state_name() const { return nullptr; }

LO_node *LO_mutex_lock::get_node() const {
  return m_mutex->get_class()->get_node();
}

void LO_mutex_lock::dump(FILE *out) const {
  if (m_thread != nullptr) {
    print_file(out,
               "LO_mutex_lock on %s, file %s, line %d, thread %s %p %lld "
               "event %ld)\n",
               m_mutex->get_class()->get_qname(), get_locking_src_file(),
               get_locking_src_line(), m_thread->get_class()->get_qname(),
               m_thread, (long long)m_locking_pthread,
               static_cast<long>(get_event_id()));
  } else {
    print_file(
        out,
        "LO_mutex_lock on %s, file %s, line %d, uninstrumented thread %lld)\n",
        m_mutex->get_class()->get_qname(), get_locking_src_file(),
        get_locking_src_line(), (long long)m_locking_pthread);
  }
}

unsigned int LO_rwlock_class::m_counter = 0;

LO_rwlock_class *LO_rwlock_class::m_array[LO_MAX_RWLOCK_CLASS];

LO_rwlock_class *LO_rwlock_class::find_by_key(int key) {
  if (key == 0) {
    return nullptr;
  }
  DBUG_ASSERT(key >= 1);
  DBUG_ASSERT(key <= LO_MAX_RWLOCK_CLASS);
  DBUG_ASSERT(m_array[key - 1] != nullptr);
  return m_array[key - 1];
}

LO_rwlock_class *LO_rwlock_class::find_by_name(const char *category,
                                               const char *name, int flags) {
  char qname[LO_MAX_QNAME_LENGTH];
  if (flags & PSI_FLAG_RWLOCK_SX) {
    safe_snprintf(qname, sizeof(qname), "sxlock/%s/%s", category, name);
  } else if (flags & PSI_FLAG_RWLOCK_PR) {
    safe_snprintf(qname, sizeof(qname), "prlock/%s/%s", category, name);
  } else {
    safe_snprintf(qname, sizeof(qname), "rwlock/%s/%s", category, name);
  }
  LO_rwlock_class *klass = find_by_qname(qname);
  return klass;
}

LO_rwlock_class *LO_rwlock_class::find_by_qname(const char *qname) {
  for (unsigned int i = 0; i < m_counter; i++) {
    LO_rwlock_class *klass = m_array[i];

    if (strcmp(qname, klass->get_qname()) == 0) {
      return klass;
    }
  }

  return nullptr;
}

LO_rwlock_class *LO_rwlock_class::create(const char *category, const char *name,
                                         int flags) {
  LO_rwlock_class *klass;
  if (flags & PSI_FLAG_RWLOCK_SX) {
    klass = new LO_rwlock_class_sx(category, name, flags);
  } else if (flags & PSI_FLAG_RWLOCK_PR) {
    klass = new LO_rwlock_class_pr(category, name, flags);
  } else {
    klass = new LO_rwlock_class_rw(category, name, flags);
  }
  return klass;
}

void LO_rwlock_class::destroy_all() {
  for (unsigned int i = 0; i < m_counter; i++) {
    LO_rwlock_class *klass = m_array[i];
    delete klass;
  }
}

bool LO_rwlock_class::get_state_by_name(const char *name,
                                        PSI_rwlock_operation *state) {
  if (name == nullptr) {
    return true;
  }
  if (strcmp(name, "R") == 0) {
    *state = PSI_RWLOCK_READLOCK;
    return false;
  }
  if (strcmp(name, "W") == 0) {
    *state = PSI_RWLOCK_WRITELOCK;
    return false;
  }
  if (strcmp(name, "S") == 0) {
    *state = PSI_RWLOCK_SHAREDLOCK;
    return false;
  }
  if (strcmp(name, "SX") == 0) {
    *state = PSI_RWLOCK_SHAREDEXCLUSIVELOCK;
    return false;
  }
  if (strcmp(name, "X") == 0) {
    *state = PSI_RWLOCK_EXCLUSIVELOCK;
    return false;
  }
  return true;
}

bool LO_rwlock_class::get_operation_by_name(const char *name,
                                            PSI_rwlock_operation *op) {
  if (name == nullptr) {
    return true;
  }
  if (strcmp(name, "R") == 0) {
    *op = PSI_RWLOCK_READLOCK;
    return false;
  }
  if (strcmp(name, "TRY R") == 0) {
    *op = PSI_RWLOCK_TRYREADLOCK;
    return false;
  }
  if (strcmp(name, "W") == 0) {
    *op = PSI_RWLOCK_WRITELOCK;
    return false;
  }
  if (strcmp(name, "TRY W") == 0) {
    *op = PSI_RWLOCK_TRYWRITELOCK;
    return false;
  }
  if (strcmp(name, "S") == 0) {
    *op = PSI_RWLOCK_SHAREDLOCK;
    return false;
  }
  if (strcmp(name, "TRY S") == 0) {
    *op = PSI_RWLOCK_TRYSHAREDLOCK;
    return false;
  }
  if (strcmp(name, "SX") == 0) {
    *op = PSI_RWLOCK_SHAREDEXCLUSIVELOCK;
    return false;
  }
  if (strcmp(name, "TRY SX") == 0) {
    *op = PSI_RWLOCK_TRYSHAREDEXCLUSIVELOCK;
    return false;
  }
  if (strcmp(name, "X") == 0) {
    *op = PSI_RWLOCK_EXCLUSIVELOCK;
    return false;
  }
  if (strcmp(name, "TRY SX") == 0) {
    *op = PSI_RWLOCK_TRYEXCLUSIVELOCK;
    return false;
  }
  return true;
}

LO_rwlock_class::LO_rwlock_class(const char *prefix, const char *category,
                                 const char *name, int /* flags */)
    : LO_class(prefix, category, name) {
  m_counter++;
  m_key = m_counter;
  DBUG_ASSERT(m_key <= LO_MAX_RWLOCK_CLASS);
  m_array[m_key - 1] = this;
}

LO_rwlock_class::~LO_rwlock_class() {
  DBUG_ASSERT(m_key <= LO_MAX_RWLOCK_CLASS);
  DBUG_ASSERT(m_array[m_key - 1] == this);
  m_array[m_key - 1] = nullptr;
}

LO_node *LO_rwlock_class::get_state_node_by_name(const char *name) const {
  PSI_rwlock_operation state;
  if (get_state_by_name(name, &state)) {
    return nullptr;
  }
  LO_node *node = get_state_node(state);
  return node;
}

LO_node *LO_rwlock_class::get_operation_node_by_name(
    bool recursive, const char *state, const char *operation) const {
  PSI_rwlock_operation operation_value;
  PSI_rwlock_operation state_value = PSI_RWLOCK_UNLOCK;

  if (get_operation_by_name(operation, &operation_value)) {
    return nullptr;
  }
  if (recursive) {
    if (get_state_by_name(state, &state_value)) {
      return nullptr;
    }
  }
  LO_node *node = get_operation_node(recursive, state_value, operation_value);
  return node;
}

LO_rwlock_class_pr::LO_rwlock_class_pr(const char *category, const char *name,
                                       int flags)
    : LO_rwlock_class("prlock", category, name, flags) {
  m_node_p_r = new LO_node(this, NODE_TYPE_RWLOCK, "prlock", category, name,
                           "+R", flags);
  m_node_m_r = new LO_node(this, NODE_TYPE_RWLOCK, "prlock", category, name,
                           "-R", flags);
  m_node_p_w = new LO_node(this, NODE_TYPE_RWLOCK, "prlock", category, name,
                           "+W", flags);
  m_node_m_w = new LO_node(this, NODE_TYPE_RWLOCK, "prlock", category, name,
                           "-W", flags);
}

LO_rwlock_class_pr::~LO_rwlock_class_pr() {
  delete m_node_p_r;
  delete m_node_m_r;
  delete m_node_p_w;
  delete m_node_m_w;
}

LO_node *LO_rwlock_class_pr::get_state_node(PSI_rwlock_operation state) const {
  if (state == PSI_RWLOCK_READLOCK) {
    return m_node_m_r;
  }
  if (state == PSI_RWLOCK_WRITELOCK) {
    return m_node_m_w;
  }
  return nullptr;
}

LO_node *LO_rwlock_class_pr::get_operation_node(bool recursive,
                                                PSI_rwlock_operation state,
                                                PSI_rwlock_operation op) const {
  if ((op == PSI_RWLOCK_READLOCK) || (op == PSI_RWLOCK_TRYREADLOCK)) {
    if (!recursive) {
      return m_node_p_r;
    }
    if (state == PSI_RWLOCK_READLOCK) {
      // STATE R + RECURSIVE OP R -> -R
      return m_node_m_r;
    }
    if (state == PSI_RWLOCK_WRITELOCK) {
      // STATE W + RECURSIVE OP R -> -W
      return m_node_m_w;
    }
    return nullptr;
  }
  if ((op == PSI_RWLOCK_WRITELOCK) || (op == PSI_RWLOCK_TRYWRITELOCK)) {
    if (!recursive) {
      return m_node_p_w;
    }
    // STATE * + RECURSIVE OP W -> -W
    return m_node_m_w;
  }
  return nullptr;
}

const char *LO_rwlock_class_pr::get_operation_name(
    PSI_rwlock_operation op) const {
  if (op == PSI_RWLOCK_READLOCK) {
    return "R";
  }
  if (op == PSI_RWLOCK_TRYREADLOCK) {
    return "TRY R";
  }
  if (op == PSI_RWLOCK_WRITELOCK) {
    return "W";
  }
  if (op == PSI_RWLOCK_TRYWRITELOCK) {
    return "TRY W";
  }
  DBUG_ASSERT(false);
  return "UNSUPPORTED";
}

void LO_rwlock_class_pr::add_to_graph(LO_graph *g) const {
  g->add_node(m_node_p_r);
  g->add_node(m_node_m_r);
  g->add_node(m_node_p_w);
  g->add_node(m_node_m_w);
  g->add_arc(m_node_p_r, m_node_m_w, false, LO_FLAG_MICRO, nullptr, nullptr);
  g->add_arc(m_node_p_w, m_node_m_w, false, LO_FLAG_MICRO, nullptr, nullptr);
  g->add_arc(m_node_p_w, m_node_m_r, false, LO_FLAG_MICRO, nullptr, nullptr);
  g->add_class(get_qname());
}

LO_rwlock *LO_rwlock_class_pr::build_instance() {
  return new LO_rwlock_pr(this);
}

LO_rwlock_class_rw::LO_rwlock_class_rw(const char *category, const char *name,
                                       int flags)
    : LO_rwlock_class("rwlock", category, name, flags) {
  m_node_p_r = new LO_node(this, NODE_TYPE_RWLOCK, "rwlock", category, name,
                           "+R", flags);
  m_node_m_r = new LO_node(this, NODE_TYPE_RWLOCK, "rwlock", category, name,
                           "-R", flags);
  m_node_p_w = new LO_node(this, NODE_TYPE_RWLOCK, "rwlock", category, name,
                           "+W", flags);
  m_node_m_w = new LO_node(this, NODE_TYPE_RWLOCK, "rwlock", category, name,
                           "-W", flags);
}

LO_rwlock_class_rw::~LO_rwlock_class_rw() {
  delete m_node_p_r;
  delete m_node_m_r;
  delete m_node_p_w;
  delete m_node_m_w;
}

LO_node *LO_rwlock_class_rw::get_state_node(PSI_rwlock_operation state) const {
  if (state == PSI_RWLOCK_READLOCK) {
    return m_node_m_r;
  }
  if (state == PSI_RWLOCK_WRITELOCK) {
    return m_node_m_w;
  }
  return nullptr;
}

LO_node *LO_rwlock_class_rw::get_operation_node(bool recursive,
                                                PSI_rwlock_operation state,
                                                PSI_rwlock_operation op) const {
  if ((op == PSI_RWLOCK_READLOCK) || (op == PSI_RWLOCK_TRYREADLOCK)) {
    if (!recursive) {
      return m_node_p_r;
    }
    if (state == PSI_RWLOCK_READLOCK) {
      // STATE R + RECURSIVE OP R -> -R
      return m_node_m_r;
    }
    if (state == PSI_RWLOCK_WRITELOCK) {
      // STATE W + RECURSIVE OP R -> -W
      return m_node_m_w;
    }
    return nullptr;
  }
  if ((op == PSI_RWLOCK_WRITELOCK) || (op == PSI_RWLOCK_TRYWRITELOCK)) {
    if (!recursive) {
      return m_node_p_w;
    }
    // STATE * + RECURSIVE OP W -> -W
    return m_node_m_w;
  }
  return nullptr;
}

const char *LO_rwlock_class_rw::get_operation_name(
    PSI_rwlock_operation op) const {
  if (op == PSI_RWLOCK_READLOCK) {
    return "R";
  }
  if (op == PSI_RWLOCK_TRYREADLOCK) {
    return "TRY R";
  }
  if (op == PSI_RWLOCK_WRITELOCK) {
    return "W";
  }
  if (op == PSI_RWLOCK_TRYWRITELOCK) {
    return "TRY W";
  }
  DBUG_ASSERT(false);
  return "UNSUPPORTED";
}

void LO_rwlock_class_rw::add_to_graph(LO_graph *g) const {
  g->add_node(m_node_p_r);
  g->add_node(m_node_m_r);
  g->add_node(m_node_p_w);
  g->add_node(m_node_m_w);
  /* +R -> -R can block, unlike prlock. */
  g->add_arc(m_node_p_r, m_node_m_r, false, LO_FLAG_MICRO, nullptr, nullptr);
  g->add_arc(m_node_p_r, m_node_m_w, false, LO_FLAG_MICRO, nullptr, nullptr);
  g->add_arc(m_node_p_w, m_node_m_w, false, LO_FLAG_MICRO, nullptr, nullptr);
  g->add_arc(m_node_p_w, m_node_m_r, false, LO_FLAG_MICRO, nullptr, nullptr);
  g->add_class(get_qname());
}

LO_rwlock *LO_rwlock_class_rw::build_instance() {
  return new LO_rwlock_rw(this);
}

LO_rwlock_class_sx::LO_rwlock_class_sx(const char *category, const char *name,
                                       int flags)
    : LO_rwlock_class("sxlock", category, name, flags) {
  m_node_p_s = new LO_node(this, NODE_TYPE_RWLOCK, "sxlock", category, name,
                           "+S", flags);
  m_node_m_s = new LO_node(this, NODE_TYPE_RWLOCK, "sxlock", category, name,
                           "-S", flags);
  m_node_p_sx = new LO_node(this, NODE_TYPE_RWLOCK, "sxlock", category, name,
                            "+SX", flags);
  m_node_m_sx = new LO_node(this, NODE_TYPE_RWLOCK, "sxlock", category, name,
                            "-SX", flags);
  m_node_p_x = new LO_node(this, NODE_TYPE_RWLOCK, "sxlock", category, name,
                           "+X", flags);
  m_node_m_x = new LO_node(this, NODE_TYPE_RWLOCK, "sxlock", category, name,
                           "-X", flags);
}

LO_rwlock_class_sx::~LO_rwlock_class_sx() {
  delete m_node_p_s;
  delete m_node_m_s;
  delete m_node_p_sx;
  delete m_node_m_sx;
  delete m_node_p_x;
  delete m_node_m_x;
}

LO_node *LO_rwlock_class_sx::get_state_node(PSI_rwlock_operation state) const {
  if (state == PSI_RWLOCK_SHAREDLOCK) {
    return m_node_m_s;
  }
  if (state == PSI_RWLOCK_SHAREDEXCLUSIVELOCK) {
    return m_node_m_sx;
  }
  if (state == PSI_RWLOCK_EXCLUSIVELOCK) {
    return m_node_m_x;
  }
  return nullptr;
}

LO_node *LO_rwlock_class_sx::get_operation_node(bool recursive,
                                                PSI_rwlock_operation state,
                                                PSI_rwlock_operation op) const {
  if ((op == PSI_RWLOCK_SHAREDLOCK) || (op == PSI_RWLOCK_TRYSHAREDLOCK)) {
    if (!recursive) {
      return m_node_p_s;
    }
    if (state == PSI_RWLOCK_SHAREDLOCK) {
      // STATE S + RECURSIVE OP S -> -S
      return m_node_m_s;
    }
    if (state == PSI_RWLOCK_SHAREDEXCLUSIVELOCK) {
      // STATE SX + RECURSIVE OP S -> -SX
      return m_node_m_sx;
    }
    if (state == PSI_RWLOCK_EXCLUSIVELOCK) {
      // STATE X + RECURSIVE OP S -> -X
      return m_node_m_x;
    }
    return nullptr;
  }
  if ((op == PSI_RWLOCK_SHAREDEXCLUSIVELOCK) ||
      (op == PSI_RWLOCK_TRYSHAREDEXCLUSIVELOCK)) {
    if (!recursive) {
      return m_node_p_sx;
    }
    if (state == PSI_RWLOCK_SHAREDLOCK) {
      // STATE S + RECURSIVE OP SX -> -SX
      return m_node_m_sx;
    }
    if (state == PSI_RWLOCK_SHAREDEXCLUSIVELOCK) {
      // STATE SX + RECURSIVE OP SX -> -SX
      return m_node_m_sx;
    }
    if (state == PSI_RWLOCK_EXCLUSIVELOCK) {
      // STATE X + RECURSIVE OP SX -> -X
      return m_node_m_x;
    }
    return nullptr;
  }
  if ((op == PSI_RWLOCK_EXCLUSIVELOCK) || (op == PSI_RWLOCK_TRYEXCLUSIVELOCK)) {
    if (!recursive) {
      return m_node_p_x;
    }
    // STATE * + RECURSIVE OP X -> -X
    return m_node_m_x;
  }
  return nullptr;
}

const char *LO_rwlock_class_sx::get_operation_name(
    PSI_rwlock_operation op) const {
  if (op == PSI_RWLOCK_SHAREDLOCK) {
    return "S";
  }
  if (op == PSI_RWLOCK_TRYSHAREDLOCK) {
    return "TRY S";
  }
  if (op == PSI_RWLOCK_SHAREDEXCLUSIVELOCK) {
    return "SX";
  }
  if (op == PSI_RWLOCK_TRYSHAREDEXCLUSIVELOCK) {
    return "TRY SX";
  }
  if (op == PSI_RWLOCK_EXCLUSIVELOCK) {
    return "X";
  }
  if (op == PSI_RWLOCK_TRYEXCLUSIVELOCK) {
    return "TRY X";
  }
  DBUG_ASSERT(false);
  return "UNSUPPORTED";
}

void LO_rwlock_class_sx::add_to_graph(LO_graph *g) const {
  g->add_node(m_node_p_s);
  g->add_node(m_node_m_s);
  g->add_node(m_node_p_sx);
  g->add_node(m_node_m_sx);
  g->add_node(m_node_p_x);
  g->add_node(m_node_m_x);
  g->add_arc(m_node_p_s, m_node_m_x, false, LO_FLAG_MICRO, nullptr, nullptr);
  g->add_arc(m_node_p_sx, m_node_m_x, false, LO_FLAG_MICRO, nullptr, nullptr);
  g->add_arc(m_node_p_x, m_node_m_x, false, LO_FLAG_MICRO, nullptr, nullptr);
  g->add_arc(m_node_p_sx, m_node_m_sx, false, LO_FLAG_MICRO, nullptr, nullptr);
  g->add_arc(m_node_p_x, m_node_m_sx, false, LO_FLAG_MICRO, nullptr, nullptr);
  g->add_arc(m_node_p_x, m_node_m_s, false, LO_FLAG_MICRO, nullptr, nullptr);
  g->add_class(get_qname());
}

LO_rwlock *LO_rwlock_class_sx::build_instance() {
  return new LO_rwlock_sx(this);
}

void LO_rwlock::add_lock(LO_rwlock_lock *lock) {
  m_rwlock_locks.push_front(lock);
}

void LO_rwlock::remove_lock(LO_rwlock_lock *lock) {
  const LO_rwlock_lock *old_lock;
  LO_rwlock_lock_list::const_iterator it;

  for (it = m_rwlock_locks.begin(); it != m_rwlock_locks.end();
       /* nothing */) {
    old_lock = *it;
    if (old_lock == lock) {
      it = m_rwlock_locks.erase(it);
    } else {
      ++it;
    }
  }
}

void LO_rwlock_lock::dump(FILE *out) const {
  const LO_node *n = get_state_node();
  print_file(out, "LO_rwlock_lock on %s, file %s, line %d, event %ld)\n",
             n->get_qname(), get_locking_src_file(), get_locking_src_line(),
             static_cast<long>(get_event_id()));
}

LO_rwlock_lock *LO_rwlock_pr::build_lock(const char *src_file, int src_line,
                                         LO_thread *thread) {
  return new LO_rwlock_lock_pr(this, src_file, src_line, thread);
}

LO_rwlock_lock *LO_rwlock_rw::build_lock(const char *src_file, int src_line,
                                         LO_thread *thread) {
  return new LO_rwlock_lock_rw(this, src_file, src_line, thread);
}

LO_rwlock_lock *LO_rwlock_sx::build_lock(const char *src_file, int src_line,
                                         LO_thread *thread) {
  return new LO_rwlock_lock_sx(this, src_file, src_line, thread);
}

void LO_rwlock_locker::start(PSI_rwlock_operation op, const char *src_file,
                             int src_line) {
  m_op = op;
  m_src_file = src_file;
  m_src_line = src_line;
}

void LO_rwlock_locker::end() {
  DBUG_ASSERT(m_rwlock != nullptr);
  LO_thread::add_rwlock_lock(m_thread, m_rwlock, m_op, m_src_file, m_src_line);
}

LO_rwlock_lock::LO_rwlock_lock(LO_rwlock *rwlock, const char *src_file,
                               int src_line, LO_thread *thread)
    : LO_lock(src_file, src_line, thread ? thread->new_event_id() : 0),
      m_thread(thread),
      m_rwlock(rwlock) {
  DBUG_ASSERT(rwlock != nullptr);
}

const char *LO_rwlock_lock::get_class_name() const {
  return m_rwlock->get_class()->get_qname();
}

LO_node *LO_rwlock_lock::get_state_node() const {
  const LO_rwlock_class *k = m_rwlock->get_class();
  PSI_rwlock_operation state = get_state();
  LO_node *n = k->get_state_node(state);
  return n;
}

LO_node *LO_rwlock_lock::get_operation_node(bool recursive,
                                            PSI_rwlock_operation op) const {
  const LO_rwlock_class *k = m_rwlock->get_class();
  LO_node *n;
  if (recursive) {
    PSI_rwlock_operation state = get_state();
    n = k->get_operation_node(true, state, op);
  } else {
    n = k->get_operation_node(false, PSI_RWLOCK_UNLOCK, op);
  }
  return n;
}

const char *LO_rwlock_lock::get_operation_name(PSI_rwlock_operation op) const {
  const LO_rwlock_class *k = m_rwlock->get_class();
  const char *name = k->get_operation_name(op);
  return name;
}

LO_rwlock_lock_pr::LO_rwlock_lock_pr(LO_rwlock *rwlock, const char *src_file,
                                     int src_line, LO_thread *thread)
    : LO_rwlock_lock(rwlock, src_file, src_line, thread),
      m_read_count(0),
      m_write_count(0),
      m_write_src_file(nullptr),
      m_write_src_line(0),
      m_read_src_file(nullptr),
      m_read_src_line(0) {}

void LO_rwlock_lock_pr::set_locked(PSI_rwlock_operation op,
                                   const char *src_file, int src_line) {
  if ((op == PSI_RWLOCK_READLOCK) || (op == PSI_RWLOCK_TRYREADLOCK)) {
    if (m_write_count != 0) {
      print_file(out_log,
                 "Integrity error: prlock recursive read, with write lock.\n");
      print_file(out_log, "- class : %s\n", get_class_name());
      print_file(out_log, "- current read : %s%d\n", src_file, src_line);
      print_file(out_log, "- previous write : %s:%d\n", m_write_src_file,
                 m_write_src_line);
    }
    m_read_count++;
    m_read_src_file = src_file;
    m_read_src_line = src_line;
    return;
  }

  if ((op == PSI_RWLOCK_WRITELOCK) || (op == PSI_RWLOCK_TRYWRITELOCK)) {
    if ((m_write_count != 0) || (m_read_count != 0)) {
      print_file(out_log, "Integrity error: prlock recursive write.\n");
      print_file(out_log, "- class : %s\n", get_class_name());
      print_file(out_log, "- current write : %s%d\n", src_file, src_line);
      if (m_read_count != 0) {
        print_file(out_log, "- previous read : %s:%d\n", m_read_src_file,
                   m_read_src_line);
      }
      if (m_read_count != 0) {
        print_file(out_log, "- previous write : %s:%d\n", m_write_src_file,
                   m_write_src_line);
      }
    }
    m_write_count++;
    m_write_src_file = src_file;
    m_write_src_line = src_line;
    return;
  }
  DBUG_ASSERT(false);
}

void LO_rwlock_lock_pr::merge_lock(PSI_rwlock_operation op,
                                   const char *src_file, int src_line) {
  set_locked(op, src_file, src_line);
}

bool LO_rwlock_lock_pr::set_unlocked(
    PSI_rwlock_operation op MY_ATTRIBUTE((unused))) {
  DBUG_ASSERT(op == PSI_RWLOCK_UNLOCK);
  if (m_read_count > 0) {
    m_read_count--;
    return (m_read_count == 0 ? true : false);
  }
  if (m_write_count > 0) {
    m_write_count--;
    return true;
  }
  DBUG_ASSERT(false);
  return false;
}

PSI_rwlock_operation LO_rwlock_lock_pr::get_state() const {
  if (m_read_count > 0) {
    return PSI_RWLOCK_READLOCK;
  }
  if (m_write_count > 0) {
    return PSI_RWLOCK_WRITELOCK;
  }
  return PSI_RWLOCK_UNLOCK;
}

const char *LO_rwlock_lock_pr::get_state_name() const {
  PSI_rwlock_operation state = get_state();
  if (state == PSI_RWLOCK_READLOCK) {
    return "R";
  }
  if (state == PSI_RWLOCK_WRITELOCK) {
    return "W";
  }
  return nullptr;
}

LO_rwlock_lock_rw::LO_rwlock_lock_rw(LO_rwlock *rwlock, const char *src_file,
                                     int src_line, LO_thread *thread)
    : LO_rwlock_lock(rwlock, src_file, src_line, thread),
      m_read_count(0),
      m_write_count(0),
      m_write_src_file(nullptr),
      m_write_src_line(0),
      m_read_src_file(nullptr),
      m_read_src_line(0) {}

void LO_rwlock_lock_rw::set_locked(PSI_rwlock_operation op,
                                   const char *src_file, int src_line) {
  if ((op == PSI_RWLOCK_READLOCK) || (op == PSI_RWLOCK_TRYREADLOCK)) {
    if (m_write_count != 0) {
      print_file(out_log,
                 "Integrity error: rwlock recursive read, with write lock.\n");
      print_file(out_log, "- class : %s\n", get_class_name());
      print_file(out_log, "- current read : %s%d\n", src_file, src_line);
      print_file(out_log, "- previous write : %s:%d\n", m_write_src_file,
                 m_write_src_line);
    }
    m_read_count++;
    m_read_src_file = src_file;
    m_read_src_line = src_line;
    return;
  }
  if ((op == PSI_RWLOCK_WRITELOCK) || (op == PSI_RWLOCK_TRYWRITELOCK)) {
    if ((m_write_count != 0) || (m_read_count != 0)) {
      print_file(out_log, "Integrity error: rwlock recursive write.\n");
      print_file(out_log, "- class : %s\n", get_class_name());
      print_file(out_log, "- current write : %s%d\n", src_file, src_line);
      if (m_read_count != 0) {
        print_file(out_log, "- previous read : %s:%d\n", m_read_src_file,
                   m_read_src_line);
      }
      if (m_read_count != 0) {
        print_file(out_log, "- previous write : %s:%d\n", m_write_src_file,
                   m_write_src_line);
      }
    }
    m_write_count++;
    m_write_src_file = src_file;
    m_write_src_line = src_line;
    return;
  }
  DBUG_ASSERT(false);
}

void LO_rwlock_lock_rw::merge_lock(PSI_rwlock_operation op,
                                   const char *src_file, int src_line) {
  // gtid_sid_lock is recursive, should not be
  set_locked(op, src_file, src_line);
}

bool LO_rwlock_lock_rw::set_unlocked(
    PSI_rwlock_operation op MY_ATTRIBUTE((unused))) {
  DBUG_ASSERT(op == PSI_RWLOCK_UNLOCK);
  if (m_read_count > 0) {
    m_read_count--;
    return (m_read_count == 0 ? true : false);
  }
  if (m_write_count > 0) {
    m_write_count--;
    return true;
  }
  DBUG_ASSERT(false);
  return true;
}

PSI_rwlock_operation LO_rwlock_lock_rw::get_state() const {
  if (m_read_count > 0) {
    return PSI_RWLOCK_READLOCK;
  }
  if (m_write_count > 0) {
    return PSI_RWLOCK_WRITELOCK;
  }
  return PSI_RWLOCK_UNLOCK;
}

const char *LO_rwlock_lock_rw::get_state_name() const {
  PSI_rwlock_operation state = get_state();
  if (state == PSI_RWLOCK_READLOCK) {
    return "R";
  }
  if (state == PSI_RWLOCK_WRITELOCK) {
    return "W";
  }
  return nullptr;
}

LO_rwlock_lock_sx::LO_rwlock_lock_sx(LO_rwlock *rwlock, const char *src_file,
                                     int src_line, LO_thread *thread)
    : LO_rwlock_lock(rwlock, src_file, src_line, thread),
      m_s_count(0),
      m_sx_count(0),
      m_x_count(0),
      m_s_src_file(nullptr),
      m_s_src_line(0),
      m_sx_src_file(nullptr),
      m_sx_src_line(0),
      m_x_src_file(nullptr),
      m_x_src_line(0) {}

void LO_rwlock_lock_sx::set_locked(PSI_rwlock_operation op,
                                   const char *src_file, int src_line) {
  if ((op == PSI_RWLOCK_SHAREDLOCK) || (op == PSI_RWLOCK_TRYSHAREDLOCK)) {
    m_s_count++;
    m_s_src_file = src_file;
    m_s_src_line = src_line;
    return;
  }
  if ((op == PSI_RWLOCK_SHAREDEXCLUSIVELOCK) ||
      (op == PSI_RWLOCK_TRYSHAREDEXCLUSIVELOCK)) {
    m_sx_count++;
    m_sx_src_file = src_file;
    m_sx_src_line = src_line;
    return;
  }
  if ((op == PSI_RWLOCK_EXCLUSIVELOCK) || (op == PSI_RWLOCK_TRYEXCLUSIVELOCK)) {
    m_x_count++;
    m_x_src_file = src_file;
    m_x_src_line = src_line;
    return;
  }
  DBUG_ASSERT(false);
}

void LO_rwlock_lock_sx::merge_lock(PSI_rwlock_operation op,
                                   const char *src_file, int src_line) {
  set_locked(op, src_file, src_line);
}

bool LO_rwlock_lock_sx::set_unlocked(PSI_rwlock_operation op) {
  if (op == PSI_RWLOCK_SHAREDUNLOCK) {
    DBUG_ASSERT(m_s_count > 0);
    m_s_count--;
    return (((m_s_count == 0) && (m_sx_count == 0) && (m_x_count == 0))
                ? true
                : false);
  }
  if (op == PSI_RWLOCK_SHAREDEXCLUSIVEUNLOCK) {
    DBUG_ASSERT(m_sx_count > 0);
    m_sx_count--;
    return (((m_s_count == 0) && (m_sx_count == 0) && (m_x_count == 0))
                ? true
                : false);
  }
  if (op == PSI_RWLOCK_EXCLUSIVEUNLOCK) {
    DBUG_ASSERT(m_x_count > 0);
    m_x_count--;
    return (((m_s_count == 0) && (m_sx_count == 0) && (m_x_count == 0))
                ? true
                : false);
  }
  DBUG_ASSERT(false);
  return false;
}

PSI_rwlock_operation LO_rwlock_lock_sx::get_state() const {
  if (m_x_count > 0) {
    return PSI_RWLOCK_EXCLUSIVELOCK;
  }
  if (m_sx_count > 0) {
    return PSI_RWLOCK_SHAREDEXCLUSIVELOCK;
  }
  if (m_s_count > 0) {
    return PSI_RWLOCK_SHAREDLOCK;
  }
  return PSI_RWLOCK_UNLOCK;
}

const char *LO_rwlock_lock_sx::get_state_name() const {
  PSI_rwlock_operation state = get_state();
  if (state == PSI_RWLOCK_SHAREDLOCK) {
    return "S";
  }
  if (state == PSI_RWLOCK_SHAREDEXCLUSIVELOCK) {
    return "SX";
  }
  if (state == PSI_RWLOCK_EXCLUSIVELOCK) {
    return "X";
  }
  return nullptr;
}

unsigned int LO_cond_class::m_counter = 0;

LO_cond_class *LO_cond_class::m_array[LO_MAX_COND_CLASS];

LO_cond_class *LO_cond_class::find_by_key(int key) {
  if (key == 0) {
    return nullptr;
  }
  DBUG_ASSERT(key >= 1);
  DBUG_ASSERT(key <= LO_MAX_COND_CLASS);
  DBUG_ASSERT(m_array[key - 1] != nullptr);
  return m_array[key - 1];
}

LO_cond_class *LO_cond_class::find_by_name(const char *category,
                                           const char *name) {
  char qname[LO_MAX_QNAME_LENGTH];
  safe_snprintf(qname, sizeof(qname), "cond/%s/%s", category, name);
  LO_cond_class *klass = find_by_qname(qname);
  return klass;
}

LO_cond_class *LO_cond_class::find_by_qname(const char *qname) {
  for (unsigned int i = 0; i < m_counter; i++) {
    LO_cond_class *klass = m_array[i];

    if (strcmp(qname, klass->get_qname()) == 0) {
      return klass;
    }
  }

  return nullptr;
}

void LO_cond_class::destroy_all() {
  for (unsigned int i = 0; i < m_counter; i++) {
    LO_cond_class *klass = m_array[i];
    delete klass;
  }
}

LO_cond_class::LO_cond_class(const char *category, const char *name, int flags)
    : LO_class("cond", category, name) {
  m_counter++;
  m_key = m_counter;
  DBUG_ASSERT(m_key <= LO_MAX_COND_CLASS);
  m_array[m_key - 1] = this;
  m_mutex_class = nullptr;
  m_unfair = false;
  m_node =
      new LO_node(this, NODE_TYPE_COND, "cond", category, name, nullptr, flags);
}

LO_cond_class::~LO_cond_class() {
  DBUG_ASSERT(m_key <= LO_MAX_COND_CLASS);
  DBUG_ASSERT(m_array[m_key - 1] == this);
  m_array[m_key - 1] = nullptr;
  delete m_node;
}

void LO_cond_class::set_mutex_class(const LO_mutex_class *mutex_class) {
  if (m_mutex_class != nullptr) {
    debug_lock_order_break_here("TODO: dup bindings message");
  }

  m_mutex_class = mutex_class;
}

void LO_cond_class::add_to_graph(LO_graph *g) const {
  g->add_node(m_node);
  g->add_class(get_qname());
}

LO_node *LO_cond_class::get_state_node_by_name(const char *state) const {
  if (state == nullptr) {
    return m_node;
  }
  return nullptr;
}

LO_node *LO_cond_class::get_operation_node_by_name(
    bool recursive MY_ATTRIBUTE((unused)),
    const char *state MY_ATTRIBUTE((unused)), const char *operation) const {
  if (operation == nullptr) {
    return m_node;
  }
  return nullptr;
}

LO_cond::LO_cond(const LO_cond_class *klass)
    : m_class(klass), m_chain(nullptr) {
  if (klass->get_mutex_class() == nullptr) {
    char buff[1024];
    safe_snprintf(
        buff, sizeof(buff),
        "Coverage Error: condition %s not bound to any mutex class.\n",
        klass->get_qname());
    print_file(out_log, "%s", buff);
    // debug_lock_order_break_here(buff);
  }
}

void LO_cond_locker::start(const char *src_file, int src_line) {
  m_src_file = src_file;
  m_src_line = src_line;

  char buff[1024];
  const LO_cond_class *cond_class = m_cond->get_class();
  const LO_mutex_class *mutex_class = m_mutex->get_class();
  DBUG_ASSERT(cond_class != nullptr);
  DBUG_ASSERT(mutex_class != nullptr);

  const LO_mutex_class *bound_mutex_class = cond_class->get_mutex_class();

  if (bound_mutex_class == nullptr) {
    /* Print in a friendly format, to add the declaration back. */
    safe_snprintf(buff, sizeof(buff), "MISSING: BIND \"%s\" TO \"%s\"\n",
                  cond_class->get_qname(), mutex_class->get_qname());

    print_file(out_log, "%s", buff);
  } else if (bound_mutex_class != mutex_class) {
    safe_snprintf(
        buff, sizeof(buff),
        "Validity Error: condition %s, expecting mutex %s, found mutex %s\n",
        cond_class->get_qname(), bound_mutex_class->get_qname(),
        mutex_class->get_qname());
    print_file(out_log, "%s", buff);
    debug_lock_order_break_here(buff);
  } else if (m_mutex->get_lock() == nullptr) {
    safe_snprintf(
        buff, sizeof(buff),
        "Server Error: condition %s, waiting without mutex lock on %s\n",
        cond_class->get_qname(), mutex_class->get_qname());
    print_file(out_log, "%s", buff);
    debug_lock_order_break_here(buff);
  }

  /* Waiting on a cond gives up the mutex lock. */
  LO_thread::remove_mutex_lock(m_thread, m_mutex);

  LO_cond_wait waiting_here(m_mutex, m_cond, m_src_file, m_src_line, m_thread);

  if (m_thread != nullptr) {
    /* Make sure no other locks are taken while waiting. */
    m_thread->check_cond_wait(&waiting_here);
  }
}

void LO_cond_locker::end() {
  /* Waiting on a cond reacquire the mutex lock. */
  LO_thread::add_mutex_lock(m_thread, m_mutex, m_src_file, m_src_line);
}

LO_cond_wait::LO_cond_wait(LO_mutex *mutex, LO_cond *cond, const char *src_file,
                           int src_line, LO_thread *thread)
    : LO_lock(src_file, src_line, thread ? thread->new_event_id() : 0),
      m_mutex(mutex),
      m_cond(cond),
      m_thread(thread) {
  DBUG_ASSERT(mutex != nullptr);
  DBUG_ASSERT(cond != nullptr);
}

const char *LO_cond_wait::get_class_name() const {
  return m_cond->get_class()->get_qname();
}

LO_node *LO_cond_wait::get_state_node() const { return get_node(); }

const char *LO_cond_wait::get_state_name() const { return nullptr; }

LO_node *LO_cond_wait::get_node() const {
  return m_cond->get_class()->get_node();
}

void LO_cond_wait::dump(FILE * /* out */) const {}

unsigned int LO_file_class::m_counter = 0;

LO_file_class *LO_file_class::m_array[LO_MAX_FILE_CLASS];

LO_file_class *LO_file_class::find_by_key(int key) {
  if (key == 0) {
    return nullptr;
  }
  DBUG_ASSERT(key >= 1);
  DBUG_ASSERT(key <= LO_MAX_FILE_CLASS);
  DBUG_ASSERT(m_array[key - 1] != nullptr);
  return m_array[key - 1];
}

LO_file_class *LO_file_class::find_by_name(const char *category,
                                           const char *name) {
  char qname[LO_MAX_QNAME_LENGTH];
  safe_snprintf(qname, sizeof(qname), "file/%s/%s", category, name);
  LO_file_class *klass = find_by_qname(qname);
  return klass;
}

LO_file_class *LO_file_class::find_by_qname(const char *qname) {
  for (unsigned int i = 0; i < m_counter; i++) {
    LO_file_class *klass = m_array[i];

    if (strcmp(qname, klass->get_qname()) == 0) {
      return klass;
    }
  }

  return nullptr;
}

void LO_file_class::destroy_all() {
  for (unsigned int i = 0; i < m_counter; i++) {
    LO_file_class *klass = m_array[i];
    delete klass;
  }
}

LO_file_class::LO_file_class(const char *category, const char *name, int flags)
    : LO_class("file", category, name) {
  m_counter++;
  m_key = m_counter;
  DBUG_ASSERT(m_key <= LO_MAX_FILE_CLASS);
  m_array[m_key - 1] = this;

  m_node =
      new LO_node(this, NODE_TYPE_FILE, "file", category, name, nullptr, flags);
}

LO_file_class::~LO_file_class() {
  DBUG_ASSERT(m_key <= LO_MAX_FILE_CLASS);
  DBUG_ASSERT(m_array[m_key - 1] == this);
  m_array[m_key - 1] = nullptr;
  delete m_node;
}

void LO_file_class::add_to_graph(LO_graph *g) const {
  g->add_node(m_node);
  g->add_class(get_qname());
}

LO_node *LO_file_class::get_state_node_by_name(const char *state) const {
  if (state == nullptr) {
    return m_node;
  }
  return nullptr;
}

LO_node *LO_file_class::get_operation_node_by_name(
    bool recursive MY_ATTRIBUTE((unused)),
    const char *state MY_ATTRIBUTE((unused)), const char *operation) const {
  if (operation == nullptr) {
    return m_node;
  }
  return nullptr;
}

bool SCC_filter::accept_node(LO_node *node) const {
  return !node->is_ignored();
}

bool SCC_filter::accept_arc(LO_arc *arc) const {
  if (arc->has_loop()) {
    return false;
  }
  if (arc->get_constraint() != nullptr) {
    return false;
  }
  return true;
}

LO_stack_trace::LO_stack_trace() {
#ifdef _WIN32
  m_array_size = 0;
#else
  m_array_size = backtrace(m_addrs_array, array_elements(m_addrs_array));
#endif
}

void LO_stack_trace::print(FILE *out) const {
  char **string_array;
#ifdef _WIN32
  /* FIXME: see mysys/stacktrace.cc. */
  print_file(out, "[0] Sorry, no stack trace on this platform.\n");
  string_array = nullptr;
#else
  string_array = backtrace_symbols(m_addrs_array, m_array_size);
#endif

  if (string_array != nullptr) {
    for (int i = 0; i < m_array_size; i++) {
      print_file(out, "[%d] %s\n", i, string_array[i]);
    }
    free(string_array);
  }
}

/*
** ========================================================================
** SECTION 6: HELPERS IMPLEMENTATION
** ========================================================================
*/

static void debug_lock_order_break_here(const char *why) {
  /*
    ASSERT in --bootstrap mode are very rude, causing trouble to debug things.
    Only assert one the server is really up, to fail a given test script only
  */

#ifdef LATER
  if (!check_activated) {
    return;
  }
#endif

  /* Put a breakpoint here in your debugger. */
  debugger_calls++;

  LogErr(ERROR_LEVEL, ER_LOCK_ORDER_MESSAGE, why);

  my_print_stacktrace(nullptr, my_thread_stack_size);

  fflush(out_log);
  fflush(out_txt);
  DBUG_ASSERT(false);
}

static void print_file(FILE *file, const char *format, ...) {
  if (file != nullptr) {
    native_mutex_lock(&serialize_logs);

    va_list args;
    va_start(args, format);
    vfprintf(file, format, args);
    va_end(args);
    fflush(file);

    native_mutex_unlock(&serialize_logs);
  }
}

static char *deep_copy_string(const char *src) {
  if (src == nullptr) {
    return nullptr;
  }

  return strdup(src);
}

static void destroy_string(char *src) {
  if (src != nullptr) {
    free(src);
  }
}

static LO_authorised_arc *deep_copy_arc(const LO_authorised_arc *src) {
  LO_authorised_arc *dst = new LO_authorised_arc;
  dst->m_from_name = deep_copy_string(src->m_from_name);
  dst->m_from_state = deep_copy_string(src->m_from_state);
  dst->m_to_name = deep_copy_string(src->m_to_name);
  dst->m_op_recursive = src->m_op_recursive;
  dst->m_to_operation = deep_copy_string(src->m_to_operation);
  dst->m_flags = src->m_flags;
  dst->m_constraint = deep_copy_string(src->m_constraint);
  dst->m_comment = deep_copy_string(src->m_comment);
  return dst;
}

static void destroy_arc(LO_authorised_arc *arc) {
  destroy_string(arc->m_from_name);
  destroy_string(arc->m_from_state);
  destroy_string(arc->m_to_name);
  destroy_string(arc->m_to_operation);
  destroy_string(arc->m_constraint);
  destroy_string(arc->m_comment);
  delete arc;
}

/*
** ========================================================================
** SECTION 7: PERFORMANCE SCHEMA INSTRUMENTATION IMPLEMENTATION
** ========================================================================
*/

thread_local LO_thread *THR_LO;

PSI_mutex_service_t *g_mutex_chain = nullptr;
PSI_rwlock_service_t *g_rwlock_chain = nullptr;
PSI_cond_service_t *g_cond_chain = nullptr;
PSI_thread_service_t *g_thread_chain = nullptr;
PSI_file_service_t *g_file_chain = nullptr;
PSI_idle_service_t *g_idle_chain = nullptr;
PSI_statement_service_t *g_statement_chain = nullptr;

static inline LO_thread *get_THR_LO() { return THR_LO; }

static inline void set_THR_LO(LO_thread *lo) {
  if (g_internal_debug) {
    print_file(out_log, "set_THR_LO my_thread_self: %lld\n",
               (long long)my_thread_self());
    LO_thread *before = THR_LO;
    if (before) {
      print_file(out_log, "set_THR_LO before:\n");
      before->dump(out_log);
    }
    if (lo) {
      print_file(out_log, "set_THR_LO after:\n");
      lo->dump(out_log);
    }
  }
  THR_LO = lo;
}

extern "C" {

static void lo_register_mutex(const char *category, PSI_mutex_info *info,
                              int count) {
  native_mutex_lock(&serialize);

  LO_mutex_class *lo;

  for (int i = 0; i < count; i++, info++) {
    lo = LO_mutex_class::find_by_name(category, info->m_name);
    if (lo == nullptr) {
      lo = new LO_mutex_class(category, info->m_name, info->m_flags);
      lo->add_to_graph(global_graph);
    }

    if (g_mutex_chain != nullptr) {
      g_mutex_chain->register_mutex(category, info, 1);
      lo->set_chain_key(*info->m_key);
    }

    *info->m_key = lo->get_key();
  }

  native_mutex_unlock(&serialize);
}

static void lo_register_rwlock(const char *category, PSI_rwlock_info *info,
                               int count) {
  native_mutex_lock(&serialize);

  LO_rwlock_class *lo;

  for (int i = 0; i < count; i++, info++) {
    lo = LO_rwlock_class::find_by_name(category, info->m_name, info->m_flags);
    if (lo == nullptr) {
      lo = LO_rwlock_class::create(category, info->m_name, info->m_flags);
      lo->add_to_graph(global_graph);
    }

    if (g_rwlock_chain != nullptr) {
      g_rwlock_chain->register_rwlock(category, info, 1);
      lo->set_chain_key(*info->m_key);
    }

    *info->m_key = lo->get_key();
  }

  native_mutex_unlock(&serialize);
}

static void lo_register_cond(const char *category, PSI_cond_info *info,
                             int count) {
  native_mutex_lock(&serialize);

  LO_cond_class *lo;

  for (int i = 0; i < count; i++, info++) {
    lo = LO_cond_class::find_by_name(category, info->m_name);
    if (lo == nullptr) {
      lo = new LO_cond_class(category, info->m_name, info->m_flags);
      lo->add_to_graph(global_graph);
    }

    if (g_cond_chain != nullptr) {
      g_cond_chain->register_cond(category, info, 1);
      lo->set_chain_key(*info->m_key);
    }

    *info->m_key = lo->get_key();
  }

  native_mutex_unlock(&serialize);
}

static void lo_register_thread(const char *category, PSI_thread_info *info,
                               int count) {
  native_mutex_lock(&serialize);

  LO_thread_class *lo;

  for (int i = 0; i < count; i++, info++) {
    lo = LO_thread_class::find_by_name(category, info->m_name);
    if (lo == nullptr) {
      lo = new LO_thread_class(category, info->m_name);
    }

    if (g_thread_chain != nullptr) {
      g_thread_chain->register_thread(category, info, 1);
      lo->set_chain_key(*info->m_key);
    }

    *info->m_key = lo->get_key();
  }

  native_mutex_unlock(&serialize);
}

static void lo_register_file(const char *category, PSI_file_info *info,
                             int count) {
  native_mutex_lock(&serialize);

  LO_file_class *lo;

  for (int i = 0; i < count; i++, info++) {
    lo = LO_file_class::find_by_name(category, info->m_name);
    if (lo == nullptr) {
      lo = new LO_file_class(category, info->m_name, info->m_flags);
      lo->add_to_graph(global_graph);
    }

    if (g_file_chain != nullptr) {
      g_file_chain->register_file(category, info, 1);
      lo->set_chain_key(*info->m_key);
    }

    *info->m_key = lo->get_key();
  }

  native_mutex_unlock(&serialize);
}

static void lo_register_statement_v2(const char *category,
                                     PSI_statement_info *info, int count) {
  if (g_statement_chain != nullptr) {
    g_statement_chain->register_statement(category, info, count);
  }
}

static PSI_mutex *lo_init_mutex(PSI_mutex_key key, const void *identity) {
  native_mutex_lock(&serialize);

  LO_mutex *lo = nullptr;
  LO_mutex_class *klass = LO_mutex_class::find_by_key(key);
  if (klass != nullptr) {
    lo = new LO_mutex(klass);

    if (g_mutex_chain != nullptr) {
      lo->m_chain = g_mutex_chain->init_mutex(klass->get_chain_key(), identity);
    }
  } else {
    if (lo_param.m_trace_missing_key) {
      LO_stack_trace stack;
      print_file(out_log,
                 "Instrumentation Error: Mutex without a proper key.\n");
      stack.print(out_log);
    }
    if (lo_param.m_debug_missing_key) {
      debug_lock_order_break_here("No mutex key");
    }
  }

  native_mutex_unlock(&serialize);
  return reinterpret_cast<PSI_mutex *>(lo);
}

static void lo_destroy_mutex(PSI_mutex *mutex) {
  native_mutex_lock(&serialize);

  LO_mutex *lo = reinterpret_cast<LO_mutex *>(mutex);
  if (lo != nullptr) {
    if ((g_mutex_chain != nullptr) && (lo->m_chain != nullptr)) {
      g_mutex_chain->destroy_mutex(lo->m_chain);
    }
    delete lo;
  }

  native_mutex_unlock(&serialize);
}

static PSI_rwlock *lo_init_rwlock(PSI_rwlock_key key, const void *identity) {
  native_mutex_lock(&serialize);

  LO_rwlock *lo = nullptr;
  LO_rwlock_class *klass = LO_rwlock_class::find_by_key(key);
  if (klass != nullptr) {
    lo = klass->build_instance();

    if (g_rwlock_chain != nullptr) {
      lo->m_chain =
          g_rwlock_chain->init_rwlock(klass->get_chain_key(), identity);
    }
  } else {
    if (lo_param.m_trace_missing_key) {
      LO_stack_trace stack;
      print_file(out_log,
                 "Instrumentation Error: Rwlock without a proper key.\n");
      stack.print(out_log);
    }
    if (lo_param.m_debug_missing_key) {
      debug_lock_order_break_here("No rwlock key");
    }
  }

  native_mutex_unlock(&serialize);
  return reinterpret_cast<PSI_rwlock *>(lo);
}

static void lo_destroy_rwlock(PSI_rwlock *rwlock) {
  native_mutex_lock(&serialize);

  LO_rwlock *lo = reinterpret_cast<LO_rwlock *>(rwlock);
  if (lo != nullptr) {
    if ((g_rwlock_chain != nullptr) && (lo->m_chain != nullptr)) {
      g_rwlock_chain->destroy_rwlock(lo->m_chain);
    }
    delete lo;
  }

  native_mutex_unlock(&serialize);
}

static PSI_cond *lo_init_cond(PSI_cond_key key, const void *identity) {
  native_mutex_lock(&serialize);

  LO_cond *lo = nullptr;
  LO_cond_class *klass = LO_cond_class::find_by_key(key);
  if (klass != nullptr) {
    lo = new LO_cond(klass);

    if (g_cond_chain != nullptr) {
      lo->m_chain = g_cond_chain->init_cond(klass->get_chain_key(), identity);
    }
  } else {
    if (lo_param.m_trace_missing_key) {
      LO_stack_trace stack;
      print_file(out_log,
                 "Instrumentation Error: Cond without a proper key.\n");
      stack.print(out_log);
    }
    if (lo_param.m_debug_missing_key) {
      debug_lock_order_break_here("No cond key");
    }
  }

  native_mutex_unlock(&serialize);
  return reinterpret_cast<PSI_cond *>(lo);
}

static void lo_destroy_cond(PSI_cond *cond) {
  native_mutex_lock(&serialize);

  LO_cond *lo = reinterpret_cast<LO_cond *>(cond);
  if (lo != nullptr) {
    if ((g_cond_chain != nullptr) && (lo->m_chain != nullptr)) {
      g_cond_chain->destroy_cond(lo->m_chain);
    }
    delete lo;
  }

  native_mutex_unlock(&serialize);
}

/** Bindings of LO_file objects to file descriptors numbers. */
static std::vector<LO_file *> lo_file_array;

static void lo_file_bindings_insert(size_t index, LO_file *lo_file) {
  DBUG_ASSERT(!lo_file->m_bound);
  if (lo_file_array.size() < index + 1) {
    lo_file_array.resize(index + 1, nullptr);
  }

  LO_file *lo_old_file = lo_file_array[index];

  if (lo_old_file != nullptr) {
    DBUG_ASSERT(lo_old_file->m_bound);
    lo_old_file->m_bound = false;
    delete lo_old_file;
  }

  lo_file_array[index] = lo_file;
  lo_file->m_bound = true;
}

static LO_file *lo_file_bindings_find(size_t index, bool remove) {
  if (lo_file_array.size() < index + 1) {
    /* No point in resizing to find no element. */
    return nullptr;
  }

  LO_file *lo_file = lo_file_array[index];
  if (lo_file != nullptr) {
    DBUG_ASSERT(lo_file->m_bound);
    if (remove) {
      lo_file_array[index] = nullptr;
      lo_file->m_bound = false;
    }
  }
  return lo_file;
}

static void lo_create_file(PSI_file_key key, const char *name, File file) {
  LO_file_class *klass = LO_file_class::find_by_key(key);

  if (klass == nullptr) {
    if (lo_param.m_trace_missing_key) {
      LO_stack_trace stack;
      print_file(out_log,
                 "Instrumentation Error: file without a proper key.\n");
      stack.print(out_log);
    }
    if (lo_param.m_debug_missing_key) {
      debug_lock_order_break_here("No file key");
    }
    return;
  }

  int index = (int)file;
  if (index >= 0) {
    LO_file *lo_file = new LO_file(klass);
    lo_file_bindings_insert(index, lo_file);
  }

  if (g_file_chain != nullptr) {
    g_file_chain->create_file(klass->get_chain_key(), name, file);
  }
}

class LO_spawn_thread_arg {
 public:
  PSI_thread_key m_child_key;
  void *(*m_user_start_routine)(void *);
  void *m_user_arg;
};

void *lo_spawn_thread_fct(void *arg) {
  LO_spawn_thread_arg *typed_arg = (LO_spawn_thread_arg *)arg;
  void *user_arg;
  void *(*user_start_routine)(void *);

  LO_thread *lo = nullptr;

  native_mutex_lock(&serialize);
  /* First, attach instrumentation to this newly created pthread. */
  LO_thread_class *klass = LO_thread_class::find(typed_arg->m_child_key);
  if (klass != nullptr) {
    lo = new LO_thread(klass);
    LO_thread::g_threads.push_back(lo);
  }
  native_mutex_unlock(&serialize);

  set_THR_LO(lo);

  if (g_thread_chain != nullptr) {
    /* [2] Get the chained thread instrumentation created by [1]. */
    lo->m_chain = g_thread_chain->get_thread();
  }

  /*
    Secondly, free the memory allocated in spawn_thread().
    It is preferable to do this before invoking the user
    routine, to avoid memory leaks at shutdown, in case
    the server exits without waiting for this thread.
  */
  user_start_routine = typed_arg->m_user_start_routine;
  user_arg = typed_arg->m_user_arg;
  delete typed_arg;

  /* Then, execute the user code for this thread. */
  (*user_start_routine)(user_arg);

  return nullptr;
}

static int lo_spawn_thread(PSI_thread_key key, my_thread_handle *thread,
                           const my_thread_attr_t *attr,
                           void *(*start_routine)(void *), void *arg) {
  LO_spawn_thread_arg *psi_arg;

  /* psi_arg can not be global, and can not be a local variable. */
  psi_arg = new LO_spawn_thread_arg();
  if (unlikely(psi_arg == nullptr)) {
    return EAGAIN;
  }

  LO_thread_class *klass = LO_thread_class::find(key);
  PSI_thread_key chain_key = PSI_NOT_INSTRUMENTED;
  if (klass != nullptr) {
    chain_key = klass->get_chain_key();
  }

  psi_arg->m_child_key = key;
  psi_arg->m_user_start_routine = start_routine;
  psi_arg->m_user_arg = arg;

  int result;

  if (g_thread_chain == nullptr) {
    result = my_thread_create(thread, attr, lo_spawn_thread_fct, psi_arg);
  } else {
    /* [1] Start the thread in the chained instrumentation. */
    result = g_thread_chain->spawn_thread(chain_key, thread, attr,
                                          lo_spawn_thread_fct, psi_arg);
  }

  if (unlikely(result != 0)) {
    delete psi_arg;
  }
  return result;
}

static PSI_thread *lo_new_thread(PSI_thread_key key, const void *identity,
                                 ulonglong processlist_id) {
  native_mutex_lock(&serialize);

  LO_thread *lo = nullptr;
  LO_thread_class *klass = LO_thread_class::find(key);

  // TODO: Auto_THD in the sql layer.
  if (klass != nullptr) {
    lo = new LO_thread(klass);
    LO_thread::g_threads.push_back(lo);

    if (g_thread_chain != nullptr) {
      lo->m_chain = g_thread_chain->new_thread(klass->get_chain_key(), identity,
                                               processlist_id);
    }
  }

  native_mutex_unlock(&serialize);
  return reinterpret_cast<PSI_thread *>(lo);
}

static void lo_set_thread_id(PSI_thread *thread, ulonglong id) {
  LO_thread *lo = reinterpret_cast<LO_thread *>(thread);

  if (lo != nullptr) {
    if ((g_thread_chain != nullptr) && (lo->m_chain != nullptr)) {
      g_thread_chain->set_thread_id(lo->m_chain, id);
    }
  }
}

static ulonglong lo_get_current_thread_internal_id() {
  ulonglong result = 0;

  if (g_thread_chain != nullptr) {
    result = g_thread_chain->get_current_thread_internal_id();
  }

  return result;
}

static ulonglong lo_get_thread_internal_id(PSI_thread *thread) {
  ulonglong result = 0;
  LO_thread *lo = reinterpret_cast<LO_thread *>(thread);

  if (lo != nullptr) {
    if ((g_thread_chain != nullptr) && (lo->m_chain != nullptr)) {
      result = g_thread_chain->get_thread_internal_id(lo->m_chain);
    }
  }

  return result;
}

static PSI_thread *lo_get_thread_by_id(ulonglong processlist_id) {
  PSI_thread *chain = nullptr;
  LO_thread *lo = nullptr;

  if (g_thread_chain != nullptr) {
    chain = g_thread_chain->get_thread_by_id(processlist_id);

    // Wrap pfs_thread into LO_thread
    LO_thread_list::const_iterator it;
    for (it = LO_thread::g_threads.begin(); it != LO_thread::g_threads.end();
         it++) {
      lo = *it;
      if (lo->m_chain == chain) {
        return reinterpret_cast<PSI_thread *>(lo);
      }
    }
  }

  return nullptr;
}

static void lo_set_thread_THD(PSI_thread *thread, THD *thd) {
  LO_thread *lo = reinterpret_cast<LO_thread *>(thread);

  if (lo != nullptr) {
    if ((g_thread_chain != nullptr) && (lo->m_chain != nullptr)) {
      g_thread_chain->set_thread_THD(lo->m_chain, thd);
    }
  }
}

static void lo_set_thread_os_id(PSI_thread *thread) {
  LO_thread *lo = reinterpret_cast<LO_thread *>(thread);

  if (lo != nullptr) {
    if ((g_thread_chain != nullptr) && (lo->m_chain != nullptr)) {
      g_thread_chain->set_thread_os_id(lo->m_chain);
    }
  }
}

static PSI_thread *lo_get_thread(void) {
  LO_thread *lo = get_THR_LO();
  return reinterpret_cast<PSI_thread *>(lo);
}

static void lo_set_thread_user(const char *user, int user_len) {
  if (g_thread_chain != nullptr) {
    g_thread_chain->set_thread_user(user, user_len);
  }
}

static void lo_set_thread_account(const char *user, int user_len,
                                  const char *host, int host_len) {
  if (g_thread_chain != nullptr) {
    g_thread_chain->set_thread_account(user, user_len, host, host_len);
  }
}

static void lo_set_thread_db(const char *db, int db_len) {
  if (g_thread_chain != nullptr) {
    g_thread_chain->set_thread_db(db, db_len);
  }
}

static void lo_set_thread_command(int command) {
  if (g_thread_chain != nullptr) {
    g_thread_chain->set_thread_command(command);
  }
}

static void lo_set_connection_type(opaque_vio_type conn_type) {
  if (g_thread_chain != nullptr) {
    g_thread_chain->set_connection_type(conn_type);
  }
}

static void lo_set_thread_start_time(time_t start_time) {
  if (g_thread_chain != nullptr) {
    g_thread_chain->set_thread_start_time(start_time);
  }
}

static void lo_set_thread_info(const char *info, uint info_len) {
  if (g_thread_chain != nullptr) {
    g_thread_chain->set_thread_info(info, info_len);
  }
}

static int lo_set_thread_resource_group(const char *group_name,
                                        int group_name_len, void *user_data) {
  int rc = 0;

  if (g_thread_chain != nullptr) {
    rc = g_thread_chain->set_thread_resource_group(group_name, group_name_len,
                                                   user_data);
  }

  return rc;
}

static int lo_set_thread_resource_group_by_id(PSI_thread *thread,
                                              ulonglong thread_id,
                                              const char *group_name,
                                              int group_name_len,
                                              void *user_data) {
  LO_thread *lo = reinterpret_cast<LO_thread *>(thread);
  int rc = 0;

  if (lo != nullptr) {
    if ((g_thread_chain != nullptr) && (lo->m_chain != nullptr)) {
      rc = g_thread_chain->set_thread_resource_group_by_id(
          lo->m_chain, thread_id, group_name, group_name_len, user_data);
    }
  }

  return rc;
}

static void lo_set_thread(PSI_thread *thread) {
  LO_thread *lo = reinterpret_cast<LO_thread *>(thread);
  set_THR_LO(lo);

  if (g_thread_chain != nullptr) {
    if (lo == nullptr) {
      g_thread_chain->set_thread(nullptr);
    } else {
      g_thread_chain->set_thread(lo->m_chain);
    }
  }
}

static void lo_aggregate_thread_status(PSI_thread *thread) {
  LO_thread *lo = reinterpret_cast<LO_thread *>(thread);

  if (g_thread_chain != nullptr) {
    if (lo == nullptr) {
      g_thread_chain->aggregate_thread_status(nullptr);
    } else {
      g_thread_chain->aggregate_thread_status(lo->m_chain);
    }
  }
}

static void lo_delete_current_thread(void) {
  native_mutex_lock(&serialize);

  LO_thread *thread = get_THR_LO();
  if (thread != nullptr) {
    thread->check_destroy();
    set_THR_LO(nullptr);
    LO_thread::g_threads.remove(thread);
    delete thread;
  }

  if (g_thread_chain != nullptr) {
    g_thread_chain->delete_current_thread();
  }

  native_mutex_unlock(&serialize);
}

static void lo_delete_thread(PSI_thread *thread) {
  LO_thread *lo = reinterpret_cast<LO_thread *>(thread);

  native_mutex_lock(&serialize);

  if (lo != nullptr) {
    if ((g_thread_chain != nullptr) && (lo->m_chain != nullptr)) {
      g_thread_chain->delete_thread(lo->m_chain);
    }

    delete lo;
  }

  native_mutex_unlock(&serialize);
}

static PSI_file_locker *lo_get_thread_file_name_locker(
    PSI_file_locker_state *state, PSI_file_key key, PSI_file_operation op,
    const char *name, const void *identity) {
  native_mutex_lock(&serialize);

  LO_file_class *klass = LO_file_class::find_by_key(key);
  if (klass == nullptr) {
    if (lo_param.m_trace_missing_key) {
      LO_stack_trace stack;
      print_file(out_log,
                 "Instrumentation Error: file without a proper key.\n");
      stack.print(out_log);
    }
    if (lo_param.m_debug_missing_key) {
      debug_lock_order_break_here("No file key");
    }
    native_mutex_unlock(&serialize);
    return nullptr;
  }
  LO_thread *lo_thread = get_THR_LO();

  LO_file_locker *lo_state = new LO_file_locker();

  lo_state->m_thread = lo_thread; /* may be null */
  lo_state->m_class = klass;
  lo_state->m_file = nullptr;
  lo_state->m_name = name;
  lo_state->m_operation = op;
  lo_state->m_chain_state = state;

  if ((g_file_chain != nullptr) && (state != nullptr)) {
    lo_state->m_chain_locker = g_file_chain->get_thread_file_name_locker(
        state, klass->get_chain_key(), op, name, identity);
  } else {
    lo_state->m_chain_locker = nullptr;
  }

  native_mutex_unlock(&serialize);
  return reinterpret_cast<PSI_file_locker *>(lo_state);
}

static PSI_file_locker *lo_get_thread_file_stream_locker(
    PSI_file_locker_state *state, PSI_file *file, PSI_file_operation op) {
  LO_file *lo_file = reinterpret_cast<LO_file *>(file);
  PSI_file *lo_file_chain = nullptr;
  LO_thread *lo_thread = get_THR_LO();

  LO_file_locker *lo_state = new LO_file_locker();

  lo_state->m_thread = lo_thread; /* may be null */
  if (lo_file != nullptr) {
    lo_state->m_class = lo_file->get_class();
    lo_file_chain = lo_file->m_chain;
  } else {
    lo_state->m_class = nullptr;
  }
  lo_state->m_file = lo_file; /* may be null */
  lo_state->m_name = nullptr;
  lo_state->m_operation = op;
  lo_state->m_chain_state = state;

  if ((g_file_chain != nullptr) && (state != nullptr)) {
    lo_state->m_chain_locker =
        g_file_chain->get_thread_file_stream_locker(state, lo_file_chain, op);
  } else {
    lo_state->m_chain_locker = nullptr;
  }

  return reinterpret_cast<PSI_file_locker *>(lo_state);
}

static PSI_file_locker *lo_get_thread_file_descriptor_locker(
    PSI_file_locker_state *state, File file, PSI_file_operation op) {
  LO_thread *lo_thread = get_THR_LO();
  LO_file *lo_file = nullptr;

  int index = (int)file;
  if (index >= 0) {
    /*
      See comment in pfs_get_thread_file_descriptor_locker_v2().

      We are about to close a file by descriptor number,
      and the calling code still holds the descriptor.
      Cleanup the file descriptor <--> file instrument association.
      Remove the instrumentation *before* the close to avoid race
      conditions with another thread opening a file
      (that could be given the same descriptor).
    */
    bool remove = (op == PSI_FILE_CLOSE);

    lo_file = lo_file_bindings_find(index, remove);

    if (lo_file == nullptr) {
      /* binlog.cc using my_open(). */
      print_file(out_log, "Error: File I/O on un instrumented file handle %d\n",
                 index);
    }
  }

  LO_file_locker *lo_state = new LO_file_locker();

  lo_state->m_thread = lo_thread; /* may be null */
  if (lo_file != nullptr) {
    lo_state->m_class = lo_file->get_class();
  } else {
    lo_state->m_class = nullptr;
  }
  lo_state->m_file = lo_file;
  lo_state->m_name = nullptr;
  lo_state->m_operation = op;
  lo_state->m_chain_state = state;

  if ((g_file_chain != nullptr) && (state != nullptr)) {
    lo_state->m_chain_locker =
        g_file_chain->get_thread_file_descriptor_locker(state, file, op);
  } else {
    lo_state->m_chain_locker = nullptr;
  }

  return reinterpret_cast<PSI_file_locker *>(lo_state);
}

static void lo_unlock_mutex(PSI_mutex *mutex) {
  LO_thread *lo_thread = get_THR_LO();
  LO_mutex *lo_mutex = reinterpret_cast<LO_mutex *>(mutex);
  if (lo_mutex == nullptr) {
    return;
  }

  native_mutex_lock(&serialize);

  LO_thread::remove_mutex_lock(lo_thread, lo_mutex);

  native_mutex_unlock(&serialize);

  if ((g_mutex_chain != nullptr) && (lo_mutex->m_chain != nullptr)) {
    g_mutex_chain->unlock_mutex(lo_mutex->m_chain);
  }
}

static void lo_unlock_rwlock(PSI_rwlock *rwlock, PSI_rwlock_operation op) {
  LO_thread *lo_thread = get_THR_LO();
  LO_rwlock *lo_rwlock = reinterpret_cast<LO_rwlock *>(rwlock);
  if (lo_rwlock == nullptr) {
    return;
  }

  native_mutex_lock(&serialize);

  LO_thread::remove_rwlock_lock(lo_thread, lo_rwlock, op);

  native_mutex_unlock(&serialize);

  if ((g_rwlock_chain != nullptr) && (lo_rwlock->m_chain != nullptr)) {
    g_rwlock_chain->unlock_rwlock(lo_rwlock->m_chain, op);
  }
}

static void lo_signal_cond(PSI_cond *cond) {
  LO_cond *lo_cond = reinterpret_cast<LO_cond *>(cond);
  if (lo_cond == nullptr) {
    return;
  }

  LO_thread *lo_thread = get_THR_LO();
  if (lo_thread != nullptr) {
    lo_thread->check_signal_broadcast("signal", lo_cond);
  }

  if (g_cond_chain != nullptr) {
    g_cond_chain->signal_cond(lo_cond->m_chain);
  }
}

static void lo_broadcast_cond(PSI_cond *cond) {
  LO_cond *lo_cond = reinterpret_cast<LO_cond *>(cond);
  if (lo_cond == nullptr) {
    return;
  }

  LO_thread *lo_thread = get_THR_LO();
  if (lo_thread != nullptr) {
    lo_thread->check_signal_broadcast("broadcast", lo_cond);
  }

  if (g_cond_chain != nullptr) {
    g_cond_chain->broadcast_cond(lo_cond->m_chain);
  }
}

static PSI_idle_locker *lo_start_idle_wait(PSI_idle_locker_state *state,
                                           const char *src_file,
                                           uint src_line) {
  PSI_idle_locker *chain = nullptr;

  if (g_idle_chain != nullptr) {
    chain = g_idle_chain->start_idle_wait(state, src_file, src_line);
  }

  return chain;
}

static void lo_end_idle_wait(PSI_idle_locker *locker) {
  if (g_idle_chain != nullptr) {
    g_idle_chain->end_idle_wait(locker);
  }
}

static PSI_mutex_locker *lo_start_mutex_wait(PSI_mutex_locker_state *state,
                                             PSI_mutex *mutex,
                                             PSI_mutex_operation op,
                                             const char *src_file,
                                             uint src_line) {
  LO_mutex *lo_mutex = reinterpret_cast<LO_mutex *>(mutex);
  if (lo_mutex == nullptr) {
    return nullptr;
  }
  LO_thread *lo_thread = get_THR_LO();
  LO_mutex_locker *lo_locker;

  native_mutex_lock(&serialize);

  lo_locker = new LO_mutex_locker(lo_thread /* possibly nullptr */, lo_mutex);
  lo_locker->start(src_file, src_line);

  if ((g_mutex_chain != nullptr) && (lo_mutex->m_chain != nullptr)) {
    lo_locker->m_chain = g_mutex_chain->start_mutex_wait(
        state, lo_mutex->m_chain, op, src_file, src_line);
  }

  native_mutex_unlock(&serialize);

  return reinterpret_cast<PSI_mutex_locker *>(lo_locker);
}

/* Not static, printed in stack traces. */
void lo_end_mutex_wait(PSI_mutex_locker *locker, int rc) {
  LO_mutex_locker *lo_locker = reinterpret_cast<LO_mutex_locker *>(locker);
  DBUG_ASSERT(lo_locker != nullptr);

  native_mutex_lock(&serialize);
  if (rc == 0) {
    lo_locker->end();
  }

  if ((g_mutex_chain != nullptr) && (lo_locker->m_chain != nullptr)) {
    g_mutex_chain->end_mutex_wait(lo_locker->m_chain, rc);
  }

  delete lo_locker;
  native_mutex_unlock(&serialize);
}

static PSI_rwlock_locker *lo_start_rwlock_rdwait(PSI_rwlock_locker_state *state,
                                                 PSI_rwlock *rwlock,
                                                 PSI_rwlock_operation op,
                                                 const char *src_file,
                                                 uint src_line) {
  LO_rwlock *lo_rwlock = reinterpret_cast<LO_rwlock *>(rwlock);
  if (lo_rwlock == nullptr) {
    return nullptr;
  }
  LO_thread *lo_thread = get_THR_LO();
  LO_rwlock_locker *lo_locker = nullptr;

  native_mutex_lock(&serialize);

  lo_locker = new LO_rwlock_locker(lo_thread /* possibly null */, lo_rwlock);
  lo_locker->start(op, src_file, src_line);

  if ((g_rwlock_chain != nullptr) && (lo_rwlock->m_chain != nullptr)) {
    lo_locker->m_chain = g_rwlock_chain->start_rwlock_rdwait(
        state, lo_rwlock->m_chain, op, src_file, src_line);
  }

  native_mutex_unlock(&serialize);

  return reinterpret_cast<PSI_rwlock_locker *>(lo_locker);
}

static void lo_end_rwlock_rdwait(PSI_rwlock_locker *locker, int rc) {
  LO_rwlock_locker *lo_locker = reinterpret_cast<LO_rwlock_locker *>(locker);
  DBUG_ASSERT(lo_locker != nullptr);

  native_mutex_lock(&serialize);
  lo_locker->end();

  if ((g_rwlock_chain != nullptr) && (lo_locker->m_chain != nullptr)) {
    g_rwlock_chain->end_rwlock_rdwait(lo_locker->m_chain, rc);
  }

  delete lo_locker;
  native_mutex_unlock(&serialize);
}

static PSI_rwlock_locker *lo_start_rwlock_wrwait(PSI_rwlock_locker_state *state,
                                                 PSI_rwlock *rwlock,
                                                 PSI_rwlock_operation op,
                                                 const char *src_file,
                                                 uint src_line) {
  LO_rwlock *lo_rwlock = reinterpret_cast<LO_rwlock *>(rwlock);
  if (lo_rwlock == nullptr) {
    return nullptr;
  }
  LO_thread *lo_thread = get_THR_LO();
  LO_rwlock_locker *lo_locker = nullptr;

  native_mutex_lock(&serialize);

  lo_locker = new LO_rwlock_locker(lo_thread, lo_rwlock);
  lo_locker->start(op, src_file, src_line);

  if ((g_rwlock_chain != nullptr) && (lo_rwlock->m_chain != nullptr)) {
    lo_locker->m_chain = g_rwlock_chain->start_rwlock_wrwait(
        state, lo_rwlock->m_chain, op, src_file, src_line);
  }

  native_mutex_unlock(&serialize);

  return reinterpret_cast<PSI_rwlock_locker *>(lo_locker);
}

static void lo_end_rwlock_wrwait(PSI_rwlock_locker *locker, int rc) {
  LO_rwlock_locker *lo_locker = reinterpret_cast<LO_rwlock_locker *>(locker);
  DBUG_ASSERT(lo_locker != nullptr);

  native_mutex_lock(&serialize);
  lo_locker->end();

  if ((g_rwlock_chain != nullptr) && (lo_locker->m_chain != nullptr)) {
    g_rwlock_chain->end_rwlock_wrwait(lo_locker->m_chain, rc);
  }

  delete lo_locker;
  native_mutex_unlock(&serialize);
}

static PSI_cond_locker *lo_start_cond_wait(PSI_cond_locker_state *state,
                                           PSI_cond *cond, PSI_mutex *mutex,
                                           PSI_cond_operation op,
                                           const char *src_file,
                                           uint src_line) {
  LO_cond *lo_cond = reinterpret_cast<LO_cond *>(cond);
  if (lo_cond == nullptr) {
    return nullptr;
  }
  LO_mutex *lo_mutex = reinterpret_cast<LO_mutex *>(mutex);
  if (lo_mutex == nullptr) {
    return nullptr;
  }
  LO_thread *lo_thread = get_THR_LO();
  LO_cond_locker *lo_locker = nullptr;

  native_mutex_lock(&serialize);

  lo_locker = new LO_cond_locker(lo_thread, lo_cond, lo_mutex);
  lo_locker->start(src_file, src_line);

  PSI_cond *cond_chain = lo_cond->m_chain;
  PSI_mutex *mutex_chain = lo_mutex->m_chain;
  if ((g_cond_chain != nullptr) && (cond_chain != nullptr) &&
      (mutex_chain != nullptr)) {
    lo_locker->m_chain = g_cond_chain->start_cond_wait(
        state, cond_chain, mutex_chain, op, src_file, src_line);
  }

  native_mutex_unlock(&serialize);

  return reinterpret_cast<PSI_cond_locker *>(lo_locker);
}

static void lo_end_cond_wait(PSI_cond_locker *locker, int rc) {
  LO_cond_locker *lo_locker = reinterpret_cast<LO_cond_locker *>(locker);
  DBUG_ASSERT(lo_locker != nullptr);

  native_mutex_lock(&serialize);
  lo_locker->end();

  PSI_cond_locker *chain = lo_locker->m_chain;
  if ((g_cond_chain != nullptr) && (chain != nullptr)) {
    g_cond_chain->end_cond_wait(chain, rc);
  }

  delete lo_locker;
  native_mutex_unlock(&serialize);
}

static PSI_statement_locker *lo_get_thread_statement_locker_v2(
    PSI_statement_locker_state *state, PSI_statement_key key, const void *cs,
    PSI_sp_share *sp_share) {
  PSI_statement_locker *chain = nullptr;

  if (g_statement_chain != nullptr) {
    chain = g_statement_chain->get_thread_statement_locker(state, key, cs,
                                                           sp_share);
  }

  /* TODO: install a statement locker if needed, to get all notifications. */

  return chain;
}

static PSI_statement_locker *lo_refine_statement_v2(
    PSI_statement_locker *locker, PSI_statement_key key) {
  PSI_statement_locker *chain = nullptr;

  if (g_statement_chain != nullptr) {
    chain = g_statement_chain->refine_statement(locker, key);
  }

  return chain;
}

static void lo_start_statement_v2(PSI_statement_locker *locker, const char *db,
                                  uint db_len, const char *src_file,
                                  uint src_line) {
  if (g_statement_chain != nullptr) {
    g_statement_chain->start_statement(locker, db, db_len, src_file, src_line);
  }
}

static void lo_set_statement_text_v2(PSI_statement_locker *locker,
                                     const char *text, uint text_len) {
  LO_thread *lo_thread = get_THR_LO();

  if (g_statement_chain != nullptr) {
    g_statement_chain->set_statement_text(locker, text, text_len);
  }

  if (lo_thread != nullptr) {
    lo_thread->set_statement_text(text, text_len);
  }
}

static void lo_set_statement_query_id_v2(PSI_statement_locker *locker,
                                         ulonglong query_id) {
  if (g_statement_chain != nullptr) {
    g_statement_chain->set_statement_query_id(locker, query_id);
  }
}

static void lo_set_statement_lock_time_v2(PSI_statement_locker *locker,
                                          ulonglong count) {
  if (g_statement_chain != nullptr) {
    g_statement_chain->set_statement_lock_time(locker, count);
  }
}

static void lo_set_statement_rows_sent_v2(PSI_statement_locker *locker,
                                          ulonglong count) {
  if (g_statement_chain != nullptr) {
    g_statement_chain->set_statement_rows_sent(locker, count);
  }
}

static void lo_set_statement_rows_examined_v2(PSI_statement_locker *locker,
                                              ulonglong count) {
  if (g_statement_chain != nullptr) {
    g_statement_chain->set_statement_rows_examined(locker, count);
  }
}

static void lo_inc_statement_created_tmp_disk_tables_v2(
    PSI_statement_locker *locker, ulong count) {
  if (g_statement_chain != nullptr) {
    g_statement_chain->inc_statement_created_tmp_disk_tables(locker, count);
  }
}

static void lo_inc_statement_created_tmp_tables_v2(PSI_statement_locker *locker,
                                                   ulong count) {
  if (g_statement_chain != nullptr) {
    g_statement_chain->inc_statement_created_tmp_tables(locker, count);
  }
}

static void lo_inc_statement_select_full_join_v2(PSI_statement_locker *locker,
                                                 ulong count) {
  if (g_statement_chain != nullptr) {
    g_statement_chain->inc_statement_select_full_join(locker, count);
  }
}

static void lo_inc_statement_select_full_range_join_v2(
    PSI_statement_locker *locker, ulong count) {
  if (g_statement_chain != nullptr) {
    g_statement_chain->inc_statement_select_full_range_join(locker, count);
  }
}

static void lo_inc_statement_select_range_v2(PSI_statement_locker *locker,
                                             ulong count) {
  if (g_statement_chain != nullptr) {
    g_statement_chain->inc_statement_select_range(locker, count);
  }
}

static void lo_inc_statement_select_range_check_v2(PSI_statement_locker *locker,
                                                   ulong count) {
  if (g_statement_chain != nullptr) {
    g_statement_chain->inc_statement_select_range_check(locker, count);
  }
}

static void lo_inc_statement_select_scan_v2(PSI_statement_locker *locker,
                                            ulong count) {
  if (g_statement_chain != nullptr) {
    g_statement_chain->inc_statement_select_scan(locker, count);
  }
}

static void lo_inc_statement_sort_merge_passes_v2(PSI_statement_locker *locker,
                                                  ulong count) {
  if (g_statement_chain != nullptr) {
    g_statement_chain->inc_statement_sort_merge_passes(locker, count);
  }
}

static void lo_inc_statement_sort_range_v2(PSI_statement_locker *locker,
                                           ulong count) {
  if (g_statement_chain != nullptr) {
    g_statement_chain->inc_statement_sort_range(locker, count);
  }
}

static void lo_inc_statement_sort_rows_v2(PSI_statement_locker *locker,
                                          ulong count) {
  if (g_statement_chain != nullptr) {
    g_statement_chain->inc_statement_sort_rows(locker, count);
  }
}

static void lo_inc_statement_sort_scan_v2(PSI_statement_locker *locker,
                                          ulong count) {
  if (g_statement_chain != nullptr) {
    g_statement_chain->inc_statement_sort_scan(locker, count);
  }
}

static void lo_set_statement_no_index_used_v2(PSI_statement_locker *locker) {
  if (g_statement_chain != nullptr) {
    g_statement_chain->set_statement_no_index_used(locker);
  }
}

static void lo_set_statement_no_good_index_used_v2(
    PSI_statement_locker *locker) {
  if (g_statement_chain != nullptr) {
    g_statement_chain->set_statement_no_good_index_used(locker);
  }
}

static void lo_update_statement_filesort_disk_usage_v2(
    PSI_statement_locker *locker, ulonglong value) {
  if (g_statement_chain != nullptr) {
    g_statement_chain->update_statement_filesort_disk_usage(locker, value);
  }
}

static void lo_update_statement_tmp_table_disk_usage_v2(
    PSI_statement_locker *locker, ulonglong value) {
  if (g_statement_chain != nullptr) {
    g_statement_chain->update_statement_tmp_table_disk_usage(locker, value);
  }
}

static void lo_end_statement_v2(PSI_statement_locker *locker, void *stmt_da) {
  if (g_statement_chain != nullptr) {
    g_statement_chain->end_statement(locker, stmt_da);
  }
}

PSI_prepared_stmt *lo_create_prepared_stmt_v2(void *identity, uint stmt_id,
                                              PSI_statement_locker *locker,
                                              const char *stmt_name,
                                              size_t stmt_name_length,
                                              const char *sql_text,
                                              size_t sql_text_length) {
  PSI_prepared_stmt *chain = nullptr;

  if (g_statement_chain != nullptr) {
    chain = g_statement_chain->create_prepared_stmt(identity, stmt_id, locker,
                                                    stmt_name, stmt_name_length,
                                                    sql_text, sql_text_length);
  }

  return chain;
}

void lo_destroy_prepared_stmt_v2(PSI_prepared_stmt *prepared_stmt) {
  if (g_statement_chain != nullptr) {
    g_statement_chain->destroy_prepared_stmt(prepared_stmt);
  }
}

void lo_reprepare_prepared_stmt_v2(PSI_prepared_stmt *prepared_stmt) {
  if (g_statement_chain != nullptr) {
    g_statement_chain->reprepare_prepared_stmt(prepared_stmt);
  }
}

void lo_execute_prepared_stmt_v2(PSI_statement_locker *locker,
                                 PSI_prepared_stmt *ps) {
  if (g_statement_chain != nullptr) {
    g_statement_chain->execute_prepared_stmt(locker, ps);
  }
}

void lo_set_prepared_stmt_text_v2(PSI_prepared_stmt *prepared_stmt,
                                  const char *text, uint text_len) {
  if (g_statement_chain != nullptr) {
    g_statement_chain->set_prepared_stmt_text(prepared_stmt, text, text_len);
  }
}

PSI_sp_share *lo_get_sp_share_v2(uint sp_type, const char *schema_name,
                                 uint schema_name_length,
                                 const char *object_name,
                                 uint object_name_length) {
  PSI_sp_share *chain = nullptr;

  if (g_statement_chain != nullptr) {
    chain = g_statement_chain->get_sp_share(sp_type, schema_name,
                                            schema_name_length, object_name,
                                            object_name_length);
  }

  return chain;
}

void lo_release_sp_share_v2(PSI_sp_share *s) {
  if (g_statement_chain != nullptr) {
    g_statement_chain->release_sp_share(s);
  }
}

PSI_sp_locker *lo_start_sp_v2(PSI_sp_locker_state *state,
                              PSI_sp_share *sp_share) {
  PSI_sp_locker *chain = nullptr;

  if (g_statement_chain != nullptr) {
    chain = g_statement_chain->start_sp(state, sp_share);
  }

  return chain;
}

void lo_end_sp_v2(PSI_sp_locker *locker) {
  if (g_statement_chain != nullptr) {
    g_statement_chain->end_sp(locker);
  }
}

void lo_drop_sp_v2(uint sp_type, const char *schema_name,
                   uint schema_name_length, const char *object_name,
                   uint object_name_length) {
  if (g_statement_chain != nullptr) {
    g_statement_chain->drop_sp(sp_type, schema_name, schema_name_length,
                               object_name, object_name_length);
  }
}

static void lo_start_file_open_wait(PSI_file_locker *locker,
                                    const char *src_file, uint src_line) {
  LO_file_locker *lo_locker = reinterpret_cast<LO_file_locker *>(locker);
  DBUG_ASSERT(lo_locker != nullptr);
  PSI_file_locker *chain_locker = lo_locker->m_chain_locker;

  if ((g_file_chain != nullptr) && (chain_locker != nullptr)) {
    g_file_chain->start_file_open_wait(chain_locker, src_file, src_line);
  }
}

static PSI_file *lo_end_file_open_wait(PSI_file_locker *locker, void *result) {
  LO_file_locker *lo_locker = reinterpret_cast<LO_file_locker *>(locker);
  DBUG_ASSERT(lo_locker != nullptr);
  PSI_file_locker *chain_locker = lo_locker->m_chain_locker;
  PSI_file_locker_state *chain_state = lo_locker->m_chain_state;

  LO_thread *lo_thread = lo_locker->m_thread;
  const LO_file_class *klass = lo_locker->m_class;
  DBUG_ASSERT(klass != nullptr);

  if (lo_thread != nullptr) {
    lo_thread->check_locks(klass);
  }

  LO_file *lo_file = nullptr;
  switch (lo_locker->m_operation) {
    case PSI_FILE_STREAM_OPEN:
    case PSI_FILE_CREATE:
    case PSI_FILE_OPEN:
      if (result != nullptr) {
        lo_file = new LO_file(klass);
      }
      break;
    default:
      break;
  }

  if ((g_file_chain != nullptr) && (chain_locker != nullptr)) {
    chain_state->m_file =
        g_file_chain->end_file_open_wait(chain_locker, result);
    if (lo_file != nullptr) {
      lo_file->m_chain = chain_state->m_file;
    }
  }

  delete lo_locker;

  return reinterpret_cast<PSI_file *>(lo_file);
}

static void lo_end_file_open_wait_and_bind_to_descriptor(
    PSI_file_locker *locker, File file) {
  LO_file_locker *lo_locker = reinterpret_cast<LO_file_locker *>(locker);
  DBUG_ASSERT(lo_locker != nullptr);
  PSI_file_locker *chain_locker = lo_locker->m_chain_locker;

  int index = (int)file;
  if (index >= 0) {
    LO_file *lo_file = new LO_file(lo_locker->m_class);
    lo_file_bindings_insert(index, lo_file);
  }

  if ((g_file_chain != nullptr) && (chain_locker != nullptr)) {
    g_file_chain->end_file_open_wait_and_bind_to_descriptor(chain_locker, file);
  }

  delete lo_locker;
}

static void lo_end_temp_file_open_wait_and_bind_to_descriptor(
    PSI_file_locker *locker, File file, const char *filename) {
  LO_file_locker *lo_locker = reinterpret_cast<LO_file_locker *>(locker);
  DBUG_ASSERT(lo_locker != nullptr);
  PSI_file_locker *chain_locker = lo_locker->m_chain_locker;

  int index = (int)file;
  if (index >= 0) {
    LO_file *lo_file = new LO_file(lo_locker->m_class);
    lo_file_bindings_insert(index, lo_file);
  }

  if ((g_file_chain != nullptr) && (chain_locker != nullptr)) {
    g_file_chain->end_temp_file_open_wait_and_bind_to_descriptor(
        chain_locker, file, filename);
  }

  delete lo_locker;
}

static void lo_start_file_wait(PSI_file_locker *locker, size_t count,
                               const char *src_file, uint src_line) {
  LO_file_locker *lo_locker = reinterpret_cast<LO_file_locker *>(locker);
  DBUG_ASSERT(lo_locker != nullptr);
  PSI_file_locker *chain_locker = lo_locker->m_chain_locker;

  if ((g_file_chain != nullptr) && (chain_locker != nullptr)) {
    g_file_chain->start_file_wait(chain_locker, count, src_file, src_line);
  }
}

static void lo_end_file_wait(PSI_file_locker *locker, size_t count) {
  LO_file_locker *lo_locker = reinterpret_cast<LO_file_locker *>(locker);
  DBUG_ASSERT(lo_locker != nullptr);
  PSI_file_locker *chain_locker = lo_locker->m_chain_locker;

  if ((g_file_chain != nullptr) && (chain_locker != nullptr)) {
    g_file_chain->end_file_wait(chain_locker, count);
  }

  delete lo_locker;
}

static void lo_start_file_close_wait(PSI_file_locker *locker,
                                     const char *src_file, uint src_line) {
  LO_file_locker *lo_locker = reinterpret_cast<LO_file_locker *>(locker);
  DBUG_ASSERT(lo_locker != nullptr);
  PSI_file_locker *chain_locker = lo_locker->m_chain_locker;

  if ((g_file_chain != nullptr) && (chain_locker != nullptr)) {
    g_file_chain->start_file_close_wait(chain_locker, src_file, src_line);
  }
}

static void lo_end_file_close_wait(PSI_file_locker *locker, int rc) {
  LO_file_locker *lo_locker = reinterpret_cast<LO_file_locker *>(locker);
  DBUG_ASSERT(lo_locker != nullptr);
  PSI_file_locker *chain_locker = lo_locker->m_chain_locker;

  if ((g_file_chain != nullptr) && (chain_locker != nullptr)) {
    g_file_chain->end_file_close_wait(chain_locker, rc);
  }

  if (rc == 0) {
    if (lo_locker->m_file != nullptr) {
      delete lo_locker->m_file;
    }
  }

  delete lo_locker;
}

static void lo_start_file_rename_wait(PSI_file_locker *locker,
                                      size_t count MY_ATTRIBUTE((unused)),
                                      const char *old_name,
                                      const char *new_name,
                                      const char *src_file, uint src_line) {
  LO_file_locker *lo_locker = reinterpret_cast<LO_file_locker *>(locker);
  DBUG_ASSERT(lo_locker != nullptr);
  PSI_file_locker *chain_locker = lo_locker->m_chain_locker;

  if ((g_file_chain != nullptr) && (chain_locker != nullptr)) {
    g_file_chain->start_file_rename_wait(chain_locker, count, old_name,
                                         new_name, src_file, src_line);
  }
}

static void lo_end_file_rename_wait(PSI_file_locker *locker,
                                    const char *old_name, const char *new_name,
                                    int rc) {
  LO_file_locker *lo_locker = reinterpret_cast<LO_file_locker *>(locker);
  DBUG_ASSERT(lo_locker != nullptr);
  PSI_file_locker *chain_locker = lo_locker->m_chain_locker;

  if ((g_file_chain != nullptr) && (chain_locker != nullptr)) {
    g_file_chain->end_file_rename_wait(chain_locker, old_name, new_name, rc);
  }

  delete lo_locker;
}

static PSI_digest_locker *lo_digest_start_v2(PSI_statement_locker *locker) {
  PSI_digest_locker *chain = nullptr;

  if (g_statement_chain != nullptr) {
    chain = g_statement_chain->digest_start(locker);
  }

  return chain;
}

static void lo_digest_end_v2(PSI_digest_locker *locker,
                             const sql_digest_storage *digest) {
  if (g_statement_chain != nullptr) {
    g_statement_chain->digest_end(locker, digest);
  }
}

static int lo_set_thread_connect_attrs(const char *a, uint b, const void *c) {
  int chain = 0;

  if (g_thread_chain != nullptr) {
    chain = g_thread_chain->set_thread_connect_attrs(a, b, c);
  }

  return chain;
}

static void lo_get_current_thread_event_id(ulonglong *thread_internal_id,
                                           ulonglong *event_id) {
  if (g_thread_chain != nullptr) {
    g_thread_chain->get_current_thread_event_id(thread_internal_id, event_id);
  } else {
    *thread_internal_id = 0;
    *event_id = 0;
  }
}

static void lo_get_thread_event_id(PSI_thread *psi,
                                   ulonglong *thread_internal_id,
                                   ulonglong *event_id) {
  LO_thread *lo = reinterpret_cast<LO_thread *>(psi);

  if ((g_thread_chain != nullptr) && (lo != nullptr)) {
    g_thread_chain->get_thread_event_id(lo->m_chain, thread_internal_id,
                                        event_id);
  } else {
    *thread_internal_id = 0;
    *event_id = 0;
  }
}

static int lo_get_thread_system_attrs(PSI_thread_attrs *thread_attrs) {
  int rc = 0;

  if (g_thread_chain != nullptr) {
    rc = g_thread_chain->get_thread_system_attrs(thread_attrs);
  }

  return rc;
}

static int lo_get_thread_system_attrs_by_id(PSI_thread *thread,
                                            ulonglong thread_id,
                                            PSI_thread_attrs *thread_attrs) {
  LO_thread *lo = reinterpret_cast<LO_thread *>(thread);
  int rc = 0;

  if (lo != nullptr) {
    if ((g_thread_chain != nullptr) && (lo->m_chain != nullptr)) {
      rc = g_thread_chain->get_thread_system_attrs_by_id(lo->m_chain, thread_id,
                                                         thread_attrs);
    }
  }

  return rc;
}

static int lo_register_notification(const PSI_notification *callbacks,
                                    bool with_ref_count) {
  int rc = 0;

  if (g_thread_chain != nullptr) {
    rc = g_thread_chain->register_notification(callbacks, with_ref_count);
  }

  return rc;
}

static int lo_unregister_notification(int handle) {
  int rc = 0;

  if (g_thread_chain != nullptr) {
    rc = g_thread_chain->unregister_notification(handle);
  }

  return rc;
}

static void lo_notify_session_connect(PSI_thread *thread) {
  LO_thread *lo = reinterpret_cast<LO_thread *>(thread);

  if (lo != nullptr) {
    if ((g_thread_chain != nullptr) && (lo->m_chain != nullptr)) {
      g_thread_chain->notify_session_connect(lo->m_chain);
    }
  }
}

static void lo_notify_session_disconnect(PSI_thread *thread) {
  LO_thread *lo = reinterpret_cast<LO_thread *>(thread);

  if (lo != nullptr) {
    if ((g_thread_chain != nullptr) && (lo->m_chain != nullptr)) {
      g_thread_chain->notify_session_disconnect(lo->m_chain);
    }
  }
}

static void lo_notify_session_change_user(PSI_thread *thread) {
  LO_thread *lo = reinterpret_cast<LO_thread *>(thread);

  if (lo != nullptr) {
    if ((g_thread_chain != nullptr) && (lo->m_chain != nullptr)) {
      g_thread_chain->notify_session_change_user(lo->m_chain);
    }
  }
}

PSI_thread_service_v3 LO_thread_v3 = {lo_register_thread,
                                      lo_spawn_thread,
                                      lo_new_thread,
                                      lo_set_thread_id,
                                      lo_get_current_thread_internal_id,
                                      lo_get_thread_internal_id,
                                      lo_get_thread_by_id,
                                      lo_set_thread_THD,
                                      lo_set_thread_os_id,
                                      lo_get_thread,
                                      lo_set_thread_user,
                                      lo_set_thread_account,
                                      lo_set_thread_db,
                                      lo_set_thread_command,
                                      lo_set_connection_type,
                                      lo_set_thread_start_time,
                                      lo_set_thread_info,
                                      lo_set_thread_resource_group,
                                      lo_set_thread_resource_group_by_id,
                                      lo_set_thread,
                                      lo_aggregate_thread_status,
                                      lo_delete_current_thread,
                                      lo_delete_thread,
                                      lo_set_thread_connect_attrs,
                                      lo_get_current_thread_event_id,
                                      lo_get_thread_event_id,
                                      lo_get_thread_system_attrs,
                                      lo_get_thread_system_attrs_by_id,
                                      lo_register_notification,
                                      lo_unregister_notification,
                                      lo_notify_session_connect,
                                      lo_notify_session_disconnect,
                                      lo_notify_session_change_user};

static void *lo_get_thread_interface(int version) {
  switch (version) {
    case PSI_THREAD_VERSION_1:
      return nullptr;
    case PSI_THREAD_VERSION_2:
      return nullptr;
    case PSI_THREAD_VERSION_3:
      return &LO_thread_v3;
    default:
      return nullptr;
  }
}

struct PSI_thread_bootstrap LO_thread_bootstrap = {lo_get_thread_interface};

PSI_mutex_service_v1 LO_mutex = {lo_register_mutex, lo_init_mutex,
                                 lo_destroy_mutex,  lo_start_mutex_wait,
                                 lo_end_mutex_wait, lo_unlock_mutex};

static void *lo_get_mutex_interface(int version) {
  switch (version) {
    case PSI_MUTEX_VERSION_1:
      return &LO_mutex;
    default:
      return nullptr;
  }
}

struct PSI_mutex_bootstrap LO_mutex_bootstrap = {lo_get_mutex_interface};

PSI_rwlock_service_v2 LO_rwlock = {
    lo_register_rwlock,     lo_init_rwlock,       lo_destroy_rwlock,
    lo_start_rwlock_rdwait, lo_end_rwlock_rdwait, lo_start_rwlock_wrwait,
    lo_end_rwlock_wrwait,   lo_unlock_rwlock,
};

static void *lo_get_rwlock_interface(int version) {
  switch (version) {
    case PSI_RWLOCK_VERSION_1:
      return nullptr;
    case PSI_RWLOCK_VERSION_2:
      return &LO_rwlock;
    default:
      return nullptr;
  }
}

struct PSI_rwlock_bootstrap LO_rwlock_bootstrap = {lo_get_rwlock_interface};

PSI_cond_service_v1 LO_cond = {
    lo_register_cond,  lo_init_cond,       lo_destroy_cond, lo_signal_cond,
    lo_broadcast_cond, lo_start_cond_wait, lo_end_cond_wait};

static void *lo_get_cond_interface(int version) {
  switch (version) {
    case PSI_COND_VERSION_1:
      return &LO_cond;
    default:
      return nullptr;
  }
}

struct PSI_cond_bootstrap LO_cond_bootstrap = {lo_get_cond_interface};

PSI_file_service_v2 LO_file = {
    lo_register_file,
    lo_create_file,
    lo_get_thread_file_name_locker,
    lo_get_thread_file_stream_locker,
    lo_get_thread_file_descriptor_locker,
    lo_start_file_open_wait,
    lo_end_file_open_wait,
    lo_end_file_open_wait_and_bind_to_descriptor,
    lo_end_temp_file_open_wait_and_bind_to_descriptor,
    lo_start_file_wait,
    lo_end_file_wait,
    lo_start_file_close_wait,
    lo_end_file_close_wait,
    lo_start_file_rename_wait,
    lo_end_file_rename_wait};

static void *lo_get_file_interface(int version) {
  switch (version) {
    case PSI_FILE_VERSION_1:
      return &LO_file;
    default:
      return nullptr;
  }
}

struct PSI_file_bootstrap LO_file_bootstrap = {lo_get_file_interface};

PSI_idle_service_v1 LO_idle = {lo_start_idle_wait, lo_end_idle_wait};

static void *lo_get_idle_interface(int version) {
  switch (version) {
    case PSI_IDLE_VERSION_1:
      return &LO_idle;
    default:
      return nullptr;
  }
}

struct PSI_idle_bootstrap LO_idle_bootstrap = {lo_get_idle_interface};

PSI_statement_service_v2 LO_statement_v2 = {
    lo_register_statement_v2,
    lo_get_thread_statement_locker_v2,
    lo_refine_statement_v2,
    lo_start_statement_v2,
    lo_set_statement_text_v2,
    lo_set_statement_query_id_v2,
    lo_set_statement_lock_time_v2,
    lo_set_statement_rows_sent_v2,
    lo_set_statement_rows_examined_v2,
    lo_inc_statement_created_tmp_disk_tables_v2,
    lo_inc_statement_created_tmp_tables_v2,
    lo_inc_statement_select_full_join_v2,
    lo_inc_statement_select_full_range_join_v2,
    lo_inc_statement_select_range_v2,
    lo_inc_statement_select_range_check_v2,
    lo_inc_statement_select_scan_v2,
    lo_inc_statement_sort_merge_passes_v2,
    lo_inc_statement_sort_range_v2,
    lo_inc_statement_sort_rows_v2,
    lo_inc_statement_sort_scan_v2,
    lo_set_statement_no_index_used_v2,
    lo_set_statement_no_good_index_used_v2,
    lo_end_statement_v2,
    lo_create_prepared_stmt_v2,
    lo_destroy_prepared_stmt_v2,
    lo_reprepare_prepared_stmt_v2,
    lo_execute_prepared_stmt_v2,
    lo_set_prepared_stmt_text_v2,
    lo_digest_start_v2,
    lo_digest_end_v2,
    lo_get_sp_share_v2,
    lo_release_sp_share_v2,
    lo_start_sp_v2,
    lo_end_sp_v2,
    lo_drop_sp_v2};

static void *lo_get_statement_interface(int version) {
  switch (version) {
    case PSI_STATEMENT_VERSION_1:
      return nullptr;
    case PSI_STATEMENT_VERSION_2:
      return &LO_statement_v2;
    default:
      return nullptr;
  }
}

struct PSI_statement_bootstrap LO_statement_bootstrap = {
    lo_get_statement_interface};

} /* extern "C" */

LO_global_param lo_param;

int LO_load_graph(LO_graph *g);

int LO_init(LO_global_param *param, PSI_thread_bootstrap **thread_bootstrap,
            PSI_mutex_bootstrap **mutex_bootstrap,
            PSI_rwlock_bootstrap **rwlock_bootstrap,
            PSI_cond_bootstrap **cond_bootstrap,
            PSI_file_bootstrap **file_bootstrap, PSI_socket_bootstrap **,
            PSI_table_bootstrap **, PSI_mdl_bootstrap **,
            PSI_idle_bootstrap **idle_bootstrap, PSI_stage_bootstrap **,
            PSI_statement_bootstrap **statement_bootstrap,
            PSI_transaction_bootstrap **, PSI_memory_bootstrap **) {
  int rc;
  native_mutex_init(&serialize, nullptr);
  native_mutex_init(&serialize_logs, nullptr);

  native_mutex_lock(&serialize);

  global_graph = new LO_graph();

  char filename[1024];
  time_t now = time(nullptr);

  /*
    Have to use time + pid,
    because filenames with pid alone are recycled,
    when running a full test suite.
  */
  if (param->m_out_dir != nullptr) {
    safe_snprintf(filename, sizeof(filename), "%s/lock_order-%ju-%d.log",
                  param->m_out_dir, (uintmax_t)now, getpid());
  } else {
    safe_snprintf(filename, sizeof(filename), "lock_order-%ju-%d.log",
                  (uintmax_t)now, getpid());
  }

  out_log = fopen(filename, "w");

  if (out_log == nullptr) {
    LogErr(ERROR_LEVEL, ER_LOCK_ORDER_FAILED_WRITE_FILE, filename);
    perror(filename);
    native_mutex_unlock(&serialize);
    return 1;
  }

  print_file(out_log, "-- begin lock order\n");

  if (param->m_print_txt) {
    if (param->m_out_dir != nullptr) {
      safe_snprintf(filename, sizeof(filename), "%s/lock_order.txt",
                    param->m_out_dir);
    } else {
      strncpy(filename, "lock_order.txt", sizeof(filename));
    }

    out_txt = fopen(filename, "w");
    if (out_txt == nullptr) {
      LogErr(ERROR_LEVEL, ER_LOCK_ORDER_FAILED_WRITE_FILE, filename);
      perror(filename);
      native_mutex_unlock(&serialize);
      return 1;
    }
  }

  if (*thread_bootstrap != nullptr) {
    g_thread_chain = (PSI_thread_service_t *)(*thread_bootstrap)
                         ->get_interface(PSI_CURRENT_THREAD_VERSION);
  }
  *thread_bootstrap = &LO_thread_bootstrap;

  if (*mutex_bootstrap != nullptr) {
    g_mutex_chain = (PSI_mutex_service_t *)(*mutex_bootstrap)
                        ->get_interface(PSI_CURRENT_MUTEX_VERSION);
  }
  *mutex_bootstrap = &LO_mutex_bootstrap;

  if (*rwlock_bootstrap != nullptr) {
    g_rwlock_chain = (PSI_rwlock_service_t *)(*rwlock_bootstrap)
                         ->get_interface(PSI_CURRENT_RWLOCK_VERSION);
  }
  *rwlock_bootstrap = &LO_rwlock_bootstrap;

  if (*cond_bootstrap != nullptr) {
    g_cond_chain = (PSI_cond_service_t *)(*cond_bootstrap)
                       ->get_interface(PSI_CURRENT_COND_VERSION);
  }
  *cond_bootstrap = &LO_cond_bootstrap;

  if (*file_bootstrap != nullptr) {
    g_file_chain = (PSI_file_service_t *)(*file_bootstrap)
                       ->get_interface(PSI_CURRENT_FILE_VERSION);
  }
  *file_bootstrap = &LO_file_bootstrap;

  if (*idle_bootstrap != nullptr) {
    g_idle_chain = (PSI_idle_service_t *)(*idle_bootstrap)
                       ->get_interface(PSI_CURRENT_IDLE_VERSION);
  }
  *idle_bootstrap = &LO_idle_bootstrap;

  if (*statement_bootstrap != nullptr) {
    g_statement_chain = (PSI_statement_service_t *)(*statement_bootstrap)
                            ->get_interface(PSI_CURRENT_STATEMENT_VERSION);
  }
  *statement_bootstrap = &LO_statement_bootstrap;

  rc = LO_load_graph(global_graph);

  native_mutex_unlock(&serialize);
  return rc;
}

int LO_load_dependencies(LO_graph *g, const char *filename) {
  int rc;
  if (filename != nullptr) {
    FILE *data = fopen(filename, "r");
    if (data != nullptr) {
      MEM_ROOT parser_root(PSI_NOT_INSTRUMENTED, 4096);
      LO_parser_param param;
      param.m_scanner = nullptr;
      param.m_memroot = &parser_root;
      param.m_graph = g;
      param.m_data = data;
      param.m_filename = filename;

      LOCK_ORDER_lex_init_extra(&parser_root, &param.m_scanner);
      LOCK_ORDER_set_in(data, param.m_scanner);
      rc = LOCK_ORDER_parse(&param);
      LOCK_ORDER_lex_destroy(param.m_scanner);
      fclose(data);
    } else {
      LogErr(ERROR_LEVEL, ER_LOCK_ORDER_FAILED_READ_FILE, filename);
      perror(filename);
      rc = 1;
    }
  } else {
    /* Dependency file is optional. */
    rc = 0;
  }
  return rc;
}

int LO_load_graph(LO_graph *g) {
  int rc;
  rc = LO_load_dependencies(g, lo_param.m_dependencies_1);
  if (rc != 0) {
    return rc;
  }
  rc = LO_load_dependencies(g, lo_param.m_dependencies_2);
  return rc;
}

void LO_add_authorised_arc(LO_graph *g, const LO_authorised_arc *arc) {
  g->add_arc(arc);
}

void LO_add_node_properties(LO_graph *g, const LO_node_properties *prop) {
  LO_node *to =
      g->find_operation_node(prop->m_name, false, nullptr, prop->m_operation);
  if (to != nullptr) {
    if (prop->m_flags & LO_FLAG_IGNORED) {
      to->set_ignored();
    }

#ifdef LATER
    // TODO: trace feature on mutex, rwlock, ...
    if (prop->m_flags & LO_FLAG_TRACE) {
      to->set_trace();
    }
#endif

    return;
  }

  /*
    The LO_node_properties structure is defined to expose a clean interface
    to the lexer / parser.
    Internally, treat as a special "*" -> "TO" arc,
    so it gets into the unresolved queue.
  */
  LO_authorised_arc *prop_arc = new LO_authorised_arc();
  prop_arc->m_from_name = deep_copy_string("*");
  prop_arc->m_from_state = nullptr;
  prop_arc->m_to_name = deep_copy_string(prop->m_name);
  prop_arc->m_op_recursive = false;
  prop_arc->m_to_operation = deep_copy_string(prop->m_operation);
  prop_arc->m_flags = prop->m_flags;
  prop_arc->m_constraint = nullptr;
  prop_arc->m_comment = nullptr;
  g->add_unresolved_arc(prop_arc);
}

void LO_activate() { check_activated = true; }

void LO_dump() { global_graph->dump_txt(); }

void LO_cleanup() {
  LO_thread *lo = nullptr;

  LO_thread_list::const_iterator it;
  for (it = LO_thread::g_threads.begin(); it != LO_thread::g_threads.end();
       it++) {
    lo = *it;
    print_file(out_log, "Warning: Found running thread during shutdown\n");
    lo->dump(out_log);
    lo->clear_all_locks();
    /*
      This thread is still running, and we are shutting down.
      Flag it as runaway, so we can at least free the LO_thread_class.
    */
    lo->set_runaway();
  }

  delete global_graph;
  global_graph = nullptr;

  LO_mutex_class::destroy_all();
  LO_rwlock_class::destroy_all();
  LO_cond_class::destroy_all();
  LO_file_class::destroy_all();
  LO_thread_class::destroy_all();

  if (out_log != nullptr) {
    print_file(out_log, "-- end lock order\n");
    fclose(out_log);
    out_log = nullptr;
  }

  if (out_txt != nullptr) {
    fclose(out_txt);
    out_txt = nullptr;
  }

  native_mutex_destroy(&serialize);
  native_mutex_destroy(&serialize_logs);

  /* Crash failing tests */
  DBUG_ASSERT(debugger_calls == 0);
}

PSI_thread *LO_get_chain_thread(PSI_thread *thread) {
  PSI_thread *chain;
  if (lo_param.m_enabled) {
    LO_thread *lo = reinterpret_cast<LO_thread *>(thread);
    if (lo == nullptr) {
      return nullptr;
    }
    chain = lo->m_chain;
  } else {
    chain = thread;
  }
  return chain;
}
