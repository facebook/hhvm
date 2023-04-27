/* Copyright (c) 2010, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/bootstrap.h"

#include "my_config.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <string>

#include "m_string.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_loglevel.h"
#include "my_sys.h"
#include "my_thread.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/components/services/log_shared.h"
#include "mysql/psi/mysql_file.h"
#include "mysql/psi/mysql_thread.h"
#include "mysql_com.h"
#include "mysqld_error.h"
#include "sql/auth/sql_security_ctx.h"
#include "sql/bootstrap_impl.h"
#include "sql/error_handler.h"  // Internal_error_handler
#include "sql/log.h"
#include "sql/mysqld.h"              // key_file_init
#include "sql/mysqld_thd_manager.h"  // Global_THD_manager
#include "sql/protocol_classic.h"
#include "sql/query_options.h"
#include "sql/set_var.h"
#include "sql/sql_bootstrap.h"
#include "sql/sql_class.h"    // THD
#include "sql/sql_connect.h"  // close_connection
#include "sql/sql_error.h"
#include "sql/sql_initialize.h"
#include "sql/sql_lex.h"
#include "sql/sql_parse.h"  // mysql_parse
#include "sql/sql_profile.h"
#include "sql/sys_vars_shared.h"  // intern_find_sys_var
#include "sql/system_variables.h"
#include "sql/thd_raii.h"
#include "sql/transaction_info.h"

namespace bootstrap {

int File_command_iterator::next(std::string &query) {
  static char query_buffer[MAX_BOOTSTRAP_QUERY_SIZE];
  size_t length = 0;
  int rc;

  rc = read_bootstrap_query(query_buffer, &length, m_input, m_fgets_fn,
                            &m_parser_state);
  if (rc == READ_BOOTSTRAP_SUCCESS) {
    query.assign(query_buffer, length);
  }
  return rc;
}

void File_command_iterator::report_error_details(log_function_t log) {
  m_parser_state.report_error_details(log);
}

static char *mysql_file_fgets_fn(char *buffer, size_t size, MYSQL_FILE *input,
                                 int *error) {
  char *line = mysql_file_fgets(buffer, static_cast<int>(size), input);
  if (error) {
    *error = (line == nullptr) ? ferror(input->m_file) : 0;
  }
  return line;
}

File_command_iterator::~File_command_iterator() {}

static void bootstrap_log_error(const char *message) {
  my_printf_error(ER_UNKNOWN_ERROR, "%s", MYF(0), message);
}

struct handle_bootstrap_args {
  THD *m_thd;
  bootstrap_functor m_bootstrap_handler;
  const char *m_file_name;
  MYSQL_FILE *m_file;
  bool m_bootstrap_error;
};

static int process_iterator(THD *thd, Command_iterator *it,
                            bool enforce_invariants);

static bool handle_bootstrap_impl(handle_bootstrap_args *args) {
  DBUG_TRACE;

  THD *thd = args->m_thd;
  int rc;

  thd->thread_stack = (char *)&thd;
  thd->security_context()->assign_user(STRING_WITH_LEN("boot"));
  thd->security_context()->skip_grants("", "");

  /*
    Make the "client" handle multiple results. This is necessary
    to enable stored procedures with SELECTs and Dynamic SQL
    in init-file.
  */
  thd->get_protocol_classic()->add_client_capability(CLIENT_MULTI_RESULTS);

  thd->init_query_mem_roots();

  if (opt_initialize) {
    /*
      During --initialize, the server will also read SQL statements from a
      file submitted with --init-file. While processing the compiled-in
      statements, DD table access is permitted. This is needed as a short
      term solution to allow SRS data to be entered by INSERT statements
      instead of CREATE statements.
    */
    DBUG_ASSERT(thd->system_thread == SYSTEM_THREAD_SERVER_INITIALIZE);

    /*
      The server must avoid logging compiled statements into the binary log
      (and generating GTIDs for them when GTID_MODE is ON) during bootstrap/
      initialize procedures.
      We disable SQL_LOG_BIN session variable while processing compiled
      statements.
    */
    Disable_binlog_guard disable_binlog(thd);
    Disable_sql_log_bin_guard disable_sql_log_bin(thd);

    Compiled_in_command_iterator comp_iter;
    rc = process_iterator(thd, &comp_iter, true);

    thd->system_thread = SYSTEM_THREAD_INIT_FILE;

    if (rc != 0) {
      return true;
    }
  }

  if (args->m_file != nullptr) {
    /*
      We must not allow the statements
      from an init file to access the DD tables. Thus, whenever we execute a
      statement from an init file, we must make sure that the thread type is
      set to the appropriate value.
    */
    DBUG_ASSERT(thd->system_thread == SYSTEM_THREAD_INIT_FILE);

    File_command_iterator file_iter(args->m_file_name, args->m_file,
                                    mysql_file_fgets_fn);
    rc = process_iterator(thd, &file_iter, false);
    if (rc != 0) {
      return true;
    }
  }

  return false;
}

static int process_iterator(THD *thd, Command_iterator *it,
                            bool enforce_invariants MY_ATTRIBUTE((unused))) {
  std::string query;
  Key_length_error_handler error_handler;
  bool error = false;

  const bool saved_sql_log_bin MY_ATTRIBUTE((unused)) =
      thd->variables.sql_log_bin;
  const ulonglong invariant_bits MY_ATTRIBUTE((unused)) = OPTION_BIN_LOG;
  const ulonglong saved_option_bits MY_ATTRIBUTE((unused)) =
      thd->variables.option_bits & invariant_bits;

  it->begin();

  for (;;) {
    int rc;

    rc = it->next(query);

    if (rc == READ_BOOTSTRAP_EOF) {
      break;
    }

    /*
      Check for bootstrap file errors. SQL syntax errors will be
      caught below.
    */
    if (rc != READ_BOOTSTRAP_SUCCESS) {
      /*
        mysql_parse() may have set a successful error status for the previous
        query. We must clear the error status to report the bootstrap error.
      */
      thd->get_stmt_da()->reset_diagnostics_area();

      it->report_error_details(bootstrap_log_error);

      thd->send_statement_status();
      error = true;
      break;
    }

    char *query_copy = static_cast<char *>(thd->alloc(query.length() + 1));
    if (query_copy == nullptr) {
      /* purecov: begin inspected */
      error = true;
      break;
      /* purecov: end */
    }
    memcpy(query_copy, query.c_str(), query.length());
    query_copy[query.length()] = '\0';
    thd->set_query(query_copy, query.length());
    thd->set_query_id(next_query_id());
    DBUG_PRINT("query", ("%-.4096s", thd->query().str));
#if defined(ENABLED_PROFILING)
    thd->profiling->start_new_query();
    thd->profiling->set_query_source(thd->query().str, thd->query().length);
#endif

    thd->set_time();
    Parser_state parser_state;
    if (parser_state.init(thd, thd->query().str, thd->query().length)) {
      /* purecov: begin inspected */
      thd->send_statement_status();
      error = true;
      break;
      /* purecov: end */
    }

    // Ignore ER_TOO_LONG_KEY for system tables.
    thd->push_internal_handler(&error_handler);
    mysql_parse(thd, &parser_state);
    thd->pop_internal_handler();

    error = thd->is_error();
    thd->send_statement_status();

#if defined(ENABLED_PROFILING)
    thd->profiling->finish_current_query();
#endif

    if (error) {
      /* FIXME: need to better report errors to log. */
      my_printf_error(ER_UNKNOWN_ERROR, "BOOTSTRAP ERROR, query %s\n", MYF(0),
                      query_copy);
      /* Abort the --init-file script execution */
      break;
    }

    free_root(thd->mem_root, MYF(MY_KEEP_PREALLOC));
    thd->get_transaction()->free_memory(MYF(MY_KEEP_PREALLOC));

    /*
      Make sure bootstrap statements do not change binlog options.
      Currently enforced for compiled in statements.
    */
    DBUG_ASSERT(
        !enforce_invariants ||
        (saved_option_bits == (thd->variables.option_bits & invariant_bits)));

    DBUG_ASSERT(!enforce_invariants ||
                (saved_sql_log_bin == thd->variables.sql_log_bin));
  }

  it->end();

  return (error ? 1 : 0);
}

/**
  Execute commands from bootstrap_file.

  Used when creating the initial grant tables.
*/

extern "C" {
static void *handle_bootstrap(void *arg) {
  handle_bootstrap_args *args;
  args = reinterpret_cast<handle_bootstrap_args *>(arg);
  THD *thd = args->m_thd;

  mysql_thread_set_psi_id(thd->thread_id());

  /* The following must be called before DBUG_TRACE */
  thd->thread_stack = (char *)&thd;
  if (my_thread_init()) {
    close_connection(thd, ER_OUT_OF_RESOURCES);
    args->m_bootstrap_error = true;
    thd->get_protocol_classic()->end_net();
    thd->release_resources();
  } else {
    thd->store_globals();
    Global_THD_manager *thd_manager = Global_THD_manager::get_instance();
    thd_manager->add_thd(thd);

    // Set tx_read_only to false to allow installing DD tables even
    // if the server is started with --transaction-read-only=true.
    thd->variables.transaction_read_only = false;
    thd->tx_read_only = false;

    // Disable binlogging since super needs to initialize the server
    if (enable_super_log_bin_read_only) {
      thd->variables.sql_log_bin = false;
      thd->variables.option_bits &= ~OPTION_BIN_LOG;
    }

    bootstrap_functor handler = args->m_bootstrap_handler;
    if (handler) {
      args->m_bootstrap_error = (*handler)(thd);
    } else {
      args->m_bootstrap_error = handle_bootstrap_impl(args);
    }

    thd->get_protocol_classic()->end_net();
    thd->release_resources();
    thd_manager->remove_thd(thd);
  }
  my_thread_end();
  return nullptr;
}
}  // extern "C"

/**
  Create a thread to execute all commands from the submitted file.
  By providing an explicit bootstrap handler functor, the default
  behavior of reading and executing SQL commands from the submitted
  file may be customized.

  @param file_name    Name of the init file with SQL statements, if non-null
  @param file         Handle of the init file with SQL statements, if non-null
  @param boot_handler Optional functor for customized handling
  @param thread_type  Bootstrap thread type, server initialize or init file

  @return             False if no errors
*/
bool run_bootstrap_thread(const char *file_name, MYSQL_FILE *file,
                          bootstrap_functor boot_handler,
                          enum_thread_type thread_type) {
  DBUG_TRACE;

  THD *thd = new THD;
  thd->system_thread = thread_type;
  thd->get_protocol_classic()->init_net(nullptr);
  // Skip grants and set the system_user flag in THD.
  thd->security_context()->skip_grants();

  thd->set_new_thread_id();

  handle_bootstrap_args args;

  args.m_thd = thd;
  args.m_bootstrap_handler = boot_handler;
  args.m_file_name = file_name;
  args.m_file = file;

  // Set server default sql_mode irrespective of mysqld server command line
  // argument.
  thd->variables.sql_mode = intern_find_sys_var("sql_mode", 0)->get_default();

  // Set session server and connection collation irrespective of
  // mysqld server command line argument.
  thd->variables.collation_server =
      get_charset_by_name(MYSQL_DEFAULT_COLLATION_NAME, MYF(0));
  thd->variables.collation_connection =
      get_charset_by_name(MYSQL_DEFAULT_COLLATION_NAME, MYF(0));

  // Set session transaction completion type to server default to
  // avoid problems due to transactions being active when they are
  // not supposed to.
  thd->variables.completion_type =
      intern_find_sys_var("completion_type", 0)->get_default();

  /*
    Set default value for explicit_defaults_for_timestamp variable. Bootstrap
    thread creates dictionary tables. The creation of dictionary tables should
    be independent of the value of explicit_defaults_for_timestamp specified by
    the user.
  */
  thd->variables.explicit_defaults_for_timestamp =
      intern_find_sys_var("explicit_defaults_for_timestamp", 0)->get_default();

  /*
    The global table encryption default setting applies to user threads.
    Setting it false for system threads.
  */
  thd->variables.default_table_encryption = false;

  my_thread_attr_t thr_attr;
  my_thread_attr_init(&thr_attr);
#ifndef _WIN32
  pthread_attr_setscope(&thr_attr, PTHREAD_SCOPE_SYSTEM);
#endif
  my_thread_attr_setdetachstate(&thr_attr, MY_THREAD_CREATE_JOINABLE);

  // Default stack size may be too small.
  size_t stacksize = 0;
  my_thread_attr_getstacksize(&thr_attr, &stacksize);
  if (stacksize < my_thread_stack_size) {
    if (0 != my_thread_attr_setstacksize(&thr_attr, my_thread_stack_size)) {
      DBUG_ASSERT(false);
    }
  }

  my_thread_handle thread_handle;
  // What about setting THD::real_id?
  int error = mysql_thread_create(key_thread_bootstrap, &thread_handle,
                                  &thr_attr, handle_bootstrap, &args);
  if (error) {
    /* purecov: begin inspected */
    LogErr(WARNING_LEVEL, ER_BOOTSTRAP_CANT_THREAD, errno).os_errno(errno);
    thd->release_resources();
    delete thd;
    return true;
    /* purecov: end */
  }
  /* Wait for thread to die */
  my_thread_join(&thread_handle, nullptr);
  // Free Items that were created during this execution.
  thd->free_items();
  delete thd;
  return args.m_bootstrap_error;
}
}  // namespace bootstrap
