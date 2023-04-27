/*
Copyright (c) 2000, 2020, Oracle and/or its affiliates. All rights reserved.

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
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

// mysql command tool

#include "my_config.h"

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <math.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>

#include "client/client_priv.h"
#include "client/my_readline.h"
#include "client/pattern_matcher.h"
#include "compression.h"
#include "lex_string.h"
#include "m_ctype.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_default.h"
#include "my_dir.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "my_loglevel.h"
#include "my_macros.h"
#include "typelib.h"
#include "violite.h"

#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif

#if defined(USE_LIBEDIT_INTERFACE)
#include <locale.h>
#endif

#ifdef HAVE_PWD_H
#include <pwd.h>
#endif

#if defined(HAVE_TERM_H)
#define NOMACROS  // move() macro interferes with std::move.
#include <curses.h>
#include <term.h>
#endif

#if defined(_WIN32)
#include <conio.h>

// Not using syslog but EventLog on Win32, so a dummy facility is enough.
#define LOG_USER 0
#else
#include <readline.h>
#include <syslog.h>

#define HAVE_READLINE
#define USE_POPEN
#endif

#include <mysqld_error.h>
#include <algorithm>
#include <new>

#include "sql_common.h"

#include "sql-common/net_ns.h"

using std::max;
using std::min;

extern CHARSET_INFO my_charset_utf16le_bin;

const char *VER = "14.14";

/* Don't try to make a nice table if the data is too big */
#define MAX_COLUMN_LENGTH 1024

/* Buffer to hold 'version' and 'version_comment' */
static char *server_version = nullptr;

/* Array of options to pass to libemysqld */
#define MAX_SERVER_ARGS 64

/* Maximum memory limit that can be claimed by alloca(). */
#define MAX_ALLOCA_SIZE 512

#include "sql_string.h"

#ifdef FN_NO_CASE_SENSE
#define cmp_database(cs, A, B) my_strcasecmp((cs), (A), (B))
#else
#define cmp_database(cs, A, B) strcmp((A), (B))
#endif

#include "client/completion_hash.h"
#include "print_version.h"
#include "welcome_copyright_notice.h"  // ORACLE_WELCOME_COPYRIGHT_NOTICE

#define PROMPT_CHAR '\\'
#define DEFAULT_DELIMITER ";"

#define MAX_BATCH_BUFFER_SIZE (1024L * 1024L * 1024L)

/** default set of patterns used for history exclusion filter */
const static std::string HI_DEFAULTS("*IDENTIFIED*:*PASSWORD*");

/** used for matching which history lines to ignore */
static Pattern_matcher ignore_matcher;

struct STATUS {
  int exit_status;
  ulong query_start_line;
  char *file_name;
  LINE_BUFFER *line_buff;
  bool batch, add_to_history;
};

static HashTable ht;
static MEM_ROOT argv_alloc{PSI_NOT_INSTRUMENTED, 512};

enum enum_info_type { INFO_INFO, INFO_ERROR, INFO_RESULT };
typedef enum enum_info_type INFO_TYPE;

static MYSQL mysql; /* The connection */
static bool ignore_errors = false, wait_flag = false, quick = false,
            connected = false, opt_raw_data = false, unbuffered = false,
            output_tables = false, opt_rehash = true, skip_updates = false,
            safe_updates = false, one_database = false, opt_compress = false,
            using_opt_local_infile = false, vertical = false,
            line_numbers = true, column_names = true, opt_html = false,
            opt_xml = false, opt_nopager = true, opt_outfile = false,
            named_cmds = false, tty_password = false, opt_nobeep = false,
            opt_reconnect = true, default_pager_set = false,
            opt_sigint_ignore = false, auto_vertical_output = false,
            show_warnings = false, executing_query = false,
            interrupted_query = false, ignore_spaces = false,
            sigint_received = false, opt_syslog = false, opt_binhex = false;
static bool opt_checksum = false;
static bool opt_binary_as_hex_set_explicitly = false;
static bool debug_info_flag, debug_check_flag;
static bool column_types_flag;
static bool preserve_comments = false;
static ulong opt_max_allowed_packet, opt_net_buffer_length;
static uint verbose = 0, opt_silent = 0, opt_mysql_port = 0,
            opt_local_infile = 0;
static uint opt_enable_cleartext_plugin = 0;
static bool using_opt_enable_cleartext_plugin = false;
static uint my_end_arg;
static char *opt_mysql_unix_port = nullptr;
static char *opt_bind_addr = nullptr;
static int connect_flag = CLIENT_INTERACTIVE;
static bool opt_binary_mode = false;
static bool opt_connect_expired_password = false;
static char *current_host;
static char *current_db;
static char *current_user = nullptr;
static char *opt_password = nullptr;
static char *current_prompt = nullptr;
static char *delimiter_str = nullptr;
static char *opt_init_command = nullptr;
static const char *default_charset = (const char *)"latin1";
static char *histfile;
static char *histfile_tmp;
static char *opt_histignore = nullptr;
static String glob_buffer, old_buffer;
static String processed_prompt;
static char *full_username = nullptr, *part_username = nullptr,
            *default_prompt = nullptr;
static char *current_os_user = nullptr, *current_os_sudouser = nullptr;
static int wait_time = 5;
static STATUS status;
static ulong select_limit, max_join_size, opt_connect_timeout = 0;
static char mysql_charsets_dir[FN_REFLEN + 1];
static char *opt_plugin_dir = nullptr, *opt_default_auth = nullptr;
#ifdef HAVE_SETNS
static char *opt_network_namespace = nullptr;
#endif
static const char *xmlmeta[] = {
    "&", "&amp;", "<", "&lt;", ">", "&gt;", "\"", "&quot;",
    /* Turn \0 into a space. Why not &#0;? That's not valid XML or HTML. */
    "\0", " ", nullptr, nullptr};
static const char *day_names[] = {"Sun", "Mon", "Tue", "Wed",
                                  "Thu", "Fri", "Sat"};
static const char *month_names[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
static char default_pager[FN_REFLEN];
static char pager[FN_REFLEN], outfile[FN_REFLEN];
static FILE *PAGER, *OUTFILE;
static MEM_ROOT hash_mem_root;
static uint prompt_counter;
static char delimiter[16] = DEFAULT_DELIMITER;
static size_t delimiter_length = 1;
unsigned short terminal_width = 80;
static uint opt_zstd_compress_level = default_zstd_compression_level;
static char *opt_compress_algorithm = nullptr;

#if defined(_WIN32)
static char *shared_memory_base_name = 0;
#endif
static uint opt_protocol = 0;
static const CHARSET_INFO *charset_info = &my_charset_latin1;

#include "caching_sha2_passwordopt-vars.h"
#include "sslopt-vars.h"

/*
  A simple session cache for SSL session. We simply try to
  reuse the most recently seen session; the server will reject it if
  it is invalid (or, say, it came from another host, etc).
*/
static void *ssl_session = nullptr;

/* The SSL context that will be reused across invocations. */
static void *ssl_context = nullptr;

const char *default_dbug_option = "d:t:o,/tmp/mysql.trace";

/*
  completion_hash is an auxiliary feature for mysql client to complete
  an object name(db name, table name and field name) automatically.
  e.g.
  mysql> use my_d
  then press <TAB>, it will check the hash and complete the db name
  for users.
  the result will be:
  mysql> use my_dbname

  In general, this feature is only on when it is an interactive mysql client.
  It is not possible to use it in test case.

  For using this feature in test case, we add the option in debug code.
*/
#ifndef DBUG_OFF
static bool opt_build_completion_hash = false;
#endif

#ifdef _WIN32
/*
  A flag that indicates if --execute buffer has already been converted,
  to avoid double conversion on reconnect.
*/
static bool execute_buffer_conversion_done = 0;

/*
  my_win_is_console(...) is quite slow.
  We cache my_win_is_console() results for stdout and stderr.
  Any other output files, except stdout and stderr,
  cannot be Windows console.
  Note, if mysql.exe is executed from a service, its _fileno(stdout) is -1,
  so shift (1 << -1) can return implementation defined result.
  This corner case is taken into account, as the shift result
  will be multiplied to 0 and we'll get 0 as a result.
  The same is true for stderr.
*/
static uint win_is_console_cache =
    ((my_win_is_console(stdout)) * (1 << _fileno(stdout))) |
    ((my_win_is_console(stderr)) * (1 << _fileno(stderr)));

static inline bool my_win_is_console_cached(FILE *file) {
  return win_is_console_cache & (1 << _fileno(file));
}
#endif /* _WIN32 */

/* Various printing flags */
#define MY_PRINT_ESC_0 1 /* Replace 0x00 bytes to "\0"              */
#define MY_PRINT_SPS_0 2 /* Replace 0x00 bytes to space             */
#define MY_PRINT_XML 4   /* Encode XML entities                     */
#define MY_PRINT_MB 8    /* Recognize multi-byte characters         */
#define MY_PRINT_CTRL 16 /* Replace TAB, NL, CR to "\t", "\n", "\r" */

void tee_write(FILE *file, const char *s, size_t slen, int flags);
void tee_fprintf(FILE *file, const char *fmt, ...)
    MY_ATTRIBUTE((format(printf, 2, 3)));
void tee_fputs(const char *s, FILE *file);
void tee_puts(const char *s, FILE *file);
void tee_putc(int c, FILE *file);
static void tee_print_sized_data(const char *, unsigned int, unsigned int,
                                 bool);
/* The names of functions that actually do the manipulation. */
static int get_options(int argc, char **argv);
extern "C" bool get_one_option(int optid, const struct my_option *opt,
                               char *argument);
static int com_quit(String *str, char *), com_go(String *str, char *),
    com_ego(String *str, char *), com_print(String *str, char *),
    com_help(String *str, char *), com_clear(String *str, char *),
    com_connect(String *str, char *), com_status(String *str, char *),
    com_use(String *str, char *), com_source(String *str, char *),
    com_rehash(String *str, char *), com_tee(String *str, char *),
    com_notee(String *str, char *), com_charset(String *str, char *),
    com_prompt(String *str, char *), com_delimiter(String *str, char *),
    com_warnings(String *str, char *), com_nowarnings(String *str, char *),
    com_resetconnection(String *str, char *), com_attr(String *str, char *),
    com_resp_attr(String *str, char *);
static int com_shell(String *str, char *);

#ifdef USE_POPEN
static int com_nopager(String *str, char *), com_pager(String *str, char *),
    com_edit(String *str, char *);
#endif

static int read_and_execute(bool interactive);
static bool init_connection_options(MYSQL *mysql);
static int sql_connect(char *host, char *database, char *user, char *password,
                       uint silent);
static const char *server_version_string(MYSQL *mysql);
static int put_info(const char *str, INFO_TYPE info, uint error = 0,
                    const char *sql_state = nullptr);
static int put_error(MYSQL *mysql);
static void safe_put_field(const char *pos, ulong length);
static void xmlencode_print(const char *src, uint length);
static void init_pager();
static void end_pager();
static void init_tee(const char *);
static void end_tee();
static const char *construct_prompt();
static inline void reset_prompt(char *in_string, bool *ml_comment);
static char *get_arg(char *line, bool get_next_arg);
static void init_username();
static void add_int_to_prompt(int toadd);
static int get_result_width(MYSQL_RES *res);
static int get_field_disp_length(MYSQL_FIELD *field);
static int normalize_dbname(const char *line, char *buff, uint buff_size);
static int get_quote_count(const char *line);

static void add_filtered_history(const char *string);
static void add_syslog(const char *buffer); /* for syslog */
static void fix_line(String *buffer);

static void get_current_os_user();
static void get_current_os_sudouser();

/* A structure which contains information on the commands this program
   can understand. */

typedef struct {
  const char *name;                 /* User printable name of the function. */
  char cmd_char;                    /* msql command character */
  int (*func)(String *str, char *); /* Function to call to do the job. */
  bool takes_params;                /* Max parameters for command */
  const char *doc;                  /* Documentation for this function.  */
} COMMANDS;

static COMMANDS commands[] = {
    {"?", '?', com_help, true, "Synonym for `help'."},
    {"clear", 'c', com_clear, false, "Clear the current input statement."},
    {"connect", 'r', com_connect, true,
     "Reconnect to the server. Optional arguments are db and host."},
    {"delimiter", 'd', com_delimiter, true, "Set statement delimiter."},
#ifdef USE_POPEN
    {"edit", 'e', com_edit, false, "Edit command with $EDITOR."},
#endif
    {"ego", 'G', com_ego, false,
     "Send command to mysql server, display result vertically."},
    {"exit", 'q', com_quit, false, "Exit mysql. Same as quit."},
    {"go", 'g', com_go, false, "Send command to mysql server."},
    {"help", 'h', com_help, true, "Display this help."},
#ifdef USE_POPEN
    {"nopager", 'n', com_nopager, false, "Disable pager, print to stdout."},
#endif
    {"notee", 't', com_notee, false, "Don't write into outfile."},
#ifdef USE_POPEN
    {"pager", 'P', com_pager, true,
     "Set PAGER [to_pager]. Print the query results via PAGER."},
#endif
    {"print", 'p', com_print, false, "Print current command."},
    {"prompt", 'R', com_prompt, true, "Change your mysql prompt."},
    {"quit", 'q', com_quit, false, "Quit mysql."},
    {"rehash", '#', com_rehash, false, "Rebuild completion hash."},
    {"setattr", 'z', com_attr, true, "Set query attribute."},
    {"getattr", 'z', com_resp_attr, true, "Get response attribute."},
    {"source", '.', com_source, true,
     "Execute an SQL script file. Takes a file name as an argument."},
    {"status", 's', com_status, false,
     "Get status information from the server."},
    {"system", '!', com_shell, true, "Execute a system shell command."},
    {"tee", 'T', com_tee, true,
     "Set outfile [to_outfile]. Append everything into given outfile."},
    {"use", 'u', com_use, true,
     "Use another database. Takes database name as argument."},
    {"charset", 'C', com_charset, true,
     "Switch to another charset. Might be needed for processing binlog with "
     "multi-byte charsets."},
    {"warnings", 'W', com_warnings, false,
     "Show warnings after every statement."},
    {"nowarning", 'w', com_nowarnings, false,
     "Don't show warnings after every statement."},
    {"resetconnection", 'x', com_resetconnection, false,
     "Clean session context."},
    /* Get bash-like expansion for some commands */
    {"create table", 0, nullptr, false, ""},
    {"create database", 0, nullptr, false, ""},
    {"show databases", 0, nullptr, false, ""},
    {"show fields from", 0, nullptr, false, ""},
    {"show keys from", 0, nullptr, false, ""},
    {"show tables", 0, nullptr, false, ""},
    {"load data from", 0, nullptr, false, ""},
    {"alter table", 0, nullptr, false, ""},
    {"set option", 0, nullptr, false, ""},
    {"lock tables", 0, nullptr, false, ""},
    {"unlock tables", 0, nullptr, false, ""},
    /* generated 2006-12-28.  Refresh occasionally from lexer. */
    {"ACTION", 0, nullptr, false, ""},
    {"ADD", 0, nullptr, false, ""},
    {"AFTER", 0, nullptr, false, ""},
    {"AGAINST", 0, nullptr, false, ""},
    {"AGGREGATE", 0, nullptr, false, ""},
    {"ALL", 0, nullptr, false, ""},
    {"ALGORITHM", 0, nullptr, false, ""},
    {"ALTER", 0, nullptr, false, ""},
    {"ANALYZE", 0, nullptr, false, ""},
    {"AND", 0, nullptr, false, ""},
    {"ANY", 0, nullptr, false, ""},
    {"AS", 0, nullptr, false, ""},
    {"ASC", 0, nullptr, false, ""},
    {"ASCII", 0, nullptr, false, ""},
    {"ASENSITIVE", 0, nullptr, false, ""},
    {"AUTO_INCREMENT", 0, nullptr, false, ""},
    {"AVG", 0, nullptr, false, ""},
    {"AVG_ROW_LENGTH", 0, nullptr, false, ""},
    {"BACKUP", 0, nullptr, false, ""},
    {"BDB", 0, nullptr, false, ""},
    {"BEFORE", 0, nullptr, false, ""},
    {"BEGIN", 0, nullptr, false, ""},
    {"BERKELEYDB", 0, nullptr, false, ""},
    {"BETWEEN", 0, nullptr, false, ""},
    {"BIGINT", 0, nullptr, false, ""},
    {"BINARY", 0, nullptr, false, ""},
    {"BINLOG", 0, nullptr, false, ""},
    {"BIT", 0, nullptr, false, ""},
    {"BLOB", 0, nullptr, false, ""},
    {"BOOL", 0, nullptr, false, ""},
    {"BOOLEAN", 0, nullptr, false, ""},
    {"BOTH", 0, nullptr, false, ""},
    {"BTREE", 0, nullptr, false, ""},
    {"BY", 0, nullptr, false, ""},
    {"BYTE", 0, nullptr, false, ""},
    {"CACHE", 0, nullptr, false, ""},
    {"CALL", 0, nullptr, false, ""},
    {"CASCADE", 0, nullptr, false, ""},
    {"CASCADED", 0, nullptr, false, ""},
    {"CASE", 0, nullptr, false, ""},
    {"CHAIN", 0, nullptr, false, ""},
    {"CHANGE", 0, nullptr, false, ""},
    {"CHANGED", 0, nullptr, false, ""},
    {"CHAR", 0, nullptr, false, ""},
    {"CHARACTER", 0, nullptr, false, ""},
    {"CHARSET", 0, nullptr, false, ""},
    {"CHECK", 0, nullptr, false, ""},
    {"CHECKSUM", 0, nullptr, false, ""},
    {"CIPHER", 0, nullptr, false, ""},
    {"CLIENT", 0, nullptr, false, ""},
    {"CLOSE", 0, nullptr, false, ""},
    {"CODE", 0, nullptr, false, ""},
    {"COLLATE", 0, nullptr, false, ""},
    {"COLLATION", 0, nullptr, false, ""},
    {"COLUMN", 0, nullptr, false, ""},
    {"COLUMNS", 0, nullptr, false, ""},
    {"COMMENT", 0, nullptr, false, ""},
    {"COMMIT", 0, nullptr, false, ""},
    {"COMMITTED", 0, nullptr, false, ""},
    {"COMPACT", 0, nullptr, false, ""},
    {"COMPRESSED", 0, nullptr, false, ""},
    {"CONCURRENT", 0, nullptr, false, ""},
    {"CONDITION", 0, nullptr, false, ""},
    {"CONNECTION", 0, nullptr, false, ""},
    {"CONSISTENT", 0, nullptr, false, ""},
    {"CONSTRAINT", 0, nullptr, false, ""},
    {"CONTAINS", 0, nullptr, false, ""},
    {"CONTINUE", 0, nullptr, false, ""},
    {"CONVERT", 0, nullptr, false, ""},
    {"CREATE", 0, nullptr, false, ""},
    {"CROSS", 0, nullptr, false, ""},
    {"CUBE", 0, nullptr, false, ""},
    {"CURRENT_DATE", 0, nullptr, false, ""},
    {"CURRENT_TIME", 0, nullptr, false, ""},
    {"CURRENT_TIMESTAMP", 0, nullptr, false, ""},
    {"CURRENT_USER", 0, nullptr, false, ""},
    {"CURSOR", 0, nullptr, false, ""},
    {"DATA", 0, nullptr, false, ""},
    {"DATABASE", 0, nullptr, false, ""},
    {"DATABASES", 0, nullptr, false, ""},
    {"DATE", 0, nullptr, false, ""},
    {"DATETIME", 0, nullptr, false, ""},
    {"DAY", 0, nullptr, false, ""},
    {"DAY_HOUR", 0, nullptr, false, ""},
    {"DAY_MICROSECOND", 0, nullptr, false, ""},
    {"DAY_MINUTE", 0, nullptr, false, ""},
    {"DAY_SECOND", 0, nullptr, false, ""},
    {"DEALLOCATE", 0, nullptr, false, ""},
    {"DEC", 0, nullptr, false, ""},
    {"DECIMAL", 0, nullptr, false, ""},
    {"DECLARE", 0, nullptr, false, ""},
    {"DEFAULT", 0, nullptr, false, ""},
    {"DEFINER", 0, nullptr, false, ""},
    {"DELAYED", 0, nullptr, false, ""},
    {"DELAY_KEY_WRITE", 0, nullptr, false, ""},
    {"DELETE", 0, nullptr, false, ""},
    {"DESC", 0, nullptr, false, ""},
    {"DESCRIBE", 0, nullptr, false, ""},
    {"DETERMINISTIC", 0, nullptr, false, ""},
    {"DIRECTORY", 0, nullptr, false, ""},
    {"DISABLE", 0, nullptr, false, ""},
    {"DISCARD", 0, nullptr, false, ""},
    {"DISTINCT", 0, nullptr, false, ""},
    {"DISTINCTROW", 0, nullptr, false, ""},
    {"DIV", 0, nullptr, false, ""},
    {"DO", 0, nullptr, false, ""},
    {"DOUBLE", 0, nullptr, false, ""},
    {"DROP", 0, nullptr, false, ""},
    {"DUAL", 0, nullptr, false, ""},
    {"DUMPFILE", 0, nullptr, false, ""},
    {"DUPLICATE", 0, nullptr, false, ""},
    {"DYNAMIC", 0, nullptr, false, ""},
    {"EACH", 0, nullptr, false, ""},
    {"ELSE", 0, nullptr, false, ""},
    {"ELSEIF", 0, nullptr, false, ""},
    {"ENABLE", 0, nullptr, false, ""},
    {"ENCLOSED", 0, nullptr, false, ""},
    {"END", 0, nullptr, false, ""},
    {"ENGINE", 0, nullptr, false, ""},
    {"ENGINES", 0, nullptr, false, ""},
    {"ENUM", 0, nullptr, false, ""},
    {"ERRORS", 0, nullptr, false, ""},
    {"ESCAPE", 0, nullptr, false, ""},
    {"ESCAPED", 0, nullptr, false, ""},
    {"EVENTS", 0, nullptr, false, ""},
    {"EXECUTE", 0, nullptr, false, ""},
    {"EXISTS", 0, nullptr, false, ""},
    {"EXIT", 0, nullptr, false, ""},
    {"EXPANSION", 0, nullptr, false, ""},
    {"EXPLAIN", 0, nullptr, false, ""},
    {"EXTENDED", 0, nullptr, false, ""},
    {"FALSE", 0, nullptr, false, ""},
    {"FAST", 0, nullptr, false, ""},
    {"FETCH", 0, nullptr, false, ""},
    {"FIELDS", 0, nullptr, false, ""},
    {"FILE", 0, nullptr, false, ""},
    {"FIRST", 0, nullptr, false, ""},
    {"FIXED", 0, nullptr, false, ""},
    {"FLOAT", 0, nullptr, false, ""},
    {"FLOAT4", 0, nullptr, false, ""},
    {"FLOAT8", 0, nullptr, false, ""},
    {"FLUSH", 0, nullptr, false, ""},
    {"FOR", 0, nullptr, false, ""},
    {"FORCE", 0, nullptr, false, ""},
    {"FOREIGN", 0, nullptr, false, ""},
    {"FOUND", 0, nullptr, false, ""},
    {"FROM", 0, nullptr, false, ""},
    {"FULL", 0, nullptr, false, ""},
    {"FULLTEXT", 0, nullptr, false, ""},
    {"FUNCTION", 0, nullptr, false, ""},
    {"GEOMETRY", 0, nullptr, false, ""},
    {"GEOMETRYCOLLECTION", 0, nullptr, false, ""},
    {"GET_FORMAT", 0, nullptr, false, ""},
    {"GLOBAL", 0, nullptr, false, ""},
    {"GRANT", 0, nullptr, false, ""},
    {"GRANTS", 0, nullptr, false, ""},
    {"GROUP", 0, nullptr, false, ""},
    {"HANDLER", 0, nullptr, false, ""},
    {"HASH", 0, nullptr, false, ""},
    {"HAVING", 0, nullptr, false, ""},
    {"HELP", 0, nullptr, false, ""},
    {"HIGH_PRIORITY", 0, nullptr, false, ""},
    {"HOSTS", 0, nullptr, false, ""},
    {"HOUR", 0, nullptr, false, ""},
    {"HOUR_MICROSECOND", 0, nullptr, false, ""},
    {"HOUR_MINUTE", 0, nullptr, false, ""},
    {"HOUR_SECOND", 0, nullptr, false, ""},
    {"IDENTIFIED", 0, nullptr, false, ""},
    {"IF", 0, nullptr, false, ""},
    {"IGNORE", 0, nullptr, false, ""},
    {"IMPORT", 0, nullptr, false, ""},
    {"IN", 0, nullptr, false, ""},
    {"INDEX", 0, nullptr, false, ""},
    {"INDEXES", 0, nullptr, false, ""},
    {"INFILE", 0, nullptr, false, ""},
    {"INNER", 0, nullptr, false, ""},
    {"INNOBASE", 0, nullptr, false, ""},
    {"INNODB", 0, nullptr, false, ""},
    {"INOUT", 0, nullptr, false, ""},
    {"INSENSITIVE", 0, nullptr, false, ""},
    {"INSERT", 0, nullptr, false, ""},
    {"INSERT_METHOD", 0, nullptr, false, ""},
    {"INT", 0, nullptr, false, ""},
    {"INT1", 0, nullptr, false, ""},
    {"INT2", 0, nullptr, false, ""},
    {"INT3", 0, nullptr, false, ""},
    {"INT4", 0, nullptr, false, ""},
    {"INT8", 0, nullptr, false, ""},
    {"INTEGER", 0, nullptr, false, ""},
    {"INTERVAL", 0, nullptr, false, ""},
    {"INTO", 0, nullptr, false, ""},
    {"IO_THREAD", 0, nullptr, false, ""},
    {"IS", 0, nullptr, false, ""},
    {"ISOLATION", 0, nullptr, false, ""},
    {"ISSUER", 0, nullptr, false, ""},
    {"ITERATE", 0, nullptr, false, ""},
    {"INVOKER", 0, nullptr, false, ""},
    {"JOIN", 0, nullptr, false, ""},
    {"KEY", 0, nullptr, false, ""},
    {"KEYS", 0, nullptr, false, ""},
    {"KILL", 0, nullptr, false, ""},
    {"LANGUAGE", 0, nullptr, false, ""},
    {"LAST", 0, nullptr, false, ""},
    {"LEADING", 0, nullptr, false, ""},
    {"LEAVE", 0, nullptr, false, ""},
    {"LEAVES", 0, nullptr, false, ""},
    {"LEFT", 0, nullptr, false, ""},
    {"LEVEL", 0, nullptr, false, ""},
    {"LIKE", 0, nullptr, false, ""},
    {"LIMIT", 0, nullptr, false, ""},
    {"LINES", 0, nullptr, false, ""},
    {"LINESTRING", 0, nullptr, false, ""},
    {"LOAD", 0, nullptr, false, ""},
    {"LOCAL", 0, nullptr, false, ""},
    {"LOCALTIME", 0, nullptr, false, ""},
    {"LOCALTIMESTAMP", 0, nullptr, false, ""},
    {"LOCK", 0, nullptr, false, ""},
    {"LOCKS", 0, nullptr, false, ""},
    {"LOGS", 0, nullptr, false, ""},
    {"LONG", 0, nullptr, false, ""},
    {"LONGBLOB", 0, nullptr, false, ""},
    {"LONGTEXT", 0, nullptr, false, ""},
    {"LOOP", 0, nullptr, false, ""},
    {"LOW_PRIORITY", 0, nullptr, false, ""},
    {"MASTER", 0, nullptr, false, ""},
    {"MASTER_CONNECT_RETRY", 0, nullptr, false, ""},
    {"MASTER_HOST", 0, nullptr, false, ""},
    {"MASTER_LOG_FILE", 0, nullptr, false, ""},
    {"MASTER_LOG_POS", 0, nullptr, false, ""},
    {"MASTER_PASSWORD", 0, nullptr, false, ""},
    {"MASTER_PORT", 0, nullptr, false, ""},
    {"MASTER_SERVER_ID", 0, nullptr, false, ""},
    {"MASTER_SSL", 0, nullptr, false, ""},
    {"MASTER_SSL_CA", 0, nullptr, false, ""},
    {"MASTER_SSL_CAPATH", 0, nullptr, false, ""},
    {"MASTER_SSL_CERT", 0, nullptr, false, ""},
    {"MASTER_SSL_CIPHER", 0, nullptr, false, ""},
    {"MASTER_TLS_VERSION", 0, nullptr, false, ""},
    {"MASTER_SSL_KEY", 0, nullptr, false, ""},
    {"MASTER_USER", 0, nullptr, false, ""},
    {"MATCH", 0, nullptr, false, ""},
    {"MAX_CONNECTIONS_PER_HOUR", 0, nullptr, false, ""},
    {"MAX_QUERIES_PER_HOUR", 0, nullptr, false, ""},
    {"MAX_ROWS", 0, nullptr, false, ""},
    {"MAX_UPDATES_PER_HOUR", 0, nullptr, false, ""},
    {"MAX_USER_CONNECTIONS", 0, nullptr, false, ""},
    {"MEDIUM", 0, nullptr, false, ""},
    {"MEDIUMBLOB", 0, nullptr, false, ""},
    {"MEDIUMINT", 0, nullptr, false, ""},
    {"MEDIUMTEXT", 0, nullptr, false, ""},
    {"MERGE", 0, nullptr, false, ""},
    {"MICROSECOND", 0, nullptr, false, ""},
    {"MIDDLEINT", 0, nullptr, false, ""},
    {"MIGRATE", 0, nullptr, false, ""},
    {"MINUTE", 0, nullptr, false, ""},
    {"MINUTE_MICROSECOND", 0, nullptr, false, ""},
    {"MINUTE_SECOND", 0, nullptr, false, ""},
    {"MIN_ROWS", 0, nullptr, false, ""},
    {"MOD", 0, nullptr, false, ""},
    {"MODE", 0, nullptr, false, ""},
    {"MODIFIES", 0, nullptr, false, ""},
    {"MODIFY", 0, nullptr, false, ""},
    {"MONTH", 0, nullptr, false, ""},
    {"MULTILINESTRING", 0, nullptr, false, ""},
    {"MULTIPOINT", 0, nullptr, false, ""},
    {"MULTIPOLYGON", 0, nullptr, false, ""},
    {"MUTEX", 0, nullptr, false, ""},
    {"NAME", 0, nullptr, false, ""},
    {"NAMES", 0, nullptr, false, ""},
    {"NATIONAL", 0, nullptr, false, ""},
    {"NATURAL", 0, nullptr, false, ""},
    {"NDB", 0, nullptr, false, ""},
    {"NDBCLUSTER", 0, nullptr, false, ""},
    {"NCHAR", 0, nullptr, false, ""},
    {"NEW", 0, nullptr, false, ""},
    {"NEXT", 0, nullptr, false, ""},
    {"NO", 0, nullptr, false, ""},
    {"NONE", 0, nullptr, false, ""},
    {"NOT", 0, nullptr, false, ""},
    {"NO_WRITE_TO_BINLOG", 0, nullptr, false, ""},
    {"NULL", 0, nullptr, false, ""},
    {"NUMERIC", 0, nullptr, false, ""},
    {"NVARCHAR", 0, nullptr, false, ""},
    {"OFFSET", 0, nullptr, false, ""},
    {"ON", 0, nullptr, false, ""},
    {"ONE", 0, nullptr, false, ""},
    {"ONE_SHOT", 0, nullptr, false, ""},
    {"OPEN", 0, nullptr, false, ""},
    {"OPTIMIZE", 0, nullptr, false, ""},
    {"OPTION", 0, nullptr, false, ""},
    {"OPTIONALLY", 0, nullptr, false, ""},
    {"OR", 0, nullptr, false, ""},
    {"ORDER", 0, nullptr, false, ""},
    {"OUT", 0, nullptr, false, ""},
    {"OUTER", 0, nullptr, false, ""},
    {"OUTFILE", 0, nullptr, false, ""},
    {"PACK_KEYS", 0, nullptr, false, ""},
    {"PARTIAL", 0, nullptr, false, ""},
    {"PASSWORD", 0, nullptr, false, ""},
    {"PHASE", 0, nullptr, false, ""},
    {"POINT", 0, nullptr, false, ""},
    {"POLYGON", 0, nullptr, false, ""},
    {"PRECISION", 0, nullptr, false, ""},
    {"PREPARE", 0, nullptr, false, ""},
    {"PREV", 0, nullptr, false, ""},
    {"PRIMARY", 0, nullptr, false, ""},
    {"PRIVILEGES", 0, nullptr, false, ""},
    {"PROCEDURE", 0, nullptr, false, ""},
    {"PROCESS", 0, nullptr, false, ""},
    {"PROCESSLIST", 0, nullptr, false, ""},
    {"PURGE", 0, nullptr, false, ""},
    {"QUARTER", 0, nullptr, false, ""},
    {"QUERY", 0, nullptr, false, ""},
    {"QUICK", 0, nullptr, false, ""},
    {"READ", 0, nullptr, false, ""},
    {"READS", 0, nullptr, false, ""},
    {"REAL", 0, nullptr, false, ""},
    {"RECOVER", 0, nullptr, false, ""},
    {"REDUNDANT", 0, nullptr, false, ""},
    {"REFERENCES", 0, nullptr, false, ""},
    {"REGEXP", 0, nullptr, false, ""},
    {"RELAY_LOG_FILE", 0, nullptr, false, ""},
    {"RELAY_LOG_POS", 0, nullptr, false, ""},
    {"RELAY_THREAD", 0, nullptr, false, ""},
    {"RELEASE", 0, nullptr, false, ""},
    {"RELOAD", 0, nullptr, false, ""},
    {"RENAME", 0, nullptr, false, ""},
    {"REPAIR", 0, nullptr, false, ""},
    {"REPEATABLE", 0, nullptr, false, ""},
    {"REPLACE", 0, nullptr, false, ""},
    {"REPLICATION", 0, nullptr, false, ""},
    {"REPEAT", 0, nullptr, false, ""},
    {"REQUIRE", 0, nullptr, false, ""},
    {"RESET", 0, nullptr, false, ""},
    {"RESTORE", 0, nullptr, false, ""},
    {"RESTRICT", 0, nullptr, false, ""},
    {"RESUME", 0, nullptr, false, ""},
    {"RETURN", 0, nullptr, false, ""},
    {"RETURNS", 0, nullptr, false, ""},
    {"REVOKE", 0, nullptr, false, ""},
    {"RIGHT", 0, nullptr, false, ""},
    {"RLIKE", 0, nullptr, false, ""},
    {"ROLLBACK", 0, nullptr, false, ""},
    {"ROLLUP", 0, nullptr, false, ""},
    {"ROUTINE", 0, nullptr, false, ""},
    {"ROW", 0, nullptr, false, ""},
    {"ROWS", 0, nullptr, false, ""},
    {"ROW_FORMAT", 0, nullptr, false, ""},
    {"RTREE", 0, nullptr, false, ""},
    {"SAVEPOINT", 0, nullptr, false, ""},
    {"SCHEMA", 0, nullptr, false, ""},
    {"SCHEMAS", 0, nullptr, false, ""},
    {"SECOND", 0, nullptr, false, ""},
    {"SECOND_MICROSECOND", 0, nullptr, false, ""},
    {"SECURITY", 0, nullptr, false, ""},
    {"SELECT", 0, nullptr, false, ""},
    {"SENSITIVE", 0, nullptr, false, ""},
    {"SEPARATOR", 0, nullptr, false, ""},
    {"SERIAL", 0, nullptr, false, ""},
    {"SERIALIZABLE", 0, nullptr, false, ""},
    {"SESSION", 0, nullptr, false, ""},
    {"SET", 0, nullptr, false, ""},
    {"SHARE", 0, nullptr, false, ""},
    {"SHOW", 0, nullptr, false, ""},
    {"SHUTDOWN", 0, nullptr, false, ""},
    {"SIGNED", 0, nullptr, false, ""},
    {"SIMPLE", 0, nullptr, false, ""},
    {"SLAVE", 0, nullptr, false, ""},
    {"SNAPSHOT", 0, nullptr, false, ""},
    {"SMALLINT", 0, nullptr, false, ""},
    {"SOME", 0, nullptr, false, ""},
    {"SONAME", 0, nullptr, false, ""},
    {"SOUNDS", 0, nullptr, false, ""},
    {"SPATIAL", 0, nullptr, false, ""},
    {"SPECIFIC", 0, nullptr, false, ""},
    {"SQL", 0, nullptr, false, ""},
    {"SQLEXCEPTION", 0, nullptr, false, ""},
    {"SQLSTATE", 0, nullptr, false, ""},
    {"SQLWARNING", 0, nullptr, false, ""},
    {"SQL_BIG_RESULT", 0, nullptr, false, ""},
    {"SQL_BUFFER_RESULT", 0, nullptr, false, ""},
    {"SQL_CALC_FOUND_ROWS", 0, nullptr, false, ""},
    {"SQL_NO_CACHE", 0, nullptr, false, ""},
    {"SQL_NO_FCACHE", 0, nullptr, false, ""},
    {"SQL_SMALL_RESULT", 0, nullptr, false, ""},
    {"SQL_THREAD", 0, nullptr, false, ""},
    {"SQL_TSI_SECOND", 0, nullptr, false, ""},
    {"SQL_TSI_MINUTE", 0, nullptr, false, ""},
    {"SQL_TSI_HOUR", 0, nullptr, false, ""},
    {"SQL_TSI_DAY", 0, nullptr, false, ""},
    {"SQL_TSI_WEEK", 0, nullptr, false, ""},
    {"SQL_TSI_MONTH", 0, nullptr, false, ""},
    {"SQL_TSI_QUARTER", 0, nullptr, false, ""},
    {"SQL_TSI_YEAR", 0, nullptr, false, ""},
    {"SSL", 0, nullptr, false, ""},
    {"START", 0, nullptr, false, ""},
    {"STARTING", 0, nullptr, false, ""},
    {"STATUS", 0, nullptr, false, ""},
    {"STOP", 0, nullptr, false, ""},
    {"STORAGE", 0, nullptr, false, ""},
    {"STRAIGHT_JOIN", 0, nullptr, false, ""},
    {"STRING", 0, nullptr, false, ""},
    {"STRIPED", 0, nullptr, false, ""},
    {"SUBJECT", 0, nullptr, false, ""},
    {"SUPER", 0, nullptr, false, ""},
    {"SUSPEND", 0, nullptr, false, ""},
    {"TABLE", 0, nullptr, false, ""},
    {"TABLES", 0, nullptr, false, ""},
    {"TABLESPACE", 0, nullptr, false, ""},
    {"TEMPORARY", 0, nullptr, false, ""},
    {"TEMPTABLE", 0, nullptr, false, ""},
    {"TERMINATED", 0, nullptr, false, ""},
    {"TEXT", 0, nullptr, false, ""},
    {"THEN", 0, nullptr, false, ""},
    {"TIME", 0, nullptr, false, ""},
    {"TIMESTAMP", 0, nullptr, false, ""},
    {"TIMESTAMPADD", 0, nullptr, false, ""},
    {"TIMESTAMPDIFF", 0, nullptr, false, ""},
    {"TINYBLOB", 0, nullptr, false, ""},
    {"TINYINT", 0, nullptr, false, ""},
    {"TINYTEXT", 0, nullptr, false, ""},
    {"TO", 0, nullptr, false, ""},
    {"TRAILING", 0, nullptr, false, ""},
    {"TRANSACTION", 0, nullptr, false, ""},
    {"TRIGGER", 0, nullptr, false, ""},
    {"TRIGGERS", 0, nullptr, false, ""},
    {"TRUE", 0, nullptr, false, ""},
    {"TRUNCATE", 0, nullptr, false, ""},
    {"TYPE", 0, nullptr, false, ""},
    {"TYPES", 0, nullptr, false, ""},
    {"UNCOMMITTED", 0, nullptr, false, ""},
    {"UNDEFINED", 0, nullptr, false, ""},
    {"UNDO", 0, nullptr, false, ""},
    {"UNICODE", 0, nullptr, false, ""},
    {"UNION", 0, nullptr, false, ""},
    {"UNIQUE", 0, nullptr, false, ""},
    {"UNKNOWN", 0, nullptr, false, ""},
    {"UNLOCK", 0, nullptr, false, ""},
    {"UNSIGNED", 0, nullptr, false, ""},
    {"UNTIL", 0, nullptr, false, ""},
    {"UPDATE", 0, nullptr, false, ""},
    {"UPGRADE", 0, nullptr, false, ""},
    {"USAGE", 0, nullptr, false, ""},
    {"USE", 0, nullptr, false, ""},
    {"USER", 0, nullptr, false, ""},
    {"USER_RESOURCES", 0, nullptr, false, ""},
    {"USE_FRM", 0, nullptr, false, ""},
    {"USING", 0, nullptr, false, ""},
    {"UTC_DATE", 0, nullptr, false, ""},
    {"UTC_TIME", 0, nullptr, false, ""},
    {"UTC_TIMESTAMP", 0, nullptr, false, ""},
    {"VALUE", 0, nullptr, false, ""},
    {"VALUES", 0, nullptr, false, ""},
    {"VARBINARY", 0, nullptr, false, ""},
    {"VARCHAR", 0, nullptr, false, ""},
    {"VARCHARACTER", 0, nullptr, false, ""},
    {"VARIABLES", 0, nullptr, false, ""},
    {"VARYING", 0, nullptr, false, ""},
    {"WARNINGS", 0, nullptr, false, ""},
    {"WEEK", 0, nullptr, false, ""},
    {"WHEN", 0, nullptr, false, ""},
    {"WHERE", 0, nullptr, false, ""},
    {"WHILE", 0, nullptr, false, ""},
    {"VIEW", 0, nullptr, false, ""},
    {"WITH", 0, nullptr, false, ""},
    {"WORK", 0, nullptr, false, ""},
    {"WRITE", 0, nullptr, false, ""},
    {"X509", 0, nullptr, false, ""},
    {"XOR", 0, nullptr, false, ""},
    {"XA", 0, nullptr, false, ""},
    {"YEAR", 0, nullptr, false, ""},
    {"YEAR_MONTH", 0, nullptr, false, ""},
    {"ZEROFILL", 0, nullptr, false, ""},
    {"ABS", 0, nullptr, false, ""},
    {"ACOS", 0, nullptr, false, ""},
    {"ADDDATE", 0, nullptr, false, ""},
    {"ADDTIME", 0, nullptr, false, ""},
    {"AES_ENCRYPT", 0, nullptr, false, ""},
    {"AES_DECRYPT", 0, nullptr, false, ""},
    {"AREA", 0, nullptr, false, ""},
    {"ASIN", 0, nullptr, false, ""},
    {"ASBINARY", 0, nullptr, false, ""},
    {"ASTEXT", 0, nullptr, false, ""},
    {"ASWKB", 0, nullptr, false, ""},
    {"ASWKT", 0, nullptr, false, ""},
    {"ATAN", 0, nullptr, false, ""},
    {"ATAN2", 0, nullptr, false, ""},
    {"BENCHMARK", 0, nullptr, false, ""},
    {"BIN", 0, nullptr, false, ""},
    {"BIT_COUNT", 0, nullptr, false, ""},
    {"BIT_OR", 0, nullptr, false, ""},
    {"BIT_AND", 0, nullptr, false, ""},
    {"BIT_XOR", 0, nullptr, false, ""},
    {"CAST", 0, nullptr, false, ""},
    {"CEIL", 0, nullptr, false, ""},
    {"CEILING", 0, nullptr, false, ""},
    {"BIT_LENGTH", 0, nullptr, false, ""},
    {"CENTROID", 0, nullptr, false, ""},
    {"CHAR_LENGTH", 0, nullptr, false, ""},
    {"CHARACTER_LENGTH", 0, nullptr, false, ""},
    {"COALESCE", 0, nullptr, false, ""},
    {"COERCIBILITY", 0, nullptr, false, ""},
    {"COMPRESS", 0, nullptr, false, ""},
    {"CONCAT", 0, nullptr, false, ""},
    {"CONCAT_WS", 0, nullptr, false, ""},
    {"CONNECTION_ID", 0, nullptr, false, ""},
    {"CONV", 0, nullptr, false, ""},
    {"CONVERT_TZ", 0, nullptr, false, ""},
    {"COUNT", 0, nullptr, false, ""},
    {"COS", 0, nullptr, false, ""},
    {"COT", 0, nullptr, false, ""},
    {"CRC32", 0, nullptr, false, ""},
    {"CROSSES", 0, nullptr, false, ""},
    {"CURDATE", 0, nullptr, false, ""},
    {"CURTIME", 0, nullptr, false, ""},
    {"DATE_ADD", 0, nullptr, false, ""},
    {"DATEDIFF", 0, nullptr, false, ""},
    {"DATE_FORMAT", 0, nullptr, false, ""},
    {"DATE_SUB", 0, nullptr, false, ""},
    {"DAYNAME", 0, nullptr, false, ""},
    {"DAYOFMONTH", 0, nullptr, false, ""},
    {"DAYOFWEEK", 0, nullptr, false, ""},
    {"DAYOFYEAR", 0, nullptr, false, ""},
    {"DEGREES", 0, nullptr, false, ""},
    {"DIMENSION", 0, nullptr, false, ""},
    {"DISJOINT", 0, nullptr, false, ""},
    {"ELT", 0, nullptr, false, ""},
    {"ENDPOINT", 0, nullptr, false, ""},
    {"ENVELOPE", 0, nullptr, false, ""},
    {"EQUALS", 0, nullptr, false, ""},
    {"EXTERIORRING", 0, nullptr, false, ""},
    {"EXTRACT", 0, nullptr, false, ""},
    {"EXP", 0, nullptr, false, ""},
    {"EXPORT_SET", 0, nullptr, false, ""},
    {"FIELD", 0, nullptr, false, ""},
    {"FIND_IN_SET", 0, nullptr, false, ""},
    {"FLOOR", 0, nullptr, false, ""},
    {"FORMAT", 0, nullptr, false, ""},
    {"FOUND_ROWS", 0, nullptr, false, ""},
    {"FROM_DAYS", 0, nullptr, false, ""},
    {"FROM_UNIXTIME", 0, nullptr, false, ""},
    {"GET_LOCK", 0, nullptr, false, ""},
    {"GEOMETRYN", 0, nullptr, false, ""},
    {"GEOMETRYTYPE", 0, nullptr, false, ""},
    {"GEOMCOLLFROMTEXT", 0, nullptr, false, ""},
    {"GEOMCOLLFROMWKB", 0, nullptr, false, ""},
    {"GEOMETRYCOLLECTIONFROMTEXT", 0, nullptr, false, ""},
    {"GEOMETRYCOLLECTIONFROMWKB", 0, nullptr, false, ""},
    {"GEOMETRYFROMTEXT", 0, nullptr, false, ""},
    {"GEOMETRYFROMWKB", 0, nullptr, false, ""},
    {"GEOMFROMTEXT", 0, nullptr, false, ""},
    {"GEOMFROMWKB", 0, nullptr, false, ""},
    {"GLENGTH", 0, nullptr, false, ""},
    {"GREATEST", 0, nullptr, false, ""},
    {"GROUP_CONCAT", 0, nullptr, false, ""},
    {"GROUP_UNIQUE_USERS", 0, nullptr, false, ""},
    {"HEX", 0, nullptr, false, ""},
    {"IFNULL", 0, nullptr, false, ""},
    {"INET_ATON", 0, nullptr, false, ""},
    {"INET_NTOA", 0, nullptr, false, ""},
    {"INSTR", 0, nullptr, false, ""},
    {"INTERIORRINGN", 0, nullptr, false, ""},
    {"INTERSECTS", 0, nullptr, false, ""},
    {"ISCLOSED", 0, nullptr, false, ""},
    {"ISEMPTY", 0, nullptr, false, ""},
    {"ISNULL", 0, nullptr, false, ""},
    {"IS_FREE_LOCK", 0, nullptr, false, ""},
    {"IS_USED_LOCK", 0, nullptr, false, ""},
    {"JSON_ARRAY_APPEND", 0, nullptr, false, ""},
    {"JSON_ARRAY", 0, nullptr, false, ""},
    {"JSON_CONTAINS", 0, nullptr, false, ""},
    {"JSON_DEPTH", 0, nullptr, false, ""},
    {"JSON_EXTRACT", 0, nullptr, false, ""},
    {"JSON_INSERT", 0, nullptr, false, ""},
    {"JSON_KEYS", 0, nullptr, false, ""},
    {"JSON_LENGTH", 0, nullptr, false, ""},
    {"JSON_MERGE", 0, nullptr, false, ""},
    {"JSON_QUOTE", 0, nullptr, false, ""},
    {"JSON_REPLACE", 0, nullptr, false, ""},
    {"JSON_ROWOBJECT", 0, nullptr, false, ""},
    {"JSON_SEARCH", 0, nullptr, false, ""},
    {"JSON_SET", 0, nullptr, false, ""},
    {"JSON_TYPE", 0, nullptr, false, ""},
    {"JSON_UNQUOTE", 0, nullptr, false, ""},
    {"JSON_VALID", 0, nullptr, false, ""},
    {"JSON_CONTAINS_PATH", 0, nullptr, false, ""},
    {"LAST_INSERT_ID", 0, nullptr, false, ""},
    {"ISSIMPLE", 0, nullptr, false, ""},
    {"LAST_DAY", 0, nullptr, false, ""},
    {"LCASE", 0, nullptr, false, ""},
    {"LEAST", 0, nullptr, false, ""},
    {"LENGTH", 0, nullptr, false, ""},
    {"LN", 0, nullptr, false, ""},
    {"LINEFROMTEXT", 0, nullptr, false, ""},
    {"LINEFROMWKB", 0, nullptr, false, ""},
    {"LINESTRINGFROMTEXT", 0, nullptr, false, ""},
    {"LINESTRINGFROMWKB", 0, nullptr, false, ""},
    {"LOAD_FILE", 0, nullptr, false, ""},
    {"LOCATE", 0, nullptr, false, ""},
    {"LOG", 0, nullptr, false, ""},
    {"LOG2", 0, nullptr, false, ""},
    {"LOG10", 0, nullptr, false, ""},
    {"LOWER", 0, nullptr, false, ""},
    {"LPAD", 0, nullptr, false, ""},
    {"LTRIM", 0, nullptr, false, ""},
    {"MAKE_SET", 0, nullptr, false, ""},
    {"MAKEDATE", 0, nullptr, false, ""},
    {"MAKETIME", 0, nullptr, false, ""},
    {"MASTER_POS_WAIT", 0, nullptr, false, ""},
    {"MAX", 0, nullptr, false, ""},
    {"MBRCONTAINS", 0, nullptr, false, ""},
    {"MBRDISJOINT", 0, nullptr, false, ""},
    {"MBREQUAL", 0, nullptr, false, ""},
    {"MBRINTERSECTS", 0, nullptr, false, ""},
    {"MBROVERLAPS", 0, nullptr, false, ""},
    {"MBRTOUCHES", 0, nullptr, false, ""},
    {"MBRWITHIN", 0, nullptr, false, ""},
    {"MD5", 0, nullptr, false, ""},
    {"MID", 0, nullptr, false, ""},
    {"MIN", 0, nullptr, false, ""},
    {"MLINEFROMTEXT", 0, nullptr, false, ""},
    {"MLINEFROMWKB", 0, nullptr, false, ""},
    {"MPOINTFROMTEXT", 0, nullptr, false, ""},
    {"MPOINTFROMWKB", 0, nullptr, false, ""},
    {"MPOLYFROMTEXT", 0, nullptr, false, ""},
    {"MPOLYFROMWKB", 0, nullptr, false, ""},
    {"MONTHNAME", 0, nullptr, false, ""},
    {"MULTILINESTRINGFROMTEXT", 0, nullptr, false, ""},
    {"MULTILINESTRINGFROMWKB", 0, nullptr, false, ""},
    {"MULTIPOINTFROMTEXT", 0, nullptr, false, ""},
    {"MULTIPOINTFROMWKB", 0, nullptr, false, ""},
    {"MULTIPOLYGONFROMTEXT", 0, nullptr, false, ""},
    {"MULTIPOLYGONFROMWKB", 0, nullptr, false, ""},
    {"NAME_CONST", 0, nullptr, false, ""},
    {"NOW", 0, nullptr, false, ""},
    {"NULLIF", 0, nullptr, false, ""},
    {"NUMGEOMETRIES", 0, nullptr, false, ""},
    {"NUMINTERIORRINGS", 0, nullptr, false, ""},
    {"NUMPOINTS", 0, nullptr, false, ""},
    {"OCTET_LENGTH", 0, nullptr, false, ""},
    {"OCT", 0, nullptr, false, ""},
    {"ORD", 0, nullptr, false, ""},
    {"OVERLAPS", 0, nullptr, false, ""},
    {"PERIOD_ADD", 0, nullptr, false, ""},
    {"PERIOD_DIFF", 0, nullptr, false, ""},
    {"PI", 0, nullptr, false, ""},
    {"POINTFROMTEXT", 0, nullptr, false, ""},
    {"POINTFROMWKB", 0, nullptr, false, ""},
    {"POINTN", 0, nullptr, false, ""},
    {"POLYFROMTEXT", 0, nullptr, false, ""},
    {"POLYFROMWKB", 0, nullptr, false, ""},
    {"POLYGONFROMTEXT", 0, nullptr, false, ""},
    {"POLYGONFROMWKB", 0, nullptr, false, ""},
    {"POSITION", 0, nullptr, false, ""},
    {"POW", 0, nullptr, false, ""},
    {"POWER", 0, nullptr, false, ""},
    {"QUOTE", 0, nullptr, false, ""},
    {"RADIANS", 0, nullptr, false, ""},
    {"RAND", 0, nullptr, false, ""},
    {"RELEASE_LOCK", 0, nullptr, false, ""},
    {"REVERSE", 0, nullptr, false, ""},
    {"ROUND", 0, nullptr, false, ""},
    {"ROW_COUNT", 0, nullptr, false, ""},
    {"RPAD", 0, nullptr, false, ""},
    {"RTRIM", 0, nullptr, false, ""},
    {"SEC_TO_TIME", 0, nullptr, false, ""},
    {"SESSION_USER", 0, nullptr, false, ""},
    {"SUBDATE", 0, nullptr, false, ""},
    {"SIGN", 0, nullptr, false, ""},
    {"SIN", 0, nullptr, false, ""},
    {"SHA", 0, nullptr, false, ""},
    {"SHA1", 0, nullptr, false, ""},
    {"SLEEP", 0, nullptr, false, ""},
    {"SOUNDEX", 0, nullptr, false, ""},
    {"SPACE", 0, nullptr, false, ""},
    {"SQRT", 0, nullptr, false, ""},
    {"SRID", 0, nullptr, false, ""},
    {"STARTPOINT", 0, nullptr, false, ""},
    {"STD", 0, nullptr, false, ""},
    {"STDDEV", 0, nullptr, false, ""},
    {"STDDEV_POP", 0, nullptr, false, ""},
    {"STDDEV_SAMP", 0, nullptr, false, ""},
    {"STR_TO_DATE", 0, nullptr, false, ""},
    {"STRCMP", 0, nullptr, false, ""},
    {"SUBSTR", 0, nullptr, false, ""},
    {"SUBSTRING", 0, nullptr, false, ""},
    {"SUBSTRING_INDEX", 0, nullptr, false, ""},
    {"SUBTIME", 0, nullptr, false, ""},
    {"SUM", 0, nullptr, false, ""},
    {"SYSDATE", 0, nullptr, false, ""},
    {"SYSTEM_USER", 0, nullptr, false, ""},
    {"TAN", 0, nullptr, false, ""},
    {"TIME_FORMAT", 0, nullptr, false, ""},
    {"TIME_TO_SEC", 0, nullptr, false, ""},
    {"TIMEDIFF", 0, nullptr, false, ""},
    {"TO_DAYS", 0, nullptr, false, ""},
    {"TOUCHES", 0, nullptr, false, ""},
    {"TRIM", 0, nullptr, false, ""},
    {"UCASE", 0, nullptr, false, ""},
    {"UNCOMPRESS", 0, nullptr, false, ""},
    {"UNCOMPRESSED_LENGTH", 0, nullptr, false, ""},
    {"UNHEX", 0, nullptr, false, ""},
    {"UNIQUE_USERS", 0, nullptr, false, ""},
    {"UNIX_TIMESTAMP", 0, nullptr, false, ""},
    {"UPPER", 0, nullptr, false, ""},
    {"UUID", 0, nullptr, false, ""},
    {"VARIANCE", 0, nullptr, false, ""},
    {"VAR_POP", 0, nullptr, false, ""},
    {"VAR_SAMP", 0, nullptr, false, ""},
    {"VERSION", 0, nullptr, false, ""},
    {"WEEKDAY", 0, nullptr, false, ""},
    {"WEEKOFYEAR", 0, nullptr, false, ""},
    {"WITHIN", 0, nullptr, false, ""},
    {"X", 0, nullptr, false, ""},
    {"Y", 0, nullptr, false, ""},
    {"YEARWEEK", 0, nullptr, false, ""},
    /* end sentinel */
    {(char *)nullptr, 0, nullptr, false, ""}};

static const char *load_default_groups[] = {"mysql", "client", nullptr};

#ifdef HAVE_READLINE
/*
 HIST_ENTRY is defined for libedit, but not for the real readline
 Need to redefine it for real readline to find it
*/
#if !defined(HAVE_HIST_ENTRY)
typedef struct _hist_entry {
  const char *line;
  const char *data;
} HIST_ENTRY;
#endif

extern "C" int add_history(const char *command); /* From readline directory */
extern "C" int read_history(const char *command);
extern "C" int write_history(const char *command);
extern "C" HIST_ENTRY *history_get(int num);
extern "C" int history_length;
static int not_in_history(const char *line);
static void initialize_readline(char *name);
#endif /* HAVE_READLINE */

static COMMANDS *find_command(char *name);
static COMMANDS *find_command(char cmd_name);
static bool add_line(String &buffer, char *line, size_t line_length,
                     char *in_string, bool *ml_comment, bool truncated);
static void remove_cntrl(String *buffer);
static void print_table_data(MYSQL_RES *result);
static void print_table_data_html(MYSQL_RES *result);
static void print_table_data_xml(MYSQL_RES *result);
static void print_tab_data(MYSQL_RES *result);
static void print_table_data_vertically(MYSQL_RES *result);
static void print_warnings(void);
static ulong start_timer(void);
static void end_timer(ulong start_time, char *buff);
static void mysql_end_timer(ulong start_time, char *buff);
static void nice_time(double sec, char *buff, bool part_second);
static void kill_query(const char *reason);
extern "C" void mysql_end(int sig);
extern "C" void handle_ctrlc_signal(int);
extern "C" void handle_quit_signal(int sig);
#if defined(HAVE_TERMIOS_H) && defined(GWINSZ_IN_SYS_IOCTL)
static void window_resize(int);
#endif

const char DELIMITER_NAME[] = "delimiter";
const uint DELIMITER_NAME_LEN = sizeof(DELIMITER_NAME) - 1;
inline bool is_delimiter_command(char *name, ulong len) {
  /*
    Delimiter command has a parameter, so the length of the whole command
    is larger than DELIMITER_NAME_LEN.  We don't care the parameter, so
    only name(first DELIMITER_NAME_LEN bytes) is checked.
  */
  return (len >= DELIMITER_NAME_LEN &&
          !my_strnncoll(
              charset_info, pointer_cast<uchar *>(name), DELIMITER_NAME_LEN,
              pointer_cast<const uchar *>(DELIMITER_NAME), DELIMITER_NAME_LEN));
}

/**
   Get the index of a command in the commands array.

   @param cmd_char    Short form command.

   @return int
     The index of the command is returned if it is found, else -1 is returned.
*/
inline int get_command_index(char cmd_char) {
  /*
    All client-specific commands are in the first part of commands array
    and have a function to implement it.
  */
  for (uint i = 0; *commands[i].func != nullptr; i++)
    if (commands[i].cmd_char == cmd_char) return i;
  return -1;
}

static int delimiter_index = -1;
static int charset_index = -1;
static bool real_binary_mode = false;

#ifdef _WIN32
BOOL windows_ctrl_handler(DWORD fdwCtrlType) {
  switch (fdwCtrlType) {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
      handle_ctrlc_signal(SIGINT);
      /* Indicate that signal has beed handled. */
      return true;
    case CTRL_CLOSE_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
      handle_quit_signal(SIGINT + 1);
  }
  /* Pass signal to the next control handler function. */
  return false;
}
#endif

int main(int argc, char *argv[]) {
  MY_INIT(argv[0]);
  DBUG_TRACE;
  DBUG_PROCESS(argv[0]);

  charset_index = get_command_index('C');
  delimiter_index = get_command_index('d');
  delimiter_str = delimiter;
  default_prompt = my_strdup(
      PSI_NOT_INSTRUMENTED,
      getenv("MYSQL_PS1") ? getenv("MYSQL_PS1") : "mysql> ", MYF(MY_WME));
  current_prompt = my_strdup(PSI_NOT_INSTRUMENTED, default_prompt, MYF(MY_WME));
  prompt_counter = 0;

  outfile[0] = 0;              // no (default) outfile
  my_stpcpy(pager, "stdout");  // the default, if --pager wasn't given
  {
    char *tmp = getenv("PAGER");
    if (tmp && strlen(tmp)) {
      default_pager_set = true;
      my_stpcpy(default_pager, tmp);
    }
  }
  if (!isatty(0) || !isatty(1)) {
    status.batch = true;
    opt_silent = 1;
    ignore_errors = false;
  } else
    status.add_to_history = true;
  status.exit_status = 1;

  {
    /*
     The file descriptor-layer may be out-of-sync with the file-number layer,
     so we make sure that "stdout" is really open.  If its file is closed then
     explicitly close the FD layer.
    */
    int stdout_fileno_copy;
    stdout_fileno_copy = dup(fileno(stdout)); /* Okay if fileno fails. */
    if (stdout_fileno_copy == -1) {
      fclose(stdout);
#ifdef LINUX_ALPINE
      // On Alpine linux we need to open a dummy file, so that the first
      // call to socket() does not get file number 1
      // If socket gets file number 1, then everything printed to stdout
      // will be sent back to the server over the socket connection.
      fopen("/dev/null", "r");
#endif
    } else
      close(stdout_fileno_copy); /* Clean up dup(). */
  }

#ifdef _WIN32
  /* Convert command line parameters from UTF16LE to UTF8MB4. */
  my_win_translate_command_line_args(&my_charset_utf8mb4_bin, &argc, &argv);
#endif

  my_getopt_use_args_separator = true;
  if (load_defaults("my", load_default_groups, &argc, &argv, &argv_alloc)) {
    my_end(0);
    return EXIT_FAILURE;
  }
  my_getopt_use_args_separator = false;

  get_current_os_user();
  get_current_os_sudouser();
  if (get_options(argc, (char **)argv)) {
    my_end(0);
    return EXIT_FAILURE;
  }
  if (status.batch && !status.line_buff &&
      !(status.line_buff = batch_readline_init(MAX_BATCH_BUFFER_SIZE, stdin))) {
    put_info(
        "Can't initialize batch_readline - may be the input source is "
        "a directory or a block device.",
        INFO_ERROR, 0);
    my_end(0);
    return EXIT_FAILURE;
  }
  if (!opt_binary_as_hex_set_explicitly && isatty(0) && isatty(1))
    opt_binhex = true;
  if (mysql_server_init(0, nullptr, nullptr)) {
    put_error(nullptr);
    my_end(0);
    return EXIT_FAILURE;
  }
  glob_buffer.mem_realloc((status.batch) ? batch_io_size : 512);
  completion_hash_init(&ht, 128);
  init_alloc_root(PSI_NOT_INSTRUMENTED, &hash_mem_root, 16384, 0);
  memset(&mysql, 0, sizeof(mysql));
  if (sql_connect(current_host, current_db, current_user, opt_password,
                  opt_silent)) {
    quick = true;  // Avoid history
    status.exit_status = 1;
    mysql_end(-1);
  }
  if (!status.batch) ignore_errors = true;  // Don't abort monitor

#ifndef _WIN32
  signal(SIGINT, handle_ctrlc_signal);  // Catch SIGINT to clean up
  signal(SIGQUIT, mysql_end);           // Catch SIGQUIT to clean up
  signal(SIGHUP, handle_quit_signal);   // Catch SIGHUP to clean up
#else
  SetConsoleCtrlHandler((PHANDLER_ROUTINE)windows_ctrl_handler, true);
#endif

#if defined(HAVE_TERMIOS_H) && defined(GWINSZ_IN_SYS_IOCTL)
  /* Readline will call this if it installs a handler */
  signal(SIGWINCH, window_resize);
  /* call the SIGWINCH handler to get the default term width */
  window_resize(0);
#endif

  put_info("Welcome to the MySQL monitor.  Commands end with ; or \\g.",
           INFO_INFO);
  snprintf(glob_buffer.ptr(), glob_buffer.alloced_length(),
           "Your MySQL connection id is %lu\nServer version: %s\n",
           mysql_thread_id(&mysql), server_version_string(&mysql));
  put_info(glob_buffer.ptr(), INFO_INFO);

  if (!status.batch) {
    // history ignore patterns are initialized to default values
    ignore_matcher.add_patterns(HI_DEFAULTS);

    /*
      Additional patterns may be supplied using either --histignore option or
      MYSQL_HISTIGNORE environment variable. If supplied, they'll get appended
      to the default patterns. In case both are specified, pattern(s) supplied
      using --histignore option will be used.
    */
    if (opt_histignore)
      ignore_matcher.add_patterns(opt_histignore);
    else if (getenv("MYSQL_HISTIGNORE"))
      ignore_matcher.add_patterns(getenv("MYSQL_HISTIGNORE"));

#ifdef HAVE_READLINE
    if (!quick) {
      initialize_readline(const_cast<char *>(my_progname));

      /* read-history from file, default ~/.mysql_history*/
      if (getenv("MYSQL_HISTFILE"))
        histfile = my_strdup(PSI_NOT_INSTRUMENTED, getenv("MYSQL_HISTFILE"),
                             MYF(MY_WME));
      else if (getenv("HOME")) {
        histfile = (char *)my_malloc(
            PSI_NOT_INSTRUMENTED,
            (uint)strlen(getenv("HOME")) + (uint)strlen("/.mysql_history") + 2,
            MYF(MY_WME));
        if (histfile) sprintf(histfile, "%s/.mysql_history", getenv("HOME"));
        char link_name[FN_REFLEN];
        if (my_readlink(link_name, histfile, 0) == 0 &&
            strncmp(link_name, "/dev/null", 10) == 0) {
          /* The .mysql_history file is a symlink to /dev/null, don't use it */
          my_free(histfile);
          histfile = nullptr;
        }
      }

      /* We used to suggest setting MYSQL_HISTFILE=/dev/null. */
      if (histfile && strncmp(histfile, "/dev/null", 10) == 0)
        histfile = nullptr;

      if (histfile && histfile[0]) {
        if (verbose) tee_fprintf(stdout, "Reading history-file %s\n", histfile);
        read_history(histfile);
        if (!(histfile_tmp =
                  (char *)my_malloc(PSI_NOT_INSTRUMENTED,
                                    (uint)strlen(histfile) + 5, MYF(MY_WME)))) {
          fprintf(stderr, "Couldn't allocate memory for temp histfile!\n");
          return EXIT_FAILURE;
        }
        sprintf(histfile_tmp, "%s.TMP", histfile);
      }
    }
#endif
  }

  uint protocol = MYSQL_PROTOCOL_DEFAULT;
  uint ssl_mode = 0;
  if (!mysql_get_option(&mysql, MYSQL_OPT_PROTOCOL, &protocol) &&
      !mysql_get_option(&mysql, MYSQL_OPT_SSL_MODE, &ssl_mode)) {
    if (protocol == MYSQL_PROTOCOL_SOCKET && ssl_mode >= SSL_MODE_REQUIRED)
      put_info(
          "You are enforcing ssl connection via unix socket. Please consider\n"
          "switching ssl off as it does not make connection via unix socket\n"
          "any more secure.",
          INFO_INFO);
  }
  status.exit_status = read_and_execute(!status.batch);
  if (opt_outfile) end_tee();
  mysql_end(0);
  return 0;  // Keep compiler happy
}

void mysql_end(int sig) {
#ifndef _WIN32
  /*
    Ingnoring SIGQUIT, SIGINT and SIGHUP signals when cleanup process starts.
    This will help in resolving the double free issues, which occures in case
    the signal handler function is started in between the clean up function.
  */
  signal(SIGQUIT, SIG_IGN);
  signal(SIGINT, SIG_IGN);
  signal(SIGHUP, SIG_IGN);
#endif

  mysql_close(&mysql);
#ifdef HAVE_READLINE
  if (!status.batch && !quick && histfile && histfile[0]) {
    /* write-history */
    if (verbose) tee_fprintf(stdout, "Writing history-file %s\n", histfile);
    if (!write_history(histfile_tmp))
      my_rename(histfile_tmp, histfile, MYF(MY_WME));
  }
  batch_readline_end(status.line_buff);
  completion_hash_free(&ht);
  free_root(&hash_mem_root, MYF(0));

  my_free(histfile);
  my_free(histfile_tmp);
#endif
  my_free(opt_histignore);

  my_free(current_os_user);
  my_free(current_os_sudouser);

  if (opt_syslog) my_closelog();

  if (sig >= 0) put_info(sig ? "Aborted" : "Bye", INFO_RESULT);
  glob_buffer.mem_free();
  old_buffer.mem_free();
  processed_prompt.mem_free();
  my_free(server_version);
  my_free(opt_password);
  my_free(opt_mysql_unix_port);
  my_free(current_db);
  my_free(current_host);
  my_free(current_user);
  my_free(full_username);
  my_free(part_username);
  my_free(default_prompt);
#if defined(_WIN32)
  my_free(shared_memory_base_name);
#endif
  my_free(current_prompt);
  mysql_server_end();
  my_end(my_end_arg);
#if defined(HAVE_OPENSSL)
  SSL_CTX_free((SSL_CTX *)ssl_context);
  if (ssl_session) SSL_SESSION_free((SSL_SESSION *)ssl_session);
#endif
  exit(status.exit_status);
}

/**
  SIGINT signal handler.

    This function handles SIGINT (Ctrl - C). It sends a 'KILL [QUERY]' command
    to the server if a query is currently executing. On Windows, 'Ctrl - Break'
    is treated alike.

  FIXME: POSIX allows only a very limited set of interactions from signal
  handlers, as the main thread could have nearly any state at the time of the
  signal and is suspended until the signal handler returns. In particular,
  only variables of type sig_atomic_t can be set and tested, and most C library
  functions (including malloc()) are banned. Thus, calling kill_query() here
  is forbidden and should not be done.
*/

void handle_ctrlc_signal(int) {
  sigint_received = true;

  /* Skip rest if --sigint-ignore is used. */
  if (opt_sigint_ignore) return;

  if (executing_query) kill_query("^C");
  /* else, do nothing, just terminate the current line (like /c command). */
  return;
}

/**
   Handler to perform a cleanup and quit the program.

     This function would send a 'KILL [QUERY]' command to the server if a
     query is currently executing and then it invokes mysql_thread_end()/
     mysql_end() in order to terminate the mysql client process.

  @param sig              Signal number
*/

void handle_quit_signal(int sig) {
  const char *reason = "Terminal close";

  if (!executing_query) {
    tee_fprintf(stdout, "%s -- exit!\n", reason);
    goto err;
  }

  kill_query(reason);

err:
#ifdef _WIN32
  /*
   When a signal is raised on Windows, the OS creates a new thread to
   handle the interrupt. Once that thread completes, the main thread
   continues running only to find that it's resources have already been
   free'd when the signal handler called mysql_end().
  */
  mysql_thread_end();
  return;
#else
  mysql_end(sig);
#endif
}

/* Send 'KILL QUERY' command to the server. */
static void kill_query(const char *reason) {
  char kill_buffer[40];
  MYSQL *kill_mysql = nullptr;

  kill_mysql = mysql_init(kill_mysql);
  init_connection_options(kill_mysql);

#ifdef HAVE_SETNS
  if (opt_network_namespace && set_network_namespace(opt_network_namespace)) {
    goto err;
  }
#endif

  if (!mysql_real_connect(kill_mysql, current_host, current_user, opt_password,
                          "", opt_mysql_port, opt_mysql_unix_port, 0)) {
#ifdef HAVE_SETNS
    if (opt_network_namespace) (void)restore_original_network_namespace();
#endif
    tee_fprintf(stdout,
                "%s -- Sorry, cannot connect to the server to kill "
                "query, giving up ...\n",
                reason);
    goto err;
  }

#ifdef HAVE_SETNS
  if (opt_network_namespace && restore_original_network_namespace()) goto err;
#endif

  interrupted_query = true;

  /* mysqld < 5 does not understand KILL QUERY, skip to KILL CONNECTION */
  sprintf(kill_buffer, "KILL %s%lu",
          (mysql_get_server_version(&mysql) < 50000) ? "" : "QUERY ",
          mysql_thread_id(&mysql));

  if (verbose)
    tee_fprintf(stdout, "%s -- sending \"%s\" to server ...\n", reason,
                kill_buffer);
  mysql_real_query(kill_mysql, kill_buffer,
                   static_cast<ulong>(strlen(kill_buffer)));
  tee_fprintf(stdout, "%s -- query aborted\n", reason);

err:
#ifdef HAVE_SETNS
  if (opt_network_namespace) (void)release_network_namespace_resources();
#endif
  mysql_close(kill_mysql);

  return;
}

#if defined(HAVE_TERMIOS_H) && defined(GWINSZ_IN_SYS_IOCTL)
void window_resize(int) {
  struct winsize window_size;

  if (ioctl(fileno(stdin), TIOCGWINSZ, &window_size) == 0)
    terminal_width = window_size.ws_col;
}
#endif

static struct my_option my_long_options[] = {
    {"help", '?', "Display this help and exit.", nullptr, nullptr, nullptr,
     GET_NO_ARG, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"help", 'I', "Synonym for -?", nullptr, nullptr, nullptr, GET_NO_ARG,
     NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"auto-rehash", OPT_AUTO_REHASH,
     "Enable automatic rehashing. One doesn't need to use 'rehash' to get "
     "table "
     "and field completion, but startup and reconnecting may take a longer "
     "time. "
     "Disable with --disable-auto-rehash.",
     &opt_rehash, &opt_rehash, nullptr, GET_BOOL, NO_ARG, 1, 0, 0, nullptr, 0,
     nullptr},
    {"no-auto-rehash", 'A',
     "No automatic rehashing. One has to use 'rehash' to get table and field "
     "completion. This gives a quicker start of mysql and disables rehashing "
     "on reconnect.",
     nullptr, nullptr, nullptr, GET_NO_ARG, NO_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
    {"auto-vertical-output", OPT_AUTO_VERTICAL_OUTPUT,
     "Automatically switch to vertical output mode if the result is wider "
     "than the terminal width.",
     &auto_vertical_output, &auto_vertical_output, nullptr, GET_BOOL, NO_ARG, 0,
     0, 0, nullptr, 0, nullptr},
    {"batch", 'B',
     "Don't use history file. Disable interactive behavior. (Enables "
     "--silent.)",
     nullptr, nullptr, nullptr, GET_NO_ARG, NO_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
    {"bind-address", 0, "IP address to bind to.", (uchar **)&opt_bind_addr,
     (uchar **)&opt_bind_addr, nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0, nullptr,
     0, nullptr},
    {"binary-as-hex", OPT_MYSQL_BINARY_AS_HEX,
     "Print binary data as hex. Enabled by default for interactive terminals.",
     &opt_binhex, &opt_binhex, nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
    {"character-sets-dir", OPT_CHARSETS_DIR,
     "Directory for character set files.", &charsets_dir, &charsets_dir,
     nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"column-type-info", OPT_COLUMN_TYPES, "Display column type information.",
     &column_types_flag, &column_types_flag, nullptr, GET_BOOL, NO_ARG, 0, 0, 0,
     nullptr, 0, nullptr},
    {"comments", 'c',
     "Preserve comments. Send comments to the server."
     " The default is --skip-comments (discard comments), enable with "
     "--comments.",
     &preserve_comments, &preserve_comments, nullptr, GET_BOOL, NO_ARG, 0, 0, 0,
     nullptr, 0, nullptr},
    {"compress", 'C', "Use compression in server/client protocol.",
     &opt_compress, &opt_compress, nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr,
     0, nullptr},
#ifdef DBUG_OFF
    {"debug", '#', "This is a non-debug version. Catch this and exit.", 0, 0, 0,
     GET_DISABLED, OPT_ARG, 0, 0, 0, 0, 0, 0},
    {"debug-check", OPT_DEBUG_CHECK,
     "This is a non-debug version. Catch this and exit.", 0, 0, 0, GET_DISABLED,
     NO_ARG, 0, 0, 0, 0, 0, 0},
    {"debug-info", 'T', "This is a non-debug version. Catch this and exit.", 0,
     0, 0, GET_DISABLED, NO_ARG, 0, 0, 0, 0, 0, 0},
#else
    {"debug", '#', "Output debug log.", &default_dbug_option,
     &default_dbug_option, nullptr, GET_STR, OPT_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
    {"debug-check", OPT_DEBUG_CHECK,
     "Check memory and open file usage at exit.", &debug_check_flag,
     &debug_check_flag, nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
    {"debug-info", 'T', "Print some debug info at exit.", &debug_info_flag,
     &debug_info_flag, nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
#endif
    {"database", 'D', "Database to use.", &current_db, &current_db, nullptr,
     GET_STR_ALLOC, REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"default-character-set", OPT_DEFAULT_CHARSET,
     "Set the default character set.", &default_charset, &default_charset,
     nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"delimiter", OPT_DELIMITER, "Delimiter to be used.", &delimiter_str,
     &delimiter_str, nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
    {"enable_cleartext_plugin", OPT_ENABLE_CLEARTEXT_PLUGIN,
     "Enable/disable the clear text authentication plugin.",
     &opt_enable_cleartext_plugin, &opt_enable_cleartext_plugin, nullptr,
     GET_BOOL, OPT_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"execute", 'e',
     "Execute command and quit. (Disables --force and history file.)", nullptr,
     nullptr, nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"vertical", 'E', "Print the output of a query (rows) vertically.",
     &vertical, &vertical, nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
    {"force", 'f', "Continue even if we get an SQL error.", &ignore_errors,
     &ignore_errors, nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"histignore", OPT_HISTIGNORE,
     "A colon-separated list of patterns to "
     "keep statements from getting logged into syslog and mysql history.",
     &opt_histignore, &opt_histignore, nullptr, GET_STR_ALLOC, REQUIRED_ARG, 0,
     0, 0, nullptr, 0, nullptr},
    {"named-commands", 'G',
     "Enable named commands. Named commands mean this program's internal "
     "commands; see mysql> help . When enabled, the named commands can be "
     "used from any line of the query, otherwise only from the first line, "
     "before an enter. Disable with --disable-named-commands. This option "
     "is disabled by default.",
     &named_cmds, &named_cmds, nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
    {"ignore-spaces", 'i', "Ignore space after function names.", &ignore_spaces,
     &ignore_spaces, nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"init-command", OPT_INIT_COMMAND,
     "SQL Command to execute when connecting to MySQL server. Will "
     "automatically be re-executed when reconnecting.",
     &opt_init_command, &opt_init_command, nullptr, GET_STR, REQUIRED_ARG, 0, 0,
     0, nullptr, 0, nullptr},
    {"local-infile", OPT_LOCAL_INFILE, "Enable/disable LOAD DATA LOCAL INFILE.",
     &opt_local_infile, &opt_local_infile, nullptr, GET_BOOL, OPT_ARG, 0, 0, 0,
     nullptr, 0, nullptr},
    {"no-beep", 'b', "Turn off beep on error.", &opt_nobeep, &opt_nobeep,
     nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"host", 'h', "Connect to host.", &current_host, &current_host, nullptr,
     GET_STR_ALLOC, REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"html", 'H', "Produce HTML output.", &opt_html, &opt_html, nullptr,
     GET_BOOL, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"xml", 'X', "Produce XML output.", &opt_xml, &opt_xml, nullptr, GET_BOOL,
     NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"line-numbers", OPT_LINE_NUMBERS, "Write line numbers for errors.",
     &line_numbers, &line_numbers, nullptr, GET_BOOL, NO_ARG, 1, 0, 0, nullptr,
     0, nullptr},
    {"skip-line-numbers", 'L', "Don't write line number for errors.", nullptr,
     nullptr, nullptr, GET_NO_ARG, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"unbuffered", 'n', "Flush buffer after each query.", &unbuffered,
     &unbuffered, nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"column-names", OPT_COLUMN_NAMES, "Write column names in results.",
     &column_names, &column_names, nullptr, GET_BOOL, NO_ARG, 1, 0, 0, nullptr,
     0, nullptr},
    {"skip-column-names", 'N', "Don't write column names in results.", nullptr,
     nullptr, nullptr, GET_NO_ARG, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"sigint-ignore", OPT_SIGINT_IGNORE, "Ignore SIGINT (CTRL-C).",
     &opt_sigint_ignore, &opt_sigint_ignore, nullptr, GET_BOOL, NO_ARG, 0, 0, 0,
     nullptr, 0, nullptr},
    {"one-database", 'o',
     "Ignore statements except those that occur while the default "
     "database is the one named at the command line.",
     nullptr, nullptr, nullptr, GET_NO_ARG, NO_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
#ifdef USE_POPEN
    {"pager", OPT_PAGER,
     "Pager to use to display results. If you don't supply an option, the "
     "default pager is taken from your ENV variable PAGER. Valid pagers are "
     "less, more, cat [> filename], etc. See interactive help (\\h) also. "
     "This option does not work in batch mode. Disable with --disable-pager. "
     "This option is disabled by default.",
     nullptr, nullptr, nullptr, GET_STR, OPT_ARG, 0, 0, 0, nullptr, 0, nullptr},
#endif
    {"password", 'p',
     "Password to use when connecting to server. If password is not given it's "
     "asked from the tty.",
     nullptr, nullptr, nullptr, GET_PASSWORD, OPT_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
#ifdef _WIN32
    {"pipe", 'W', "Use named pipes to connect to server.", 0, 0, 0, GET_NO_ARG,
     NO_ARG, 0, 0, 0, 0, 0, 0},
#endif
    {"port", 'P',
     "Port number to use for connection or 0 for default to, in "
     "order of preference, my.cnf, $MYSQL_TCP_PORT, "
#if MYSQL_PORT_DEFAULT == 0
     "/etc/services, "
#endif
     "built-in default (" STRINGIFY_ARG(MYSQL_PORT) ").",
     &opt_mysql_port, &opt_mysql_port, nullptr, GET_UINT, REQUIRED_ARG, 0, 0, 0,
     nullptr, 0, nullptr},
    {"prompt", OPT_PROMPT, "Set the mysql prompt to this value.",
     &current_prompt, &current_prompt, nullptr, GET_STR_ALLOC, REQUIRED_ARG, 0,
     0, 0, nullptr, 0, nullptr},
    {"protocol", OPT_MYSQL_PROTOCOL,
     "The protocol to use for connection (tcp, socket, pipe, memory).", nullptr,
     nullptr, nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"quick", 'q',
     "Don't cache result, print it row by row. This may slow down the server "
     "if the output is suspended. Doesn't use history file.",
     &quick, &quick, nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"raw", 'r', "Write fields without conversion. Used with --batch.",
     &opt_raw_data, &opt_raw_data, nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr,
     0, nullptr},
    {"reconnect", OPT_RECONNECT,
     "Reconnect if the connection is lost. Disable "
     "with --disable-reconnect. This option is enabled by default.",
     &opt_reconnect, &opt_reconnect, nullptr, GET_BOOL, NO_ARG, 1, 0, 0,
     nullptr, 0, nullptr},
    {"checksum", OPT_CHECKSUM,
     "Use query and resultset checksums to verify the integrity of the query "
     "and resultset in transit. This is disabled by default.",
     &opt_checksum, &opt_checksum, 0, GET_BOOL, NO_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
    {"silent", 's',
     "Be more silent. Print results with a tab as separator, "
     "each row on new line.",
     nullptr, nullptr, nullptr, GET_NO_ARG, NO_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
#if defined(_WIN32)
    {"shared-memory-base-name", OPT_SHARED_MEMORY_BASE_NAME,
     "Base name of shared memory.", &shared_memory_base_name,
     &shared_memory_base_name, 0, GET_STR_ALLOC, REQUIRED_ARG, 0, 0, 0, 0, 0,
     0},
#endif
    {"socket", 'S', "The socket file to use for connection.",
     &opt_mysql_unix_port, &opt_mysql_unix_port, nullptr, GET_STR_ALLOC,
     REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
#include "caching_sha2_passwordopt-longopts.h"
#include "sslopt-longopts.h"

    {"table", 't', "Output in table format.", &output_tables, &output_tables,
     nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"tee", OPT_TEE,
     "Append everything into outfile. See interactive help (\\h) also. "
     "Does not work in batch mode. Disable with --disable-tee. "
     "This option is disabled by default.",
     nullptr, nullptr, nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
    {"user", 'u', "User for login if not current user.", &current_user,
     &current_user, nullptr, GET_STR_ALLOC, REQUIRED_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
    {"safe-updates", 'U', "Only allow UPDATE and DELETE that uses keys.",
     &safe_updates, &safe_updates, nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr,
     0, nullptr},
    {"i-am-a-dummy", 'U', "Synonym for option --safe-updates, -U.",
     &safe_updates, &safe_updates, nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr,
     0, nullptr},
    {"verbose", 'v', "Write more. (-v -v -v gives the table output format).",
     nullptr, nullptr, nullptr, GET_NO_ARG, NO_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
    {"version", 'V', "Output version information and exit.", nullptr, nullptr,
     nullptr, GET_NO_ARG, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"wait", 'w', "Wait and retry if connection is down.", nullptr, nullptr,
     nullptr, GET_NO_ARG, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"connect_timeout", OPT_CONNECT_TIMEOUT,
     "Number of seconds before connection timeout.", &opt_connect_timeout,
     &opt_connect_timeout, nullptr, GET_ULONG, REQUIRED_ARG, 0, 0, 3600 * 12,
     nullptr, 0, nullptr},
    {"max_allowed_packet", OPT_MAX_ALLOWED_PACKET,
     "The maximum packet length to send to or receive from server.",
     &opt_max_allowed_packet, &opt_max_allowed_packet, nullptr, GET_ULONG,
     REQUIRED_ARG, 16 * 1024L * 1024L, 4096,
     (longlong)2 * 1024L * 1024L * 1024L, nullptr, 1024, nullptr},
    {"net_buffer_length", OPT_NET_BUFFER_LENGTH,
     "The buffer size for TCP/IP and socket communication.",
     &opt_net_buffer_length, &opt_net_buffer_length, nullptr, GET_ULONG,
     REQUIRED_ARG, 16384, 1024, 512 * 1024 * 1024L, nullptr, 1024, nullptr},
    {"select_limit", OPT_SELECT_LIMIT,
     "Automatic limit for SELECT when using --safe-updates.", &select_limit,
     &select_limit, nullptr, GET_ULONG, REQUIRED_ARG, 1000L, 1, ULONG_MAX,
     nullptr, 1, nullptr},
    {"max_join_size", OPT_MAX_JOIN_SIZE,
     "Automatic limit for rows in a join when using --safe-updates.",
     &max_join_size, &max_join_size, nullptr, GET_ULONG, REQUIRED_ARG, 1000000L,
     1, ULONG_MAX, nullptr, 1, nullptr},
    {"show-warnings", OPT_SHOW_WARNINGS, "Show warnings after every statement.",
     &show_warnings, &show_warnings, nullptr, GET_BOOL, NO_ARG, 0, 0, 0,
     nullptr, 0, nullptr},
    {"syslog", 'j',
     "Log filtered interactive commands to syslog. Filtering of "
     "commands depends on the patterns supplied via histignore option besides "
     "the default patterns.",
     nullptr, nullptr, nullptr, GET_NO_ARG, NO_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
    {"plugin_dir", OPT_PLUGIN_DIR, "Directory for client-side plugins.",
     &opt_plugin_dir, &opt_plugin_dir, nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0,
     nullptr, 0, nullptr},
    {"default_auth", OPT_DEFAULT_AUTH,
     "Default authentication client-side plugin to use.", &opt_default_auth,
     &opt_default_auth, nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
    {"binary-mode", OPT_BINARY_MODE,
     "By default, ASCII '\\0' is disallowed and '\\r\\n' is translated to "
     "'\\n'. "
     "This switch turns off both features, and also turns off parsing of all "
     "client"
     "commands except \\C and DELIMITER, in non-interactive mode (for input "
     "piped to mysql or loaded using the 'source' command). This is necessary "
     "when processing output from mysqlbinlog that may contain blobs.",
     &opt_binary_mode, &opt_binary_mode, nullptr, GET_BOOL, NO_ARG, 0, 0, 0,
     nullptr, 0, nullptr},
    {"connect-expired-password", 0,
     "Notify the server that this client is prepared to handle expired "
     "password sandbox mode.",
     &opt_connect_expired_password, &opt_connect_expired_password, nullptr,
     GET_BOOL, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
#ifndef DBUG_OFF
    {"build-completion-hash", 0,
     "Build completion hash even when it is in batch mode. It is used for "
     "test purpose, so it is just built when DEBUG is on.",
     &opt_build_completion_hash, &opt_build_completion_hash, nullptr, GET_BOOL,
     NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
#endif
#ifdef HAVE_SETNS
    {"network-namespace", 0,
     "Network namespace to use for connection via tcp with a server.",
     &opt_network_namespace, &opt_network_namespace, nullptr, GET_STR,
     REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
#endif
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

static void usage(int version) {
  print_version();

  if (version) return;
  puts(ORACLE_WELCOME_COPYRIGHT_NOTICE("2000"));
  printf("Usage: %s [OPTIONS] [database]\n", my_progname);
  my_print_help(my_long_options);
  print_defaults("my", load_default_groups);
  my_print_variables(my_long_options);
}

bool get_one_option(int optid,
                    const struct my_option *opt MY_ATTRIBUTE((unused)),
                    char *argument) {
  switch (optid) {
    case OPT_CHARSETS_DIR:
      strmake(mysql_charsets_dir, argument, sizeof(mysql_charsets_dir) - 1);
      charsets_dir = mysql_charsets_dir;
      break;
    case OPT_DELIMITER:
      if (argument == disabled_my_option) {
        my_stpcpy(delimiter, DEFAULT_DELIMITER);
      } else {
        /* Check that delimiter does not contain a backslash */
        if (!strstr(argument, "\\")) {
          strmake(delimiter, argument, sizeof(delimiter) - 1);
        } else {
          put_info("DELIMITER cannot contain a backslash character",
                   INFO_ERROR);
          return false;
        }
      }
      delimiter_length = (uint)strlen(delimiter);
      delimiter_str = delimiter;
      break;
    case OPT_LOCAL_INFILE:
      using_opt_local_infile = true;
      break;
    case OPT_ENABLE_CLEARTEXT_PLUGIN:
      using_opt_enable_cleartext_plugin = true;
      break;
    case OPT_TEE:
      if (argument == disabled_my_option) {
        if (opt_outfile) end_tee();
      } else
        init_tee(argument);
      break;
    case OPT_PAGER:
      if (argument == disabled_my_option)
        opt_nopager = true;
      else {
        opt_nopager = false;
        if (argument && strlen(argument)) {
          default_pager_set = true;
          strmake(pager, argument, sizeof(pager) - 1);
          my_stpcpy(default_pager, pager);
        } else if (default_pager_set)
          my_stpcpy(pager, default_pager);
        else
          opt_nopager = true;
      }
      break;
    case OPT_MYSQL_PROTOCOL:
      opt_protocol =
          find_type_or_exit(argument, &sql_protocol_typelib, opt->name);
      break;
    case 'A':
      opt_rehash = false;
      break;
    case 'N':
      column_names = false;
      break;
    case 'e':
      status.batch = true;
      status.add_to_history = false;
      if (!status.line_buff)
        ignore_errors = false;  // do it for the first -e only
      if (!(status.line_buff =
                batch_readline_command(status.line_buff, argument)))
        return true;
      break;
    case 'j':
      if (my_openlog("MysqlClient", 0, LOG_USER)) {
        /* error */
        put_info(strerror(errno), INFO_ERROR, errno);
        return true;
      }
      opt_syslog = true;
      break;
    case 'o':
      if (argument == disabled_my_option)
        one_database = false;
      else
        one_database = skip_updates = true;
      break;
    case 'p':
      if (argument == disabled_my_option) {
        // Don't require password
        static char empty_password[] = {'\0'};
        DBUG_ASSERT(empty_password[0] ==
                    '\0');  // Check that it has not been overwritten
        argument = empty_password;
      }
      if (argument) {
        char *start = argument;
        my_free(opt_password);
        opt_password = my_strdup(PSI_NOT_INSTRUMENTED, argument, MYF(MY_FAE));
        while (*argument) *argument++ = 'x';  // Destroy argument
        if (*start) start[1] = 0;
        tty_password = false;
      } else
        tty_password = true;
      break;
    case '#':
      DBUG_PUSH(argument ? argument : default_dbug_option);
      debug_info_flag = true;
      break;
    case 's':
      if (argument == disabled_my_option)
        opt_silent = 0;
      else
        opt_silent++;
      break;
    case 'v':
      if (argument == disabled_my_option)
        verbose = 0;
      else
        verbose++;
      break;
    case 'B':
      status.batch = true;
      status.add_to_history = false;
      opt_silent = std::max(opt_silent, 1U);  // more silent
      break;
    case 'W':
#ifdef _WIN32
      opt_protocol = MYSQL_PROTOCOL_PIPE;
#endif
      break;
#include "sslopt-case.h"

    case 'V':
      usage(1);
      exit(0);
    case 'I':
    case '?':
      usage(0);
      exit(0);
    case OPT_MYSQL_BINARY_AS_HEX:
      opt_binhex = (argument != disabled_my_option);
      opt_binary_as_hex_set_explicitly = true;
      break;
  }
  return false;
}

static int get_options(int argc, char **argv) {
  char *tmp, *pagpoint;
  int ho_error;

  tmp = (char *)getenv("MYSQL_HOST");
  if (tmp) current_host = my_strdup(PSI_NOT_INSTRUMENTED, tmp, MYF(MY_WME));

  pagpoint = getenv("PAGER");
  if (!((char *)(pagpoint))) {
    my_stpcpy(pager, "stdout");
    opt_nopager = true;
  } else
    my_stpcpy(pager, pagpoint);
  my_stpcpy(default_pager, pager);

  if (mysql_get_option(nullptr, MYSQL_OPT_MAX_ALLOWED_PACKET,
                       &opt_max_allowed_packet) ||
      mysql_get_option(nullptr, MYSQL_OPT_NET_BUFFER_LENGTH,
                       &opt_max_allowed_packet)) {
    exit(1);
  }

  if ((ho_error =
           handle_options(&argc, &argv, my_long_options, get_one_option)))
    exit(ho_error);

  if (mysql_options(nullptr, MYSQL_OPT_MAX_ALLOWED_PACKET,
                    &opt_max_allowed_packet) ||
      mysql_options(nullptr, MYSQL_OPT_NET_BUFFER_LENGTH,
                    &opt_net_buffer_length)) {
    exit(1);
  }

  if (status.batch) /* disable pager and outfile in this case */
  {
    my_stpcpy(default_pager, "stdout");
    my_stpcpy(pager, "stdout");
    opt_nopager = true;
    default_pager_set = false;
    opt_outfile = false;
    opt_reconnect = false;
    connect_flag = 0; /* Not in interactive mode */
  }

  if (argc > 1) {
    usage(0);
    exit(1);
  }
  if (argc == 1) {
    skip_updates = false;
    my_free(current_db);
    current_db = my_strdup(PSI_NOT_INSTRUMENTED, *argv, MYF(MY_WME));
  }
  if (tty_password) opt_password = get_tty_password(NullS);
  if (debug_info_flag) my_end_arg = MY_CHECK_ERROR | MY_GIVE_INFO;
  if (debug_check_flag) my_end_arg = MY_CHECK_ERROR;

  if (ignore_spaces) connect_flag |= CLIENT_IGNORE_SPACE;

  return (0);
}

static int read_and_execute(bool interactive) {
#if defined(_WIN32)
  String tmpbuf;
  String buffer;
#endif

  /*
    line can be allocated by:
    - batch_readline. Use my_free()
    - my_win_console_readline. Do not free, see tmpbuf.
    - readline. Use free()
  */
  char *line = nullptr;
  char in_string = 0;
  ulong line_number = 0;
  bool ml_comment = false;
  COMMANDS *com;
  size_t line_length = 0;
  status.exit_status = 1;

  real_binary_mode = !interactive && opt_binary_mode;
  for (;;) {
    /* Reset as SIGINT has already got handled. */
    sigint_received = false;

    if (!interactive) {
      /*
        batch_readline can return 0 on EOF or error.
        In that case, we need to double check that we have a valid
        line before actually setting line_length to read_length.
        */
      line = batch_readline(status.line_buff, real_binary_mode);
      if (line) {
        line_length = status.line_buff->read_length;

        /*
          ASCII 0x00 is not allowed appearing in queries if it is not in binary
          mode.
        */
        if (!real_binary_mode && strlen(line) != line_length) {
          status.exit_status = 1;
          String msg;
          msg.append(
              "ASCII '\\0' appeared in the statement, but this is not "
              "allowed unless option --binary-mode is enabled and mysql is "
              "run in non-interactive mode. Set --binary-mode to 1 if ASCII "
              "'\\0' is expected. Query: '");
          msg.append(glob_buffer);
          msg.append(line);
          msg.append("'.");
          put_info(msg.c_ptr(), INFO_ERROR);
          break;
        }

        /*
          Skip UTF8 Byte Order Marker (BOM) 0xEFBBBF.
          Editors like "notepad" put this marker in
          the very beginning of a text file when
          you save the file using "Unicode UTF-8" format.
        */
        if (!line_number && (uchar)line[0] == 0xEF && (uchar)line[1] == 0xBB &&
            (uchar)line[2] == 0xBF) {
          line += 3;
          // decrease the line length accordingly to the 3 bytes chopped
          line_length -= 3;
        }
      }
      line_number++;
      if (!glob_buffer.length()) status.query_start_line = line_number;
    } else {
      const char *prompt =
          (ml_comment
               ? "   /*> "
               : glob_buffer.is_empty()
                     ? construct_prompt()
                     : !in_string
                           ? "    -> "
                           : in_string == '\''
                                 ? "    '> "
                                 : (in_string == '`' ? "    `> " : "    \"> "));
      if (opt_outfile && glob_buffer.is_empty()) fflush(OUTFILE);

#if defined(_WIN32)
      size_t nread;
      tee_fputs(prompt, stdout);
      if (!tmpbuf.is_alloced()) tmpbuf.alloc(65535);
      tmpbuf.length(0);
      buffer.length(0);
      line = my_win_console_readline(charset_info, (char *)tmpbuf.ptr(),
                                     tmpbuf.alloced_length(), &nread);
      if (line && (nread == 0)) {
        tee_puts("^C", stdout);
        reset_prompt(&in_string, &ml_comment);
        continue;
      } else if (*line == 0x1A) /* (Ctrl + Z) */
        break;
#else
      if (opt_outfile) fputs(prompt, OUTFILE);
      /*
        free the previous entered line.
      */
      if (line) free(line);
      line = readline(prompt);

      if (sigint_received) {
        sigint_received = false;
        tee_puts("^C", stdout);
        reset_prompt(&in_string, &ml_comment);
        continue;
      }
#endif /* defined(_WIN32) */
      /*
        When Ctrl+d or Ctrl+z is pressed, the line may be NULL on some OS
        which may cause coredump.
      */
      if (opt_outfile && line) fprintf(OUTFILE, "%s\n", line);

      line_length = line ? strlen(line) : 0;
    }
    // End of file or system error
    if (!line) {
      if (status.line_buff && status.line_buff->error)
        status.exit_status = 1;
      else
        status.exit_status = 0;
      break;
    }

    /*
      Check if line is a mysql command line
      (We want to allow help, print and clear anywhere at line start
    */
    if ((named_cmds || glob_buffer.is_empty()) && !ml_comment && !in_string &&
        (com = find_command(line))) {
      if ((*com->func)(&glob_buffer, line) > 0) {
        // lets log the exit/quit command.
        if (interactive && status.add_to_history && com->cmd_char == 'q')
          add_filtered_history(line);
        break;
      }
      if (glob_buffer.is_empty())  // If buffer was emptied
        in_string = 0;
      if (interactive && status.add_to_history) add_filtered_history(line);
      continue;
    }
    if (add_line(glob_buffer, line, line_length, &in_string, &ml_comment,
                 status.line_buff ? status.line_buff->truncated : false))
      break;
  }
  /* if in batch mode, send last query even if it doesn't end with \g or go */

  if (!interactive && !status.exit_status) {
    remove_cntrl(&glob_buffer);
    if (!glob_buffer.is_empty()) {
      status.exit_status = 1;
      if (com_go(&glob_buffer, line) <= 0) status.exit_status = 0;
    }
  }

#if defined(_WIN32)
  buffer.mem_free();
  tmpbuf.mem_free();
#else
  if (interactive)
    /*
      free the last entered line.
    */
    free(line);
#endif

  /*
    If the function is called by 'source' command, it will return to interactive
    mode, so real_binary_mode should be false. Otherwise, it will exit the
    program, it is safe to set real_binary_mode to false.
  */
  real_binary_mode = false;
  return status.exit_status;
}

static inline void reset_prompt(char *in_string, bool *ml_comment) {
  glob_buffer.length(0);
  *ml_comment = false;
  *in_string = 0;
}

/**
   It checks if the input is a short form command. It returns the command's
   pointer if a command is found, else return NULL. Note that if binary-mode
   is set, then only @\C is searched for.

   @param cmd_char    A character of one byte.

   @return
     the command's pointer or NULL.
*/
static COMMANDS *find_command(char cmd_char) {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("cmd_char: %d", cmd_char));

  int index = -1;

  /*
    In binary-mode, we disallow all mysql commands except '\C'
    and DELIMITER.
  */
  if (real_binary_mode) {
    if (cmd_char == 'C') index = charset_index;
  } else
    index = get_command_index(cmd_char);

  if (index >= 0) {
    DBUG_PRINT("exit", ("found command: %s", commands[index].name));
    return &commands[index];
  } else
    return (COMMANDS *)nullptr;
}

/**
   It checks if the input is a long form command. It returns the command's
   pointer if a command is found, else return NULL. Note that if binary-mode
   is set, then only DELIMITER is searched for.

   @param name    A string.
   @return
     the command's pointer or NULL.
*/
static COMMANDS *find_command(char *name) {
  uint len;
  char *end;
  DBUG_TRACE;

  DBUG_ASSERT(name != nullptr);
  DBUG_PRINT("enter", ("name: '%s'", name));

  while (my_isspace(charset_info, *name)) name++;
  /*
    If there is an \\g in the row or if the row has a delimiter but
    this is not a delimiter command, let add_line() take care of
    parsing the row and calling find_command().
  */
  if ((!real_binary_mode && strstr(name, "\\g")) ||
      (strstr(name, delimiter) &&
       !is_delimiter_command(name, DELIMITER_NAME_LEN)))
    return (COMMANDS *)nullptr;

  if ((end = strcont(name, " \t"))) {
    len = (uint)(end - name);
    while (my_isspace(charset_info, *end)) end++;
    if (!*end) end = nullptr;  // no arguments to function
  } else
    len = (uint)strlen(name);

  int index = -1;
  if (real_binary_mode) {
    if (is_delimiter_command(name, len)) index = delimiter_index;
  } else {
    /*
      All commands are in the first part of commands array and have a function
      to implement it.
    */
    for (uint i = 0; commands[i].func; i++) {
      if (!my_strnncoll(&my_charset_latin1, (uchar *)name, len,
                        pointer_cast<const uchar *>(commands[i].name), len) &&
          (commands[i].name[len] == '\0') &&
          (!end || commands[i].takes_params)) {
        index = i;
        break;
      }
    }
  }

  if (index >= 0) {
    DBUG_PRINT("exit", ("found command: %s", commands[index].name));
    return &commands[index];
  }
  return (COMMANDS *)nullptr;
}

static bool add_line(String &buffer, char *line, size_t line_length,
                     char *in_string, bool *ml_comment, bool truncated) {
  uchar inchar;
  char buff[80], *pos, *out;
  COMMANDS *com;
  bool need_space = false;
  enum { SSC_NONE = 0, SSC_CONDITIONAL, SSC_HINT } ss_comment = SSC_NONE;
  DBUG_TRACE;

  if (!line[0] && buffer.is_empty()) return false;

  if (status.add_to_history && line[0]) add_filtered_history(line);

  char *end_of_line = line + line_length;

  for (pos = out = line; pos < end_of_line; pos++) {
    inchar = (uchar)*pos;
    if (!preserve_comments) {
      // Skip spaces at the beginning of a statement
      if (my_isspace(charset_info, inchar) && (out == line) &&
          buffer.is_empty())
        continue;
    }
    // Accept multi-byte characters as-is
    int length;
    if (use_mb(charset_info) &&
        (length = my_ismbchar(charset_info, pos, end_of_line))) {
      if (!*ml_comment || preserve_comments) {
        while (length--) *out++ = *pos++;
        pos--;
      } else
        pos += length - 1;
      continue;
    }
    if (!*ml_comment && inchar == '\\' &&
        !(*in_string &&
          (mysql.server_status & SERVER_STATUS_NO_BACKSLASH_ESCAPES))) {
      // Found possbile one character command like \c

      if (!(inchar = (uchar) * ++pos)) break;  // readline adds one '\'
      if (*in_string || inchar == 'N')         // \N is short for NULL
      {                                        // Don't allow commands in string
        *out++ = '\\';
        if ((inchar == '`') && (*in_string == inchar))
          pos--;
        else
          *out++ = (char)inchar;
        continue;
      }
      if ((com = find_command((char)inchar))) {
        // Flush previously accepted characters
        if (out != line) {
          buffer.append(line, (uint)(out - line));
          out = line;
        }

        if ((*com->func)(&buffer, pos - 1) > 0) return true;  // Quit
        if (com->takes_params) {
          if (ss_comment) {
            /*
              If a client-side macro appears inside a server-side comment,
              discard all characters in the comment after the macro (that is,
              until the end of the comment rather than the next delimiter)
            */
            for (pos++; *pos && (*pos != '*' || *(pos + 1) != '/'); pos++)
              ;
            pos--;
          } else {
            for (pos++; *pos && (*pos != *delimiter ||
                                 !is_prefix(pos + 1, delimiter + 1));
                 pos++)
              ;  // Remove parameters
            if (!*pos)
              pos--;
            else
              pos += delimiter_length - 1;  // Point at last delim char
          }
        }
      } else {
        sprintf(buff, "Unknown command '\\%c'.", inchar);
        if (put_info(buff, INFO_ERROR) > 0) return true;
        *out++ = '\\';
        *out++ = (char)inchar;
        continue;
      }
    } else if (!*ml_comment && !*in_string && ss_comment != SSC_HINT &&
               is_prefix(pos, delimiter)) {
      // Found a statement. Continue parsing after the delimiter
      pos += delimiter_length;

      if (preserve_comments) {
        while (my_isspace(charset_info, *pos)) *out++ = *pos++;
      }
      // Flush previously accepted characters
      if (out != line) {
        buffer.append(line, (uint32)(out - line));
        out = line;
      }

      if (preserve_comments &&
          ((*pos == '#') || ((*pos == '-') && (pos[1] == '-') &&
                             my_isspace(charset_info, pos[2])))) {
        // Add trailing single line comments to this statement
        buffer.append(pos);
        pos += strlen(pos);
      }

      pos--;

      if ((com = find_command(buffer.c_ptr()))) {
        if ((*com->func)(&buffer, buffer.c_ptr()) > 0) return true;  // Quit
      } else {
        if (com_go(&buffer, nullptr) > 0)  // < 0 is not fatal
          return true;
      }
      buffer.length(0);
    } else if (!*ml_comment &&
               (!*in_string &&
                (inchar == '#' ||
                 (inchar == '-' && pos[1] == '-' &&
                  /*
                    The third byte is either whitespace or is the
                    end of the line -- which would occur only
                    because of the user sending newline -- which is
                    itself whitespace and should also match.
                  */
                  (my_isspace(charset_info, pos[2]) || !pos[2]))))) {
      // Flush previously accepted characters
      if (out != line) {
        buffer.append(line, (uint32)(out - line));
        out = line;
      }

      // comment to end of line
      if (preserve_comments) {
        bool started_with_nothing = !buffer.length();

        buffer.append(pos);

        /*
          A single-line comment by itself gets sent immediately so that
          client commands (delimiter, status, etc) will be interpreted on
          the next line.
        */
        if (started_with_nothing) {
          if (com_go(&buffer, nullptr) > 0)  // < 0 is not fatal
            return true;
          buffer.length(0);
        }
      }

      break;
    } else if (!*in_string && inchar == '/' && pos[1] == '*' && pos[2] != '!' &&
               pos[2] != '+' && ss_comment != SSC_HINT) {
      if (preserve_comments) {
        *out++ = *pos++;  // copy '/'
        *out++ = *pos;    // copy '*'
      } else
        pos++;
      *ml_comment = true;
      if (out != line) {
        buffer.append(line, (uint)(out - line));
        out = line;
      }
    } else if (*ml_comment && !ss_comment && inchar == '*' &&
               *(pos + 1) == '/') {
      if (preserve_comments) {
        *out++ = *pos++;  // copy '*'
        *out++ = *pos;    // copy '/'
      } else
        pos++;
      *ml_comment = false;
      if (out != line) {
        buffer.append(line, (uint32)(out - line));
        out = line;
      }
      // Consumed a 2 chars or more, and will add 1 at most,
      // so using the 'line' buffer to edit data in place is ok.
      need_space = true;
    } else {  // Add found char to buffer
      if (!*in_string && inchar == '/' && pos[1] == '*') {
        if (pos[2] == '!')
          ss_comment = SSC_CONDITIONAL;
        else if (pos[2] == '+')
          ss_comment = SSC_HINT;
      } else if (!*in_string && ss_comment && inchar == '*' &&
                 *(pos + 1) == '/')
        ss_comment = SSC_NONE;
      if (inchar == *in_string)
        *in_string = 0;
      else if (!*ml_comment && !*in_string && ss_comment != SSC_HINT &&
               (inchar == '\'' || inchar == '"' || inchar == '`'))
        *in_string = (char)inchar;
      if (!*ml_comment || preserve_comments) {
        if (need_space && !my_isspace(charset_info, (char)inchar)) *out++ = ' ';
        need_space = false;
        *out++ = (char)inchar;
      }
    }
  }
  if (out != line || !buffer.is_empty()) {
    uint length = (uint)(out - line);

    if (!truncated &&
        (!is_delimiter_command(line, length) || (*in_string || *ml_comment))) {
      /*
        Don't add a new line in case there's a DELIMITER command to be
        added to the glob buffer (e.g. on processing a line like
        "<command>;DELIMITER <non-eof>") : similar to how a new line is
        not added in the case when the DELIMITER is the first command
        entered with an empty glob buffer. However, if the delimiter is
        part of a string or a comment, the new line should be added. (e.g.
        SELECT '\ndelimiter\n';\n)
      */
      *out++ = '\n';
      length++;
    }
    if (buffer.length() + length >= buffer.alloced_length())
      buffer.mem_realloc(buffer.length() + length + batch_io_size);
    if ((!*ml_comment || preserve_comments) && buffer.append(line, length))
      return true;
  }
  return false;
}

/*****************************************************************
            Interface to Readline Completion
******************************************************************/

#ifdef HAVE_READLINE

static char *new_command_generator(const char *text, int);
static char **new_mysql_completion(const char *text, int start, int end);

/*
  Tell the GNU Readline library how to complete.  We want to try to complete
  on command names if this is the first word in the line, or on filenames
  if not.
*/

#if defined(USE_NEW_EDITLINE_INTERFACE)
static int fake_magic_space(int, int);
char *no_completion(const char *, int)
#elif defined(USE_LIBEDIT_INTERFACE)
static int fake_magic_space(int, int);
char *no_completion(const char *, int)
#else
char *no_completion()
#endif
{
  return 0; /* No filename completion */
}

/*
  returns 0 if line matches the previous history entry
  returns 1 if the line doesn't match the previous history entry
*/
static int not_in_history(const char *line) {
  HIST_ENTRY *oldhist = history_get(history_length);

  if (oldhist == nullptr) return 1;
  if (strcmp(oldhist->line, line) == 0) return 0;
  return 1;
}

#if defined(USE_NEW_EDITLINE_INTERFACE)
static int fake_magic_space(int, int)
#else
static int fake_magic_space(int, int)
#endif
{
  rl_insert(1, ' ');
  return 0;
}

static void initialize_readline(char *name) {
  /* Allow conditional parsing of the ~/.inputrc file. */
  rl_readline_name = name;

  /* Tell the completer that we want a crack first. */
#if defined(USE_NEW_EDITLINE_INTERFACE)
  rl_attempted_completion_function = &new_mysql_completion;
  rl_completion_entry_function = &no_completion;

  rl_add_defun("magic-space", &fake_magic_space, -1);
#elif defined(USE_LIBEDIT_INTERFACE)
  setlocale(LC_ALL, ""); /* so as libedit use isprint */
  rl_attempted_completion_function = &new_mysql_completion;
  rl_completion_entry_function = &no_completion;
  rl_add_defun("magic-space", &fake_magic_space, -1);
#else
  rl_attempted_completion_function = (CPPFunction *)&new_mysql_completion;
  rl_completion_entry_function = &no_completion;
#endif
}

/*
  Attempt to complete on the contents of TEXT.  START and END show the
  region of TEXT that contains the word to complete.  We can use the
  entire line in case we want to do some simple parsing.  Return the
  array of matches, or NULL if there aren't any.
*/

static char **new_mysql_completion(const char *text,
                                   int start MY_ATTRIBUTE((unused)),
                                   int end MY_ATTRIBUTE((unused))) {
  if (!status.batch && !quick)
#if defined(USE_NEW_EDITLINE_INTERFACE)
    return rl_completion_matches(text, new_command_generator);
#else
    return completion_matches(const_cast<char *>(text), new_command_generator);
#endif
  else
    return (char **)nullptr;
}

static char *new_command_generator(const char *text, int state) {
  static int textlen;
  char *ptr;
  static Bucket *b;
  static entry *e;
  static uint i;

  if (!state) textlen = (uint)strlen(text);

  if (textlen > 0) { /* lookup in the hash */
    if (!state) {
      uint len;

      b = find_all_matches(&ht, text, (uint)strlen(text), &len);
      if (!b) return NullS;
      e = b->pData;
    }

    if (e) {
      ptr = strdup(e->str);
      e = e->pNext;
      return ptr;
    }
  } else { /* traverse the entire hash, ugly but works */

    if (!state) {
      /* find the first used bucket */
      for (i = 0; i < ht.nTableSize; i++) {
        if (ht.arBuckets[i]) {
          b = ht.arBuckets[i];
          e = b->pData;
          break;
        }
      }
    }
    ptr = NullS;
    while (e && !ptr) { /* find valid entry in bucket */
      if ((uint)strlen(e->str) == b->nKeyLength) ptr = strdup(e->str);
      /* find the next used entry */
      e = e->pNext;
      if (!e) { /* find the next used bucket */
        b = b->pNext;
        if (!b) {
          for (i++; i < ht.nTableSize; i++) {
            if (ht.arBuckets[i]) {
              b = ht.arBuckets[i];
              e = b->pData;
              break;
            }
          }
        } else
          e = b->pData;
      }
    }
    if (ptr) return ptr;
  }
  return NullS;
}

/* Build up the completion hash */

static void build_completion_hash(bool rehash, bool write_info) {
  COMMANDS *cmd = commands;
  MYSQL_RES *databases = nullptr, *tables = nullptr;
  MYSQL_RES *fields;
  static char ***field_names = nullptr;
  MYSQL_ROW database_row, table_row;
  MYSQL_FIELD *sql_field;
  char buf[NAME_LEN * 2 + 2];  // table name plus field name plus 2
  int i, j, num_fields;
  DBUG_TRACE;

#ifndef DBUG_OFF
  if (!opt_build_completion_hash)
#endif
  {
    if (status.batch || quick || !current_db)
      return;  // We don't need completion in batches
  }

  if (!rehash) return;

  /* Free old used memory */
  if (field_names) field_names = nullptr;
  completion_hash_clean(&ht);
  free_root(&hash_mem_root, MYF(0));

  /* hash this file's known subset of SQL commands */
  while (cmd->name) {
    add_word(&ht, cmd->name);
    cmd++;
  }

  /* hash MySQL functions (to be implemented) */

  /* hash all database names */
  if (mysql_query(&mysql, "show databases") == 0) {
    if (!(databases = mysql_store_result(&mysql)))
      put_info(mysql_error(&mysql), INFO_INFO);
    else {
      while ((database_row = mysql_fetch_row(databases))) {
        char *str = strdup_root(&hash_mem_root, (char *)database_row[0]);
        if (str) add_word(&ht, (char *)str);
      }
      mysql_free_result(databases);
    }
  }
  /* hash all table names */
  if (mysql_query(&mysql, "show tables") == 0) {
    if (!(tables = mysql_store_result(&mysql)))
      put_info(mysql_error(&mysql), INFO_INFO);
    else {
      if (mysql_num_rows(tables) > 0 && !opt_silent && write_info) {
        tee_fprintf(stdout,
                    "\
Reading table information for completion of table and column names\n\
You can turn off this feature to get a quicker startup with -A\n\n");
      }
      while ((table_row = mysql_fetch_row(tables))) {
        char *str = strdup_root(&hash_mem_root, (char *)table_row[0]);
        if (str && !completion_hash_exists(&ht, (char *)str, (uint)strlen(str)))
          add_word(&ht, str);
      }
    }
  }

  /* hash all field names, both with the table prefix and without it */
  if (!tables) /* no tables */
  {
    return;
  }
  mysql_data_seek(tables, 0);
  if (!(field_names = (char ***)hash_mem_root.Alloc(
            sizeof(char **) * (uint)(mysql_num_rows(tables) + 1)))) {
    mysql_free_result(tables);
    return;
  }
  i = 0;
  while ((table_row = mysql_fetch_row(tables))) {
    if ((fields =
             mysql_list_fields(&mysql, (const char *)table_row[0], NullS))) {
      num_fields = mysql_num_fields(fields);
      if (!(field_names[i] = (char **)hash_mem_root.Alloc(
                sizeof(char *) * (num_fields * 2 + 1)))) {
        mysql_free_result(fields);
        break;
      }
      field_names[i][num_fields * 2] = nullptr;
      j = 0;
      while ((sql_field = mysql_fetch_field(fields))) {
        sprintf(buf, "%.64s.%.64s", table_row[0], sql_field->name);
        field_names[i][j] = strdup_root(&hash_mem_root, buf);
        add_word(&ht, field_names[i][j]);
        field_names[i][num_fields + j] =
            strdup_root(&hash_mem_root, sql_field->name);
        if (!completion_hash_exists(
                &ht, field_names[i][num_fields + j],
                (uint)strlen(field_names[i][num_fields + j])))
          add_word(&ht, field_names[i][num_fields + j]);
        j++;
      }
      mysql_free_result(fields);
    } else
      field_names[i] = nullptr;

    i++;
  }
  mysql_free_result(tables);
  field_names[i] = nullptr;  // End pointer
}

/* for gnu readline */

#ifndef HAVE_INDEX
extern "C" {
extern char *index(const char *, int c), *rindex(const char *, int);

char *index(const char *s, int c) {
  for (;;) {
    if (*s == (char)c) return (char *)s;
    if (!*s++) return NullS;
  }
}

char *rindex(const char *s, int c) {
  char *t;

  t = NullS;
  do
    if (*s == (char)c) t = (char *)s;
  while (*s++);
  return (char *)t;
}
}
#endif /* ! HAVE_INDEX */
#endif /* HAVE_READLINE */

static void fix_line(String *final_command) {
  int total_lines = 1;
  char *ptr = final_command->c_ptr();
  String fixed_buffer; /* Converted buffer */

  /* Character if we are in a string or not */
  char str_char = '\0';

  /* find out how many lines we have and remove newlines */
  while (*ptr != '\0') {
    switch (*ptr) {
      /* string character */
      case '"':
      case '\'':
      case '`':
        if (str_char == '\0') /* open string */
          str_char = *ptr;
        else if (str_char == *ptr) /* close string */
          str_char = '\0';
        fixed_buffer.append(ptr, 1);
        break;
      case '\n':
        /* not in string, change to space if in string, leave it alone */
        fixed_buffer.append(str_char == '\0' ? " " : "\n");
        total_lines++;
        break;
      case '\\':
        fixed_buffer.append('\\');
        /* need to see if the backslash is escaping anything */
        if (str_char) {
          ptr++;
          /* special characters that need escaping */
          if (*ptr == '\'' || *ptr == '"' || *ptr == '\\')
            fixed_buffer.append(ptr, 1);
          else
            ptr--;
        }
        break;

      default:
        fixed_buffer.append(ptr, 1);
    }
    ptr++;
  }
  if (total_lines > 1) add_filtered_history(fixed_buffer.ptr());
}

/* Add the given line to mysql history and syslog. */
static void add_filtered_history(const char *string) {
  // line shouldn't be on history ignore list
  if (ignore_matcher.is_matching(string, charset_info)) return;

#ifdef HAVE_READLINE
  if (!quick && not_in_history(string)) add_history(string);
#endif

  if (opt_syslog) add_syslog(string);
}

void add_syslog(const char *line) {
  char buff[MAX_SYSLOG_MESSAGE_SIZE];
  snprintf(buff, sizeof(buff),
           "SYSTEM_USER:'%s', MYSQL_USER:'%s', "
           "CONNECTION_ID:%lu, DB_SERVER:'%s', DB:'%s', QUERY:'%s'",
           /* use the cached user/sudo_user value. */
           current_os_sudouser ? current_os_sudouser
                               : current_os_user ? current_os_user : "--",
           current_user ? current_user : "--", mysql_thread_id(&mysql),
           current_host ? current_host : "--", current_db ? current_db : "--",
           line);

  (void)my_syslog(charset_info, INFORMATION_LEVEL, buff);
  return;
}

static int reconnect(void) {
  /* purecov: begin tested */
  if (opt_reconnect) {
    put_info("No connection. Trying to reconnect...", INFO_INFO);
    (void)com_connect((String *)nullptr, nullptr);
    if (opt_rehash) com_rehash(nullptr, nullptr);
  }
  if (!connected) return put_info("Can't connect to the server\n", INFO_ERROR);
  /* purecov: end */
  return 0;
}

static void get_current_db() {
  MYSQL_RES *res;

  /* If one_database is set, current_db is not supposed to change. */
  if (one_database) return;

  my_free(current_db);
  current_db = nullptr;
  /* In case of error below current_db will be NULL */
  if (!mysql_query(&mysql, "SELECT DATABASE()") &&
      (res = mysql_use_result(&mysql))) {
    MYSQL_ROW row = mysql_fetch_row(res);
    if (row && row[0])
      current_db = my_strdup(PSI_NOT_INSTRUMENTED, row[0], MYF(MY_WME));
    mysql_free_result(res);
  }
}

/***************************************************************************
 The different commands
***************************************************************************/

static int mysql_real_query_for_lazy(const char *buf, size_t length) {
  for (uint retry = 0;; retry++) {
    int error;
    if (opt_checksum)
      mysql_options4(&mysql, MYSQL_OPT_QUERY_ATTR_ADD, "checksum", "ON");
    if (!mysql_real_query(&mysql, buf, (ulong)length)) return 0;
    error = put_error(&mysql);
    if (mysql_errno(&mysql) != CR_SERVER_GONE_ERROR || retry > 1 ||
        !opt_reconnect)
      return error;
    if (reconnect()) return error;
  }
}

static int mysql_store_result_for_lazy(MYSQL_RES **result) {
  if ((*result = mysql_store_result(&mysql))) return 0;

  if (mysql_error(&mysql)[0]) return put_error(&mysql);
  return 0;
}

static void print_help_item(MYSQL_ROW *cur, int num_name, int num_cat,
                            char *last_char) {
  char ccat = (*cur)[num_cat][0];
  if (*last_char != ccat) {
    put_info(ccat == 'Y' ? "categories:" : "topics:", INFO_INFO);
    *last_char = ccat;
  }
  tee_fprintf(PAGER, "   %s\n", (*cur)[num_name]);
}

static int com_server_help(String *buffer MY_ATTRIBUTE((unused)),
                           char *line MY_ATTRIBUTE((unused)), char *help_arg) {
  MYSQL_ROW cur;
  const char *server_cmd;
  char cmd_buf[100 + 1];
  MYSQL_RES *result;
  int error;

  if (help_arg[0] != '\'') {
    char *end_arg = strend(help_arg);
    if (--end_arg) {
      while (my_isspace(charset_info, *end_arg)) end_arg--;
      *++end_arg = '\0';
    }
    (void)strxnmov(cmd_buf, sizeof(cmd_buf), "help '", help_arg, "'", NullS);
  } else
    (void)strxnmov(cmd_buf, sizeof(cmd_buf), "help ", help_arg, NullS);

  server_cmd = cmd_buf;

  if (!status.batch) {
    old_buffer = *buffer;
    old_buffer.copy();
  }

  if (!connected && reconnect()) return 1;

  if ((error =
           mysql_real_query_for_lazy(server_cmd, (int)strlen(server_cmd))) ||
      (error = mysql_store_result_for_lazy(&result)))
    return error;

  if (result) {
    unsigned int num_fields = mysql_num_fields(result);
    uint64_t num_rows = mysql_num_rows(result);
    mysql_fetch_fields(result);
    if (num_fields == 3 && num_rows == 1) {
      if (!(cur = mysql_fetch_row(result))) {
        error = -1;
        goto err;
      }

      init_pager();
      tee_fprintf(PAGER, "Name: \'%s\'\n", cur[0]);
      tee_fprintf(PAGER, "Description:\n%s", cur[1]);
      if (cur[2] && *((char *)cur[2]))
        tee_fprintf(PAGER, "Examples:\n%s", cur[2]);
      tee_fprintf(PAGER, "\n");
      end_pager();
    } else if (num_fields >= 2 && num_rows) {
      init_pager();
      char last_char = 0;

      int num_name = 0, num_cat = 0;

      if (num_fields == 2) {
        put_info("Many help items for your request exist.", INFO_INFO);
        put_info(
            "To make a more specific request, please type 'help "
            "<item>',\nwhere <item> is one of the following",
            INFO_INFO);
        num_name = 0;
        num_cat = 1;
      } else if ((cur = mysql_fetch_row(result))) {
        tee_fprintf(PAGER, "You asked for help about help category: \"%s\"\n",
                    cur[0]);
        put_info(
            "For more information, type 'help <item>', where <item> is one of "
            "the following",
            INFO_INFO);
        num_name = 1;
        num_cat = 2;
        print_help_item(&cur, 1, 2, &last_char);
      }

      while ((cur = mysql_fetch_row(result)))
        print_help_item(&cur, num_name, num_cat, &last_char);
      tee_fprintf(PAGER, "\n");
      end_pager();
    } else {
      put_info("\nNothing found", INFO_INFO);
      if (native_strncasecmp(server_cmd, "help 'contents'", 15) == 0) {
        put_info("\nPlease check if 'help tables' are loaded.\n", INFO_INFO);
        goto err;
      }
      put_info(
          "Please try to run 'help contents' for a list of all accessible "
          "topics\n",
          INFO_INFO);
    }
  }

err:
  mysql_free_result(result);
  return error;
}

static int com_help(String *buffer MY_ATTRIBUTE((unused)),
                    char *line MY_ATTRIBUTE((unused))) {
  int i, j;
  char *help_arg = strchr(line, ' '), buff[32], *end;
  if (help_arg) {
    while (my_isspace(charset_info, *help_arg)) help_arg++;
    if (*help_arg) return com_server_help(buffer, line, help_arg);
  }

  put_info(
      "\nFor information about MySQL products and services, visit:\n"
      "   http://www.mysql.com/\n"
      "For developer information, including the MySQL Reference Manual, "
      "visit:\n"
      "   http://dev.mysql.com/\n"
      "To buy MySQL Enterprise support, training, or other products, visit:\n"
      "   https://shop.mysql.com/\n",
      INFO_INFO);
  put_info("List of all MySQL commands:", INFO_INFO);
  if (!named_cmds)
    put_info(
        "Note that all text commands must be first on line and end with ';'",
        INFO_INFO);
  for (i = 0; commands[i].name; i++) {
    end = my_stpcpy(buff, commands[i].name);
    for (j = (int)strlen(commands[i].name); j < 10; j++)
      end = my_stpcpy(end, " ");
    if (commands[i].func)
      tee_fprintf(stdout, "%s(\\%c) %s\n", buff, commands[i].cmd_char,
                  commands[i].doc);
  }
  if (connected && mysql_get_server_version(&mysql) >= 40100)
    put_info("\nFor server side help, type 'help contents'\n", INFO_INFO);
  return 0;
}

/* ARGSUSED */
static int com_clear(String *buffer, char *line MY_ATTRIBUTE((unused))) {
  if (status.add_to_history) fix_line(buffer);
  buffer->length(0);
  return 0;
}

/* ARGSUSED */
static int com_charset(String *buffer MY_ATTRIBUTE((unused)), char *line) {
  char buff[256], *param;
  const CHARSET_INFO *new_cs;
  strmake(buff, line, sizeof(buff) - 1);
  param = get_arg(buff, false);
  if (!param || !*param) {
    return put_info("Usage: \\C charset_name | charset charset_name",
                    INFO_ERROR, 0);
  }
  new_cs = get_charset_by_csname(param, MY_CS_PRIMARY, MYF(MY_WME));
  if (new_cs) {
    charset_info = new_cs;
    mysql_set_character_set(&mysql, charset_info->csname);
    default_charset = charset_info->csname;
    put_info("Charset changed", INFO_INFO);
  } else
    put_info("Charset is not found", INFO_INFO);
  return 0;
}

/*
  Execute command
  Returns: 0  if ok
          -1 if not fatal error
          1  if fatal error
*/

static int com_go(String *buffer, char *line MY_ATTRIBUTE((unused))) {
  char buff[200];             /* about 110 chars used so far */
  char time_buff[52 + 3 + 1]; /* time max + space&parens + NUL */
  MYSQL_RES *result;
  ulong timer, warnings = 0;
  uint error = 0;
  int err = 0;

  interrupted_query = false;
  if (!status.batch) {
    old_buffer = *buffer;  // Save for edit command
    old_buffer.copy();
  }

  /* Remove garbage for nicer messages */
  buff[0] = 0;
  remove_cntrl(buffer);

  if (buffer->is_empty()) {
    if (status.batch)  // Ignore empty quries
      return 0;
    return put_info("No query specified\n", INFO_ERROR);
  }
  if (!connected && reconnect()) {
    buffer->length(0);              // Remove query on error
    return opt_reconnect ? -1 : 1;  // Fatal error
  }
  if (verbose) (void)com_print(buffer, nullptr);

  if (skip_updates && (buffer->length() < 4 ||
                       my_strnncoll(charset_info, (const uchar *)buffer->ptr(),
                                    4, (const uchar *)"SET ", 4))) {
    (void)put_info("Ignoring query to other database", INFO_INFO);
    return 0;
  }

  timer = start_timer();
  executing_query = true;
  error = mysql_real_query_for_lazy(buffer->ptr(), buffer->length());

  if (status.add_to_history) {
    buffer->append(vertical ? "\\G" : delimiter);
    /* Append final command onto history and syslog. */
    fix_line(buffer);
  }
  buffer->length(0);

  if (error) goto end;

  do {
    char *pos;
    bool batchmode = (status.batch && verbose <= 1);
    buff[0] = 0;

    if (quick) {
      if (!(result = mysql_use_result(&mysql)) && mysql_field_count(&mysql)) {
        error = put_error(&mysql);
        goto end;
      }
    } else {
      error = mysql_store_result_for_lazy(&result);
      if (error) goto end;
    }

    if (verbose >= 3 || !opt_silent)
      mysql_end_timer(timer, time_buff);
    else
      time_buff[0] = '\0';

    /* Every branch must truncate  buff . */
    if (result) {
      if (!mysql_num_rows(result) && !quick && !column_types_flag) {
        my_stpcpy(buff, "Empty set");
        if (opt_xml) {
          /*
            We must print XML header and footer
            to produce a well-formed XML even if
            the result set is empty (Bug#27608).
          */
          init_pager();
          print_table_data_xml(result);
          end_pager();
        }
      } else {
        init_pager();
        if (opt_html)
          print_table_data_html(result);
        else if (opt_xml)
          print_table_data_xml(result);
        else if (vertical || (auto_vertical_output &&
                              (terminal_width < get_result_width(result))))
          print_table_data_vertically(result);
        else if (opt_silent && verbose <= 2 && !output_tables)
          print_tab_data(result);
        else
          print_table_data(result);
        if (!batchmode)
          sprintf(buff, "%" PRId64 " %s in set", mysql_num_rows(result),
                  mysql_num_rows(result) == 1LL ? "row" : "rows");
        end_pager();
        if (mysql_errno(&mysql)) error = put_error(&mysql);
      }
    } else if (mysql_affected_rows(&mysql) == ~(ulonglong)0)
      my_stpcpy(buff, "Query OK");
    else if (!batchmode)
      sprintf(buff, "Query OK, %" PRId64 " %s affected",
              mysql_affected_rows(&mysql),
              mysql_affected_rows(&mysql) == 1LL ? "row" : "rows");

    pos = strend(buff);
    if ((warnings = mysql_warning_count(&mysql)) && !batchmode) {
      *pos++ = ',';
      *pos++ = ' ';
      pos = longlong10_to_str(warnings, pos, 10);
      pos = my_stpcpy(pos, " warning");
      if (warnings != 1) *pos++ = 's';
    }
    my_stpcpy(pos, time_buff);
    put_info(buff, INFO_RESULT);
    if (mysql_info(&mysql)) put_info(mysql_info(&mysql), INFO_RESULT);
    put_info("", INFO_RESULT);  // Empty row

    if (result && !mysql_eof(result)) /* Something wrong when using quick */
      error = put_error(&mysql);
    else if (unbuffered)
      fflush(stdout);
    mysql_free_result(result);
  } while (!(err = mysql_next_result(&mysql)));
  if (err >= 1) error = put_error(&mysql);

end:

  /* Show warnings if any or error occurred */
  if (show_warnings == 1 && (warnings >= 1 || error)) print_warnings();

  if (!error && !status.batch &&
      (mysql.server_status & SERVER_STATUS_DB_DROPPED))
    get_current_db();

  executing_query = false;
  return error; /* New command follows */
}

static void init_pager() {
#ifdef USE_POPEN
  if (!opt_nopager) {
    if (!(PAGER = popen(pager, "w"))) {
      tee_fprintf(stdout, "popen() failed! defaulting PAGER to stdout!\n");
      PAGER = stdout;
    }
  } else
#endif
    PAGER = stdout;
}

static void end_pager() {
#ifdef USE_POPEN
  if (!opt_nopager) pclose(PAGER);
#endif
}

static void init_tee(const char *file_name) {
  FILE *new_outfile;
  if (opt_outfile) end_tee();
  if (!(new_outfile = my_fopen(file_name, O_APPEND | O_WRONLY, MYF(MY_WME)))) {
    tee_fprintf(stdout, "Error logging to file '%s'\n", file_name);
    return;
  }
  OUTFILE = new_outfile;
  strmake(outfile, file_name, FN_REFLEN - 1);
  tee_fprintf(stdout, "Logging to file '%s'\n", file_name);
  opt_outfile = true;
  return;
}

static void end_tee() {
  my_fclose(OUTFILE, MYF(0));
  OUTFILE = nullptr;
  opt_outfile = false;
  return;
}

static int com_ego(String *buffer, char *line) {
  int result;
  bool oldvertical = vertical;
  vertical = true;
  result = com_go(buffer, line);
  vertical = oldvertical;
  return result;
}

static const char *fieldtype2str(enum enum_field_types type) {
  switch (type) {
    case MYSQL_TYPE_BIT:
      return "BIT";
    case MYSQL_TYPE_BLOB:
      return "BLOB";
    case MYSQL_TYPE_DATE:
      return "DATE";
    case MYSQL_TYPE_DATETIME:
      return "DATETIME";
    case MYSQL_TYPE_NEWDECIMAL:
      return "NEWDECIMAL";
    case MYSQL_TYPE_DECIMAL:
      return "DECIMAL";
    case MYSQL_TYPE_DOUBLE:
      return "DOUBLE";
    case MYSQL_TYPE_ENUM:
      return "ENUM";
    case MYSQL_TYPE_FLOAT:
      return "FLOAT";
    case MYSQL_TYPE_GEOMETRY:
      return "GEOMETRY";
    case MYSQL_TYPE_INT24:
      return "INT24";
    case MYSQL_TYPE_JSON:
      return "JSON";
    case MYSQL_TYPE_LONG:
      return "LONG";
    case MYSQL_TYPE_LONGLONG:
      return "LONGLONG";
    case MYSQL_TYPE_LONG_BLOB:
      return "LONG_BLOB";
    case MYSQL_TYPE_MEDIUM_BLOB:
      return "MEDIUM_BLOB";
    case MYSQL_TYPE_NEWDATE:
      return "NEWDATE";
    case MYSQL_TYPE_NULL:
      return "NULL";
    case MYSQL_TYPE_SET:
      return "SET";
    case MYSQL_TYPE_SHORT:
      return "SHORT";
    case MYSQL_TYPE_STRING:
      return "STRING";
    case MYSQL_TYPE_TIME:
      return "TIME";
    case MYSQL_TYPE_TIMESTAMP:
      return "TIMESTAMP";
    case MYSQL_TYPE_TINY:
      return "TINY";
    case MYSQL_TYPE_TINY_BLOB:
      return "TINY_BLOB";
    case MYSQL_TYPE_VAR_STRING:
      return "VAR_STRING";
    case MYSQL_TYPE_YEAR:
      return "YEAR";
    default:
      return "?-unknown-?";
  }
}

static char *fieldflags2str(uint f) {
  static char buf[1024];
  char *s = buf;
  *s = 0;
#define ff2s_check_flag(X)    \
  if (f & X##_FLAG) {         \
    s = my_stpcpy(s, #X " "); \
    f &= ~X##_FLAG;           \
  }
  ff2s_check_flag(NOT_NULL);
  ff2s_check_flag(PRI_KEY);
  ff2s_check_flag(UNIQUE_KEY);
  ff2s_check_flag(MULTIPLE_KEY);
  ff2s_check_flag(BLOB);
  ff2s_check_flag(UNSIGNED);
  ff2s_check_flag(ZEROFILL);
  ff2s_check_flag(BINARY);
  ff2s_check_flag(ENUM);
  ff2s_check_flag(AUTO_INCREMENT);
  ff2s_check_flag(TIMESTAMP);
  ff2s_check_flag(SET);
  ff2s_check_flag(NO_DEFAULT_VALUE);
  ff2s_check_flag(NUM);
  ff2s_check_flag(PART_KEY);
  ff2s_check_flag(GROUP);
  ff2s_check_flag(UNIQUE);
  ff2s_check_flag(BINCMP);
  ff2s_check_flag(ON_UPDATE_NOW);
#undef ff2s_check_flag
  if (f) sprintf(s, " unknows=0x%04x", f);
  return buf;
}

static void print_field_types(MYSQL_RES *result) {
  MYSQL_FIELD *field;
  uint i = 0;

  while ((field = mysql_fetch_field(result))) {
    tee_fprintf(PAGER,
                "Field %3u:  `%s`\n"
                "Catalog:    `%s`\n"
                "Database:   `%s`\n"
                "Table:      `%s`\n"
                "Org_table:  `%s`\n"
                "Type:       %s\n"
                "Collation:  %s (%u)\n"
                "Length:     %lu\n"
                "Max_length: %lu\n"
                "Decimals:   %u\n"
                "Flags:      %s\n\n",
                ++i, field->name, field->catalog, field->db, field->table,
                field->org_table, fieldtype2str(field->type),
                get_charset_name(field->charsetnr), field->charsetnr,
                field->length, field->max_length, field->decimals,
                fieldflags2str(field->flags));
  }
  tee_puts("", PAGER);
}

/* Used to determine if we should invoke print_as_hex for this field */

static bool is_binary_field(MYSQL_FIELD *field) {
  if ((field->charsetnr == 63) &&
      (field->type == MYSQL_TYPE_BIT || field->type == MYSQL_TYPE_BLOB ||
       field->type == MYSQL_TYPE_LONG_BLOB ||
       field->type == MYSQL_TYPE_MEDIUM_BLOB ||
       field->type == MYSQL_TYPE_TINY_BLOB ||
       field->type == MYSQL_TYPE_VAR_STRING ||
       field->type == MYSQL_TYPE_STRING || field->type == MYSQL_TYPE_VARCHAR ||
       field->type == MYSQL_TYPE_GEOMETRY))
    return true;
  return false;
}

/* Print binary value as hex literal (0x ...) */

static void print_as_hex(FILE *output_file, const char *str, ulong len,
                         ulong total_bytes_to_send) {
  const char *ptr = str, *end = ptr + len;
  ulong i;

  if (len > 0) {
    fprintf(output_file, "0x");
    for (; ptr < end; ptr++)
      fprintf(output_file, "%02X",
              *(static_cast<const uchar *>(static_cast<const void *>(ptr))));
    /* Printed string length: two chars "0x" + two chars for each byte. */
    i = 2 + len * 2;
  } else {
    i = fprintf(output_file, "NULL");
  }
  for (; i < total_bytes_to_send; i++)
    tee_putc(static_cast<int>(' '), output_file);
}

static void print_table_data(MYSQL_RES *result) {
  String separator(256);
  MYSQL_ROW cur;
  MYSQL_FIELD *field;
  bool *num_flag;
  size_t sz;

  sz = sizeof(bool) * mysql_num_fields(result);
  num_flag = (bool *)my_safe_alloca(sz, MAX_ALLOCA_SIZE);
  if (column_types_flag) {
    print_field_types(result);
    if (!mysql_num_rows(result)) return;
    mysql_field_seek(result, 0);
  }
  separator.copy("+", 1, charset_info);
  while ((field = mysql_fetch_field(result))) {
    size_t length = column_names ? field->name_length : 0;
    if (quick)
      length = max<size_t>(length, field->length);
    else
      length = max<size_t>(length, field->max_length);
    if (length < 4 && !IS_NOT_NULL(field->flags))
      length = 4;  // Room for "NULL"
    if (opt_binhex && is_binary_field(field)) length = 2 + length * 2;
    field->max_length = (ulong)length;
    separator.fill(separator.length() + length + 2, '-');
    separator.append('+');
  }
  separator.append('\0');  // End marker for \0
  tee_puts(separator.ptr(), PAGER);
  if (column_names) {
    mysql_field_seek(result, 0);
    (void)tee_fputs("|", PAGER);
    for (uint off = 0; (field = mysql_fetch_field(result)); off++) {
      size_t name_length = strlen(field->name);
      size_t numcells = charset_info->cset->numcells(charset_info, field->name,
                                                     field->name + name_length);
      size_t display_length = field->max_length + name_length - numcells;
      tee_fprintf(PAGER, " %-*s |",
                  min<int>((int)display_length, MAX_COLUMN_LENGTH),
                  field->name);
      num_flag[off] = IS_NUM(field->type);
    }
    (void)tee_fputs("\n", PAGER);
    tee_puts(separator.ptr(), PAGER);
  }

  while ((cur = mysql_fetch_row(result))) {
    ulong *lengths = mysql_fetch_lengths(result);
    (void)tee_fputs("| ", PAGER);
    mysql_field_seek(result, 0);
    for (uint off = 0; off < mysql_num_fields(result); off++) {
      const char *buffer;
      uint data_length;
      uint field_max_length;
      size_t visible_length;
      uint extra_padding;

      if (off) (void)tee_fputs(" ", PAGER);

      if (cur[off] == nullptr) {
        buffer = "NULL";
        data_length = 4;
      } else {
        buffer = cur[off];
        data_length = (uint)lengths[off];
      }

      field = mysql_fetch_field(result);
      field_max_length = field->max_length;

      /*
       How many text cells on the screen will this string span?  If it contains
       multibyte characters, then the number of characters we occupy on screen
       will be fewer than the number of bytes we occupy in memory.

       We need to find how much screen real-estate we will occupy to know how
       many extra padding-characters we should send with the printing function.
      */
      visible_length = charset_info->cset->numcells(charset_info, buffer,
                                                    buffer + data_length);
      extra_padding = (uint)(data_length - visible_length);

      if (opt_binhex && is_binary_field(field))
        print_as_hex(PAGER, cur[off], lengths[off], field_max_length);
      else if (field_max_length > MAX_COLUMN_LENGTH)
        tee_print_sized_data(buffer, data_length,
                             MAX_COLUMN_LENGTH + extra_padding, false);
      else {
        if (num_flag[off] != 0) /* if it is numeric, we right-justify it */
          tee_print_sized_data(buffer, data_length,
                               field_max_length + extra_padding, true);
        else
          tee_print_sized_data(buffer, data_length,
                               field_max_length + extra_padding, false);
      }
      tee_fputs(" |", PAGER);
    }
    (void)tee_fputs("\n", PAGER);

    // Check interrupted_query last; this ensures that we get at least one row.
    // This is useful for aborted EXPLAIN ANALYZE queries.
    if (interrupted_query) break;
  }
  tee_puts(separator.ptr(), PAGER);
  my_safe_afree((bool *)num_flag, sz, MAX_ALLOCA_SIZE);
}

/**
  Return the length of a field after it would be rendered into text.

  This doesn't know or care about multibyte characters.  Assume we're
  using such a charset.  We can't know that all of the upcoming rows
  for this column will have bytes that each render into some fraction
  of a character.  It's at least possible that a row has bytes that
  all render into one character each, and so the maximum length is
  still the number of bytes.  (Assumption 1:  This can't be better
  because we can never know the number of characters that the DB is
  going to send -- only the number of bytes.  2: Chars <= Bytes.)

  @param  field  Pointer to a field to be inspected

  @returns  number of character positions to be used, at most
*/
static int get_field_disp_length(MYSQL_FIELD *field) {
  uint length = column_names ? field->name_length : 0;

  if (quick)
    length = max<uint>(length, field->length);
  else
    length = max<uint>(length, field->max_length);

  if (length < 4 && !IS_NOT_NULL(field->flags))
    length = 4; /* Room for "NULL" */

  return length;
}

/**
  For a new result, return the max number of characters that any
  upcoming row may return.

  @param  result  Pointer to the result to judge

  @returns  The max number of characters in any row of this result
*/
static int get_result_width(MYSQL_RES *result) {
  unsigned int len = 0;
  MYSQL_FIELD *field;
  MYSQL_FIELD_OFFSET offset;

#ifndef DBUG_OFF
  offset = mysql_field_tell(result);
  DBUG_ASSERT(offset == 0);
#else
  offset = 0;
#endif

  while ((field = mysql_fetch_field(result)) != nullptr)
    len +=
        get_field_disp_length(field) + 3; /* plus bar, space, & final space */

  (void)mysql_field_seek(result, offset);

  return len + 1; /* plus final bar. */
}

static void tee_print_sized_data(const char *data, unsigned int data_length,
                                 unsigned int total_bytes_to_send,
                                 bool right_justified) {
  /*
    For '\0's print ASCII spaces instead, as '\0' is eaten by (at
    least my) console driver, and that messes up the pretty table
    grid.  (The \0 is also the reason we can't use fprintf() .)
  */
  unsigned int i;

  if (right_justified)
    for (i = data_length; i < total_bytes_to_send; i++)
      tee_putc((int)' ', PAGER);

  tee_write(PAGER, data, data_length, MY_PRINT_SPS_0 | MY_PRINT_MB);

  if (!right_justified)
    for (i = data_length; i < total_bytes_to_send; i++)
      tee_putc((int)' ', PAGER);
}

static void print_table_data_html(MYSQL_RES *result) {
  MYSQL_ROW cur;
  MYSQL_FIELD *field;

  mysql_field_seek(result, 0);
  (void)tee_fputs("<TABLE BORDER=1><TR>", PAGER);
  if (column_names) {
    while ((field = mysql_fetch_field(result))) {
      tee_fputs("<TH>", PAGER);
      if (field->name && field->name[0])
        xmlencode_print(field->name, field->name_length);
      else
        tee_fputs(field->name ? " &nbsp; " : "NULL", PAGER);
      tee_fputs("</TH>", PAGER);
    }
    (void)tee_fputs("</TR>", PAGER);
  }
  while ((cur = mysql_fetch_row(result))) {
    if (interrupted_query) break;
    ulong *lengths = mysql_fetch_lengths(result);
    field = mysql_fetch_fields(result);
    (void)tee_fputs("<TR>", PAGER);
    for (uint i = 0; i < mysql_num_fields(result); i++) {
      (void)tee_fputs("<TD>", PAGER);
      if (opt_binhex && is_binary_field(&field[i]))
        print_as_hex(PAGER, cur[i], lengths[i], lengths[i]);
      else
        xmlencode_print(cur[i], lengths[i]);
      (void)tee_fputs("</TD>", PAGER);
    }
    (void)tee_fputs("</TR>", PAGER);
  }
  (void)tee_fputs("</TABLE>", PAGER);
}

static void print_table_data_xml(MYSQL_RES *result) {
  MYSQL_ROW cur;
  MYSQL_FIELD *fields;

  mysql_field_seek(result, 0);

  tee_fputs("<?xml version=\"1.0\"?>\n\n<resultset statement=\"", PAGER);
  xmlencode_print(glob_buffer.ptr(), (int)strlen(glob_buffer.ptr()));
  tee_fputs("\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">",
            PAGER);

  fields = mysql_fetch_fields(result);
  while ((cur = mysql_fetch_row(result))) {
    if (interrupted_query) break;
    ulong *lengths = mysql_fetch_lengths(result);
    (void)tee_fputs("\n  <row>\n", PAGER);
    for (uint i = 0; i < mysql_num_fields(result); i++) {
      tee_fprintf(PAGER, "\t<field name=\"");
      xmlencode_print(fields[i].name, (uint)strlen(fields[i].name));
      if (cur[i]) {
        tee_fprintf(PAGER, "\">");
        if (opt_binhex && is_binary_field(&fields[i]))
          print_as_hex(PAGER, cur[i], lengths[i], lengths[i]);
        else
          xmlencode_print(cur[i], lengths[i]);
        tee_fprintf(PAGER, "</field>\n");
      } else
        tee_fprintf(PAGER, "\" xsi:nil=\"true\" />\n");
    }
    (void)tee_fputs("  </row>\n", PAGER);
  }
  (void)tee_fputs("</resultset>\n", PAGER);
}

static void print_table_data_vertically(MYSQL_RES *result) {
  MYSQL_ROW cur;
  uint max_length = 0;
  MYSQL_FIELD *field;

  while ((field = mysql_fetch_field(result))) {
    uint length = field->name_length;
    if (length > max_length) max_length = length;
    field->max_length = length;
  }

  mysql_field_seek(result, 0);
  for (uint row_count = 1; (cur = mysql_fetch_row(result)); row_count++) {
    if (interrupted_query) break;
    mysql_field_seek(result, 0);
    tee_fprintf(
        PAGER,
        "*************************** %d. row ***************************\n",
        row_count);

    ulong *lengths = mysql_fetch_lengths(result);

    for (uint off = 0; off < mysql_num_fields(result); off++) {
      field = mysql_fetch_field(result);
      if (column_names)
        tee_fprintf(PAGER, "%*s: ", (int)max_length, field->name);
      if (cur[off]) {
        if (opt_binhex && is_binary_field(field))
          print_as_hex(PAGER, cur[off], lengths[off], lengths[off]);
        else
          tee_write(PAGER, cur[off], lengths[off],
                    MY_PRINT_SPS_0 | MY_PRINT_MB);
        tee_putc('\n', PAGER);
      } else
        tee_fprintf(PAGER, "NULL\n");
    }
  }
}

/* print_warnings should be called right after executing a statement */

static void print_warnings() {
  const char *query;
  MYSQL_RES *result;
  MYSQL_ROW cur;
  uint64_t num_rows;

  /* Save current error before calling "show warnings" */
  uint error = mysql_errno(&mysql);

  /* Get the warnings */
  query = "show warnings";
  mysql_real_query_for_lazy(query, strlen(query));
  mysql_store_result_for_lazy(&result);

  /* Bail out when no warnings */
  if (!result || !(num_rows = mysql_num_rows(result))) goto end;

  cur = mysql_fetch_row(result);

  /*
    Don't print a duplicate of the current error.  It is possible for SHOW
    WARNINGS to return multiple errors with the same code, but different
    messages.  To be safe, skip printing the duplicate only if it is the only
    warning.
  */
  if (!cur || (num_rows == 1 && error == (uint)strtoul(cur[1], nullptr, 10)))
    goto end;

  /* Print the warnings */
  init_pager();
  do {
    tee_fprintf(PAGER, "%s (Code %s): %s\n", cur[0], cur[1], cur[2]);
  } while ((cur = mysql_fetch_row(result)));
  end_pager();

end:
  mysql_free_result(result);
}

static const char *array_value(const char **array, char key) {
  for (; *array; array += 2)
    if (**array == key) return array[1];
  return nullptr;
}

static void xmlencode_print(const char *src, uint length) {
  if (!src)
    tee_fputs("NULL", PAGER);
  else
    tee_write(PAGER, src, length, MY_PRINT_XML | MY_PRINT_MB);
}

static void safe_put_field(const char *pos, ulong length) {
  if (!pos)
    tee_fputs("NULL", PAGER);
  else {
    int flags =
        MY_PRINT_MB | (opt_raw_data ? 0 : (MY_PRINT_ESC_0 | MY_PRINT_CTRL));
    /* Can't use tee_fputs(), it stops with NUL characters. */
    tee_write(PAGER, pos, length, flags);
  }
}

static void print_tab_data(MYSQL_RES *result) {
  MYSQL_ROW cur;
  MYSQL_FIELD *field;
  ulong *lengths;

  if (opt_silent < 2 && column_names) {
    int first = 0;
    while ((field = mysql_fetch_field(result))) {
      if (first++) (void)tee_fputs("\t", PAGER);
      (void)tee_fputs(field->name, PAGER);
    }
    (void)tee_fputs("\n", PAGER);
  }
  while ((cur = mysql_fetch_row(result))) {
    lengths = mysql_fetch_lengths(result);
    field = mysql_fetch_fields(result);
    if (opt_binhex && is_binary_field(&field[0]))
      print_as_hex(PAGER, cur[0], lengths[0], lengths[0]);
    else
      safe_put_field(cur[0], lengths[0]);
    for (uint off = 1; off < mysql_num_fields(result); off++) {
      (void)tee_fputs("\t", PAGER);
      if (opt_binhex && field && is_binary_field(&field[off]))
        print_as_hex(PAGER, cur[off], lengths[off], lengths[off]);
      else
        safe_put_field(cur[off], lengths[off]);
    }
    (void)tee_fputs("\n", PAGER);
  }
}

static int com_tee(String *buffer MY_ATTRIBUTE((unused)),
                   char *line MY_ATTRIBUTE((unused))) {
  char file_name[FN_REFLEN], *end, *param;

  while (my_isspace(charset_info, *line)) line++;
  if (!(param = strchr(line, ' ')))  // if outfile wasn't given, use the default
  {
    if (!strlen(outfile)) {
      printf("No previous outfile available, you must give a filename!\n");
      return 0;
    } else if (opt_outfile) {
      tee_fprintf(stdout, "Currently logging to file '%s'\n", outfile);
      return 0;
    } else
      param = outfile;  // resume using the old outfile
  }

  /* eliminate the spaces before the parameters */
  while (my_isspace(charset_info, *param)) param++;
  end = strmake(file_name, param, sizeof(file_name) - 1);
  /* remove end space from command line */
  while (end > file_name && (my_isspace(charset_info, end[-1]) ||
                             my_iscntrl(charset_info, end[-1])))
    end--;
  end[0] = 0;
  if (end == file_name) {
    printf("No outfile specified!\n");
    return 0;
  }
  init_tee(file_name);
  return 0;
}

static int com_notee(String *buffer MY_ATTRIBUTE((unused)),
                     char *line MY_ATTRIBUTE((unused))) {
  if (opt_outfile) end_tee();
  tee_fprintf(stdout, "Outfile disabled.\n");
  return 0;
}

/*
  Sorry, this command is not available in Windows.
*/

#ifdef USE_POPEN
static int com_pager(String *buffer MY_ATTRIBUTE((unused)),
                     char *line MY_ATTRIBUTE((unused))) {
  char pager_name[FN_REFLEN], *end, *param;

  if (status.batch) return 0;
  /* Skip spaces in front of the pager command */
  while (my_isspace(charset_info, *line)) line++;
  /* Skip the pager command */
  param = strchr(line, ' ');
  /* Skip the spaces between the command and the argument */
  while (param && my_isspace(charset_info, *param)) param++;
  if (!param || !strlen(param))  // if pager was not given, use the default
  {
    if (!default_pager_set) {
      tee_fprintf(stdout, "Default pager wasn't set, using stdout.\n");
      opt_nopager = true;
      my_stpcpy(pager, "stdout");
      PAGER = stdout;
      return 0;
    }
    my_stpcpy(pager, default_pager);
  } else {
    end = strmake(pager_name, param, sizeof(pager_name) - 1);
    while (end > pager_name && (my_isspace(charset_info, end[-1]) ||
                                my_iscntrl(charset_info, end[-1])))
      end--;
    end[0] = 0;
    my_stpcpy(pager, pager_name);
    my_stpcpy(default_pager, pager_name);
  }
  opt_nopager = false;
  tee_fprintf(stdout, "PAGER set to '%s'\n", pager);
  return 0;
}

static int com_nopager(String *buffer MY_ATTRIBUTE((unused)),
                       char *line MY_ATTRIBUTE((unused))) {
  my_stpcpy(pager, "stdout");
  opt_nopager = true;
  PAGER = stdout;
  tee_fprintf(stdout, "PAGER set to stdout\n");
  return 0;
}
#endif

/*
  Sorry, you can't send the result to an editor in Win32
*/

#ifdef USE_POPEN
static int com_edit(String *buffer, char *line MY_ATTRIBUTE((unused))) {
  char filename[FN_REFLEN], buff[160];
  int fd, tmp;
  const char *editor;

  if ((fd = create_temp_file(filename, NullS, "sql", O_CREAT | O_WRONLY,
                             KEEP_FILE, MYF(MY_WME))) < 0)
    goto err;
  if (buffer->is_empty() && !old_buffer.is_empty())
    (void)my_write(fd, (uchar *)old_buffer.ptr(), old_buffer.length(),
                   MYF(MY_WME));
  else
    (void)my_write(fd, (uchar *)buffer->ptr(), buffer->length(), MYF(MY_WME));
  (void)my_close(fd, MYF(0));

  if (!(editor = (char *)getenv("EDITOR")) &&
      !(editor = (char *)getenv("VISUAL")))
    editor = "vi";
  strxmov(buff, editor, " ", filename, NullS);
  if (system(buff) == -1) goto err;

  MY_STAT stat_arg;
  if (!my_stat(filename, &stat_arg, MYF(MY_WME))) goto err;
  if ((fd = my_open(filename, O_RDONLY, MYF(MY_WME))) < 0) goto err;
  (void)buffer->alloc((uint)stat_arg.st_size);
  if ((tmp = read(fd, buffer->ptr(), buffer->alloced_length())) >= 0L)
    buffer->length((uint)tmp);
  else
    buffer->length(0);
  (void)my_close(fd, MYF(0));
  (void)my_delete(filename, MYF(MY_WME));
err:
  return 0;
}
#endif

/* If arg is given, exit without errors. This happens on command 'quit' */

static int com_quit(String *buffer MY_ATTRIBUTE((unused)),
                    char *line MY_ATTRIBUTE((unused))) {
  status.exit_status = 0;
  return 1;
}

static int com_rehash(String *buffer MY_ATTRIBUTE((unused)),
                      char *line MY_ATTRIBUTE((unused))) {
#ifdef HAVE_READLINE
  build_completion_hash(true, false);
#endif
  return 0;
}

static int com_shell(String *buffer MY_ATTRIBUTE((unused)),
                     char *line MY_ATTRIBUTE((unused))) {
  char *shell_cmd;

  /* Skip space from line begin */
  while (my_isspace(charset_info, *line)) line++;
  if (!(shell_cmd = strchr(line, ' '))) {
    put_info("Usage: \\! shell-command", INFO_ERROR);
    return -1;
  }
  /*
    The output of the shell command does not
    get directed to the pager or the outfile
  */
  if (system(shell_cmd) == -1) {
    put_info(strerror(errno), INFO_ERROR, errno);
    return -1;
  }
  return 0;
}

static int com_attr(String *buffer MY_ATTRIBUTE((unused)), char *line) {
  static const char *delim = " \t";
  char *ptr = nullptr;
  char *buf = strdup(line);
  const char *cmd __attribute__((unused)) = strtok_r(buf, delim, &ptr);
  const char *key = strtok_r(nullptr, delim, &ptr);
  const char *val = strtok_r(nullptr, delim, &ptr);

  if (!key || !val) {
    put_info("Usage: setattr key value", INFO_ERROR);
    free(buf);
    return -1;
  }

  mysql_options4(&mysql, MYSQL_OPT_QUERY_ATTR_ADD, key, val);

  free(buf);
  return 0;
}

static int com_resp_attr(String *buffer MY_ATTRIBUTE((unused)), char *line) {
  static const char *delim = " \t";
  char *ptr = nullptr;
  char *buf = strdup(line);
  const char *cmd __attribute__((unused)) = strtok_r(buf, delim, &ptr);
  const char *key = strtok_r(nullptr, delim, &ptr);

  if (!key) {
    put_info("Usage: getattr key", INFO_ERROR);
    free(buf);
    return -1;
  }

  const char *value;
  size_t len;
  if (!mysql_resp_attr_find(&mysql, key, &value, &len)) {
    char *tmp = strndup(value, len);
    put_info(tmp, INFO_INFO);
    free(tmp);
  }

  free(buf);
  return 0;
}

static int com_print(String *buffer, char *line MY_ATTRIBUTE((unused))) {
  tee_puts("--------------", stdout);
  (void)tee_fputs(buffer->c_ptr(), stdout);
  if (!buffer->length() || (*buffer)[buffer->length() - 1] != '\n')
    tee_putc('\n', stdout);
  tee_puts("--------------\n", stdout);
  return 0; /* If empty buffer */
}

/* ARGSUSED */
static int com_connect(String *buffer, char *line) {
  char *tmp, buff[256];
  bool save_rehash = opt_rehash;
  int error;

  memset(buff, 0, sizeof(buff));
  if (buffer) {
    /*
      Two null bytes are needed in the end of buff to allow
      get_arg to find end of string the second time it's called.
    */
    tmp = strmake(buff, line, sizeof(buff) - 2);
#ifdef EXTRA_DEBUG
    tmp[1] = 0;
#endif
    tmp = get_arg(buff, false);
    if (tmp && *tmp) {
      my_free(current_db);
      current_db = my_strdup(PSI_NOT_INSTRUMENTED, tmp, MYF(MY_WME));
      tmp = get_arg(buff, true);
      if (tmp) {
        my_free(current_host);
        current_host = my_strdup(PSI_NOT_INSTRUMENTED, tmp, MYF(MY_WME));
      }
    } else {
      /* Quick re-connect */
      opt_rehash = false; /* purecov: tested */
    }
    buffer->length(0);  // command used
  } else
    opt_rehash = false;
  error = sql_connect(current_host, current_db, current_user, opt_password, 0);
  opt_rehash = save_rehash;

  if (connected) {
    sprintf(buff, "Connection id:    %lu", mysql_thread_id(&mysql));
    put_info(buff, INFO_INFO);
    sprintf(buff, "Current database: %.128s\n",
            current_db ? current_db : "*** NONE ***");
    put_info(buff, INFO_INFO);
  }
  return error;
}

static int com_source(String *buffer MY_ATTRIBUTE((unused)), char *line) {
  char source_name[FN_REFLEN], *end, *param;
  LINE_BUFFER *line_buff;
  int error;
  STATUS old_status;
  FILE *sql_file;

  /* Skip space from file name */
  while (my_isspace(charset_info, *line)) line++;
  if (!(param = strchr(line, ' ')))  // Skip command name
    return put_info("Usage: \\. <filename> | source <filename>", INFO_ERROR, 0);
  while (my_isspace(charset_info, *param)) param++;
  end = strmake(source_name, param, sizeof(source_name) - 1);
  while (end > source_name && (my_isspace(charset_info, end[-1]) ||
                               my_iscntrl(charset_info, end[-1])))
    end--;
  end[0] = 0;
  unpack_filename(source_name, source_name);
  /* open file name */
  if (!(sql_file = my_fopen(source_name, O_RDONLY | MY_FOPEN_BINARY, MYF(0)))) {
    char buff[FN_REFLEN + 60];
    sprintf(buff, "Failed to open file '%s', error: %d", source_name, errno);
    return put_info(buff, INFO_ERROR, 0);
  }

  if (!(line_buff = batch_readline_init(MAX_BATCH_BUFFER_SIZE, sql_file))) {
    my_fclose(sql_file, MYF(0));
    return put_info("Can't initialize batch_readline", INFO_ERROR, 0);
  }

  /* Save old status */
  old_status = status;
  memset(&status, 0, sizeof(status));

  status.batch = old_status.batch;  // Run in batch mode
  status.line_buff = line_buff;
  status.file_name = source_name;
  glob_buffer.length(0);  // Empty command buffer
  error = read_and_execute(false);
  status = old_status;  // Continue as before
  my_fclose(sql_file, MYF(0));
  batch_readline_end(line_buff);
  return error;
}

/* ARGSUSED */
static int com_delimiter(String *buffer MY_ATTRIBUTE((unused)), char *line) {
  char buff[256], *tmp;

  strmake(buff, line, sizeof(buff) - 1);
  tmp = get_arg(buff, false);

  if (!tmp || !*tmp) {
    put_info("DELIMITER must be followed by a 'delimiter' character or string",
             INFO_ERROR);
    return 0;
  } else {
    if (strstr(tmp, "\\")) {
      put_info("DELIMITER cannot contain a backslash character", INFO_ERROR);
      return 0;
    }
  }
  strmake(delimiter, tmp, sizeof(delimiter) - 1);
  delimiter_length = (int)strlen(delimiter);
  delimiter_str = delimiter;
  return 0;
}

/* ARGSUSED */
static int com_use(String *buffer MY_ATTRIBUTE((unused)), char *line) {
  char *tmp, buff[FN_REFLEN + 1];
  int select_db;
  uint warnings;

  memset(buff, 0, sizeof(buff));

  /*
    In case of quotes used, try to get the normalized db name.
  */
  if (get_quote_count(line) > 0) {
    if (normalize_dbname(line, buff, sizeof(buff))) return put_error(&mysql);
    tmp = buff;
  } else {
    strmake(buff, line, sizeof(buff) - 1);
    tmp = get_arg(buff, false);
  }

  if (!tmp || !*tmp) {
    put_info("USE must be followed by a database name", INFO_ERROR);
    return 0;
  }
  /*
    We need to recheck the current database, because it may change
    under our feet, for example if DROP DATABASE or RENAME DATABASE
    (latter one not yet available by the time the comment was written)
  */
  get_current_db();

  if (!current_db || cmp_database(charset_info, current_db, tmp)) {
    if (one_database) {
      skip_updates = true;
      select_db = 0;  // don't do mysql_select_db()
    } else
      select_db = 2;  // do mysql_select_db() and build_completion_hash()
  } else {
    /*
      USE to the current db specified.
      We do need to send mysql_select_db() to make server
      update database level privileges, which might
      change since last USE (see bug#10979).
      For performance purposes, we'll skip rebuilding of completion hash.
    */
    skip_updates = false;
    select_db = 1;  // do only mysql_select_db(), without completion
  }

  if (select_db) {
    /*
      reconnect once if connection is down or if connection was found to
      be down during query
    */
    if (!connected && reconnect())
      return opt_reconnect ? -1 : 1;  // Fatal error
    if (mysql_select_db(&mysql, tmp)) {
      if (mysql_errno(&mysql) != CR_SERVER_GONE_ERROR) return put_error(&mysql);

      if (reconnect()) return opt_reconnect ? -1 : 1;  // Fatal error
      if (mysql_select_db(&mysql, tmp)) return put_error(&mysql);
    }
    my_free(current_db);
    current_db = my_strdup(PSI_NOT_INSTRUMENTED, tmp, MYF(MY_WME));
#ifdef HAVE_READLINE
    if (select_db > 1) build_completion_hash(opt_rehash, true);
#endif
  }

  if (0 < (warnings = mysql_warning_count(&mysql))) {
    snprintf(buff, sizeof(buff), "Database changed, %u warning%s", warnings,
             warnings > 1 ? "s" : "");
    put_info(buff, INFO_INFO);
    if (show_warnings == 1) print_warnings();
  } else
    put_info("Database changed", INFO_INFO);
  return 0;
}

/**
  Normalize database name.

  @param [in] line           The command.
  @param [out] buff          Normalized db name.
  @param [in] buff_size      Buffer size.

  @return Operation status
      @retval 0    Success
      @retval 1    Failure

  @note Sometimes server normalizes the database names
        & APIs like mysql_select_db() expect normalized
        database names. Since it is difficult to perform
        the name conversion/normalization on the client
        side, this function tries to get the normalized
        dbname (indirectly) from the server.
*/

static int normalize_dbname(const char *line, char *buff, uint buff_size) {
  MYSQL_RES *res = nullptr;

  /* Send the "USE db" commmand to the server. */
  if (mysql_query(&mysql, line)) return 1;

  /*
    Now, get the normalized database name and store it
    into the buff.
  */
  if (!mysql_query(&mysql, "SELECT DATABASE()") &&
      (res = mysql_use_result(&mysql))) {
    MYSQL_ROW row = mysql_fetch_row(res);
    if (row && row[0]) {
      size_t len = strlen(row[0]);
      /* Make sure there is enough room to store the dbname. */
      if ((len > buff_size) || !memcpy(buff, row[0], len)) {
        mysql_free_result(res);
        return 1;
      }
    }
    mysql_free_result(res);
  }

  /* Restore the original database. */
  if (current_db && mysql_select_db(&mysql, current_db)) return 1;

  return 0;
}

static int com_warnings(String *buffer MY_ATTRIBUTE((unused)),
                        char *line MY_ATTRIBUTE((unused))) {
  show_warnings = true;
  put_info("Show warnings enabled.", INFO_INFO);
  return 0;
}

static int com_nowarnings(String *buffer MY_ATTRIBUTE((unused)),
                          char *line MY_ATTRIBUTE((unused))) {
  show_warnings = false;
  put_info("Show warnings disabled.", INFO_INFO);
  return 0;
}

/*
  Gets argument from a command on the command line. If get_next_arg is
  not defined, skips the command and returns the first argument. The
  line is modified by adding zero to the end of the argument. If
  get_next_arg is defined, then the function searches for end of string
  first, after found, returns the next argument and adds zero to the
  end. If you ever wish to use this feature, remember to initialize all
  items in the array to zero first.
*/

char *get_arg(char *line, bool get_next_arg) {
  char *ptr, *start;
  bool quoted = false, valid_arg = false;
  char qtype = 0;

  ptr = line;
  if (get_next_arg) {
    for (; *ptr; ptr++)
      ;
    if (*(ptr + 1)) ptr++;
  } else {
    /* skip leading white spaces */
    while (my_isspace(charset_info, *ptr)) ptr++;
    if (*ptr == '\\')  // short command was used
      ptr += 2;
    else
      while (*ptr && !my_isspace(charset_info, *ptr))  // skip command
        ptr++;
  }
  if (!*ptr) return NullS;
  while (my_isspace(charset_info, *ptr)) ptr++;
  if (*ptr == '\'' || *ptr == '\"' || *ptr == '`') {
    qtype = *ptr;
    quoted = true;
    ptr++;
  }
  for (start = ptr; *ptr; ptr++) {
    // if it is a quoted string do not remove backslash
    if (!quoted && *ptr == '\\' && ptr[1])  // escaped character
    {
      // Remove the backslash
      my_stpmov(ptr, ptr + 1);
    } else if ((!quoted && *ptr == ' ') || (quoted && *ptr == qtype)) {
      *ptr = 0;
      break;
    }
  }
  valid_arg = ptr != start;
  return valid_arg ? start : NullS;
}

/*
  Number of quotes present in the command's argument.
*/
static int get_quote_count(const char *line) {
  int quote_count = 0;
  const char *quote = line;

  while ((quote = strpbrk(quote, "'`\"")) != nullptr) {
    quote_count++;
    quote++;
  }

  return quote_count;
}

static int sql_real_connect(char *host, char *database, char *user,
                            char *password, uint silent) {
  if (connected) {
    connected = false;
#ifdef HAVE_SETNS
    if (opt_network_namespace) (void)release_network_namespace_resources();
#endif
    mysql_close(&mysql);
  }

  mysql_init(&mysql);
  if (init_connection_options(&mysql)) {
    (void)put_error(&mysql);
    (void)fflush(stdout);
    return ignore_errors ? -1 : 1;
  }
#ifdef _WIN32
  uint cnv_errors;
  String converted_database, converted_user;
  if (!my_charset_same(&my_charset_utf8mb4_bin, mysql.charset)) {
    /* Convert user and database from UTF8MB4 to connection character set */
    if (user) {
      converted_user.copy(user, strlen(user) + 1, &my_charset_utf8mb4_bin,
                          mysql.charset, &cnv_errors);
      user = (char *)converted_user.ptr();
    }
    if (database) {
      converted_database.copy(database, strlen(database) + 1,
                              &my_charset_utf8mb4_bin, mysql.charset,
                              &cnv_errors);
      database = (char *)converted_database.ptr();
    }
  }
#endif

#ifdef HAVE_SETNS
  if (opt_network_namespace && set_network_namespace(opt_network_namespace)) {
    if (!silent) {
      char msgbuf[PATH_MAX];
      snprintf(msgbuf, sizeof(msgbuf), "Network namespace error: %s",
               strerror(errno));
      put_info(msgbuf, INFO_ERROR);
    }

    return ignore_errors ? -1 : 1;  // Abort
  }
#endif

  if (!mysql_real_connect(&mysql, host, user, password, database,
                          opt_mysql_port, opt_mysql_unix_port,
                          connect_flag | CLIENT_MULTI_STATEMENTS)) {
#ifdef HAVE_SETNS
    if (opt_network_namespace) (void)restore_original_network_namespace();
#endif
    if (mysql_errno(&mysql) == ER_MUST_CHANGE_PASSWORD_LOGIN) {
      tee_fprintf(stdout,
                  "Please use --connect-expired-password option or "
                  "invoke mysql in interactive mode.\n");
      return ignore_errors ? -1 : 1;
    }
    if (!silent || (mysql_errno(&mysql) != CR_CONN_HOST_ERROR &&
                    mysql_errno(&mysql) != CR_CONNECTION_ERROR)) {
      (void)put_error(&mysql);
      (void)fflush(stdout);
      return ignore_errors ? -1 : 1;  // Abort
    }
    return -1;  // Retryable
  }

#ifdef HAVE_SETNS
  if (opt_network_namespace && restore_original_network_namespace()) {
    if (!silent) {
      char msgbuf[PATH_MAX];
      snprintf(msgbuf, sizeof(msgbuf), "Network namespace error: %s",
               strerror(errno));
      put_info(msgbuf, INFO_ERROR);
    }

    return ignore_errors ? -1 : 1;  // Abort
  }
#endif
  if (!mysql_get_ssl_session_reused(&mysql)) {
#if defined(HAVE_OPENSSL)
    if (ssl_session) SSL_SESSION_free((SSL_SESSION *)ssl_session);
#endif
    ssl_session = mysql_get_ssl_session(&mysql);
    if (ssl_session == nullptr)
      DBUG_PRINT("error", ("unable to save SSL session"));
  }
  ssl_context = mysql_take_ssl_context_ownership(&mysql);

#ifdef _WIN32
  /* Convert --execute buffer from UTF8MB4 to connection character set */
  if (!execute_buffer_conversion_done++ && status.line_buff &&
      !status.line_buff->file && /* Convert only -e buffer, not real file */
      status.line_buff->buffer < status.line_buff->end && /* Non-empty */
      !my_charset_same(&my_charset_utf8mb4_bin, mysql.charset)) {
    String tmp;
    size_t len = status.line_buff->end - status.line_buff->buffer;
    uint dummy_errors;
    /*
      Don't convert trailing '\n' character - it was appended during
      last batch_readline_command() call.
      Oherwise we'll get an extra line, which makes some tests fail.
    */
    if (status.line_buff->buffer[len - 1] == '\n') len--;
    if (tmp.copy(status.line_buff->buffer, len, &my_charset_utf8mb4_bin,
                 mysql.charset, &dummy_errors))
      return 1;

    /* Free the old line buffer */
    batch_readline_end(status.line_buff);

    /* Re-initialize line buffer from the converted string */
    if (!(status.line_buff =
              batch_readline_command(NULL, (char *)tmp.c_ptr_safe())))
      return 1;
  }
#endif /* _WIN32 */

  charset_info = mysql.charset;

  connected = true;
  mysql.reconnect = debug_info_flag;  // We want to know if this happens
#ifdef HAVE_READLINE
  build_completion_hash(opt_rehash, true);
#endif
  return 0;
}

/* Initialize options for the given connection handle. */
static bool init_connection_options(MYSQL *mysql) {
  bool handle_expired = (opt_connect_expired_password || !status.batch);

  // Using the compression_lib connection attribute requires connection
  // attributes be reset early. Parsing the command arguments could add a new
  // connection attribute
  mysql_options(mysql, MYSQL_OPT_CONNECT_ATTR_RESET, nullptr);

  if (opt_init_command)
    mysql_options(mysql, MYSQL_INIT_COMMAND, opt_init_command);

  if (opt_connect_timeout) {
    uint timeout = opt_connect_timeout;
    mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT, (char *)&timeout);
  }

  if (opt_bind_addr) mysql_options(mysql, MYSQL_OPT_BIND, opt_bind_addr);

  if (opt_compress) mysql_options(mysql, MYSQL_OPT_COMPRESS, NullS);
  if (opt_compress_algorithm)
    mysql_options(mysql, MYSQL_OPT_COMPRESSION_ALGORITHMS,
                  opt_compress_algorithm);

  mysql_options(mysql, MYSQL_OPT_ZSTD_COMPRESSION_LEVEL,
                &opt_zstd_compress_level);

  if (using_opt_local_infile)
    mysql_options(mysql, MYSQL_OPT_LOCAL_INFILE, (char *)&opt_local_infile);

  if (SSL_SET_OPTIONS(mysql)) {
    tee_fprintf(stdout, "%s", SSL_SET_OPTIONS_ERROR);
    return true;
  }

  if (ssl_context) {
    mysql_options(mysql, MYSQL_OPT_SSL_CONTEXT, ssl_context);
  }

  if (ssl_session)
    mysql_options4(mysql, MYSQL_OPT_SSL_SESSION, ssl_session, (void *)false);

  if (opt_protocol)
    mysql_options(mysql, MYSQL_OPT_PROTOCOL, (char *)&opt_protocol);

#if defined(_WIN32)
  if (shared_memory_base_name)
    mysql_options(mysql, MYSQL_SHARED_MEMORY_BASE_NAME,
                  shared_memory_base_name);
#endif

  if (safe_updates) {
    char init_command[100];
    sprintf(init_command,
            "SET SQL_SAFE_UPDATES=1,SQL_SELECT_LIMIT=%lu,MAX_JOIN_SIZE=%lu",
            select_limit, max_join_size);
    mysql_options(mysql, MYSQL_INIT_COMMAND, init_command);
  }

  mysql_set_character_set(mysql, default_charset);

  if (opt_plugin_dir && *opt_plugin_dir)
    mysql_options(mysql, MYSQL_PLUGIN_DIR, opt_plugin_dir);

  if (opt_default_auth && *opt_default_auth)
    mysql_options(mysql, MYSQL_DEFAULT_AUTH, opt_default_auth);

  set_server_public_key(mysql);

  set_get_server_public_key_option(mysql);

  if (using_opt_enable_cleartext_plugin)
    mysql_options(mysql, MYSQL_ENABLE_CLEARTEXT_PLUGIN,
                  (char *)&opt_enable_cleartext_plugin);

  mysql_options4(mysql, MYSQL_OPT_CONNECT_ATTR_ADD, "program_name", "mysql");
  if (current_os_user)
    mysql_options4(mysql, MYSQL_OPT_CONNECT_ATTR_ADD, "os_user",
                   current_os_user);
  if (current_os_sudouser)
    mysql_options4(mysql, MYSQL_OPT_CONNECT_ATTR_ADD, "os_sudouser",
                   current_os_sudouser);

  mysql_options(mysql, MYSQL_OPT_CAN_HANDLE_EXPIRED_PASSWORDS, &handle_expired);

  return false;
}

static int sql_connect(char *host, char *database, char *user, char *password,
                       uint silent) {
  bool message = false;
  uint count = 0;
  int error;
  for (;;) {
    if ((error = sql_real_connect(host, database, user, password, wait_flag)) >=
        0) {
      if (count) {
        tee_fputs("\n", stderr);
        (void)fflush(stderr);
      }
      return error;
    }
    if (!wait_flag) return ignore_errors ? -1 : 1;
    if (!message && !silent) {
      message = true;
      tee_fputs("Waiting", stderr);
      (void)fflush(stderr);
    }
    (void)sleep(wait_time);
    if (!silent) {
      putc('.', stderr);
      (void)fflush(stderr);
      count++;
    }
  }
}

static int com_status(String *buffer MY_ATTRIBUTE((unused)),
                      char *line MY_ATTRIBUTE((unused))) {
  const char *status_str;
  char buff[40];
  ulonglong id;
  MYSQL_RES *result = nullptr;

  if (mysql_real_query_for_lazy(
          STRING_WITH_LEN("select DATABASE(), USER() limit 1")))
    return 0;

  tee_puts("--------------", stdout);
  usage(1); /* Print version */
  tee_fprintf(stdout, "\nConnection id:\t\t%lu\n", mysql_thread_id(&mysql));
  /*
    Don't remove "limit 1",
    it is protection againts SQL_SELECT_LIMIT=0
  */
  if (!mysql_store_result_for_lazy(&result)) {
    MYSQL_ROW cur = mysql_fetch_row(result);
    if (cur) {
      tee_fprintf(stdout, "Current database:\t%s\n", cur[0] ? cur[0] : "");
      tee_fprintf(stdout, "Current user:\t\t%s\n", cur[1]);
    }
    mysql_free_result(result);
  }

  if ((status_str = mysql_get_ssl_cipher(&mysql)))
    tee_fprintf(stdout, "SSL:\t\t\tCipher in use is %s\n", status_str);
  else
    tee_puts("SSL:\t\t\tNot in use", stdout);

  if (skip_updates) {
    tee_fprintf(stdout, "\nAll updates ignored to this database\n");
  }
#ifdef USE_POPEN
  tee_fprintf(stdout, "Current pager:\t\t%s\n", pager);
  tee_fprintf(stdout, "Using outfile:\t\t'%s'\n", opt_outfile ? outfile : "");
#endif
  tee_fprintf(stdout, "Using delimiter:\t%s\n", delimiter);
  tee_fprintf(stdout, "Server version:\t\t%s\n", server_version_string(&mysql));
  tee_fprintf(stdout, "Protocol version:\t%d\n", mysql_get_proto_info(&mysql));
  tee_fprintf(stdout, "Connection:\t\t%s\n", mysql_get_host_info(&mysql));
  if ((id = mysql_insert_id(&mysql)))
    tee_fprintf(stdout, "Insert id:\t\t%s\n", llstr(id, buff));

  /* "limit 1" is protection against SQL_SELECT_LIMIT=0 */
  if (mysql_real_query_for_lazy(STRING_WITH_LEN(
          "select @@character_set_client, @@character_set_connection, "
          "@@character_set_server, @@character_set_database limit 1"))) {
    if (mysql_errno(&mysql) == CR_SERVER_GONE_ERROR) return 0;
  }
  if (!mysql_store_result_for_lazy(&result)) {
    MYSQL_ROW cur = mysql_fetch_row(result);
    if (cur) {
      tee_fprintf(stdout, "Server characterset:\t%s\n", cur[2] ? cur[2] : "");
      tee_fprintf(stdout, "Db     characterset:\t%s\n", cur[3] ? cur[3] : "");
      tee_fprintf(stdout, "Client characterset:\t%s\n", cur[0] ? cur[0] : "");
      tee_fprintf(stdout, "Conn.  characterset:\t%s\n", cur[1] ? cur[1] : "");
    }
    mysql_free_result(result);
  } else {
    /* Probably pre-4.1 server */
    tee_fprintf(stdout, "Client characterset:\t%s\n", charset_info->csname);
    tee_fprintf(stdout, "Server characterset:\t%s\n", mysql.charset->csname);
  }

  if (strstr(mysql_get_host_info(&mysql), "TCP/IP") || !mysql.unix_socket)
    tee_fprintf(stdout, "TCP port:\t\t%d\n", mysql.port);
  else
    tee_fprintf(stdout, "UNIX socket:\t\t%s\n", mysql.unix_socket);
  if (mysql.net.compress) tee_fprintf(stdout, "Protocol:\t\tCompressed\n");
  if (opt_binhex) tee_fprintf(stdout, "Binary data as:\t\tHexadecimal\n");

  if ((status_str = mysql_stat(&mysql)) && !mysql_error(&mysql)[0]) {
    ulong sec;
    const char *pos = strchr(status_str, ' ');
    /* print label */
    tee_fprintf(stdout, "%.*s\t\t\t", (int)(pos - status_str), status_str);
    if ((status_str = str2int(pos, 10, 0, LONG_MAX, (long *)&sec))) {
      nice_time((double)sec, buff, false);
      tee_puts(buff, stdout);                  /* print nice time */
      while (*status_str == ' ') status_str++; /* to next info */
      tee_putc('\n', stdout);
      tee_puts(status_str, stdout);
    }
  }
  if (safe_updates) {
    tee_fprintf(stdout, "\nNote that you are running in safe_update_mode:\n");
    tee_fprintf(stdout,
                "\
UPDATEs and DELETEs that don't use a key in the WHERE clause are not allowed.\n\
(One can force an UPDATE/DELETE by adding LIMIT # at the end of the command.)\n\
SELECT has an automatic 'LIMIT %lu' if LIMIT is not used.\n\
Max number of examined row combination in a join is set to: %lu\n\n",
                select_limit, max_join_size);
  }
  tee_puts("--------------\n", stdout);
  return 0;
}

static const char *server_version_string(MYSQL *con) {
  /* Only one thread calls this, so no synchronization is needed */
  if (server_version == nullptr) {
    /* version, space, comment, \0 */
    size_t len = strlen(mysql_get_server_info(con)) + 1;

    if ((server_version =
             (char *)my_malloc(PSI_NOT_INSTRUMENTED, len, MYF(MY_WME))))
      my_stpcpy(server_version, mysql_get_server_info(con));
  }

  return server_version ? server_version : "";
}

static int put_info(const char *str, INFO_TYPE info_type, uint error,
                    const char *sqlstate) {
  FILE *file = (info_type == INFO_ERROR ? stderr : stdout);
  static int inited = 0;

  if (status.batch) {
    if (info_type == INFO_ERROR) {
      (void)fflush(stdout);  // flush stdout before stderr
      (void)fflush(file);
      fprintf(file, "ERROR");
      if (error) {
        if (sqlstate)
          (void)fprintf(file, " %d (%s)", error, sqlstate);
        else
          (void)fprintf(file, " %d", error);
      }
      if (status.query_start_line && line_numbers) {
        (void)fprintf(file, " at line %lu", status.query_start_line);
        if (status.file_name)
          (void)fprintf(file, " in file: '%s'", status.file_name);
      }
      (void)fprintf(file, ": %s\n", str);
      (void)fflush(file);
      if (!ignore_errors) return 1;
    } else if (info_type == INFO_RESULT && verbose > 1)
      tee_puts(str, file);
    if (unbuffered) fflush(file);
    return info_type == INFO_ERROR ? -1 : 0;
  }
  if (!opt_silent || info_type == INFO_ERROR) {
    if (!inited) {
      inited = 1;
    }
    if (info_type == INFO_ERROR) {
      if (!opt_nobeep) putchar('\a'); /* This should make a bell */
      if (error) {
        if (sqlstate)
          (void)tee_fprintf(file, "ERROR %d (%s): ", error, sqlstate);
        else
          (void)tee_fprintf(file, "ERROR %d: ", error);
      } else
        tee_puts("ERROR: ", file);
    }
    (void)tee_puts(str, file);
  }
  if (unbuffered) fflush(file);
  return info_type == INFO_ERROR ? -1 : 0;
}

static int put_error(MYSQL *con) {
  return put_info(mysql_error(con), INFO_ERROR, mysql_errno(con),
                  mysql_sqlstate(con));
}

static void remove_cntrl(String *buffer) {
  const char *start = buffer->ptr();
  const char *end = start + buffer->length();
  while (start < end && !my_isgraph(charset_info, end[-1])) end--;
  buffer->length((uint)(end - start));
}

/**
  Write data to a stream.
  Various modes, corresponding to --tab, --xml, --raw parameters,
  are supported.

  @param file   Stream to write to
  @param s      String to write
  @param slen   String length
  @param flags  Flags for --tab, --xml, --raw.
*/
void tee_write(FILE *file, const char *s, size_t slen, int flags) {
#ifdef _WIN32
  bool is_console = my_win_is_console_cached(file);
#endif
  const char *se;
  for (se = s + slen; s < se; s++) {
    const char *t;

    if (flags & MY_PRINT_MB) {
      int mblen;
      if (use_mb(charset_info) && (mblen = my_ismbchar(charset_info, s, se))) {
#ifdef _WIN32
        if (is_console)
          my_win_console_write(charset_info, s, mblen);
        else
#endif
            if (fwrite(s, 1, mblen, file) != (size_t)mblen) {
          perror("fwrite");
        }
        if (opt_outfile) {
          if (fwrite(s, 1, mblen, OUTFILE) != (size_t)mblen) {
            perror("fwrite");
          }
        }
        s += mblen - 1;
        continue;
      }
    }

    if ((flags & MY_PRINT_XML) && (t = array_value(xmlmeta, *s)))
      tee_fputs(t, file);
    else if ((flags & MY_PRINT_SPS_0) && *s == '\0')
      tee_putc((int)' ', file);  // This makes everything hard
    else if ((flags & MY_PRINT_ESC_0) && *s == '\0')
      tee_fputs("\\0", file);  // This makes everything hard
    else if ((flags & MY_PRINT_CTRL) && *s == '\t')
      tee_fputs("\\t", file);  // This would destroy tab format
    else if ((flags & MY_PRINT_CTRL) && *s == '\n')
      tee_fputs("\\n", file);  // This too
    else if ((flags & MY_PRINT_CTRL) && *s == '\\')
      tee_fputs("\\\\", file);
    else {
#ifdef _WIN32
      if (is_console)
        my_win_console_putc(charset_info, (int)*s);
      else
#endif
        putc((int)*s, file);
      if (opt_outfile) putc((int)*s, OUTFILE);
    }
  }
}

void tee_fprintf(FILE *file, const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
#ifdef _WIN32
  if (my_win_is_console_cached(file))
    my_win_console_vfprintf(charset_info, fmt, args);
  else
#endif
    (void)vfprintf(file, fmt, args);
  va_end(args);

  if (opt_outfile) {
    va_start(args, fmt);
    (void)vfprintf(OUTFILE, fmt, args);
    va_end(args);
  }
}

/*
  Write a 0-terminated string to file and OUTFILE.
  TODO: possibly it's nice to have a version with length some day,
  e.g. tee_fnputs(s, slen, file),
  to print numerous ASCII constant strings among mysql.cc
  code, to avoid strlen(s) in my_win_console_fputs().
*/
void tee_fputs(const char *s, FILE *file) {
#ifdef _WIN32
  if (my_win_is_console_cached(file))
    my_win_console_fputs(charset_info, s);
  else
#endif
    fputs(s, file);
  if (opt_outfile) fputs(s, OUTFILE);
}

void tee_puts(const char *s, FILE *file) {
  tee_fputs(s, file);
  tee_putc('\n', file);
}

void tee_putc(int c, FILE *file) {
#ifdef _WIN32
  if (my_win_is_console_cached(file))
    my_win_console_putc(charset_info, c);
  else
#endif
    putc(c, file);
  if (opt_outfile) putc(c, OUTFILE);
}

#if defined(_WIN32)
#include <time.h>
#else
#include <sys/times.h>
#ifdef _SC_CLK_TCK  // For mit-pthreads
#undef CLOCKS_PER_SEC
#define CLOCKS_PER_SEC (sysconf(_SC_CLK_TCK))
#endif
#endif

static ulong start_timer(void) {
#if defined(_WIN32)
  return clock();
#else
  struct tms tms_tmp;
  return times(&tms_tmp);
#endif
}

/**
  Write as many as 52+1 bytes to buff, in the form of a legible duration of
  time.

  len("4294967296 days, 23 hours, 59 minutes, 60.00 seconds")  ->  52
*/
static void nice_time(double sec, char *buff, bool part_second) {
  ulong tmp;
  if (sec >= 3600.0 * 24) {
    tmp = (ulong)floor(sec / (3600.0 * 24));
    sec -= 3600.0 * 24 * tmp;
    buff = longlong10_to_str(tmp, buff, 10);
    buff = my_stpcpy(buff, tmp > 1 ? " days " : " day ");
  }
  if (sec >= 3600.0) {
    tmp = (ulong)floor(sec / 3600.0);
    sec -= 3600.0 * tmp;
    buff = longlong10_to_str(tmp, buff, 10);
    buff = my_stpcpy(buff, tmp > 1 ? " hours " : " hour ");
  }
  if (sec >= 60.0) {
    tmp = (ulong)floor(sec / 60.0);
    sec -= 60.0 * tmp;
    buff = longlong10_to_str(tmp, buff, 10);
    buff = my_stpcpy(buff, " min ");
  }
  if (part_second)
    sprintf(buff, "%.2f sec", sec);
  else
    sprintf(buff, "%d sec", (int)sec);
}

static void end_timer(ulong start_time, char *buff) {
  nice_time((double)(start_timer() - start_time) / CLOCKS_PER_SEC, buff, true);
}

static void mysql_end_timer(ulong start_time, char *buff) {
  buff[0] = ' ';
  buff[1] = '(';
  end_timer(start_time, buff + 2);
  my_stpcpy(strend(buff), ")");
}

static const char *construct_prompt() {
  processed_prompt.mem_free();    // Erase the old prompt
  time_t lclock = time(nullptr);  // Get the date struct
  struct tm *t = localtime(&lclock);

  /* parse thru the settings for the prompt */
  for (char *c = current_prompt; *c; c++) {
    if (*c != PROMPT_CHAR)
      processed_prompt.append(*c);
    else {
      switch (*++c) {
        case '\0':
          c--;  // stop it from going beyond if ends with %
          break;
        case 'c':
          add_int_to_prompt(++prompt_counter);
          break;
        case 'C':
          add_int_to_prompt(mysql_thread_id(&mysql));
          break;
        case 'v':
          if (connected)
            processed_prompt.append(mysql_get_server_info(&mysql));
          else
            processed_prompt.append("not_connected");
          break;
        case 'd':
          processed_prompt.append(current_db ? current_db : "(none)");
          break;
        case 'h': {
          const char *prompt;
          prompt = connected ? mysql_get_host_info(&mysql) : "not_connected";
          if (strstr(prompt, "Localhost"))
            processed_prompt.append("localhost");
          else {
            const char *end = strcend(prompt, ' ');
            processed_prompt.append(prompt, (uint)(end - prompt));
          }
          break;
        }
        case 'p': {
          if (!connected) {
            processed_prompt.append("not_connected");
            break;
          }

          const char *host_info = mysql_get_host_info(&mysql);
          if (strstr(host_info, "memory")) {
            processed_prompt.append(mysql.host);
          } else if (strstr(host_info, "TCP/IP") || !mysql.unix_socket)
            add_int_to_prompt(mysql.port);
          else {
            char *pos = strrchr(mysql.unix_socket, '/');
            processed_prompt.append(pos ? pos + 1 : mysql.unix_socket);
          }
        } break;
        case 'U':
          if (!full_username) init_username();
          processed_prompt.append(
              full_username ? full_username
                            : (current_user ? current_user : "(unknown)"));
          break;
        case 'u':
          if (!full_username) init_username();
          processed_prompt.append(
              part_username ? part_username
                            : (current_user ? current_user : "(unknown)"));
          break;
        case PROMPT_CHAR:
          processed_prompt.append(PROMPT_CHAR);
          break;
        case 'n':
          processed_prompt.append('\n');
          break;
        case ' ':
        case '_':
          processed_prompt.append(' ');
          break;
        case 'R':
          if (t->tm_hour < 10) processed_prompt.append('0');
          add_int_to_prompt(t->tm_hour);
          break;
        case 'r':
          int getHour;
          getHour = t->tm_hour % 12;
          if (getHour == 0) getHour = 12;
          if (getHour < 10) processed_prompt.append('0');
          add_int_to_prompt(getHour);
          break;
        case 'm':
          if (t->tm_min < 10) processed_prompt.append('0');
          add_int_to_prompt(t->tm_min);
          break;
        case 'y':
          int getYear;
          getYear = t->tm_year % 100;
          if (getYear < 10) processed_prompt.append('0');
          add_int_to_prompt(getYear);
          break;
        case 'Y':
          add_int_to_prompt(t->tm_year + 1900);
          break;
        case 'D':
          char *dateTime;
          dateTime = ctime(&lclock);
          processed_prompt.append(strtok(dateTime, "\n"));
          break;
        case 's':
          if (t->tm_sec < 10) processed_prompt.append('0');
          add_int_to_prompt(t->tm_sec);
          break;
        case 'w':
          processed_prompt.append(day_names[t->tm_wday]);
          break;
        case 'P':
          processed_prompt.append(t->tm_hour < 12 ? "am" : "pm");
          break;
        case 'o':
          add_int_to_prompt(t->tm_mon + 1);
          break;
        case 'O':
          processed_prompt.append(month_names[t->tm_mon]);
          break;
        case '\'':
          processed_prompt.append("'");
          break;
        case '"':
          processed_prompt.append('"');
          break;
        case 'S':
          processed_prompt.append(';');
          break;
        case 't':
          processed_prompt.append('\t');
          break;
        case 'l':
          processed_prompt.append(delimiter_str);
          break;
        default:
          processed_prompt.append(c);
      }
    }
  }
  processed_prompt.append('\0');
  return processed_prompt.ptr();
}

static void add_int_to_prompt(int toadd) {
  processed_prompt.append_longlong(toadd);
}

static void init_username() {
  my_free(full_username);
  my_free(part_username);

  MYSQL_RES *result = nullptr;
  if (!mysql_query(&mysql, "select USER()") &&
      (result = mysql_use_result(&mysql))) {
    MYSQL_ROW cur = mysql_fetch_row(result);
    full_username = my_strdup(PSI_NOT_INSTRUMENTED, cur[0], MYF(MY_WME));
    part_username =
        my_strdup(PSI_NOT_INSTRUMENTED, strtok(cur[0], "@"), MYF(MY_WME));
    (void)mysql_fetch_row(result);  // Read eof
  }
}

// Get the current OS user name.
static void get_current_os_user() {
  const char *user;

#ifdef _WIN32
  char buf[255];
  WCHAR wbuf[255];
  DWORD wbuf_len = sizeof(wbuf) / sizeof(WCHAR);
  size_t len;
  uint dummy_errors;

  if (GetUserNameW(wbuf, &wbuf_len)) {
    len = my_convert(buf, sizeof(buf) - 1, charset_info, (char *)wbuf,
                     wbuf_len * sizeof(WCHAR), &my_charset_utf16le_bin,
                     &dummy_errors);
    buf[len] = 0;
    user = buf;
  } else {
    user = "UNKNOWN USER";
  }
#else
#ifdef HAVE_GETPWUID
  struct passwd *pw;

  if ((pw = getpwuid(geteuid())) != nullptr)
    user = pw->pw_name;
  else
#endif
      if (!(user = getenv("USER")) && !(user = getenv("LOGNAME")) &&
          !(user = getenv("LOGIN")))
    user = "UNKNOWN USER";
#endif /* _WIN32 */
  current_os_user = my_strdup(PSI_NOT_INSTRUMENTED, user, MYF(MY_WME));
  return;
}

// Get the current OS sudo user name (only for non-Windows platforms).
static void get_current_os_sudouser() {
#ifndef _WIN32
  if (getenv("SUDO_USER"))
    current_os_sudouser =
        my_strdup(PSI_NOT_INSTRUMENTED, getenv("SUDO_USER"), MYF(MY_WME));
#endif /* !_WIN32 */
  return;
}

static int com_prompt(String *buffer MY_ATTRIBUTE((unused)), char *line) {
  char *ptr = strchr(line, ' ');
  prompt_counter = 0;
  my_free(current_prompt);
  current_prompt = my_strdup(PSI_NOT_INSTRUMENTED,
                             ptr ? ptr + 1 : default_prompt, MYF(MY_WME));
  if (!ptr)
    tee_fprintf(stdout, "Returning to default PROMPT of %s\n", default_prompt);
  else
    tee_fprintf(stdout, "PROMPT set to '%s'\n", current_prompt);
  return 0;
}

static int com_resetconnection(String *buffer MY_ATTRIBUTE((unused)),
                               char *line MY_ATTRIBUTE((unused))) {
  int error;
  error = mysql_reset_connection(&mysql);
  if (error) {
    if (status.batch) return 0;
    return put_error(&mysql);
  }
  return error;
}
