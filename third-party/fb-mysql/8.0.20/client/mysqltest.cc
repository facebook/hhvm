// Copyright (c) 2000, 2020, Oracle and/or its affiliates. All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License, version 2.0,
// as published by the Free Software Foundation.
//
// This program is also distributed with certain software (including
// but not limited to OpenSSL) that is licensed under separate terms,
// as designated in a particular file or component or in included license
// documentation.  The authors of MySQL hereby grant you an additional
// permission to link the program and your derivative works with the
// separately licensed software that they have included with MySQL.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License, version 2.0, for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA.

/// @file
///
/// mysqltest client - Tool used for executing a .test file.
///
/// See @ref PAGE_MYSQL_TEST_RUN "The MySQL Test Framework" for more
/// information.

#include "client/mysqltest/error_names.h"
#include "client/mysqltest/expected_errors.h"
#include "client/mysqltest/expected_warnings.h"
#include "client/mysqltest/logfile.h"
#include "client/mysqltest/regular_expressions.h"
#include "client/mysqltest/secondary_engine.h"
#include "client/mysqltest/utils.h"
#include "compression.h"

#include <algorithm>
#include <chrono>
#include <cmath>  // std::isinf
#include <limits>
#include <new>
#include <sstream>
#ifdef _WIN32
#include <thread>  // std::thread
#endif

#include <assert.h>
#if defined MY_MSCRT_DEBUG || defined _WIN32
#include <crtdbg.h>
#endif
#ifdef _WIN32
#include <direct.h>
#endif
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <mysql_async.h>
#include <mysql_version.h>
#include <mysqld_error.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#ifndef _WIN32
#include <poll.h>
#include <sys/time.h>
#include <sys/wait.h>
#endif
#ifdef _WIN32
#include <windows.h>
#endif

#include "caching_sha2_passwordopt-vars.h"
#include "client/client_priv.h"
#include "m_ctype.h"
#include "map_helpers.h"
#include "mf_wcomp.h"  // wild_compare
#include "my_byteorder.h"
#include "my_compiler.h"
#include "my_config.h"
#include "my_dbug.h"
#include "my_default.h"
#include "my_dir.h"
#include "my_inttypes.h"
#include "my_macros.h"
#include "my_pointer_arithmetic.h"
#include "my_stacktrace.h"
#include "my_systime.h"  // my_sleep()
#include "my_thread_local.h"
#include "prealloced_array.h"
#include "print_version.h"
#include "sql_common.h"
#include "template_utils.h"
#include "typelib.h"
#include "violite.h"
#include "welcome_copyright_notice.h"  // ORACLE_WELCOME_COPYRIGHT_NOTICE

#ifdef _WIN32
#define SIGNAL_FMT "exception 0x%x"
#else
#define SIGNAL_FMT "signal %d"
#endif

#ifdef _WIN32
#define setenv(a, b, c) _putenv_s(a, b)
#define popen _popen
#define pclose _pclose
#endif

#define MAX_VAR_NAME_LENGTH 256
#define MAX_COLUMNS 256
#define MAX_DELIMITER_LENGTH 16
#define DEFAULT_MAX_CONN 128
#define REPLACE_ROUND_MAX 16

/* Flags controlling send and reap */
#define QUERY_SEND_FLAG 1
#define QUERY_REAP_FLAG 2

#define APPEND_TYPE(type)                                                 \
  {                                                                       \
    dynstr_append(ds, "-- ");                                             \
    switch (type) {                                                       \
      case SESSION_TRACK_SYSTEM_VARIABLES:                                \
        dynstr_append(ds, "Tracker : SESSION_TRACK_SYSTEM_VARIABLES\n");  \
        break;                                                            \
      case SESSION_TRACK_SCHEMA:                                          \
        dynstr_append(ds, "Tracker : SESSION_TRACK_SCHEMA\n");            \
        break;                                                            \
      case SESSION_TRACK_STATE_CHANGE:                                    \
        dynstr_append(ds, "Tracker : SESSION_TRACK_STATE_CHANGE\n");      \
        break;                                                            \
      case SESSION_TRACK_GTIDS:                                           \
        dynstr_append(ds, "Tracker : SESSION_TRACK_GTIDS\n");             \
        break;                                                            \
      case SESSION_TRACK_TRANSACTION_CHARACTERISTICS:                     \
        dynstr_append(                                                    \
            ds, "Tracker : SESSION_TRACK_TRANSACTION_CHARACTERISTICS\n"); \
        break;                                                            \
      case SESSION_TRACK_TRANSACTION_STATE:                               \
        dynstr_append(ds, "Tracker : SESSION_TRACK_TRANSACTION_STATE\n"); \
        break;                                                            \
      case SESSION_TRACK_RESP_ATTR:                                       \
        dynstr_append(ds, "Tracker : SESSION_TRACK_RESP_ATTR\n");         \
        break;                                                            \
      default:                                                            \
        dynstr_append(ds, "\n");                                          \
    }                                                                     \
  }

extern CHARSET_INFO my_charset_utf16le_bin;

// List of error codes specified with 'error' command.
Expected_errors *expected_errors = new Expected_errors();

// List of warnings disabled with 'disable_warnings' command.
Expected_warnings *disabled_warnings = new Expected_warnings();

// List of warnings enabled with 'enable_warnings' command.
Expected_warnings *enabled_warnings = new Expected_warnings();

enum {
  OPT_COLORED_DIFF = OPT_MAX_CLIENT_OPTION,
  OPT_CURSOR_PROTOCOL,
  OPT_EXPLAIN_PROTOCOL,
  OPT_JSON_EXPLAIN_PROTOCOL,
  OPT_LOG_DIR,
  OPT_MARK_PROGRESS,
  OPT_MAX_CONNECT_RETRIES,
  OPT_MAX_CONNECTIONS,
  OPT_NO_SKIP,
  OPT_OFFLOAD_COUNT_FILE,
  OPT_PS_PROTOCOL,
  OPT_RESULT_FORMAT_VERSION,
#ifdef _WIN32
  OPT_SAFEPROCESS_PID,
#endif
  OPT_SP_PROTOCOL,
  OPT_TAIL_LINES,
  OPT_TRACE_EXEC,
  OPT_TRACE_PROTOCOL,
  OPT_VIEW_PROTOCOL,
};

static int record = 0, opt_sleep = -1;
static char *opt_db = nullptr, *opt_pass = nullptr;
const char *opt_user = nullptr, *opt_host = nullptr, *unix_sock = nullptr,
           *opt_basedir = "./";
const char *excluded_string = nullptr;
static char *shared_memory_base_name = nullptr;
const char *opt_logdir = "";
const char *opt_include = nullptr, *opt_charsets_dir;
static int opt_port = 0;
static int opt_max_connect_retries;
static int opt_result_format_version;
static int opt_max_connections = DEFAULT_MAX_CONN;
static bool opt_colored_diff = false;
static bool opt_compress = false, silent = false, verbose = false,
            trace_exec = false;
static bool debug_info_flag = false, debug_check_flag = false;
static bool tty_password = false;
static bool opt_mark_progress = false;
static bool ps_protocol = false, ps_protocol_enabled = false;
static bool sp_protocol = false, sp_protocol_enabled = false;
static bool no_skip = false;
static bool view_protocol = false, view_protocol_enabled = false;
static bool opt_trace_protocol = false, opt_trace_protocol_enabled = false;
static bool explain_protocol = false, explain_protocol_enabled = false;
static bool json_explain_protocol = false,
            json_explain_protocol_enabled = false;
static bool cursor_protocol = false, cursor_protocol_enabled = false;
static bool testcase_disabled = false;
static bool display_result_vertically = false, display_result_lower = false,
            display_metadata = false, display_result_sorted = false,
            display_session_track_info = false;
static int start_sort_column = 0;
static bool disable_query_log = false, disable_result_log = false;
static bool disable_connect_log = true;
static bool disable_warnings = false;
static bool disable_info = true;
static bool abort_on_error = true;
static bool server_initialized = false;
static bool is_windows = false;
static MEM_ROOT argv_alloc{PSI_NOT_INSTRUMENTED, 512};
static const char *load_default_groups[] = {"mysqltest", "client", nullptr};
static char line_buffer[MAX_DELIMITER_LENGTH], *line_buffer_pos = line_buffer;
static bool can_handle_expired_passwords = true;

/*
  These variables control the behavior of the asynchronous operations for
  mysqltest client. If --async-client is specified, use_async_client is true.
  Each command checks enable_async_client (which can be forced off or
  disabled for a single command) to decide the mode it uses to run.
*/
static bool use_async_client = false;
static bool enable_async_client = false;

// To enable sending CLIENT_INTERACTIVE in client flags
static bool enable_client_interactive = false;

// Secondary engine options
static const char *opt_offload_count_file;

static Secondary_engine *secondary_engine = nullptr;

static uint opt_zstd_compress_level = default_zstd_compression_level;
static char *opt_compress_algorithm = nullptr;

#ifdef _WIN32
static DWORD opt_safe_process_pid;
static HANDLE mysqltest_thread;
// Event handle for stacktrace request event
static HANDLE stacktrace_request_event = NULL;
static std::thread wait_for_stacktrace_request_event_thread;
#endif

Logfile log_file;

/// Info on properties that can be set with '--disable_X' and
/// '--disable_X' commands.
struct Property {
  bool *var;             // Actual variable
  bool set;              // Has been set for ONCE command
  bool old;              // If set, thus is the old value
  bool reverse;          // Variable is true if disabled
  const char *env_name;  // Environment variable name
};

static struct Property prop_list[] = {
    {&abort_on_error, false, true, false, "$ENABLE_ABORT_ON_ERROR"},
    {&disable_connect_log, false, true, true, "$ENABLE_CONNECT_LOG"},
    {&disable_info, false, true, true, "$ENABLE_INFO"},
    {&display_session_track_info, false, true, true,
     "$ENABLE_STATE_CHANGE_INFO"},
    {&display_metadata, false, false, false, "$ENABLE_METADATA"},
    {&ps_protocol_enabled, false, false, false, "$ENABLE_PS_PROTOCOL"},
    {&disable_query_log, false, false, true, "$ENABLE_QUERY_LOG"},
    {&disable_result_log, false, false, true, "$ENABLE_RESULT_LOG"},
    {&disable_warnings, false, false, true, "$ENABLE_WARNINGS"},
    {&enable_async_client, false, false, false, "$ENABLE_ASYNC_CLIENT"}};

static bool once_property = false;

enum enum_prop {
  P_ABORT = 0,
  P_CONNECT,
  P_INFO,
  P_SESSION_TRACK,
  P_META,
  P_PS,
  P_QUERY,
  P_RESULT,
  P_WARN,
  P_ASYNC,
  P_MAX
};

static uint start_lineno = 0; /* Start line of current command */
static uint my_end_arg = 0;

/* Number of lines of the result to include in failure report */
static uint opt_tail_lines = 0;

static uint opt_connect_timeout = 0;

static char delimiter[MAX_DELIMITER_LENGTH] = ";";
static size_t delimiter_length = 1;

static char TMPDIR[FN_REFLEN];

/* Block stack */
enum block_cmd { cmd_none, cmd_if, cmd_while };

struct st_block {
  int line;                         /* Start line of block */
  bool ok;                          /* Should block be executed */
  enum block_cmd cmd;               /* Command owning the block */
  char delim[MAX_DELIMITER_LENGTH]; /* Delimiter before block */
};

static struct st_block block_stack[32];
static struct st_block *cur_block, *block_stack_end;

/* Open file stack */
struct st_test_file {
  FILE *file;
  char *file_name;
  uint lineno; /* Current line in file */
};

static struct st_test_file file_stack[16];
static struct st_test_file *cur_file;
static struct st_test_file *file_stack_end;

static const char *default_charset = MYSQL_DEFAULT_CHARSET_NAME;
CHARSET_INFO *charset_info =
    &my_charset_utf8mb4_0900_ai_ci; /* Default charset */

/*
  Timer related variables
  See the timer_output() definition for details
*/
static char *timer_file = nullptr;
static ulonglong timer_start;
static void timer_output(void);
static ulonglong timer_now(void);

static ulong connection_retry_sleep = 100000; /* Microseconds */

static char *opt_plugin_dir = nullptr;

/* To retrieve a filename from a filepath */
const char *get_filename_from_path(const char *path) {
  const char *fname = nullptr;
  if (is_windows)
    fname = strrchr(path, '\\');
  else
    fname = strrchr(path, '/');
  if (fname == nullptr)
    return path;
  else
    return ++fname;
}

static uint opt_protocol = 0;

#if defined(_WIN32)
static uint opt_protocol_for_default_connection = MYSQL_PROTOCOL_PIPE;
#endif

struct st_command;
typedef Prealloced_array<st_command *, 1024> Q_lines;
Q_lines *q_lines;

#include "sslopt-vars.h"

struct Parser {
  int read_lines, current_line;
} parser;

struct MasterPos {
  char file[FN_REFLEN];
  ulong pos;
} master_pos;

/* if set, all results are concated and compared against this file */
const char *result_file_name = nullptr;

typedef struct {
  char *name;
  size_t name_len;
  char *str_val;
  size_t str_val_len;
  int int_val;
  size_t alloced_len;
  bool int_dirty; /* do not update string if int is updated until first read */
  bool is_int;
  bool alloced;
} VAR;

/*Perl/shell-like variable registers */
VAR var_reg[10];

struct var_free {
  void operator()(VAR *var) const;
};

collation_unordered_map<std::string, std::unique_ptr<VAR, var_free>> *var_hash;

struct st_connection {
  MYSQL mysql;
  /* Used when creating views and sp, to avoid implicit commit */
  MYSQL *util_mysql;
  char *name;
  size_t name_len;
  MYSQL_STMT *stmt;
  /* Set after send to disallow other queries before reap */
  bool pending;
};

struct st_connection *connections = nullptr;
struct st_connection *cur_con = nullptr, *next_con, *connections_end;

/*
  List of commands in mysqltest
  Must match the "command_names" array
  Add new commands before Q_UNKNOWN!
*/
enum enum_commands {
  Q_CONNECTION = 1,
  Q_QUERY,
  Q_CONNECT,
  Q_SLEEP,
  Q_REAL_SLEEP,
  Q_INC,
  Q_DEC,
  Q_SOURCE,
  Q_DISCONNECT,
  Q_LET,
  Q_ECHO,
  Q_EXPR,
  Q_WHILE,
  Q_END_BLOCK,
  Q_SAVE_MASTER_POS,
  Q_SYNC_WITH_MASTER,
  Q_SYNC_SLAVE_WITH_MASTER,
  Q_ERROR,
  Q_SEND,
  Q_REAP,
  Q_DIRTY_CLOSE,
  Q_REPLACE,
  Q_REPLACE_COLUMN,
  Q_PING,
  Q_EVAL,
  Q_ENABLE_QUERY_LOG,
  Q_DISABLE_QUERY_LOG,
  Q_ENABLE_RESULT_LOG,
  Q_DISABLE_RESULT_LOG,
  Q_ENABLE_CONNECT_LOG,
  Q_DISABLE_CONNECT_LOG,
  Q_WAIT_FOR_SLAVE_TO_STOP,
  Q_ENABLE_WARNINGS,
  Q_DISABLE_WARNINGS,
  Q_ENABLE_INFO,
  Q_DISABLE_INFO,
  Q_ENABLE_SESSION_TRACK_INFO,
  Q_DISABLE_SESSION_TRACK_INFO,
  Q_ENABLE_METADATA,
  Q_DISABLE_METADATA,
  Q_ENABLE_ASYNC_CLIENT,
  Q_DISABLE_ASYNC_CLIENT,
  Q_EXEC,
  Q_EXECW,
  Q_EXEC_BACKGROUND,
  Q_DELIMITER,
  Q_DISABLE_ABORT_ON_ERROR,
  Q_ENABLE_ABORT_ON_ERROR,
  Q_DISPLAY_VERTICAL_RESULTS,
  Q_DISPLAY_HORIZONTAL_RESULTS,
  Q_QUERY_VERTICAL,
  Q_QUERY_HORIZONTAL,
  Q_SORTED_RESULT,
  Q_PARTIALLY_SORTED_RESULT,
  Q_LOWERCASE,
  Q_START_TIMER,
  Q_END_TIMER,
  Q_CHARACTER_SET,
  Q_DISABLE_PS_PROTOCOL,
  Q_ENABLE_PS_PROTOCOL,
  Q_DISABLE_RECONNECT,
  Q_ENABLE_RECONNECT,
  Q_IF,
  Q_DISABLE_TESTCASE,
  Q_ENABLE_TESTCASE,
  Q_REPLACE_REGEX,
  Q_REPLACE_NUMERIC_ROUND,
  Q_REMOVE_FILE,
  Q_FILE_EXIST,
  Q_WRITE_FILE,
  Q_COPY_FILE,
  Q_PERL,
  Q_DIE,
  Q_EXIT,
  Q_SKIP,
  Q_CHMOD_FILE,
  Q_APPEND_FILE,
  Q_CAT_FILE,
  Q_DIFF_FILES,
  Q_SEND_QUIT,
  Q_CHANGE_USER,
  Q_MKDIR,
  Q_RMDIR,
  Q_FORCE_RMDIR,
  Q_FORCE_CPDIR,
  Q_LIST_FILES,
  Q_LIST_FILES_WRITE_FILE,
  Q_LIST_FILES_APPEND_FILE,
  Q_SEND_SHUTDOWN,
  Q_SHUTDOWN_SERVER,
  Q_RESULT_FORMAT_VERSION,
  Q_MOVE_FILE,
  Q_REMOVE_FILES_WILDCARD,
  Q_COPY_FILES_WILDCARD,
  Q_SEND_EVAL,
  Q_OUTPUT, /* redirect output to a file */
  Q_RESET_CONNECTION,
  Q_QUERY_ATTRS_ADD,
  Q_QUERY_ATTRS_DELETE,
  Q_QUERY_ATTRS_RESET,
  Q_CONN_ATTRS_ADD,
  Q_CONN_ATTRS_DELETE,
  Q_CONN_ATTRS_RESET,
  Q_ENABLE_CLIENT_INTERACTIVE,
  Q_DUMP_TIMED_OUT_CONNECTION_SOCKET_BUFFER,
  Q_UNKNOWN, /* Unknown command.   */
  Q_COMMENT, /* Comments, ignored. */
  Q_COMMENT_WITH_COMMAND,
  Q_EMPTY_LINE
};

const char *command_names[] = {
    "connection", "query", "connect", "sleep", "real_sleep", "inc", "dec",
    "source", "disconnect", "let", "echo", "expr", "while", "end",
    "save_master_pos", "sync_with_master", "sync_slave_with_master", "error",
    "send", "reap", "dirty_close", "replace_result", "replace_column", "ping",
    "eval",
    /* Enable/disable that the _query_ is logged to result file */
    "enable_query_log", "disable_query_log",
    /* Enable/disable that the _result_ from a query is logged to result file */
    "enable_result_log", "disable_result_log", "enable_connect_log",
    "disable_connect_log", "wait_for_slave_to_stop", "enable_warnings",
    "disable_warnings", "enable_info", "disable_info",
    "enable_session_track_info", "disable_session_track_info",
    "enable_metadata", "disable_metadata", "enable_async_client",
    "disable_async_client", "exec", "execw", "exec_in_background", "delimiter",
    "disable_abort_on_error", "enable_abort_on_error", "vertical_results",
    "horizontal_results", "query_vertical", "query_horizontal", "sorted_result",
    "partially_sorted_result", "lowercase_result", "start_timer", "end_timer",
    "character_set", "disable_ps_protocol", "enable_ps_protocol",
    "disable_reconnect", "enable_reconnect", "if", "disable_testcase",
    "enable_testcase", "replace_regex", "replace_numeric_round", "remove_file",
    "file_exists", "write_file", "copy_file", "perl", "die",

    /* Don't execute any more commands, compare result */
    "exit", "skip", "chmod", "append_file", "cat_file", "diff_files",
    "send_quit", "change_user", "mkdir", "rmdir", "force-rmdir", "force-cpdir",
    "list_files", "list_files_write_file", "list_files_append_file",
    "send_shutdown", "shutdown_server", "result_format", "move_file",
    "remove_files_wildcard", "copy_files_wildcard", "send_eval", "output",
    "reset_connection", "query_attrs_add", "query_attrs_delete",
    "query_attrs_reset", "conn_attrs_add", "conn_attrs_delete",
    "conn_attrs_reset", "enable_client_interactive",
    "dump_timed_out_connection_socket_buffer",

    nullptr};

struct st_command {
  char *query, *query_buf, *first_argument, *last_argument, *end;
  DYNAMIC_STRING content;
  size_t first_word_len, query_len;
  bool abort_on_error, used_replace;
  char output_file[FN_REFLEN];
  enum enum_commands type;
  // Line number of the command
  uint lineno;
};

TYPELIB command_typelib = {array_elements(command_names), "", command_names,
                           nullptr};

DYNAMIC_STRING ds_res;
DYNAMIC_STRING ds_result;
/* Points to ds_warning in run_query, so it can be freed */
DYNAMIC_STRING *ds_warn = nullptr;
struct st_command *curr_command = nullptr;

char builtin_echo[FN_REFLEN];

/* Stores regex substitutions */

struct st_replace_regex *glob_replace_regex = nullptr;

struct REPLACE;
REPLACE *glob_replace = nullptr;
void replace_strings_append(REPLACE *rep, DYNAMIC_STRING *ds, const char *from,
                            size_t len);

static void cleanup_and_exit(int exit_code) MY_ATTRIBUTE((noreturn));

void die(const char *fmt, ...) MY_ATTRIBUTE((format(printf, 1, 2)))
    MY_ATTRIBUTE((noreturn));
void abort_not_supported_test(const char *fmt, ...)
    MY_ATTRIBUTE((format(printf, 1, 2))) MY_ATTRIBUTE((noreturn));
void verbose_msg(const char *fmt, ...) MY_ATTRIBUTE((format(printf, 1, 2)));
void log_msg(const char *fmt, ...) MY_ATTRIBUTE((format(printf, 1, 2)));

VAR *var_from_env(const char *, const char *);
VAR *var_init(VAR *v, const char *name, size_t name_len, const char *val,
              size_t val_len);
VAR *var_get(const char *var_name, const char **var_name_end, bool raw,
             bool ignore_not_existing);
void eval_expr(VAR *v, const char *p, const char **p_end, bool open_end = false,
               bool do_eval = true);
bool match_delimiter(int c, const char *delim, size_t length);

void do_eval(DYNAMIC_STRING *query_eval, const char *query,
             const char *query_end, bool pass_through_escape_chars);
void str_to_file(const char *fname, char *str, size_t size);
void str_to_file2(const char *fname, char *str, size_t size, bool append);

void fix_win_paths(const char *val, size_t len);

#ifdef _WIN32
void free_win_path_patterns();
#endif

/* For replace_column */
static char *replace_column[MAX_COLUMNS];
static uint max_replace_column = 0;
void do_get_replace_column(struct st_command *);
void free_replace_column();

/* For replace */
void do_get_replace(struct st_command *command);
void free_replace();

/* For replace_regex */
void do_get_replace_regex(struct st_command *command);
void free_replace_regex();

/* For replace numeric round */
static int glob_replace_numeric_round = -1;
void do_get_replace_numeric_round(struct st_command *command);
void free_replace_numeric_round();
void replace_numeric_round_append(int round, DYNAMIC_STRING *ds,
                                  const char *from, size_t len);

/* Used by sleep */
void check_eol_junk_line(const char *eol);

static void var_set(const char *var_name, const char *var_name_end,
                    const char *var_val, const char *var_val_end);

static void free_all_replace() {
  free_replace();
  free_replace_regex();
  free_replace_column();
  free_replace_numeric_round();
}

/*
  To run tests via the async API, invoke mysqltest with --async-client.
*/
class AsyncTimer {
 public:
  explicit AsyncTimer(std::string label)
      : label_(label), time_(std::chrono::system_clock::now()), start_(time_) {}

  ~AsyncTimer() {
    auto now = std::chrono::system_clock::now();
    auto delta = now - start_;
    ulonglong MY_ATTRIBUTE((unused)) micros =
        std::chrono::duration_cast<std::chrono::microseconds>(delta).count();
    DBUG_PRINT("async_timing",
               ("%s total micros: %llu", label_.c_str(), micros));
  }

  void check() {
    auto now = std::chrono::system_clock::now();
    auto delta = now - time_;
    time_ = now;
    ulonglong MY_ATTRIBUTE((unused)) micros =
        std::chrono::duration_cast<std::chrono::microseconds>(delta).count();
    DBUG_PRINT("async_timing", ("%s op micros: %llu", label_.c_str(), micros));
  }

 private:
  std::string label_;
  std::chrono::system_clock::time_point time_;
  std::chrono::system_clock::time_point start_;
};

#ifdef _WIN32
/*
  Check if any data is available in the socket to be read or written.
*/
static int socket_event_listen(net_async_block_state async_blocking_state,
                               my_socket fd) {
  int result;
  fd_set readfds, writefds, exceptfds;

  FD_ZERO(&readfds);
  FD_ZERO(&writefds);
  FD_ZERO(&exceptfds);

  FD_SET(fd, &exceptfds);

  switch (async_blocking_state) {
    case NET_NONBLOCKING_READ:
      FD_SET(fd, &readfds);
      break;
    case NET_NONBLOCKING_WRITE:
    case NET_NONBLOCKING_CONNECT:
      FD_SET(fd, &writefds);
      break;
    default:
      DBUG_ASSERT(false);
  }
  result = select((int)(fd + 1), &readfds, &writefds, &exceptfds, NULL);
  if (result < 0) {
    perror("select");
  }
  return result;
}
#else
static int socket_event_listen(net_async_block_state, my_socket fd) {
  int result;
  pollfd pfd;
  pfd.fd = fd;
  /*
    Listen to both in/out because SSL can perform reads during writes (and
    vice versa).
  */
  pfd.events = POLLIN | POLLOUT;
  result = poll(&pfd, 1, -1);
  if (result < 0) {
    perror("poll");
  }
  return result;
}
#endif

static int async_mysql_change_user_wrapper(MYSQL *mysql, const char *user,
                                           const char *passwd, const char *db) {
  net_async_status status;
  AsyncTimer t(__func__);
  while ((status = mysql_change_user_nonblocking(mysql, user, passwd, db)) ==
         NET_ASYNC_NOT_READY) {
    t.check();
    NET_ASYNC *net_async = NET_ASYNC_DATA(&(mysql->net));
    int result = socket_event_listen(net_async->async_blocking_state,
                                     mysql_get_socket_descriptor(mysql));
    if (result == -1) return 1;
  }
  if (status == NET_ASYNC_ERROR) {
    return 1;
  }
  return 0;
}

/*
  Below async_mysql_*_wrapper functions are used to measure how much time
  each nonblocking call spends before completing the operations.
i*/
static MYSQL_ROW async_mysql_fetch_row_wrapper(MYSQL_RES *res) {
  MYSQL_ROW row;
  MYSQL *mysql = res->handle;
  AsyncTimer t(__func__);
  while (mysql_fetch_row_nonblocking(res, &row) == NET_ASYNC_NOT_READY) {
    t.check();
    NET_ASYNC *net_async = NET_ASYNC_DATA((&(mysql->net)));
    int result = socket_event_listen(net_async->async_blocking_state,
                                     mysql_get_socket_descriptor(mysql));
    if (result == -1) return nullptr;
  }
  return row;
}

static MYSQL_RES *async_mysql_store_result_wrapper(MYSQL *mysql) {
  MYSQL_RES *mysql_result;
  AsyncTimer t(__func__);
  while (mysql_store_result_nonblocking(mysql, &mysql_result) ==
         NET_ASYNC_NOT_READY) {
    t.check();
    NET_ASYNC *net_async = NET_ASYNC_DATA(&(mysql->net));
    int result = socket_event_listen(net_async->async_blocking_state,
                                     mysql_get_socket_descriptor(mysql));
    if (result == -1) return nullptr;
  }
  return mysql_result;
}

static int async_mysql_real_query_wrapper(MYSQL *mysql, const char *query,
                                          ulong length) {
  net_async_status status;
  AsyncTimer t(__func__);
  while ((status = mysql_real_query_nonblocking(mysql, query, length)) ==
         NET_ASYNC_NOT_READY) {
    t.check();
    NET_ASYNC *net_async = NET_ASYNC_DATA(&(mysql->net));
    int result = socket_event_listen(net_async->async_blocking_state,
                                     mysql_get_socket_descriptor(mysql));
    if (result == -1) return 1;
  }
  if (status == NET_ASYNC_ERROR) {
    return 1;
  }
  return 0;
}

static int async_mysql_send_query_wrapper(MYSQL *mysql, const char *query,
                                          ulong length) {
  net_async_status status;
  ASYNC_DATA(mysql)->async_query_length = length;
  ASYNC_DATA(mysql)->async_query_state = QUERY_SENDING;

  AsyncTimer t(__func__);
  while ((status = mysql_send_query_nonblocking(mysql, query, length)) ==
         NET_ASYNC_NOT_READY) {
    t.check();
    NET_ASYNC *net_async = NET_ASYNC_DATA(&(mysql->net));
    int result = socket_event_listen(net_async->async_blocking_state,
                                     mysql_get_socket_descriptor(mysql));
    if (result == -1) return 1;
  }
  if (status == NET_ASYNC_ERROR) {
    return 1;
  }
  return 0;
}

static bool async_mysql_read_query_result_wrapper(MYSQL *mysql) {
  net_async_status status;
  AsyncTimer t(__func__);
  while ((status = (*mysql->methods->read_query_result_nonblocking)(mysql)) ==
         NET_ASYNC_NOT_READY) {
    t.check();
    NET_ASYNC *net_async = NET_ASYNC_DATA(&(mysql->net));
    int result = socket_event_listen(net_async->async_blocking_state,
                                     mysql_get_socket_descriptor(mysql));
    if (result == -1) return true;
  }
  if (status == NET_ASYNC_ERROR) {
    return true;
  }
  return false;
}

static int async_mysql_next_result_wrapper(MYSQL *mysql) {
  net_async_status status;
  AsyncTimer t(__func__);
  while ((status = mysql_next_result_nonblocking(mysql)) ==
         NET_ASYNC_NOT_READY) {
    t.check();
    NET_ASYNC *net_async = NET_ASYNC_DATA(&(mysql->net));
    int result = socket_event_listen(net_async->async_blocking_state,
                                     mysql_get_socket_descriptor(mysql));
    if (result == -1) return 1;
  }
  if (status == NET_ASYNC_ERROR)
    return 1;
  else if (status == NET_ASYNC_COMPLETE_NO_MORE_RESULTS)
    return -1;
  else
    return 0;
}

static MYSQL *async_mysql_real_connect_wrapper(
    MYSQL *mysql, const char *host, const char *user, const char *passwd,
    const char *db, uint port, const char *unix_socket, ulong client_flag) {
  net_async_status status;
  AsyncTimer t(__func__);

  while ((status = mysql_real_connect_nonblocking(
              mysql, host, user, passwd, db, port, unix_socket, client_flag)) ==
         NET_ASYNC_NOT_READY) {
    t.check();
  }
  if (status == NET_ASYNC_ERROR)
    return nullptr;
  else
    return mysql;
}

static int async_mysql_query_wrapper(MYSQL *mysql, const char *query) {
  net_async_status status;
  AsyncTimer t(__func__);
  while ((status = mysql_real_query_nonblocking(mysql, query, strlen(query))) ==
         NET_ASYNC_NOT_READY) {
    t.check();
    NET_ASYNC *net_async = NET_ASYNC_DATA(&(mysql->net));
    int result = socket_event_listen(net_async->async_blocking_state,
                                     mysql_get_socket_descriptor(mysql));
    if (result == -1) return 1;
  }
  if (status == NET_ASYNC_ERROR) {
    return 1;
  }
  return 0;
}

static void async_mysql_free_result_wrapper(MYSQL_RES *result) {
  AsyncTimer t(__func__);
  while (mysql_free_result_nonblocking(result) == NET_ASYNC_NOT_READY) {
    t.check();
    MYSQL *mysql = result->handle;
    NET_ASYNC *net_async = NET_ASYNC_DATA(&(mysql->net));
    int listen_result = socket_event_listen(net_async->async_blocking_state,
                                            mysql_get_socket_descriptor(mysql));
    if (listen_result == -1) return;
  }
  return;
}

static int async_mysql_reset_connection_wrapper(MYSQL *mysql) {
  net_async_status status;
  AsyncTimer t(__func__);
  while ((status = mysql_reset_connection_nonblocking(mysql)) ==
         NET_ASYNC_NOT_READY) {
    t.check();
    NET_ASYNC *net_async = NET_ASYNC_DATA(&(mysql->net));
    int result = socket_event_listen(net_async->async_blocking_state,
                                     mysql_get_socket_descriptor(mysql));
    if (result == -1) return 1;
  }
  if (status == NET_ASYNC_ERROR) {
    return 1;
  }
  return 0;
}

/*
  Below are the wrapper functions which are defined on top of standard C APIs
  to make a decision on whether to call blocking or non blocking API based on
  --async-client option is set or not.
*/
static MYSQL_ROW mysql_fetch_row_wrapper(MYSQL_RES *res) {
  if (enable_async_client)
    return async_mysql_fetch_row_wrapper(res);
  else
    return mysql_fetch_row(res);
}

static MYSQL_RES *mysql_store_result_wrapper(MYSQL *mysql) {
  if (enable_async_client)
    return async_mysql_store_result_wrapper(mysql);
  else
    return mysql_store_result(mysql);
}

static int mysql_real_query_wrapper(MYSQL *mysql, const char *query,
                                    ulong length) {
  if (enable_async_client)
    return async_mysql_real_query_wrapper(mysql, query, length);
  else
    return mysql_real_query(mysql, query, length);
}

static int mysql_send_query_wrapper(MYSQL *mysql, const char *query,
                                    ulong length) {
  if (enable_async_client)
    return async_mysql_send_query_wrapper(mysql, query, length);
  else
    return mysql_send_query(mysql, query, length);
}

static bool mysql_read_query_result_wrapper(MYSQL *mysql) {
  bool ret;
  if (enable_async_client)
    ret = async_mysql_read_query_result_wrapper(mysql);
  else
    ret = mysql_read_query_result(mysql);
  return ret;
}

static int mysql_query_wrapper(MYSQL *mysql, const char *query) {
  if (enable_async_client)
    return async_mysql_query_wrapper(mysql, query);
  else
    return mysql_query(mysql, query);
}

static int mysql_next_result_wrapper(MYSQL *mysql) {
  if (enable_async_client)
    return async_mysql_next_result_wrapper(mysql);
  else
    return mysql_next_result(mysql);
}

static MYSQL *mysql_real_connect_wrapper(MYSQL *mysql, const char *host,
                                         const char *user, const char *passwd,
                                         const char *db, uint port,
                                         const char *unix_socket,
                                         ulong client_flag) {
  client_flag |= (enable_client_interactive ? CLIENT_INTERACTIVE : 0);
  if (enable_async_client)
    return async_mysql_real_connect_wrapper(mysql, host, user, passwd, db, port,
                                            unix_socket, client_flag);
  else
    return mysql_real_connect(mysql, host, user, passwd, db, port, unix_socket,
                              client_flag);
}

static void mysql_free_result_wrapper(MYSQL_RES *result) {
  if (enable_async_client)
    return async_mysql_free_result_wrapper(result);
  else
    return mysql_free_result(result);
}

static int mysql_reset_connection_wrapper(MYSQL *mysql) {
  if (enable_async_client)
    return async_mysql_reset_connection_wrapper(mysql);
  else
    return mysql_reset_connection(mysql);
}

static int mysql_change_user_wrapper(MYSQL *mysql, const char *user,
                                     const char *passwd, const char *db) {
  if (enable_async_client)
    return async_mysql_change_user_wrapper(mysql, user, passwd, db);
  else
    return mysql_change_user(mysql, user, passwd, db);
}

/* async client test code (end) */

void replace_dynstr_append_mem(DYNAMIC_STRING *ds, const char *val, size_t len);
void replace_dynstr_append(DYNAMIC_STRING *ds, const char *val);
void replace_dynstr_append_uint(DYNAMIC_STRING *ds, uint val);
void dynstr_append_sorted(DYNAMIC_STRING *ds, DYNAMIC_STRING *ds_input,
                          int start_sort_column);

void revert_properties();

void do_eval(DYNAMIC_STRING *query_eval, const char *query,
             const char *query_end, bool pass_through_escape_chars) {
  const char *p;
  char c, next_c;
  int escaped = 0;
  VAR *v;
  DBUG_TRACE;

  for (p = query; (c = *p) && p < query_end; ++p) {
    next_c = *(p + 1);
    switch (c) {
      case '$':
        if (escaped ||
            // a JSON path expression
            next_c == '.' || next_c == '[' || next_c == '\'' || next_c == '"') {
          escaped = 0;
          dynstr_append_mem(query_eval, p, 1);
        } else {
          if (!(v = var_get(p, &p, false, false))) die("Bad variable in eval");
          dynstr_append_mem(query_eval, v->str_val, v->str_val_len);
        }
        break;
      case '\\':
        if (escaped) {
          escaped = 0;
          dynstr_append_mem(query_eval, p, 1);
        } else if (next_c == '\\' || next_c == '$' || next_c == '"') {
          /* Set escaped only if next char is \, " or $ */
          escaped = 1;

          if (pass_through_escape_chars) {
            /* The escape char should be added to the output string. */
            dynstr_append_mem(query_eval, p, 1);
          }
        } else
          dynstr_append_mem(query_eval, p, 1);
        break;
      default:
        escaped = 0;
        dynstr_append_mem(query_eval, p, 1);
        break;
    }
  }
#ifdef _WIN32
  fix_win_paths(query_eval->str, query_eval->length);
#endif
}

/*
  Run query and dump the result to stderr in vertical format

  NOTE! This function should be safe to call when an error
  has occurred and thus any further errors will be ignored(although logged)

  SYNOPSIS
  show_query
  mysql - connection to use
  query - query to run

*/

static void show_query(MYSQL *mysql, const char *query) {
  MYSQL_RES *res;
  DBUG_TRACE;

  if (!mysql) return;

  if (mysql_query_wrapper(mysql, query)) {
    log_msg("Error running query '%s': %d %s", query, mysql_errno(mysql),
            mysql_error(mysql));
    return;
  }

  if ((res = mysql_store_result_wrapper(mysql)) == nullptr) {
    /* No result set returned */
    return;
  }

  {
    MYSQL_ROW row;
    unsigned int i;
    unsigned int row_num = 0;
    unsigned int num_fields = mysql_num_fields(res);
    MYSQL_FIELD *fields = mysql_fetch_fields(res);

    fprintf(stderr, "=== %s ===\n", query);
    while ((row = mysql_fetch_row_wrapper(res))) {
      unsigned long *lengths = mysql_fetch_lengths(res);
      row_num++;

      fprintf(stderr, "---- %d. ----\n", row_num);
      for (i = 0; i < num_fields; i++) {
        /* looks ugly , but put here to convince parfait */
        assert(lengths);
        fprintf(stderr, "%s\t%.*s\n", fields[i].name, (int)lengths[i],
                row[i] ? row[i] : "NULL");
      }
    }
    for (i = 0; i < std::strlen(query) + 8; i++) fprintf(stderr, "=");
    fprintf(stderr, "\n\n");
  }
  mysql_free_result_wrapper(res);
}

/*
  Show any warnings just before the error. Since the last error
  is added to the warning stack, only print @@warning_count-1 warnings.

  NOTE! This function should be safe to call when an error
  has occurred and this any further errors will be ignored(although logged)

  SYNOPSIS
  show_warnings_before_error
  mysql - connection to use

*/

static void show_warnings_before_error(MYSQL *mysql) {
  MYSQL_RES *res;
  const char *query = "SHOW WARNINGS";
  DBUG_TRACE;

  if (!mysql) return;

  if (mysql_query_wrapper(mysql, query)) {
    log_msg("Error running query '%s': %d %s", query, mysql_errno(mysql),
            mysql_error(mysql));
    return;
  }

  if ((res = mysql_store_result_wrapper(mysql)) == nullptr) {
    /* No result set returned */
    return;
  }

  if (mysql_num_rows(res) <= 1) {
    /* Don't display the last row, it's "last error" */
  } else {
    MYSQL_ROW row;
    unsigned int row_num = 0;
    unsigned int num_fields = mysql_num_fields(res);

    fprintf(stderr, "\nWarnings from just before the error:\n");
    while ((row = mysql_fetch_row_wrapper(res))) {
      unsigned int i;
      unsigned long *lengths = mysql_fetch_lengths(res);

      if (++row_num >= mysql_num_rows(res)) {
        /* Don't display the last row, it's "last error" */
        break;
      }

      for (i = 0; i < num_fields; i++) {
        /* looks ugly , but put here to convince parfait */
        assert(lengths);
        fprintf(stderr, "%.*s ", (int)lengths[i], row[i] ? row[i] : "NULL");
      }
      fprintf(stderr, "\n");
    }
  }
  mysql_free_result_wrapper(res);
}

enum arg_type { ARG_STRING, ARG_REST };

struct command_arg {
  const char *argname;     /* Name of argument   */
  enum arg_type type;      /* Type of argument   */
  bool required;           /* Argument required  */
  DYNAMIC_STRING *ds;      /* Storage for argument */
  const char *description; /* Description of the argument */
};

static void check_command_args(struct st_command *command, char *arguments,
                               const struct command_arg *args, int num_args,
                               const char delimiter_arg) {
  int i;
  char *ptr = arguments;
  const char *start;
  DBUG_TRACE;
  DBUG_PRINT("enter", ("num_args: %d", num_args));

  for (i = 0; i < num_args; i++) {
    const struct command_arg *arg = &args[i];
    char delimiter;

    switch (arg->type) {
        /* A string */
      case ARG_STRING:
        /* Skip leading spaces */
        while (*ptr && *ptr == ' ') ptr++;
        start = ptr;
        delimiter = delimiter_arg;
        /* If start of arg is ' ` or " search to matching quote end instead */
        if (*ptr && strchr("'`\"", *ptr)) {
          delimiter = *ptr;
          start = ++ptr;
        }
        /* Find end of arg, terminated by "delimiter" */
        while (*ptr && *ptr != delimiter) ptr++;
        if (ptr > start) {
          init_dynamic_string(arg->ds, nullptr, ptr - start, 32);
          do_eval(arg->ds, start, ptr, false);
        } else {
          /* Empty string */
          init_dynamic_string(arg->ds, "", 0, 0);
        }
        /* Find real end of arg, terminated by "delimiter_arg" */
        /* This will do nothing if arg was not closed by quotes */
        while (*ptr && *ptr != delimiter_arg) ptr++;

        command->last_argument = ptr;

        /* Step past the delimiter */
        if (*ptr && *ptr == delimiter_arg) ptr++;
        DBUG_PRINT("info", ("val: %s", arg->ds->str));
        break;

        /* Rest of line */
      case ARG_REST:
        start = ptr;
        init_dynamic_string(arg->ds, nullptr, command->query_len, 256);
        do_eval(arg->ds, start, command->end, false);
        command->last_argument = command->end;
        DBUG_PRINT("info", ("val: %s", arg->ds->str));
        break;

      default:
        DBUG_ASSERT("Unknown argument type");
        break;
    }

    /* Check required arg */
    if (arg->ds->length == 0 && arg->required)
      die("Missing required argument '%s' to command '%.*s'", arg->argname,
          static_cast<int>(command->first_word_len), command->query);
  }
  /* Check for too many arguments passed */
  ptr = command->last_argument;
  while (ptr <= command->end && *ptr != '#') {
    if (*ptr && *ptr != ' ')
      die("Extra argument '%s' passed to '%.*s'", ptr,
          static_cast<int>(command->first_word_len), command->query);
    ptr++;
  }
}

/// Check whether given error is in list of expected errors.
///
/// @param command      Pointer to the st_command structure which holds the
///                     arguments and information for the command.
/// @param err_errno    Error number of the error that actually occurred.
/// @param err_sqlstate SQLSTATE that was thrown, or NULL for impossible
///                     (file-ops, diff, etc.)
///
/// @retval -1 if the given error is not in the list, index in the
///         list of expected errors otherwise.
///
/// @note
/// If caller needs to know whether the list was empty, they should
/// check the value of expected_errors.count.
static int match_expected_error(struct st_command *command,
                                std::uint32_t err_errno,
                                const char *err_sqlstate) {
  std::uint8_t index = 0;

  // Iterator for list/vector of expected errors
  std::vector<std::unique_ptr<Error>>::iterator error =
      expected_errors->begin();

  // Iterate over list of expected errors
  for (; error != expected_errors->end(); error++) {
    if ((*error)->type() == ERR_ERRNO) {
      // Error type is ERR_ERRNO
      if ((*error)->error_code() == err_errno) return index;
    } else if ((*error)->type() == ERR_SQLSTATE) {
      // Error type is ERR_SQLSTATE. NULL is quite likely, but not in
      // conjunction with a SQL-state expect.
      if (unlikely(err_sqlstate == nullptr)) {
        die("Expecting a SQLSTATE (%s) from query '%s' which cannot produce "
            "one.",
            (*error)->sqlstate(), command->query);
      }

      if (!std::strncmp((*error)->sqlstate(), err_sqlstate, SQLSTATE_LENGTH))
        return index;
    }

    index++;
  }

  return -1;
}

/// Handle errors which occurred during execution of a query.
///
/// @param command      Pointer to the st_command structure which holds the
///                     arguments and information for the command.
/// @param err_errno    Error number
/// @param err_error    Error message
/// @param err_sqlstate SQLSTATE that was thrown
/// @param ds           Dynamic string to store the result.
///
/// @note
/// If there is an unexpected error, this function will abort mysqltest
/// immediately.
void handle_error(struct st_command *command, std::uint32_t err_errno,
                  const char *err_error, const char *err_sqlstate,
                  DYNAMIC_STRING *ds) {
  DBUG_TRACE;

  if (command->abort_on_error)
    die("Query '%s' failed.\nERROR %d (%s): %s", command->query, err_errno,
        err_sqlstate, err_error);

  DBUG_PRINT("info", ("Expected errors count: %zu", expected_errors->count()));

  int i = match_expected_error(command, err_errno, err_sqlstate);

  if (i >= 0) {
    if (!disable_result_log) {
      if (expected_errors->count() == 1) {
        // Only log error if there is one possible error
        dynstr_append_mem(ds, "ERROR ", 6);
        replace_dynstr_append(ds, err_sqlstate);
        dynstr_append_mem(ds, ": ", 2);
        replace_dynstr_append(ds, err_error);
        dynstr_append_mem(ds, "\n", 1);
      }
      // Don't log error if we may not get an error
      else if (expected_errors->type() == ERR_SQLSTATE ||
               (expected_errors->type() == ERR_ERRNO &&
                expected_errors->error_code() != 0))
        dynstr_append(ds, "Got one of the listed errors\n");
    }

    revert_properties();
    return;
  }

  DBUG_PRINT("info",
             ("i: %d Expected errors count: %zu", i, expected_errors->count()));

  if (!disable_result_log) {
    dynstr_append_mem(ds, "ERROR ", 6);
    replace_dynstr_append(ds, err_sqlstate);
    dynstr_append_mem(ds, ": ", 2);
    replace_dynstr_append(ds, err_error);
    dynstr_append_mem(ds, "\n", 1);
  }

  if (expected_errors->count()) {
    if (expected_errors->count() == 1) {
      die("Query '%s' failed with wrong error %d: '%s', should have failed "
          "with error '%s'.",
          command->query, err_errno, err_error,
          expected_errors->error_list().c_str());
    } else {
      die("Query '%s' failed with wrong error %d: '%s', should have failed "
          "with any of '%s' errors.",
          command->query, err_errno, err_error,
          expected_errors->error_list().c_str());
    }
  }

  revert_properties();
}

/// Handle absence of errors after execution.
///
/// Abort test run if the query succeeds and was expected to fail with
/// an error.
///
/// @param command Pointer to the st_command structure which holds the
///                arguments and information for the command.
void handle_no_error(struct st_command *command) {
  DBUG_TRACE;

  if (expected_errors->count()) {
    int index = match_expected_error(command, 0, "00000");
    if (index == -1) {
      if (expected_errors->count() == 1) {
        die("Query '%s' succeeded, should have failed with error '%s'",
            command->query, expected_errors->error_list().c_str());
      } else {
        die("Query '%s' succeeded, should have failed with any of '%s' errors.",
            command->query, expected_errors->error_list().c_str());
      }
    }
  }
}

/// Save error code returned by a mysqltest command in '$__error'
/// variable.
///
/// @param error Error code
static void save_error_code(std::uint32_t error) {
  char error_value[10];
  size_t error_length = std::snprintf(error_value, 10, "%u", error);
  error_value[error_length > 9 ? 9 : error_length] = '0';
  const char *var_name = "__error";
  var_set(var_name, var_name + 7, error_value, error_value + error_length);
}

/// Handle errors which occurred during execution of mysqltest commands
/// like 'move_file', 'remove_file' etc which are used to perform file
/// system operations.
///
/// @param command Pointer to the st_command structure which holds the
///                arguments and information for the command.
/// @param error   Error number
///
/// @note
/// If there is an unexpected error, this function will abort mysqltest
/// client immediately.
static void handle_command_error(struct st_command *command,
                                 std::uint32_t error) {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("error: %d", error));

  if (error != 0 && command->abort_on_error) {
    die("Command \"%s\" failed with error %d. my_errno=%d. "
        "net.last_errno=%d socket_errno=%d",
        command_names[command->type - 1], error, my_errno(),
        cur_con->mysql.net.last_errno, socket_errno);
  }

  if (expected_errors->count()) {
    int index = match_expected_error(command, error, nullptr);
    if (index == -1) {
      if (error != 0) {
        if (expected_errors->count() == 1) {
          die("Command \"%s\" failed with wrong error: %d, my_errno=%d. should "
              "have failed with error '%s'.",
              command_names[command->type - 1], error, my_errno(),
              expected_errors->error_list().c_str());
        } else {
          die("Command \"%s\" failed with wrong error: %d, my_errno=%d. should "
              "have failed with any of '%s' errors.",
              command_names[command->type - 1], error, my_errno(),
              expected_errors->error_list().c_str());
        }
      } else {
        // Command succeeded, should have failed with an error
        if (expected_errors->count() == 1) {
          die("Command \"%s\" succeeded, should have failed with error '%s'.",
              command_names[command->type - 1],
              expected_errors->error_list().c_str());
        } else {
          die("Command \"%s\" succeeded, should have failed with any of '%s' "
              "errors.",
              command_names[command->type - 1],
              expected_errors->error_list().c_str());
        }
      }
    }
  }

  // Save the error code
  save_error_code(error);

  revert_properties();
}

static void close_connections() {
  DBUG_TRACE;
  for (--next_con; next_con >= connections; --next_con) {
    if (next_con->stmt) mysql_stmt_close(next_con->stmt);
    next_con->stmt = nullptr;
    mysql_close(&next_con->mysql);
    if (next_con->util_mysql) mysql_close(next_con->util_mysql);
    my_free(next_con->name);
  }
  my_free(connections);
}

static void close_statements() {
  struct st_connection *con;
  DBUG_TRACE;
  for (con = connections; con < next_con; con++) {
    if (con->stmt) mysql_stmt_close(con->stmt);
    con->stmt = nullptr;
  }
}

static void close_files() {
  DBUG_TRACE;
  for (; cur_file >= file_stack; cur_file--) {
    if (cur_file->file && cur_file->file != stdin) {
      DBUG_PRINT("info", ("closing file: %s", cur_file->file_name));
      fclose(cur_file->file);
    }
    my_free(cur_file->file_name);
    cur_file->file_name = nullptr;
  }
}

static void free_used_memory() {
  // Delete the exptected errors pointer
  delete expected_errors;

  // Delete disabled and enabled warning list
  delete disabled_warnings;
  delete enabled_warnings;

  if (connections) close_connections();
  close_files();
  delete var_hash;
  var_hash = nullptr;

  struct st_command **q;
  for (q = q_lines->begin(); q != q_lines->end(); ++q) {
    my_free((*q)->query_buf);
    if ((*q)->content.str) dynstr_free(&(*q)->content);
    my_free((*q));
  }

  for (size_t i = 0; i < 10; i++) {
    if (var_reg[i].alloced_len) my_free(var_reg[i].str_val);
  }

  delete q_lines;
  dynstr_free(&ds_res);
  dynstr_free(&ds_result);
  if (ds_warn) dynstr_free(ds_warn);
  free_all_replace();
  my_free(opt_pass);
#ifdef _WIN32
  free_win_path_patterns();
#endif

  // Only call mysql_server_end if mysql_server_init has been called.
  if (server_initialized) mysql_server_end();

  // Don't use DBUG after mysql_server_end()
  return;
}

static void cleanup_and_exit(int exit_code) {
  if (opt_offload_count_file) {
    // Check if the current connection is active, if not create one.
    if (cur_con->mysql.net.vio == nullptr) {
      mysql_real_connect(&cur_con->mysql, opt_host, opt_user, opt_pass, opt_db,
                         opt_port, unix_sock,
                         CLIENT_MULTI_STATEMENTS | CLIENT_REMEMBER_OPTIONS);
    }

    // Save the final value of secondary engine execution status.
    if (secondary_engine->offload_count(&cur_con->mysql, "after"))
      exit_code = 1;
    secondary_engine->report_offload_count(opt_offload_count_file);
  }

  free_used_memory();
  my_end(my_end_arg);

  if (!silent) {
    switch (exit_code) {
      case 1:
        printf("not ok\n");
        break;
      case 0:
        printf("ok\n");
        break;
      case 62:
        printf("skipped\n");
        break;
      default:
        printf("unknown exit code: %d\n", exit_code);
        DBUG_ASSERT(0);
    }
  }

// exit() appears to be not 100% reliable on Windows under some conditions.
#ifdef _WIN32
  if (opt_safe_process_pid) {
    // Close the stack trace request event handle
    if (stacktrace_request_event != NULL) CloseHandle(stacktrace_request_event);

    // Detach or stop the thread waiting for stack trace event to occur.
    if (wait_for_stacktrace_request_event_thread.joinable())
      wait_for_stacktrace_request_event_thread.detach();

    // Close the thread handle
    if (!CloseHandle(mysqltest_thread))
      die("CloseHandle failed, err = %d.\n", GetLastError());
  }

  fflush(stdout);
  fflush(stderr);
  _exit(exit_code);
#else
  exit(exit_code);
#endif
}

static void print_file_stack() {
  fprintf(stderr, "file %s: %d\n", cur_file->file_name, cur_file->lineno);

  struct st_test_file *err_file;
  for (err_file = cur_file - 1; err_file >= file_stack; err_file--) {
    fprintf(stderr, "included from %s: %d\n", err_file->file_name,
            err_file->lineno);
  }
}

void die(const char *fmt, ...) {
  static int dying = 0;
  va_list args;
  DBUG_PRINT("enter", ("start_lineno: %d", start_lineno));

  // Protect against dying twice first time 'die' is called, try to
  // write log files second time, just exit.
  if (dying) cleanup_and_exit(1);
  dying = 1;

  // Print the error message
  fprintf(stderr, "mysqltest: ");

  if (start_lineno > 0) fprintf(stderr, "At line %u: ", start_lineno);

  if (fmt) {
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
  } else
    fprintf(stderr, "Unknown error");

  // Print the file stack
  if (cur_file && cur_file != file_stack) {
    fprintf(stderr, "In included ");
    print_file_stack();
  }

  fflush(stderr);

  if (result_file_name) log_file.show_tail(opt_tail_lines);

  // Help debugging by displaying any warnings that might have
  // been produced prior to the error.
  if (cur_con && !cur_con->pending) show_warnings_before_error(&cur_con->mysql);

  cleanup_and_exit(1);
}

void abort_not_supported_test(const char *fmt, ...) {
  va_list args;
  DBUG_TRACE;

  /* Print include filestack */
  fprintf(stderr, "The test '%s' is not supported by this installation\n",
          file_stack->file_name);
  fprintf(stderr, "Detected in ");
  print_file_stack();

  /* Print error message */
  va_start(args, fmt);
  if (fmt) {
    fprintf(stderr, "reason: ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    fflush(stderr);
  }
  va_end(args);

  cleanup_and_exit(62);
}

void verbose_msg(const char *fmt, ...) {
  va_list args;
  DBUG_TRACE;
  if (!verbose) return;

  va_start(args, fmt);
  fprintf(stderr, "mysqltest: ");
  if (cur_file && cur_file != file_stack)
    fprintf(stderr, "In included file \"%s\": ", cur_file->file_name);
  if (start_lineno != 0) fprintf(stderr, "At line %u: ", start_lineno);
  vfprintf(stderr, fmt, args);
  fprintf(stderr, "\n");
  va_end(args);
}

void log_msg(const char *fmt, ...) {
  va_list args;
  char buff[1024];
  size_t len;
  DBUG_TRACE;

  va_start(args, fmt);
  len = vsnprintf(buff, sizeof(buff) - 1, fmt, args);
  va_end(args);

  dynstr_append_mem(&ds_res, buff, len);
  dynstr_append(&ds_res, "\n");
}

/*
  Read a file and append it to ds

  SYNOPSIS
  cat_file
  ds - pointer to dynamic string where to add the files content
  filename - name of the file to read

*/

static int cat_file(DYNAMIC_STRING *ds, const char *filename) {
  int fd;
  size_t len;
  char buff[512];
  bool dangling_cr = false;

  if ((fd = my_open(filename, O_RDONLY, MYF(0))) < 0) return 1;

  std::string file_content;

  while ((len = my_read(fd, (uchar *)&buff, sizeof(buff), MYF(0))) > 0) {
    char *p = buff, *start = buff;
    if (dangling_cr) {
      if (*p != '\n') file_content.append("\r");
      dangling_cr = false;
    }
    while (p < buff + len) {
      /* Convert cr/lf to lf */
      if (*p == '\r' && *(p + 1) && *(p + 1) == '\n') {
        /* Add fake newline instead of cr and output the line */
        *p = '\n';
        p++; /* Step past the "fake" newline */
        file_content.append(start, p - start);
        p++; /* Step past the "fake" newline */
        start = p;
      } else
        p++;
    }
    if (*(p - 1) == '\r' && len == 512) dangling_cr = true;
    size_t buf_len;
    /* Add any characters that might be left */
    if (dangling_cr)
      buf_len = p - start - 1;
    else
      buf_len = p - start;
    file_content.append(start, buf_len);
  }

  replace_dynstr_append_mem(ds, file_content.c_str(), file_content.length());
  my_close(fd, MYF(0));

  return 0;
}

/*
  Run the specified command with popen

  SYNOPSIS
  run_command
  cmd - command to execute(should be properly quoted
  ds_res- pointer to dynamic string where to store the result

*/

static int run_command(char *cmd, DYNAMIC_STRING *ds_res) {
  char buf[512] = {0};
  FILE *res_file;
  int error;

  if (!(res_file = popen(cmd, "r"))) die("popen(\"%s\", \"r\") failed", cmd);

  while (fgets(buf, sizeof(buf), res_file)) {
    DBUG_PRINT("info", ("buf: %s", buf));
    if (ds_res) {
      /* Save the output of this command in the supplied string */
      dynstr_append(ds_res, buf);
    } else {
      /* Print it directly on screen */
      fprintf(stdout, "%s", buf);
    }
  }

  error = pclose(res_file);
  return WEXITSTATUS(error);
}

/*
  Run the specified tool with variable number of arguments

  SYNOPSIS
  run_tool
  tool_path - the name of the tool to run
  ds_res - pointer to dynamic string where to store the result
  ... - variable number of arguments that will be properly
        quoted and appended after the tool's name

*/

static int run_tool(const char *tool_path, DYNAMIC_STRING *ds_res, ...) {
  int ret;
  const char *arg;
  va_list args;
  DYNAMIC_STRING ds_cmdline;

  DBUG_TRACE;
  DBUG_PRINT("enter", ("tool_path: %s", tool_path));

  if (init_dynamic_string(&ds_cmdline, IF_WIN("\"", ""), FN_REFLEN, FN_REFLEN))
    die("Out of memory");

  dynstr_append_os_quoted(&ds_cmdline, tool_path, NullS);
  dynstr_append(&ds_cmdline, " ");

  va_start(args, ds_res);

  while ((arg = va_arg(args, char *))) {
    /* Options should be os quoted */
    if (std::strncmp(arg, "--", 2) == 0)
      dynstr_append_os_quoted(&ds_cmdline, arg, NullS);
    else
      dynstr_append(&ds_cmdline, arg);
    dynstr_append(&ds_cmdline, " ");
  }

  va_end(args);

#ifdef _WIN32
  dynstr_append(&ds_cmdline, "\"");
#endif

  DBUG_PRINT("info", ("Running: %s", ds_cmdline.str));
  ret = run_command(ds_cmdline.str, ds_res);
  DBUG_PRINT("exit", ("ret: %d", ret));
  dynstr_free(&ds_cmdline);
  return ret;
}

/*
  Test if diff is present.  This is needed on Windows systems
  as the OS returns 1 whether diff is successful or if it is
  not present.

  We run diff -v and look for output in stdout.
  We don't redirect stderr to stdout to make for a simplified check
  Windows will output '"diff"' is not recognized... to stderr if it is
  not present.
*/

#ifdef _WIN32

static int diff_check(const char *diff_name) {
  FILE *res_file;
  char buf[128];
  int have_diff = 0;

  snprintf(buf, sizeof(buf), "%s -v", diff_name);

  if (!(res_file = popen(buf, "r"))) die("popen(\"%s\", \"r\") failed", buf);

  /* if diff is not present, nothing will be in stdout to increment have_diff */
  if (fgets(buf, sizeof(buf), res_file)) have_diff = 1;

  pclose(res_file);

  return have_diff;
}

#endif

/// Show the diff of two files using the systems builtin diff
/// command. If no such diff command exist, just dump the content
/// of the two files and inform about how to get "diff"
///
/// @param ds        Pointer to dynamic string where to add the
///                  diff. If NULL, print the diff to stderr.
/// @param filename1 Name of the first file
/// @param filename2 Name of the second file
static void show_diff(DYNAMIC_STRING *ds, const char *filename1,
                      const char *filename2) {
  DYNAMIC_STRING ds_diff;
  if (init_dynamic_string(&ds_diff, "", 256, 256)) die("Out of memory");

  const char *diff_name = nullptr;

  // Determine if we have diff on Windows. If yes, then needs special
  // processing due to return values on that OS. This test is only done
  // on Windows since it's only needed there in order to correctly
  // detect non-availibility of 'diff', and the way it's implemented
  // does not work with default 'diff' on Solaris.
#ifdef _WIN32
  if (diff_check("diff"))
    diff_name = "diff";
  else if (diff_check("mtrdiff"))
    diff_name = "mtrdiff";
  else
    diff_name = 0;
#else
  // Otherwise always assume it's called diff
  diff_name = "diff";
#endif

  if (diff_name) {
    int exit_code = 0;
    // Use 'diff --color=always' to print the colored diff if it is enabled
    if (opt_colored_diff) {
      // Most "diff" tools return '> 1' if error
      exit_code = run_tool(diff_name, &ds_diff, "-u --color='always'",
                           filename1, filename2, "2>&1", NULL);

      if (exit_code > 1)
        die("Option '--colored-diff' is not supported on this machine. "
            "To get colored diff output, install GNU diffutils version "
            "3.4 or higher.");
    } else {
      // Colored diff is disabled, clear the diff string and try unified
      // diff with "diff -u".
      dynstr_set(&ds_diff, "");
      exit_code = run_tool(diff_name, &ds_diff, "-u", filename1, filename2,
                           "2>&1", NULL);

      if (exit_code > 1) {
        // Clear the diff string and fallback to context diff with "diff -c"
        dynstr_set(&ds_diff, "");
        exit_code = run_tool(diff_name, &ds_diff, "-c", filename1, filename2,
                             "2>&1", NULL);

        if (exit_code > 1) {
          // Clear the diff string and fallback to simple diff with "diff"
          dynstr_set(&ds_diff, "");
          exit_code =
              run_tool(diff_name, &ds_diff, filename1, filename2, "2>&1", NULL);
          if (exit_code > 1) diff_name = nullptr;
        }
      }
    }
  }

  if (!diff_name) {
    // Fallback to dump both files to result file and inform
    // about installing "diff".
    dynstr_append(&ds_diff, "\n");
    dynstr_append(
        &ds_diff,
        "\n"
        "The two files differ but it was not possible to execute 'diff' in\n"
        "order to show only the difference. Instead the whole content of the\n"
        "two files was shown for you to diff manually.\n\n"
        "To get a better report you should install 'diff' on your system, "
        "which you\n"
        "for example can get from "
        "http://www.gnu.org/software/diffutils/diffutils.html\n"
#ifdef _WIN32
        "or http://gnuwin32.sourceforge.net/packages/diffutils.htm\n"
#endif
        "\n");

    dynstr_append(&ds_diff, " --- ");
    dynstr_append(&ds_diff, filename1);
    dynstr_append(&ds_diff, " >>>\n");
    cat_file(&ds_diff, filename1);
    dynstr_append(&ds_diff, "<<<\n --- ");
    dynstr_append(&ds_diff, filename1);
    dynstr_append(&ds_diff, " >>>\n");
    cat_file(&ds_diff, filename2);
    dynstr_append(&ds_diff, "<<<<\n");
  }

  if (ds)
    // Add the diff to output
    dynstr_append_mem(ds, ds_diff.str, ds_diff.length);
  else
    // Print diff directly to stderr
    fprintf(stderr, "%s\n", ds_diff.str);

  dynstr_free(&ds_diff);
}

enum compare_files_result_enum {
  RESULT_OK = 0,
  RESULT_CONTENT_MISMATCH = 1,
  RESULT_LENGTH_MISMATCH = 2
};

/*
  Compare two files, given a fd to the first file and
  name of the second file

  SYNOPSIS
  compare_files2
  fd - Open file descriptor of the first file
  filename2 - Name of second file

  RETURN VALUES
  According to the values in "compare_files_result_enum"

*/

static int compare_files2(File fd, const char *filename2) {
  int error = RESULT_OK;
  File fd2;
  size_t len, len2;
  char buff[512], buff2[512];

  if ((fd2 = my_open(filename2, O_RDONLY, MYF(0))) < 0) {
    my_close(fd, MYF(0));
    die("Failed to open second file: '%s'", filename2);
  }
  while ((len = my_read(fd, (uchar *)&buff, sizeof(buff), MYF(0))) > 0) {
    if ((len2 = my_read(fd2, (uchar *)&buff2, sizeof(buff2), MYF(0))) < len) {
      /* File 2 was smaller */
      error = RESULT_LENGTH_MISMATCH;
      break;
    }
    if (len2 > len) {
      /* File 1 was smaller */
      error = RESULT_LENGTH_MISMATCH;
      break;
    }
    if ((memcmp(buff, buff2, len))) {
      /* Content of this part differed */
      error = RESULT_CONTENT_MISMATCH;
      break;
    }
  }
  if (!error && my_read(fd2, (uchar *)&buff2, sizeof(buff2), MYF(0)) > 0) {
    /* File 1 was smaller */
    error = RESULT_LENGTH_MISMATCH;
  }

  my_close(fd2, MYF(0));

  return error;
}

/*
  Compare two files, given their filenames

  SYNOPSIS
  compare_files
  filename1 - Name of first file
  filename2 - Name of second file

  RETURN VALUES
  See 'compare_files2'

*/

static int compare_files(const char *filename1, const char *filename2) {
  File fd;
  int error;

  if ((fd = my_open(filename1, O_RDONLY, MYF(0))) < 0)
    die("Failed to open first file: '%s'", filename1);

  error = compare_files2(fd, filename2);

  my_close(fd, MYF(0));

  return error;
}

/*
  Check the content of log against result file

  SYNOPSIS
  check_result

  RETURN VALUES
  error - the function will not return

*/

static void check_result() {
  const char *mess = "Result content mismatch\n";

  DBUG_TRACE;
  DBUG_ASSERT(result_file_name);
  DBUG_PRINT("enter", ("result_file_name: %s", result_file_name));

  /*
    Removing the unnecessary warning messages generated
    on GCOV platform.
  */
#ifdef HAVE_GCOV
  char cmd[FN_REFLEN];
  strcpy(cmd, "sed -i '/gcda:Merge mismatch for function/d' ");
  std::strcat(cmd, log_file.file_name());
  system(cmd);
#endif

  switch (compare_files(log_file.file_name(), result_file_name)) {
    case RESULT_OK:
      break; /* ok */
    case RESULT_LENGTH_MISMATCH:
      mess = "Result length mismatch\n";
      /* Fallthrough */
    case RESULT_CONTENT_MISMATCH: {
      /*
        Result mismatched, dump results to .reject file
        and then show the diff
      */
      char reject_file[FN_REFLEN];
      size_t reject_length;
      dirname_part(reject_file, result_file_name, &reject_length);

      /* Put reject file in opt_logdir */
      fn_format(reject_file, result_file_name, opt_logdir, ".reject",
                MY_REPLACE_DIR | MY_REPLACE_EXT);

      if (my_copy(log_file.file_name(), reject_file, MYF(0)) != 0)
        die("Failed to copy '%s' to '%s', errno: %d", log_file.file_name(),
            reject_file, errno);

      show_diff(nullptr, result_file_name, reject_file);
      die("%s", mess);
      break;
    }
    default: /* impossible */
      die("Unknown error code from dyn_string_cmp()");
  }
}

/*
   Remove surrounding chars from string

   Return 1 if first character is found but not last
*/
static int strip_surrounding(char *str, char c1, char c2) {
  char *ptr = str;

  /* Check if the first non space character is c1 */
  while (*ptr && my_isspace(charset_info, *ptr)) ptr++;
  if (*ptr == c1) {
    /* Replace it with a space */
    *ptr = ' ';

    /* Last non space charecter should be c2 */
    ptr = strend(str) - 1;
    while (*ptr && my_isspace(charset_info, *ptr)) ptr--;
    if (*ptr == c2) {
      /* Replace it with \0 */
      *ptr = 0;
    } else {
      /* Mismatch detected */
      return 1;
    }
  }
  return 0;
}

static void strip_parentheses(struct st_command *command) {
  if (strip_surrounding(command->first_argument, '(', ')'))
    die("%.*s - argument list started with '%c' must be ended with '%c'",
        static_cast<int>(command->first_word_len), command->query, '(', ')');
}

void var_free::operator()(VAR *var) const {
  my_free(var->str_val);
  if (var->alloced) my_free(var);
}

static void var_check_int(VAR *v) {
  char *endptr;
  char *str = v->str_val;

  /* Initially assume not a number */
  v->int_val = 0;
  v->is_int = false;
  v->int_dirty = false;
  if (!str) return;

  v->int_val = (int)strtol(str, &endptr, 10);
  /* It is an int if strtol consumed something up to end/space/tab */
  if (endptr > str && (!*endptr || *endptr == ' ' || *endptr == '\t'))
    v->is_int = true;
}

VAR *var_init(VAR *v, const char *name, size_t name_len, const char *val,
              size_t val_len) {
  size_t val_alloc_len;
  VAR *tmp_var;
  if (!name_len && name) name_len = std::strlen(name);
  if (!val_len && val) val_len = std::strlen(val);
  if (!val) val_len = 0;
  val_alloc_len = val_len + 16; /* room to grow */
  if (!(tmp_var = v) && !(tmp_var = (VAR *)my_malloc(
                              PSI_NOT_INSTRUMENTED,
                              sizeof(*tmp_var) + name_len + 2, MYF(MY_WME))))
    die("Out of memory");

  if (name != nullptr) {
    tmp_var->name = reinterpret_cast<char *>(tmp_var) + sizeof(*tmp_var);
    memcpy(tmp_var->name, name, name_len);
    tmp_var->name[name_len] = 0;
  } else
    tmp_var->name = nullptr;

  tmp_var->alloced = (v == nullptr);

  if (!(tmp_var->str_val = (char *)my_malloc(PSI_NOT_INSTRUMENTED,
                                             val_alloc_len + 1, MYF(MY_WME))))
    die("Out of memory");

  if (val) memcpy(tmp_var->str_val, val, val_len);
  tmp_var->str_val[val_len] = 0;

  var_check_int(tmp_var);
  tmp_var->name_len = name_len;
  tmp_var->str_val_len = val_len;
  tmp_var->alloced_len = val_alloc_len;
  return tmp_var;
}

VAR *var_from_env(const char *name, const char *def_val) {
  const char *tmp;
  VAR *v;
  if (!(tmp = getenv(name))) tmp = def_val;

  v = var_init(nullptr, name, std::strlen(name), tmp, std::strlen(tmp));
  var_hash->emplace(name, std::unique_ptr<VAR, var_free>(v));
  return v;
}

VAR *var_get(const char *var_name, const char **var_name_end, bool raw,
             bool ignore_not_existing) {
  int digit;
  VAR *v;
  DBUG_TRACE;
  DBUG_PRINT("enter", ("var_name: %s", var_name));

  if (*var_name != '$') goto err;
  digit = *++var_name - '0';
  if (digit < 0 || digit >= 10) {
    const char *save_var_name = var_name, *end;
    uint length;
    end = (var_name_end) ? *var_name_end : nullptr;
    while (my_isvar(charset_info, *var_name) && var_name != end) var_name++;
    if (var_name == save_var_name) {
      if (ignore_not_existing) return nullptr;
      die("Empty variable");
    }
    length = (uint)(var_name - save_var_name);
    if (length >= MAX_VAR_NAME_LENGTH)
      die("Too long variable name: %s", save_var_name);

    if (!(v = find_or_nullptr(*var_hash, std::string(save_var_name, length)))) {
      char buff[MAX_VAR_NAME_LENGTH + 1];
      strmake(buff, save_var_name, length);
      v = var_from_env(buff, "");
    }
    var_name--; /* Point at last character */
  } else
    v = var_reg + digit;

  if (!raw && v->int_dirty) {
    sprintf(v->str_val, "%d", v->int_val);
    v->int_dirty = false;
    v->str_val_len = std::strlen(v->str_val);
  }
  if (var_name_end) *var_name_end = var_name;
  return v;
err:
  if (var_name_end) *var_name_end = nullptr;
  die("Unsupported variable name: %s", var_name);
  return nullptr;
}

static VAR *var_obtain(const char *name, int len) {
  VAR *v = find_or_nullptr(*var_hash, std::string(name, len));
  if (v == nullptr) {
    v = var_init(nullptr, name, len, "", 0);
    var_hash->emplace(std::string(name, len),
                      std::unique_ptr<VAR, var_free>(v));
  }
  return v;
}

/*
  - if variable starts with a $ it is regarded as a local test varable
  - if not it is treated as a environment variable, and the corresponding
  environment variable will be updated
*/

void var_set(const char *var_name, const char *var_name_end,
             const char *var_val, const char *var_val_end) {
  int digit, env_var = 0;
  VAR *v;
  DBUG_TRACE;
  DBUG_PRINT("enter", ("var_name: '%.*s' = '%.*s' (length: %d)",
                       (int)(var_name_end - var_name), var_name,
                       (int)(var_val_end - var_val), var_val,
                       (int)(var_val_end - var_val)));

  if (*var_name != '$')
    env_var = 1;
  else
    var_name++;

  digit = *var_name - '0';
  if (!(digit < 10 && digit >= 0)) {
    v = var_obtain(var_name, (uint)(var_name_end - var_name));
  } else
    v = var_reg + digit;

  eval_expr(v, var_val, (const char **)&var_val_end);

  if (env_var) {
    if (v->int_dirty) {
      sprintf(v->str_val, "%d", v->int_val);
      v->int_dirty = false;
      v->str_val_len = std::strlen(v->str_val);
    }
    /* setenv() expects \0-terminated strings */
    DBUG_ASSERT(v->name[v->name_len] == 0);
    setenv(v->name, v->str_val, 1);
  }
}

static void var_set_string(const char *name, const char *value) {
  var_set(name, name + std::strlen(name), value, value + std::strlen(value));
}

static void var_set_int(const char *name, int value) {
  char buf[21];
  snprintf(buf, sizeof(buf), "%d", value);
  var_set_string(name, buf);
}

/*
  Store an integer (typically the returncode of the last SQL)
  statement in the mysqltest builtin variable $mysql_errno
*/

static void var_set_errno(int sql_errno) {
  var_set_int("$mysql_errno", sql_errno);
  var_set_string("$mysql_errname", get_errname_from_code(sql_errno));
}

/// Variable '$DISABLED_WARNINGS_LIST' contains comma separated list
/// of disabled warnings and variable '$ENABLED_WARNINGS_LIST' contains
/// comma separated list of enabled warnings.
///
/// Update the value of these two variables with the latest list of
/// disabled and enabled warnings. The value of these variables will be
/// empty if there are no disabled or enabled warnings.
///
/// These variables will  always contain the latest list of disabled
/// and enabled warnings, and can be referenced inside a test or inside
/// a test utility file to access the current list of disabled or
/// enabled warnings.
static void update_disabled_enabled_warnings_list_var() {
  // Update '$DISABLED_WARNINGS_LIST' variable
  std::string disabled_warning_list = disabled_warnings->warnings_list();
  var_set_string("DISABLED_WARNINGS_LIST", disabled_warning_list.c_str());

  // Update '$ENABLED_WARNINGS_LIST' variable
  std::string enabled_warning_list = enabled_warnings->warnings_list();
  var_set_string("ENABLED_WARNINGS_LIST", enabled_warning_list.c_str());
}

/// Set a property value to either 0 or 1 for a disable_X or a enable_X
/// command, and the new value set will be applicable for next statement
/// only. After that, property value will be reset back to the old value.
///
/// @param property Enum value representing a Property
/// @param value    Value for the property, either 0 or 1
static void set_once_property(enum_prop property, bool value) {
  Property &prop = prop_list[property];
  prop.set = true;
  prop.old = *prop.var;
  *prop.var = value;
  var_set_int(prop.env_name, (value != prop.reverse));
  once_property = true;
}

/// Set a property value to either 0 or 1 for a disable_X or a enable_X
/// command.
///
/// @param command  Pointer to the st_command structure which holds the
///                 arguments and information for the command.
/// @param property Enum value representing a Property
/// @param value    Value for the property, either 0 or 1
static void set_property(st_command *command, enum_prop property, bool value) {
  char *arg = command->first_argument;

  // If "ONCE" argument is specified, the new value for the property is
  // set for next statement only. After that, property value will be
  // reset back to the old value.
  if (arg) {
    // "ONCE" is the second argument to 'disable_warnings/enable_warnings'
    // command.
    if (((command->type == Q_DISABLE_WARNINGS ||
          command->type == Q_ENABLE_WARNINGS) &&
         std::strstr(arg, "ONCE") != nullptr) ||
        !std::strcmp(arg, "ONCE")) {
      command->last_argument = arg + std::strlen(arg);
      set_once_property(property, value);
      return;
    }
  }

  Property &prop = prop_list[property];
  prop.set = false;
  *prop.var = value;
  var_set_int(prop.env_name, (value != prop.reverse));
}

/// Reset property value to the old value for all properties which are
/// set for the next statement only, i.e properties specified using
/// keyword "ONCE" argument.
void revert_properties() {
  if (!once_property) return;

  for (std::size_t i = 0; i < P_MAX; i++) {
    Property &prop = prop_list[i];
    if (prop.set) {
      *prop.var = prop.old;
      prop.set = false;
      var_set_int(prop.env_name, (prop.old != prop.reverse));
    }
  }

  // Remove warnings which are disabled or enabled for the next
  // statement only.
  disabled_warnings->update_list();
  enabled_warnings->update_list();

  // Update $DISABLED_WARNINGS_LIST and $ENABLED_WARNINGS_LIST
  // variable value.
  update_disabled_enabled_warnings_list_var();

  once_property = false;
}

/*
  Set variable from the result of a query

  SYNOPSIS
  var_query_set()
  var	        variable to set from query
  query       start of query string to execute
  query_end   end of the query string to execute


  DESCRIPTION
  let @<var_name> = `<query>`

  Execute the query and assign the first row of result to var as
  a tab separated strings

  Also assign each column of the result set to
  variable "$<var_name>_<column_name>"
  Thus the tab separated output can be read from $<var_name> and
  and each individual column can be read as $<var_name>_<col_name>

*/

static void var_query_set(VAR *var, const char *query, const char **query_end) {
  const char *end =
      (query_end && *query_end) ? *query_end : query + std::strlen(query);
  MYSQL_RES *res = nullptr;
  MYSQL_ROW row;
  MYSQL *mysql = &cur_con->mysql;
  DYNAMIC_STRING ds_query;
  DBUG_TRACE;

  /* Only white space or ) allowed past ending ` */
  while (end > query && *end != '`') {
    if (*end && (*end != ' ' && *end != '\t' && *end != '\n' && *end != ')'))
      die("Spurious text after `query` expression");
    --end;
  }

  if (query == end) die("Syntax error in query, missing '`'");
  ++query;

  /* Eval the query, thus replacing all environment variables */
  init_dynamic_string(&ds_query, nullptr, (end - query) + 32, 256);
  do_eval(&ds_query, query, end, false);

  if (mysql_real_query_wrapper(mysql, ds_query.str,
                               static_cast<ulong>(ds_query.length))) {
    handle_error(curr_command, mysql_errno(mysql), mysql_error(mysql),
                 mysql_sqlstate(mysql), &ds_res);
    /* If error was acceptable, return empty string */
    dynstr_free(&ds_query);
    eval_expr(var, "", nullptr);
    return;
  }

  if (!(res = mysql_store_result_wrapper(mysql)))
    die("Query '%s' didn't return a result set", ds_query.str);
  dynstr_free(&ds_query);

  if ((row = mysql_fetch_row_wrapper(res)) && row[0]) {
    /*
      Concatenate all fields in the first row with tab in between
      and assign that string to the $variable
    */
    DYNAMIC_STRING result;
    uint i;
    ulong *lengths;

    init_dynamic_string(&result, "", 512, 512);
    lengths = mysql_fetch_lengths(res);
    for (i = 0; i < mysql_num_fields(res); i++) {
      if (row[i]) {
        /* Add column to tab separated string */
        char *val = row[i];
        size_t len = lengths[i];

        if (glob_replace_regex) {
          size_t orig_len = len;
          // Regex replace
          if (!multi_reg_replace(glob_replace_regex, (char *)val, &len)) {
            val = glob_replace_regex->buf;
          } else {
            len = orig_len;
          }
        }
        DYNAMIC_STRING ds_temp;
        init_dynamic_string(&ds_temp, "", 512, 512);

        /* Store result from replace_result in ds_temp */
        if (glob_replace)
          replace_strings_append(glob_replace, &ds_temp, val, len);

        /*
          Call the replace_numeric_round function with the specified
          precision. It may be used along with replace_result, so use the
          output from replace_result as the input for replace_numeric_round.
*/
        if (glob_replace_numeric_round >= 0) {
          /* Copy the result from replace_result if it was used, into buffer */
          if (ds_temp.length > 0) {
            char buffer[512];
            strcpy(buffer, ds_temp.str);
            dynstr_free(&ds_temp);
            init_dynamic_string(&ds_temp, "", 512, 512);
            replace_numeric_round_append(glob_replace_numeric_round, &ds_temp,
                                         buffer, std::strlen(buffer));
          } else
            replace_numeric_round_append(glob_replace_numeric_round, &ds_temp,
                                         val, len);
        }

        if (!glob_replace && glob_replace_numeric_round < 0)
          dynstr_append_mem(&result, val, len);
        else
          dynstr_append_mem(&result, ds_temp.str, std::strlen(ds_temp.str));
        dynstr_free(&ds_temp);
      }
      dynstr_append_mem(&result, "\t", 1);
    }
    end = result.str + result.length - 1;
    /* Evaluation should not recurse via backtick */
    eval_expr(var, result.str, &end, false, false);
    dynstr_free(&result);
  } else
    eval_expr(var, "", nullptr);

  mysql_free_result_wrapper(res);
}

static void set_result_format_version(ulong new_version) {
  switch (new_version) {
    case 1:
      /* The first format */
      break;
    case 2:
      /* New format that also writes comments and empty lines
         from test file to result */
      break;
    default:
      die("Version format %lu has not yet been implemented", new_version);
      break;
  }
  opt_result_format_version = new_version;
}

/*
  Set the result format version to use when generating
  the .result file
*/

static void do_result_format_version(struct st_command *command) {
  long version;
  static DYNAMIC_STRING ds_version;
  const struct command_arg result_format_args[] = {
      {"version", ARG_STRING, true, &ds_version, "Version to use"}};

  DBUG_TRACE;

  check_command_args(command, command->first_argument, result_format_args,
                     sizeof(result_format_args) / sizeof(struct command_arg),
                     ',');

  /* Convert version  number to int */
  if (!str2int(ds_version.str, 10, (long)0, (long)INT_MAX, &version))
    die("Invalid version number: '%s'", ds_version.str);

  set_result_format_version(version);

  dynstr_append(&ds_res, "result_format: ");
  dynstr_append_mem(&ds_res, ds_version.str, ds_version.length);
  dynstr_append(&ds_res, "\n");
  dynstr_free(&ds_version);
}

/// Convert between error numbers and error names/strings.
///
/// @code
/// let $var = convert_error(ER_UNKNOWN_ERROR);
/// let $var = convert_error(1234);
/// @endcode
///
/// The variable '$var' will be populated with error number if the
/// argument is string. The variable var will be populated with error
/// string if the argument is number.
///
/// @param command Pointer to the st_command structure which holds the
///                arguments and information for the command.
/// @param var     Pointer to VAR object containing a variable
///                information.
static void var_set_convert_error(struct st_command *command, VAR *var) {
  // The command->query contains the statement convert_error(1234)
  char *first = std::strchr(command->query, '(') + 1;
  char *last = std::strchr(command->query, ')');

  // Denoting an empty string
  if (last == first) {
    eval_expr(var, "0", nullptr);
    return;
  }

  // If the string is an error string , it starts with 'E' as is the norm
  if (*first == 'E') {
    std::string error_name(first, int(last - first));
    int error = get_errcode_from_name(error_name);
    if (error == -1) die("Unknown SQL error name '%s'.", error_name.c_str());
    char str[100];
    std::sprintf(str, "%d", error);
    eval_expr(var, str, nullptr);
  } else if (my_isdigit(charset_info, *first)) {
    // Error number argument
    long int err = std::strtol(first, &last, 0);
    const char *err_name = get_errname_from_code(err);
    eval_expr(var, err_name, nullptr);
  } else {
    die("Invalid error in input");
  }
}

/// Get response attributes from current connection
///
/// @code
/// let $var = get_response_attribute(somekey);
/// @endcode
///
/// The variable '$var' will be populated with value of the response attribute
//  key if found. Otherwise, it'll contain an empty string.
///
/// @param command Pointer to the st_command structure which holds the
///                arguments and information for the command.
/// @param var     Pointer to VAR object containing a variable
///                information.
static void var_set_get_response_attribute(struct st_command *command,
                                           VAR *var) {
  // The command->query contains the statement get_response_attribute(key)
  char *first = std::strchr(command->query, '(') + 1;
  char *last = std::strchr(command->query, ')');

  // Denoting an empty string
  if (last == first) {
    die("Invalid error in input");
  }

  std::string key_name(first, int(last - first));

  MYSQL *mysql = &cur_con->mysql;
  const char *ptr = nullptr;
  size_t len = 0;
  (void)mysql_resp_attr_find(mysql, key_name.c_str(), &ptr, &len);

  if (ptr == nullptr) {
    ptr = "";
  }

  const char *ptr_end = ptr + len;

  eval_expr(var, ptr, &ptr_end, false, false);
  if (ptr_end != ptr) {
    DBUG_PRINT("info", ("%s: %s", key_name.c_str(), var->str_val));
  }
}

/*
  Set variable from the result of a field in a query

  This function is useful when checking for a certain value
  in the output from a query that can't be restricted to only
  return some values. A very good example of that is most SHOW
  commands.

  SYNOPSIS
  var_set_query_get_value()

  DESCRIPTION
  let $variable= query_get_value(<query to run>,<column name>,<row no>);

  <query to run> -    The query that should be sent to the server
  <column name> -     Name of the column that holds the field be compared
                      against the expected value
  <row no> -          Number of the row that holds the field to be
                      compared against the expected value

*/

static void var_set_query_get_value(struct st_command *command, VAR *var) {
  long row_no;
  int col_no = -1;
  MYSQL_RES *res = nullptr;
  MYSQL *mysql = &cur_con->mysql;

  static DYNAMIC_STRING ds_query;
  static DYNAMIC_STRING ds_col;
  static DYNAMIC_STRING ds_row;
  const struct command_arg query_get_value_args[] = {
      {"query", ARG_STRING, true, &ds_query, "Query to run"},
      {"column name", ARG_STRING, true, &ds_col, "Name of column"},
      {"row number", ARG_STRING, true, &ds_row, "Number for row"}};

  DBUG_TRACE;

  strip_parentheses(command);
  DBUG_PRINT("info", ("query: %s", command->query));
  check_command_args(command, command->first_argument, query_get_value_args,
                     sizeof(query_get_value_args) / sizeof(struct command_arg),
                     ',');

  DBUG_PRINT("info", ("query: %s", ds_query.str));
  DBUG_PRINT("info", ("col: %s", ds_col.str));

  /* Convert row number to int */
  if (!str2int(ds_row.str, 10, (long)0, (long)INT_MAX, &row_no))
    die("Invalid row number: '%s'", ds_row.str);
  DBUG_PRINT("info", ("row: %s, row_no: %ld", ds_row.str, row_no));
  dynstr_free(&ds_row);

  /* Remove any surrounding "'s from the query - if there is any */
  if (strip_surrounding(ds_query.str, '"', '"'))
    die("Mismatched \"'s around query '%s'", ds_query.str);

  /* Run the query */
  if (mysql_real_query_wrapper(mysql, ds_query.str,
                               static_cast<ulong>(ds_query.length))) {
    handle_error(curr_command, mysql_errno(mysql), mysql_error(mysql),
                 mysql_sqlstate(mysql), &ds_res);
    /* If error was acceptable, return empty string */
    dynstr_free(&ds_query);
    dynstr_free(&ds_col);
    eval_expr(var, "", nullptr);
    return;
  }

  if (!(res = mysql_store_result_wrapper(mysql)))
    die("Query '%s' didn't return a result set", ds_query.str);

  {
    /* Find column number from the given column name */
    uint i;
    uint num_fields = mysql_num_fields(res);
    MYSQL_FIELD *fields = mysql_fetch_fields(res);

    for (i = 0; i < num_fields; i++) {
      if (std::strcmp(fields[i].name, ds_col.str) == 0 &&
          std::strlen(fields[i].name) == ds_col.length) {
        col_no = i;
        break;
      }
    }
    if (col_no == -1) {
      mysql_free_result_wrapper(res);
      die("Could not find column '%s' in the result of '%s'", ds_col.str,
          ds_query.str);
    }
    DBUG_PRINT("info", ("Found column %d with name '%s'", i, fields[i].name));
  }
  dynstr_free(&ds_col);

  {
    /* Get the value */
    MYSQL_ROW row;
    long rows = 0;
    const char *value = "No such row";

    while ((row = mysql_fetch_row_wrapper(res))) {
      if (++rows == row_no) {
        DBUG_PRINT("info", ("At row %ld, column %d is '%s'", row_no, col_no,
                            row[col_no]));
        /* Found the row to get */
        if (row[col_no])
          value = row[col_no];
        else
          value = "NULL";

        break;
      }
    }
    eval_expr(var, value, nullptr, false, false);
  }
  dynstr_free(&ds_query);
  mysql_free_result_wrapper(res);
}

/*
  Set variable from the result of a field in a query

  This function is useful when checking for a certain value
  in the output from a query that can't be restricted to only
  return some values. A very good example of that is most SHOW
  commands.

  SYNOPSIS
  var_set_query_get_value_by_name()

  DESCRIPTION
  let $variable= query_get_value(<query to run>,<column name>,<first column
  value>);

  <query to run> -    The query that should be sent to the server
  <column name> -     Name of the column that holds the field be compared
                      against the expected value
  <first column value> - The expected value of in the first column

*/

static void var_set_query_get_value_by_name(struct st_command *command,
                                            VAR *var) {
  int col_no = -1;
  MYSQL_RES *res = nullptr;
  MYSQL *mysql = &cur_con->mysql;

  static DYNAMIC_STRING ds_query;
  static DYNAMIC_STRING ds_col;
  static DYNAMIC_STRING ds_row;
  const struct command_arg query_get_value_args[] = {
      {"query", ARG_STRING, true, &ds_query, "Query to run"},
      {"column name", ARG_STRING, true, &ds_col, "Name of column"},
      {"first column value>", ARG_STRING, true, &ds_row,
       "Value of in the first column"}};

  DBUG_TRACE;

  strip_parentheses(command);
  DBUG_PRINT("info", ("query: %s", command->query));
  check_command_args(command, command->first_argument, query_get_value_args,
                     sizeof(query_get_value_args) / sizeof(struct command_arg),
                     ',');

  DBUG_PRINT("info", ("query: %s", ds_query.str));
  DBUG_PRINT("info", ("col: %s", ds_col.str));

  /* Remove any surrounding "'s from the query - if there is any */
  if (strip_surrounding(ds_query.str, '"', '"'))
    die("Mismatched \"'s around query '%s'", ds_query.str);

  /* Run the query */
  if (mysql_real_query_wrapper(mysql, ds_query.str,
                               static_cast<ulong>(ds_query.length))) {
    handle_error(curr_command, mysql_errno(mysql), mysql_error(mysql),
                 mysql_sqlstate(mysql), &ds_res);
    /* If error was acceptable, return empty string */
    dynstr_free(&ds_query);
    dynstr_free(&ds_col);
    dynstr_free(&ds_row);
    eval_expr(var, "", nullptr);
    return;
  }

  if (!(res = mysql_store_result_wrapper(mysql)))
    die("Query '%s' didn't return a result set", ds_query.str);

  {
    /* Find column number from the given column name */
    uint i;
    uint num_fields = mysql_num_fields(res);
    MYSQL_FIELD *fields = mysql_fetch_fields(res);

    for (i = 0; i < num_fields; i++) {
      if (std::strcmp(fields[i].name, ds_col.str) == 0 &&
          std::strlen(fields[i].name) == ds_col.length) {
        col_no = i;
        break;
      }
    }
    if (col_no == -1) {
      mysql_free_result_wrapper(res);
      die("Could not find column '%s' in the result of '%s'", ds_col.str,
          ds_query.str);
    }
    DBUG_PRINT("info", ("Found column %d with name '%s'", i, fields[i].name));
  }
  dynstr_free(&ds_col);

  {
    /* Get the value */
    MYSQL_ROW row;
    long rows = 0;
    const char *value = "No such row";

    while ((row = mysql_fetch_row_wrapper(res))) {
      ++rows;
      if (row[0] && std::strcmp(row[0], ds_row.str) == 0 &&
          std::strlen(row[0]) == ds_row.length) {
        DBUG_PRINT("info", ("At row %ld, column %d is '%s'", rows, col_no,
                            row[col_no]));
        /* Found the row to get */
        if (row[col_no])
          value = row[col_no];
        else
          value = "NULL";

        break;
      }
    }
    eval_expr(var, value, nullptr, false, false);
  }
  dynstr_free(&ds_row);
  dynstr_free(&ds_query);
  mysql_free_result_wrapper(res);
}

static void var_copy(VAR *dest, VAR *src) {
  dest->int_val = src->int_val;
  dest->is_int = src->is_int;
  dest->int_dirty = src->int_dirty;

  /* Alloc/realloc data for str_val in dest */
  if (dest->alloced_len < src->alloced_len &&
      !(dest->str_val =
            dest->str_val
                ? (char *)my_realloc(PSI_NOT_INSTRUMENTED, dest->str_val,
                                     src->alloced_len, MYF(MY_WME))
                : (char *)my_malloc(PSI_NOT_INSTRUMENTED, src->alloced_len,
                                    MYF(MY_WME))))
    die("Out of memory");
  else
    dest->alloced_len = src->alloced_len;

  /* Copy str_val data to dest */
  dest->str_val_len = src->str_val_len;
  if (src->str_val_len) memcpy(dest->str_val, src->str_val, src->str_val_len);
}

void eval_expr(VAR *v, const char *p, const char **p_end, bool open_end,
               bool do_eval) {
  DBUG_TRACE;
  if (p_end) {
    DBUG_PRINT("enter", ("p: '%.*s'", (int)(*p_end - p), p));
  } else {
    DBUG_PRINT("enter", ("p: '%s'", p));
  }
  /* Skip to treat as pure string if no evaluation */
  if (!do_eval) goto NO_EVAL;

  if (*p == '$') {
    VAR *vp;
    const char *expected_end = *p_end;  // Remember var end
    if ((vp = var_get(p, p_end, false, false))) var_copy(v, vp);

    /* Apparently it is not safe to assume null-terminated string */
    v->str_val[v->str_val_len] = 0;

    /* Make sure there was just a $variable and nothing else */
    const char *end = *p_end + 1;
    if (end < expected_end && !open_end)
      die("Found junk '%.*s' after $variable in expression",
          (int)(expected_end - end - 1), end);

    return;
  }

  if (*p == '`') {
    var_query_set(v, p, p_end);
    return;
  }

  {
    /* Check if this is a "let $var= query_get_value_by_name()" */
    const char *get_value_str_by_name = "query_get_value_by_name";
    const size_t lenn = std::strlen(get_value_str_by_name);
    if (std::strncmp(p, get_value_str_by_name, lenn) == 0) {
      struct st_command command;
      memset(&command, 0, sizeof(command));
      command.query = const_cast<char *>(p);
      command.first_word_len = lenn;
      command.first_argument = command.query + lenn;
      command.end = const_cast<char *>(*p_end);
      var_set_query_get_value_by_name(&command, v);
      return;
    }
    /* Check if this is a "let $var= query_get_value()" */
    const char *get_value_str = "query_get_value";
    const size_t len = std::strlen(get_value_str);
    if (std::strncmp(p, get_value_str, len) == 0) {
      struct st_command command;
      memset(&command, 0, sizeof(command));
      command.query = const_cast<char *>(p);
      command.first_word_len = len;
      command.first_argument = command.query + len;
      command.end = const_cast<char *>(*p_end);
      var_set_query_get_value(&command, v);
      return;
    }
    /* Check if this is a "let $var= convert_error()" */
    const char *get_value_str1 = "convert_error";
    const size_t len1 = std::strlen(get_value_str1);
    if (std::strncmp(p, get_value_str1, len1) == 0) {
      struct st_command command;
      memset(&command, 0, sizeof(command));
      command.query = const_cast<char *>(p);
      command.first_word_len = len;
      command.first_argument = command.query + len;
      command.end = const_cast<char *>(*p_end);
      var_set_convert_error(&command, v);
      return;
    }
    /* Check if this is a "let $var= get_response_attribute()" */
    const char *get_value_str2 = "get_response_attribute";
    const size_t len2 = std::strlen(get_value_str2);
    if (std::strncmp(p, get_value_str2, len2) == 0) {
      struct st_command command;
      memset(&command, 0, sizeof(command));
      command.query = const_cast<char *>(p);
      command.first_word_len = len2;
      command.first_argument = command.query + len2;
      command.end = const_cast<char *>(*p_end);
      var_set_get_response_attribute(&command, v);
      return;
    }
  }

NO_EVAL : {
  size_t new_val_len =
      (p_end && *p_end) ? static_cast<size_t>(*p_end - p) : std::strlen(p);
  if (new_val_len + 1 >= v->alloced_len) {
    static size_t MIN_VAR_ALLOC = 32;
    v->alloced_len =
        (new_val_len < MIN_VAR_ALLOC - 1) ? MIN_VAR_ALLOC : new_val_len + 1;
    if (!(v->str_val =
              v->str_val ? (char *)my_realloc(PSI_NOT_INSTRUMENTED, v->str_val,
                                              v->alloced_len + 1, MYF(MY_WME))
                         : (char *)my_malloc(PSI_NOT_INSTRUMENTED,
                                             v->alloced_len + 1, MYF(MY_WME))))
      die("Out of memory");
  }
  v->str_val_len = new_val_len;
  memcpy(v->str_val, p, new_val_len);
  v->str_val[new_val_len] = 0;
  var_check_int(v);
}
}

static int open_file(const char *name) {
  char buff[FN_REFLEN];
  size_t length;
  DBUG_TRACE;
  DBUG_PRINT("enter", ("name: %s", name));

  bool file_exists = false;
  /* Extract path from current file and try it as base first */
  if (dirname_part(buff, cur_file->file_name, &length)) {
    strxmov(buff, buff, name, NullS);
    if (access(buff, F_OK) == 0) {
      DBUG_PRINT("info", ("The file exists"));
      name = buff;
      file_exists = true;
    }
  }

  if (!test_if_hard_path(name) && !file_exists) {
    strxmov(buff, opt_basedir, name, NullS);
    name = buff;
  }
  fn_format(buff, name, "", "", MY_UNPACK_FILENAME);

  if (cur_file == file_stack_end) die("Source directives are nesting too deep");
  cur_file++;
  if (!(cur_file->file = fopen(buff, "rb"))) {
    cur_file--;
    die("Could not open '%s' for reading, errno: %d", buff, errno);
  }
  cur_file->file_name = my_strdup(PSI_NOT_INSTRUMENTED, buff, MYF(MY_FAE));
  cur_file->lineno = 1;
  return 0;
}

/*
  Source and execute the given file

  SYNOPSIS
  do_source()
  query	called command

  DESCRIPTION
  source <file_name>

  Open the file <file_name> and execute it

*/

static void do_source(struct st_command *command) {
  static DYNAMIC_STRING ds_filename;
  const struct command_arg source_args[] = {
      {"filename", ARG_STRING, true, &ds_filename, "File to source"}};
  DBUG_TRACE;

  check_command_args(command, command->first_argument, source_args,
                     sizeof(source_args) / sizeof(struct command_arg), ' ');

  /*
    If this file has already been sourced, don't source it again.
    It's already available in the q_lines cache.
  */
  if (parser.current_line < (parser.read_lines - 1))
    ; /* Do nothing */
  else {
    DBUG_PRINT("info", ("sourcing file: %s", ds_filename.str));
    open_file(ds_filename.str);
  }

  dynstr_free(&ds_filename);
}

static FILE *my_popen(DYNAMIC_STRING *ds_cmd, const char *mode,
                      struct st_command *command MY_ATTRIBUTE((unused))) {
#ifdef _WIN32
  /*
    --execw is for tests executing commands containing non-ASCII characters.

    To correctly start such a program on Windows, we need to use the "wide"
    version of popen, with prior translation of the command line from
    the file character set to wide string. We use the current value
    of --character_set as a file character set, so before using --execw
    make sure to set --character_set properly.

    If we use the non-wide version of popen, Windows internally
    converts command line from the current ANSI code page to wide string.
    In case when character set of the command line does not match the
    current ANSI code page, non-ASCII characters get garbled in most cases.

    On Linux, the command line passed to popen() is considered
    as a binary string, no any internal to-wide and from-wide
    character set conversion happens, so we don't need to do anything.
    On Linux --execw is just a synonym to --exec.

    For simplicity, assume that  command line is limited to 4KB
    (like in cmd.exe) and that mode at most 10 characters.
  */
  if (command->type == Q_EXECW) {
    wchar_t wcmd[4096];
    wchar_t wmode[10];
    const char *cmd = ds_cmd->str;
    uint dummy_errors;
    size_t len;
    len = my_convert((char *)wcmd, sizeof(wcmd) - sizeof(wcmd[0]),
                     &my_charset_utf16le_bin, ds_cmd->str,
                     std::strlen(ds_cmd->str), charset_info, &dummy_errors);
    wcmd[len / sizeof(wchar_t)] = 0;
    len = my_convert((char *)wmode, sizeof(wmode) - sizeof(wmode[0]),
                     &my_charset_utf16le_bin, mode, std::strlen(mode),
                     charset_info, &dummy_errors);
    wmode[len / sizeof(wchar_t)] = 0;
    return _wpopen(wcmd, wmode);
  }
#endif /* _WIN32 */

  return popen(ds_cmd->str, mode);
}

static void init_builtin_echo(void) {
#ifdef _WIN32
  size_t echo_length;

  /* Look for "echo.exe" in same dir as mysqltest was started from */
  dirname_part(builtin_echo, my_progname, &echo_length);
  fn_format(builtin_echo, ".\\echo.exe", builtin_echo, "", MYF(MY_REPLACE_DIR));

  /* Make sure echo.exe exists */
  if (access(builtin_echo, F_OK) != 0) builtin_echo[0] = 0;
  return;

#else

  builtin_echo[0] = 0;
  return;

#endif
}

/*
  Replace a substring

  SYNOPSIS
    replace
    ds_str      The string to search and perform the replace in
    search_str  The string to search for
    search_len  Length of the string to search for
    replace_str The string to replace with
    replace_len Length of the string to replace with

  RETURN
    0 String replaced
    1 Could not find search_str in str
*/

static int replace(DYNAMIC_STRING *ds_str, const char *search_str,
                   size_t search_len, const char *replace_str,
                   size_t replace_len) {
  DYNAMIC_STRING ds_tmp;
  const char *start = strstr(ds_str->str, search_str);
  if (!start) return 1;
  init_dynamic_string(&ds_tmp, "", ds_str->length + replace_len, 256);
  dynstr_append_mem(&ds_tmp, ds_str->str, start - ds_str->str);
  dynstr_append_mem(&ds_tmp, replace_str, replace_len);
  dynstr_append(&ds_tmp, start + search_len);
  dynstr_set(ds_str, ds_tmp.str);
  dynstr_free(&ds_tmp);
  return 0;
}

#ifdef _WIN32
/**
 Replace CRLF sequence with LF in place.

 This function is required as a workaround for a bug in the Microsoft
 C runtime library introduced in Visual Studio 2015.
 See bug#22608247 and bug#22811243

 @param buf  Null terminated buffer.
*/
static void replace_crlf_with_lf(char *buf) {
  char *replace = buf;
  while (*buf) {
    *replace = *buf++;
    if (!((*replace == '\x0D') && (*buf == '\x0A'))) {
      replace++;
    }
  }
  *replace = '\x0';
}
#endif

/// Execute the shell command using the popen() library call. References
/// to variables within the command are replaced with the corresponding
/// values. Use “\\$” to specify a literal “$” character.
///
/// The error code returned from the subprocess is checked against the
/// expected error array, previously set with the --error command. It can
/// thus be used to execute a command that shall fail.
///
/// @code
/// exec command [args]
/// @endcode
///
/// @param command Pointer to the st_command structure which holds the
///                arguments and information for the command.
/// @param run_in_background Specifies if command should be run in background.
///                          In such case we don't wait nor attempt to read the
///                          output.
///
/// @note
/// It is recommended to use mysqltest command(s) like "remove_file"
/// instead of executing the shell commands using 'exec' command.
static void do_exec(struct st_command *command, bool run_in_background) {
  DBUG_TRACE;

  const char *cmd = command->first_argument;
  DBUG_PRINT("enter", ("cmd: '%s'", cmd));

  // Skip leading space
  while (*cmd && my_isspace(charset_info, *cmd)) cmd++;
  if (!*cmd) die("Missing argument in exec");
  command->last_argument = command->end;

  DYNAMIC_STRING ds_cmd;
  init_dynamic_string(&ds_cmd, nullptr, command->query_len + 256, 256);

  // Eval the command, thus replacing all environment variables
  do_eval(&ds_cmd, cmd, command->end, !is_windows);

  // Check if echo should be replaced with "builtin" echo
  if (builtin_echo[0] && std::strncmp(cmd, "echo", 4) == 0) {
    // Replace echo with our "builtin" echo
    replace(&ds_cmd, "echo", 4, builtin_echo, std::strlen(builtin_echo));
  }

#ifdef _WIN32
  // Replace "/dev/null" with NUL
  while (replace(&ds_cmd, "/dev/null", 9, "NUL", 3) == 0)
    ;

  // Replace "closed stdout" with non existing output fd
  while (replace(&ds_cmd, ">&-", 3, ">&4", 3) == 0)
    ;
#endif

  if (run_in_background) {
    /* Add an invocation of "START /B" on Windows, append " &" on Linux*/
    DYNAMIC_STRING ds_tmp;
#ifdef WIN32
    init_dynamic_string(&ds_tmp, "START /B ", ds_cmd.length + 9, 256);
    dynstr_append_mem(&ds_tmp, ds_cmd.str, ds_cmd.length);
#else
    init_dynamic_string(&ds_tmp, ds_cmd.str, ds_cmd.length + 2, 256);
    dynstr_append_mem(&ds_tmp, " &", 2);
#endif
    dynstr_set(&ds_cmd, ds_tmp.str);
    dynstr_free(&ds_tmp);
  }

  // exec command is interpreted externally and will not take newlines
  while (replace(&ds_cmd, "\n", 1, " ", 1) == 0)
    ;

  DBUG_PRINT("info",
             ("Executing '%s' as '%s'", command->first_argument, ds_cmd.str));

#ifdef WIN32
  // Open pipe in binary mode as part of handling Microsoft _read bug.
  // See bug#22608247 and bug#22811243
  const char *mode = "rb";
#else
  const char *mode = "r";
#endif
  FILE *res_file;
  if (!(res_file = my_popen(&ds_cmd, mode, command)) &&
      command->abort_on_error) {
    dynstr_free(&ds_cmd);
    die("popen(\"%s\", \"r\") failed", command->first_argument);
  }

  if (!run_in_background) {
    char buf[512];
    std::string str;
    while (std::fgets(buf, sizeof(buf), res_file)) {
      if (std::strlen(buf) < 1) continue;

#ifdef WIN32
      // Replace CRLF char with LF.
      // See bug#22608247 and bug#22811243
      DBUG_ASSERT(!std::strcmp(mode, "rb"));
      replace_crlf_with_lf(buf);
#endif
      if (trace_exec) {
        fprintf(stdout, "%s", buf);
        fflush(stdout);
      }
      if (disable_result_log) {
        buf[std::strlen(buf) - 1] = 0;
        DBUG_PRINT("exec_result", ("%s", buf));
      } else {
        // Read the file line by line. Check if the buffer read from the
        // file ends with EOL character.
        if ((buf[std::strlen(buf) - 1] != '\n' &&
             std::strlen(buf) < (sizeof(buf) - 1)) ||
            (buf[std::strlen(buf) - 1] == '\n')) {
          // Found EOL
          if (str.length()) {
            // Temporary string exists, append the current buffer read
            // to the temporary string.
            str.append(buf);
            replace_dynstr_append(&ds_res, str.c_str());
            str.clear();
          } else {
            // Entire line is read at once
            replace_dynstr_append(&ds_res, buf);
          }
        } else {
          // The buffer read from the file doesn't end with EOL character,
          // store it in a temporary string.
          str.append(buf);
        }
      }
    }
  }

  std::uint32_t status = 0;
  int error = pclose(res_file);

  if (error > 0) {
#ifdef _WIN32
    status = WEXITSTATUS(error);
#else
    // Do the same as many shells here: show SIGKILL as 137
    if (WIFEXITED(error))
      status = WEXITSTATUS(error);
    else if (WIFSIGNALED(error))
      status = 0x80 + WTERMSIG(error);
#endif
  }

  if (error != 0 && command->abort_on_error) {
    log_msg("exec of '%s' failed, error: %d, status: %d, errno: %d.",
            ds_cmd.str, error, status, errno);
    dynstr_free(&ds_cmd);
    die("Command \"%s\" failed.\n\nOutput from before failure:\n%s",
        command->first_argument, ds_res.str);
  }

  dynstr_free(&ds_cmd);
  handle_command_error(command, status);

  // Save error code
  save_error_code(error);
}

enum enum_operator { DO_DEC, DO_INC };

/// Template function that frees memory of the dynamic string
/// passed to the function.
///
/// @param val Dynamic string whose memory needs to be freed.
template <typename T>
static void free_dynamic_strings(T *val) {
  dynstr_free(val);
}

/// Frees the memory of dynamic strings passed to the function.
/// It accepts a variable number of dynamic strings, and through
/// recursion, frees the memory. The other template function
/// which calls dynstr_free() is called here.
///
/// @param first The dynamic string passed to the function which
///              gets freed using dynstr_free().
/// @param rest  Rest of the dynamic strings which are passed to
///              the function, through recursion, end up being
///              freed by dynstr_free().
template <typename T1, typename... T2>
static void free_dynamic_strings(T1 *first, T2 *... rest) {
  free_dynamic_strings(first);
  free_dynamic_strings(rest...);
}

/*
  Decrease or increase the value of a variable

  SYNOPSIS
  do_modify_var()
  query	called command
  op    operation to perform on the var

  DESCRIPTION
  dec $var_name
  inc $var_name

*/

static int do_modify_var(struct st_command *command, enum enum_operator op) {
  const char *p = command->first_argument;
  VAR *v;
  if (!*p)
    die("Missing argument to %.*s", static_cast<int>(command->first_word_len),
        command->query);
  if (*p != '$')
    die("The argument to %.*s must be a variable (start with $)",
        static_cast<int>(command->first_word_len), command->query);
  v = var_get(p, &p, true, false);
  if (!v->is_int) die("Cannot perform inc/dec on a non-numeric value");
  switch (op) {
    case DO_DEC:
      v->int_val--;
      break;
    case DO_INC:
      v->int_val++;
      break;
    default:
      die("Invalid operator to do_modify_var");
      break;
  }
  v->int_dirty = true;
  command->last_argument = const_cast<char *>(++p);
  return 0;
}

/// Removes the file passed as the argument and retries a specified
/// number of times, if it is unsuccessful.
///
/// @param command Pointer to the st_command structure which holds the
///                arguments and information for the command.
static void do_remove_file(struct st_command *command) {
  int error;
  static DYNAMIC_STRING ds_filename;
  static DYNAMIC_STRING ds_retry;

  const struct command_arg rm_args[] = {
      {"filename", ARG_STRING, true, &ds_filename, "File to delete"},
      {"retry", ARG_STRING, false, &ds_retry, "Number of retries"}};
  DBUG_TRACE;

  check_command_args(command, command->first_argument, rm_args,
                     sizeof(rm_args) / sizeof(struct command_arg), ' ');

  // Check if the retry value is passed, and if it is an integer
  int retry = 0;
  if (ds_retry.length) {
    retry = get_int_val(ds_retry.str);
    if (retry < 0) {
      // In case of invalid retry, copy the value passed to print later
      char buf[32];
      strmake(buf, ds_retry.str, sizeof(buf) - 1);
      free_dynamic_strings(&ds_filename, &ds_retry);
      die("Invalid value '%s' for retry argument given to remove_file "
          "command.",
          buf);
    }
  }

  DBUG_PRINT("info", ("removing file: %s", ds_filename.str));
  error = my_delete(ds_filename.str, MYF(0)) != 0;

  /*
    If the remove command fails due to an environmental issue, the command can
    be retried a specified number of times before throwing an error.
  */
  for (int i = 0; error && (i < retry); i++) {
    my_sleep(1000 * 1000);
    error = my_delete(ds_filename.str, MYF(0)) != 0;
  }

  handle_command_error(command, error);
  free_dynamic_strings(&ds_filename, &ds_retry);
}

/// Removes the files in the specified directory, by matching the
/// file name pattern. Retry of the command can happen optionally with
/// an interval of one second between each retry if the command fails.
///
/// @param command Pointer to the st_command structure which holds the
///                arguments and information for the command.
static void do_remove_files_wildcard(struct st_command *command) {
  int error = 0;
  uint i;
  MY_DIR *dir_info;
  FILEINFO *file;
  char dir_separator[2];
  static DYNAMIC_STRING ds_directory;
  static DYNAMIC_STRING ds_wild;
  static DYNAMIC_STRING ds_retry;
  char dirname[FN_REFLEN];

  const struct command_arg rm_args[] = {
      {"directory", ARG_STRING, true, &ds_directory,
       "Directory containing files to delete"},
      {"pattern", ARG_STRING, true, &ds_wild, "File pattern to delete"},
      {"retry", ARG_STRING, false, &ds_retry, "Number of retries"}};
  DBUG_TRACE;

  check_command_args(command, command->first_argument, rm_args,
                     sizeof(rm_args) / sizeof(struct command_arg), ' ');
  fn_format(dirname, ds_directory.str, "", "", MY_UNPACK_FILENAME);

  // Check if the retry value is passed, and if it is an interger
  int retry = 0;
  if (ds_retry.length) {
    retry = get_int_val(ds_retry.str);
    if (retry < 0) {
      // In case of invalid retry, copy the value passed to print later
      char buf[32];
      strmake(buf, ds_retry.str, sizeof(buf) - 1);
      free_dynamic_strings(&ds_directory, &ds_wild, &ds_retry);
      die("Invalid value '%s' for retry argument given to "
          "remove_files_wildcard command.",
          buf);
    }
  }

  static DYNAMIC_STRING ds_file_to_remove;
  DBUG_PRINT("info", ("listing directory: %s", dirname));
  /* Note that my_dir sorts the list if not given any flags */
  if (!(dir_info = my_dir(dirname, MYF(MY_DONT_SORT | MY_WANT_STAT)))) {
    error = 1;
    goto end;
  }
  init_dynamic_string(&ds_file_to_remove, dirname, 1024, 1024);
  dir_separator[0] = FN_LIBCHAR;
  dir_separator[1] = 0;
  dynstr_append(&ds_file_to_remove, dir_separator);

  size_t length;
  /* Storing the length of the path to the file, so it can be reused */
  length = ds_file_to_remove.length;
  for (i = 0; i < (uint)dir_info->number_off_files; i++) {
    ds_file_to_remove.length = length;
    file = dir_info->dir_entry + i;
    /* Remove only regular files, i.e. no directories etc. */
    /* if (!MY_S_ISREG(file->mystat->st_mode)) */
    /* MY_S_ISREG does not work here on Windows, just skip directories */
    if (MY_S_ISDIR(file->mystat->st_mode)) continue;
    if (wild_compare_full(file->name, std::strlen(file->name), ds_wild.str,
                          std::strlen(ds_wild.str), false, 0, '?', '*'))
      continue;
    /* Not required as the var ds_file_to_remove.length already has the
       length in canonnicalized form */
    /* ds_file_to_remove.length= ds_directory.length + 1;
    ds_file_to_remove.str[ds_directory.length + 1]= 0; */
    dynstr_append(&ds_file_to_remove, file->name);
    DBUG_PRINT("info", ("removing file: %s", ds_file_to_remove.str));
    error = my_delete(ds_file_to_remove.str, MYF(0)) != 0;

    /*
      If the remove command fails due to an environmental issue, the command
      can be retried a specified number of times before throwing an error.
    */
    for (int j = 0; error && (j < retry); j++) {
      my_sleep(1000 * 1000);
      error = my_delete(ds_file_to_remove.str, MYF(0)) != 0;
    }
    if (error) break;
  }
  my_dirend(dir_info);

end:
  handle_command_error(command, error);
  free_dynamic_strings(&ds_directory, &ds_wild, &ds_file_to_remove, &ds_retry);
}

/// Copy the source file to destination file. Copy will fail if the
/// destination file exists. Retry of the command can happen optionally with
/// an interval of one second between each retry if the command fails.
///
/// @param command Pointer to the st_command structure which holds the
///                arguments and information for the command.
static void do_copy_file(struct st_command *command) {
  int error;
  static DYNAMIC_STRING ds_from_file;
  static DYNAMIC_STRING ds_to_file;
  static DYNAMIC_STRING ds_retry;

  const struct command_arg copy_file_args[] = {
      {"from_file", ARG_STRING, true, &ds_from_file, "Filename to copy from"},
      {"to_file", ARG_STRING, true, &ds_to_file, "Filename to copy to"},
      {"retry", ARG_STRING, false, &ds_retry, "Number of retries"}};
  DBUG_TRACE;

  check_command_args(command, command->first_argument, copy_file_args,
                     sizeof(copy_file_args) / sizeof(struct command_arg), ' ');

  // Check if the retry value is passed, and if it is an interger
  int retry = 0;
  if (ds_retry.length) {
    retry = get_int_val(ds_retry.str);
    if (retry < 0) {
      // In case of invalid retry, copy the value passed to print later
      char buf[32];
      strmake(buf, ds_retry.str, sizeof(buf) - 1);
      free_dynamic_strings(&ds_from_file, &ds_to_file, &ds_retry);
      die("Invalid value '%s' for retry argument given to copy_file "
          "command.",
          buf);
    }
  }

  DBUG_PRINT("info", ("Copy %s to %s", ds_from_file.str, ds_to_file.str));
  /* MY_HOLD_ORIGINAL_MODES prevents attempts to chown the file */
  error = (my_copy(ds_from_file.str, ds_to_file.str,
                   MYF(MY_DONT_OVERWRITE_FILE | MY_HOLD_ORIGINAL_MODES)) != 0);

  /*
    If the copy command fails due to an environmental issue, the command can
    be retried a specified number of times before throwing an error.
  */
  for (int i = 0; error && (i < retry); i++) {
    my_sleep(1000 * 1000);
    error =
        (my_copy(ds_from_file.str, ds_to_file.str,
                 MYF(MY_DONT_OVERWRITE_FILE | MY_HOLD_ORIGINAL_MODES)) != 0);
  }

  handle_command_error(command, error);
  free_dynamic_strings(&ds_from_file, &ds_to_file, &ds_retry);
}

/*
  SYNOPSIS
  recursive_copy
  ds_source      - pointer to dynamic string containing source
                   directory informtion
  ds_destination - pointer to dynamic string containing destination
                   directory informtion

  DESCRIPTION
  Recursive copy of <ds_source> to <ds_destination>
*/

static int recursive_copy(DYNAMIC_STRING *ds_source,
                          DYNAMIC_STRING *ds_destination) {
  /* Note that my_dir sorts the list if not given any flags */
  MY_DIR *src_dir_info =
      my_dir(ds_source->str, MYF(MY_DONT_SORT | MY_WANT_STAT));

  int error = 0;

  /* Source directory exists */
  if (src_dir_info) {
    /* Note that my_dir sorts the list if not given any flags */
    MY_DIR *dest_dir_info =
        my_dir(ds_destination->str, MYF(MY_DONT_SORT | MY_WANT_STAT));

    /* Create destination directory if it doesn't exist */
    if (!dest_dir_info) {
      error = my_mkdir(ds_destination->str, 0777, MYF(0)) != 0;
      if (error) {
        my_dirend(dest_dir_info);
        goto end;
      }
    } else {
      /* Extracting the source directory name */
      if (ds_source->str[std::strlen(ds_source->str) - 1] == '/') {
        strmake(ds_source->str, ds_source->str,
                std::strlen(ds_source->str) - 1);
        ds_source->length = ds_source->length - 1;
      }
      char *src_dir_name = strrchr(ds_source->str, '/');

      /* Extracting the destination directory name */
      if (ds_destination->str[std::strlen(ds_destination->str) - 1] == '/') {
        strmake(ds_destination->str, ds_destination->str,
                std::strlen(ds_destination->str) - 1);
        ds_destination->length = ds_destination->length - 1;
      }
      char *dest_dir_name = strrchr(ds_destination->str, '/');

      /*
        Destination directory might not exist if source directory
        name and destination directory name are not same.

        For example, if source is "abc" and destintion is "def",
        check for the existance of directory "def/abc". If it exists
        then, copy the files from source directory(i.e "abc") to
        destination directory(i.e "def/abc"), otherwise create a new
        directory "abc" under "def" and copy the files from source to
        destination directory.
      */
      if (std::strcmp(src_dir_name, dest_dir_name)) {
        dynstr_append(ds_destination, src_dir_name);
        my_dirend(dest_dir_info);
        dest_dir_info =
            my_dir(ds_destination->str, MYF(MY_DONT_SORT | MY_WANT_STAT));

        /* Create destination directory if it doesn't exist */
        if (!dest_dir_info) {
          error = my_mkdir(ds_destination->str, 0777, MYF(0)) != 0;
          if (error) {
            my_dirend(dest_dir_info);
            goto end;
          }
        }
      }
    }

    char dir_separator[2] = {FN_LIBCHAR, 0};
    dynstr_append(ds_source, dir_separator);
    dynstr_append(ds_destination, dir_separator);

    /*
      Storing the length of source and destination
      directory paths so it can be reused.
    */
    size_t source_dir_length = ds_source->length;
    size_t destination_dir_length = ds_destination->length;
    ;

    for (uint i = 0; i < src_dir_info->number_off_files; i++) {
      ds_source->length = source_dir_length;
      ds_destination->length = destination_dir_length;
      FILEINFO *file = src_dir_info->dir_entry + i;

      /* Skip the names "." and ".." */
      if (!std::strcmp(file->name, ".") || !std::strcmp(file->name, ".."))
        continue;

      dynstr_append(ds_source, file->name);
      dynstr_append(ds_destination, file->name);

      if (MY_S_ISDIR(file->mystat->st_mode))
        error = (recursive_copy(ds_source, ds_destination) != 0) ? 1 : error;
      else {
        DBUG_PRINT("info", ("Copying file: %s to %s", ds_source->str,
                            ds_destination->str));

        /* MY_HOLD_ORIGINAL_MODES prevents attempts to chown the file */
        error = (my_copy(ds_source->str, ds_destination->str,
                         MYF(MY_HOLD_ORIGINAL_MODES)) != 0)
                    ? 1
                    : error;
      }
    }
    my_dirend(dest_dir_info);
  }
  /* Source directory does not exist or access denied */
  else
    error = 1;

end:
  my_dirend(src_dir_info);
  return error;
}

/*
  SYNOPSIS
  do_force_cpdir
  command    - command handle

  DESCRIPTION
  force-cpdir <from_directory> <to_directory>
  Recursive copy of <from_directory> to <to_directory>.
  Destination directory is created if it doesn't exist.

  NOTE
  Will fail if  <from_directory> doesn't exist.
*/

static void do_force_cpdir(struct st_command *command) {
  DBUG_TRACE;

  static DYNAMIC_STRING ds_source;
  static DYNAMIC_STRING ds_destination;

  const struct command_arg copy_file_args[] = {
      {"from_directory", ARG_STRING, true, &ds_source,
       "Directory to copy from"},
      {"to_directory", ARG_STRING, true, &ds_destination,
       "Directory to copy to"}};

  check_command_args(command, command->first_argument, copy_file_args,
                     sizeof(copy_file_args) / sizeof(struct command_arg), ' ');

  DBUG_PRINT("info", ("Recursive copy files of %s to %s", ds_source.str,
                      ds_destination.str));

  DBUG_PRINT("info", ("listing directory: %s", ds_source.str));

  int error = 0;

  /*
    Throw an error if source directory path and
    destination directory path are same.
  */
  if (!std::strcmp(ds_source.str, ds_destination.str)) {
    error = 1;
    set_my_errno(EEXIST);
  } else
    error = recursive_copy(&ds_source, &ds_destination);

  handle_command_error(command, error);
  dynstr_free(&ds_source);
  dynstr_free(&ds_destination);
}

/// Copy files from source directory to destination directory, by matching
/// a specified file name pattern.
///
/// Copy will fail if no files match the pattern. It will fail if source
/// directory is empty and/or there are no files in it. Copy will also
/// fail if source directory or destination directory or both do not
/// exist. Retry of the command can happen optionally with an interval of
/// one second between each retry if the command fails.
///
/// @param command Pointer to the st_command structure which holds the
///                arguments and information for the command.
static void do_copy_files_wildcard(struct st_command *command) {
  static DYNAMIC_STRING ds_source;
  static DYNAMIC_STRING ds_destination;
  static DYNAMIC_STRING ds_wild;
  static DYNAMIC_STRING ds_retry;

  const struct command_arg copy_file_args[] = {
      {"from_directory", ARG_STRING, true, &ds_source,
       "Directory to copy from"},
      {"to_directory", ARG_STRING, true, &ds_destination,
       "Directory to copy to"},
      {"pattern", ARG_STRING, true, &ds_wild, "File name pattern"},
      {"retry", ARG_STRING, false, &ds_retry, "Number of retries"}};
  DBUG_TRACE;

  check_command_args(command, command->first_argument, copy_file_args,
                     sizeof(copy_file_args) / sizeof(struct command_arg), ' ');

  DBUG_PRINT("info",
             ("Copy files of %s to %s", ds_source.str, ds_destination.str));

  DBUG_PRINT("info", ("listing directory: %s", ds_source.str));

  int error = 0;

  // Check if the retry value is passed, and if it is an integer
  int retry = 0;
  if (ds_retry.length) {
    retry = get_int_val(ds_retry.str);
    if (retry < 0) {
      // In case of invalid retry, copy the value passed to print later
      char buf[32];
      strmake(buf, ds_retry.str, sizeof(buf) - 1);
      free_dynamic_strings(&ds_source, &ds_destination, &ds_wild, &ds_retry);
      die("Invalid value '%s' for retry argument given to "
          "copy_files_wildcard command.",
          buf);
    }
  }

  /* Note that my_dir sorts the list if not given any flags */
  MY_DIR *dir_info = my_dir(ds_source.str, MYF(MY_DONT_SORT | MY_WANT_STAT));

  /* Directory does not exist or access denied */
  if (!dir_info) {
    error = 1;
    goto end;
  }

  /* The directory exists but is empty */
  if (dir_info->number_off_files == 2) {
    error = 1;
    set_my_errno(ENOENT);
    goto end;
  }

  char dir_separator[2];
  dir_separator[0] = FN_LIBCHAR;
  dir_separator[1] = 0;
  dynstr_append(&ds_source, dir_separator);
  dynstr_append(&ds_destination, dir_separator);

  /* Storing the length of the path to the file, so it can be reused */
  size_t source_file_length;
  size_t dest_file_length;
  dest_file_length = ds_destination.length;
  source_file_length = ds_source.length;
  uint match_count;
  match_count = 0;

  for (uint i = 0; i < dir_info->number_off_files; i++) {
    ds_source.length = source_file_length;
    ds_destination.length = dest_file_length;
    FILEINFO *file = dir_info->dir_entry + i;

    /*
      Copy only regular files, i.e. no directories etc.
      if (!MY_S_ISREG(file->mystat->st_mode))
      MY_S_ISREG does not work here on Windows, just skip directories
    */
    if (MY_S_ISDIR(file->mystat->st_mode)) continue;

    /* Copy only those files which the pattern matches */
    if (wild_compare_full(file->name, std::strlen(file->name), ds_wild.str,
                          std::strlen(ds_wild.str), false, 0, '?', '*'))
      continue;

    match_count++;
    dynstr_append(&ds_source, file->name);
    dynstr_append(&ds_destination, file->name);
    DBUG_PRINT("info",
               ("Copying file: %s to %s", ds_source.str, ds_destination.str));

    /* MY_HOLD_ORIGINAL_MODES prevents attempts to chown the file */
    error = (my_copy(ds_source.str, ds_destination.str,
                     MYF(MY_HOLD_ORIGINAL_MODES)) != 0);

    /*
      If the copy command fails due to an environmental issue, the command can
      be retried a specified number of times before throwing an error.
    */
    for (int j = 0; error && (j < retry); j++) {
      my_sleep(1000 * 1000);
      error =
          (my_copy(ds_source.str, ds_destination.str,
                   MYF(MY_DONT_OVERWRITE_FILE | MY_HOLD_ORIGINAL_MODES)) != 0);
    }

    if (error) goto end;
  }

  /* Pattern did not match any files */
  if (!match_count) {
    error = 1;
    set_my_errno(ENOENT);
  }

end:
  my_dirend(dir_info);
  handle_command_error(command, error);
  free_dynamic_strings(&ds_source, &ds_destination, &ds_wild, &ds_retry);
}

/*
  SYNOPSIS
  move_file_by_copy_delete
  from  path of source
  to    path of destination

  DESCRIPTION
  Move <from_file> to <to_file>
  Auxiliary function for copying <from_file> to <to_file> followed by
  deleting <to_file>.
*/

static int move_file_by_copy_delete(const char *from, const char *to) {
  int error_copy, error_delete;
  error_copy = (my_copy(from, to, MYF(MY_HOLD_ORIGINAL_MODES)) != 0);
  if (error_copy) {
    return error_copy;
  }

  error_delete = my_delete(from, MYF(0)) != 0;

  /*
    If deleting the source file fails, rollback by deleting the
    redundant copy at the destinatiion.
  */
  if (error_delete) {
    my_delete(to, MYF(0));
  }
  return error_delete;
}

/// Moves a file to destination file. Retry of the command can happen
/// optionally with an interval of one second between each retry if
/// the command fails.
///
/// @param command Pointer to the st_command structure which holds the
///                arguments and information for the command.
static void do_move_file(struct st_command *command) {
  int error;
  static DYNAMIC_STRING ds_from_file;
  static DYNAMIC_STRING ds_to_file;
  static DYNAMIC_STRING ds_retry;

  const struct command_arg move_file_args[] = {
      {"from_file", ARG_STRING, true, &ds_from_file, "Filename to move from"},
      {"to_file", ARG_STRING, true, &ds_to_file, "Filename to move to"},
      {"retry", ARG_STRING, false, &ds_retry, "Number of retries"}};
  DBUG_TRACE;

  check_command_args(command, command->first_argument, move_file_args,
                     sizeof(move_file_args) / sizeof(struct command_arg), ' ');

  // Check if the retry value is passed, and if it is an interger
  int retry = 0;
  if (ds_retry.length) {
    retry = get_int_val(ds_retry.str);
    if (retry < 0) {
      // In case of invalid retry, copy the value passed to print later
      char buf[32];
      strmake(buf, ds_retry.str, sizeof(buf) - 1);
      free_dynamic_strings(&ds_from_file, &ds_to_file, &ds_retry);
      die("Invalid value '%s' for retry argument given to move_file "
          "command.",
          buf);
    }
  }

  DBUG_PRINT("info", ("Move %s to %s", ds_from_file.str, ds_to_file.str));
  error = (my_rename(ds_from_file.str, ds_to_file.str, MYF(0)) != 0);

  /*
    Use my_copy() followed by my_delete() for moving a file instead of
    my_rename() when my_errno is EXDEV. This is because my_rename() fails
    with the error "Invalid cross-device link" while moving a file between
    locations having different filesystems in some operating systems.
  */
  if (error && (my_errno() == EXDEV)) {
    error = move_file_by_copy_delete(ds_from_file.str, ds_to_file.str);
  }

  /*
    If the command fails due to an environmental issue, the command can be
    retried a specified number of times before throwing an error.
  */
  for (int i = 0; error && (i < retry); i++) {
    my_sleep(1000 * 1000);
    error = (my_rename(ds_from_file.str, ds_to_file.str, MYF(0)) != 0);

    if (error && (my_errno() == EXDEV))
      error = move_file_by_copy_delete(ds_from_file.str, ds_to_file.str);
  }

  handle_command_error(command, error);
  free_dynamic_strings(&ds_from_file, &ds_to_file, &ds_retry);
}

/*
  SYNOPSIS
  do_chmod_file
  command	command handle

  DESCRIPTION
  chmod <octal> <file_name>
  Change file permission of <file_name>

*/

static void do_chmod_file(struct st_command *command) {
  long mode = 0;
  int err_code;
  static DYNAMIC_STRING ds_mode;
  static DYNAMIC_STRING ds_file;
  const struct command_arg chmod_file_args[] = {
      {"mode", ARG_STRING, true, &ds_mode, "Mode of file(octal) ex. 0660"},
      {"filename", ARG_STRING, true, &ds_file, "Filename of file to modify"}};
  DBUG_TRACE;

  check_command_args(command, command->first_argument, chmod_file_args,
                     sizeof(chmod_file_args) / sizeof(struct command_arg), ' ');

  /* Parse what mode to set */
  if (ds_mode.length != 4 ||
      str2int(ds_mode.str, 8, 0, INT_MAX, &mode) == NullS)
    die("You must write a 4 digit octal number for mode");

  DBUG_PRINT("info", ("chmod %o %s", (uint)mode, ds_file.str));
  err_code = chmod(ds_file.str, mode);
  if (err_code < 0) err_code = 1;
  handle_command_error(command, err_code);
  dynstr_free(&ds_mode);
  dynstr_free(&ds_file);
}

/// Check if specified file exists. Retry of the command can happen
/// optionally with an interval of one second between each retry if
/// the command fails.
///
/// @param command Pointer to the st_command structure which holds the
///                arguments and information for the command.
static void do_file_exist(struct st_command *command) {
  int error;
  static DYNAMIC_STRING ds_filename;
  static DYNAMIC_STRING ds_retry;

  const struct command_arg file_exist_args[] = {
      {"filename", ARG_STRING, true, &ds_filename, "File to check if it exist"},
      {"retry", ARG_STRING, false, &ds_retry, "Number of retries"}};
  DBUG_TRACE;

  check_command_args(command, command->first_argument, file_exist_args,
                     sizeof(file_exist_args) / sizeof(struct command_arg), ' ');

  // Check if the retry value is passed, and if it is an interger
  int retry = 0;
  if (ds_retry.length) {
    retry = get_int_val(ds_retry.str);
    if (retry < 0) {
      // In case of invalid retry, copy the value passed to print later
      char buf[32];
      strmake(buf, ds_retry.str, sizeof(buf) - 1);
      free_dynamic_strings(&ds_filename, &ds_retry);
      die("Invalid value '%s' for retry argument given to file_exists "
          "command.",
          buf);
    }
  }

  DBUG_PRINT("info", ("Checking for existence of file: %s", ds_filename.str));
  error = (access(ds_filename.str, F_OK) != 0);

  /*
    If the file_exists command fails due to an environmental issue, the command
    can be retried a specified number of times before throwing an error.
  */
  for (int i = 0; error && (i < retry); i++) {
    my_sleep(1000 * 1000);
    error = (access(ds_filename.str, F_OK) != 0);
  }

  handle_command_error(command, error);
  free_dynamic_strings(&ds_filename, &ds_retry);
}

/*
  SYNOPSIS
  do_mkdir
  command	called command

  DESCRIPTION
  mkdir <dir_name>
  Create the directory <dir_name>
*/

static void do_mkdir(struct st_command *command) {
  int error;
  static DYNAMIC_STRING ds_dirname;
  const struct command_arg mkdir_args[] = {
      {"dirname", ARG_STRING, true, &ds_dirname, "Directory to create"}};
  DBUG_TRACE;

  check_command_args(command, command->first_argument, mkdir_args,
                     sizeof(mkdir_args) / sizeof(struct command_arg), ' ');

  DBUG_PRINT("info", ("creating directory: %s", ds_dirname.str));
  error = my_mkdir(ds_dirname.str, 0777, MYF(0)) != 0;
  handle_command_error(command, error);
  dynstr_free(&ds_dirname);
}

/*
  SYNOPSIS
  do_force_rmdir
  command    - command handle
  ds_dirname - pointer to dynamic string containing directory informtion

  DESCRIPTION
  force-rmdir <dir_name>
  Remove the directory <dir_name>
*/

void do_force_rmdir(struct st_command *command, DYNAMIC_STRING *ds_dirname) {
  DBUG_TRACE;

  char dir_name[FN_REFLEN + 1];
  strncpy(dir_name, ds_dirname->str, sizeof(dir_name) - 1);
  dir_name[FN_REFLEN] = '\0';

  /* Note that my_dir sorts the list if not given any flags */
  MY_DIR *dir_info = my_dir(ds_dirname->str, MYF(MY_DONT_SORT | MY_WANT_STAT));

  if (dir_info && dir_info->number_off_files > 2) {
    /* Storing the length of the path to the file, so it can be reused */
    size_t length = ds_dirname->length;

    /* Delete the directory recursively */
    for (uint i = 0; i < dir_info->number_off_files; i++) {
      FILEINFO *file = dir_info->dir_entry + i;

      /* Skip the names "." and ".." */
      if (!std::strcmp(file->name, ".") || !std::strcmp(file->name, ".."))
        continue;

      ds_dirname->length = length;
      char dir_separator[2] = {FN_LIBCHAR, 0};
      dynstr_append(ds_dirname, dir_separator);
      dynstr_append(ds_dirname, file->name);

      if (MY_S_ISDIR(file->mystat->st_mode)) /* It's a directory */
        do_force_rmdir(command, ds_dirname);
      else
        /* It's a file */
        my_delete(ds_dirname->str, MYF(0));
    }
  }

  my_dirend(dir_info);
  int error = rmdir(dir_name) != 0;
  set_my_errno(errno);
  handle_command_error(command, error);
}

/*
  SYNOPSIS
  do_rmdir
  command	called command
  force         Recursively delete a directory if the value is set to true,
                otherwise delete an empty direcory

  DESCRIPTION
  rmdir <dir_name>
  Remove the empty directory <dir_name>
*/

static void do_rmdir(struct st_command *command, bool force) {
  int error;
  static DYNAMIC_STRING ds_dirname;
  const struct command_arg rmdir_args[] = {
      {"dirname", ARG_STRING, true, &ds_dirname, "Directory to remove"}};
  DBUG_TRACE;

  check_command_args(command, command->first_argument, rmdir_args,
                     sizeof(rmdir_args) / sizeof(struct command_arg), ' ');

  DBUG_PRINT("info", ("removing directory: %s", ds_dirname.str));
  if (force)
    do_force_rmdir(command, &ds_dirname);
  else {
    error = rmdir(ds_dirname.str) != 0;
    set_my_errno(errno);
    handle_command_error(command, error);
  }
  dynstr_free(&ds_dirname);
}

/*
  SYNOPSIS
  get_list_files
  ds          output
  ds_dirname  dir to list
  ds_wild     wild-card file pattern (can be empty)

  DESCRIPTION
  list all entries in directory (matching ds_wild if given)
*/

static int get_list_files(DYNAMIC_STRING *ds, const DYNAMIC_STRING *ds_dirname,
                          const DYNAMIC_STRING *ds_wild) {
  uint i;
  MY_DIR *dir_info;
  FILEINFO *file;
  DBUG_TRACE;

  DBUG_PRINT("info", ("listing directory: %s", ds_dirname->str));
  /* Note that my_dir sorts the list if not given any flags */
  if (!(dir_info = my_dir(ds_dirname->str, MYF(0)))) return 1;
  for (i = 0; i < (uint)dir_info->number_off_files; i++) {
    file = dir_info->dir_entry + i;
    if (file->name[0] == '.' &&
        (file->name[1] == '\0' ||
         (file->name[1] == '.' && file->name[2] == '\0')))
      continue; /* . or .. */
    if (ds_wild && ds_wild->length &&
        wild_compare_full(file->name, std::strlen(file->name), ds_wild->str,
                          std::strlen(ds_wild->str), false, 0, '?', '*'))
      continue;
    replace_dynstr_append(ds, file->name);
    dynstr_append(ds, "\n");
  }
  my_dirend(dir_info);
  return 0;
}

/*
  SYNOPSIS
  do_list_files
  command	called command

  DESCRIPTION
  list_files <dir_name> [<file_name>]
  List files and directories in directory <dir_name> (like `ls`)
  [Matching <file_name>, where wild-cards are allowed]
*/

static void do_list_files(struct st_command *command) {
  int error;
  static DYNAMIC_STRING ds_dirname;
  static DYNAMIC_STRING ds_wild;
  const struct command_arg list_files_args[] = {
      {"dirname", ARG_STRING, true, &ds_dirname, "Directory to list"},
      {"file", ARG_STRING, false, &ds_wild, "Filename (incl. wildcard)"}};
  DBUG_TRACE;
  command->used_replace = true;

  check_command_args(command, command->first_argument, list_files_args,
                     sizeof(list_files_args) / sizeof(struct command_arg), ' ');

  error = get_list_files(&ds_res, &ds_dirname, &ds_wild);
  handle_command_error(command, error);
  dynstr_free(&ds_dirname);
  dynstr_free(&ds_wild);
}

/*
  SYNOPSIS
  do_list_files_write_file_command
  command       called command
  append        append file, or create new

  DESCRIPTION
  list_files_{write|append}_file <filename> <dir_name> [<match_file>]
  List files and directories in directory <dir_name> (like `ls`)
  [Matching <match_file>, where wild-cards are allowed]

  Note: File will be truncated if exists and append is not true.
*/

static void do_list_files_write_file_command(struct st_command *command,
                                             bool append) {
  int error;
  static DYNAMIC_STRING ds_content;
  static DYNAMIC_STRING ds_filename;
  static DYNAMIC_STRING ds_dirname;
  static DYNAMIC_STRING ds_wild;
  const struct command_arg list_files_args[] = {
      {"filename", ARG_STRING, true, &ds_filename, "Filename for write"},
      {"dirname", ARG_STRING, true, &ds_dirname, "Directory to list"},
      {"file", ARG_STRING, false, &ds_wild, "Filename (incl. wildcard)"}};
  DBUG_TRACE;
  command->used_replace = true;

  check_command_args(command, command->first_argument, list_files_args,
                     sizeof(list_files_args) / sizeof(struct command_arg), ' ');

  init_dynamic_string(&ds_content, "", 1024, 1024);
  error = get_list_files(&ds_content, &ds_dirname, &ds_wild);
  handle_command_error(command, error);
  str_to_file2(ds_filename.str, ds_content.str, ds_content.length, append);
  dynstr_free(&ds_content);
  dynstr_free(&ds_filename);
  dynstr_free(&ds_dirname);
  dynstr_free(&ds_wild);
}

/*
  Read characters from line buffer or file. This is needed to allow
  my_ungetc() to buffer MAX_DELIMITER_LENGTH characters for a file

  NOTE:
  This works as long as one doesn't change files (with 'source file_name')
  when there is things pushed into the buffer.  This should however not
  happen for any tests in the test suite.
*/

static int my_getc(FILE *file) {
  if (line_buffer_pos == line_buffer) return fgetc(file);
  return *--line_buffer_pos;
}

static void my_ungetc(int c) { *line_buffer_pos++ = (char)c; }

static void read_until_delimiter(DYNAMIC_STRING *ds,
                                 DYNAMIC_STRING *ds_delimiter) {
  char c;
  DBUG_TRACE;
  DBUG_PRINT("enter", ("delimiter: %s, length: %u", ds_delimiter->str,
                       (uint)ds_delimiter->length));

  if (ds_delimiter->length > MAX_DELIMITER_LENGTH)
    die("Max delimiter length(%d) exceeded", MAX_DELIMITER_LENGTH);

  /* Read from file until delimiter is found */
  while (true) {
    c = my_getc(cur_file->file);

    if (c == '\n') {
      cur_file->lineno++;

      /* Skip newline from the same line as the command */
      if (start_lineno == (cur_file->lineno - 1)) continue;
    } else if (start_lineno == cur_file->lineno) {
      /*
        No characters except \n are allowed on
        the same line as the command
      */
      die("Trailing characters found after command");
    }

    if (feof(cur_file->file))
      die("End of file encountered before '%s' delimiter was found",
          ds_delimiter->str);

    if (match_delimiter(c, ds_delimiter->str, ds_delimiter->length)) {
      DBUG_PRINT("exit", ("Found delimiter '%s'", ds_delimiter->str));
      break;
    }
    dynstr_append_mem(ds, (const char *)&c, 1);
  }
  DBUG_PRINT("exit", ("ds: %s", ds->str));
}

static void do_write_file_command(struct st_command *command, bool append) {
  static DYNAMIC_STRING ds_content;
  static DYNAMIC_STRING ds_filename;
  static DYNAMIC_STRING ds_delimiter;
  const struct command_arg write_file_args[] = {
      {"filename", ARG_STRING, true, &ds_filename, "File to write to"},
      {"delimiter", ARG_STRING, false, &ds_delimiter,
       "Delimiter to read until"}};
  DBUG_TRACE;

  check_command_args(command, command->first_argument, write_file_args,
                     sizeof(write_file_args) / sizeof(struct command_arg), ' ');

  if (!append && access(ds_filename.str, F_OK) == 0) {
    /* The file should not be overwritten */
    die("File already exist: '%s'", ds_filename.str);
  }

  ds_content = command->content;
  /* If it hasn't been done already by a loop iteration, fill it in */
  if (!ds_content.str) {
    /* If no delimiter was provided, use EOF */
    if (ds_delimiter.length == 0) dynstr_set(&ds_delimiter, "EOF");

    init_dynamic_string(&ds_content, "", 1024, 1024);
    read_until_delimiter(&ds_content, &ds_delimiter);
    command->content = ds_content;
  }
  /* This function could be called even if "false", so check before printing */
  if (cur_block->ok) {
    DBUG_PRINT("info", ("Writing to file: %s", ds_filename.str));
    str_to_file2(ds_filename.str, ds_content.str, ds_content.length, append);
  }
  dynstr_free(&ds_filename);
  dynstr_free(&ds_delimiter);
}

/*
  SYNOPSIS
  do_write_file
  command	called command

  DESCRIPTION
  write_file <file_name> [<delimiter>];
  <what to write line 1>
  <...>
  < what to write line n>
  EOF

  --write_file <file_name>;
  <what to write line 1>
  <...>
  < what to write line n>
  EOF

  Write everything between the "write_file" command and 'delimiter'
  to "file_name"

  NOTE! Will fail if <file_name> exists

  Default <delimiter> is EOF

*/

static void do_write_file(struct st_command *command) {
  do_write_file_command(command, false);
}

/*
  SYNOPSIS
  do_append_file
  command	called command

  DESCRIPTION
  append_file <file_name> [<delimiter>];
  <what to write line 1>
  <...>
  < what to write line n>
  EOF

  --append_file <file_name>;
  <what to write line 1>
  <...>
  < what to write line n>
  EOF

  Append everything between the "append_file" command
  and 'delimiter' to "file_name"

  Default <delimiter> is EOF

*/

static void do_append_file(struct st_command *command) {
  do_write_file_command(command, true);
}

/*
  SYNOPSIS
  do_cat_file
  command	called command

  DESCRIPTION
  cat_file <file_name>;

  Print the given file to result log

*/

static void do_cat_file(struct st_command *command) {
  int error;
  static DYNAMIC_STRING ds_filename;
  const struct command_arg cat_file_args[] = {
      {"filename", ARG_STRING, true, &ds_filename, "File to read from"}};
  DBUG_TRACE;
  command->used_replace = true;

  check_command_args(command, command->first_argument, cat_file_args,
                     sizeof(cat_file_args) / sizeof(struct command_arg), ' ');

  DBUG_PRINT("info", ("Reading from, file: %s", ds_filename.str));

  error = cat_file(&ds_res, ds_filename.str);
  handle_command_error(command, error);
  dynstr_free(&ds_filename);
}

/// Compare the two files.
///
/// Success if 2 files are same, failure if the files are different or
/// either file does not exist.
///
/// @code
/// diff_files <file1> <file2>;
/// @endcode
///
/// @param command Pointer to the st_command structure which holds the
///                arguments and information for the command.
static void do_diff_files(struct st_command *command) {
  static DYNAMIC_STRING ds_filename1;
  static DYNAMIC_STRING ds_filename2;

  const struct command_arg diff_file_args[] = {
      {"file1", ARG_STRING, true, &ds_filename1, "First file to diff"},
      {"file2", ARG_STRING, true, &ds_filename2, "Second file to diff"}};

  check_command_args(command, command->first_argument, diff_file_args,
                     sizeof(diff_file_args) / sizeof(struct command_arg), ' ');

  if (access(ds_filename1.str, F_OK) != 0)
    die("Command \"diff_files\" failed, file '%s' does not exist.",
        ds_filename1.str);

  if (access(ds_filename2.str, F_OK) != 0)
    die("Command \"diff_files\" failed, file '%s' does not exist.",
        ds_filename2.str);

  int error = 0;
  if ((error = compare_files(ds_filename1.str, ds_filename2.str)) &&
      match_expected_error(command, error, nullptr) < 0) {
    // Compare of the two files failed, append them to output so the
    // failure can be analyzed, but only if it was not expected to fail.
    show_diff(&ds_res, ds_filename1.str, ds_filename2.str);
    if (log_file.write(ds_res.str, ds_res.length) || log_file.flush())
      cleanup_and_exit(1);
    dynstr_set(&ds_res, nullptr);
  }

  free_dynamic_strings(&ds_filename1, &ds_filename2);
  handle_command_error(command, error);
}

static struct st_connection *find_connection_by_name(const char *name) {
  struct st_connection *con;
  for (con = connections; con < next_con; con++) {
    if (!std::strcmp(con->name, name)) {
      return con;
    }
  }
  return nullptr; /* Connection not found */
}

/*
  SYNOPSIS
  do_send_quit
  command	called command

  DESCRIPTION
  Sends a simple quit command to the server for the named connection.

*/

static void do_send_quit(struct st_command *command) {
  char *p = command->first_argument;
  const char *name;
  struct st_connection *con;

  DBUG_TRACE;
  DBUG_PRINT("enter", ("name: '%s'", p));

  if (!*p) die("Missing connection name in send_quit");
  name = p;
  while (*p && !my_isspace(charset_info, *p)) p++;

  if (*p) *p++ = 0;
  command->last_argument = p;

  if (!(con = find_connection_by_name(name)))
    die("connection '%s' not found in connection pool", name);

  simple_command(&con->mysql, COM_QUIT, nullptr, 0, 1);
}

/*
  SYNOPSIS
  do_change_user
  command       called command

  DESCRIPTION
  change_user [<user>], [<passwd>], [<db>]
  <user> - user to change to
  <passwd> - user password
  <db> - default database

  Changes the user and causes the database specified by db to become
  the default (current) database for the the current connection.

*/

static void do_change_user(struct st_command *command) {
  MYSQL *mysql = &cur_con->mysql;
  static DYNAMIC_STRING ds_user, ds_passwd, ds_db;
  const struct command_arg change_user_args[] = {
      {"user", ARG_STRING, false, &ds_user, "User to connect as"},
      {"password", ARG_STRING, false, &ds_passwd,
       "Password used when connecting"},
      {"database", ARG_STRING, false, &ds_db,
       "Database to select after connect"},
  };

  DBUG_TRACE;

  check_command_args(command, command->first_argument, change_user_args,
                     sizeof(change_user_args) / sizeof(struct command_arg),
                     ',');

  if (cur_con->stmt) {
    mysql_stmt_close(cur_con->stmt);
    cur_con->stmt = nullptr;
  }

  if (!ds_user.length) {
    dynstr_set(&ds_user, mysql->user);

    if (!ds_passwd.length) dynstr_set(&ds_passwd, mysql->passwd);

    if (!ds_db.length) dynstr_set(&ds_db, mysql->db);
  }

  DBUG_PRINT("info",
             ("connection: '%s' user: '%s' password: '%s' database: '%s'",
              cur_con->name, ds_user.str, ds_passwd.str, ds_db.str));

  if (mysql_change_user_wrapper(mysql, ds_user.str, ds_passwd.str, ds_db.str)) {
    handle_error(curr_command, mysql_errno(mysql), mysql_error(mysql),
                 mysql_sqlstate(mysql), &ds_res);
    mysql->reconnect = true;
    mysql_reconnect(&cur_con->mysql);
  }

  dynstr_free(&ds_user);
  dynstr_free(&ds_passwd);
  dynstr_free(&ds_db);
}

/*
  SYNOPSIS
  do_perl
  command	command handle

  DESCRIPTION
  perl [<delimiter>];
  <perlscript line 1>
  <...>
  <perlscript line n>
  EOF

  Execute everything after "perl" until <delimiter> as perl.
  Useful for doing more advanced things
  but still being able to execute it on all platforms.

  Default <delimiter> is EOF
*/

static void do_perl(struct st_command *command) {
  int error;
  File fd;
  FILE *res_file;
  char buf[FN_REFLEN + 10];
  char temp_file_path[FN_REFLEN];
  static DYNAMIC_STRING ds_script;
  static DYNAMIC_STRING ds_delimiter;
  const struct command_arg perl_args[] = {{"delimiter", ARG_STRING, false,
                                           &ds_delimiter,
                                           "Delimiter to read until"}};
  DBUG_TRACE;

  check_command_args(command, command->first_argument, perl_args,
                     sizeof(perl_args) / sizeof(struct command_arg), ' ');

  ds_script = command->content;
  /* If it hasn't been done already by a loop iteration, fill it in */
  if (!ds_script.str) {
    /* If no delimiter was provided, use EOF */
    if (ds_delimiter.length == 0) dynstr_set(&ds_delimiter, "EOF");

    init_dynamic_string(&ds_script, "", 1024, 1024);
    read_until_delimiter(&ds_script, &ds_delimiter);
    command->content = ds_script;
  }

  /* This function could be called even if "false", so check before doing */
  if (cur_block->ok) {
    DBUG_PRINT("info", ("Executing perl: %s", ds_script.str));

    /* Create temporary file name */
    if ((fd =
             create_temp_file(temp_file_path, getenv("MYSQLTEST_VARDIR"), "tmp",
                              O_CREAT | O_RDWR, KEEP_FILE, MYF(MY_WME))) < 0)
      die("Failed to create temporary file for perl command");
    my_close(fd, MYF(0));

    /* Compatibility for Perl 5.24 and newer. */
    std::string script = "push @INC, \".\";\n";
    script.append(ds_script.str, ds_script.length);

    str_to_file(temp_file_path, &script[0], script.size());

    /* Format the "perl <filename>" command */
    snprintf(buf, sizeof(buf), "perl %s", temp_file_path);

    if (!(res_file = popen(buf, "r")) && command->abort_on_error)
      die("popen(\"%s\", \"r\") failed", buf);

    while (fgets(buf, sizeof(buf), res_file)) {
      if (disable_result_log) {
        buf[std::strlen(buf) - 1] = 0;
        DBUG_PRINT("exec_result", ("%s", buf));
      } else {
        replace_dynstr_append(&ds_res, buf);
      }
    }
    error = pclose(res_file);

    /* Remove the temporary file, but keep it if perl failed */
    if (!error) my_delete(temp_file_path, MYF(0));

    /* Check for error code that indicates perl could not be started */
    int exstat = WEXITSTATUS(error);
#ifdef _WIN32
    if (exstat == 1) /* Text must begin 'perl not found' as mtr looks for it */
      abort_not_supported_test("perl not found in path or did not start");
#else
    if (exstat == 127) abort_not_supported_test("perl not found in path");
#endif
    else
      handle_command_error(command, exstat);
  }
  dynstr_free(&ds_delimiter);
}

/*
  Print the content between echo and <delimiter> to result file.
  Evaluate all variables in the string before printing, allow
  for variable names to be escaped using \

  SYNOPSIS
  do_echo()
  command  called command

  DESCRIPTION
  echo text
  Print the text after echo until end of command to result file

  echo $<var_name>
  Print the content of the variable <var_name> to result file

  echo Some text $<var_name>
  Print "Some text" plus the content of the variable <var_name> to
  result file

  echo Some text \$<var_name>
  Print "Some text" plus $<var_name> to result file
*/

static int do_echo(struct st_command *command) {
  DYNAMIC_STRING ds_echo;
  DBUG_TRACE;

  init_dynamic_string(&ds_echo, "", command->query_len, 256);
  do_eval(&ds_echo, command->first_argument, command->end, false);
  dynstr_append_mem(&ds_res, ds_echo.str, ds_echo.length);
  dynstr_append_mem(&ds_res, "\n", 1);
  dynstr_free(&ds_echo);
  command->last_argument = command->end;
  return 0;
}

static void do_wait_for_slave_to_stop(
    struct st_command *c MY_ATTRIBUTE((unused))) {
  static int SLAVE_POLL_INTERVAL = 300000;
  MYSQL *mysql = &cur_con->mysql;
  for (;;) {
    MYSQL_RES *res = nullptr;
    MYSQL_ROW row;
    int done;

    if (mysql_query_wrapper(
            mysql,
            "SELECT 'Slave_running' as Variable_name,"
            " IF(count(*)>0,'ON','OFF') as Value FROM"
            " performance_schema.replication_applier_status ras,"
            "performance_schema.replication_connection_status rcs WHERE "
            "ras.SERVICE_STATE='ON' AND rcs.SERVICE_STATE='ON'") ||
        !(res = mysql_store_result_wrapper(mysql)))

      die("Query failed while probing slave for stop: %s", mysql_error(mysql));

    if (!(row = mysql_fetch_row_wrapper(res)) || !row[1]) {
      mysql_free_result_wrapper(res);
      die("Strange result from query while probing slave for stop");
    }
    done = !std::strcmp(row[1], "OFF");
    mysql_free_result_wrapper(res);
    if (done) break;
    my_sleep(SLAVE_POLL_INTERVAL);
  }
  return;
}

static void do_sync_with_master2(struct st_command *command, long offset) {
  MYSQL_RES *res;
  MYSQL_ROW row;
  MYSQL *mysql = &cur_con->mysql;
  char query_buf[FN_REFLEN + 128];
  int timeout = 300; /* seconds */
  char *valgrind_env;
  if ((valgrind_env = getenv("VALGRIND_TEST")) && strcmp(valgrind_env, "0")) {
    timeout *= 10;
  }

  if (!master_pos.file[0])
    die("Calling 'sync_with_master' without calling 'save_master_pos'");

  sprintf(query_buf, "select master_pos_wait('%s', %ld, %d)", master_pos.file,
          master_pos.pos + offset, timeout);

  if (mysql_query_wrapper(mysql, query_buf))
    die("failed in '%s': %d: %s", query_buf, mysql_errno(mysql),
        mysql_error(mysql));

  if (!(res = mysql_store_result_wrapper(mysql)))
    die("mysql_store_result() returned NULL for '%s'", query_buf);
  if (!(row = mysql_fetch_row_wrapper(res))) {
    mysql_free_result_wrapper(res);
    die("empty result in %s", query_buf);
  }

  int result = -99;
  const char *result_str = row[0];
  if (result_str) result = atoi(result_str);

  mysql_free_result_wrapper(res);

  if (!result_str || result < 0) {
    /* master_pos_wait returned NULL or < 0 */
    show_query(mysql, "SHOW MASTER STATUS");
    show_query(mysql, "SHOW SLAVE STATUS");
    show_query(mysql, "SHOW PROCESSLIST");
    fprintf(stderr, "analyze: sync_with_master\n");

    if (!result_str) {
      /*
        master_pos_wait returned NULL. This indicates that
        slave SQL thread is not started, the slave's master
        information is not initialized, the arguments are
        incorrect, or an error has occurred
      */
      die("%.*s failed: '%s' returned NULL "
          "indicating slave SQL thread failure",
          static_cast<int>(command->first_word_len), command->query, query_buf);
    }

    if (result == -1)
      die("%.*s failed: '%s' returned -1 "
          "indicating timeout after %d seconds",
          static_cast<int>(command->first_word_len), command->query, query_buf,
          timeout);
    else
      die("%.*s failed: '%s' returned unknown result :%d",
          static_cast<int>(command->first_word_len), command->query, query_buf,
          result);
  }

  return;
}

static void do_sync_with_master(struct st_command *command) {
  long offset = 0;
  char *p = command->first_argument;
  const char *offset_start = p;
  if (*offset_start) {
    for (; my_isdigit(charset_info, *p); p++) offset = offset * 10 + *p - '0';

    if (*p && !my_isspace(charset_info, *p))
      die("Invalid integer argument \"%s\"", offset_start);
    command->last_argument = p;
  }
  do_sync_with_master2(command, offset);
  return;
}

/*
  Wait for ndb binlog injector to be up-to-date with all changes
  done on the local mysql server
*/

static void ndb_wait_for_binlog_injector(void) {
  MYSQL_RES *res;
  MYSQL_ROW row;
  MYSQL *mysql = &cur_con->mysql;
  const char *query;
  ulong have_ndbcluster;
  if (mysql_query_wrapper(
          mysql, query = "select count(*) from information_schema.engines"
                         "  where engine = 'ndbcluster' and"
                         "        support in ('YES', 'DEFAULT')"))
    die("'%s' failed: %d %s", query, mysql_errno(mysql), mysql_error(mysql));
  if (!(res = mysql_store_result_wrapper(mysql)))
    die("mysql_store_result() returned NULL for '%s'", query);
  if (!(row = mysql_fetch_row_wrapper(res)))
    die("Query '%s' returned empty result", query);

  have_ndbcluster = std::strcmp(row[0], "1") == 0;
  mysql_free_result_wrapper(res);

  if (!have_ndbcluster) {
    return;
  }

  ulonglong start_epoch = 0, handled_epoch = 0, latest_trans_epoch = 0,
            latest_handled_binlog_epoch = 0, start_handled_binlog_epoch = 0;
  const int WaitSeconds = 150;

  int count = 0;
  int do_continue = 1;
  while (do_continue) {
    const char binlog[] = "binlog";
    const char latest_trans_epoch_str[] = "latest_trans_epoch=";
    const char latest_handled_binlog_epoch_str[] =
        "latest_handled_binlog_epoch=";
    if (count) my_sleep(100 * 1000); /* 100ms */

    if (mysql_query_wrapper(mysql, query = "show engine ndb status"))
      die("failed in '%s': %d %s", query, mysql_errno(mysql),
          mysql_error(mysql));

    if (!(res = mysql_store_result_wrapper(mysql)))
      die("mysql_store_result() returned NULL for '%s'", query);

    while ((row = mysql_fetch_row_wrapper(res))) {
      if (std::strcmp(row[1], binlog) == 0) {
        const char *status = row[2];

        /* latest_trans_epoch */
        while (*status && std::strncmp(status, latest_trans_epoch_str,
                                       sizeof(latest_trans_epoch_str) - 1))
          status++;
        if (*status) {
          status += sizeof(latest_trans_epoch_str) - 1;
          latest_trans_epoch = my_strtoull(status, (char **)nullptr, 10);
        } else
          die("result does not contain '%s' in '%s'", latest_trans_epoch_str,
              query);

        /* latest_handled_binlog */
        while (*status &&
               std::strncmp(status, latest_handled_binlog_epoch_str,
                            sizeof(latest_handled_binlog_epoch_str) - 1))
          status++;
        if (*status) {
          status += sizeof(latest_handled_binlog_epoch_str) - 1;
          latest_handled_binlog_epoch =
              my_strtoull(status, (char **)nullptr, 10);
        } else
          die("result does not contain '%s' in '%s'",
              latest_handled_binlog_epoch_str, query);

        if (count == 0) {
          start_epoch = latest_trans_epoch;
          start_handled_binlog_epoch = latest_handled_binlog_epoch;
        }
        break;
      }
    }
    if (!row) die("result does not contain '%s' in '%s'", binlog, query);
    if (latest_handled_binlog_epoch > handled_epoch) count = 0;
    handled_epoch = latest_handled_binlog_epoch;
    count++;
    if (latest_handled_binlog_epoch >= start_epoch)
      do_continue = 0;
    else if (count > (WaitSeconds * 10)) {
      die("do_save_master_pos() timed out after %u s waiting for "
          "last committed epoch to be applied by the "
          "Ndb binlog injector.  "
          "Ndb epoch %llu/%llu to be handled.  "
          "Last handled epoch : %llu/%llu.  "
          "First handled epoch : %llu/%llu.",
          WaitSeconds, start_epoch >> 32, start_epoch & 0xffffffff,
          latest_handled_binlog_epoch >> 32,
          latest_handled_binlog_epoch & 0xffffffff,
          start_handled_binlog_epoch >> 32,
          start_handled_binlog_epoch & 0xffffffff);
    }

    mysql_free_result_wrapper(res);
  }
}

static int do_save_master_pos() {
  MYSQL_RES *res;
  MYSQL_ROW row;
  MYSQL *mysql = &cur_con->mysql;
  const char *query;
  DBUG_TRACE;
  /*
    when ndb binlog is on, this call will wait until last updated epoch
    (locally in the mysqld) has been received into the binlog
  */
  ndb_wait_for_binlog_injector();

  if (mysql_query_wrapper(mysql, query = "show master status"))
    die("failed in 'show master status': %d %s", mysql_errno(mysql),
        mysql_error(mysql));

  if (!(res = mysql_store_result_wrapper(mysql)))
    die("mysql_store_result() retuned NULL for '%s'", query);
  if (!(row = mysql_fetch_row_wrapper(res)))
    die("empty result in show master status");
  my_stpnmov(master_pos.file, row[0], sizeof(master_pos.file) - 1);
  master_pos.pos = strtoul(row[1], (char **)nullptr, 10);
  mysql_free_result_wrapper(res);
  return 0;
}

/*
  Check if a variable name is valid or not.

  SYNOPSIS
  check_variable_name()
  var_name     - pointer to the beginning of variable name
  var_name_end - pointer to the end of variable name
  dollar_flag  - flag to check whether variable name should start with '$'
*/
static void check_variable_name(const char *var_name, const char *var_name_end,
                                const bool dollar_flag) {
  char save_var_name[MAX_VAR_NAME_LENGTH];
  strmake(save_var_name, var_name, (var_name_end - var_name));

  // Check if variable name should start with '$'
  if (!dollar_flag && (*var_name != '$'))
    die("Variable name '%s' should start with '$'", save_var_name);

  if (*var_name == '$') var_name++;

  // Check if variable name exists or not
  if (var_name == var_name_end) die("Missing variable name.");

  // Check for non alphanumeric character(s) in variable name
  while ((var_name != var_name_end) && my_isvar(charset_info, *var_name))
    var_name++;

  if (var_name != var_name_end)
    die("Invalid variable name '%s'", save_var_name);
}

/*
  Check if the pointer points to an operator.

  SYNOPSIS
  is_operator()
  op - character pointer to mathematical expression
*/
static bool is_operator(const char *op) {
  if (*op == '+')
    return true;
  else if (*op == '-')
    return true;
  else if (*op == '*')
    return true;
  else if (*op == '/')
    return true;
  else if (*op == '%')
    return true;
  else if (*op == '&' && *(op + 1) == '&')
    return true;
  else if (*op == '|' && *(op + 1) == '|')
    return true;
  else if (*op == '&')
    return true;
  else if (*op == '|')
    return true;
  else if (*op == '^')
    return true;
  else if (*op == '>' && *(op + 1) == '>')
    return true;
  else if (*op == '<' && *(op + 1) == '<')
    return true;

  return false;
}

/*
  Perform basic mathematical operation.

  SYNOPSIS
  do_expr()
  command - command handle

  DESCRIPTION
  expr $<var_name>= <operand1> <operator> <operand2>
  Perform basic mathematical operation and store the result
  in a variable($<var_name>). Both <operand1> and <operand2>
  should be valid MTR variables.

  'expr' command supports only binary operators that operates
  on two operands and manipulates them to return a result.

  Following mathematical operators are supported.
  1 Arithmetic Operators
    1.1 Addition
    1.2 Subtraction
    1.3 Multiplication
    1.4 Division
    1.5 Modulo

  2 Logical Operators
    2.1 Logical AND
    2.2 Logical OR

  3 Bitwise Operators
    3.1 Binary AND
    3.2 Binary OR
    3.3 Binary XOR
    3.4 Binary Left Shift
    3.5 Binary Right Shift

  NOTE
  1. Non-integer operand is truncated to integer value for operations
     that dont support non-integer operand.
  2. If the result is an infinite value, then expr command will return
     'inf' keyword to indicate the result is infinity.
  3. Division by 0 will result in an infinite value and expr command
     will return 'inf' keyword to indicate the result is infinity.
*/
static void do_expr(struct st_command *command) {
  DBUG_TRACE;

  const char *p = command->first_argument;
  if (!*p) die("Missing arguments to expr command.");

  // Find <var_name>
  const char *var_name = p;
  while (*p && (*p != '=') && !my_isspace(charset_info, *p)) p++;
  const char *var_name_end = p;
  check_variable_name(var_name, var_name_end, true);

  // Skip spaces between <var_name> and '='
  while (my_isspace(charset_info, *p)) p++;

  if (*p++ != '=') die("Missing assignment operator in expr command.");

  // Skip spaces after '='
  while (*p && my_isspace(charset_info, *p)) p++;

  // Save the mathematical expression in a variable
  const char *expr = p;

  // First operand in the expression
  const char *operand_name = p;
  while (*p && !is_operator(p) && !my_isspace(charset_info, *p)) p++;
  const char *operand_name_end = p;
  check_variable_name(operand_name, operand_name_end, false);
  VAR *v1 = var_get(operand_name, &operand_name_end, false, false);

  double operand1;
  if ((my_isdigit(charset_info, *v1->str_val)) ||
      ((*v1->str_val == '-') && my_isdigit(charset_info, *(v1->str_val + 1))))
    operand1 = strtod(v1->str_val, nullptr);
  else
    die("Undefined/invalid first operand '$%s' in expr command.", v1->name);

  // Skip spaces after the first operand
  while (*p && my_isspace(charset_info, *p)) p++;

  // Extract the operator
  const char *operator_start = p;
  while (*p && (*p != '$') &&
         !(my_isspace(charset_info, *p) || my_isvar(charset_info, *p)))
    p++;

  char math_operator[3];
  strmake(math_operator, operator_start, (p - operator_start));
  if (!std::strlen(math_operator))
    die("Missing mathematical operator in expr command.");

  // Skip spaces after the operator
  while (*p && my_isspace(charset_info, *p)) p++;

  // Second operand in the expression
  operand_name = p;
  while (*p && !my_isspace(charset_info, *p)) p++;
  operand_name_end = p;
  check_variable_name(operand_name, operand_name_end, false);
  VAR *v2 = var_get(operand_name, &operand_name_end, false, false);

  double operand2;
  if ((my_isdigit(charset_info, *v2->str_val)) ||
      ((*v2->str_val == '-') && my_isdigit(charset_info, *(v2->str_val + 1))))
    operand2 = strtod(v2->str_val, nullptr);
  else
    die("Undefined/invalid second operand '$%s' in expr command.", v2->name);

  // Skip spaces at the end
  while (*p && my_isspace(charset_info, *p)) p++;

  // Check for any spurious text after the second operand
  if (*p) die("Invalid mathematical expression '%s' in expr command.", expr);

  double result;
  // Arithmetic Operators
  if (!std::strcmp(math_operator, "+"))
    result = operand1 + operand2;
  else if (!std::strcmp(math_operator, "-"))
    result = operand1 - operand2;
  else if (!std::strcmp(math_operator, "*"))
    result = operand1 * operand2;
  else if (!std::strcmp(math_operator, "/")) {
    if (operand2 == 0.0) {
      if (operand1 == 0.0)
        result = std::numeric_limits<double>::quiet_NaN();
      else
        result = std::numeric_limits<double>::infinity();
    } else
      result = operand1 / operand2;
  } else if (!std::strcmp(math_operator, "%"))
    result = (int)operand1 % (int)operand2;
  // Logical Operators
  else if (!std::strcmp(math_operator, "&&"))
    result = operand1 && operand2;
  else if (!std::strcmp(math_operator, "||"))
    result = operand1 || operand2;
  // Bitwise Operators
  else if (!std::strcmp(math_operator, "&"))
    result = (int)operand1 & (int)operand2;
  else if (!std::strcmp(math_operator, "|"))
    result = (int)operand1 | (int)operand2;
  else if (!std::strcmp(math_operator, "^"))
    result = (int)operand1 ^ (int)operand2;
  else if (!std::strcmp(math_operator, ">>"))
    result = (int)operand1 >> (int)operand2;
  else if (!std::strcmp(math_operator, "<<"))
    result = (int)operand1 << (int)operand2;
  else
    die("Invalid operator '%s' in expr command", math_operator);

  char buf[128];
  size_t result_len;
  // Check if result is an infinite value
  if (std::isnan(result)) {
    // Print 'nan' if result is Not a Number
    result_len = snprintf(buf, sizeof(buf), "%s", "nan");
  } else if (!std::isinf(result)) {
    const char *format = (result < 1e10 && result > -1e10) ? "%f" : "%g";
    result_len = snprintf(buf, sizeof(buf), format, result);
  } else
    // Print 'inf' if result is an infinite value
    result_len = snprintf(buf, sizeof(buf), "%s", "inf");

  if (result < 1e10 && result > -1e10) {
    /*
      Remove the trailing 0's i.e 2.0000000 need to be represented
      as 2 for consistency, 2.0010000 also becomes 2.001.
    */
    while (buf[result_len - 1] == '0') result_len--;

    // Remove trailing '.' if exists
    if (buf[result_len - 1] == '.') result_len--;
  }

  var_set(var_name, var_name_end, buf, buf + result_len);
  command->last_argument = command->end;
}

/*
  Assign the variable <var_name> with <var_val>

  SYNOPSIS
  do_let()
  query	called command

  DESCRIPTION
  let $<var_name>=<var_val><delimiter>

  <var_name>  - is the string string found between the $ and =
  <var_val>   - is the content between the = and <delimiter>, it may span
  multiple line and contain any characters except <delimiter>
  <delimiter> - is a string containing of one or more chars, default is ;

  RETURN VALUES
  Program will die if error detected
*/

static void do_let(struct st_command *command) {
  const char *p = command->first_argument;
  const char *var_name, *var_name_end;
  DYNAMIC_STRING let_rhs_expr;
  DBUG_TRACE;

  init_dynamic_string(&let_rhs_expr, "", 512, 2048);

  /* Find <var_name> */
  if (!*p) die("Missing arguments to let");
  var_name = p;
  while (*p && (*p != '=') && !my_isspace(charset_info, *p)) p++;
  var_name_end = p;
  if (var_name == var_name_end ||
      (var_name + 1 == var_name_end && *var_name == '$'))
    die("Missing variable name in let");
  while (my_isspace(charset_info, *p)) p++;
  if (*p++ != '=') die("Missing assignment operator in let");

  /* Find start of <var_val> */
  while (*p && my_isspace(charset_info, *p)) p++;

  do_eval(&let_rhs_expr, p, command->end, false);

  command->last_argument = command->end;
  /* Assign var_val to var_name */
  var_set(var_name, var_name_end, let_rhs_expr.str,
          (let_rhs_expr.str + let_rhs_expr.length));
  dynstr_free(&let_rhs_expr);
  revert_properties();
}

/*
  Sleep the number of specified seconds

  SYNOPSIS
  do_sleep()
  q	       called command
  real_sleep   use the value from opt_sleep as number of seconds to sleep
               if real_sleep is false

  DESCRIPTION
  sleep <seconds>
  real_sleep <seconds>

  The difference between the sleep and real_sleep commands is that sleep
  uses the delay from the --sleep command-line option if there is one.
  (If the --sleep option is not given, the sleep command uses the delay
  specified by its argument.) The real_sleep command always uses the
  delay specified by its argument.  The logic is that sometimes delays are
  cpu-dependent, and --sleep can be used to set this delay.  real_sleep is
  used for cpu-independent delays.
*/

static int do_sleep(struct st_command *command, bool real_sleep) {
  int error = 0;
  double sleep_val;
  char *p;
  static DYNAMIC_STRING ds_sleep;
  const struct command_arg sleep_args[] = {{"sleep_delay", ARG_STRING, true,
                                            &ds_sleep,
                                            "Number of seconds to sleep."}};
  check_command_args(command, command->first_argument, sleep_args,
                     sizeof(sleep_args) / sizeof(struct command_arg), ' ');

  p = ds_sleep.str;
  const char *sleep_end = ds_sleep.str + ds_sleep.length;
  while (my_isspace(charset_info, *p)) p++;
  if (!*p)
    die("Missing argument to %.*s", static_cast<int>(command->first_word_len),
        command->query);
  const char *sleep_start = p;
  /* Check that arg starts with a digit, not handled by my_strtod */
  if (!my_isdigit(charset_info, *sleep_start))
    die("Invalid argument to %.*s \"%s\"",
        static_cast<int>(command->first_word_len), command->query, sleep_start);
  sleep_val = my_strtod(sleep_start, &sleep_end, &error);
  check_eol_junk_line(sleep_end);
  if (error)
    die("Invalid argument to %.*s \"%s\"",
        static_cast<int>(command->first_word_len), command->query,
        command->first_argument);
  dynstr_free(&ds_sleep);

  /* Fixed sleep time selected by --sleep option */
  if (opt_sleep >= 0 && !real_sleep) sleep_val = opt_sleep;

  DBUG_PRINT("info", ("sleep_val: %f", sleep_val));
  if (sleep_val) my_sleep((ulong)(sleep_val * 1000000L));
  return 0;
}

static void do_set_charset(struct st_command *command) {
  char *charset_name = command->first_argument;
  char *p;

  if (!charset_name || !*charset_name)
    die("Missing charset name in 'character_set'");
  /* Remove end space */
  p = charset_name;
  while (*p && !my_isspace(charset_info, *p)) p++;
  if (*p) *p++ = 0;
  command->last_argument = p;
  charset_info =
      get_charset_by_csname(charset_name, MY_CS_PRIMARY, MYF(MY_WME));
  if (!charset_info)
    abort_not_supported_test("Test requires charset '%s'", charset_name);
}

/// Check if the bug number argument to disable_testcase is in a proper
/// format.
///
/// Bug number argument should follow 'BUG#XXXX' format
///
///   - Keyword 'BUG" is case-insensitive
///   - XXXX should contain only digits
///
/// @param bug_number String representing a bug number
///
/// @retval True if the bug number argument is in correct format, false
///         otherwise.
static bool validate_bug_number_argument(std::string bug_number) {
  // Convert the string to lowercase characters
  std::transform(bug_number.begin(), bug_number.end(), bug_number.begin(),
                 ::tolower);

  // Check if string representing a bug number starts 'BUG' keyword.
  // Note: This keyword is case-inseinsitive.
  if (bug_number.substr(0, 3).compare("bug") != 0) return false;

  // Check if the string contains '#' after 'BUG' keyword
  if (bug_number.at(3) != '#') return false;

  // Check if the bug number string contains only digits after '#'
  if (get_int_val(bug_number.substr(4).c_str()) == -1) return false;

  return true;
}

/// Disable or don't execute the statements or commands appear after
/// this command until the execution is enabled by 'enable_testcase'
/// command. If test cases are already disabled, then throw an error
/// and abort the test execution.
///
/// This command also takes a mandatory parameter specifying the bug
/// number.
///
/// @code
/// --disable_testcase BUG#XXXX
/// statements or commands
/// --enable_testcase
/// @endcode
///
/// @param command Pointer to the st_command structure which holds the
///                arguments and information for the command.
static void do_disable_testcase(struct st_command *command) {
  DYNAMIC_STRING ds_bug_number;
  const struct command_arg disable_testcase_args[] = {
      {"Bug number", ARG_STRING, true, &ds_bug_number, "Bug number"}};

  check_command_args(command, command->first_argument, disable_testcase_args,
                     sizeof(disable_testcase_args) / sizeof(struct command_arg),
                     ' ');

  /// Check if the bug number argument to disable_testcase is in a
  /// proper format.
  if (validate_bug_number_argument(ds_bug_number.str) == 0) {
    free_dynamic_strings(&ds_bug_number);
    die("Bug number mentioned in '%s' command is not in a correct format. "
        "It should be 'BUG#XXXX', where keyword 'BUG' is case-insensitive "
        "and 'XXXX' should contain only digits.",
        command->query);
  }

  testcase_disabled = true;
  free_dynamic_strings(&ds_bug_number);
}

/**
  Check if process is active.

  @param pid  Process id.

  @return true if process is active, false otherwise.
*/
static bool is_process_active(int pid) {
#ifdef _WIN32
  DWORD exit_code;
  HANDLE proc;
  proc = OpenProcess(PROCESS_QUERY_INFORMATION, false, pid);
  if (proc == NULL) return false; /* Process could not be found. */

  if (!GetExitCodeProcess(proc, &exit_code)) exit_code = 0;

  CloseHandle(proc);
  if (exit_code != STILL_ACTIVE)
    return false; /* Error or process has terminated. */

  return true;
#else
  return (kill(pid, 0) == 0);
#endif
}

/**
  kill process.

  @param pid  Process id.

  @return true if process is terminated, false otherwise.
*/
static bool kill_process(int pid) {
  bool killed = true;
#ifdef _WIN32
  HANDLE proc;
  proc = OpenProcess(PROCESS_TERMINATE, false, pid);
  if (proc == NULL) return true; /* Process could not be found. */

  if (!TerminateProcess(proc, 201)) killed = false;

  CloseHandle(proc);
#else
  killed = (kill(pid, SIGKILL) == 0);
#endif
  return killed;
}

/**
  Abort process.

  @param pid  Process id.
  @param path Path to create minidump file in.
*/
static void abort_process(int pid, const char *path MY_ATTRIBUTE((unused))) {
#ifdef _WIN32
  HANDLE proc;
  proc = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
  verbose_msg("Aborting pid %d (handle: %p)\n", pid, proc);
  if (proc != NULL) {
    char name[FN_REFLEN], *end;
    BOOL is_debugged;

    if (path) {
      /* Append mysqld.<pid>.dmp to the path. */
      strncpy(name, path, sizeof(name) - 1);
      name[sizeof(name) - 1] = '\0';
      end = name + std::strlen(name);
      /* Make sure "/mysqld.nnnnnnnnnn.dmp" fits */
      if ((end - name) < (sizeof(name) - 23)) {
        if (!is_directory_separator(end[-1])) {
          end[0] = FN_LIBCHAR2;  // datadir path normally uses '/'.
          end++;
        }
        snprintf(end, sizeof(name) + name - end - 1, "mysqld.%d.dmp", pid);

        verbose_msg("Creating minidump.\n");
        my_create_minidump(name, proc, pid);
      } else
        die("Path too long for creating minidump!\n");
    }
    /* If running in a debugger, send a break, otherwise terminate. */
    if (CheckRemoteDebuggerPresent(proc, &is_debugged) && is_debugged) {
      if (!DebugBreakProcess(proc)) {
        DWORD err = GetLastError();
        verbose_msg("DebugBreakProcess failed: %d\n", err);
      } else
        verbose_msg("DebugBreakProcess succeeded!\n");
      CloseHandle(proc);
    } else {
      CloseHandle(proc);
      (void)kill_process(pid);
    }
  } else {
    DWORD err = GetLastError();
    verbose_msg("OpenProcess failed: %d\n", err);
  }
#else
  kill(pid, SIGABRT);
#endif
}

/// Shutdown or kill the server. If timeout is set to 0 the server is
/// killed or terminated immediately. Otherwise the shutdown command
/// is first sent and then it waits for the server to terminate within
/// 'timeout' seconds. If it has not terminated before 'timeout'
/// seconds the command will fail.
///
/// @note
/// Currently only works with local server
///
/// @param command Pointer to the st_command structure which holds the
///                arguments and information for the command. Optionally
///                including a timeout else the default of 60 seconds
static void do_shutdown_server(struct st_command *command) {
  static DYNAMIC_STRING ds_timeout;
  const struct command_arg shutdown_args[] = {
      {"timeout", ARG_STRING, false, &ds_timeout,
       "Timeout before killing server"}};

  check_command_args(command, command->first_argument, shutdown_args,
                     sizeof(shutdown_args) / sizeof(struct command_arg), ' ');
  if (opt_offload_count_file) {
    // Save the value of secondary engine execution status
    // before shutting down the server.
    if (secondary_engine->offload_count(&cur_con->mysql, "after"))
      cleanup_and_exit(1);
  }

  long timeout = 60;

  if (ds_timeout.length) {
    char *endptr;
    timeout = std::strtol(ds_timeout.str, &endptr, 10);
    if (*endptr != '\0')
      die("Illegal argument for timeout: '%s'", ds_timeout.str);
  }

  dynstr_free(&ds_timeout);

  char *valgrind_env;
  if ((valgrind_env = getenv("VALGRIND_TEST")) && strcmp(valgrind_env, "0")) {
    timeout *= 10;
  }

  MYSQL *mysql = &cur_con->mysql;
  std::string ds_file_name;

  // Get the servers pid_file name and use it to read pid
  if (query_get_string(mysql, "SHOW VARIABLES LIKE 'pid_file'", 1,
                       &ds_file_name))
    die("Failed to get pid_file from server");

  // Read the pid from the file
  int fd;
  char buff[32];

  if ((fd = my_open(ds_file_name.c_str(), O_RDONLY, MYF(0))) < 0)
    die("Failed to open file '%s'", ds_file_name.c_str());

  if (my_read(fd, (uchar *)&buff, sizeof(buff), MYF(0)) <= 0) {
    my_close(fd, MYF(0));
    die("pid file was empty");
  }

  my_close(fd, MYF(0));

  int pid = std::atoi(buff);
  if (pid == 0) die("Pidfile didn't contain a valid number");

  int error = 0;
  if (timeout) {
    // Check if we should generate a minidump on timeout.
    if (query_get_string(mysql, "SHOW VARIABLES LIKE 'core_file'", 1,
                         &ds_file_name) ||
        std::strcmp("ON", ds_file_name.c_str())) {
    } else {
      // Get the data dir and use it as path for a minidump if needed.
      if (query_get_string(mysql, "SHOW VARIABLES LIKE 'datadir'", 1,
                           &ds_file_name))
        die("Failed to get datadir from server");
    }

    const char *var_name = "$MTR_MANUAL_DEBUG";
    VAR *var = var_get(var_name, &var_name, false, false);
    if (var->int_val) {
      if (!kill_process(pid) && is_process_active(pid)) error = 3;
    } else {
      // Tell server to shutdown if timeout > 0.
      if (timeout > 0 && mysql_query_wrapper(mysql, "shutdown")) {
        // Failed to issue shutdown command.
        error = 1;
        goto end;
      }

      // Check that server dies
      do {
        if (!is_process_active(pid)) {
          DBUG_PRINT("info", ("Process %d does not exist anymore", pid));
          goto end;
        }
        if (timeout > 0) {
          DBUG_PRINT("info", ("Sleeping, timeout: %ld", timeout));
          my_sleep(1000000L);
        }
      } while (timeout-- > 0);

      error = 2;

      // Abort to make it easier to find the hang/problem.
      abort_process(pid, ds_file_name.c_str());
    }
  } else {
    // timeout value is 0, kill the server
    DBUG_PRINT("info", ("Killing server, pid: %d", pid));

    // kill_process can fail (bad privileges, non existing process on
    // *nix etc), so also check if the process is active before setting
    // error.
    if (!kill_process(pid) && is_process_active(pid)) error = 3;
  }

end:
  if (error) handle_command_error(command, error);
}

/// Evaluate the warning list argument specified with either
/// disable_warnings or enable_warnings command and replace the
/// variables with actual values if there exist any.
///
/// @param command     Pointer to the st_command structure which holds
///                    the arguments and information for the command.
/// @param ds_warnings DYNAMIC_STRING object containing the argument.
///
/// @retval Evaluated string after replacing the variables with values.
const char *eval_warning_argument(struct st_command *command,
                                  DYNAMIC_STRING *ds_warnings) {
  dynstr_trunc(ds_warnings, ds_warnings->length);
  do_eval(ds_warnings, command->first_argument, command->end, false);
  return ds_warnings->str;
}

/// Check if second argument "ONCE" to disable_warnings or enable_warnings
/// command is specified. If yes, filter out the keyword "ONCE" from the
/// argument string.
///
/// @param ds_property   DYNAMIC_STRING object containing the second argument
/// @param warn_argument String to store the argument string containing only
///                      the list of warnings.
///
/// @retval True if the second argument is specified, false otherwise.
static bool check_and_filter_once_property(DYNAMIC_STRING ds_property,
                                           std::string *warn_argument) {
  if (ds_property.length) {
    // Second argument exists, and it should be "ONCE" keyword.
    if (std::strcmp(ds_property.str, "ONCE"))
      die("Second argument to '%s' command should always be \"ONCE\" keyword.",
          command_names[curr_command->type - 1]);

    // Filter out the keyword and save only the warnings.
    std::size_t position = warn_argument->find(" ONCE");
    DBUG_ASSERT(position != std::string::npos);
    warn_argument->erase(position, 5);
    return true;
  }

  return false;
}

/// Handle disabling of warnings.
///
/// * If all warnings are disabled, don't add the warning to disabled
///   warning list.
/// * If there exist enabled warnings, remove the disabled warning from
///   the list of enabled warnings.
/// * If all the warnings are enabled or if there exist disabled warnings,
///   add or append the new warning to the list of disabled warnings.
///
/// @param warning_code Warning code
/// @param warning      Warning string
/// @param once_prop    Flag specifying whether a property should be set
///                     for next statement only.
static void handle_disable_warnings(std::uint32_t warning_code,
                                    std::string warning, bool once_prop) {
  if (enabled_warnings->count()) {
    // Remove the warning from list of enabled warnings.
    enabled_warnings->remove_warning(warning_code, once_prop);
  } else if (!disable_warnings || disabled_warnings->count()) {
    // Add the warning to list of expected warnings only if all the
    // warnings are not disabled.
    disabled_warnings->add_warning(warning_code, warning.c_str(), once_prop);
  }
}

/// Handle enabling of warnings.
///
/// * If all the warnings are enabled, don't add the warning to enabled
///   warning list.
/// * If there exist disabled warnings, remove the enabled warning from
///   the list of disabled warnings.
/// * If all the warnings are disabled or if there exist enabled warnings,
///   add or append the new warning to the list of enabled warnings.
///
/// @param warning_code Warning code
/// @param warning      Warning string
/// @param once_prop    Flag specifying whether a property should be set
///                     for next statement only.
static void handle_enable_warnings(std::uint32_t warning_code,
                                   std::string warning, bool once_prop) {
  if (disabled_warnings->count()) {
    // Remove the warning from list of disabled warnings.
    disabled_warnings->remove_warning(warning_code, once_prop);
  } else if (disabled_warnings) {
    // All the warnings are disabled, enable only the warnings specified
    // in the argument.
    enabled_warnings->add_warning(warning_code, warning.c_str(), once_prop);
  }
}

/// Get an error code corresponding to a warning name. The warning name
/// specified is in symbolic error name format.
///
/// @param command       Pointer to the st_command structure which holds the
///                      arguments and information for the command.
/// @param warn_argument String containing warning argument
/// @param once_prop     Flag specifying whether a property should be set for
///                      next statement only.
static void get_warning_codes(struct st_command *command,
                              std::string warn_argument, bool once_prop) {
  std::string warning;
  std::stringstream warnings(warn_argument);

  if (!my_isalnum(charset_info, warn_argument.back())) {
    die("Invalid argument '%s' to '%s' command.", command->first_argument,
        command_names[command->type - 1]);
  }

  while (std::getline(warnings, warning, ',')) {
    // Remove any space in a string representing a warning.
    warning.erase(remove_if(warning.begin(), warning.end(), isspace),
                  warning.end());

    // Check if a warning name is a valid symbolic error name.
    if (warning.front() == 'E' || warning.front() == 'W') {
      int warning_code = get_errcode_from_name(warning);
      if (warning_code == -1)
        die("Unknown SQL error name '%s'.", warning.c_str());
      if (command->type == Q_DISABLE_WARNINGS)
        handle_disable_warnings(warning_code, warning, once_prop);
      else if (command->type == Q_ENABLE_WARNINGS) {
        handle_enable_warnings(warning_code, warning, once_prop);
        // If disable_warnings flag is set, and there are no disabled or
        // enabled warnings, set the disable_warnings flag to 0.
        if (disable_warnings) {
          if (!disabled_warnings->count() && !enabled_warnings->count())
            set_property(command, P_WARN, false);
        }
      }
    } else {
      // Invalid argument, should only consist of warnings specified in
      // symbolic error name format.
      die("Invalid argument '%s' to '%s' command, list of disabled or enabled "
          "warnings may only consist of symbolic error names.",
          command->first_argument, command_names[command->type - 1]);
    }
  }
}

/// Parse the warning list argument specified with disable_warnings or
/// enable_warnings command. Check if the second argument "ONCE" is
/// specified, if yes, set once_property flag.
///
/// @param command Pointer to the st_command structure which holds the
///                arguments and information for the command.
///
/// @retval True if second argument "ONCE" is specified, false otherwise.
static bool parse_warning_list_argument(struct st_command *command) {
  DYNAMIC_STRING ds_warnings, ds_property;
  const struct command_arg warning_args[] = {
      {"Warnings", ARG_STRING, false, &ds_warnings,
       "Comma separated list of warnings to be disabled or enabled."},
      {"Property", ARG_STRING, false, &ds_property,
       "Keyword \"ONCE\" repesenting the property should be set for next "
       "statement only."}};

  check_command_args(command, command->first_argument, warning_args,
                     sizeof(warning_args) / sizeof(struct command_arg), ' ');

  // Waning list argument can't be an empty string.
  if (!ds_warnings.length)
    die("Warning list argument to command '%s' can't be an empty string.",
        command_names[command->type - 1]);

  // Evaluate warning list argument and replace the variables with
  // actual values
  std::string warn_argument = eval_warning_argument(command, &ds_warnings);

  // Set once_prop flag to true if keyword "ONCE" is specified as an
  // argument to a disable_warnings or a enable_warnings command.
  // Filter this keyword from the argument string and save only the
  // list of warnings.
  bool once_prop = check_and_filter_once_property(ds_property, &warn_argument);

  // Free all the DYNAMIC_STRING objects created
  free_dynamic_strings(&ds_warnings, &ds_property);

  // Get warning codes
  get_warning_codes(command, warn_argument, once_prop);

  return once_prop;
}

/// Create a list of disabled warnings that should be suppressed for
/// the next statements until enabled by enable_warnings command.
///
/// disable_warnings command can take an optional argument specifying
/// a warning or a comma separated list of warnings to be disabled.
/// The warnings specified should be in symbolic error name format.
///
/// disable_warnings command can also take a second optional argument,
/// which when specified will suppress the warnings for next statement
/// only. The second argument must be a "ONCE" keyword.
///
/// @param command Pointer to the st_command structure which holds the
///                arguments and information for the command.
static void do_disable_warnings(struct st_command *command) {
  // Revert the previously set properties
  if (once_property) revert_properties();

  // Check if disable_warnings command has warning list argument.
  if (!*command->first_argument) {
    // disable_warnings without an argument, disable all the warnings.
    if (disabled_warnings->count()) disabled_warnings->clear_list();
    if (enabled_warnings->count()) enabled_warnings->clear_list();

    // Update the environment variables containing the list of disabled
    // and enabled warnings.
    update_disabled_enabled_warnings_list_var();

    // Set 'disable_warnings' property value to 1
    set_property(command, P_WARN, true);
    return;
  } else {
    // Parse the warning list argument specified with disable_warnings
    // command.
    parse_warning_list_argument(command);

    // Update the environment variables containing the list of disabled
    // and enabled warnings.
    update_disabled_enabled_warnings_list_var();

    // Set 'disable_warnings' property value to 1
    set_property(command, P_WARN, true);
  }

  command->last_argument = command->end;
}

/// Create a list of enabled warnings that should be enabled for the
/// next statements until disabled by disable_warnings command.
///
/// enable_warnings command can take an optional argument specifying
/// a warning or a comma separated list of warnings to be enabled. The
/// warnings specified should be in symbolic error name format.
///
/// enable_warnings command can also take a second optional argument,
/// which when specified will enable the warnings for next statement
/// only. The second argument must be a "ONCE" keyword.
///
/// @param command Pointer to the st_command structure which holds the
///                arguments and information for the command.
static void do_enable_warnings(struct st_command *command) {
  // Revert the previously set properties
  if (once_property) revert_properties();

  bool once_prop = false;
  if (!*command->first_argument) {
    // enable_warnings without an argument, enable all the warnings.
    if (disabled_warnings->count()) disabled_warnings->clear_list();
    if (enabled_warnings->count()) enabled_warnings->clear_list();

    // Update the environment variables containing the list of disabled
    // and enabled warnings.
    update_disabled_enabled_warnings_list_var();

    // Set 'disable_warnings' property value to 0
    set_property(command, P_WARN, false);
  } else {
    // Parse the warning list argument specified with enable_warnings command.
    once_prop = parse_warning_list_argument(command);

    // Update the environment variables containing the list of disabled and
    // enabled warnings.
    update_disabled_enabled_warnings_list_var();
  }

  // Call set_once_property() to set once_propetry flag.
  if (disable_warnings && once_prop) set_once_property(P_WARN, true);

  command->last_argument = command->end;
}

/// Create a list of error values that the next statement is expected
/// to return. Each error must be an error number or an SQLSTATE value
/// or a symbolic error name representing an error.
///
/// SQLSTATE value must start with 'S'. It is also possible to specify
/// client errors with 'error' command.
///
/// @code
/// --error 1064
/// --error S42S01
/// --error ER_TABLE_EXISTS_ERROR,ER_PARSE_ERROR
/// --error CR_SERVER_GONE_ERROR
/// @endcode
///
/// @param command Pointer to the st_command structure which holds the
///                arguments and information for the command.
static void do_error(struct st_command *command) {
  if (!*command->first_argument) die("Missing argument(s) to 'error' command.");

  // Check if error command ends with a comment
  char *end = command->first_argument + std::strlen(command->first_argument);
  while (std::strlen(command->first_argument) > std::strlen(end)) {
    end--;
    if (*end == '#') break;
  }

  if (std::strlen(command->first_argument) == std::strlen(end))
    end = command->first_argument + std::strlen(command->first_argument);

  std::string error;
  std::stringstream errors(std::string(command->first_argument, end));

  // Get error codes
  while (std::getline(errors, error, ',')) {
    // Remove any space from the string representing an error.
    error.erase(remove_if(error.begin(), error.end(), isspace), error.end());

    // Code to handle a variable containing an error.
    if (error.front() == '$') {
      const char *varname_end = nullptr;
      VAR *var = var_get(error.c_str(), &varname_end, false, false);
      error.assign(var->str_val);
    }

    if (error.front() == 'S') {
      // SQLSTATE string
      //   * Must be SQLSTATE_LENGTH long
      //   * May contain only digits[0-9] and _uppercase_ letters

      // Step pass 'S' character
      error = error.substr(1, error.length());

      if (error.length() != SQLSTATE_LENGTH)
        die("The sqlstate must be exactly %d chars long.", SQLSTATE_LENGTH);

      // Check the validity of an SQLSTATE string.
      for (std::size_t i = 0; i < error.length(); i++) {
        if (!my_isdigit(charset_info, error[i]) &&
            !my_isupper(charset_info, error[i]))
          die("The sqlstate may only consist of digits[0-9] and _uppercase_ "
              "letters.");
      }
      expected_errors->add_error(error.c_str(), ERR_SQLSTATE);
    } else if (error.front() == 's') {
      die("The sqlstate definition must start with an uppercase S.");
    }
    // Checking for both server error names as well as client error names.
    else if (error.front() == 'C' || error.front() == 'E') {
      // Code to handle --error <error_string>.
      int error_code = get_errcode_from_name(error);
      if (error_code == -1) die("Unknown SQL error name '%s'.", error.c_str());
      expected_errors->add_error(error_code, ERR_ERRNO);
    } else if (error.front() == 'c' || error.front() == 'e') {
      die("The error name definition must start with an uppercase C or E.");
    } else {
      // Check that the string passed to error command contains only digits.
      int error_code = get_int_val(error.c_str());
      if (error_code == -1)
        die("Invalid argument '%s' to 'error' command, the argument may "
            "consist of either symbolic error names or error codes.",
            command->first_argument);
      expected_errors->add_error((std::uint32_t)error_code, ERR_ERRNO);
    }

    if (expected_errors->count() >= MAX_ERROR_COUNT)
      die("Too many errorcodes specified.");
  }

  command->last_argument = command->end;
}

/*
  Get a string;  Return ptr to end of string
  Strings may be surrounded by " or '

  If string is a '$variable', return the value of the variable.
*/

static char *get_string(char **to_ptr, const char **from_ptr,
                        struct st_command *command) {
  char c, sep;
  char *to = *to_ptr, *start = to;
  const char *from = *from_ptr;
  DBUG_TRACE;

  /* Find separator */
  if (*from == '"' || *from == '\'')
    sep = *from++;
  else
    sep = ' '; /* Separated with space */

  for (; (c = *from); from++) {
    if (c == '\\' && from[1]) { /* Escaped character */
      /* We can't translate \0 -> ASCII 0 as replace can't handle ASCII 0 */
      switch (*++from) {
        case 'n':
          *to++ = '\n';
          break;
        case 't':
          *to++ = '\t';
          break;
        case 'r':
          *to++ = '\r';
          break;
        case 'b':
          *to++ = '\b';
          break;
        case 'Z': /* ^Z must be escaped on Win32 */
          *to++ = '\032';
          break;
        default:
          *to++ = *from;
          break;
      }
    } else if (c == sep) {
      if (c == ' ' || c != *++from) break; /* Found end of string */
      *to++ = c;                           /* Copy duplicated separator */
    } else
      *to++ = c;
  }
  if (*from != ' ' && *from) die("Wrong string argument in %s", command->query);

  while (my_isspace(charset_info, *from)) /* Point to next string */
    from++;

  *to = 0;          /* End of string marker */
  *to_ptr = to + 1; /* Store pointer to end */
  *from_ptr = from;

  /* Check if this was a variable */
  if (*start == '$') {
    const char *end = to;
    VAR *var = var_get(start, &end, false, true);
    if (var && to == end + 1) {
      DBUG_PRINT("info", ("var: '%s' -> '%s'", start, var->str_val));
      return var->str_val; /* return found variable value */
    }
  }
  return start;
}

static void set_reconnect(MYSQL *mysql, int val) {
  bool reconnect = val;
  DBUG_TRACE;
  DBUG_PRINT("info", ("val: %d", val));
  mysql_options(mysql, MYSQL_OPT_RECONNECT, (char *)&reconnect);
}

/**
  Change the current connection to the given st_connection, and update
  $mysql_get_server_version and $CURRENT_CONNECTION accordingly.
*/
static void set_current_connection(struct st_connection *con) {
  cur_con = con;
  /* Update $mysql_get_server_version to that of current connection */
  var_set_int("$mysql_get_server_version",
              mysql_get_server_version(&con->mysql));
  /* Update $CURRENT_CONNECTION to the name of the current connection */
  var_set_string("$CURRENT_CONNECTION", con->name);
}

static void select_connection_name(const char *name) {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("name: '%s'", name));
  st_connection *con = find_connection_by_name(name);

  if (!con) die("connection '%s' not found in connection pool", name);

  set_current_connection(con);

  /* Connection logging if enabled */
  if (!disable_connect_log && !disable_query_log) {
    DYNAMIC_STRING *ds = &ds_res;

    dynstr_append_mem(ds, "connection ", 11);
    replace_dynstr_append(ds, name);
    dynstr_append_mem(ds, ";\n", 2);
  }
}

static void select_connection(struct st_command *command) {
  DBUG_TRACE;
  static DYNAMIC_STRING ds_connection;
  const struct command_arg connection_args[] = {
      {"connection_name", ARG_STRING, true, &ds_connection,
       "Name of the connection that we switch to."}};
  check_command_args(command, command->first_argument, connection_args,
                     sizeof(connection_args) / sizeof(struct command_arg), ' ');

  DBUG_PRINT("info", ("changing connection: %s", ds_connection.str));
  select_connection_name(ds_connection.str);
  dynstr_free(&ds_connection);
}

static void do_close_connection(struct st_command *command) {
  DBUG_TRACE;

  struct st_connection *con;
  static DYNAMIC_STRING ds_connection;
  const struct command_arg close_connection_args[] = {
      {"connection_name", ARG_STRING, true, &ds_connection,
       "Name of the connection to close."}};
  check_command_args(command, command->first_argument, close_connection_args,
                     sizeof(close_connection_args) / sizeof(struct command_arg),
                     ' ');

  DBUG_PRINT("enter", ("connection name: '%s'", ds_connection.str));

  if (!(con = find_connection_by_name(ds_connection.str)))
    die("connection '%s' not found in connection pool", ds_connection.str);

  DBUG_PRINT("info", ("Closing connection %s", con->name));
  if (command->type == Q_DIRTY_CLOSE) {
    if (con->mysql.net.vio) {
      vio_delete(con->mysql.net.vio);
      con->mysql.net.vio = nullptr;
      end_server(&con->mysql);
    }
  }
  if (con->stmt) mysql_stmt_close(con->stmt);
  con->stmt = nullptr;

  mysql_close(&con->mysql);

  if (con->util_mysql) mysql_close(con->util_mysql);
  con->util_mysql = nullptr;
  con->pending = false;

  my_free(con->name);

  /*
    When the connection is closed set name to "-closed_connection-"
    to make it possible to reuse the connection name.
  */
  if (!(con->name = my_strdup(PSI_NOT_INSTRUMENTED, "-closed_connection-",
                              MYF(MY_WME))))
    die("Out of memory");

  if (con == cur_con) {
    /* Current connection was closed */
    var_set_int("$mysql_get_server_version", 0xFFFFFFFF);
    var_set_string("$CURRENT_CONNECTION", con->name);
  }

  /* Connection logging if enabled */
  if (!disable_connect_log && !disable_query_log) {
    DYNAMIC_STRING *ds = &ds_res;

    dynstr_append_mem(ds, "disconnect ", 11);
    replace_dynstr_append(ds, ds_connection.str);
    dynstr_append_mem(ds, ";\n", 2);
  }

  dynstr_free(&ds_connection);
}

/* Append a line to a dynamic string. */
void dynstr_append_line(DYNAMIC_STRING *ds, const char *msg, int len) {
  dynstr_append_mem(ds, msg, len);
  dynstr_append_mem(ds, "\n", 1);
}

/*
  When mysql global sys var "send_error_before_closing_timed_out_connection"
  is turned on, mysqld will push error 2006 with the following message:
  "Closed connection: idle timeout after <wait_timeout>s"
  into socket before closing it due to timeout. This error will sit in the
  socket buffer on client side so that client will be able to find out the
  reason of lost connection or mysql server has gone away. This function will
  try to read this error from socket buffer and verify if the packet is valid.
*/
void dump_timed_out_connection_socket_buffer(struct st_connection *con) {
  if (!con->mysql.net.vio) return;
  DYNAMIC_STRING ds;
  init_dynamic_string(&ds, "", 2048, 2048);
  /* vio read buffer size is 16KB */
  uchar buf[VIO_READ_BUFFER_SIZE + 1];
  int sz = vio_read(con->mysql.net.vio, (uchar *)buf, VIO_READ_BUFFER_SIZE);
  bool ok = true;
  char err_msg[256];
  buf[sz] = 0;
  // buf[0..2] is the length of the message, will check it later
  int message_len = sint3korr(&buf[0]);
  // buf[3] is the packet serial number, which has been forced to 1
  int packet_serial_num = buf[3];
  if (packet_serial_num != 1) {
    ok = false;
    sprintf(err_msg,
            "Error: packet serial number in buf[3] is %d, "
            "but 1 is expected;",
            packet_serial_num);
    dynstr_append_line(&ds, err_msg, sizeof(err_msg));
  }
  // buf[4] is 255
  if (buf[4] != 255) {
    ok = false;
    sprintf(err_msg, "Error: buf[4] != 255;");
    dynstr_append_line(&ds, err_msg, sizeof(err_msg));
  }
  // buf[5..6] is the error code 2006
  int err_code = sint2korr(&buf[5]);
  if (err_code != 2006) {
    ok = false;
    sprintf(err_msg,
            "Error: error code in buf[5..6] is %d, "
            "but 2006 is expected;",
            err_code);
    dynstr_append_line(&ds, err_msg, sizeof(err_msg));
  }
  // buf[7] is '#'
  if (buf[7] != '#') {
    ok = false;
    sprintf(err_msg, "Error: buf[7] != #;");
    dynstr_append_line(&ds, err_msg, sizeof(err_msg));
  }
  // Starting from buf[8] is the sql state "HY000" + message
  // The message is as below, wait_timeout is a unsigned integer
  // HY000Closed connection: idle timeout after <wait_timeout>s
  static const char *msg_body = "HY000Closed connection: idle timeout after ";
  int msg_body_len = strlen(msg_body);
  uchar *end = NULL;
  if (strncmp((const char *)&buf[8], msg_body, msg_body_len) == 0) {
    // Skip the wait_timeout value, which is a unsigned integer,
    // followed by 's', which is the last character of the string
    end = &buf[8] + msg_body_len;
    while (*end >= '0' && *end <= '9') ++end;
    if (*end != 's' || *(++end) != '\0') ok = false;
  } else
    ok = false;
  if (ok) {
    // It is time to verify the packet and message length
    DBUG_ASSERT(*end == '\0');
    int packet_len = end - &buf[0];
    // packet_len is the length of the whole network packet
    // that includes 4 bytes of header (3 bytes of payload
    // length followed by 1 byte of serial number) plus 42
    // bytes of payload (the error message string)
    if (sz != packet_len) {
      sprintf(err_msg,
              "Error: packet length is %d, "
              "but %d is expected;",
              sz, packet_len);
      dynstr_append_line(&ds, err_msg, sizeof(err_msg));
      log_file.write(ds.str, ds.length);
      log_file.flush();
      dynstr_free(&ds);
      return;
    }
    // The header length is 4 bytes
    if (message_len + 4 != packet_len) {
      ok = false;
      sprintf(err_msg,
              "Error: message length in buf[0..2] is %d, "
              "but %d is expected;",
              message_len, packet_len - 4);
      dynstr_append_line(&ds, err_msg, sizeof(err_msg));
    }
  }
  if (ok) {
    // Print the error message if no error was found
    const char *err_str = "Error 2006: ";
    replace_dynstr_append_mem(&ds, err_str, strlen(err_str));
    replace_dynstr_append_mem(&ds, (const char *)&buf[8], sz - 8);
    dynstr_append_mem(&ds, "\n", 1);
  } else {
    const char *err_str = "Error message is wrong: ";
    dynstr_append_mem(&ds, err_str, std::strlen(err_str));
    dynstr_append_line(&ds, reinterpret_cast<char *>(&buf[8]), sz - 8);
  }
  log_file.write(ds.str, ds.length);
  log_file.flush();
  dynstr_free(&ds);
}

/*
  Connect to a server doing several retries if needed.

  SYNOPSIS
  safe_connect()
  con               - connection structure to be used
  host, user, pass, - connection parameters
  db, port, sock

  NOTE

  Sometimes in a test the client starts before
  the server - to solve the problem, we try again
  after some sleep if connection fails the first
  time

  This function will try to connect to the given server
  "opt_max_connect_retries" times and sleep "connection_retry_sleep"
  seconds between attempts before finally giving up.
  This helps in situation when the client starts
  before the server (which happens sometimes).
  It will only ignore connection errors during these retries.

*/

static void safe_connect(MYSQL *mysql, const char *name, const char *host,
                         const char *user, const char *pass, const char *db,
                         int port, const char *sock) {
  int failed_attempts = 0;

  DBUG_TRACE;

  verbose_msg(
      "Connecting to server %s:%d (socket %s) as '%s'"
      ", connection '%s', attempt %d ...",
      host, port, sock, user, name, failed_attempts);

  mysql_options(mysql, MYSQL_OPT_CONNECT_ATTR_RESET, nullptr);
  mysql_options4(mysql, MYSQL_OPT_CONNECT_ATTR_ADD, "program_name",
                 "mysqltest");
  mysql_options(mysql, MYSQL_OPT_CAN_HANDLE_EXPIRED_PASSWORDS,
                &can_handle_expired_passwords);
  while (!mysql_real_connect_wrapper(
      mysql, host, user, pass, db, port, sock,
      CLIENT_MULTI_STATEMENTS | CLIENT_REMEMBER_OPTIONS)) {
    /*
      Connect failed

      Only allow retry if this was an error indicating the server
      could not be contacted. Error code differs depending
      on protocol/connection type
    */

    if ((mysql_errno(mysql) == CR_CONN_HOST_ERROR ||
         mysql_errno(mysql) == CR_CONNECTION_ERROR ||
         mysql_errno(mysql) == CR_NAMEDPIPEOPEN_ERROR) &&
        failed_attempts < opt_max_connect_retries) {
      verbose_msg("Connect attempt %d/%d failed: %d: %s", failed_attempts,
                  opt_max_connect_retries, mysql_errno(mysql),
                  mysql_error(mysql));
      my_sleep(connection_retry_sleep);
    } else {
      if (failed_attempts > 0)
        die("Could not open connection '%s' after %d attempts: %d %s", name,
            failed_attempts, mysql_errno(mysql), mysql_error(mysql));
      else
        die("Could not open connection '%s': %d %s", name, mysql_errno(mysql),
            mysql_error(mysql));
    }
    failed_attempts++;
  }
  verbose_msg("... Connected.");
}

/// Connect to a server and handle connection errors in case they
/// occur.
///
/// This function will try to establish a connection to server and
/// handle possible errors in the same manner as if "connect" was usual
/// SQL-statement. If an error is expected it will ignore it once it
/// occurs and log the "statement" to the query log. Unlike
/// safe_connect() it won't do several attempts.
///
/// @param command Pointer to the st_command structure which holds the
///                     arguments and information for the command.
/// @param con     Connection structure to be used
/// @param host    Host name
/// @param user    User name
/// @param pass    Password
/// @param db      Database name
/// @param port    Port number
/// @param sock    Socket value
///
/// @retval 1 if connection succeeds, 0 otherwise
static int connect_n_handle_errors(struct st_command *command, MYSQL *con,
                                   const char *host, const char *user,
                                   const char *pass, const char *db, int port,
                                   const char *sock) {
  DYNAMIC_STRING *ds;
  int failed_attempts = 0;

  ds = &ds_res;

  /* Only log if an error is expected */
  if (expected_errors->count() > 0 && !disable_query_log) {
    /*
      Log the connect to result log
    */
    dynstr_append_mem(ds, "connect(", 8);
    replace_dynstr_append(ds, host);
    dynstr_append_mem(ds, ",", 1);
    replace_dynstr_append(ds, user);
    dynstr_append_mem(ds, ",", 1);
    replace_dynstr_append(ds, pass);
    dynstr_append_mem(ds, ",", 1);
    if (db) replace_dynstr_append(ds, db);
    dynstr_append_mem(ds, ",", 1);
    replace_dynstr_append_uint(ds, port);
    dynstr_append_mem(ds, ",", 1);
    if (sock) replace_dynstr_append(ds, sock);
    dynstr_append_mem(ds, ")", 1);
    dynstr_append_mem(ds, delimiter, delimiter_length);
    dynstr_append_mem(ds, "\n", 1);
  }
  /* Simlified logging if enabled */
  if (!disable_connect_log && !disable_query_log) {
    replace_dynstr_append(ds, command->query);
    dynstr_append_mem(ds, ";\n", 2);
  }

  mysql_options4(con, MYSQL_OPT_CONNECT_ATTR_ADD, "program_name", "mysqltest");
  mysql_options(con, MYSQL_OPT_CAN_HANDLE_EXPIRED_PASSWORDS,
                &can_handle_expired_passwords);
  while (!mysql_real_connect_wrapper(con, host, user, pass, db, port,
                                     sock ? sock : nullptr,
                                     CLIENT_MULTI_STATEMENTS)) {
    /*
      If we have used up all our connections check whether this
      is expected (by --error). If so, handle the error right away.
      Otherwise, give it some extra time to rule out race-conditions.
      If extra-time doesn't help, we have an unexpected error and
      must abort -- just proceeding to handle_error() when second
      and third chances are used up will handle that for us.

      There are various user-limits of which only max_user_connections
      and max_connections_per_hour apply at connect time. For the
      the second to create a race in our logic, we'd need a limits
      test that runs without a FLUSH for longer than an hour, so we'll
      stay clear of trying to work out which exact user-limit was
      exceeded.
    */

    if (((mysql_errno(con) == ER_TOO_MANY_USER_CONNECTIONS) ||
         (mysql_errno(con) == ER_USER_LIMIT_REACHED)) &&
        (failed_attempts++ < opt_max_connect_retries)) {
      int i;

      i = match_expected_error(command, mysql_errno(con), mysql_sqlstate(con));

      if (i >= 0) goto do_handle_error; /* expected error, handle */

      my_sleep(connection_retry_sleep); /* unexpected error, wait */
      continue;                         /* and give it 1 more chance */
    }

  do_handle_error:
    var_set_errno(mysql_errno(con));
    handle_error(command, mysql_errno(con), mysql_error(con),
                 mysql_sqlstate(con), ds);
    return 0; /* Not connected */
  }

  var_set_errno(0);
  handle_no_error(command);
  revert_properties();
  return 1; /* Connected */
}

/*
  Open a new connection to MySQL Server with the parameters
  specified. Make the new connection the current connection.

  SYNOPSIS
  do_connect()
  q	       called command

  DESCRIPTION
  connect(<name>,<host>,<user>,[<pass>,[<db>,[<port>,<sock>[<opts>]]]]);
  connect <name>,<host>,<user>,[<pass>,[<db>,[<port>,<sock>[<opts>]]]];

  <name> - name of the new connection
  <host> - hostname of server
  <user> - user to connect as
  <pass> - password used when connecting
  <db>   - initial db when connected
  <port> - server port
  <sock> - server socket
  <opts> - options to use for the connection
   * SSL - use SSL if available
   * COMPRESS - use compression if available
   * SHM - use shared memory if available
   * PIPE - use named pipe if available
   * SOCKET - use socket protocol
   * TCP - use tcp protocol
*/

static void do_connect(struct st_command *command) {
  int con_port = opt_port;
  char *con_options;
  bool con_ssl = false, con_compress = false;
  bool con_pipe = false, con_shm = false, con_cleartext_enable = false;
  bool con_timeout_1s = false, con_timeout_1500ms = false;
  struct st_connection *con_slot;
  uint save_opt_ssl_mode = opt_ssl_mode;

  static DYNAMIC_STRING ds_connection_name;
  static DYNAMIC_STRING ds_host;
  static DYNAMIC_STRING ds_user;
  static DYNAMIC_STRING ds_password;
  static DYNAMIC_STRING ds_database;
  static DYNAMIC_STRING ds_port;
  static DYNAMIC_STRING ds_sock;
  static DYNAMIC_STRING ds_options;
  static DYNAMIC_STRING ds_default_auth;
  static DYNAMIC_STRING ds_shm;
  static DYNAMIC_STRING ds_compression_algorithm;
  static DYNAMIC_STRING ds_zstd_compression_level;
  const struct command_arg connect_args[] = {
      {"connection name", ARG_STRING, true, &ds_connection_name,
       "Name of the connection"},
      {"host", ARG_STRING, true, &ds_host, "Host to connect to"},
      {"user", ARG_STRING, false, &ds_user, "User to connect as"},
      {"passsword", ARG_STRING, false, &ds_password,
       "Password used when connecting"},
      {"database", ARG_STRING, false, &ds_database,
       "Database to select after connect"},
      {"port", ARG_STRING, false, &ds_port, "Port to connect to"},
      {"socket", ARG_STRING, false, &ds_sock, "Socket to connect with"},
      {"options", ARG_STRING, false, &ds_options,
       "Options to use while connecting"},
      {"default_auth", ARG_STRING, false, &ds_default_auth,
       "Default authentication to use"},
      {"default_compression_algorithm", ARG_STRING, false,
       &ds_compression_algorithm, "Default compression algorithm to use"},
      {"default_zstd_compression_level", ARG_STRING, false,
       &ds_zstd_compression_level,
       "Default compression level to use "
       "when using zstd compression."}};

  DBUG_TRACE;
  DBUG_PRINT("enter", ("connect: %s", command->first_argument));

  strip_parentheses(command);
  check_command_args(command, command->first_argument, connect_args,
                     sizeof(connect_args) / sizeof(struct command_arg), ',');

  /* Port */
  if (ds_port.length) {
    con_port = atoi(ds_port.str);
    if (con_port == 0) die("Illegal argument for port: '%s'", ds_port.str);
  }

  /* Shared memory */
  init_dynamic_string(&ds_shm, ds_sock.str, 0, 0);

  /* Sock */
  if (ds_sock.length) {
    /*
      If the socket is specified just as a name without path
      append tmpdir in front
    */
    if (*ds_sock.str != FN_LIBCHAR) {
      char buff[FN_REFLEN];
      fn_format(buff, ds_sock.str, TMPDIR, "", 0);
      dynstr_set(&ds_sock, buff);
    }
  } else {
    /* No socket specified, use default */
    dynstr_set(&ds_sock, unix_sock);
  }
  DBUG_PRINT("info", ("socket: %s", ds_sock.str));

  /* Options */
  con_options = ds_options.str;
  bool con_socket = false, con_tcp = false;
  while (*con_options) {
    /* Step past any spaces in beginning of option */
    while (*con_options && my_isspace(charset_info, *con_options))
      con_options++;

    /* Find end of this option */
    char *end = con_options;
    while (*end && !my_isspace(charset_info, *end)) end++;

    size_t con_option_len = end - con_options;
    const size_t cur_con_option_len = 32;
    char cur_con_option[cur_con_option_len + 1] = {};
    DBUG_ASSERT(cur_con_option_len >= con_option_len);
    strmake(cur_con_option, con_options,
            std::min(cur_con_option_len, con_option_len));

    if (!std::strcmp(cur_con_option, "SSL"))
      con_ssl = true;
    else if (!std::strcmp(cur_con_option, "COMPRESS"))
      con_compress = true;
    else if (!std::strcmp(cur_con_option, "TIMEOUT_1S"))
      con_timeout_1s = true;
    else if (!std::strcmp(cur_con_option, "TIMEOUT_1500MS"))
      con_timeout_1500ms = true;
    else if (!std::strcmp(cur_con_option, "PIPE"))
      con_pipe = true;
    else if (!std::strcmp(cur_con_option, "SHM"))
      con_shm = true;
    else if (!std::strcmp(cur_con_option, "CLEARTEXT"))
      con_cleartext_enable = true;
    else if (!std::strcmp(cur_con_option, "SOCKET"))
      con_socket = true;
    else if (!std::strcmp(cur_con_option, "TCP"))
      con_tcp = true;
    else
      die("Illegal option to connect: %s", cur_con_option);

    /* Process next option */
    con_options = end;
  }

  if (find_connection_by_name(ds_connection_name.str))
    die("Connection %s already exists", ds_connection_name.str);

  if (next_con != connections_end)
    con_slot = next_con;
  else {
    if (!(con_slot = find_connection_by_name("-closed_connection-")))
      die("Connection limit exhausted, you can have max %d connections",
          opt_max_connections);
  }

  if (!mysql_init(&con_slot->mysql)) die("Failed on mysql_init()");

  if (opt_connect_timeout)
    mysql_options(&con_slot->mysql, MYSQL_OPT_CONNECT_TIMEOUT,
                  (void *)&opt_connect_timeout);

  if (opt_compress || con_compress)
    mysql_options(&con_slot->mysql, MYSQL_OPT_COMPRESS, NullS);

  if (con_timeout_1s) {
    int timeout = 1;
    mysql_options(&con_slot->mysql, MYSQL_OPT_READ_TIMEOUT, &timeout);
    mysql_options(&con_slot->mysql, MYSQL_OPT_WRITE_TIMEOUT, &timeout);
    mysql_options(&con_slot->mysql, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);
  } else if (con_timeout_1500ms) {
    int timeout = 1500;
    mysql_options(&con_slot->mysql, MYSQL_OPT_READ_TIMEOUT_MS, &timeout);
    mysql_options(&con_slot->mysql, MYSQL_OPT_WRITE_TIMEOUT_MS, &timeout);
    mysql_options(&con_slot->mysql, MYSQL_OPT_CONNECT_TIMEOUT_MS, &timeout);
  }
  mysql_options(&con_slot->mysql, MYSQL_OPT_LOCAL_INFILE, nullptr);
  mysql_options(&con_slot->mysql, MYSQL_SET_CHARSET_NAME, charset_info->csname);
  if (opt_charsets_dir)
    mysql_options(&con_slot->mysql, MYSQL_SET_CHARSET_DIR, opt_charsets_dir);

  /* FB - reset the connection attrs before processing compression */
  mysql_options(&con_slot->mysql, MYSQL_OPT_CONNECT_ATTR_RESET, nullptr);

  if (ds_compression_algorithm.length)
    mysql_options(&con_slot->mysql, MYSQL_OPT_COMPRESSION_ALGORITHMS,
                  ds_compression_algorithm.str);
  else if (opt_compress_algorithm)
    mysql_options(&con_slot->mysql, MYSQL_OPT_COMPRESSION_ALGORITHMS,
                  opt_compress_algorithm);
  if (ds_zstd_compression_level.length) {
    char *end = nullptr;
    opt_zstd_compress_level = strtol(ds_zstd_compression_level.str, &end, 10);
  }
  mysql_options(&con_slot->mysql, MYSQL_OPT_ZSTD_COMPRESSION_LEVEL,
                &opt_zstd_compress_level);

  /*
    If mysqltest --ssl-mode option is set to DISABLED
    and connect(.., SSL) command used, set proper opt_ssl_mode.

    So, SSL connection is used either:
    a) mysqltest --ssl-mode option is NOT set to DISABLED or
    b) connect(.., SSL) command used.
  */
  if (opt_ssl_mode == SSL_MODE_DISABLED && con_ssl) {
    opt_ssl_mode =
        (opt_ssl_ca || opt_ssl_capath) ? SSL_MODE_VERIFY_CA : SSL_MODE_REQUIRED;
  }
  if (SSL_SET_OPTIONS(&con_slot->mysql)) die("%s", SSL_SET_OPTIONS_ERROR);
  opt_ssl_mode = save_opt_ssl_mode;

  if (con_pipe && !con_ssl) {
    opt_protocol = MYSQL_PROTOCOL_PIPE;
  }

  if (opt_protocol) {
    mysql_options(&con_slot->mysql, MYSQL_OPT_PROTOCOL, (char *)&opt_protocol);
    /*
      Resetting the opt_protocol value to 0 to avoid the
      possible failure in the next connect() command.
    */
    opt_protocol = 0;
  }

  if (con_shm) {
    uint protocol = MYSQL_PROTOCOL_MEMORY;
    if (!ds_shm.length) die("Missing shared memory base name");

    mysql_options(&con_slot->mysql, MYSQL_SHARED_MEMORY_BASE_NAME, ds_shm.str);
    mysql_options(&con_slot->mysql, MYSQL_OPT_PROTOCOL, &protocol);
  } else if (shared_memory_base_name) {
    mysql_options(&con_slot->mysql, MYSQL_SHARED_MEMORY_BASE_NAME,
                  shared_memory_base_name);
  }

  if (con_socket) {
    uint protocol = MYSQL_PROTOCOL_SOCKET;
    mysql_options(&con_slot->mysql, MYSQL_OPT_PROTOCOL, &protocol);
  }

  if (con_tcp) {
    uint protocol = MYSQL_PROTOCOL_TCP;
    mysql_options(&con_slot->mysql, MYSQL_OPT_PROTOCOL, &protocol);
  }

  /* Use default db name */
  if (ds_database.length == 0) dynstr_set(&ds_database, opt_db);

  if (opt_plugin_dir && *opt_plugin_dir)
    mysql_options(&con_slot->mysql, MYSQL_PLUGIN_DIR, opt_plugin_dir);

  if (ds_default_auth.length)
    mysql_options(&con_slot->mysql, MYSQL_DEFAULT_AUTH, ds_default_auth.str);

  /* Set server public_key */
  set_server_public_key(&con_slot->mysql);

  set_get_server_public_key_option(&con_slot->mysql);

  if (con_cleartext_enable)
    mysql_options(&con_slot->mysql, MYSQL_ENABLE_CLEARTEXT_PLUGIN,
                  (char *)&con_cleartext_enable);

  /* Special database to allow one to connect without a database name */
  if (ds_database.length && !std::strcmp(ds_database.str, "*NO-ONE*"))
    dynstr_set(&ds_database, "");

  if (connect_n_handle_errors(command, &con_slot->mysql, ds_host.str,
                              ds_user.str, ds_password.str, ds_database.str,
                              con_port, ds_sock.str)) {
    DBUG_PRINT("info", ("Inserting connection %s in connection pool",
                        ds_connection_name.str));
    my_free(con_slot->name);
    if (!(con_slot->name = my_strdup(PSI_NOT_INSTRUMENTED,
                                     ds_connection_name.str, MYF(MY_WME))))
      die("Out of memory");
    con_slot->name_len = std::strlen(con_slot->name);
    set_current_connection(con_slot);

    if (con_slot == next_con)
      next_con++; /* if we used the next_con slot, advance the pointer */
  }

  dynstr_free(&ds_connection_name);
  dynstr_free(&ds_host);
  dynstr_free(&ds_user);
  dynstr_free(&ds_password);
  dynstr_free(&ds_database);
  dynstr_free(&ds_port);
  dynstr_free(&ds_sock);
  dynstr_free(&ds_options);
  dynstr_free(&ds_default_auth);
  dynstr_free(&ds_shm);
  dynstr_free(&ds_compression_algorithm);
  dynstr_free(&ds_zstd_compression_level);
}

static int do_done(struct st_command *command) {
  /* Check if empty block stack */
  if (cur_block == block_stack) {
    if (*command->query != '}')
      die("Stray 'end' command - end of block before beginning");
    die("Stray '}' - end of block before beginning");
  }

  /* Test if inner block has been executed */
  if (cur_block->ok && cur_block->cmd == cmd_while) {
    /* Pop block from stack, re-execute outer block */
    cur_block--;
    parser.current_line = cur_block->line;
  } else {
    if (*cur_block->delim) {
      /* Restore "old" delimiter after false if block */
      strcpy(delimiter, cur_block->delim);
      delimiter_length = std::strlen(delimiter);
    }
    /* Pop block from stack, goto next line */
    cur_block--;
    parser.current_line++;
  }
  return 0;
}

/* Operands available in if or while conditions */

enum block_op { EQ_OP, NE_OP, GT_OP, GE_OP, LT_OP, LE_OP, ILLEG_OP };

static enum block_op find_operand(const char *start) {
  char first = *start;
  char next = *(start + 1);

  if (first == '=' && next == '=') return EQ_OP;
  if (first == '!' && next == '=') return NE_OP;
  if (first == '>' && next == '=') return GE_OP;
  if (first == '>') return GT_OP;
  if (first == '<' && next == '=') return LE_OP;
  if (first == '<') return LT_OP;

  return ILLEG_OP;
}

/*
  Process start of a "if" or "while" statement

  SYNOPSIS
  do_block()
  cmd        Type of block
  q	       called command

  DESCRIPTION
  if ([!]<expr>)
  {
  <block statements>
  }

  while ([!]<expr>)
  {
  <block statements>
  }

  Evaluates the <expr> and if it evaluates to
  greater than zero executes the following code block.
  A '!' can be used before the <expr> to indicate it should
  be executed if it evaluates to zero.

  <expr> can also be a simple comparison condition:

  <variable> <op> <expr>

  The left hand side must be a variable, the right hand side can be a
  variable, number, string or `query`. Operands are ==, !=, <, <=, >, >=.
  == and != can be used for strings, all can be used for numerical values.
*/

static void do_block(enum block_cmd cmd, struct st_command *command) {
  const char *p = command->first_argument;
  const char *expr_start;
  const char *expr_end;
  VAR v;
  const char *cmd_name = (cmd == cmd_while ? "while" : "if");
  bool not_expr = false;
  DBUG_TRACE;
  DBUG_PRINT("enter", ("%s", cmd_name));

  /* Check stack overflow */
  if (cur_block == block_stack_end) die("Nesting too deeply");

  /* Set way to find outer block again, increase line counter */
  cur_block->line = parser.current_line++;

  /* If this block is ignored */
  if (!cur_block->ok) {
    /* Inner block should be ignored too */
    cur_block++;
    cur_block->cmd = cmd;
    cur_block->ok = false;
    cur_block->delim[0] = '\0';
    return;
  }

  /* Parse and evaluate test expression */
  expr_start = strchr(p, '(');
  if (!expr_start++) die("missing '(' in %s", cmd_name);

  while (my_isspace(charset_info, *expr_start)) expr_start++;

  /* Check for !<expr> */
  if (*expr_start == '!') {
    not_expr = true;
    expr_start++; /* Step past the '!', then any whitespace */
    while (*expr_start && my_isspace(charset_info, *expr_start)) expr_start++;
  }
  /* Find ending ')' */
  expr_end = strrchr(expr_start, ')');
  if (!expr_end) die("missing ')' in %s", cmd_name);
  p = expr_end + 1;

  while (*p && my_isspace(charset_info, *p)) p++;
  if (*p && *p != '{') die("Missing '{' after %s. Found \"%s\"", cmd_name, p);

  var_init(&v, nullptr, 0, nullptr, 0);

  /* If expression starts with a variable, it may be a compare condition */

  if (*expr_start == '$') {
    const char *curr_ptr = expr_end;
    eval_expr(&v, expr_start, &curr_ptr, true);
    while (my_isspace(charset_info, *++curr_ptr)) {
    }
    /* If there was nothing past the variable, skip condition part */
    if (curr_ptr == expr_end) goto NO_COMPARE;

    enum block_op operand = find_operand(curr_ptr);
    if (operand == ILLEG_OP)
      die("Found junk '%.*s' after $variable in condition",
          (int)(expr_end - curr_ptr), curr_ptr);

    /* We could silently allow this, but may be confusing */
    if (not_expr)
      die("Negation and comparison should not be combined, please rewrite");

    /* Skip the 1 or 2 chars of the operand, then white space */
    if (operand == LT_OP || operand == GT_OP) {
      curr_ptr++;
    } else {
      curr_ptr += 2;
    }
    while (my_isspace(charset_info, *curr_ptr)) curr_ptr++;
    if (curr_ptr == expr_end) die("Missing right operand in comparison");

    /* Strip off trailing white space */
    while (my_isspace(charset_info, expr_end[-1])) expr_end--;
    /* strip off ' or " around the string */
    if (*curr_ptr == '\'' || *curr_ptr == '"') {
      if (expr_end[-1] != *curr_ptr) die("Unterminated string value");
      curr_ptr++;
      expr_end--;
    }
    VAR v2;
    var_init(&v2, nullptr, 0, nullptr, 0);
    eval_expr(&v2, curr_ptr, &expr_end);

    if ((operand != EQ_OP && operand != NE_OP) && !(v.is_int && v2.is_int))
      die("Only == and != are supported for string values");

    /* Now we overwrite the first variable with 0 or 1 (for false or true) */

    switch (operand) {
      case EQ_OP:
        if (v.is_int)
          v.int_val = (v2.is_int && v2.int_val == v.int_val);
        else
          v.int_val = !std::strcmp(v.str_val, v2.str_val);
        break;

      case NE_OP:
        if (v.is_int)
          v.int_val = !(v2.is_int && v2.int_val == v.int_val);
        else
          v.int_val = (std::strcmp(v.str_val, v2.str_val) != 0);
        break;

      case LT_OP:
        v.int_val = (v.int_val < v2.int_val);
        break;
      case LE_OP:
        v.int_val = (v.int_val <= v2.int_val);
        break;
      case GT_OP:
        v.int_val = (v.int_val > v2.int_val);
        break;
      case GE_OP:
        v.int_val = (v.int_val >= v2.int_val);
        break;
      case ILLEG_OP:
        die("Impossible operator, this cannot happen");
    }

    v.is_int = true;
    var_free()(&v2);
  } else {
    if (*expr_start != '`' && !my_isdigit(charset_info, *expr_start))
      die("Expression in if/while must beging with $, ` or a number");
    eval_expr(&v, expr_start, &expr_end);
  }

NO_COMPARE:
  /* Define inner block */
  cur_block++;
  cur_block->cmd = cmd;
  if (v.is_int) {
    cur_block->ok = (v.int_val != 0);
  } else
  /* Any non-empty string which does not begin with 0 is also true */
  {
    p = v.str_val;
    /* First skip any leading white space or unary -+ */
    while (*p && ((my_isspace(charset_info, *p) || *p == '-' || *p == '+')))
      p++;

    cur_block->ok = (*p && *p != '0') ? true : false;
  }

  if (not_expr) cur_block->ok = !cur_block->ok;

  if (cur_block->ok) {
    cur_block->delim[0] = '\0';
  } else {
    /* Remember "old" delimiter if entering a false if block */
    strcpy(cur_block->delim, delimiter);
  }

  DBUG_PRINT("info", ("OK: %d", cur_block->ok));

  var_free()(&v);
}

static void do_delimiter(struct st_command *command) {
  char *p = command->first_argument;
  DBUG_TRACE;
  DBUG_PRINT("enter", ("first_argument: %s", command->first_argument));

  while (*p && my_isspace(charset_info, *p)) p++;

  if (!(*p)) die("Can't set empty delimiter");

  strmake(delimiter, p, sizeof(delimiter) - 1);
  delimiter_length = std::strlen(delimiter);

  DBUG_PRINT("exit", ("delimiter: %s", delimiter));
  command->last_argument = p + delimiter_length;
}

/*
  do_reset_connection

  DESCRIPTION
  Reset the current session.
*/
static void do_reset_connection() {
  MYSQL *mysql = &cur_con->mysql;

  DBUG_TRACE;
  if (mysql_reset_connection_wrapper(mysql))
    die("reset connection failed: %s", mysql_error(mysql));
  if (cur_con->stmt) {
    mysql_stmt_close(cur_con->stmt);
    cur_con->stmt = nullptr;
  }
}

void do_query_attrs_add(struct st_command *command) {
  int error;
  static DYNAMIC_STRING key;
  static DYNAMIC_STRING value;
  const struct command_arg query_attrs_args[] = {
      {"key", ARG_STRING, true, &key, "Key for query attributes"},
      {"value", ARG_STRING, true, &value, "Value for query attributes"}};
  DBUG_ENTER("do_query_attrs_add");

  check_command_args(command, command->first_argument, query_attrs_args,
                     sizeof(query_attrs_args) / sizeof(struct command_arg),
                     ' ');

  error = mysql_options4(&cur_con->mysql, MYSQL_OPT_QUERY_ATTR_ADD, key.str,
                         value.str);
  handle_command_error(command, error);
  dynstr_free(&key);
  dynstr_free(&value);
  DBUG_VOID_RETURN;
}

void do_query_attrs_delete(struct st_command *command) {
  int error;
  static DYNAMIC_STRING key;
  const struct command_arg query_attrs_args[] = {
      {"key", ARG_STRING, true, &key, "Key for query attributes"},
  };
  DBUG_ENTER("do_query_attrs_delete");

  check_command_args(command, command->first_argument, query_attrs_args,
                     sizeof(query_attrs_args) / sizeof(struct command_arg),
                     ' ');

  error = mysql_options(&cur_con->mysql, MYSQL_OPT_QUERY_ATTR_DELETE, key.str);
  handle_command_error(command, error);
  dynstr_free(&key);
  DBUG_VOID_RETURN;
}

void do_conn_attrs_add(struct st_command *command) {
  int error;
  static DYNAMIC_STRING key;
  static DYNAMIC_STRING value;
  const struct command_arg conn_attrs_args[] = {
      {"key", ARG_STRING, true, &key, "Key for connection attributes"},
      {"value", ARG_STRING, true, &value, "Value for connection attributes"}};
  DBUG_ENTER("do_conn_attrs_add");

  check_command_args(command, command->first_argument, conn_attrs_args,
                     sizeof(conn_attrs_args) / sizeof(struct command_arg), ' ');

  error = mysql_options4(&cur_con->mysql, MYSQL_OPT_CONNECT_ATTR_ADD, key.str,
                         value.str);
  handle_command_error(command, error);
  dynstr_free(&key);
  dynstr_free(&value);
  DBUG_VOID_RETURN;
}

void do_conn_attrs_delete(struct st_command *command) {
  int error;
  static DYNAMIC_STRING key;
  const struct command_arg conn_attrs_args[] = {
      {"key", ARG_STRING, true, &key, "Key for connection attributes"},
  };
  DBUG_ENTER("do_conn_attrs_delete");

  check_command_args(command, command->first_argument, conn_attrs_args,
                     sizeof(conn_attrs_args) / sizeof(struct command_arg), ' ');

  error =
      mysql_options(&cur_con->mysql, MYSQL_OPT_CONNECT_ATTR_DELETE, key.str);
  handle_command_error(command, error);
  dynstr_free(&key);
  DBUG_VOID_RETURN;
}

bool match_delimiter(int c, const char *delim, size_t length) {
  uint i;
  char tmp[MAX_DELIMITER_LENGTH];

  if (c != *delim) return false;

  for (i = 1; i < length && (c = my_getc(cur_file->file)) == *(delim + i); i++)
    tmp[i] = c;

  if (i == length) return true; /* Found delimiter */

  /* didn't find delimiter, push back things that we read */
  my_ungetc(c);
  while (i > 1) my_ungetc(tmp[--i]);
  return false;
}

static bool end_of_query(int c) {
  return match_delimiter(c, delimiter, delimiter_length);
}

/*
  Read one "line" from the file

  SYNOPSIS
  read_line
  buf     buffer for the read line
  size    size of the buffer i.e max size to read

  DESCRIPTION
  This function actually reads several lines and adds them to the
  buffer buf. It continues to read until it finds what it believes
  is a complete query.

  Normally that means it will read lines until it reaches the
  "delimiter" that marks end of query. Default delimiter is ';'
  The function should be smart enough not to detect delimiter's
  found inside strings surrounded with '"' and '\'' escaped strings.

  If the first line in a query starts with '#' or '-' this line is treated
  as a comment. A comment is always terminated when end of line '\n' is
  reached.

*/

static int read_line(char *buf, int size) {
  char c, last_quote = 0, last_char = 0;
  char *p = buf, *buf_end = buf + size - 1;
  int skip_char = 0;
  int query_comment = 0, query_comment_start = 0, query_comment_end = 0;
  bool have_slash = false;

  enum {
    R_NORMAL,
    R_Q,
    R_SLASH_IN_Q,
    R_COMMENT,
    R_LINE_START
  } state = R_LINE_START;
  DBUG_TRACE;

  start_lineno = cur_file->lineno;
  DBUG_PRINT("info", ("Starting to read at lineno: %d", start_lineno));
  for (; p < buf_end;) {
    skip_char = 0;
    c = my_getc(cur_file->file);
    if (feof(cur_file->file)) {
    found_eof:
      if (cur_file->file != stdin) {
        fclose(cur_file->file);
        cur_file->file = nullptr;
      }
      my_free(cur_file->file_name);
      cur_file->file_name = nullptr;
      if (cur_file == file_stack) {
        /* We're back at the first file, check if
           all { have matching }
        */
        if (cur_block != block_stack) die("Missing end of block");

        *p = 0;
        DBUG_PRINT("info", ("end of file at line %d", cur_file->lineno));
        return 1;
      }
      cur_file--;
      start_lineno = cur_file->lineno;
      continue;
    }

    if (c == '\n') {
      /* Line counting is independent of state */
      cur_file->lineno++;

      /* Convert cr/lf to lf */
      if (p != buf && *(p - 1) == '\r') p--;
    }

    switch (state) {
      case R_NORMAL:
        if (end_of_query(c)) {
          *p = 0;
          DBUG_PRINT("exit", ("Found delimiter '%s' at line %d", delimiter,
                              cur_file->lineno));
          return 0;
        } else if ((c == '{' &&
                    (!charset_info->coll->strnncoll(
                         charset_info, (const uchar *)"while", 5, (uchar *)buf,
                         std::min<ptrdiff_t>(5, p - buf), false) ||
                     !charset_info->coll->strnncoll(
                         charset_info, (const uchar *)"if", 2, (uchar *)buf,
                         std::min<ptrdiff_t>(2, p - buf), false)))) {
          /* Only if and while commands can be terminated by { */
          *p++ = c;
          *p = 0;
          DBUG_PRINT("exit", ("Found '{' indicating start of block at line %d",
                              cur_file->lineno));
          return 0;
        } else if (c == '\'' || c == '"' || c == '`') {
          if (!have_slash) {
            last_quote = c;
            state = R_Q;
          }
        } else if (c == '/') {
          if ((query_comment_start == 0) && (query_comment == 0))
            query_comment_start = 1;
          else if (query_comment_end == 1) {
            query_comment = 0;
            query_comment_end = 0;
          }
        } else if (c == '*') {
          if ((query_comment == 1) && (query_comment_end == 0))
            query_comment_end = 1;
          else if (query_comment_start == 1)
            query_comment = 1;
        } else if ((c == '+') || (c == '!')) {
          if ((query_comment_start == 1) && (query_comment == 1)) {
            query_comment_start = 0;
            query_comment = 0;
          }
        } else if (query_comment_start == 1)
          query_comment_start = 0;
        else if (query_comment_end == 1)
          query_comment_end = 0;

        have_slash = (c == '\\');
        break;

      case R_COMMENT:
        if (c == '\n') {
          /* Comments are terminated by newline */
          *p = 0;
          DBUG_PRINT("exit", ("Found newline in comment at line: %d",
                              cur_file->lineno));
          return 0;
        }
        break;

      case R_LINE_START:
        if (c == '#' || c == '-') {
          /* A # or - in the first position of the line - this is a comment */
          state = R_COMMENT;
        } else if (my_isspace(charset_info, c)) {
          if (c == '\n') {
            if (last_char == '\n') {
              /* Two new lines in a row, return empty line */
              DBUG_PRINT("info", ("Found two new lines in a row"));
              *p++ = c;
              *p = 0;
              return 0;
            }

            /* Query hasn't started yet */
            start_lineno = cur_file->lineno;
            DBUG_PRINT("info", ("Query hasn't started yet, start_lineno: %d",
                                start_lineno));
          }

          /* Skip all space at begining of line */
          skip_char = 1;
        } else if (end_of_query(c)) {
          *p = 0;
          DBUG_PRINT("exit", ("Found delimiter '%s' at line: %d", delimiter,
                              cur_file->lineno));
          return 0;
        } else if (c == '}') {
          /* A "}" need to be by itself in the begining of a line to terminate
           */
          *p++ = c;
          *p = 0;
          DBUG_PRINT("exit", ("Found '}' in begining of a line at line: %d",
                              cur_file->lineno));
          return 0;
        } else if (c == '\'' || c == '"' || c == '`') {
          last_quote = c;
          state = R_Q;
        } else
          state = R_NORMAL;
        break;

      case R_Q:
        if (c == last_quote)
          state = R_NORMAL;
        else if (c == '\\')
          state = R_SLASH_IN_Q;
        else if (query_comment)
          state = R_NORMAL;
        break;

      case R_SLASH_IN_Q:
        state = R_Q;
        break;
    }

    last_char = c;

    if (!skip_char) {
      /* Could be a multibyte character */
      /* This code is based on the code in "sql_load.cc" */
      uint charlen;
      if (my_mbmaxlenlen(charset_info) == 1)
        charlen = my_mbcharlen(charset_info, (unsigned char)c);
      else {
        if (!(charlen = my_mbcharlen(charset_info, (unsigned char)c))) {
          int c1 = my_getc(cur_file->file);
          if (c1 == EOF) {
            *p++ = c;
            goto found_eof;
          }

          charlen =
              my_mbcharlen_2(charset_info, (unsigned char)c, (unsigned char)c1);
          my_ungetc(c1);
        }
      }
      if (charlen == 0) return 1;
      /* We give up if multibyte character is started but not */
      /* completed before we pass buf_end */
      if ((charlen > 1) && (p + charlen) <= buf_end) {
        char *mb_start = p;

        *p++ = c;

        for (uint i = 1; i < charlen; i++) {
          c = my_getc(cur_file->file);
          if (feof(cur_file->file)) goto found_eof;
          *p++ = c;
        }
        if (!my_ismbchar(charset_info, mb_start, p)) {
          /* It was not a multiline char, push back the characters */
          /* We leave first 'c', i.e. pretend it was a normal char */
          while (p - 1 > mb_start) my_ungetc(*--p);
        }
      } else
        *p++ = c;
    }
  }
  die("The input buffer is too small for this query.x\n"
      "check your query or increase MAX_QUERY and recompile");
  return 0;
}

/*
  Convert the read query to result format version 1

  That is: After newline, all spaces need to be skipped
  unless the previous char was a quote

  This is due to an old bug that has now been fixed, but the
  version 1 output format is preserved by using this function

*/

static void convert_to_format_v1(char *query) {
  int last_c_was_quote = 0;
  char *p = query, *to = query;
  char *end = strend(query);
  char last_c;

  while (p <= end) {
    if (*p == '\n' && !last_c_was_quote) {
      *to++ = *p++; /* Save the newline */

      /* Skip any spaces on next line */
      while (*p && my_isspace(charset_info, *p)) p++;

      last_c_was_quote = 0;
    } else if (*p == '\'' || *p == '"' || *p == '`') {
      last_c = *p;
      *to++ = *p++;

      /* Copy anything until the next quote of same type */
      while (*p && *p != last_c) *to++ = *p++;

      *to++ = *p++;

      last_c_was_quote = 1;
    } else {
      *to++ = *p++;
      last_c_was_quote = 0;
    }
  }
}

/*
  Check for unexpected "junk" after the end of query
  This is normally caused by missing delimiters or when
  switching between different delimiters
*/

void check_eol_junk_line(const char *line) {
  const char *p = line;
  DBUG_TRACE;
  DBUG_PRINT("enter", ("line: %s", line));

  /* Check for extra delimiter */
  if (*p && !std::strncmp(p, delimiter, delimiter_length))
    die("Extra delimiter \"%s\" found", delimiter);

  /* Allow trailing # comment */
  if (*p && *p != '#') {
    if (*p == '\n') die("Missing delimiter");
    die("End of line junk detected: \"%s\"", p);
  }
}

static void check_eol_junk(const char *eol) {
  const char *p = eol;
  DBUG_TRACE;
  DBUG_PRINT("enter", ("eol: %s", eol));

  /* Skip past all spacing chars and comments */
  while (*p && (my_isspace(charset_info, *p) || *p == '#' || *p == '\n')) {
    /* Skip past comments started with # and ended with newline */
    if (*p && *p == '#') {
      p++;
      while (*p && *p != '\n') p++;
    }

    /* Check this line */
    if (*p && *p == '\n') check_eol_junk_line(p);

    if (*p) p++;
  }

  check_eol_junk_line(p);
}

static bool is_delimiter(const char *p) {
  uint match = 0;
  char *delim = delimiter;
  while (*p && *p == *delim++) {
    match++;
    p++;
  }

  return (match == delimiter_length);
}

// 256K -- a test in sp-big is >128K
#define MAX_QUERY (256 * 1024 * 2)
static char read_command_buf[MAX_QUERY];

/// Create a command from a set of lines.
///
/// Converts lines returned by read_line into a command, this involves
/// parsing the first word in the read line to find the command type.
///
/// A '`--`' comment may contain a valid query as the first word after
/// the comment start. Thus it's always checked to see if that is the
/// case. The advantage with this approach is to be able to execute
/// commands terminated by new line '\n' regardless how many "delimiter"
/// it contain.
///
/// @param [in] command_ptr pointer where to return the new query
///
/// @retval 0 on success, else 1
static int read_command(struct st_command **command_ptr) {
  char *p = read_command_buf;
  DBUG_TRACE;

  if (parser.current_line < parser.read_lines) {
    *command_ptr = q_lines->at(parser.current_line);
    // Assign the current command line number
    start_lineno = (*command_ptr)->lineno;
    return 0;
  }

  struct st_command *command;
  if (!(*command_ptr = command = (struct st_command *)my_malloc(
            PSI_NOT_INSTRUMENTED, sizeof(*command),
            MYF(MY_WME | MY_ZEROFILL))) ||
      q_lines->push_back(command))
    die("Out of memory");
  command->type = Q_UNKNOWN;

  read_command_buf[0] = 0;
  if (read_line(read_command_buf, sizeof(read_command_buf))) {
    check_eol_junk(read_command_buf);
    return 1;
  }

  // Set the line number for the command
  command->lineno = start_lineno;

  if (opt_result_format_version == 1) convert_to_format_v1(read_command_buf);

  DBUG_PRINT("info", ("query: '%s'", read_command_buf));
  if (*p == '#') {
    command->type = Q_COMMENT;
  } else if (p[0] == '-' && p[1] == '-') {
    command->type = Q_COMMENT_WITH_COMMAND;
    // Skip past '--'
    p += 2;
  } else if (*p == '\n') {
    command->type = Q_EMPTY_LINE;
  }

  // Skip leading spaces
  while (*p && my_isspace(charset_info, *p)) p++;

  if (!(command->query_buf = command->query =
            my_strdup(PSI_NOT_INSTRUMENTED, p, MYF(MY_WME))))
    die("Out of memory");

  // Calculate first word length(the command), terminated
  // by 'space' , '(' or 'delimiter'
  p = command->query;
  while (*p && !my_isspace(charset_info, *p) && *p != '(' && !is_delimiter(p))
    p++;
  command->first_word_len = (uint)(p - command->query);
  DBUG_PRINT("info",
             ("first_word: %.*s", static_cast<int>(command->first_word_len),
              command->query));

  // Skip spaces between command and first argument
  while (*p && my_isspace(charset_info, *p)) p++;
  command->first_argument = p;

  command->end = strend(command->query);
  command->query_len = (command->end - command->query);
  parser.read_lines++;
  return 0;
}

static struct my_option my_long_options[] = {
#include "caching_sha2_passwordopt-longopts.h"
#include "sslopt-longopts.h"
    {"basedir", 'b', "Basedir for tests.", &opt_basedir, &opt_basedir, nullptr,
     GET_STR, REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"character-sets-dir", OPT_CHARSETS_DIR,
     "Directory for character set files.", &opt_charsets_dir, &opt_charsets_dir,
     nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"colored-diff", OPT_COLORED_DIFF, "Colorize the diff outout.",
     &opt_colored_diff, &opt_colored_diff, nullptr, GET_BOOL, NO_ARG, 0, 0, 0,
     nullptr, 0, nullptr},
    {"compress", 'C', "Use the compressed server/client protocol.",
     &opt_compress, &opt_compress, nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr,
     0, nullptr},
    {"connect_timeout", OPT_CONNECT_TIMEOUT,
     "Number of seconds before connection timeout.", &opt_connect_timeout,
     &opt_connect_timeout, nullptr, GET_UINT, REQUIRED_ARG, 120, 0, 3600 * 12,
     nullptr, 0, nullptr},
    {"cursor-protocol", OPT_CURSOR_PROTOCOL,
     "Use cursors for prepared statements.", &cursor_protocol, &cursor_protocol,
     nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"database", 'D', "Database to use.", &opt_db, &opt_db, nullptr, GET_STR,
     REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
#ifdef DBUG_OFF
    {"debug", '#', "This is a non-debug version. Catch this and exit.", 0, 0, 0,
     GET_DISABLED, OPT_ARG, 0, 0, 0, 0, 0, 0},
    {"debug-check", OPT_DEBUG_CHECK,
     "This is a non-debug version. Catch this and exit.", 0, 0, 0, GET_DISABLED,
     NO_ARG, 0, 0, 0, 0, 0, 0},
    {"debug-info", OPT_DEBUG_INFO,
     "This is a non-debug version. Catch this and exit.", 0, 0, 0, GET_DISABLED,
     NO_ARG, 0, 0, 0, 0, 0, 0},
#else
    {"debug", '#', "Output debug log. Often this is 'd:t:o,filename'.", nullptr,
     nullptr, nullptr, GET_STR, OPT_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"debug-check", OPT_DEBUG_CHECK,
     "Check memory and open file usage at exit.", &debug_check_flag,
     &debug_check_flag, nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
    {"debug-info", OPT_DEBUG_INFO, "Print some debug info at exit.",
     &debug_info_flag, &debug_info_flag, nullptr, GET_BOOL, NO_ARG, 0, 0, 0,
     nullptr, 0, nullptr},
#endif
    {"default-character-set", OPT_DEFAULT_CHARSET,
     "Set the default character set.", &default_charset, &default_charset,
     nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"explain-protocol", OPT_EXPLAIN_PROTOCOL,
     "Explain all SELECT/INSERT/REPLACE/UPDATE/DELETE statements",
     &explain_protocol, &explain_protocol, nullptr, GET_BOOL, NO_ARG, 0, 0, 0,
     nullptr, 0, nullptr},
    {"help", '?', "Display this help and exit.", nullptr, nullptr, nullptr,
     GET_NO_ARG, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"host", 'h', "Connect to host.", &opt_host, &opt_host, nullptr, GET_STR,
     REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"include", 'i', "Include SQL before each test case.", &opt_include,
     &opt_include, nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
    {"json-explain-protocol", OPT_JSON_EXPLAIN_PROTOCOL,
     "Explain all SELECT/INSERT/REPLACE/UPDATE/DELETE statements with "
     "FORMAT=JSON",
     &json_explain_protocol, &json_explain_protocol, nullptr, GET_BOOL, NO_ARG,
     0, 0, 0, nullptr, 0, nullptr},
    {"logdir", OPT_LOG_DIR, "Directory for log files", &opt_logdir, &opt_logdir,
     nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"mark-progress", OPT_MARK_PROGRESS,
     "Write line number and elapsed time to <testname>.progress.",
     &opt_mark_progress, &opt_mark_progress, nullptr, GET_BOOL, NO_ARG, 0, 0, 0,
     nullptr, 0, nullptr},
    {"max-connect-retries", OPT_MAX_CONNECT_RETRIES,
     "Maximum number of attempts to connect to server.",
     &opt_max_connect_retries, &opt_max_connect_retries, nullptr, GET_INT,
     REQUIRED_ARG, 500, 1, 10000, nullptr, 0, nullptr},
    {"max-connections", OPT_MAX_CONNECTIONS,
     "Max number of open connections to server", &opt_max_connections,
     &opt_max_connections, nullptr, GET_INT, REQUIRED_ARG, 128, 8, 5120,
     nullptr, 0, nullptr},
    {"no-skip", OPT_NO_SKIP, "Force the test to run without skip.", &no_skip,
     &no_skip, nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"no-skip-exclude-list", 'n',
     "Contains comma seperated list of to be excluded inc files.",
     &excluded_string, &excluded_string, nullptr, GET_STR, REQUIRED_ARG, 0, 0,
     0, nullptr, 0, nullptr},
    {"offload-count-file", OPT_OFFLOAD_COUNT_FILE, "Offload count report file",
     &opt_offload_count_file, &opt_offload_count_file, nullptr, GET_STR,
     REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"opt-trace-protocol", OPT_TRACE_PROTOCOL,
     "Trace DML statements with optimizer trace", &opt_trace_protocol,
     &opt_trace_protocol, nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
    {"password", 'p', "Password to use when connecting to server.", nullptr,
     nullptr, nullptr, GET_STR, OPT_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"plugin_dir", OPT_PLUGIN_DIR, "Directory for client-side plugins.",
     &opt_plugin_dir, &opt_plugin_dir, nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0,
     nullptr, 0, nullptr},
    {"port", 'P',
     "Port number to use for connection or 0 for default to, in "
     "order of preference, my.cnf, $MYSQL_TCP_PORT, "
#if MYSQL_PORT_DEFAULT == 0
     "/etc/services, "
#endif
     "built-in default (" STRINGIFY_ARG(MYSQL_PORT) ").",
     &opt_port, &opt_port, nullptr, GET_INT, REQUIRED_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
    {"protocol", OPT_MYSQL_PROTOCOL,
     "The protocol of connection (tcp,socket,pipe,memory).", nullptr, nullptr,
     nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"ps-protocol", OPT_PS_PROTOCOL,
     "Use prepared-statement protocol for communication.", &ps_protocol,
     &ps_protocol, nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"quiet", 's', "Suppress all normal output.", &silent, &silent, nullptr,
     GET_BOOL, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"record", 'r', "Record output of test_file into result file.", nullptr,
     nullptr, nullptr, GET_NO_ARG, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"result-file", 'R', "Read/store result from/in this file.",
     &result_file_name, &result_file_name, nullptr, GET_STR, REQUIRED_ARG, 0, 0,
     0, nullptr, 0, nullptr},
    {"result-format-version", OPT_RESULT_FORMAT_VERSION,
     "Version of the result file format to use", &opt_result_format_version,
     &opt_result_format_version, nullptr, GET_INT, REQUIRED_ARG, 1, 1, 2,
     nullptr, 0, nullptr},
#ifdef _WIN32
    {"safe-process-pid", OPT_SAFEPROCESS_PID, "PID of safeprocess.",
     &opt_safe_process_pid, &opt_safe_process_pid, 0, GET_INT, REQUIRED_ARG, 0,
     0, 0, 0, 0, 0},
#endif
    {"shared-memory-base-name", OPT_SHARED_MEMORY_BASE_NAME,
     "Base name of shared memory.", &shared_memory_base_name,
     &shared_memory_base_name, nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0, nullptr,
     0, nullptr},
    {"silent", 's', "Suppress all normal output. Synonym for --quiet.", &silent,
     &silent, nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"sleep", 'T', "Always sleep this many seconds on sleep commands.",
     &opt_sleep, &opt_sleep, nullptr, GET_INT, REQUIRED_ARG, -1, -1, 0, nullptr,
     0, nullptr},
    {"socket", 'S', "The socket file to use for connection.", &unix_sock,
     &unix_sock, nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"sp-protocol", OPT_SP_PROTOCOL, "Use stored procedures for select.",
     &sp_protocol, &sp_protocol, nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
    {"tail-lines", OPT_TAIL_LINES,
     "Number of lines of the result to include in a failure report.",
     &opt_tail_lines, &opt_tail_lines, nullptr, GET_INT, REQUIRED_ARG, 0, 0,
     10000, nullptr, 0, nullptr},
    {"test-file", 'x', "Read test from/in this file (default stdin).", nullptr,
     nullptr, nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"timer-file", 'm', "File where the timing in microseconds is stored.",
     nullptr, nullptr, nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
    {"tmpdir", 't', "Temporary directory where sockets are put.", nullptr,
     nullptr, nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"trace-exec", OPT_TRACE_EXEC, "Print output from exec to stdout.",
     &trace_exec, &trace_exec, nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
    {"user", 'u', "User for login.", &opt_user, &opt_user, nullptr, GET_STR,
     REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"verbose", 'v', "Write more.", &verbose, &verbose, nullptr, GET_BOOL,
     NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"version", 'V', "Output version information and exit.", nullptr, nullptr,
     nullptr, GET_NO_ARG, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"view-protocol", OPT_VIEW_PROTOCOL, "Use views for select.",
     &view_protocol, &view_protocol, nullptr, GET_BOOL, NO_ARG, 0, 0, 0,
     nullptr, 0, nullptr},
    {"async-client", '*', "Use async client.", &use_async_client,
     &use_async_client, nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
    {"compression-algorithms", 0,
     "Use compression algorithm in server/client protocol. Valid values "
     "are any combination of 'zstd','zlib','uncompressed'.",
     &opt_compress_algorithm, &opt_compress_algorithm, nullptr, GET_STR,
     REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"zstd-compression-level", 0,
     "Use this compression level in the client/server protocol, in case "
     "--compression-algorithms=zstd. Valid range is between 1 and 22, "
     "inclusive. Default is 3.",
     &opt_zstd_compress_level, &opt_zstd_compress_level, nullptr, GET_UINT,
     REQUIRED_ARG, 3, 1, 22, nullptr, 0, nullptr},

    {nullptr, 0, nullptr, nullptr, nullptr, nullptr, GET_NO_ARG, NO_ARG, 0, 0,
     0, nullptr, 0, nullptr}};

static void usage() {
  print_version();
  puts(ORACLE_WELCOME_COPYRIGHT_NOTICE("2000"));
  printf(
      "Runs a test against the mysql server and compares output with a results "
      "file.\n\n");
  printf("Usage: %s [OPTIONS] [database] < test_file\n", my_progname);
  my_print_help(my_long_options);
  printf(
      "  --no-defaults       Don't read default options from any options "
      "file.\n");
  my_print_variables(my_long_options);
}

static bool get_one_option(int optid, const struct my_option *opt,
                           char *argument) {
  switch (optid) {
    case '#':
#ifndef DBUG_OFF
      DBUG_PUSH(argument ? argument : "d:t:S:i:O,/tmp/mysqltest.trace");
      debug_check_flag = true;
#endif
      break;
    case 'r':
      record = 1;
      break;
    case 'x': {
      char buff[FN_REFLEN];
      if (!test_if_hard_path(argument)) {
        strxmov(buff, opt_basedir, argument, NullS);
        argument = buff;
      }
      fn_format(buff, argument, "", "", MY_UNPACK_FILENAME);
      DBUG_ASSERT(cur_file == file_stack && cur_file->file == nullptr);
      if (!(cur_file->file = fopen(buff, "rb")))
        die("Could not open '%s' for reading, errno: %d", buff, errno);
      cur_file->file_name = my_strdup(PSI_NOT_INSTRUMENTED, buff, MYF(MY_FAE));
      cur_file->lineno = 1;
      break;
    }
    case 'm': {
      static char buff[FN_REFLEN];
      if (!test_if_hard_path(argument)) {
        strxmov(buff, opt_basedir, argument, NullS);
        argument = buff;
      }
      fn_format(buff, argument, "", "", MY_UNPACK_FILENAME);
      timer_file = buff;
      unlink(timer_file); /* Ignore error, may not exist */
      break;
    }
    case 'p':
      if (argument == disabled_my_option) {
        // Don't require password
        static char empty_password[] = {'\0'};
        DBUG_ASSERT(empty_password[0] ==
                    '\0');  // Check that it has not been overwritten
        argument = empty_password;
      }
      if (argument) {
        my_free(opt_pass);
        opt_pass = my_strdup(PSI_NOT_INSTRUMENTED, argument, MYF(MY_FAE));
        while (*argument) *argument++ = 'x'; /* Destroy argument */
        tty_password = false;
      } else
        tty_password = true;
      break;
#include "sslopt-case.h"

    case 't':
      my_stpnmov(TMPDIR, argument, sizeof(TMPDIR));
      break;
    case OPT_LOG_DIR:
      /* Check that the file exists */
      if (access(opt_logdir, F_OK) != 0)
        die("The specified log directory does not exist: '%s'", opt_logdir);
      break;
    case OPT_RESULT_FORMAT_VERSION:
      set_result_format_version(opt_result_format_version);
      break;
    case 'V':
      print_version();
      exit(0);
    case OPT_MYSQL_PROTOCOL:
      opt_protocol =
          find_type_or_exit(argument, &sql_protocol_typelib, opt->name);
      break;
    case '?':
      usage();
      exit(0);
  }
  return false;
}

/**
  Test case or the result file names may use alphanumeric characters
  (A-Z, a-z, 0-9), dash ('-') or underscore ('_'), but should not
  start with dash or underscore.

  Check if a file name conatins any other special characters. If yes,
  throw an error and abort the test run.

  @param[in] file_name File name
*/

static void validate_filename(const char *file_name) {
  const char *fname = strrchr(file_name, '/');

  if (fname == nullptr) {
    if (is_windows) {
      fname = strrchr(file_name, '\\');

      if (fname == nullptr)
        fname = file_name;
      else
        fname++;
    } else
      fname = file_name;
  } else
    fname++;

  file_name = fname;

  // Check if first character in the file name is a alphanumeric character
  if (!my_isalnum(charset_info, file_name[0])) {
    die("Invalid file name '%s', first character must be alpha-numeric.",
        file_name);
  } else
    file_name++;

  // Skip extension('.test' or '.result' or '.inc' etc) in the file name
  const char *file_name_end = strrchr(file_name, '.');

  while (*file_name && (file_name != file_name_end) &&
         (file_name[0] == '-' || file_name[0] == '_' ||
          my_isalnum(charset_info, file_name[0]))) {
    file_name++;
  }

  if (file_name != file_name_end) {
    die("Invalid file name '%s'. Test or result file name should "
        "consist of only alpha-numeric characters, dash (-) or "
        "underscore (_), but should not start with dash or "
        "underscore.",
        fname);
  }
}

static int parse_args(int argc, char **argv) {
  if (load_defaults("my", load_default_groups, &argc, &argv, &argv_alloc))
    exit(1);

  if ((handle_options(&argc, &argv, my_long_options, get_one_option))) exit(1);

  // Check for special characters in test case file name
  if (cur_file->file_name) validate_filename(cur_file->file_name);

  // Check for special characters in result file name
  if (result_file_name) validate_filename(result_file_name);

  if (argc > 1) {
    usage();
    exit(1);
  }

  if (argc == 1) opt_db = *argv;
  if (tty_password) opt_pass = get_tty_password(NullS);
  if (debug_info_flag) my_end_arg = MY_CHECK_ERROR | MY_GIVE_INFO;
  if (debug_check_flag) my_end_arg = MY_CHECK_ERROR;

  if (!record) {
    /* Check that the result file exists */
    if (result_file_name && access(result_file_name, F_OK) != 0)
      die("The specified result file '%s' does not exist", result_file_name);
  }

  return 0;
}

/*
  Write the content of str into file

  SYNOPSIS
  str_to_file2
  fname - name of file to truncate/create and write to
  str - content to write to file
  size - size of content witten to file
  append - append to file instead of overwriting old file
*/

void str_to_file2(const char *fname, char *str, size_t size, bool append) {
  int fd;
  char buff[FN_REFLEN];
  int flags = O_WRONLY | O_CREAT;
  if (!test_if_hard_path(fname)) {
    strxmov(buff, opt_basedir, fname, NullS);
    fname = buff;
  }
  fn_format(buff, fname, "", "", MY_UNPACK_FILENAME);

  if (!append) flags |= O_TRUNC;
  if ((fd = my_open(buff, flags, MYF(MY_WME))) < 0)
    die("Could not open '%s' for writing, errno: %d", buff, errno);
  if (append && my_seek(fd, 0, SEEK_END, MYF(0)) == MY_FILEPOS_ERROR)
    die("Could not find end of file '%s', errno: %d", buff, errno);
  if (my_write(fd, (uchar *)str, size, MYF(MY_WME | MY_FNABP)))
    die("write failed, errno: %d", errno);
  my_close(fd, MYF(0));
}

/*
  Write the content of str into file

  SYNOPSIS
  str_to_file
  fname - name of file to truncate/create and write to
  str - content to write to file
  size - size of content witten to file
*/

void str_to_file(const char *fname, char *str, size_t size) {
  str_to_file2(fname, str, size, false);
}

#ifdef _WIN32

typedef Prealloced_array<const char *, 16> Patterns;
Patterns *patterns;

/*
  init_win_path_patterns

  DESCRIPTION
  Setup string patterns that will be used to detect filenames that
  needs to be converted from Win to Unix format

*/

void init_win_path_patterns() {
  /* List of string patterns to match in order to find paths */
  const char *paths[] = {
      "$MYSQL_TEST_DIR", "$MYSQL_TMP_DIR",  "$MYSQLTEST_VARDIR",
      "$MASTER_MYSOCK",  "$MYSQL_SHAREDIR", "$MYSQL_CHARSETSDIR",
      "$MYSQL_LIBDIR",   "./test/",         ".ibd",
      ".\\ibdata",       ".\\ibtmp",        ".\\undo"};
  int num_paths = sizeof(paths) / sizeof(char *);
  int i;
  char *p;

  DBUG_TRACE;

  patterns = new Patterns(PSI_NOT_INSTRUMENTED);

  /* Loop through all paths in the array */
  for (i = 0; i < num_paths; i++) {
    VAR *v;
    if (*(paths[i]) == '$') {
      v = var_get(paths[i], 0, 0, 0);
      p = my_strdup(PSI_NOT_INSTRUMENTED, v->str_val, MYF(MY_FAE));
    } else
      p = my_strdup(PSI_NOT_INSTRUMENTED, paths[i], MYF(MY_FAE));

    /* Don't insert zero length strings in patterns array */
    if (std::strlen(p) == 0) {
      my_free(p);
      continue;
    }

    if (patterns->push_back(p)) die("Out of memory");

    DBUG_PRINT("info", ("p: %s", p));
    while (*p) {
      if (*p == '/') *p = '\\';
      p++;
    }
  }
}

void free_win_path_patterns() {
  uint i = 0;
  const char **pat;
  for (pat = patterns->begin(); pat != patterns->end(); ++pat) {
    my_free(const_cast<char *>(*pat));
  }
  delete patterns;
  patterns = NULL;
}

/*
  fix_win_paths

  DESCRIPTION
  Search the string 'val' for the patterns that are known to be
  strings that contain filenames. Convert all \ to / in the
  filenames that are found.

  Ex:
  val = 'Error "c:\mysql\mysql-test\var\test\t1.frm" didn't exist'
  => $MYSQL_TEST_DIR is found by strstr
  => all \ from c:\mysql\m... until next space is converted into /
*/

void fix_win_paths(const char *val, size_t len) {
  DBUG_TRACE;
  const char **pat;
  for (pat = patterns->begin(); pat != patterns->end(); ++pat) {
    char *p;
    DBUG_PRINT("info", ("pattern: %s", *pat));

    /* Find and fix each path in this string */
    p = const_cast<char *>(val);
    while (p = strstr(p, *pat)) {
      DBUG_PRINT("info", ("Found %s in val p: %s", *pat, p));
      /* Found the pattern.  Back up to the start of this path */
      while (p > val && !my_isspace(charset_info, *(p - 1))) {
        p--;
      }

      while (*p && !my_isspace(charset_info, *p)) {
        if (*p == '\\') *p = '/';
        p++;
      }
      DBUG_PRINT("info", ("Converted \\ to / in %s", val));
    }
  }
  DBUG_PRINT("exit", (" val: %s, len: %d", val, len));
}
#endif

/*
  Append the result for one field to the dynamic string ds
*/

static void append_field(DYNAMIC_STRING *ds, uint col_idx, MYSQL_FIELD *field,
                         char *val, size_t len, bool is_null) {
  char null[] = "NULL";

  if (col_idx < max_replace_column && replace_column[col_idx]) {
    val = replace_column[col_idx];
    len = std::strlen(val);
  } else if (is_null) {
    val = null;
    len = 4;
  }
#ifdef _WIN32
  else if ((field->type == MYSQL_TYPE_DOUBLE ||
            field->type == MYSQL_TYPE_FLOAT) &&
           field->decimals >= 31) {
    /* Convert 1.2e+018 to 1.2e+18 and 1.2e-018 to 1.2e-18 */
    char *start = strchr(val, 'e');
    if (start && std::strlen(start) >= 5 &&
        (start[1] == '-' || start[1] == '+') && start[2] == '0') {
      start += 2; /* Now points at first '0' */
      if (field->flags & ZEROFILL_FLAG) {
        /* Move all chars before the first '0' one step right */
        memmove(val + 1, val, start - val);
        *val = '0';
      } else {
        /* Move all chars after the first '0' one step left */
        memmove(start, start + 1, std::strlen(start));
        len--;
      }
    }
  }
#endif

  if (!display_result_vertically) {
    if (col_idx) dynstr_append_mem(ds, "\t", 1);
    replace_dynstr_append_mem(ds, val, len);
  } else {
    dynstr_append(ds, field->name);
    dynstr_append_mem(ds, "\t", 1);
    replace_dynstr_append_mem(ds, val, len);
    dynstr_append_mem(ds, "\n", 1);
  }
}

/*
  Append all results to the dynamic string separated with '\t'
  Values may be converted with 'replace_column'
*/

static void append_result(DYNAMIC_STRING *ds, MYSQL_RES *res) {
  MYSQL_ROW row;
  uint num_fields = mysql_num_fields(res);
  MYSQL_FIELD *fields = mysql_fetch_fields(res);
  ulong *lengths;

  while ((row = mysql_fetch_row_wrapper(res))) {
    uint i;
    lengths = mysql_fetch_lengths(res);
    for (i = 0; i < num_fields; i++) {
      /* looks ugly , but put here to convince parfait */
      assert(lengths);
      append_field(ds, i, &fields[i], row[i], lengths[i], !row[i]);
    }
    if (!display_result_vertically) dynstr_append_mem(ds, "\n", 1);
  }
}

/*
  Append all results from ps execution to the dynamic string separated
  with '\t'. Values may be converted with 'replace_column'
*/

static void append_stmt_result(DYNAMIC_STRING *ds, MYSQL_STMT *stmt,
                               MYSQL_FIELD *fields, uint num_fields) {
  MYSQL_BIND *my_bind;
  bool *is_null;
  ulong *length;
  uint i;

  /* Allocate array with bind structs, lengths and NULL flags */
  my_bind = (MYSQL_BIND *)my_malloc(PSI_NOT_INSTRUMENTED,
                                    num_fields * sizeof(MYSQL_BIND),
                                    MYF(MY_WME | MY_FAE | MY_ZEROFILL));
  length = (ulong *)my_malloc(PSI_NOT_INSTRUMENTED, num_fields * sizeof(ulong),
                              MYF(MY_WME | MY_FAE));
  is_null = (bool *)my_malloc(PSI_NOT_INSTRUMENTED, num_fields * sizeof(bool),
                              MYF(MY_WME | MY_FAE));

  /* Allocate data for the result of each field */
  for (i = 0; i < num_fields; i++) {
    size_t max_length = fields[i].max_length + 1;
    my_bind[i].buffer_type = MYSQL_TYPE_STRING;
    my_bind[i].buffer =
        my_malloc(PSI_NOT_INSTRUMENTED, max_length, MYF(MY_WME | MY_FAE));
    my_bind[i].buffer_length = static_cast<ulong>(max_length);
    my_bind[i].is_null = &is_null[i];
    my_bind[i].length = &length[i];

    DBUG_PRINT("bind", ("col[%d]: buffer_type: %d, buffer_length: %lu", i,
                        my_bind[i].buffer_type, my_bind[i].buffer_length));
  }

  if (mysql_stmt_bind_result(stmt, my_bind))
    die("mysql_stmt_bind_result failed: %d: %s", mysql_stmt_errno(stmt),
        mysql_stmt_error(stmt));

  while (mysql_stmt_fetch(stmt) == 0) {
    for (i = 0; i < num_fields; i++)
      append_field(ds, i, &fields[i], (char *)my_bind[i].buffer,
                   *my_bind[i].length, *my_bind[i].is_null);
    if (!display_result_vertically) dynstr_append_mem(ds, "\n", 1);
  }

  int rc;
  if ((rc = mysql_stmt_fetch(stmt)) != MYSQL_NO_DATA)
    die("fetch didn't end with MYSQL_NO_DATA from statement: %d: %s; rc=%d",
        mysql_stmt_errno(stmt), mysql_stmt_error(stmt), rc);

  for (i = 0; i < num_fields; i++) {
    /* Free data for output */
    my_free(my_bind[i].buffer);
  }
  /* Free array with bind structs, lengths and NULL flags */
  my_free(my_bind);
  my_free(length);
  my_free(is_null);
}

/*
  Append metadata for fields to output
*/

static void append_metadata(DYNAMIC_STRING *ds, MYSQL_FIELD *field,
                            uint num_fields) {
  MYSQL_FIELD *field_end;
  dynstr_append(ds,
                "Catalog\tDatabase\tTable\tTable_alias\tColumn\t"
                "Column_alias\tType\tLength\tMax length\tIs_null\t"
                "Flags\tDecimals\tCharsetnr\n");

  for (field_end = field + num_fields; field < field_end; field++) {
    dynstr_append_mem(ds, field->catalog, field->catalog_length);
    dynstr_append_mem(ds, "\t", 1);
    dynstr_append_mem(ds, field->db, field->db_length);
    dynstr_append_mem(ds, "\t", 1);
    dynstr_append_mem(ds, field->org_table, field->org_table_length);
    dynstr_append_mem(ds, "\t", 1);
    dynstr_append_mem(ds, field->table, field->table_length);
    dynstr_append_mem(ds, "\t", 1);
    dynstr_append_mem(ds, field->org_name, field->org_name_length);
    dynstr_append_mem(ds, "\t", 1);
    dynstr_append_mem(ds, field->name, field->name_length);
    dynstr_append_mem(ds, "\t", 1);
    replace_dynstr_append_uint(ds, field->type);
    dynstr_append_mem(ds, "\t", 1);
    replace_dynstr_append_uint(ds, field->length);
    dynstr_append_mem(ds, "\t", 1);
    replace_dynstr_append_uint(ds, field->max_length);
    dynstr_append_mem(ds, "\t", 1);
    dynstr_append_mem(ds, (IS_NOT_NULL(field->flags) ? "N" : "Y"), 1);
    dynstr_append_mem(ds, "\t", 1);
    replace_dynstr_append_uint(ds, field->flags);
    dynstr_append_mem(ds, "\t", 1);
    replace_dynstr_append_uint(ds, field->decimals);
    dynstr_append_mem(ds, "\t", 1);
    replace_dynstr_append_uint(ds, field->charsetnr);
    dynstr_append_mem(ds, "\n", 1);
  }
}

/*
  Append affected row count and other info to output
*/

static void append_info(DYNAMIC_STRING *ds, ulonglong affected_rows,
                        const char *info) {
  char buf[40], buff2[21];
  sprintf(buf, "affected rows: %s\n", llstr(affected_rows, buff2));
  dynstr_append(ds, buf);
  if (info) {
    dynstr_append(ds, "info: ");
    dynstr_append(ds, info);
    dynstr_append_mem(ds, "\n", 1);
  }
}

/**
  @brief Append state change information (received through Ok packet) to the
  output.

  @param [in,out] ds         Dynamic string to hold the content to be printed.
  @param [in] mysql          Connection handle.
*/

static void append_session_track_info(DYNAMIC_STRING *ds, MYSQL *mysql) {
  for (unsigned int type = SESSION_TRACK_BEGIN; type <= SESSION_TRACK_END;
       type++) {
    const char *data;
    size_t data_length;

    if (!mysql_session_track_get_first(mysql, (enum_session_state_type)type,
                                       &data, &data_length)) {
      /*
        Append the type information. Please update the definition of APPEND_TYPE
        when any changes are made to enum_session_state_type.
      */
      APPEND_TYPE(type);
      dynstr_append(ds, "-- ");
      dynstr_append_mem(ds, data, data_length);
    } else
      continue;
    while (!mysql_session_track_get_next(mysql, (enum_session_state_type)type,
                                         &data, &data_length)) {
      dynstr_append(ds, "\n-- ");
      dynstr_append_mem(ds, data, data_length);
    }
    dynstr_append(ds, "\n\n");
  }
}

/*
  Display the table headings with the names tab separated
*/

static void append_table_headings(DYNAMIC_STRING *ds, MYSQL_FIELD *field,
                                  uint num_fields) {
  uint col_idx;
  for (col_idx = 0; col_idx < num_fields; col_idx++) {
    if (col_idx) dynstr_append_mem(ds, "\t", 1);
    replace_dynstr_append(ds, field[col_idx].name);
  }
  dynstr_append_mem(ds, "\n", 1);
}

/// Check whether a given warning is in list of disabled or enabled warnings.
///
/// @param warnings      List of either disabled or enabled warnings.
/// @param error         Error number
/// @param warning_found Boolean value, should be set to true if warning
///                      is found in the list, false otherwise.
///
/// @retval True if the given warning is present in the list, and
///         ignore flag for that warning is not set, false otherwise.
static bool match_warnings(Expected_warnings *warnings, std::uint32_t error,
                           bool *warning_found) {
  bool match_found = false;
  std::vector<std::unique_ptr<Warning>>::iterator warning = warnings->begin();

  for (; warning != warnings->end(); warning++) {
    if ((*warning)->warning_code() == error) {
      *warning_found = true;
      if (!(*warning)->ignore_warning()) {
        match_found = true;
        break;
      }
    }
  }

  return match_found;
}

/// Handle one warning which occurred during execution of a query.
///
/// @param ds      DYNAMIC_STRING object to store the warnings.
/// @param warning Warning string
///
/// @retval True if a warning is found in the list of disabled or enabled
///         warnings, false otherwise.
static bool handle_one_warning(DYNAMIC_STRING *ds, std::string warning) {
  // Each line of show warnings output contains information about
  // error level, error code and the error/warning message separated
  // by '\t'. Parse each line from the show warnings output to
  // extract the error code and compare it with list of expected
  // warnings.
  bool warning_found = false;
  std::string error_code;
  std::stringstream warn_msg(warning);

  while (std::getline(warn_msg, error_code, '\t')) {
    int errcode = get_int_val(error_code.c_str());
    if (errcode != -1) {
      if (disabled_warnings->count()) {
        // Print the warning if it doesn't match with any of the
        // disabled warnings.
        if (!match_warnings(disabled_warnings, errcode, &warning_found)) {
          dynstr_append_mem(ds, warning.c_str(), warning.length());
          dynstr_append_mem(ds, "\n", 1);
        }
      } else if (enabled_warnings->count()) {
        if (match_warnings(enabled_warnings, errcode, &warning_found)) {
          dynstr_append_mem(ds, warning.c_str(), warning.length());
          dynstr_append_mem(ds, "\n", 1);
        }
      }
    }
  }

  return warning_found;
}

/// Handle warnings which occurred during execution of a query.
///
/// @param ds          DYNAMIC_STRING object to store the warnings.
/// @param ds_warnings String containing all the generated warnings.
static void handle_warnings(DYNAMIC_STRING *ds, const char *ds_warnings) {
  bool warning_found = false;
  std::string warning;
  std::stringstream warnings(ds_warnings);

  // Set warning_found only if at least one of the warning exist
  // in expected list of warnings.
  while (std::getline(warnings, warning))
    if (handle_one_warning(ds, warning)) warning_found = true;

  // Throw an error and abort the test run if a query generates warnings
  // which are not listed as expected.
  if (!warning_found) {
    std::string warning_list;

    if (disabled_warnings->count())
      warning_list = disabled_warnings->warnings_list();
    else if (enabled_warnings->count())
      warning_list = enabled_warnings->warnings_list();

    die("Query '%s' didn't generate any of the expected warning(s) '%s'.",
        curr_command->query, warning_list.c_str());
  }
}

/// Fetch warnings generated by server while executing a query and
/// append them to warnings buffer 'ds'.
///
/// @param ds    DYNAMIC_STRING object to store the warnings
/// @param mysql mysql handle object
///
/// @retval Number of warnings appended to ds
static int append_warnings(DYNAMIC_STRING *ds, MYSQL *mysql) {
  unsigned int count;
  if (!(count = mysql_warning_count(mysql))) return 0;

  // If one day we will support execution of multi-statements
  // through PS API we should not issue SHOW WARNINGS until
  // we have not read all results.
  DBUG_ASSERT(!mysql_more_results(mysql));

  MYSQL_RES *warn_res;
  if (mysql_real_query_wrapper(mysql, "SHOW WARNINGS", 13))
    die("Error running query \"SHOW WARNINGS\": %s", mysql_error(mysql));

  if (!(warn_res = mysql_store_result_wrapper(mysql)))
    die("Warning count is %u but didn't get any warnings", count);

  DYNAMIC_STRING ds_warnings;
  init_dynamic_string(&ds_warnings, "", 1024, 1024);
  append_result(&ds_warnings, warn_res);
  mysql_free_result_wrapper(warn_res);

  if (disable_warnings &&
      (disabled_warnings->count() || enabled_warnings->count()))
    handle_warnings(ds, ds_warnings.str);
  else if (!disable_warnings)
    dynstr_append_mem(ds, ds_warnings.str, ds_warnings.length);

  dynstr_free(&ds_warnings);
  return ds->length;
}

/// Run query using MySQL C API
///
/// @param cn          Connection object
/// @param command     Pointer to the st_command structure which holds the
///                    arguments and information for the command.
/// @param flags       Flags indicating if we should SEND and/or REAP.
/// @param query       Query string
/// @param query_len   Length of the query string
/// @param ds          Output buffer to store the query result.
/// @param ds_warnings Buffer to store the warnings generated while
///                    executing the query.
static void run_query_normal(struct st_connection *cn,
                             struct st_command *command, int flags,
                             const char *query, size_t query_len,
                             DYNAMIC_STRING *ds, DYNAMIC_STRING *ds_warnings) {
  int error = 0;
  std::uint32_t counter = 0;
  MYSQL *mysql = &cn->mysql;
  MYSQL_RES *res = nullptr;

  if (flags & QUERY_SEND_FLAG) {
    /* Send the query */
    if (mysql_send_query_wrapper(&cn->mysql, query,
                                 static_cast<ulong>(query_len))) {
      handle_error(command, mysql_errno(mysql), mysql_error(mysql),
                   mysql_sqlstate(mysql), ds);
      goto end;
    }
  }

  if (!(flags & QUERY_REAP_FLAG)) {
    cn->pending = true;
    return;
  }

  do {
    /*
      When  on first result set, call mysql_read_query_result_wrapper to
      retrieve answer to the query sent earlier
    */
    if ((counter == 0) && mysql_read_query_result_wrapper(&cn->mysql)) {
      /* we've failed to collect the result set */
      cn->pending = true;
      handle_error(command, mysql_errno(mysql), mysql_error(mysql),
                   mysql_sqlstate(mysql), ds);
      goto end;
    }

    /*
      Store the result of the query if it will return any fields
    */
    if (mysql_field_count(mysql) &&
        ((res = mysql_store_result_wrapper(mysql)) == nullptr)) {
      handle_error(command, mysql_errno(mysql), mysql_error(mysql),
                   mysql_sqlstate(mysql), ds);
      goto end;
    }

    if (!disable_result_log) {
      if (res) {
        MYSQL_FIELD *fields = mysql_fetch_fields(res);
        std::uint32_t num_fields = mysql_num_fields(res);

        if (display_metadata) append_metadata(ds, fields, num_fields);

        if (!display_result_vertically)
          append_table_headings(ds, fields, num_fields);

        append_result(ds, res);
      }

      // Need to call mysql_affected_rows() before the "new"
      // query to find the warnings.
      if (!disable_info)
        append_info(ds, mysql_affected_rows(mysql), mysql_info(mysql));

      if (display_session_track_info) append_session_track_info(ds, mysql);

      // Add all warnings to the result. We can't do this if we are in
      // the middle of processing results from multi-statement, because
      // this will break protocol.
      if ((!disable_warnings || disabled_warnings->count() ||
           enabled_warnings->count()) &&
          !mysql_more_results(mysql)) {
        if (append_warnings(ds_warnings, mysql) || ds_warnings->length) {
          dynstr_append_mem(ds, "Warnings:\n", 10);
          dynstr_append_mem(ds, ds_warnings->str, ds_warnings->length);
        }
      }
    }

    if (res) {
      mysql_free_result_wrapper(res);
      res = nullptr;
    }
    counter++;
  } while (!(error = mysql_next_result_wrapper(mysql)));
  if (error > 0) {
    // We got an error from mysql_next_result, maybe expected.
    handle_error(command, mysql_errno(mysql), mysql_error(mysql),
                 mysql_sqlstate(mysql), ds);
    goto end;
  }

  // Successful and there are no more results.
  DBUG_ASSERT(error == -1);

  // If we come here the query is both executed and read successfully.
  handle_no_error(command);
  revert_properties();

end:
  cn->pending = false;

  // We save the return code (mysql_errno(mysql)) from the last call sent
  // to the server into the mysqltest builtin variable $mysql_errno. This
  // variable then can be used from the test case itself.
  var_set_errno(mysql_errno(mysql));
}

/// Run query using prepared statement C API
///
/// @param mysql       mysql handle
/// @param command     Pointer to the st_command structure which holds the
///                    arguments and information for the command.
/// @param query       Query string
/// @param query_len   Length of the query string
/// @param ds          Output buffer to store the query result.
/// @param ds_warnings Buffer to store the warnings generated while
///                    executing the query.
static void run_query_stmt(MYSQL *mysql, struct st_command *command,
                           const char *query, size_t query_len,
                           DYNAMIC_STRING *ds, DYNAMIC_STRING *ds_warnings) {
  // Init a new stmt if it's not already one created for this connection.
  MYSQL_STMT *stmt;
  if (!(stmt = cur_con->stmt)) {
    if (!(stmt = mysql_stmt_init(mysql))) die("unable to init stmt structure");
    cur_con->stmt = stmt;
  }

  DYNAMIC_STRING ds_prepare_warnings;
  DYNAMIC_STRING ds_execute_warnings;

  // Init dynamic strings for warnings.
  if (!disable_warnings || disabled_warnings->count() ||
      enabled_warnings->count()) {
    init_dynamic_string(&ds_prepare_warnings, nullptr, 0, 256);
    init_dynamic_string(&ds_execute_warnings, nullptr, 0, 256);
  }

  // Note that here 'res' is meta data result set
  MYSQL_RES *res = nullptr;
  int err = 0;

  // Prepare the query
  if (mysql_stmt_prepare(stmt, query, static_cast<ulong>(query_len))) {
    handle_error(command, mysql_stmt_errno(stmt), mysql_stmt_error(stmt),
                 mysql_stmt_sqlstate(stmt), ds);
    goto end;
  }

  // Get the warnings from mysql_stmt_prepare and keep them in a
  // separate string.
  if (!disable_warnings || disabled_warnings->count() ||
      enabled_warnings->count())
    append_warnings(&ds_prepare_warnings, mysql);

  // No need to call mysql_stmt_bind_param() because we have no
  // parameter markers.
  if (cursor_protocol_enabled) {
    // Use cursor when retrieving result.
    unsigned long type = CURSOR_TYPE_READ_ONLY;
    if (mysql_stmt_attr_set(stmt, STMT_ATTR_CURSOR_TYPE, (void *)&type))
      die("mysql_stmt_attr_set(STMT_ATTR_CURSOR_TYPE) failed': %d %s",
          mysql_stmt_errno(stmt), mysql_stmt_error(stmt));
  }

  // Execute the query
  if (mysql_stmt_execute(stmt)) {
    handle_error(command, mysql_stmt_errno(stmt), mysql_stmt_error(stmt),
                 mysql_stmt_sqlstate(stmt), ds);
    goto end;
  }

  // When running in cursor_protocol get the warnings from execute here
  // and keep them in a separate string for later.
  if (cursor_protocol_enabled &&
      (!disable_warnings || disabled_warnings->count() ||
       enabled_warnings->count()))
    append_warnings(&ds_execute_warnings, mysql);

  // We instruct that we want to update the "max_length" field in
  // mysql_stmt_store_result(), this is our only way to know how much
  // buffer to allocate for result data
  {
    bool one = true;
    if (mysql_stmt_attr_set(stmt, STMT_ATTR_UPDATE_MAX_LENGTH, (void *)&one))
      die("mysql_stmt_attr_set(STMT_ATTR_UPDATE_MAX_LENGTH) failed': %d %s",
          mysql_stmt_errno(stmt), mysql_stmt_error(stmt));
  }

  do {
    // If we got here the statement succeeded and was expected to do so,
    // get data. Note that this can still give errors found during execution.
    // Store the result of the query if if will return any fields
    if (mysql_stmt_field_count(stmt) && mysql_stmt_store_result(stmt)) {
      handle_error(command, mysql_stmt_errno(stmt), mysql_stmt_error(stmt),
                   mysql_stmt_sqlstate(stmt), ds);
      goto end;
    }

    if (!disable_result_log) {
      // Not all statements creates a result set. If there is one we can
      // now create another normal result set that contains the meta
      // data. This set can be handled almost like any other non prepared
      // statement result set.
      if ((res = mysql_stmt_result_metadata(stmt)) != nullptr) {
        // Take the column count from meta info
        MYSQL_FIELD *fields = mysql_fetch_fields(res);
        std::uint32_t num_fields = mysql_num_fields(res);

        if (display_metadata) append_metadata(ds, fields, num_fields);

        if (!display_result_vertically)
          append_table_headings(ds, fields, num_fields);

        append_stmt_result(ds, stmt, fields, num_fields);

        // Free normal result set with meta data
        mysql_free_result_wrapper(res);

        // Clear prepare warnings if there are execute warnings,
        // since they are probably duplicated.
        if (ds_execute_warnings.length || mysql->warning_count)
          dynstr_set(&ds_prepare_warnings, nullptr);
      } else {
        // This is a query without resultset
      }

      // Fetch info before fetching warnings, since it will be reset
      // otherwise.
      if (!disable_info)
        append_info(ds, mysql_affected_rows(stmt->mysql), mysql_info(mysql));

      if (display_session_track_info) append_session_track_info(ds, mysql);

      // Add all warnings to the result. We can't do this if we are in
      // the middle of processing results from multi-statement, because
      // this will break protocol.
      if ((!disable_warnings || disabled_warnings->count() ||
           enabled_warnings->count()) &&
          !mysql_more_results(stmt->mysql)) {
        // Get the warnings from execute. Append warnings to ds,
        // if there are any.
        append_warnings(&ds_execute_warnings, mysql);
        if (ds_execute_warnings.length || ds_prepare_warnings.length ||
            ds_warnings->length) {
          dynstr_append_mem(ds, "Warnings:\n", 10);

          // Append warnings if exist any
          if (ds_warnings->length)
            dynstr_append_mem(ds, ds_warnings->str, ds_warnings->length);

          // Append prepare warnings if exist any
          if (ds_prepare_warnings.length)
            dynstr_append_mem(ds, ds_prepare_warnings.str,
                              ds_prepare_warnings.length);

          // Append execute warnings if exist any
          if (ds_execute_warnings.length)
            dynstr_append_mem(ds, ds_execute_warnings.str,
                              ds_execute_warnings.length);
        }
      }
    }
  } while ((err = mysql_stmt_next_result(stmt)) == 0);

  if (err > 0) {
    // We got an error from mysql_stmt_next_result, maybe expected.
    handle_error(command, mysql_stmt_errno(stmt), mysql_stmt_error(stmt),
                 mysql_stmt_sqlstate(stmt), ds);
    goto end;
  }

  // If we got here the statement was both executed and read successfully.
  handle_no_error(command);

end:
  if (!disable_warnings || disabled_warnings->count() ||
      enabled_warnings->count()) {
    dynstr_free(&ds_prepare_warnings);
    dynstr_free(&ds_execute_warnings);
  }
  revert_properties();

  // We save the return code (mysql_stmt_errno(stmt)) from the last call sent
  // to the server into the mysqltest builtin variable $mysql_errno. This
  // variable then can be used from the test case itself.
  var_set_errno(mysql_stmt_errno(stmt));

  // Close the statement if no reconnect, need new prepare.
  if (mysql->reconnect) {
    mysql_stmt_close(stmt);
    cur_con->stmt = nullptr;
  }
}

/*
  Create a util connection if one does not already exists
  and use that to run the query
  This is done to avoid implict commit when creating/dropping objects such
  as view, sp etc.
*/

static int util_query(MYSQL *org_mysql, const char *query) {
  MYSQL *mysql;
  DBUG_TRACE;

  if (!(mysql = cur_con->util_mysql)) {
    DBUG_PRINT("info", ("Creating util_mysql"));
    if (!(mysql = mysql_init(mysql))) die("Failed in mysql_init()");

    if (opt_connect_timeout)
      mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT,
                    (void *)&opt_connect_timeout);

    /* enable local infile, in non-binary builds often disabled by default */
    mysql_options(mysql, MYSQL_OPT_LOCAL_INFILE, nullptr);
    safe_connect(mysql, "util", org_mysql->host, org_mysql->user,
                 org_mysql->passwd, org_mysql->db, org_mysql->port,
                 org_mysql->unix_socket);

    cur_con->util_mysql = mysql;
  }

  return mysql_query_wrapper(mysql, query);
}

/*
  Run query

  SYNPOSIS
    run_query()
     mysql	mysql handle
     command	currrent command pointer

  flags control the phased/stages of query execution to be performed
  if QUERY_SEND_FLAG bit is on, the query will be sent. If QUERY_REAP_FLAG
  is on the result will be read - for regular query, both bits must be on
*/

static void run_query(struct st_connection *cn, struct st_command *command,
                      int flags) {
  MYSQL *mysql = &cn->mysql;
  DYNAMIC_STRING *ds;
  DYNAMIC_STRING *save_ds = nullptr;
  DYNAMIC_STRING ds_sorted;
  DYNAMIC_STRING ds_warnings;
  DYNAMIC_STRING eval_query;
  const char *query;
  size_t query_len;
  bool view_created = false, sp_created = false;
  bool complete_query =
      ((flags & QUERY_SEND_FLAG) && (flags & QUERY_REAP_FLAG));
  DBUG_TRACE;
  dynstr_set(&ds_result, "");

  if (cn->pending && (flags & QUERY_SEND_FLAG))
    die("Cannot run query on connection between send and reap");

  if (!(flags & QUERY_SEND_FLAG) && !cn->pending)
    die("Cannot reap on a connection without pending send");

  init_dynamic_string(&ds_warnings, nullptr, 0, 256);
  ds_warn = &ds_warnings;

  /*
    Evaluate query if this is an eval command
  */
  if (command->type == Q_EVAL || command->type == Q_SEND_EVAL) {
    init_dynamic_string(&eval_query, "", command->query_len + 256, 1024);
    do_eval(&eval_query, command->query, command->end, false);
    query = eval_query.str;
    query_len = eval_query.length;
  } else {
    query = command->query;
    query_len = std::strlen(query);
  }

  /*
    Create a temporary dynamic string to contain the
    output from this query.
  */
  if (command->output_file[0])
    ds = &ds_result;
  else
    ds = &ds_res;

  /*
    Log the query into the output buffer
  */
  if (!disable_query_log && (flags & QUERY_SEND_FLAG)) {
    replace_dynstr_append_mem(ds, query, query_len);
    dynstr_append_mem(ds, delimiter, delimiter_length);
    dynstr_append_mem(ds, "\n", 1);
  }

  if (view_protocol_enabled && complete_query &&
      search_protocol_re(&view_re, query)) {
    /*
      Create the query as a view.
      Use replace since view can exist from a failed mysqltest run
    */
    DYNAMIC_STRING query_str;
    init_dynamic_string(&query_str,
                        "CREATE OR REPLACE VIEW mysqltest_tmp_v AS ",
                        query_len + 64, 256);
    dynstr_append_mem(&query_str, query, query_len);
    if (util_query(mysql, query_str.str)) {
      /*
        Failed to create the view, this is not fatal
        just run the query the normal way
      */
      DBUG_PRINT("view_create_error",
                 ("Failed to create view '%s': %d: %s", query_str.str,
                  mysql_errno(mysql), mysql_error(mysql)));

      /* Log error to create view */
      verbose_msg("Failed to create view '%s' %d: %s", query_str.str,
                  mysql_errno(mysql), mysql_error(mysql));
    } else {
      /*
        Yes, it was possible to create this query as a view
      */
      view_created = true;
      query = "SELECT * FROM mysqltest_tmp_v";
      query_len = std::strlen(query);

      /*
        Collect warnings from create of the view that should otherwise
        have been produced when the SELECT was executed
      */
      append_warnings(&ds_warnings, cur_con->util_mysql);
    }

    dynstr_free(&query_str);
  }

  if (sp_protocol_enabled && complete_query &&
      search_protocol_re(&sp_re, query)) {
    /*
      Create the query as a stored procedure
      Drop first since sp can exist from a failed mysqltest run
    */
    DYNAMIC_STRING query_str;
    init_dynamic_string(&query_str,
                        "DROP PROCEDURE IF EXISTS mysqltest_tmp_sp;",
                        query_len + 64, 256);
    util_query(mysql, query_str.str);
    dynstr_set(&query_str, "CREATE PROCEDURE mysqltest_tmp_sp()\n");
    dynstr_append_mem(&query_str, query, query_len);
    if (util_query(mysql, query_str.str)) {
      /*
        Failed to create the stored procedure for this query,
        this is not fatal just run the query the normal way
      */
      DBUG_PRINT("sp_create_error",
                 ("Failed to create sp '%s': %d: %s", query_str.str,
                  mysql_errno(mysql), mysql_error(mysql)));

      /* Log error to create sp */
      verbose_msg("Failed to create sp '%s' %d: %s", query_str.str,
                  mysql_errno(mysql), mysql_error(mysql));

    } else {
      sp_created = true;

      query = "CALL mysqltest_tmp_sp()";
      query_len = std::strlen(query);
    }
    dynstr_free(&query_str);
  }

  if (display_result_sorted) {
    /*
       Collect the query output in a separate string
       that can be sorted before it's added to the
       global result string
    */
    init_dynamic_string(&ds_sorted, "", 1024, 1024);
    save_ds = ds; /* Remember original ds */
    ds = &ds_sorted;
  }

  /*
    Find out how to run this query

    Always run with normal C API if it's not a complete
    SEND + REAP

    If it is a '?' in the query it may be a SQL level prepared
    statement already and we can't do it twice
  */
  if (ps_protocol_enabled && complete_query &&
      search_protocol_re(&ps_re, query))
    run_query_stmt(mysql, command, query, query_len, ds, &ds_warnings);
  else
    run_query_normal(cn, command, flags, query, query_len, ds, &ds_warnings);

  dynstr_free(&ds_warnings);
  ds_warn = nullptr;
  if (command->type == Q_EVAL || command->type == Q_SEND_EVAL)
    dynstr_free(&eval_query);

  if (display_result_sorted) {
    /* Sort the result set and append it to result */
    dynstr_append_sorted(save_ds, &ds_sorted, start_sort_column);
    ds = save_ds;
    dynstr_free(&ds_sorted);
  }

  if (sp_created) {
    if (util_query(mysql, "DROP PROCEDURE mysqltest_tmp_sp "))
      die("Failed to drop sp: %d: %s", mysql_errno(mysql), mysql_error(mysql));
  }

  if (view_created) {
    if (util_query(mysql, "DROP VIEW mysqltest_tmp_v "))
      die("Failed to drop view: %d: %s", mysql_errno(mysql),
          mysql_error(mysql));
  }
  if (command->output_file[0]) {
    /* An output file was specified for _this_ query */
    str_to_file2(command->output_file, ds_result.str, ds_result.length, false);
    command->output_file[0] = 0;
  }
}

/**
   Display the optimizer trace produced by the last executed statement.
 */
static void display_opt_trace(struct st_connection *cn,
                              struct st_command *command, int flags) {
  if (!disable_query_log && opt_trace_protocol_enabled && !cn->pending &&
      !expected_errors->count() &&
      search_protocol_re(&opt_trace_re, command->query)) {
    st_command save_command = *command;
    DYNAMIC_STRING query_str;
    init_dynamic_string(&query_str,
                        "SELECT trace FROM information_schema.optimizer_trace"
                        " /* injected by --opt-trace-protocol */",
                        128, 128);

    command->query = query_str.str;
    command->query_len = query_str.length;
    command->end = strend(command->query);

    /* Sorted trace is not readable at all, don't bother to lower case */
    /* No need to keep old values, will be reset anyway */
    display_result_sorted = false;
    display_result_lower = false;
    run_query(cn, command, flags);

    dynstr_free(&query_str);
    *command = save_command;
  }
}

static void run_explain(struct st_connection *cn, struct st_command *command,
                        int flags, bool json) {
  if ((flags & QUERY_REAP_FLAG) && !expected_errors->count() &&
      search_protocol_re(&explain_re, command->query)) {
    st_command save_command = *command;
    DYNAMIC_STRING query_str;
    DYNAMIC_STRING ds_warning_messages;

    init_dynamic_string(&ds_warning_messages, "", 0, 2048);
    init_dynamic_string(&query_str, json ? "EXPLAIN FORMAT=JSON " : "EXPLAIN ",
                        256, 256);
    dynstr_append_mem(&query_str, command->query,
                      command->end - command->query);

    command->query = query_str.str;
    command->query_len = query_str.length;
    command->end = strend(command->query);

    run_query(cn, command, flags);

    dynstr_free(&query_str);
    dynstr_free(&ds_warning_messages);

    *command = save_command;
  }
}

static void get_command_type(struct st_command *command) {
  char save;
  uint type;
  DBUG_TRACE;

  if (*command->query == '}') {
    command->type = Q_END_BLOCK;
    return;
  }

  save = command->query[command->first_word_len];
  command->query[command->first_word_len] = 0;
  type = find_type(command->query, &command_typelib, FIND_TYPE_NO_PREFIX);
  command->query[command->first_word_len] = save;
  if (type > 0) {
    command->type = (enum enum_commands)type; /* Found command */

    /*
      Look for case where "query" was explicitly specified to
      force command being sent to server
    */
    if (type == Q_QUERY) {
      /* Skip the "query" part */
      command->query = command->first_argument;
    }
  } else {
    /* No mysqltest command matched */

    if (command->type != Q_COMMENT_WITH_COMMAND) {
      /* A query that will sent to mysqld */
      command->type = Q_QUERY;
    } else {
      /* -- "comment" that didn't contain a mysqltest command */
      die("Found line '%s' beginning with -- that didn't contain "
          "a valid mysqltest command, check your syntax or "
          "use # if you intended to write a comment",
          command->query);
    }
  }
}

/// Record how many milliseconds it took to execute the test file
/// up until the current line and write it to .progress file.
///
/// @param progress_file Logfile object to store the progress information
/// @param line          Line number of the progress file where the progress
///                      information should be recorded.
static void mark_progress(Logfile *progress_file, int line) {
  static unsigned long long int progress_start = 0;
  unsigned long long int timer = timer_now();

  if (!progress_start) progress_start = timer;
  timer = timer - progress_start;

  std::string str_progress;

  // Milliseconds since start
  std::string str_timer = std::to_string(timer);
  str_progress.append(str_timer);
  str_progress.append("\t");

  // Parse the line number
  std::string str_line = std::to_string(line);
  str_progress.append(str_line);
  str_progress.append("\t");

  // Filename
  str_progress.append(cur_file->file_name);
  str_progress.append(":");

  // Line in file
  str_line = std::to_string(cur_file->lineno);
  str_progress.append(str_line);
  str_progress.append("\n");

  if (progress_file->write(str_progress.c_str(), str_progress.length()) ||
      progress_file->flush()) {
    cleanup_and_exit(1);
  }
}

#ifdef HAVE_STACKTRACE
static void dump_backtrace() {
  struct st_connection *conn = cur_con;

  fprintf(stderr, "mysqltest: ");

  // Print the query and the line number
  if (start_lineno > 0) fprintf(stderr, "At line %u: ", start_lineno);
  fprintf(stderr, "%s\n", curr_command->query);

  // Print the file stack
  if (cur_file && cur_file != file_stack) {
    fprintf(stderr, "In included ");
    print_file_stack();
  }

  if (conn) fprintf(stderr, "conn->name: %s\n", conn->name);

  fprintf(stderr, "Attempting backtrace.\n");
  fflush(stderr);
  my_print_stacktrace(nullptr, my_thread_stack_size);
}

#else
static void dump_backtrace() { fputs("Backtrace not available.\n", stderr); }

#endif

static void signal_handler(int sig) {
  fprintf(stderr, "mysqltest got " SIGNAL_FMT "\n", sig);
  dump_backtrace();

  fprintf(stderr, "Writing a core file.\n");
  fflush(stderr);
  my_write_core(sig);
#ifndef _WIN32
  // Shouldn't get here but just in case
  exit(1);
#endif
}

#ifdef _WIN32

LONG WINAPI exception_filter(EXCEPTION_POINTERS *exp) {
  __try {
    my_set_exception_pointers(exp);
    signal_handler(exp->ExceptionRecord->ExceptionCode);
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    fputs("Got exception in exception handler!\n", stderr);
  }

  return EXCEPTION_CONTINUE_SEARCH;
}

static void init_signal_handling(void) {
  UINT mode;

  mysqltest_thread = OpenThread(THREAD_ALL_ACCESS, FALSE, GetCurrentThreadId());
  if (mysqltest_thread == NULL)
    die("OpenThread failed, err = %d.", GetLastError());

  /* Set output destination of messages to the standard error stream. */
  _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
  _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
  _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
  _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
  _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
  _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);

  /* Do not not display the a error message box. */
  mode = SetErrorMode(0) | SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX;
  SetErrorMode(mode);

  SetUnhandledExceptionFilter(exception_filter);
}

/// Function to handle the stacktrace request event.
///
/// - Suspend the thread running the test
/// - Fetch CONTEXT record from the thread handle
/// - Initialize EXCEPTION_RECORD structure
/// - Use EXCEPTION_POINTERS and EXCEPTION_RECORD to set EXCEPTION_POINTERS
///   structure
/// - Call exception_filter() method to generate to stack trace
/// - Resume the suspended test thread
static void handle_wait_stacktrace_request_event() {
  fprintf(stderr, "Test case timeout failure.\n");

  // Suspend the thread running the test
  if (SuspendThread(mysqltest_thread) == -1) {
    DWORD error = GetLastError();
    CloseHandle(mysqltest_thread);
    die("Error suspending thread, err = %d.\n", error);
  }

  // Fetch the thread context
  CONTEXT test_thread_ctx = {0};
  test_thread_ctx.ContextFlags = CONTEXT_FULL;

  if (GetThreadContext(mysqltest_thread, &test_thread_ctx) == FALSE) {
    DWORD error = GetLastError();
    CloseHandle(mysqltest_thread);
    die("Error while fetching thread conext information, err = %d.\n", error);
  }

  EXCEPTION_POINTERS exp = {0};
  exp.ContextRecord = &test_thread_ctx;

  // Set up an Exception record with EXCEPTION_BREAKPOINT code
  EXCEPTION_RECORD exc_rec = {0};
  exc_rec.ExceptionCode = EXCEPTION_BREAKPOINT;
  exp.ExceptionRecord = &exc_rec;

  exception_filter(&exp);

  // Resume the suspended test thread
  if (ResumeThread(mysqltest_thread) == -1) {
    DWORD error = GetLastError();
    CloseHandle(mysqltest_thread);
    die("Error resuming thread, err = %d.\n", error);
  }

  my_set_exception_pointers(nullptr);
}

/// Thread waiting for timeout event to occur. If the event occurs,
/// this method will trigger signal_handler() function.
static void wait_stacktrace_request_event() {
  DWORD wait_res = WaitForSingleObject(stacktrace_request_event, INFINITE);
  switch (wait_res) {
    case WAIT_OBJECT_0:
      handle_wait_stacktrace_request_event();
      break;
    default:
      die("Unexpected result %d from WaitForSingleObject.", wait_res);
      break;
  }
  CloseHandle(stacktrace_request_event);
}

/// Create an event name from the safeprocess PID value of the form
/// mysqltest[%d]stacktrace and spawn thread waiting for that event
/// to occur.
///
/// When this event occurs, signal_handler() method is called and
/// stacktrace for the mysqltest client process is printed in the
/// log file.
static void create_stacktrace_request_event() {
  char event_name[64];
  std::sprintf(event_name, "mysqltest[%d]stacktrace", opt_safe_process_pid);

  // Create an event for the signal handler
  if ((stacktrace_request_event = CreateEvent(NULL, TRUE, FALSE, event_name)) ==
      NULL)
    die("Failed to create timeout_event.");

  wait_for_stacktrace_request_event_thread =
      std::thread(wait_stacktrace_request_event);
}

#else /* _WIN32 */

static void init_signal_handling(void) {
  struct sigaction sa;
  DBUG_TRACE;

#ifdef HAVE_STACKTRACE
  my_init_stacktrace();
#endif

  sa.sa_flags = SA_RESETHAND | SA_NODEFER;
  sigemptyset(&sa.sa_mask);
  sigprocmask(SIG_SETMASK, &sa.sa_mask, nullptr);

  sa.sa_handler = signal_handler;

  sigaction(SIGSEGV, &sa, nullptr);
  sigaction(SIGABRT, &sa, nullptr);
#ifdef SIGBUS
  sigaction(SIGBUS, &sa, nullptr);
#endif
  sigaction(SIGILL, &sa, nullptr);
  sigaction(SIGFPE, &sa, nullptr);
}

#endif /* !_WIN32 */

int main(int argc, char **argv) {
  struct st_command *command;
  bool abort_flag = false;
  int q_send_flag = 0;
  uint command_executed = 0, last_command_executed = 0;
  char output_file[FN_REFLEN];
  MY_INIT(argv[0]);

  output_file[0] = 0;
  TMPDIR[0] = 0;

  init_signal_handling();

  /* Init file stack */
  memset(file_stack, 0, sizeof(file_stack));
  file_stack_end =
      file_stack + (sizeof(file_stack) / sizeof(struct st_test_file)) - 1;
  cur_file = file_stack;

  /* Init block stack */
  memset(block_stack, 0, sizeof(block_stack));
  block_stack_end =
      block_stack + (sizeof(block_stack) / sizeof(struct st_block)) - 1;
  cur_block = block_stack;
  cur_block->ok = true; /* Outer block should always be executed */
  cur_block->cmd = cmd_none;

  q_lines = new Q_lines(PSI_NOT_INSTRUMENTED);

  var_hash =
      new collation_unordered_map<std::string, std::unique_ptr<VAR, var_free>>(
          charset_info, PSI_NOT_INSTRUMENTED);

  {
    char path_separator[] = {FN_LIBCHAR, 0};
    var_set_string("SYSTEM_PATH_SEPARATOR", path_separator);
  }
  var_set_string("MYSQL_SERVER_VERSION", MYSQL_SERVER_VERSION);
  var_set_string("MYSQL_SYSTEM_TYPE", SYSTEM_TYPE);
  var_set_string("MYSQL_MACHINE_TYPE", MACHINE_TYPE);
  if (sizeof(void *) == 8) {
    var_set_string("MYSQL_SYSTEM_ARCHITECTURE", "64");
  } else {
    var_set_string("MYSQL_SYSTEM_ARCHITECTURE", "32");
  }

  memset(&master_pos, 0, sizeof(master_pos));

  parser.current_line = parser.read_lines = 0;
  memset(&var_reg, 0, sizeof(var_reg));

  init_builtin_echo();
#ifdef _WIN32
  is_windows = 1;
  init_win_path_patterns();
#endif

  init_dynamic_string(&ds_res, "", 2048, 2048);
  init_dynamic_string(&ds_result, "", 1024, 1024);

  parse_args(argc, argv);

#ifdef _WIN32
  // Create an event to request stack trace when timeout occurs
  if (opt_safe_process_pid) create_stacktrace_request_event();
#endif

  /* Init connections, allocate 1 extra as buffer + 1 for default */
  connections = (struct st_connection *)my_malloc(
      PSI_NOT_INSTRUMENTED,
      (opt_max_connections + 2) * sizeof(struct st_connection),
      MYF(MY_WME | MY_ZEROFILL));
  connections_end = connections + opt_max_connections + 1;
  next_con = connections + 1;

  var_set_int("$PS_PROTOCOL", ps_protocol);
  var_set_int("$SP_PROTOCOL", sp_protocol);
  var_set_int("$VIEW_PROTOCOL", view_protocol);
  var_set_int("$OPT_TRACE_PROTOCOL", opt_trace_protocol);
  var_set_int("$EXPLAIN_PROTOCOL", explain_protocol);
  var_set_int("$JSON_EXPLAIN_PROTOCOL", json_explain_protocol);
  var_set_int("$CURSOR_PROTOCOL", cursor_protocol);

  var_set_int("$ENABLE_QUERY_LOG", 1);
  var_set_int("$ENABLE_ABORT_ON_ERROR", 1);
  var_set_int("$ENABLE_RESULT_LOG", 1);
  var_set_int("$ENABLE_CONNECT_LOG", 0);
  var_set_int("$ENABLE_WARNINGS", 1);
  var_set_int("$ENABLE_INFO", 0);
  var_set_int("$ENABLE_METADATA", 0);
  var_set_int("$ENABLE_ASYNC_CLIENT", 0);

  DBUG_PRINT("info",
             ("result_file: '%s'", result_file_name ? result_file_name : ""));
  verbose_msg("Results saved in '%s'.",
              result_file_name ? result_file_name : "");
  if (mysql_server_init(0, nullptr, nullptr))
    die("Can't initialize MySQL server");
  server_initialized = true;
  if (cur_file == file_stack && cur_file->file == nullptr) {
    cur_file->file = stdin;
    cur_file->file_name =
        my_strdup(PSI_NOT_INSTRUMENTED, "<stdin>", MYF(MY_WME));
    cur_file->lineno = 1;
  }

  if (log_file.open(opt_logdir, result_file_name, ".log")) cleanup_and_exit(1);

  verbose_msg("Logging to '%s'.", log_file.file_name());
  enable_async_client = use_async_client;

  // Creating a log file using current file name if result file doesn't exist.
  if (result_file_name) {
    if (log_file.open(opt_logdir, result_file_name, ".log"))
      cleanup_and_exit(1);
  } else {
    if (std::strcmp(cur_file->file_name, "<stdin>")) {
      if (log_file.open(opt_logdir, cur_file->file_name, ".log"))
        cleanup_and_exit(1);
    } else {
      if (log_file.open(opt_logdir, "stdin", ".log")) cleanup_and_exit(1);
    }
  }

  // File to store the progress
  Logfile progress_file;

  if (opt_mark_progress) {
    if (result_file_name) {
      if (progress_file.open(opt_logdir, result_file_name, ".progress"))
        cleanup_and_exit(1);
    } else {
      if (std::strcmp(cur_file->file_name, "<stdin>")) {
        if (progress_file.open(opt_logdir, cur_file->file_name, ".progress"))
          cleanup_and_exit(1);
      } else {
        if (progress_file.open(opt_logdir, "stdin", ".progress"))
          cleanup_and_exit(1);
      }
    }
    verbose_msg("Tracing progress in '%s'.", progress_file.file_name());
  }

  var_set_string("MYSQLTEST_FILE", cur_file->file_name);

  /* Cursor protcol implies ps protocol */
  if (cursor_protocol) ps_protocol = true;

  ps_protocol_enabled = ps_protocol;
  sp_protocol_enabled = sp_protocol;
  view_protocol_enabled = view_protocol;
  opt_trace_protocol_enabled = opt_trace_protocol;
  explain_protocol_enabled = explain_protocol;
  json_explain_protocol_enabled = json_explain_protocol;
  cursor_protocol_enabled = cursor_protocol;

  st_connection *con = connections;
  if (!(mysql_init(&con->mysql))) die("Failed in mysql_init()");
  if (opt_connect_timeout)
    mysql_options(&con->mysql, MYSQL_OPT_CONNECT_TIMEOUT,
                  (void *)&opt_connect_timeout);
  if (opt_compress) mysql_options(&con->mysql, MYSQL_OPT_COMPRESS, NullS);
  mysql_options(&con->mysql, MYSQL_OPT_LOCAL_INFILE, nullptr);
  if (std::strcmp(default_charset, charset_info->csname) &&
      !(charset_info =
            get_charset_by_csname(default_charset, MY_CS_PRIMARY, MYF(MY_WME))))
    die("Invalid character set specified.");
  mysql_options(&con->mysql, MYSQL_SET_CHARSET_NAME, charset_info->csname);
  if (opt_charsets_dir)
    mysql_options(&con->mysql, MYSQL_SET_CHARSET_DIR, opt_charsets_dir);

  if (opt_protocol)
    mysql_options(&con->mysql, MYSQL_OPT_PROTOCOL, (char *)&opt_protocol);

  /* Turn on VERIFY_IDENTITY mode only if host=="localhost". */
  if (opt_ssl_mode == SSL_MODE_VERIFY_IDENTITY) {
    if (!opt_host || std::strcmp(opt_host, "localhost"))
      opt_ssl_mode = SSL_MODE_VERIFY_CA;
  }

  if (SSL_SET_OPTIONS(&con->mysql)) die("%s", SSL_SET_OPTIONS_ERROR);
#if defined(_WIN32)
  if (shared_memory_base_name)
    mysql_options(&con->mysql, MYSQL_SHARED_MEMORY_BASE_NAME,
                  shared_memory_base_name);

  if (opt_ssl_mode == SSL_MODE_DISABLED)
    mysql_options(&con->mysql, MYSQL_OPT_PROTOCOL,
                  (char *)&opt_protocol_for_default_connection);
#endif

  if (!(con->name = my_strdup(PSI_NOT_INSTRUMENTED, "default", MYF(MY_WME))))
    die("Out of memory");

  safe_connect(&con->mysql, con->name, opt_host, opt_user, opt_pass, opt_db,
               opt_port, unix_sock);

  /* Use all time until exit if no explicit 'start_timer' */
  timer_start = timer_now();

  /*
    Initialize $mysql_errno with -1, so we can
    - distinguish it from valid values ( >= 0 ) and
    - detect if there was never a command sent to the server
  */
  var_set_errno(-1);

  set_current_connection(con);

  if (opt_include) {
    open_file(opt_include);
  }

  if (opt_offload_count_file) {
    secondary_engine = new Secondary_engine();
    // Save the initial value of secondary engine execution status.
    if (secondary_engine->offload_count(&cur_con->mysql, "before"))
      cleanup_and_exit(1);
  }

  verbose_msg("Start processing test commands from '%s' ...",
              cur_file->file_name);
  while (!read_command(&command) && !abort_flag) {
    int current_line_inc = 1, processed = 0;
    if (command->type == Q_UNKNOWN || command->type == Q_COMMENT_WITH_COMMAND)
      get_command_type(command);

    if (command->type == Q_ERROR && expected_errors->count())
      // Delete all the error codes from previous 'error' command.
      expected_errors->clear_list();

    if (testcase_disabled && command->type != Q_ENABLE_TESTCASE &&
        command->type != Q_DISABLE_TESTCASE) {
      // Test case is disabled, silently convert this line to a comment
      command->type = Q_COMMENT;
    }

    /* (Re-)set abort_on_error for this command */
    command->abort_on_error = (expected_errors->count() == 0 && abort_on_error);

    /* delimiter needs to be executed so we can continue to parse */
    bool ok_to_do = cur_block->ok || command->type == Q_DELIMITER;

    /*
      'source' command needs to be "done" the first time if it may get
      re-iterated over in a true context. This can only happen if there's
      a while loop at some level above the current block.
    */
    if (!ok_to_do && command->type == Q_SOURCE) {
      for (struct st_block *stb = cur_block - 1; stb >= block_stack; stb--) {
        if (stb->cmd == cmd_while) {
          ok_to_do = true;
          break;
        }
      }
    }

    /*
      Some commands need to be parsed in false context also to
      avoid any parsing errors.
    */
    if (!ok_to_do &&
        (command->type == Q_APPEND_FILE || command->type == Q_PERL ||
         command->type == Q_WRITE_FILE)) {
      ok_to_do = true;
    }

    if (ok_to_do) {
      command->last_argument = command->first_argument;
      processed = 1;
      /* Need to remember this for handle_error() */
      curr_command = command;
      switch (command->type) {
        case Q_CONNECT:
          do_connect(command);
          break;
        case Q_CONNECTION:
          select_connection(command);
          break;
        case Q_DISCONNECT:
        case Q_DIRTY_CLOSE:
          do_close_connection(command);
          break;
        case Q_ENABLE_QUERY_LOG:
          set_property(command, P_QUERY, false);
          break;
        case Q_DISABLE_QUERY_LOG:
          set_property(command, P_QUERY, true);
          break;
        case Q_ENABLE_ABORT_ON_ERROR:
          set_property(command, P_ABORT, true);
          break;
        case Q_DISABLE_ABORT_ON_ERROR:
          set_property(command, P_ABORT, false);
          break;
        case Q_ENABLE_RESULT_LOG:
          set_property(command, P_RESULT, false);
          break;
        case Q_DISABLE_RESULT_LOG:
          set_property(command, P_RESULT, true);
          break;
        case Q_ENABLE_CONNECT_LOG:
          set_property(command, P_CONNECT, false);
          break;
        case Q_DISABLE_CONNECT_LOG:
          set_property(command, P_CONNECT, true);
          break;
        case Q_ENABLE_WARNINGS:
          do_enable_warnings(command);
          break;
        case Q_DISABLE_WARNINGS:
          do_disable_warnings(command);
          break;
        case Q_ENABLE_INFO:
          set_property(command, P_INFO, false);
          break;
        case Q_DISABLE_INFO:
          set_property(command, P_INFO, true);
          break;
        case Q_ENABLE_SESSION_TRACK_INFO:
          set_property(command, P_SESSION_TRACK, true);
          break;
        case Q_DISABLE_SESSION_TRACK_INFO:
          set_property(command, P_SESSION_TRACK, false);
          break;
        case Q_DUMP_TIMED_OUT_CONNECTION_SOCKET_BUFFER:
          dump_timed_out_connection_socket_buffer(cur_con);
          break;
        case Q_ENABLE_METADATA:
          set_property(command, P_META, true);
          break;
        case Q_DISABLE_METADATA:
          set_property(command, P_META, false);
          break;
        case Q_SOURCE:
          do_source(command);
          break;
        case Q_SLEEP:
          do_sleep(command, false);
          break;
        case Q_REAL_SLEEP:
          do_sleep(command, true);
          break;
        case Q_WAIT_FOR_SLAVE_TO_STOP:
          do_wait_for_slave_to_stop(command);
          break;
        case Q_INC:
          do_modify_var(command, DO_INC);
          break;
        case Q_DEC:
          do_modify_var(command, DO_DEC);
          break;
        case Q_ECHO:
          do_echo(command);
          command_executed++;
          break;
        case Q_REMOVE_FILE:
          do_remove_file(command);
          break;
        case Q_REMOVE_FILES_WILDCARD:
          do_remove_files_wildcard(command);
          break;
        case Q_COPY_FILES_WILDCARD:
          do_copy_files_wildcard(command);
          break;
        case Q_MKDIR:
          do_mkdir(command);
          break;
        case Q_RMDIR:
          do_rmdir(command, false);
          break;
        case Q_FORCE_RMDIR:
          do_rmdir(command, true);
          break;
        case Q_FORCE_CPDIR:
          do_force_cpdir(command);
          break;
        case Q_LIST_FILES:
          do_list_files(command);
          break;
        case Q_LIST_FILES_WRITE_FILE:
          do_list_files_write_file_command(command, false);
          break;
        case Q_LIST_FILES_APPEND_FILE:
          do_list_files_write_file_command(command, true);
          break;
        case Q_FILE_EXIST:
          do_file_exist(command);
          break;
        case Q_WRITE_FILE:
          do_write_file(command);
          break;
        case Q_APPEND_FILE:
          do_append_file(command);
          break;
        case Q_DIFF_FILES:
          do_diff_files(command);
          break;
        case Q_SEND_QUIT:
          do_send_quit(command);
          break;
        case Q_CHANGE_USER:
          do_change_user(command);
          break;
        case Q_CAT_FILE:
          do_cat_file(command);
          break;
        case Q_COPY_FILE:
          do_copy_file(command);
          break;
        case Q_MOVE_FILE:
          do_move_file(command);
          break;
        case Q_CHMOD_FILE:
          do_chmod_file(command);
          break;
        case Q_PERL:
          do_perl(command);
          break;
        case Q_RESULT_FORMAT_VERSION:
          do_result_format_version(command);
          break;
        case Q_DELIMITER:
          do_delimiter(command);
          break;
        case Q_DISPLAY_VERTICAL_RESULTS:
          display_result_vertically = true;
          break;
        case Q_DISPLAY_HORIZONTAL_RESULTS:
          display_result_vertically = false;
          break;
        case Q_SORTED_RESULT:
          /*
            Turn on sorting of result set, will be reset after next
            command
          */
          display_result_sorted = true;
          start_sort_column = 0;
          break;
        case Q_PARTIALLY_SORTED_RESULT:
          /*
            Turn on sorting of result set, will be reset after next
            command
          */
          display_result_sorted = true;
          start_sort_column = atoi(command->first_argument);
          command->last_argument = command->end;
          break;
        case Q_LOWERCASE:
          /*
            Turn on lowercasing of result, will be reset after next
            command
          */
          display_result_lower = true;
          break;
        case Q_LET:
          do_let(command);
          break;
        case Q_EXPR:
          do_expr(command);
          break;
        case Q_EVAL:
        case Q_QUERY_VERTICAL:
        case Q_QUERY_HORIZONTAL:
          if (command->query == command->query_buf) {
            /* Skip the first part of command, i.e query_xxx */
            command->query = command->first_argument;
            command->first_word_len = 0;
          }
          /* fall through */
        case Q_QUERY:
        case Q_REAP: {
          bool old_display_result_vertically = display_result_vertically;
          /* Default is full query, both reap and send  */
          int flags = QUERY_REAP_FLAG | QUERY_SEND_FLAG;

          if (q_send_flag) {
            // Last command was an empty 'send' or 'send_eval'
            flags = QUERY_SEND_FLAG;
            if (q_send_flag == 2)
              // Last command was an empty 'send_eval' command. Set the command
              // type to Q_SEND_EVAL so that the variable gets replaced with its
              // value before executing.
              command->type = Q_SEND_EVAL;
            q_send_flag = 0;
          } else if (command->type == Q_REAP) {
            flags = QUERY_REAP_FLAG;
          }

          /* Check for special property for this query */
          display_result_vertically |= (command->type == Q_QUERY_VERTICAL);

          /*
            We run EXPLAIN _before_ the query. If query is UPDATE/DELETE is
            matters: a DELETE may delete rows, and then EXPLAIN DELETE will
            usually terminate quickly with "no matching rows". To make it more
            interesting, EXPLAIN is now first.
          */
          if (explain_protocol_enabled)
            run_explain(cur_con, command, flags, false);
          if (json_explain_protocol_enabled)
            run_explain(cur_con, command, flags, true);

          if (*output_file) {
            strmake(command->output_file, output_file, sizeof(output_file) - 1);
            *output_file = 0;
          }
          run_query(cur_con, command, flags);
          display_opt_trace(cur_con, command, flags);
          command_executed++;
          command->last_argument = command->end;

          /* Restore settings */
          display_result_vertically = old_display_result_vertically;

          break;
        }
        case Q_SEND:
        case Q_SEND_EVAL:
          if (!*command->first_argument) {
            // This is a 'send' or 'send_eval' command without arguments, it
            // indicates that _next_ query should be send only.
            if (command->type == Q_SEND)
              q_send_flag = 1;
            else if (command->type == Q_SEND_EVAL)
              q_send_flag = 2;
            break;
          }

          /* Remove "send" if this is first iteration */
          if (command->query == command->query_buf)
            command->query = command->first_argument;

          /*
            run_query() can execute a query partially, depending on the flags.
            QUERY_SEND_FLAG flag without QUERY_REAP_FLAG tells it to just send
            the query and read the result some time later when reap instruction
            is given on this connection.
          */
          run_query(cur_con, command, QUERY_SEND_FLAG);
          command_executed++;
          command->last_argument = command->end;
          break;
        case Q_ERROR:
          do_error(command);
          break;
        case Q_REPLACE:
          do_get_replace(command);
          break;
        case Q_REPLACE_REGEX:
          do_get_replace_regex(command);
          break;
        case Q_REPLACE_COLUMN:
          do_get_replace_column(command);
          break;
        case Q_REPLACE_NUMERIC_ROUND:
          do_get_replace_numeric_round(command);
          break;
        case Q_SAVE_MASTER_POS:
          do_save_master_pos();
          break;
        case Q_SYNC_WITH_MASTER:
          do_sync_with_master(command);
          break;
        case Q_SYNC_SLAVE_WITH_MASTER: {
          do_save_master_pos();
          if (*command->first_argument)
            select_connection(command);
          else
            select_connection_name("slave");
          do_sync_with_master2(command, 0);
          break;
        }
        case Q_COMMENT: {
          command->last_argument = command->end;

          /* Don't output comments in v1 */
          if (opt_result_format_version == 1) break;

          /* Don't output comments if query logging is off */
          if (disable_query_log) break;

          /* Write comment's with two starting #'s to result file */
          const char *p = command->query;
          if (p && *p == '#' && *(p + 1) == '#') {
            dynstr_append_mem(&ds_res, command->query, command->query_len);
            dynstr_append(&ds_res, "\n");
          }
          break;
        }
        case Q_EMPTY_LINE:
          /* Don't output newline in v1 */
          if (opt_result_format_version == 1) break;

          /* Don't output newline if query logging is off */
          if (disable_query_log) break;

          dynstr_append(&ds_res, "\n");
          break;
        case Q_PING:
          handle_command_error(command, mysql_ping(&cur_con->mysql));
          break;
        case Q_RESET_CONNECTION:
          do_reset_connection();
          break;
        case Q_SEND_SHUTDOWN:
          if (opt_offload_count_file) {
            // Save the value of secondary engine execution status
            // before shutting down the server.
            if (secondary_engine->offload_count(&cur_con->mysql, "after"))
              cleanup_and_exit(1);
          }

          handle_command_error(
              command, mysql_query_wrapper(&cur_con->mysql, "shutdown"));
          break;
        case Q_SHUTDOWN_SERVER:
          do_shutdown_server(command);
          break;
        case Q_EXEC:
        case Q_EXECW:
          do_exec(command, false);
          command_executed++;
          break;
        case Q_EXEC_BACKGROUND:
          do_exec(command, true);
          command_executed++;
          break;
        case Q_START_TIMER:
          /* Overwrite possible earlier start of timer */
          timer_start = timer_now();
          break;
        case Q_END_TIMER:
          /* End timer before ending mysqltest */
          timer_output();
          break;
        case Q_CHARACTER_SET:
          do_set_charset(command);
          break;
        case Q_DISABLE_PS_PROTOCOL:
          set_property(command, P_PS, false);
          /* Close any open statements */
          close_statements();
          break;
        case Q_ENABLE_PS_PROTOCOL:
          set_property(command, P_PS, ps_protocol);
          break;
        case Q_DISABLE_RECONNECT:
          set_reconnect(&cur_con->mysql, 0);
          break;
        case Q_ENABLE_RECONNECT:
          set_reconnect(&cur_con->mysql, 1);
          enable_async_client = false;
          /* Close any open statements - no reconnect, need new prepare */
          close_statements();
          break;
        case Q_ENABLE_ASYNC_CLIENT:
          set_property(command, P_ASYNC, true);
          break;
        case Q_DISABLE_ASYNC_CLIENT:
          set_property(command, P_ASYNC, false);
          break;
        case Q_DISABLE_TESTCASE:
          if (testcase_disabled == 0)
            do_disable_testcase(command);
          else
            die("Test case is already disabled.");
          break;
        case Q_ENABLE_TESTCASE:
          // Ensure we don't get testcase_disabled < 0 as this would
          // accidentally disable code we don't want to have disabled.
          if (testcase_disabled == 1)
            testcase_disabled = false;
          else
            die("Test case is already enabled.");
          break;
        case Q_DIE:
          /* Abort test with error code and error message */
          die("%s", command->first_argument);
          break;
        case Q_EXIT:
          /* Stop processing any more commands */
          abort_flag = true;
          break;
        case Q_SKIP: {
          DYNAMIC_STRING ds_skip_msg;
          init_dynamic_string(&ds_skip_msg, nullptr, command->query_len, 256);

          // Evaluate the skip message
          do_eval(&ds_skip_msg, command->first_argument, command->end, false);

          char skip_msg[FN_REFLEN];
          strmake(skip_msg, ds_skip_msg.str, FN_REFLEN - 1);
          dynstr_free(&ds_skip_msg);

          if (!no_skip) {
            // --no-skip option is disabled, skip the test case
            abort_not_supported_test("%s", skip_msg);
          } else {
            const char *path = cur_file->file_name;
            const char *fn = get_filename_from_path(path);

            // Check if the file is in excluded list
            if (excluded_string && strstr(excluded_string, fn)) {
              // File is present in excluded list, skip the test case
              abort_not_supported_test("%s", skip_msg);
            } else {
              // File is not present in excluded list, ignore the skip
              // and continue running the test case
              command->last_argument = command->end;
            }
          }
        } break;
        case Q_OUTPUT: {
          static DYNAMIC_STRING ds_to_file;
          const struct command_arg output_file_args[] = {
              {"to_file", ARG_STRING, true, &ds_to_file, "Output filename"}};
          check_command_args(command, command->first_argument, output_file_args,
                             1, ' ');
          strmake(output_file, ds_to_file.str, FN_REFLEN);
          dynstr_free(&ds_to_file);
          break;
        }
        case Q_QUERY_ATTRS_ADD:
          do_query_attrs_add(command);
          break;
        case Q_QUERY_ATTRS_DELETE:
          do_query_attrs_delete(command);
          break;
        case Q_QUERY_ATTRS_RESET:
          mysql_options(&cur_con->mysql, MYSQL_OPT_QUERY_ATTR_RESET, 0);
          break;
        case Q_CONN_ATTRS_ADD:
          do_conn_attrs_add(command);
          break;
        case Q_CONN_ATTRS_DELETE:
          do_conn_attrs_delete(command);
          break;
        case Q_CONN_ATTRS_RESET:
          mysql_options(&cur_con->mysql, MYSQL_OPT_CONNECT_ATTR_RESET, 0);
          break;

        case Q_ENABLE_CLIENT_INTERACTIVE:
          enable_client_interactive = true;
          /* fallthrough */
        default:
          processed = 0;
          break;
      }
    }

    if (!processed) {
      current_line_inc = 0;
      switch (command->type) {
        case Q_WHILE:
          do_block(cmd_while, command);
          break;
        case Q_IF:
          do_block(cmd_if, command);
          break;
        case Q_END_BLOCK:
          do_done(command);
          break;
        default:
          current_line_inc = 1;
          break;
      }
    } else
      check_eol_junk(command->last_argument);

    if (command->type != Q_ERROR && command->type != Q_COMMENT &&
        command->type != Q_IF && command->type != Q_END_BLOCK) {
      // As soon as any non "error" command or comment has been executed,
      // the array with expected errors should be cleared
      expected_errors->clear_list();
    }

    if (command_executed != last_command_executed || command->used_replace) {
      /*
        As soon as any command has been executed,
        the replace structures should be cleared
      */
      free_all_replace();

      /* Also reset "sorted_result" and "lowercase"*/
      display_result_sorted = false;
      display_result_lower = false;
    }
    last_command_executed = command_executed;

    parser.current_line += current_line_inc;
    if (opt_mark_progress) mark_progress(&progress_file, parser.current_line);

    // Write result from command to log file immediately.
    if (log_file.write(ds_res.str, ds_res.length) || log_file.flush())
      cleanup_and_exit(1);

    if (!result_file_name) {
      std::fwrite(ds_res.str, 1, ds_res.length, stdout);
      std::fflush(stdout);
    }

    dynstr_set(&ds_res, nullptr);
  }

  log_file.close();

  if (opt_mark_progress) progress_file.close();

  start_lineno = 0;
  verbose_msg("... Done processing test commands.");

  if (testcase_disabled) die("Test ended with test case execution disabled.");

  if (disable_warnings || disabled_warnings->count())
    die("The test didn't enable all the disabled warnings, enable "
        "all of them before end of the test.");

  bool empty_result = false;

  /*
    The whole test has been executed _sucessfully_.
    Time to compare result or save it to record file.
    The entire output from test is in the log file
  */
  if (log_file.bytes_written()) {
    if (result_file_name) {
      /* A result file has been specified */

      if (record) {
        /* Recording */

        /* save a copy of the log to result file */
        if (my_copy(log_file.file_name(), result_file_name, MYF(0)) != 0)
          die("Failed to copy '%s' to '%s', errno: %d", log_file.file_name(),
              result_file_name, errno);

      } else {
        /* Check that the output from test is equal to result file */
        check_result();
      }
    }
  } else {
    /* Empty output is an error *unless* we also have an empty result file */
    if (!result_file_name || record ||
        compare_files(log_file.file_name(), result_file_name)) {
      die("The test didn't produce any output");
    } else {
      empty_result = true; /* Meaning empty was expected */
    }
  }

  if (!command_executed && result_file_name && !empty_result)
    die("No queries executed but non-empty result file found!");

  verbose_msg("Test has succeeded!");
  timer_output();
  /* Yes, if we got this far the test has suceeded! Sakila smiles */
  cleanup_and_exit(0);
  return 0; /* Keep compiler happy too */
}

/*
  A primitive timer that give results in milliseconds if the
  --timer-file=<filename> is given. The timer result is written
  to that file when the result is available. To not confuse
  mysql-test-run with an old obsolete result, we remove the file
  before executing any commands. The time we measure is

  - If no explicit 'start_timer' or 'end_timer' is given in the
  test case, the timer measure how long we execute in mysqltest.

  - If only 'start_timer' is given we measure how long we execute
  from that point until we terminate mysqltest.

  - If only 'end_timer' is given we measure how long we execute
  from that we enter mysqltest to the 'end_timer' is command is
  executed.

  - If both 'start_timer' and 'end_timer' are given we measure
  the time between executing the two commands.
*/

void timer_output(void) {
  if (timer_file) {
    char buf[32], *end;
    ulonglong timer = timer_now() - timer_start;
    end = longlong10_to_str(timer, buf, 10);
    str_to_file(timer_file, buf, (int)(end - buf));
    /* Timer has been written to the file, don't use it anymore */
    timer_file = nullptr;
  }
}

ulonglong timer_now(void) { return my_micro_time() / 1000; }

/*
  Get arguments for replace_columns. The syntax is:
  replace-column column_number to_string [column_number to_string ...]
  Where each argument may be quoted with ' or "
  A argument may also be a variable, in which case the value of the
  variable is replaced.
*/

void do_get_replace_column(struct st_command *command) {
  const char *from = command->first_argument;
  char *buff, *start;
  DBUG_TRACE;

  free_replace_column();
  if (!*from) die("Missing argument in %s", command->query);

  /* Allocate a buffer for results */
  start = buff = (char *)my_malloc(PSI_NOT_INSTRUMENTED, std::strlen(from) + 1,
                                   MYF(MY_WME | MY_FAE));
  while (*from) {
    char *to;
    uint column_number;
    to = get_string(&buff, &from, command);
    if (!(column_number = atoi(to)) || column_number > MAX_COLUMNS)
      die("Wrong column number to replace_column in '%s'", command->query);
    if (!*from)
      die("Wrong number of arguments to replace_column in '%s'",
          command->query);
    to = get_string(&buff, &from, command);
    my_free(replace_column[column_number - 1]);
    replace_column[column_number - 1] =
        my_strdup(PSI_NOT_INSTRUMENTED, to, MYF(MY_WME | MY_FAE));
    max_replace_column = std::max(max_replace_column, column_number);
  }
  my_free(start);
  command->last_argument = command->end;
}

void free_replace_column() {
  uint i;
  for (i = 0; i < max_replace_column; i++) {
    if (replace_column[i]) {
      my_free(replace_column[i]);
      replace_column[i] = nullptr;
    }
  }
  max_replace_column = 0;
}

/*
  Functions to round numeric results.

SYNOPSIS
  do_get_replace_numeric_round()
  command - command handle

DESCRIPTION
  replace_numeric_round <precision>

  where precision is the number of digits after the decimal point
  that the result will be rounded off to. The precision can only
  be a number between 0 and 16.
  eg. replace_numeric_round 10;
  Numbers which are > 1e10 or < -1e10 are represented using the
  exponential notation after they are rounded off.
  Trailing zeroes after the decimal point are removed from the
  numbers.
  If the precision is 0, then the value is rounded off to the
  nearest whole number.
*/
void do_get_replace_numeric_round(struct st_command *command) {
  DYNAMIC_STRING ds_round;
  const struct command_arg numeric_arg = {
      "precision", ARG_STRING, true, &ds_round, "Number of decimal precision"};
  DBUG_TRACE;

  check_command_args(command, command->first_argument, &numeric_arg,
                     sizeof(numeric_arg) / sizeof(struct command_arg), ' ');

  // Parse the argument string to get the precision
  long int v = 0;
  if (str2int(ds_round.str, 10, 0, REPLACE_ROUND_MAX, &v) == NullS)
    die("A number between 0 and %d is required for the precision "
        "in replace_numeric_round",
        REPLACE_ROUND_MAX);

  glob_replace_numeric_round = (int)v;
  dynstr_free(&ds_round);
}

void free_replace_numeric_round() { glob_replace_numeric_round = -1; }

/*
  Round the digits after the decimal point to the specified precision
  by iterating through the result set element, identifying the part to
  be rounded off, and rounding that part off.
*/
void replace_numeric_round_append(int round, DYNAMIC_STRING *result,
                                  const char *from, size_t len) {
  while (len > 0) {
    // Move pointer to the start of the numeric values
    size_t size = strcspn(from, "0123456789");
    if (size > 0) {
      dynstr_append_mem(result, from, size);
      from += size;
      len -= size;
    }

    /*
      Move the pointer to the end of the numeric values and the
      the start of the non-numeric values such as "." and "e"
    */
    size = strspn(from, "0123456789");
    int r = round;

    /*
      If result from one of the rows of the result set is null,
      break the loop
    */
    if (*(from + size) == 0) {
      dynstr_append_mem(result, from, size);
      break;
    }

    switch (*(from + size)) {
      // double/float
      case '.':
        size_t size1;
        size1 = strspn(from + size + 1, "0123456789");

        /*
          Restrict rounding to less than the
          the existing precision to avoid 1.2 being replaced
          to 1.2000000
        */
        if (size1 < (size_t)r) r = size1;
      // fallthrough: all cases till next break are executed
      case 'e':
      case 'E':
        if (isdigit(*(from + size + 1))) {
          char *end;
          double val = strtod(from, &end);
          if (end != nullptr) {
            const char *format = (val < 1e10 && val > -1e10) ? "%.*f" : "%.*e";
            char buf[40];

            size = snprintf(buf, sizeof(buf), format, r, val);
            if (val < 1e10 && val > -1e10 && r > 0) {
              /*
                2.0000000 need to be represented as 2 for consistency
                2.0010000 also becomes 2.001
              */
              while (buf[size - 1] == '0') size--;

              // don't leave 100. trailing
              if (buf[size - 1] == '.') size--;
            }
            dynstr_append_mem(result, buf, size);
            len -= (end - from);
            from = end;
            break;
          }
        }

        /*
          This is because strtod didn't convert or there wasn't digits after
          [.eE] so output without changing
        */
        dynstr_append_mem(result, from, size);
        from += size;
        len -= size;
        break;
      // int
      default:
        dynstr_append_mem(result, from, size);
        from += size;
        len -= size;
        break;
    }
  }
}

/****************************************************************************/
/*
  Replace functions
*/

/* Definitions for replace result */

struct POINTER_ARRAY {  /* when using array-strings */
  TYPELIB typelib;      /* Pointer to strings */
  uchar *str{nullptr};  /* Strings is here */
  uint8 *flag{nullptr}; /* Flag about each var. */
  uint array_allocs{0}, max_count{0}, length{0}, max_length{0};
};

REPLACE *init_replace(const char **from, const char **to, uint count,
                      const char *word_end_chars);
int insert_pointer_name(POINTER_ARRAY *pa, char *name);
void free_pointer_array(POINTER_ARRAY *pa);

/*
  Get arguments for replace. The syntax is:
  replace from to [from to ...]
  Where each argument may be quoted with ' or "
  A argument may also be a variable, in which case the value of the
  variable is replaced.
*/

void do_get_replace(struct st_command *command) {
  uint i;
  const char *from = command->first_argument;
  char *buff, *start;
  char word_end_chars[256], *pos;
  POINTER_ARRAY to_array, from_array;
  DBUG_TRACE;

  free_replace();

  if (!*from) die("Missing argument in %s", command->query);
  start = buff = (char *)my_malloc(PSI_NOT_INSTRUMENTED, std::strlen(from) + 1,
                                   MYF(MY_WME | MY_FAE));
  while (*from) {
    char *to = buff;
    to = get_string(&buff, &from, command);
    if (!*from)
      die("Wrong number of arguments to replace_result in '%s'",
          command->query);
#ifdef _WIN32
    fix_win_paths(to, from - to);
#endif
    insert_pointer_name(&from_array, to);
    to = get_string(&buff, &from, command);
    insert_pointer_name(&to_array, to);
  }
  for (i = 1, pos = word_end_chars; i < 256; i++)
    if (my_isspace(charset_info, i)) *pos++ = i;
  *pos = 0; /* End pointer */
  if (!(glob_replace = init_replace(
            from_array.typelib.type_names, to_array.typelib.type_names,
            (uint)from_array.typelib.count, word_end_chars)))
    die("Can't initialize replace from '%s'", command->query);
  free_pointer_array(&from_array);
  free_pointer_array(&to_array);
  my_free(start);
  command->last_argument = command->end;
}

void free_replace() {
  DBUG_TRACE;
  my_free(glob_replace);
  glob_replace = nullptr;
}

struct REPLACE {
  int found;
  REPLACE *next[256];
};

struct REPLACE_STRING {
  int found;
  const char *replace_string;
  uint to_offset;
  int from_offset;
};

void replace_strings_append(REPLACE *rep, DYNAMIC_STRING *ds, const char *str,
                            size_t len MY_ATTRIBUTE((unused))) {
  REPLACE *rep_pos;
  REPLACE_STRING *rep_str;
  const char *start, *from;
  DBUG_TRACE;

  start = from = str;
  rep_pos = rep + 1;
  for (;;) {
    /* Loop through states */
    DBUG_PRINT("info", ("Looping through states"));
    while (!rep_pos->found) rep_pos = rep_pos->next[(uchar)*from++];

    /* Does this state contain a string to be replaced */
    if (!(rep_str = ((REPLACE_STRING *)rep_pos))->replace_string) {
      /* No match found */
      dynstr_append_mem(ds, start, from - start - 1);
      DBUG_PRINT("exit",
                 ("Found no more string to replace, appended: %s", start));
      return;
    }

    /* Found a string that needs to be replaced */
    DBUG_PRINT("info", ("found: %d, to_offset: %d, from_offset: %d, string: %s",
                        rep_str->found, rep_str->to_offset,
                        rep_str->from_offset, rep_str->replace_string));

    /* Append part of original string before replace string */
    dynstr_append_mem(ds, start, (from - rep_str->to_offset) - start);

    /* Append replace string */
    dynstr_append_mem(ds, rep_str->replace_string,
                      std::strlen(rep_str->replace_string));

    if (!*(from -= rep_str->from_offset) && rep_pos->found != 2) {
      /* End of from string */
      DBUG_PRINT("exit", ("Found end of from string"));
      return;
    }
    DBUG_ASSERT(from <= str + len);
    start = from;
    rep_pos = rep;
  }
}

/*
  Regex replace  functions
*/

/*
  Finds the next (non-escaped) '/' in the expression.
  (If the character '/' is needed, it can be escaped using '\'.)
*/

#define PARSE_REGEX_ARG     \
  while (p < expr_end) {    \
    char c = *p;            \
    if (c == '/') {         \
      if (last_c == '\\') { \
        buf_p[-1] = '/';    \
      } else {              \
        *buf_p++ = 0;       \
        break;              \
      }                     \
    } else                  \
      *buf_p++ = c;         \
                            \
    last_c = c;             \
    p++;                    \
  }

/**
  Initializes the regular substitution expression to be used in the
  result output of test.

  @param  expr  Pointer to string having regular expression to be used
                for substitution.
  @retval st_replace_regex structure with pairs of substitutions.
*/
static struct st_replace_regex *init_replace_regex(const char *expr) {
  struct st_replace_regex *res;
  char *buf;
  const char *expr_end;
  const char *p;
  char *buf_p;
  size_t expr_len = std::strlen(expr);
  char last_c = 0;
  struct st_regex reg;

  /* my_malloc() will die on fail with MY_FAE */
  void *rawmem = my_malloc(PSI_NOT_INSTRUMENTED, sizeof(*res) + expr_len,
                           MYF(MY_FAE + MY_WME));
  res = new (rawmem) st_replace_regex;

  buf = (char *)res + sizeof(*res);
  expr_end = expr + expr_len;
  p = expr;
  buf_p = buf;

  /* for each regexp substitution statement */
  while (p < expr_end) {
    memset(&reg, 0, sizeof(reg));
    /* find the start of the statement */
    while (p < expr_end) {
      if (*p == '/') break;
      p++;
    }

    if (p == expr_end || ++p == expr_end) {
      if (res->regex_arr.size())
        break;
      else
        goto err;
    }
    /* we found the start */
    reg.pattern = buf_p;

    /* Find first argument -- pattern string to be removed */
    PARSE_REGEX_ARG

    if (p == expr_end || ++p == expr_end) goto err;

    /* buf_p now points to the replacement pattern terminated with \0 */
    reg.replace = buf_p;

    /* Find second argument -- replace string to replace pattern */
    PARSE_REGEX_ARG

    if (p == expr_end) goto err;

    /* skip the ending '/' in the statement */
    p++;

    /* Check if we should do matching case insensitive */
    if (p < expr_end && *p == 'i') reg.icase = 1;

    /* done parsing the statement, now place it in regex_arr */
    if (res->regex_arr.push_back(reg)) die("Out of memory");
  }
  res->odd_buf_len = res->even_buf_len = 8192;
  res->even_buf = (char *)my_malloc(PSI_NOT_INSTRUMENTED, res->even_buf_len,
                                    MYF(MY_WME + MY_FAE));
  res->odd_buf = (char *)my_malloc(PSI_NOT_INSTRUMENTED, res->odd_buf_len,
                                   MYF(MY_WME + MY_FAE));
  res->buf = res->even_buf;

  return res;

err:
  my_free(res);
  die("Error parsing replace_regex \"%s\"", expr);
  return nullptr;
}

/*
  Parse the regular expression to be used in all result files
  from now on.

  The syntax is --replace_regex /from/to/i /from/to/i ...
  i means case-insensitive match. If omitted, the match is
  case-sensitive

*/
void do_get_replace_regex(struct st_command *command) {
  const char *expr = command->first_argument;
  free_replace_regex();
  /* Allow variable for the *entire* list of replacements */
  if (*expr == '$') {
    VAR *val = var_get(expr, nullptr, false, true);
    expr = val ? val->str_val : nullptr;
  }
  if (expr && *expr && !(glob_replace_regex = init_replace_regex(expr)))
    die("Could not init replace_regex");
  command->last_argument = command->end;
}

void free_replace_regex() {
  if (glob_replace_regex) {
    my_free(glob_replace_regex->even_buf);
    my_free(glob_replace_regex->odd_buf);
    glob_replace_regex->~st_replace_regex();
    my_free(glob_replace_regex);
    glob_replace_regex = nullptr;
  }
}

#ifndef WORD_BIT
#define WORD_BIT (8 * sizeof(uint))
#endif

#define SET_MALLOC_HUNC 64
#define LAST_CHAR_CODE 259

struct REP_SET {
  uint *bits;                 /* Pointer to used sets */
  short next[LAST_CHAR_CODE]; /* Pointer to next sets */
  uint found_len;             /* Best match to date */
  int found_offset;
  uint table_offset;
  uint size_of_bits; /* For convinience */
};

struct REP_SETS {
  uint count;     /* Number of sets */
  uint extra;     /* Extra sets in buffer */
  uint invisible; /* Sets not chown */
  uint size_of_bits;
  REP_SET *set, *set_buffer;
  uint *bit_buffer;
};

struct FOUND_SET {
  uint table_offset;
  int found_offset;
};

struct FOLLOWS {
  int chr;
  uint table_offset;
  uint len;
};

int init_sets(REP_SETS *sets, uint states);
REP_SET *make_new_set(REP_SETS *sets);
void make_sets_invisible(REP_SETS *sets);
void free_last_set(REP_SETS *sets);
void free_sets(REP_SETS *sets);
void internal_set_bit(REP_SET *set, uint bit);
void internal_clear_bit(REP_SET *set, uint bit);
void or_bits(REP_SET *to, REP_SET *from);
void copy_bits(REP_SET *to, REP_SET *from);
int cmp_bits(REP_SET *set1, REP_SET *set2);
int get_next_bit(REP_SET *set, uint lastpos);
int find_set(REP_SETS *sets, REP_SET *find);
int find_found(FOUND_SET *found_set, uint table_offset, int found_offset);
uint start_at_word(const char *pos);
uint end_of_word(const char *pos);

static uint found_sets = 0;

static uint replace_len(const char *str) {
  uint len = 0;
  while (*str) {
    str++;
    len++;
  }
  return len;
}

/* Init a replace structure for further calls */

REPLACE *init_replace(const char **from, const char **to, uint count,
                      const char *word_end_chars) {
  static const int SPACE_CHAR = 256;
  static const int END_OF_LINE = 258;

  uint i, j, states, set_nr, len, result_len, max_length, found_end, bits_set,
      bit_nr;
  int used_sets, chr, default_state;
  char used_chars[LAST_CHAR_CODE], is_word_end[256];
  const char *pos, **to_array;
  char *to_pos;
  REP_SETS sets;
  REP_SET *set, *start_states, *word_states, *new_set;
  FOLLOWS *follow, *follow_ptr;
  REPLACE *replace;
  FOUND_SET *found_set;
  REPLACE_STRING *rep_str;
  DBUG_TRACE;

  /* Count number of states */
  for (i = result_len = max_length = 0, states = 2; i < count; i++) {
    len = replace_len(from[i]);
    if (!len) {
      errno = EINVAL;
      return nullptr;
    }
    states += len + 1;
    result_len += (uint)std::strlen(to[i]) + 1;
    if (len > max_length) max_length = len;
  }
  memset(is_word_end, 0, sizeof(is_word_end));
  for (i = 0; word_end_chars[i]; i++) is_word_end[(uchar)word_end_chars[i]] = 1;

  if (init_sets(&sets, states)) return nullptr;
  found_sets = 0;
  if (!(found_set = (FOUND_SET *)my_malloc(
            PSI_NOT_INSTRUMENTED, sizeof(FOUND_SET) * max_length * count,
            MYF(MY_WME)))) {
    free_sets(&sets);
    return nullptr;
  }
  (void)make_new_set(&sets);  /* Set starting set */
  make_sets_invisible(&sets); /* Hide previus sets */
  used_sets = -1;
  word_states = make_new_set(&sets);  /* Start of new word */
  start_states = make_new_set(&sets); /* This is first state */
  if (!(follow = (FOLLOWS *)my_malloc(PSI_NOT_INSTRUMENTED,
                                      (states + 2) * sizeof(FOLLOWS),
                                      MYF(MY_WME)))) {
    free_sets(&sets);
    my_free(found_set);
    return nullptr;
  }

  /* Init follow_ptr[] */
  for (i = 0, states = 1, follow_ptr = follow + 1; i < count; i++) {
    if (from[i][0] == '\\' && from[i][1] == '^') {
      internal_set_bit(start_states, states + 1);
      if (!from[i][2]) {
        start_states->table_offset = i;
        start_states->found_offset = 1;
      }
    } else if (from[i][0] == '\\' && from[i][1] == '$') {
      internal_set_bit(start_states, states);
      internal_set_bit(word_states, states);
      if (!from[i][2] && start_states->table_offset == (uint)~0) {
        start_states->table_offset = i;
        start_states->found_offset = 0;
      }
    } else {
      internal_set_bit(word_states, states);
      if (from[i][0] == '\\' && (from[i][1] == 'b' && from[i][2]))
        internal_set_bit(start_states, states + 1);
      else
        internal_set_bit(start_states, states);
    }
    for (pos = from[i], len = 0; *pos; pos++) {
      follow_ptr->chr = (uchar)*pos;
      follow_ptr->table_offset = i;
      follow_ptr->len = ++len;
      follow_ptr++;
    }
    follow_ptr->chr = 0;
    follow_ptr->table_offset = i;
    follow_ptr->len = len;
    follow_ptr++;
    states += (uint)len + 1;
  }

  for (set_nr = 0, pos = nullptr; set_nr < sets.count; set_nr++) {
    set = sets.set + set_nr;
    default_state = 0; /* Start from beginning */

    /* If end of found-string not found or start-set with current set */

    for (i = (uint)~0; (i = get_next_bit(set, i));) {
      if (!follow[i].chr) {
        if (!default_state)
          default_state =
              find_found(found_set, set->table_offset, set->found_offset + 1);
      }
    }
    copy_bits(sets.set + used_sets, set); /* Save set for changes */
    if (!default_state)
      or_bits(sets.set + used_sets, sets.set); /* Can restart from start */

    /* Find all chars that follows current sets */
    memset(used_chars, 0, sizeof(used_chars));
    for (i = (uint)~0; (i = get_next_bit(sets.set + used_sets, i));) {
      used_chars[follow[i].chr] = 1;
      if ((follow[i].chr == SPACE_CHAR && !follow[i + 1].chr &&
           follow[i].len > 1) ||
          follow[i].chr == END_OF_LINE)
        used_chars[0] = 1;
    }

    /* Mark word_chars used if \b is in state */
    if (used_chars[SPACE_CHAR])
      for (pos = word_end_chars; *pos; pos++) used_chars[(int)(uchar)*pos] = 1;

    /* Handle other used characters */
    for (chr = 0; chr < 256; chr++) {
      if (!used_chars[chr])
        set->next[chr] = chr ? default_state : -1;
      else {
        new_set = make_new_set(&sets);
        assert(new_set);
        set = sets.set + set_nr; /* if realloc */
        new_set->table_offset = set->table_offset;
        new_set->found_len = set->found_len;
        new_set->found_offset = set->found_offset + 1;
        found_end = 0;

        for (i = (uint)~0; (i = get_next_bit(sets.set + used_sets, i));) {
          if (!follow[i].chr || follow[i].chr == chr ||
              (follow[i].chr == SPACE_CHAR &&
               (is_word_end[chr] ||
                (!chr && follow[i].len > 1 && !follow[i + 1].chr))) ||
              (follow[i].chr == END_OF_LINE && !chr)) {
            if ((!chr || (follow[i].chr && !follow[i + 1].chr)) &&
                follow[i].len > found_end)
              found_end = follow[i].len;
            if (chr && follow[i].chr)
              internal_set_bit(new_set, i + 1); /* To next set */
            else
              internal_set_bit(new_set, i);
          }
        }
        if (found_end) {
          new_set->found_len = 0; /* Set for testing if first */
          bits_set = 0;
          for (i = (uint)~0; (i = get_next_bit(new_set, i));) {
            if ((follow[i].chr == SPACE_CHAR || follow[i].chr == END_OF_LINE) &&
                !chr)
              bit_nr = i + 1;
            else
              bit_nr = i;
            if (follow[bit_nr - 1].len < found_end ||
                (new_set->found_len && (chr == 0 || !follow[bit_nr].chr)))
              internal_clear_bit(new_set, i);
            else {
              if (chr == 0 || !follow[bit_nr].chr) { /* best match  */
                new_set->table_offset = follow[bit_nr].table_offset;
                if (chr || (follow[i].chr == SPACE_CHAR ||
                            follow[i].chr == END_OF_LINE))
                  new_set->found_offset = found_end; /* New match */
                new_set->found_len = found_end;
              }
              bits_set++;
            }
          }
          if (bits_set == 1) {
            set->next[chr] = find_found(found_set, new_set->table_offset,
                                        new_set->found_offset);
            free_last_set(&sets);
          } else
            set->next[chr] = find_set(&sets, new_set);
        } else
          set->next[chr] = find_set(&sets, new_set);
      }
    }
  }

  /* Alloc replace structure for the replace-state-machine */

  if ((replace =
           (REPLACE *)my_malloc(PSI_NOT_INSTRUMENTED,
                                sizeof(REPLACE) * (sets.count) +
                                    sizeof(REPLACE_STRING) * (found_sets + 1) +
                                    sizeof(char *) * count + result_len,
                                MYF(MY_WME | MY_ZEROFILL)))) {
    rep_str = (REPLACE_STRING *)(replace + sets.count);
    to_array = pointer_cast<const char **>(rep_str + found_sets + 1);
    to_pos = (char *)(to_array + count);
    for (i = 0; i < count; i++) {
      to_array[i] = to_pos;
      to_pos = my_stpcpy(to_pos, to[i]) + 1;
    }
    rep_str[0].found = 1;
    rep_str[0].replace_string = nullptr;
    for (i = 1; i <= found_sets; i++) {
      pos = from[found_set[i - 1].table_offset];
      rep_str[i].found = !memcmp(pos, "\\^", 3) ? 2 : 1;
      rep_str[i].replace_string = to_array[found_set[i - 1].table_offset];
      rep_str[i].to_offset = found_set[i - 1].found_offset - start_at_word(pos);
      rep_str[i].from_offset =
          found_set[i - 1].found_offset - replace_len(pos) + end_of_word(pos);
    }
    for (i = 0; i < sets.count; i++) {
      for (j = 0; j < 256; j++)
        if (sets.set[i].next[j] >= 0)
          replace[i].next[j] = replace + sets.set[i].next[j];
        else
          replace[i].next[j] =
              (REPLACE *)(rep_str + (-sets.set[i].next[j] - 1));
    }
  }
  my_free(follow);
  free_sets(&sets);
  my_free(found_set);
  DBUG_PRINT("exit", ("Replace table has %d states", sets.count));
  return replace;
}

int init_sets(REP_SETS *sets, uint states) {
  memset(sets, 0, sizeof(*sets));
  sets->size_of_bits = ((states + 7) / 8);
  if (!(sets->set_buffer = (REP_SET *)my_malloc(
            PSI_NOT_INSTRUMENTED, sizeof(REP_SET) * SET_MALLOC_HUNC,
            MYF(MY_WME))))
    return 1;
  if (!(sets->bit_buffer = (uint *)my_malloc(
            PSI_NOT_INSTRUMENTED,
            sizeof(uint) * sets->size_of_bits * SET_MALLOC_HUNC,
            MYF(MY_WME)))) {
    my_free(sets->set);
    return 1;
  }
  return 0;
}

/* Make help sets invisible for nicer codeing */

void make_sets_invisible(REP_SETS *sets) {
  sets->invisible = sets->count;
  sets->set += sets->count;
  sets->count = 0;
}

REP_SET *make_new_set(REP_SETS *sets) {
  uint i, count, *bit_buffer;
  REP_SET *set;
  if (sets->extra) {
    sets->extra--;
    set = sets->set + sets->count++;
    memset(set->bits, 0, sizeof(uint) * sets->size_of_bits);
    memset(&set->next[0], 0, sizeof(set->next[0]) * LAST_CHAR_CODE);
    set->found_offset = 0;
    set->found_len = 0;
    set->table_offset = (uint)~0;
    set->size_of_bits = sets->size_of_bits;
    return set;
  }
  count = sets->count + sets->invisible + SET_MALLOC_HUNC;
  if (!(set = (REP_SET *)my_realloc(PSI_NOT_INSTRUMENTED,
                                    (uchar *)sets->set_buffer,
                                    sizeof(REP_SET) * count, MYF(MY_WME))))
    return nullptr;
  sets->set_buffer = set;
  sets->set = set + sets->invisible;
  if (!(bit_buffer = (uint *)my_realloc(
            PSI_NOT_INSTRUMENTED, (uchar *)sets->bit_buffer,
            (sizeof(uint) * sets->size_of_bits) * count, MYF(MY_WME))))
    return nullptr;
  sets->bit_buffer = bit_buffer;
  for (i = 0; i < count; i++) {
    sets->set_buffer[i].bits = bit_buffer;
    bit_buffer += sets->size_of_bits;
  }
  sets->extra = SET_MALLOC_HUNC;
  return make_new_set(sets);
}

void free_last_set(REP_SETS *sets) {
  sets->count--;
  sets->extra++;
  return;
}

void free_sets(REP_SETS *sets) {
  my_free(sets->set_buffer);
  my_free(sets->bit_buffer);
  return;
}

void internal_set_bit(REP_SET *set, uint bit) {
  set->bits[bit / WORD_BIT] |= 1 << (bit % WORD_BIT);
  return;
}

void internal_clear_bit(REP_SET *set, uint bit) {
  set->bits[bit / WORD_BIT] &= ~(1 << (bit % WORD_BIT));
  return;
}

void or_bits(REP_SET *to, REP_SET *from) {
  uint i;
  for (i = 0; i < to->size_of_bits; i++) to->bits[i] |= from->bits[i];
  return;
}

void copy_bits(REP_SET *to, REP_SET *from) {
  memcpy((uchar *)to->bits, (uchar *)from->bits,
         (size_t)(sizeof(uint) * to->size_of_bits));
}

int cmp_bits(REP_SET *set1, REP_SET *set2) {
  return memcmp(set1->bits, set2->bits, sizeof(uint) * set1->size_of_bits);
}

/* Get next set bit from set. */

int get_next_bit(REP_SET *set, uint lastpos) {
  uint pos, *start, *end, bits;

  start = set->bits + ((lastpos + 1) / WORD_BIT);
  end = set->bits + set->size_of_bits;
  bits = start[0] & ~((1 << ((lastpos + 1) % WORD_BIT)) - 1U);

  while (!bits && ++start < end) bits = start[0];
  if (!bits) return 0;
  pos = (uint)(start - set->bits) * WORD_BIT;
  while (!(bits & 1)) {
    bits >>= 1;
    pos++;
  }
  return pos;
}

/* find if there is a same set in sets. If there is, use it and
   free given set, else put in given set in sets and return its
   position */

int find_set(REP_SETS *sets, REP_SET *find) {
  uint i;
  for (i = 0; i < sets->count - 1; i++) {
    if (!cmp_bits(sets->set + i, find)) {
      free_last_set(sets);
      return i;
    }
  }
  return i; /* return new position */
}

/* find if there is a found_set with same table_offset & found_offset
   If there is return offset to it, else add new offset and return pos.
   Pos returned is -offset-2 in found_set_structure because it is
   saved in set->next and set->next[] >= 0 points to next set and
   set->next[] == -1 is reserved for end without replaces.
*/

int find_found(FOUND_SET *found_set, uint table_offset, int found_offset) {
  int i;
  for (i = 0; (uint)i < found_sets; i++)
    if (found_set[i].table_offset == table_offset &&
        found_set[i].found_offset == found_offset)
      return -i - 2;
  found_set[i].table_offset = table_offset;
  found_set[i].found_offset = found_offset;
  found_sets++;
  return -i - 2; /* return new position */
}

/* Return 1 if regexp starts with \b or ends with \b*/

uint start_at_word(const char *pos) {
  return (((!memcmp(pos, "\\b", 2) && pos[2]) || !memcmp(pos, "\\^", 2)) ? 1
                                                                         : 0);
}

uint end_of_word(const char *pos) {
  const char *end = strend(pos);
  return ((end > pos + 2 && !memcmp(end - 2, "\\b", 2)) ||
          (end >= pos + 2 && !memcmp(end - 2, "\\$", 2)))
             ? 1
             : 0;
}

/****************************************************************************
 * Handle replacement of strings
 ****************************************************************************/

#define PC_MALLOC 256 /* Bytes for pointers */
#define PS_MALLOC 512 /* Bytes for data */

int insert_pointer_name(POINTER_ARRAY *pa, char *name) {
  uint i, length, old_count;
  uchar *new_pos;
  const char **new_array;
  DBUG_TRACE;

  if (!pa->typelib.count) {
    if (!(pa->typelib.type_names =
              (const char **)my_malloc(PSI_NOT_INSTRUMENTED,
                                       ((PC_MALLOC - MALLOC_OVERHEAD) /
                                        (sizeof(char *) + sizeof(*pa->flag)) *
                                        (sizeof(char *) + sizeof(*pa->flag))),
                                       MYF(MY_WME))))
      return -1;
    if (!(pa->str = (uchar *)my_malloc(PSI_NOT_INSTRUMENTED,
                                       (uint)(PS_MALLOC - MALLOC_OVERHEAD),
                                       MYF(MY_WME)))) {
      my_free(pa->typelib.type_names);
      return -1;
    }
    pa->max_count =
        (PC_MALLOC - MALLOC_OVERHEAD) / (sizeof(uchar *) + sizeof(*pa->flag));
    pa->flag = (uint8 *)(pa->typelib.type_names + pa->max_count);
    pa->length = 0;
    pa->max_length = PS_MALLOC - MALLOC_OVERHEAD;
    pa->array_allocs = 1;
  }
  length = (uint)std::strlen(name) + 1;
  if (pa->length + length >= pa->max_length) {
    if (!(new_pos = (uchar *)my_realloc(PSI_NOT_INSTRUMENTED, (uchar *)pa->str,
                                        (uint)(pa->length + length + PS_MALLOC),
                                        MYF(MY_WME))))
      return 1;
    if (new_pos != pa->str) {
      ptrdiff_t diff = new_pos - pa->str;
      for (i = 0; i < pa->typelib.count; i++)
        pa->typelib.type_names[i] = pa->typelib.type_names[i] + diff;
      pa->str = new_pos;
    }
    pa->max_length = pa->length + length + PS_MALLOC;
  }
  if (pa->typelib.count >= pa->max_count - 1) {
    int len;
    pa->array_allocs++;
    len = (PC_MALLOC * pa->array_allocs - MALLOC_OVERHEAD);
    if (!(new_array = (const char **)my_realloc(
              PSI_NOT_INSTRUMENTED, (uchar *)pa->typelib.type_names,
              (uint)len / (sizeof(uchar *) + sizeof(*pa->flag)) *
                  (sizeof(uchar *) + sizeof(*pa->flag)),
              MYF(MY_WME))))
      return 1;
    pa->typelib.type_names = new_array;
    old_count = pa->max_count;
    pa->max_count = len / (sizeof(uchar *) + sizeof(*pa->flag));
    pa->flag = (uint8 *)(pa->typelib.type_names + pa->max_count);
    memcpy((uchar *)pa->flag, (char *)(pa->typelib.type_names + old_count),
           old_count * sizeof(*pa->flag));
  }
  pa->flag[pa->typelib.count] = 0; /* Reset flag */
  pa->typelib.type_names[pa->typelib.count++] = (char *)pa->str + pa->length;
  pa->typelib.type_names[pa->typelib.count] = NullS; /* Put end-mark */
  (void)my_stpcpy((char *)pa->str + pa->length, name);
  pa->length += length;
  return 0;
} /* insert_pointer_name */

/* free pointer array */

void free_pointer_array(POINTER_ARRAY *pa) {
  if (pa->typelib.count) {
    pa->typelib.count = 0;
    my_free(pa->typelib.type_names);
    pa->typelib.type_names = nullptr;
    my_free(pa->str);
  }
} /* free_pointer_array */

/* Functions that uses replace and replace_regex */

/* Append the string to ds, with optional replace */
void replace_dynstr_append_mem(DYNAMIC_STRING *ds, const char *val,
                               size_t len) {
#ifdef _WIN32
  fix_win_paths(val, len);
#endif

  if (display_result_lower) {
    /* Convert to lower case, and do this first */
    my_casedn_str(charset_info, const_cast<char *>(val));
  }

  if (glob_replace_regex) {
    size_t orig_len = len;
    // Regex replace
    if (!multi_reg_replace(glob_replace_regex, const_cast<char *>(val), &len)) {
      val = glob_replace_regex->buf;
    } else {
      len = orig_len;
    }
  }

  DYNAMIC_STRING ds_temp;
  init_dynamic_string(&ds_temp, "", 512, 512);

  /* Store result from replace_result in ds_temp */
  if (glob_replace) {
    /* Normal replace */
    replace_strings_append(glob_replace, &ds_temp, val, len);
  }

  /*
    Call the replace_numeric_round function with the specified
    precision. It may be used along with replace_result, so use the
    output from replace_result as the input for replace_numeric_round.
  */
  if (glob_replace_numeric_round >= 0) {
    /* Copy the result from replace_result if it was used, into buffer */
    if (ds_temp.length > 0) {
      char buffer[512];
      strcpy(buffer, ds_temp.str);
      dynstr_free(&ds_temp);
      init_dynamic_string(&ds_temp, "", 512, 512);
      replace_numeric_round_append(glob_replace_numeric_round, &ds_temp, buffer,
                                   std::strlen(buffer));
    } else
      replace_numeric_round_append(glob_replace_numeric_round, &ds_temp, val,
                                   len);
  }

  if (!glob_replace && glob_replace_numeric_round < 0)
    dynstr_append_mem(ds, val, len);
  else
    dynstr_append_mem(ds, ds_temp.str, std::strlen(ds_temp.str));
  dynstr_free(&ds_temp);
}

/* Append zero-terminated string to ds, with optional replace */
void replace_dynstr_append(DYNAMIC_STRING *ds, const char *val) {
  replace_dynstr_append_mem(ds, val, std::strlen(val));
}

/* Append uint to ds, with optional replace */
void replace_dynstr_append_uint(DYNAMIC_STRING *ds, uint val) {
  char buff[22]; /* This should be enough for any int */
  char *end = longlong10_to_str(val, buff, 10);
  replace_dynstr_append_mem(ds, buff, end - buff);
}

/*
  Build a list of pointer to each line in ds_input, sort
  the list and use the sorted list to append the strings
  sorted to the output ds

  SYNOPSIS
  dynstr_append_sorted
  ds - string where the sorted output will be appended
  ds_input - string to be sorted
  start_sort_column - column to start sorting from (0 for sorting
    the entire line); a stable sort will be used
*/

class Comp_lines {
 public:
  bool operator()(const char *a, const char *b) {
    return std::strcmp(a, b) < 0;
  }
};

static size_t length_of_n_first_columns(std::string str,
                                        int start_sort_column) {
  std::stringstream columns(str);
  std::string temp;
  size_t size_of_columns = 0;

  int i = 0;
  while (getline(columns, temp, '\t') && i < start_sort_column) {
    size_of_columns = size_of_columns + temp.length();
    i++;
  }

  return size_of_columns;
}

void dynstr_append_sorted(DYNAMIC_STRING *ds, DYNAMIC_STRING *ds_input,
                          int start_sort_column) {
  char *start = ds_input->str;
  char *end = ds_input->str + ds_input->length;
  std::vector<std::string> sorted;
  DBUG_TRACE;

  if (!*start) return; /* No input */

  /* First line is result header, skip past it */
  while (*start && *start != '\n') start++;
  start++; /* Skip past \n */
  dynstr_append_mem(ds, ds_input->str, start - ds_input->str);

  /*
    Traverse through the result set from start to end to avoid
    ignoring null characters (0x00), and insert line by line
    into array.
  */
  size_t first_unsorted_row = 0;
  while (start < end) {
    char *line_end = (char *)start;

    /* Find end of line */
    while (*line_end != '\n') line_end++;
    *line_end = 0;

    std::string result_row = std::string(start, line_end - start);
    if (!sorted.empty() && start_sort_column > 0) {
      /*
        If doing partial sorting, and the prefix is different from that of the
        previous line, the group is done. Sort it and start another one.
       */
      size_t prev_line_prefix_len =
          length_of_n_first_columns(sorted.back(), start_sort_column);
      if (sorted.back().compare(0, prev_line_prefix_len, result_row, 0,
                                prev_line_prefix_len) != 0) {
        std::sort(sorted.begin() + first_unsorted_row, sorted.end());
        first_unsorted_row = sorted.size();
      }
    }

    /* Insert line into the array */
    sorted.push_back(result_row);
    start = line_end + 1;
  }

  /* Sort array */
  std::stable_sort(sorted.begin() + first_unsorted_row, sorted.end());

  /* Create new result */
  for (auto i : sorted) {
    dynstr_append_mem(ds, i.c_str(), i.length());
    dynstr_append(ds, "\n");
  }
}
