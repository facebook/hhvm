/* Copyright (c) 2015, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef SQL_THD_INTERNAL_API_INCLUDED
#define SQL_THD_INTERNAL_API_INCLUDED

/*
  This file defines THD-related API calls that are meant for internal
  usage (e.g. InnoDB, Thread Pool) only. There are therefore no stabilty
  guarantees.
*/

#include <stddef.h>
#include <sys/types.h>

#include <list>
#include <string>
#include <utility>

#include "dur_prop.h"  // durability_properties
#include "lex_string.h"
#include "m_ctype.h"
#include "mysql/components/services/psi_thread_bits.h"
#include "sql/handler.h"  // enum_tx_isolation

class THD;
class partition_info;

/**
  Set up various THD data for a new connection

  @param              thd            THD object
  @param              stack_start    Start of stack for connection
  @param              bound          True if bound to a physical thread.
  @param              psi_key        Instrumentation key for the thread.
*/
void thd_init(THD *thd, char *stack_start, bool bound, PSI_thread_key psi_key);

/**
  Create a THD and do proper initialization of it.

  @param enable_plugins     Should dynamic plugin support be enabled?
  @param background_thread  Is this a background thread?
  @param bound              True if bound to a physical thread.
  @param psi_key            Instrumentation key for the thread.

  @note Dynamic plugin support is only possible for THDs that
        are created after the server has initialized properly.
  @note THDs for background threads are currently not added to
        the global THD list. So they will e.g. not be visible in
        SHOW PROCESSLIST and the server will not wait for them to
        terminate during shutdown.
*/
THD *create_thd(bool enable_plugins, bool background_thread, bool bound,
                PSI_thread_key psi_key);

/**
  Cleanup the THD object, remove it from the global list of THDs
  and delete it.

  @param    thd   pointer to THD object.
*/
void destroy_thd(THD *thd);

/**
  Set thread stack in THD object

  @param thd              Thread object
  @param stack_start      Start of stack to set in THD object
*/
void thd_set_thread_stack(THD *thd, const char *stack_start);

/**
  Returns the partition_info working copy.
  Used to see if a table should be created with partitioning.

  @param thd thread context

  @return Pointer to the working copy of partition_info or NULL.
*/
partition_info *thd_get_work_part_info(THD *thd);

enum_tx_isolation thd_get_trx_isolation(const THD *thd);

const CHARSET_INFO *thd_charset(THD *thd);

/**
  Get the current query string for the thread.

  @param thd   The MySQL internal thread pointer

  @return query string and length. May be non-null-terminated.

  @note This function is not thread safe and should only be called
        from the thread owning thd. @see thd_query_safe().
*/
LEX_CSTRING thd_query_unsafe(THD *thd);

/**
  Get the current query string for the thread.

  @param thd     The MySQL internal thread pointer
  @param buf     Buffer where the query string will be copied
  @param buflen  Length of the buffer

  @return Length of the query

  @note This function is thread safe as the query string is
        accessed under mutex protection and the string is copied
        into the provided buffer. @see thd_query_unsafe().
*/
size_t thd_query_safe(THD *thd, char *buf, size_t buflen);

void thd_query_safe(THD *thd, std::string *query);

/**
  Check if a user thread is a replication slave thread
  @param thd user thread
  @retval 0 the user thread is not a replication slave thread
  @retval 1 the user thread is a replication slave thread
*/
int thd_slave_thread(const THD *thd);

/**
  Check if a user thread is running a non-transactional update
  @param thd user thread
  @retval 0 the user thread is not running a non-transactional update
  @retval 1 the user thread is running a non-transactional update
*/
int thd_non_transactional_update(const THD *thd);

/**
  Get the user thread's binary logging format
  @param thd user thread
  @return Value to be used as index into the binlog_format_names array
*/
int thd_binlog_format(const THD *thd);

/**
  Check if binary logging is filtered for thread's current db.
  @param thd Thread handle
  @retval 1 the query is not filtered, 0 otherwise.
*/
bool thd_binlog_filter_ok(const THD *thd);

/**
  Check if the query may generate row changes which may end up in the binary.
  @param thd Thread handle
  @retval 1 the query may generate row changes, 0 otherwise.
*/
bool thd_sqlcom_can_generate_row_events(const THD *thd);

/**
  Gets information on the durability property requested by a thread.
  @param thd Thread handle
  @return a durability property.
*/
durability_properties thd_get_durability_property(const THD *thd);

/**
  Get the auto_increment_offset auto_increment_increment.
  @param thd Thread object
  @param off auto_increment_offset
  @param inc auto_increment_increment
*/
void thd_get_autoinc(const THD *thd, ulong *off, ulong *inc);

/**
  Is strict sql_mode set.
  Needed by InnoDB.
  @param thd	Thread object
  @return True if sql_mode has strict mode (all or trans).
    @retval true  sql_mode has strict mode (all or trans).
    @retval false sql_mode has not strict mode (all or trans).
*/
bool thd_is_strict_mode(const THD *thd);

/**
  Is an error set in the DA.
  Needed by InnoDB to catch behavior modified by an error handler.
  @param thd	Thread object
  @return True if THD::is_error() returns true.
    @retval true  An error has been raised.
    @retval false No error has been raised.
*/
bool thd_is_error(const THD *thd);

/**
  Does current user have access of SUPER_ACL?

  @param thd	Thread object
  @return true if has access, false otherwise.
*/
bool thd_is_super(const THD *thd);

/**
  Client host or IP address.

  @param thd	Thread object
  @return String with host or IP info.
*/
LEX_CSTRING thd_host_or_ip(THD *thd);

/**
  Test a file path whether it is same as mysql data directory path.

  @param path null terminated character string

  @retval true The path is different from mysql data directory.
  @retval false The path is same as mysql data directory.
*/
bool is_mysql_datadir_path(const char *path);

/**
  Create a temporary file.

  @details
  The temporary file is created in a location specified by the parameter
  path. if path is null, then it will be created on the location given
  by the mysql server configuration (--tmpdir option).  The caller
  does not need to delete the file, it will be deleted automatically.

  @param path	location for creating temporary file
  @param prefix	prefix for temporary file name
  @retval -1	error
  @retval >=0	a file handle that can be passed to dup or my_close
*/

int mysql_tmpfile_path(const char *path, const char *prefix);

/**
  Check if the server is in the process of being initialized.

  Check the thread type of the THD. If this is a thread type
  being used for initializing the DD or the server, return
  true.

  @param   thd    Needed since this is an opaque type in the SE.

  @retval  true   The thread is a bootstrap thread.
  @retval  false  The thread is not a bootstrap thread.
*/

bool thd_is_bootstrap_thread(THD *thd);

/**
  Is statement updating the data dictionary tables.

  @details
  The thread switches to the data dictionary tables update context using
  the dd::Update_dictionary_tables_ctx while updating dictionary tables.
  If thread is in this context then the method returns true otherwise
  false.
  This method is used by the InnoDB while updating the tables to mark
  transaction as DDL if this method returns true.

  @param  thd     Thread handle.

  @retval true    Updates data dictionary tables.
  @retval false   Otherwise.
*/
bool thd_is_dd_update_stmt(const THD *thd);

my_thread_id thd_thread_id(const THD *thd);

/**
  Get tables in the query. The tables are returned as a list of pairs
  where the first value is the dbname and the second value is the table name.

  @param  thd  Thread pointer

  @return List of pairs: dbname, table name
 */
std::list<std::pair<const char *, const char *>> thd_get_query_tables(THD *thd);

/**
  Get the value of the query attribute

  @param thd       The MySQL internal thread pointer
  @param qattr_key Name of the query attribute

  @return Value of the query attribute 'qattr_key'
*/
const std::string &thd_get_query_attr(THD *thd, const std::string &qattr_key);

/**
  Get the value of the connection attribute

  @param thd       The MySQL internal thread pointer
  @param cattr_key Name of the connection attribute

  @return Value of the query attribute 'cattr_key'
*/
const std::string &thd_get_connection_attr(THD *thd,
                                           const std::string &cattr_key);

/**
  Get the query SQL ID

  @param thd       The MySQL internal thread pointer

  @return the SQL ID of the query
*/
const std::string thd_get_sql_id(THD *thd);

/**
  Invoke yield_cond.

  @return true if should yield, false otherwise.
*/
bool thd_yield_cond(THD *thd);

/*
  Same as thd_security_context but with the ability to return query digest
  instead of real query
*/
char *thd_security_context_internal(THD *thd, char *buffer, size_t length,
                                    size_t max_query_len,
                                    bool show_query_digest);

/**
  Get currently held mutex (for cond wait).

  @return current mutex if held, nullptr otherwise.
*/
mysql_mutex_t *thd_current_mutex(THD *thd);

/**
  Get currently held mutex (for cond wait).

  @return current mutex if held, nullptr otherwise.
*/
mysql_mutex_t *thd_current_mutex(THD *thd);

/**
  Set thread priority.
*/
void thd_set_priority(THD *thd);

void thd_add_response_attr(THD *thd, const std::string &rattr_key,
                           const std::string &rattr_val);

#endif  // SQL_THD_INTERNAL_API_INCLUDED
