/*
   Copyright (c) 2005, 2020, Oracle and/or its affiliates. All rights reserved.

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

/*
  MySQL Slap

  A simple program designed to work as if multiple clients querying the
database, then reporting the timing of each stage.

  MySQL slap runs three stages:
  1) Create schema,table, and optionally any SP or data you want to beign
     the test with. (single client)
  2) Load test (many clients)
  3) Cleanup (disconnection, drop table if specified, single client)

  Examples:

  Supply your own create and query SQL statements, with 50 clients
  querying (200 selects for each):

    mysqlslap --delimiter=";" \
              --create="CREATE TABLE A (a int);INSERT INTO A VALUES (23)" \
              --query="SELECT * FROM A" --concurrency=50 --iterations=200

  Let the program build the query SQL statement with a table of two int
  columns, three varchar columns, five clients querying (20 times each),
  don't create the table or insert the data (using the previous test's
  schema and data):

    mysqlslap --concurrency=5 --iterations=20 \
              --number-int-cols=2 --number-char-cols=3 \
              --auto-generate-sql

  Tell the program to load the create, insert and query SQL statements from
  the specified files, where the create.sql file has multiple table creation
  statements delimited by ';' and multiple insert statements delimited by ';'.
  The --query file will have multiple queries delimited by ';', run all the
  load statements, and then run all the queries in the query file
  with five clients (five times each):

    mysqlslap --concurrency=5 \
              --iterations=5 --query=query.sql --create=create.sql \
              --delimiter=";"

TODO:
  Add language for better tests
  String length for files and those put on the command line are not
    setup to handle binary data.
  More stats
  Break up tests and run them on multiple hosts at once.
  Allow output to be fed into a database directly.

*/

#define HUGE_STRING_LENGTH 8196
#define RAND_STRING_SIZE 126

/* Types */
#define SELECT_TYPE 0
#define UPDATE_TYPE 1
#define INSERT_TYPE 2
#define UPDATE_TYPE_REQUIRES_PREFIX 3
#define CREATE_TABLE_TYPE 4
#define SELECT_TYPE_REQUIRES_PREFIX 5
#define DELETE_TYPE_REQUIRES_PREFIX 6

#include "my_config.h"

#include <ctype.h>
#include <fcntl.h>
#include <mysqld_error.h>
#include <signal.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/types.h>

#include "caching_sha2_passwordopt-vars.h"
#include "my_dir.h"
#include "sslopt-vars.h"
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <stdio.h>
#include <time.h>

#include "client/client_priv.h"
#include "compression.h"
#include "my_alloc.h"
#include "my_dbug.h"
#include "my_default.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "my_systime.h"
#include "mysql/service_mysql_alloc.h"
#include "print_version.h"
#include "thr_cond.h"
#include "typelib.h"
#include "welcome_copyright_notice.h" /* ORACLE_WELCOME_COPYRIGHT_NOTICE */

#ifdef _WIN32
#define srandom srand
#define random rand
#endif

#if defined(_WIN32)
static char *shared_memory_base_name = 0;
#endif

/* Global Thread counter */
uint thread_counter;
native_mutex_t counter_mutex;
native_cond_t count_threshold;
uint master_wakeup;
native_mutex_t sleeper_mutex;
native_cond_t sleep_threshold;

char **primary_keys;
unsigned long long primary_keys_number_of;

static char *host = nullptr, *opt_password = nullptr,
            *user_supplied_query = nullptr,
            *user_supplied_pre_statements = nullptr,
            *user_supplied_post_statements = nullptr, *default_engine = nullptr,
            *pre_system = nullptr, *post_system = nullptr,
            *opt_mysql_unix_port = nullptr;
static const char *user = nullptr;
static char *opt_plugin_dir = nullptr, *opt_default_auth = nullptr;
static uint opt_enable_cleartext_plugin = 0;
static bool using_opt_enable_cleartext_plugin = false;

const char *delimiter = "\n";

const char *create_schema_string = "mysqlslap";

static bool opt_preserve = true, opt_no_drop = false;
static bool debug_info_flag = false, debug_check_flag = false;
static bool opt_only_print = false;
static bool opt_compress = false, tty_password = false, opt_silent = false,
            auto_generate_sql_autoincrement = false,
            auto_generate_sql_guid_primary = false, auto_generate_sql = false;
const char *auto_generate_sql_type = "mixed";

static uint opt_zstd_compress_level = default_zstd_compression_level;
static char *opt_compress_algorithm = nullptr;

static unsigned long connect_flags =
    CLIENT_MULTI_RESULTS | CLIENT_MULTI_STATEMENTS | CLIENT_REMEMBER_OPTIONS;

static int verbose;
static uint commit_rate;
static uint detach_rate;
const char *num_int_cols_opt;
const char *num_char_cols_opt;

/* Yes, we do set defaults here */
static unsigned int num_int_cols = 1;
static unsigned int num_char_cols = 1;
static unsigned int num_int_cols_index = 0;
static unsigned int num_char_cols_index = 0;
static unsigned int iterations;
static uint my_end_arg = 0;
static const char *default_charset = MYSQL_DEFAULT_CHARSET_NAME;
static ulonglong actual_queries = 0;
static ulonglong auto_actual_queries;
static ulonglong auto_generate_sql_unique_write_number;
static ulonglong auto_generate_sql_unique_query_number;
static unsigned int auto_generate_sql_secondary_indexes;
static ulonglong num_of_query;
static ulonglong auto_generate_sql_number;
static const char *sql_mode = nullptr;
const char *concurrency_str = nullptr;
static char *create_string;
uint *concurrency;

const char *default_dbug_option = "d:t:o,/tmp/mysqlslap.trace";
const char *opt_csv_str;
File csv_file;

static uint opt_protocol = 0;

static int get_options(int *argc, char ***argv);
static uint opt_mysql_port = 0;

static const char *load_default_groups[] = {"mysqlslap", "client", nullptr};

typedef struct statement statement;

struct statement {
  char *string;
  size_t length;
  unsigned char type;
  char *option;
  size_t option_length;
  statement *next;
};

typedef struct option_string option_string;

struct option_string {
  char *string;
  size_t length;
  char *option;
  size_t option_length;
  option_string *next;
};

typedef struct stats stats;

struct stats {
  long int timing;
  uint users;
  unsigned long long rows;
};

typedef struct thread_context thread_context;

struct thread_context {
  statement *stmt;
  ulonglong limit;
};

typedef struct conclusions conclusions;

struct conclusions {
  char *engine;
  long int avg_timing;
  long int max_timing;
  long int min_timing;
  uint users;
  unsigned long long avg_rows;
  /* The following are not used yet */
  unsigned long long max_rows;
  unsigned long long min_rows;
};

static option_string *engine_options = nullptr;
static statement *pre_statements = nullptr;
static statement *post_statements = nullptr;
static statement *create_statements = nullptr, *query_statements = nullptr;

/* Prototypes */
void print_conclusions(conclusions *con);
void print_conclusions_csv(conclusions *con);
void generate_stats(conclusions *con, option_string *eng, stats *sptr);
uint parse_comma(const char *string, uint **range);
uint parse_delimiter(const char *script, statement **stmt, char delm);
int parse_option(const char *origin, option_string **stmt, char delm);
static int drop_schema(MYSQL *mysql, const char *db);
size_t get_random_string(char *buf);
static statement *build_table_string(void);
static statement *build_insert_string(void);
static statement *build_update_string(void);
static statement *build_select_string(bool key);
static int generate_primary_key_list(MYSQL *mysql, option_string *engine_stmt);
static int drop_primary_key_list(void);
static int create_schema(MYSQL *mysql, const char *db, statement *stmt,
                         option_string *engine_stmt);
static void set_sql_mode(MYSQL *mysql);
static int run_scheduler(stats *sptr, statement *stmts, uint concur,
                         ulonglong limit);
extern "C" void *run_task(void *p);
void statement_cleanup(statement *stmt);
void option_cleanup(option_string *stmt);
void concurrency_loop(MYSQL *mysql, uint current, option_string *eptr);
static int run_statements(MYSQL *mysql, statement *stmt);
int slap_connect(MYSQL *mysql);
static int run_query(MYSQL *mysql, const char *query, size_t len);

static const char ALPHANUMERICS[] =
    "0123456789ABCDEFGHIJKLMNOPQRSTWXYZabcdefghijklmnopqrstuvwxyz";

#define ALPHANUMERICS_SIZE (sizeof(ALPHANUMERICS) - 1)

static long int timedif(struct timeval a, struct timeval b) {
  int us, s;

  us = a.tv_usec - b.tv_usec;
  us /= 1000;
  s = a.tv_sec - b.tv_sec;
  s *= 1000;
  return s + us;
}

#ifdef _WIN32
static int gettimeofday(struct timeval *tp, void *tzp) {
  unsigned int ticks;
  ticks = GetTickCount();
  tp->tv_usec = ticks * 1000;
  tp->tv_sec = ticks / 1000;

  return 0;
}
#endif

int main(int argc, char **argv) {
  MYSQL mysql{};
  option_string *eptr;

  MY_INIT(argv[0]);

  my_getopt_use_args_separator = true;
  MEM_ROOT alloc{PSI_NOT_INSTRUMENTED, 512};
  if (load_defaults("my", load_default_groups, &argc, &argv, &alloc)) {
    my_end(0);
    return EXIT_FAILURE;
  }
  my_getopt_use_args_separator = false;
  if (get_options(&argc, &argv)) {
    my_end(0);
    return EXIT_FAILURE;
  }

  /* Seed the random number generator if we will be using it. */
  if (auto_generate_sql) srandom((uint)time(nullptr));

  if (argc > 2) {
    fprintf(stderr, "%s: Too many arguments\n", my_progname);
    my_end(0);
    return EXIT_FAILURE;
  }
  mysql_init(&mysql);
  if (opt_compress) mysql_options(&mysql, MYSQL_OPT_COMPRESS, NullS);

  if (opt_compress_algorithm)
    mysql_options(&mysql, MYSQL_OPT_COMPRESSION_ALGORITHMS,
                  opt_compress_algorithm);

  mysql_options(&mysql, MYSQL_OPT_ZSTD_COMPRESSION_LEVEL,
                &opt_zstd_compress_level);

  if (SSL_SET_OPTIONS(&mysql)) {
    fprintf(stderr, "%s", SSL_SET_OPTIONS_ERROR);
    return EXIT_FAILURE;
  }
  if (opt_protocol)
    mysql_options(&mysql, MYSQL_OPT_PROTOCOL, (char *)&opt_protocol);
#if defined(_WIN32)
  if (shared_memory_base_name)
    mysql_options(&mysql, MYSQL_SHARED_MEMORY_BASE_NAME,
                  shared_memory_base_name);
#endif
  mysql_options(&mysql, MYSQL_SET_CHARSET_NAME, default_charset);

  if (opt_plugin_dir && *opt_plugin_dir)
    mysql_options(&mysql, MYSQL_PLUGIN_DIR, opt_plugin_dir);

  if (opt_default_auth && *opt_default_auth)
    mysql_options(&mysql, MYSQL_DEFAULT_AUTH, opt_default_auth);

  mysql_options(&mysql, MYSQL_OPT_CONNECT_ATTR_RESET, nullptr);
  mysql_options4(&mysql, MYSQL_OPT_CONNECT_ATTR_ADD, "program_name",
                 "mysqlslap");
  if (using_opt_enable_cleartext_plugin)
    mysql_options(&mysql, MYSQL_ENABLE_CLEARTEXT_PLUGIN,
                  (char *)&opt_enable_cleartext_plugin);
  set_server_public_key(&mysql);
  set_get_server_public_key_option(&mysql);
  if (!opt_only_print) {
    if (!(mysql_real_connect(&mysql, host, user, opt_password, nullptr,
                             opt_mysql_port, opt_mysql_unix_port,
                             connect_flags))) {
      fprintf(stderr, "%s: Error when connecting to server: %s\n", my_progname,
              mysql_error(&mysql));
      mysql_close(&mysql);
      my_end(0);
      return EXIT_FAILURE;
    }
  }
  set_sql_mode(&mysql);

  native_mutex_init(&counter_mutex, nullptr);
  native_cond_init(&count_threshold);
  native_mutex_init(&sleeper_mutex, nullptr);
  native_cond_init(&sleep_threshold);

  /* Main iterations loop */
  eptr = engine_options;
  do {
    /* For the final stage we run whatever queries we were asked to run */
    uint *current;

    if (verbose >= 2) printf("Starting Concurrency Test\n");

    if (*concurrency) {
      for (current = concurrency; current && *current; current++)
        concurrency_loop(&mysql, *current, eptr);
    } else {
      uint infinite = 1;
      do {
        concurrency_loop(&mysql, infinite, eptr);
      } while (infinite++);
    }

    if (!opt_preserve) drop_schema(&mysql, create_schema_string);

  } while (eptr ? (eptr = eptr->next) : nullptr);

  native_mutex_destroy(&counter_mutex);
  native_cond_destroy(&count_threshold);
  native_mutex_destroy(&sleeper_mutex);
  native_cond_destroy(&sleep_threshold);

  mysql_close(&mysql); /* Close & free connection */

  /* now free all the strings we created */
  my_free(opt_password);
  my_free(concurrency);

  statement_cleanup(create_statements);
  statement_cleanup(query_statements);
  statement_cleanup(pre_statements);
  statement_cleanup(post_statements);
  option_cleanup(engine_options);

#if defined(_WIN32)
  my_free(shared_memory_base_name);
#endif
  mysql_server_end();
  my_end(my_end_arg);

  return EXIT_SUCCESS;
}

void concurrency_loop(MYSQL *mysql, uint current, option_string *eptr) {
  unsigned int x;
  stats *head_sptr;
  stats *sptr;
  conclusions conclusion;
  unsigned long long client_limit;
  int sysret;

  head_sptr =
      (stats *)my_malloc(PSI_NOT_INSTRUMENTED, sizeof(stats) * iterations,
                         MYF(MY_ZEROFILL | MY_FAE | MY_WME));

  memset(&conclusion, 0, sizeof(conclusions));

  if (auto_actual_queries)
    client_limit = auto_actual_queries;
  else if (num_of_query)
    client_limit = num_of_query / current;
  else
    client_limit = actual_queries;

  for (x = 0, sptr = head_sptr; x < iterations; x++, sptr++) {
    /*
      We might not want to load any data, such as when we are calling
      a stored_procedure that doesn't use data, or we know we already have
      data in the table.
    */
    if (!opt_preserve) drop_schema(mysql, create_schema_string);

    /* First we create */
    if (create_statements)
      create_schema(mysql, create_schema_string, create_statements, eptr);

    /*
      If we generated GUID we need to build a list of them from creation that
      we can later use.
    */
    if (verbose >= 2) printf("Generating primary key list\n");
    if (auto_generate_sql_autoincrement || auto_generate_sql_guid_primary)
      generate_primary_key_list(mysql, eptr);

    if (commit_rate)
      run_query(mysql, "SET AUTOCOMMIT=0", strlen("SET AUTOCOMMIT=0"));

    if (pre_system)
      if ((sysret = system(pre_system)) != 0)
        fprintf(stderr,
                "Warning: Execution of pre_system option returned %d.\n",
                sysret);

    /*
      Pre statements are always run after all other logic so they can
      correct/adjust any item that they want.
    */
    if (pre_statements) run_statements(mysql, pre_statements);

    run_scheduler(sptr, query_statements, current, client_limit);

    if (post_statements) run_statements(mysql, post_statements);

    if (post_system)
      if ((sysret = system(post_system)) != 0)
        fprintf(stderr,
                "Warning: Execution of post_system option returned %d.\n",
                sysret);

    /* We are finished with this run */
    if (auto_generate_sql_autoincrement || auto_generate_sql_guid_primary)
      drop_primary_key_list();
  }

  if (verbose >= 2) printf("Generating stats\n");

  generate_stats(&conclusion, eptr, head_sptr);

  if (!opt_silent) print_conclusions(&conclusion);
  if (opt_csv_str) print_conclusions_csv(&conclusion);

  my_free(head_sptr);
}

static struct my_option my_long_options[] = {
    {"help", '?', "Display this help and exit.", nullptr, nullptr, nullptr,
     GET_NO_ARG, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"auto-generate-sql", 'a',
     "Generate SQL where not supplied by file or command line.",
     &auto_generate_sql, &auto_generate_sql, nullptr, GET_BOOL, NO_ARG, 0, 0, 0,
     nullptr, 0, nullptr},
    {"auto-generate-sql-add-autoincrement", OPT_SLAP_AUTO_GENERATE_ADD_AUTO,
     "Add an AUTO_INCREMENT column to auto-generated tables.",
     &auto_generate_sql_autoincrement, &auto_generate_sql_autoincrement,
     nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"auto-generate-sql-execute-number", OPT_SLAP_AUTO_GENERATE_EXECUTE_QUERIES,
     "Set this number to generate a set number of queries to run.",
     &auto_actual_queries, &auto_actual_queries, nullptr, GET_ULL, REQUIRED_ARG,
     0, 0, 0, nullptr, 0, nullptr},
    {"auto-generate-sql-guid-primary", OPT_SLAP_AUTO_GENERATE_GUID_PRIMARY,
     "Add GUID based primary keys to auto-generated tables.",
     &auto_generate_sql_guid_primary, &auto_generate_sql_guid_primary, nullptr,
     GET_BOOL, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"auto-generate-sql-load-type", OPT_SLAP_AUTO_GENERATE_SQL_LOAD_TYPE,
     "Specify test load type: mixed, update, write, key, or read; default is "
     "mixed.",
     &auto_generate_sql_type, &auto_generate_sql_type, nullptr, GET_STR,
     REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"auto-generate-sql-secondary-indexes",
     OPT_SLAP_AUTO_GENERATE_SECONDARY_INDEXES,
     "Number of secondary indexes to add to auto-generated tables.",
     &auto_generate_sql_secondary_indexes, &auto_generate_sql_secondary_indexes,
     nullptr, GET_UINT, REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"auto-generate-sql-unique-query-number",
     OPT_SLAP_AUTO_GENERATE_UNIQUE_QUERY_NUM,
     "Number of unique queries to generate for automatic tests.",
     &auto_generate_sql_unique_query_number,
     &auto_generate_sql_unique_query_number, nullptr, GET_ULL, REQUIRED_ARG, 10,
     0, 0, nullptr, 0, nullptr},
    {"auto-generate-sql-unique-write-number",
     OPT_SLAP_AUTO_GENERATE_UNIQUE_WRITE_NUM,
     "Number of unique queries to generate for auto-generate-sql-write-number.",
     &auto_generate_sql_unique_write_number,
     &auto_generate_sql_unique_write_number, nullptr, GET_ULL, REQUIRED_ARG, 10,
     0, 0, nullptr, 0, nullptr},
    {"auto-generate-sql-write-number", OPT_SLAP_AUTO_GENERATE_WRITE_NUM,
     "Number of row inserts to perform for each thread (default is 100).",
     &auto_generate_sql_number, &auto_generate_sql_number, nullptr, GET_ULL,
     REQUIRED_ARG, 100, 0, 0, nullptr, 0, nullptr},
    {"commit", OPT_SLAP_COMMIT, "Commit records every X number of statements.",
     &commit_rate, &commit_rate, nullptr, GET_UINT, REQUIRED_ARG, 0, 0, 0,
     nullptr, 0, nullptr},
    {"compress", 'C', "Use compression in server/client protocol.",
     &opt_compress, &opt_compress, nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr,
     0, nullptr},
    {"concurrency", 'c', "Number of clients to simulate for query to run.",
     &concurrency_str, &concurrency_str, nullptr, GET_STR, REQUIRED_ARG, 0, 0,
     0, nullptr, 0, nullptr},
    {"create", OPT_SLAP_CREATE_STRING, "File or string to use create tables.",
     &create_string, &create_string, nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0,
     nullptr, 0, nullptr},
    {"create-schema", OPT_CREATE_SLAP_SCHEMA, "Schema to run tests in.",
     &create_schema_string, &create_schema_string, nullptr, GET_STR,
     REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"csv", OPT_SLAP_CSV,
     "Generate CSV output to named file or to stdout if no file is named.",
     nullptr, nullptr, nullptr, GET_STR, OPT_ARG, 0, 0, 0, nullptr, 0, nullptr},
#ifdef DBUG_OFF
    {"debug", '#', "This is a non-debug version. Catch this and exit.", 0, 0, 0,
     GET_DISABLED, OPT_ARG, 0, 0, 0, 0, 0, 0},
    {"debug-check", OPT_DEBUG_CHECK,
     "This is a non-debug version. Catch this and exit.", 0, 0, 0, GET_DISABLED,
     NO_ARG, 0, 0, 0, 0, 0, 0},
    {"debug-info", 'T', "This is a non-debug version. Catch this and exit.", 0,
     0, 0, GET_DISABLED, NO_ARG, 0, 0, 0, 0, 0, 0},
#else
    {"debug", '#', "Output debug log. Often this is 'd:t:o,filename'.",
     &default_dbug_option, &default_dbug_option, nullptr, GET_STR, OPT_ARG, 0,
     0, 0, nullptr, 0, nullptr},
    {"debug-check", OPT_DEBUG_CHECK,
     "Check memory and open file usage at exit.", &debug_check_flag,
     &debug_check_flag, nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
    {"debug-info", 'T', "Print some debug info at exit.", &debug_info_flag,
     &debug_info_flag, nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
#endif
    {"default_auth", OPT_DEFAULT_AUTH,
     "Default authentication client-side plugin to use.", &opt_default_auth,
     &opt_default_auth, nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
    {"delimiter", 'F',
     "Delimiter to use in SQL statements supplied in file or command line.",
     &delimiter, &delimiter, nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0, nullptr,
     0, nullptr},
    {"detach", OPT_SLAP_DETACH,
     "Detach (close and reopen) connections after X number of requests.",
     &detach_rate, &detach_rate, nullptr, GET_UINT, REQUIRED_ARG, 0, 0, 0,
     nullptr, 0, nullptr},
    {"enable_cleartext_plugin", OPT_ENABLE_CLEARTEXT_PLUGIN,
     "Enable/disable the clear text authentication plugin.",
     &opt_enable_cleartext_plugin, &opt_enable_cleartext_plugin, nullptr,
     GET_BOOL, OPT_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"engine", 'e', "Storage engine to use for creating the table.",
     &default_engine, &default_engine, nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0,
     nullptr, 0, nullptr},
    {"host", 'h', "Connect to host.", &host, &host, nullptr, GET_STR,
     REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"iterations", 'i', "Number of times to run the tests.", &iterations,
     &iterations, nullptr, GET_UINT, REQUIRED_ARG, 1, 1, UINT_MAX, nullptr, 0,
     nullptr},
    {"no-drop", OPT_SLAP_NO_DROP, "Do not drop the schema after the test.",
     &opt_no_drop, &opt_no_drop, nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
    {"number-char-cols", 'x',
     "Number of VARCHAR columns to create in table if specifying "
     "--auto-generate-sql.",
     &num_char_cols_opt, &num_char_cols_opt, nullptr, GET_STR, REQUIRED_ARG, 0,
     0, 0, nullptr, 0, nullptr},
    {"number-int-cols", 'y',
     "Number of INT columns to create in table if specifying "
     "--auto-generate-sql.",
     &num_int_cols_opt, &num_int_cols_opt, nullptr, GET_STR, REQUIRED_ARG, 0, 0,
     0, nullptr, 0, nullptr},
    {"number-of-queries", OPT_MYSQL_NUMBER_OF_QUERY,
     "Limit each client to this number of queries (this is not exact).",
     &num_of_query, &num_of_query, nullptr, GET_ULL, REQUIRED_ARG, 0, 0, 0,
     nullptr, 0, nullptr},
    {"only-print", OPT_MYSQL_ONLY_PRINT,
     "Do not connect to the databases, but instead print out what would have "
     "been done.",
     &opt_only_print, &opt_only_print, nullptr, GET_BOOL, NO_ARG, 0, 0, 0,
     nullptr, 0, nullptr},
    {"password", 'p',
     "Password to use when connecting to server. If password is not given it's "
     "asked from the tty.",
     nullptr, nullptr, nullptr, GET_PASSWORD, OPT_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
#ifdef _WIN32
    {"pipe", 'W', "Use named pipes to connect to server.", 0, 0, 0, GET_NO_ARG,
     NO_ARG, 0, 0, 0, 0, 0, 0},
#endif
    {"plugin_dir", OPT_PLUGIN_DIR, "Directory for client-side plugins.",
     &opt_plugin_dir, &opt_plugin_dir, nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0,
     nullptr, 0, nullptr},
    {"port", 'P', "Port number to use for connection.", &opt_mysql_port,
     &opt_mysql_port, nullptr, GET_UINT, REQUIRED_ARG, MYSQL_PORT, 0, 0,
     nullptr, 0, nullptr},
    {"post-query", OPT_SLAP_POST_QUERY,
     "Query to run or file containing query to execute after tests have "
     "completed.",
     &user_supplied_post_statements, &user_supplied_post_statements, nullptr,
     GET_STR, REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"post-system", OPT_SLAP_POST_SYSTEM,
     "system() string to execute after tests have completed.", &post_system,
     &post_system, nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
    {"pre-query", OPT_SLAP_PRE_QUERY,
     "Query to run or file containing query to execute before running tests.",
     &user_supplied_pre_statements, &user_supplied_pre_statements, nullptr,
     GET_STR, REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"pre-system", OPT_SLAP_PRE_SYSTEM,
     "system() string to execute before running tests.", &pre_system,
     &pre_system, nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"protocol", OPT_MYSQL_PROTOCOL,
     "The protocol to use for connection (tcp, socket, pipe, memory).", nullptr,
     nullptr, nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"query", 'q', "Query to run or file containing query to run.",
     &user_supplied_query, &user_supplied_query, nullptr, GET_STR, REQUIRED_ARG,
     0, 0, 0, nullptr, 0, nullptr},
#if defined(_WIN32)
    {"shared-memory-base-name", OPT_SHARED_MEMORY_BASE_NAME,
     "Base name of shared memory.", &shared_memory_base_name,
     &shared_memory_base_name, 0, GET_STR_ALLOC, REQUIRED_ARG, 0, 0, 0, 0, 0,
     0},
#endif
    {"silent", 's', "Run program in silent mode - no output.", &opt_silent,
     &opt_silent, nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"socket", 'S', "The socket file to use for connection.",
     &opt_mysql_unix_port, &opt_mysql_unix_port, nullptr, GET_STR, REQUIRED_ARG,
     0, 0, 0, nullptr, 0, nullptr},
    {"sql_mode", 0, "Specify sql-mode to run mysqlslap tool.", &sql_mode,
     &sql_mode, nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
#include "caching_sha2_passwordopt-longopts.h"
#include "sslopt-longopts.h"

    {"user", 'u', "User for login if not current user.", &user, &user, nullptr,
     GET_STR, REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"verbose", 'v',
     "More verbose output; you can use this multiple times to get even more "
     "verbose output.",
     &verbose, &verbose, nullptr, GET_NO_ARG, NO_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
    {"version", 'V', "Output version information and exit.", nullptr, nullptr,
     nullptr, GET_NO_ARG, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
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

static void usage(void) {
  print_version();
  puts(ORACLE_WELCOME_COPYRIGHT_NOTICE("2005"));
  puts("Run a query multiple times against the server.\n");
  printf("Usage: %s [OPTIONS]\n", my_progname);
  print_defaults("my", load_default_groups);
  my_print_help(my_long_options);
}

extern "C" {
static bool get_one_option(int optid, const struct my_option *opt,
                           char *argument) {
  DBUG_TRACE;
  switch (optid) {
    case 'v':
      verbose++;
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
        while (*argument) *argument++ = 'x'; /* Destroy argument */
        if (*start) start[1] = 0;            /* Cut length of argument */
        tty_password = false;
      } else
        tty_password = true;
      break;
    case 'W':
#ifdef _WIN32
      opt_protocol = MYSQL_PROTOCOL_PIPE;
#endif
      break;
    case OPT_MYSQL_PROTOCOL:
      opt_protocol =
          find_type_or_exit(argument, &sql_protocol_typelib, opt->name);
      break;
    case '#':
      DBUG_PUSH(argument ? argument : default_dbug_option);
      debug_check_flag = true;
      break;
    case OPT_SLAP_CSV:
      if (!argument) argument = const_cast<char *>("-"); /* use stdout */
      opt_csv_str = argument;
      break;
#include "sslopt-case.h"

    case 'V':
      print_version();
      exit(0);
      break;
    case '?':
    case 'I': /* Info */
      usage();
      exit(0);
    case OPT_ENABLE_CLEARTEXT_PLUGIN:
      using_opt_enable_cleartext_plugin = true;
      break;
  }
  return false;
}
}

size_t get_random_string(char *buf) {
  char *buf_ptr = buf;
  int x;
  DBUG_TRACE;
  for (x = RAND_STRING_SIZE; x > 0; x--)
    *buf_ptr++ = ALPHANUMERICS[random() % ALPHANUMERICS_SIZE];
  return buf_ptr - buf;
}

/*
  build_table_string

  This function builds a create table query if the user opts to not supply
  a file or string containing a create table statement
*/
static statement *build_table_string(void) {
  char buf[HUGE_STRING_LENGTH];
  unsigned int col_count;
  statement *ptr;
  DYNAMIC_STRING table_string;
  DBUG_TRACE;

  DBUG_PRINT("info",
             ("num int cols %u num char cols %u", num_int_cols, num_char_cols));

  init_dynamic_string(&table_string, "", 1024, 1024);

  dynstr_append(&table_string, "CREATE TABLE `t1` (");

  if (auto_generate_sql_autoincrement) {
    dynstr_append(&table_string, "id serial");

    if (num_int_cols || num_char_cols) dynstr_append(&table_string, ",");
  }

  if (auto_generate_sql_guid_primary) {
    dynstr_append(&table_string, "id varchar(36) primary key");

    if (num_int_cols || num_char_cols || auto_generate_sql_guid_primary)
      dynstr_append(&table_string, ",");
  }

  if (auto_generate_sql_secondary_indexes) {
    unsigned int count;

    for (count = 0; count < auto_generate_sql_secondary_indexes; count++) {
      if (count) /* Except for the first pass we add a comma */
        dynstr_append(&table_string, ",");

      if (snprintf(buf, HUGE_STRING_LENGTH, "id%d varchar(36) unique key",
                   count) > HUGE_STRING_LENGTH) {
        fprintf(stderr, "Memory Allocation error in create table\n");
        exit(1);
      }
      dynstr_append(&table_string, buf);
    }

    if (num_int_cols || num_char_cols) dynstr_append(&table_string, ",");
  }

  if (num_int_cols)
    for (col_count = 1; col_count <= num_int_cols; col_count++) {
      if (num_int_cols_index) {
        if (snprintf(buf, HUGE_STRING_LENGTH,
                     "intcol%d INT(32), INDEX(intcol%d)", col_count,
                     col_count) > HUGE_STRING_LENGTH) {
          fprintf(stderr, "Memory Allocation error in create table\n");
          exit(1);
        }
      } else {
        if (snprintf(buf, HUGE_STRING_LENGTH, "intcol%d INT(32) ", col_count) >
            HUGE_STRING_LENGTH) {
          fprintf(stderr, "Memory Allocation error in create table\n");
          exit(1);
        }
      }
      dynstr_append(&table_string, buf);

      if (col_count < num_int_cols || num_char_cols > 0)
        dynstr_append(&table_string, ",");
    }

  if (num_char_cols)
    for (col_count = 1; col_count <= num_char_cols; col_count++) {
      if (num_char_cols_index) {
        if (snprintf(buf, HUGE_STRING_LENGTH,
                     "charcol%d VARCHAR(128), INDEX(charcol%d) ", col_count,
                     col_count) > HUGE_STRING_LENGTH) {
          fprintf(stderr, "Memory Allocation error in creating table\n");
          exit(1);
        }
      } else {
        if (snprintf(buf, HUGE_STRING_LENGTH, "charcol%d VARCHAR(128)",
                     col_count) > HUGE_STRING_LENGTH) {
          fprintf(stderr, "Memory Allocation error in creating table\n");
          exit(1);
        }
      }
      dynstr_append(&table_string, buf);

      if (col_count < num_char_cols) dynstr_append(&table_string, ",");
    }

  dynstr_append(&table_string, ")");
  ptr = (statement *)my_malloc(PSI_NOT_INSTRUMENTED, sizeof(statement),
                               MYF(MY_ZEROFILL | MY_FAE | MY_WME));
  ptr->string = (char *)my_malloc(PSI_NOT_INSTRUMENTED, table_string.length + 1,
                                  MYF(MY_ZEROFILL | MY_FAE | MY_WME));
  ptr->length = table_string.length + 1;
  ptr->type = CREATE_TABLE_TYPE;
  my_stpcpy(ptr->string, table_string.str);
  dynstr_free(&table_string);
  return ptr;
}

/*
  build_update_string()

  This function builds insert statements when the user opts to not supply
  an insert file or string containing insert data
*/
static statement *build_update_string(void) {
  char buf[HUGE_STRING_LENGTH];
  unsigned int col_count;
  statement *ptr;
  DYNAMIC_STRING update_string;
  DBUG_TRACE;

  init_dynamic_string(&update_string, "", 1024, 1024);

  dynstr_append(&update_string, "UPDATE t1 SET ");

  if (num_int_cols)
    for (col_count = 1; col_count <= num_int_cols; col_count++) {
      if (snprintf(buf, HUGE_STRING_LENGTH, "intcol%d = %ld", col_count,
                   random()) > HUGE_STRING_LENGTH) {
        fprintf(stderr, "Memory Allocation error in creating update\n");
        exit(1);
      }
      dynstr_append(&update_string, buf);

      if (col_count < num_int_cols || num_char_cols > 0)
        dynstr_append_mem(&update_string, ",", 1);
    }

  if (num_char_cols)
    for (col_count = 1; col_count <= num_char_cols; col_count++) {
      char rand_buffer[RAND_STRING_SIZE];
      size_t buf_len = get_random_string(rand_buffer);

      if (snprintf(buf, HUGE_STRING_LENGTH, "charcol%d = '%.*s'", col_count,
                   (int)buf_len, rand_buffer) > HUGE_STRING_LENGTH) {
        fprintf(stderr, "Memory Allocation error in creating update\n");
        exit(1);
      }
      dynstr_append(&update_string, buf);

      if (col_count < num_char_cols) dynstr_append_mem(&update_string, ",", 1);
    }

  if (auto_generate_sql_autoincrement || auto_generate_sql_guid_primary)
    dynstr_append(&update_string, " WHERE id = ");

  ptr = (statement *)my_malloc(PSI_NOT_INSTRUMENTED, sizeof(statement),
                               MYF(MY_ZEROFILL | MY_FAE | MY_WME));

  ptr->string =
      (char *)my_malloc(PSI_NOT_INSTRUMENTED, update_string.length + 1,
                        MYF(MY_ZEROFILL | MY_FAE | MY_WME));
  ptr->length = update_string.length + 1;
  if (auto_generate_sql_autoincrement || auto_generate_sql_guid_primary)
    ptr->type = UPDATE_TYPE_REQUIRES_PREFIX;
  else
    ptr->type = UPDATE_TYPE;
  my_stpcpy(ptr->string, update_string.str);
  dynstr_free(&update_string);
  return ptr;
}

/*
  build_insert_string()

  This function builds insert statements when the user opts to not supply
  an insert file or string containing insert data
*/
static statement *build_insert_string(void) {
  char buf[HUGE_STRING_LENGTH];
  unsigned int col_count;
  statement *ptr;
  DYNAMIC_STRING insert_string;
  DBUG_TRACE;

  init_dynamic_string(&insert_string, "", 1024, 1024);

  dynstr_append(&insert_string, "INSERT INTO t1 VALUES (");

  if (auto_generate_sql_autoincrement) {
    dynstr_append(&insert_string, "NULL");

    if (num_int_cols || num_char_cols) dynstr_append(&insert_string, ",");
  }

  if (auto_generate_sql_guid_primary) {
    dynstr_append(&insert_string, "uuid()");

    if (num_int_cols || num_char_cols) dynstr_append(&insert_string, ",");
  }

  if (auto_generate_sql_secondary_indexes) {
    unsigned int count;

    for (count = 0; count < auto_generate_sql_secondary_indexes; count++) {
      if (count) /* Except for the first pass we add a comma */
        dynstr_append(&insert_string, ",");

      dynstr_append(&insert_string, "uuid()");
    }

    if (num_int_cols || num_char_cols) dynstr_append(&insert_string, ",");
  }

  if (num_int_cols)
    for (col_count = 1; col_count <= num_int_cols; col_count++) {
      if (snprintf(buf, HUGE_STRING_LENGTH, "%ld", random()) >
          HUGE_STRING_LENGTH) {
        fprintf(stderr, "Memory Allocation error in creating insert\n");
        exit(1);
      }
      dynstr_append(&insert_string, buf);

      if (col_count < num_int_cols || num_char_cols > 0)
        dynstr_append_mem(&insert_string, ",", 1);
    }

  if (num_char_cols)
    for (col_count = 1; col_count <= num_char_cols; col_count++) {
      size_t buf_len = get_random_string(buf);
      dynstr_append_mem(&insert_string, "'", 1);
      dynstr_append_mem(&insert_string, buf, buf_len);
      dynstr_append_mem(&insert_string, "'", 1);

      if (col_count < num_char_cols) dynstr_append_mem(&insert_string, ",", 1);
    }

  dynstr_append_mem(&insert_string, ")", 1);

  ptr = (statement *)my_malloc(PSI_NOT_INSTRUMENTED, sizeof(statement),
                               MYF(MY_ZEROFILL | MY_FAE | MY_WME));
  ptr->string =
      (char *)my_malloc(PSI_NOT_INSTRUMENTED, insert_string.length + 1,
                        MYF(MY_ZEROFILL | MY_FAE | MY_WME));
  ptr->length = insert_string.length + 1;
  ptr->type = INSERT_TYPE;
  my_stpcpy(ptr->string, insert_string.str);
  dynstr_free(&insert_string);
  return ptr;
}

/*
  build_select_string()

  This function builds a query if the user opts to not supply a query
  statement or file containing a query statement
*/
static statement *build_select_string(bool key) {
  char buf[HUGE_STRING_LENGTH];
  unsigned int col_count;
  statement *ptr;
  static DYNAMIC_STRING query_string;
  DBUG_TRACE;

  init_dynamic_string(&query_string, "", 1024, 1024);

  dynstr_append_mem(&query_string, "SELECT ", 7);
  for (col_count = 1; col_count <= num_int_cols; col_count++) {
    if (snprintf(buf, HUGE_STRING_LENGTH, "intcol%d", col_count) >
        HUGE_STRING_LENGTH) {
      fprintf(stderr, "Memory Allocation error in creating select\n");
      exit(1);
    }
    dynstr_append(&query_string, buf);

    if (col_count < num_int_cols || num_char_cols > 0)
      dynstr_append_mem(&query_string, ",", 1);
  }
  for (col_count = 1; col_count <= num_char_cols; col_count++) {
    if (snprintf(buf, HUGE_STRING_LENGTH, "charcol%d", col_count) >
        HUGE_STRING_LENGTH) {
      fprintf(stderr, "Memory Allocation error in creating select\n");
      exit(1);
    }
    dynstr_append(&query_string, buf);

    if (col_count < num_char_cols) dynstr_append_mem(&query_string, ",", 1);
  }
  dynstr_append(&query_string, " FROM t1");

  if ((key) &&
      (auto_generate_sql_autoincrement || auto_generate_sql_guid_primary))
    dynstr_append(&query_string, " WHERE id = ");

  ptr = (statement *)my_malloc(PSI_NOT_INSTRUMENTED, sizeof(statement),
                               MYF(MY_ZEROFILL | MY_FAE | MY_WME));
  ptr->string = (char *)my_malloc(PSI_NOT_INSTRUMENTED, query_string.length + 1,
                                  MYF(MY_ZEROFILL | MY_FAE | MY_WME));
  ptr->length = query_string.length + 1;
  if ((key) &&
      (auto_generate_sql_autoincrement || auto_generate_sql_guid_primary))
    ptr->type = SELECT_TYPE_REQUIRES_PREFIX;
  else
    ptr->type = SELECT_TYPE;
  my_stpcpy(ptr->string, query_string.str);
  dynstr_free(&query_string);
  return ptr;
}

static int get_options(int *argc, char ***argv) {
  int ho_error;
  char *tmp_string;
  MY_STAT sbuf; /* Stat information for the data file */

  DBUG_TRACE;
  if ((ho_error = handle_options(argc, argv, my_long_options, get_one_option)))
    exit(ho_error);
  if (debug_info_flag) my_end_arg = MY_CHECK_ERROR | MY_GIVE_INFO;
  if (debug_check_flag) my_end_arg = MY_CHECK_ERROR;

  if (!user) user = "root";

  /*
    If something is created and --no-drop is not specified, we drop the
    schema.
  */
  if (!opt_no_drop && (create_string || auto_generate_sql))
    opt_preserve = false;

  if (auto_generate_sql && (create_string || user_supplied_query)) {
    fprintf(stderr,
            "%s: Can't use --auto-generate-sql when create and query strings "
            "are specified!\n",
            my_progname);
    exit(1);
  }

  if (auto_generate_sql && auto_generate_sql_guid_primary &&
      auto_generate_sql_autoincrement) {
    fprintf(stderr,
            "%s: Either auto-generate-sql-guid-primary or "
            "auto-generate-sql-add-autoincrement can be used!\n",
            my_progname);
    exit(1);
  }

  /*
    We are testing to make sure that if someone specified a key search
    that we actually added a key!
  */
  if (auto_generate_sql && auto_generate_sql_type[0] == 'k')
    if (auto_generate_sql_autoincrement == false &&
        auto_generate_sql_guid_primary == false) {
      fprintf(stderr, "%s: Can't perform key test without a primary key!\n",
              my_progname);
      exit(1);
    }

  if (auto_generate_sql && num_of_query && auto_actual_queries) {
    fprintf(stderr,
            "%s: Either auto-generate-sql-execute-number or number-of-queries "
            "can be used!\n",
            my_progname);
    exit(1);
  }

  parse_comma(concurrency_str ? concurrency_str : "1", &concurrency);

  if (opt_csv_str) {
    opt_silent = true;

    if (opt_csv_str[0] == '-') {
      csv_file = my_fileno(stdout);
    } else {
      if ((csv_file = my_open(opt_csv_str, O_CREAT | O_WRONLY | O_APPEND,
                              MYF(0))) == -1) {
        fprintf(stderr, "%s: Could not open csv file: %sn\n", my_progname,
                opt_csv_str);
        exit(1);
      }
    }
  }

  if (opt_only_print) opt_silent = true;

  if (num_int_cols_opt) {
    option_string *str;
    if (parse_option(num_int_cols_opt, &str, ',') == -1) {
      fprintf(stderr,
              "Invalid value specified for the option "
              "'number-int-cols'\n");
      option_cleanup(str);
      return 1;
    }
    num_int_cols = atoi(str->string);
    if (str->option) num_int_cols_index = atoi(str->option);
    option_cleanup(str);
  }

  if (num_char_cols_opt) {
    option_string *str;
    if (parse_option(num_char_cols_opt, &str, ',') == -1) {
      fprintf(stderr,
              "Invalid value specified for the option "
              "'number-char-cols'\n");
      option_cleanup(str);
      return 1;
    }
    num_char_cols = atoi(str->string);
    if (str->option)
      num_char_cols_index = atoi(str->option);
    else
      num_char_cols_index = 0;
    option_cleanup(str);
  }

  if (auto_generate_sql) {
    unsigned long long x = 0;
    statement *ptr_statement;

    if (verbose >= 2) printf("Building Create Statements for Auto\n");

    create_statements = build_table_string();
    /*
      Pre-populate table
    */
    for (ptr_statement = create_statements, x = 0;
         x < auto_generate_sql_unique_write_number;
         x++, ptr_statement = ptr_statement->next) {
      ptr_statement->next = build_insert_string();
    }

    if (verbose >= 2) printf("Building Query Statements for Auto\n");

    if (auto_generate_sql_type[0] == 'r') {
      if (verbose >= 2) printf("Generating SELECT Statements for Auto\n");

      query_statements = build_select_string(false);
      for (ptr_statement = query_statements, x = 0;
           x < auto_generate_sql_unique_query_number;
           x++, ptr_statement = ptr_statement->next) {
        ptr_statement->next = build_select_string(false);
      }
    } else if (auto_generate_sql_type[0] == 'k') {
      if (verbose >= 2)
        printf("Generating SELECT for keys Statements for Auto\n");

      query_statements = build_select_string(true);
      for (ptr_statement = query_statements, x = 0;
           x < auto_generate_sql_unique_query_number;
           x++, ptr_statement = ptr_statement->next) {
        ptr_statement->next = build_select_string(true);
      }
    } else if (auto_generate_sql_type[0] == 'w') {
      /*
        We generate a number of strings in case the engine is
        Archive (since strings which were identical one after another
        would be too easily optimized).
      */
      if (verbose >= 2) printf("Generating INSERT Statements for Auto\n");
      query_statements = build_insert_string();
      for (ptr_statement = query_statements, x = 0;
           x < auto_generate_sql_unique_query_number;
           x++, ptr_statement = ptr_statement->next) {
        ptr_statement->next = build_insert_string();
      }
    } else if (auto_generate_sql_type[0] == 'u') {
      query_statements = build_update_string();
      for (ptr_statement = query_statements, x = 0;
           x < auto_generate_sql_unique_query_number;
           x++, ptr_statement = ptr_statement->next) {
        ptr_statement->next = build_update_string();
      }
    } else /* Mixed mode is default */
    {
      int coin = 0;

      query_statements = build_insert_string();
      /*
        This logic should be extended to do a more mixed load,
        at the moment it results in "every other".
      */
      for (ptr_statement = query_statements, x = 0;
           x < auto_generate_sql_unique_query_number;
           x++, ptr_statement = ptr_statement->next) {
        if (coin) {
          ptr_statement->next = build_insert_string();
          coin = 0;
        } else {
          ptr_statement->next = build_select_string(true);
          coin = 1;
        }
      }
    }
  } else {
    if (create_string && my_stat(create_string, &sbuf, MYF(0))) {
      File data_file;
      if (!MY_S_ISREG(sbuf.st_mode)) {
        fprintf(stderr, "%s: Create file was not a regular file\n",
                my_progname);
        exit(1);
      }
      if ((data_file = my_open(create_string, O_RDWR, MYF(0))) == -1) {
        fprintf(stderr, "%s: Could not open create file\n", my_progname);
        exit(1);
      }
      tmp_string =
          (char *)my_malloc(PSI_NOT_INSTRUMENTED, (size_t)sbuf.st_size + 1,
                            MYF(MY_ZEROFILL | MY_FAE | MY_WME));
      my_read(data_file, (uchar *)tmp_string, (size_t)sbuf.st_size, MYF(0));
      tmp_string[sbuf.st_size] = '\0';
      my_close(data_file, MYF(0));
      parse_delimiter(tmp_string, &create_statements, delimiter[0]);
      my_free(tmp_string);
    } else if (create_string) {
      parse_delimiter(create_string, &create_statements, delimiter[0]);
    }

    if (user_supplied_query && my_stat(user_supplied_query, &sbuf, MYF(0))) {
      File data_file;
      if (!MY_S_ISREG(sbuf.st_mode)) {
        fprintf(stderr, "%s: User query supplied file was not a regular file\n",
                my_progname);
        exit(1);
      }
      if ((data_file = my_open(user_supplied_query, O_RDWR, MYF(0))) == -1) {
        fprintf(stderr, "%s: Could not open query supplied file\n",
                my_progname);
        exit(1);
      }
      tmp_string =
          (char *)my_malloc(PSI_NOT_INSTRUMENTED, (size_t)sbuf.st_size + 1,
                            MYF(MY_ZEROFILL | MY_FAE | MY_WME));
      my_read(data_file, (uchar *)tmp_string, (size_t)sbuf.st_size, MYF(0));
      tmp_string[sbuf.st_size] = '\0';
      my_close(data_file, MYF(0));
      if (user_supplied_query)
        actual_queries =
            parse_delimiter(tmp_string, &query_statements, delimiter[0]);
      my_free(tmp_string);
    } else if (user_supplied_query) {
      actual_queries =
          parse_delimiter(user_supplied_query, &query_statements, delimiter[0]);
    }
  }

  if (user_supplied_pre_statements &&
      my_stat(user_supplied_pre_statements, &sbuf, MYF(0))) {
    File data_file;
    if (!MY_S_ISREG(sbuf.st_mode)) {
      fprintf(stderr, "%s: User query supplied file was not a regular file\n",
              my_progname);
      exit(1);
    }
    if ((data_file = my_open(user_supplied_pre_statements, O_RDWR, MYF(0))) ==
        -1) {
      fprintf(stderr, "%s: Could not open query supplied file\n", my_progname);
      exit(1);
    }
    tmp_string =
        (char *)my_malloc(PSI_NOT_INSTRUMENTED, (size_t)sbuf.st_size + 1,
                          MYF(MY_ZEROFILL | MY_FAE | MY_WME));
    my_read(data_file, (uchar *)tmp_string, (size_t)sbuf.st_size, MYF(0));
    tmp_string[sbuf.st_size] = '\0';
    my_close(data_file, MYF(0));
    if (user_supplied_pre_statements)
      (void)parse_delimiter(tmp_string, &pre_statements, delimiter[0]);
    my_free(tmp_string);
  } else if (user_supplied_pre_statements) {
    (void)parse_delimiter(user_supplied_pre_statements, &pre_statements,
                          delimiter[0]);
  }

  if (user_supplied_post_statements &&
      my_stat(user_supplied_post_statements, &sbuf, MYF(0))) {
    File data_file;
    if (!MY_S_ISREG(sbuf.st_mode)) {
      fprintf(stderr, "%s: User query supplied file was not a regular file\n",
              my_progname);
      exit(1);
    }
    if ((data_file = my_open(user_supplied_post_statements, O_RDWR, MYF(0))) ==
        -1) {
      fprintf(stderr, "%s: Could not open query supplied file\n", my_progname);
      exit(1);
    }
    tmp_string =
        (char *)my_malloc(PSI_NOT_INSTRUMENTED, (size_t)sbuf.st_size + 1,
                          MYF(MY_ZEROFILL | MY_FAE | MY_WME));
    my_read(data_file, (uchar *)tmp_string, (size_t)sbuf.st_size, MYF(0));
    tmp_string[sbuf.st_size] = '\0';
    my_close(data_file, MYF(0));
    if (user_supplied_post_statements)
      (void)parse_delimiter(tmp_string, &post_statements, delimiter[0]);
    my_free(tmp_string);
  } else if (user_supplied_post_statements) {
    (void)parse_delimiter(user_supplied_post_statements, &post_statements,
                          delimiter[0]);
  }

  if (verbose >= 2) printf("Parsing engines to use.\n");

  if (default_engine) {
    if (parse_option(default_engine, &engine_options, ',') == -1) {
      fprintf(stderr, "Invalid value specified for the option 'engine'\n");
      return 1;
    }
  }

  if (tty_password) opt_password = get_tty_password(NullS);
  return 0;
}

static int run_query(MYSQL *mysql, const char *query, size_t len) {
  if (opt_only_print) {
    printf("%.*s;\n", (int)len, query);
    return 0;
  }

  if (verbose >= 3) printf("%.*s;\n", (int)len, query);
  return mysql_real_query(mysql, query, (ulong)len);
}

static int generate_primary_key_list(MYSQL *mysql, option_string *engine_stmt) {
  MYSQL_RES *result;
  MYSQL_ROW row;
  unsigned long long counter;
  DBUG_TRACE;

  /*
    Blackhole is a special case, this allows us to test the upper end
    of the server during load runs.
  */
  if (opt_only_print ||
      (engine_stmt && strstr(engine_stmt->string, "blackhole"))) {
    primary_keys_number_of = 1;
    primary_keys = (char **)my_malloc(
        PSI_NOT_INSTRUMENTED, (uint)(sizeof(char *) * primary_keys_number_of),
        MYF(MY_ZEROFILL | MY_FAE | MY_WME));
    /* Yes, we strdup a const string to simplify the interface */
    primary_keys[0] = my_strdup(PSI_NOT_INSTRUMENTED,
                                "796c4422-1d94-102a-9d6d-00e0812d", MYF(0));
  } else {
    if (run_query(mysql, "SELECT id from t1", strlen("SELECT id from t1"))) {
      fprintf(stderr, "%s: Cannot select GUID primary keys. (%s)\n",
              my_progname, mysql_error(mysql));
      exit(1);
    }

    if (!(result = mysql_store_result(mysql))) {
      fprintf(stderr, "%s: Error when storing result: %d %s\n", my_progname,
              mysql_errno(mysql), mysql_error(mysql));
      exit(1);
    }
    primary_keys_number_of = mysql_num_rows(result);

    /* So why check this? Blackhole :) */
    if (primary_keys_number_of) {
      /*
        We create the structure and loop and create the items.
      */
      primary_keys = (char **)my_malloc(
          PSI_NOT_INSTRUMENTED, (uint)(sizeof(char *) * primary_keys_number_of),
          MYF(MY_ZEROFILL | MY_FAE | MY_WME));
      row = mysql_fetch_row(result);
      for (counter = 0; counter < primary_keys_number_of;
           counter++, row = mysql_fetch_row(result))
        primary_keys[counter] = my_strdup(PSI_NOT_INSTRUMENTED, row[0], MYF(0));
    }

    mysql_free_result(result);
  }

  return 0;
}

static int drop_primary_key_list(void) {
  unsigned long long counter;

  if (primary_keys_number_of) {
    for (counter = 0; counter < primary_keys_number_of; counter++)
      my_free(primary_keys[counter]);

    my_free(primary_keys);
  }

  return 0;
}

static void set_sql_mode(MYSQL *mysql) {
  if (sql_mode != nullptr) {
    char query[512];
    size_t len;
    len = snprintf(query, sizeof(query), "SET sql_mode = `%s`", sql_mode);

    if (run_query(mysql, query, len)) {
      fprintf(stderr, "%s:%s\n", my_progname, mysql_error(mysql));
      exit(1);
    }
  }
}

static int create_schema(MYSQL *mysql, const char *db, statement *stmt,
                         option_string *engine_stmt) {
  char query[HUGE_STRING_LENGTH];
  statement *ptr;
  statement *after_create;
  size_t len;
  ulonglong count;
  DBUG_TRACE;

  len = snprintf(query, HUGE_STRING_LENGTH, "CREATE SCHEMA `%s`", db);

  if (verbose >= 2) printf("Loading Pre-data\n");

  if (run_query(mysql, query, len)) {
    fprintf(stderr, "%s: Cannot create schema %s : %s\n", my_progname, db,
            mysql_error(mysql));
    exit(1);
  }

  if (opt_only_print) {
    printf("use %s;\n", db);
  } else {
    if (verbose >= 3) printf("%s;\n", query);

    if (mysql_select_db(mysql, db)) {
      fprintf(stderr, "%s: Cannot select schema '%s': %s\n", my_progname, db,
              mysql_error(mysql));
      exit(1);
    }
  }

  if (engine_stmt) {
    len = snprintf(query, HUGE_STRING_LENGTH, "set default_storage_engine=`%s`",
                   engine_stmt->string);
    if (run_query(mysql, query, len)) {
      fprintf(stderr, "%s: Cannot set default engine: %s\n", my_progname,
              mysql_error(mysql));
      exit(1);
    }
  }

  count = 0;
  after_create = stmt;

limit_not_met:
  for (ptr = after_create; ptr && ptr->length; ptr = ptr->next, count++) {
    if (auto_generate_sql && (auto_generate_sql_number == count)) break;

    if (engine_stmt && engine_stmt->option && ptr->type == CREATE_TABLE_TYPE) {
      char buffer[HUGE_STRING_LENGTH];

      snprintf(buffer, HUGE_STRING_LENGTH, "%s %s", ptr->string,
               engine_stmt->option);
      if (run_query(mysql, buffer, strlen(buffer))) {
        fprintf(stderr, "%s: Cannot run query %.*s ERROR : %s\n", my_progname,
                (uint)ptr->length, ptr->string, mysql_error(mysql));
        exit(1);
      }
    } else {
      if (run_query(mysql, ptr->string, ptr->length)) {
        fprintf(stderr, "%s: Cannot run query %.*s ERROR : %s\n", my_progname,
                (uint)ptr->length, ptr->string, mysql_error(mysql));
        exit(1);
      }
    }
  }

  if (auto_generate_sql && (auto_generate_sql_number > count)) {
    /* Special case for auto create, we don't want to create tables twice */
    after_create = stmt->next;
    goto limit_not_met;
  }

  return 0;
}

static int drop_schema(MYSQL *mysql, const char *db) {
  char query[HUGE_STRING_LENGTH];
  size_t len;
  DBUG_TRACE;
  len = snprintf(query, HUGE_STRING_LENGTH, "DROP SCHEMA IF EXISTS `%s`", db);

  if (run_query(mysql, query, len)) {
    fprintf(stderr, "%s: Cannot drop database '%s' ERROR : %s\n", my_progname,
            db, mysql_error(mysql));
    exit(1);
  }

  return 0;
}

static int run_statements(MYSQL *mysql, statement *stmt) {
  statement *ptr;
  MYSQL_RES *result;
  DBUG_TRACE;

  for (ptr = stmt; ptr && ptr->length; ptr = ptr->next) {
    if (run_query(mysql, ptr->string, ptr->length)) {
      fprintf(stderr, "%s: Cannot run query %.*s ERROR : %s\n", my_progname,
              (uint)ptr->length, ptr->string, mysql_error(mysql));
      exit(1);
    }
    if (mysql_field_count(mysql)) {
      result = mysql_store_result(mysql);
      mysql_free_result(result);
    }
  }

  return 0;
}

static int run_scheduler(stats *sptr, statement *stmts, uint concur,
                         ulonglong limit) {
  uint x;
  struct timeval start_time, end_time;
  thread_context con;
  my_thread_handle mainthread; /* Thread descriptor */
  my_thread_attr_t attr;       /* Thread attributes */
  DBUG_TRACE;

  con.stmt = stmts;
  con.limit = limit;

  my_thread_attr_init(&attr);
  my_thread_attr_setdetachstate(&attr, MY_THREAD_CREATE_DETACHED);

  native_mutex_lock(&counter_mutex);
  thread_counter = 0;

  native_mutex_lock(&sleeper_mutex);
  master_wakeup = 1;
  native_mutex_unlock(&sleeper_mutex);
  for (x = 0; x < concur; x++) {
    /* now you create the thread */
    if (my_thread_create(&mainthread, &attr, run_task, (void *)&con) != 0) {
      fprintf(stderr, "%s: Could not create thread\n", my_progname);
      exit(0);
    }
    thread_counter++;
  }
  native_mutex_unlock(&counter_mutex);
  my_thread_attr_destroy(&attr);

  native_mutex_lock(&sleeper_mutex);
  master_wakeup = 0;
  native_mutex_unlock(&sleeper_mutex);
  native_cond_broadcast(&sleep_threshold);

  gettimeofday(&start_time, nullptr);

  /*
    We loop until we know that all children have cleaned up.
  */
  native_mutex_lock(&counter_mutex);
  while (thread_counter) {
    struct timespec abstime;

    set_timespec(&abstime, 3);
    native_cond_timedwait(&count_threshold, &counter_mutex, &abstime);
  }
  native_mutex_unlock(&counter_mutex);

  gettimeofday(&end_time, nullptr);

  sptr->timing = timedif(end_time, start_time);
  sptr->users = concur;
  sptr->rows = limit;

  return 0;
}

extern "C" void *run_task(void *p) {
  ulonglong counter = 0, queries;
  ulonglong detach_counter;
  unsigned int commit_counter;
  MYSQL *mysql;
  MYSQL_RES *result;
  MYSQL_ROW row;
  statement *ptr;
  thread_context *con = (thread_context *)p;

  {
    DBUG_TRACE;
    DBUG_PRINT("info",
               ("task script \"%s\"", con->stmt ? con->stmt->string : ""));

    native_mutex_lock(&sleeper_mutex);
    while (master_wakeup) {
      native_cond_wait(&sleep_threshold, &sleeper_mutex);
    }
    native_mutex_unlock(&sleeper_mutex);

    if (!(mysql = mysql_init(nullptr))) {
      fprintf(stderr, "%s: mysql_init() failed ERROR : %s\n", my_progname,
              mysql_error(mysql));
      exit(0);
    }

    if (mysql_thread_init()) {
      fprintf(stderr, "%s: mysql_thread_init() failed ERROR : %s\n",
              my_progname, mysql_error(mysql));
      mysql_close(mysql);
      exit(0);
    }

    SSL_SET_OPTIONS(mysql);
    if (opt_protocol)
      mysql_options(mysql, MYSQL_OPT_PROTOCOL, (char *)&opt_protocol);

    DBUG_PRINT("info", ("trying to connect to host %s as user %s", host, user));

    if (!opt_only_print) {
      if (slap_connect(mysql)) goto end;
    }

    DBUG_PRINT("info", ("connected."));
    if (verbose >= 3) printf("connected!\n");
    queries = 0;

    commit_counter = 0;
    if (commit_rate)
      run_query(mysql, "SET AUTOCOMMIT=0", strlen("SET AUTOCOMMIT=0"));

  limit_not_met:
    for (ptr = con->stmt, detach_counter = 0; ptr && ptr->length;
         ptr = ptr->next, detach_counter++) {
      if (!opt_only_print && detach_rate && !(detach_counter % detach_rate)) {
        mysql_close(mysql);

        if (!(mysql = mysql_init(nullptr))) {
          fprintf(stderr, "%s: mysql_init() failed ERROR : %s\n", my_progname,
                  mysql_error(mysql));
          goto end;
        }

        if (slap_connect(mysql)) goto end;
      }

      /*
        We have to execute differently based on query type. This should become a
        function.
      */
      if ((ptr->type == UPDATE_TYPE_REQUIRES_PREFIX) ||
          (ptr->type == SELECT_TYPE_REQUIRES_PREFIX)) {
        size_t length;
        unsigned int key_val;
        char *key;
        char buffer[HUGE_STRING_LENGTH];

        /*
          This should only happen if some sort of new engine was
          implemented that didn't properly handle UPDATEs.

          Just in case someone runs this under an experimental engine we don't
          want a crash so the if() is placed here.
        */
        DBUG_ASSERT(primary_keys_number_of);
        if (primary_keys_number_of) {
          key_val = (unsigned int)(random() % primary_keys_number_of);
          key = primary_keys[key_val];

          DBUG_ASSERT(key);

          length = snprintf(buffer, HUGE_STRING_LENGTH, "%.*s '%s'",
                            (int)ptr->length, ptr->string, key);

          if (run_query(mysql, buffer, length)) {
            fprintf(stderr, "%s: Cannot run query %.*s ERROR : %s\n",
                    my_progname, (uint)length, buffer, mysql_error(mysql));
            goto end;
          }
        }
      } else {
        if (run_query(mysql, ptr->string, ptr->length)) {
          fprintf(stderr, "%s: Cannot run query %.*s ERROR : %s\n", my_progname,
                  (uint)ptr->length, ptr->string, mysql_error(mysql));
          goto end;
        }
      }

      do {
        if (mysql_field_count(mysql)) {
          if (!(result = mysql_store_result(mysql)))
            fprintf(stderr, "%s: Error when storing result: %d %s\n",
                    my_progname, mysql_errno(mysql), mysql_error(mysql));
          else {
            while ((row = mysql_fetch_row(result))) counter++;
            mysql_free_result(result);
          }
        }
      } while (mysql_next_result(mysql) == 0);
      queries++;

      if (commit_rate && (++commit_counter == commit_rate)) {
        commit_counter = 0;
        run_query(mysql, "COMMIT", strlen("COMMIT"));
      }

      if (con->limit && queries == con->limit) goto end;
    }

    if (con->limit && queries < con->limit) goto limit_not_met;

  end:
    if (commit_rate) run_query(mysql, "COMMIT", strlen("COMMIT"));

    mysql_close(mysql);

    mysql_thread_end();

    native_mutex_lock(&counter_mutex);
    thread_counter--;
    native_cond_signal(&count_threshold);
    native_mutex_unlock(&counter_mutex);
  }
  my_thread_exit(nullptr);
  return nullptr;
}

int parse_option(const char *origin, option_string **stmt, char delm) {
  const char *retstr;
  const char *ptr = origin;
  option_string **sptr = stmt;
  option_string *tmp;
  size_t length = strlen(origin);
  uint count = 0; /* We know that there is always one */

  for (tmp = *sptr = (option_string *)my_malloc(
           PSI_NOT_INSTRUMENTED, sizeof(option_string),
           MYF(MY_ZEROFILL | MY_FAE | MY_WME));
       (retstr = strchr(ptr, delm));
       tmp->next = (option_string *)my_malloc(
           PSI_NOT_INSTRUMENTED, sizeof(option_string),
           MYF(MY_ZEROFILL | MY_FAE | MY_WME)),
      tmp = tmp->next) {
    char buffer[HUGE_STRING_LENGTH];
    char *buffer_ptr;

    /*
      Return an error if the length of the any of the comma seprated value
      exceeds HUGE_STRING_LENGTH.
    */
    if ((size_t)(retstr - ptr) > HUGE_STRING_LENGTH) return -1;

    count++;
    strncpy(buffer, ptr, (size_t)(retstr - ptr));
    buffer[retstr - ptr] = 0;
    if ((buffer_ptr = strchr(buffer, ':'))) {
      const char *option_ptr;

      tmp->length = (size_t)(buffer_ptr - buffer);
      tmp->string =
          my_strndup(PSI_NOT_INSTRUMENTED, ptr, (uint)tmp->length, MYF(MY_FAE));

      option_ptr = ptr + 1 + tmp->length;

      /* Move past the : and the first string */
      tmp->option_length = (size_t)(retstr - option_ptr);
      tmp->option = my_strndup(PSI_NOT_INSTRUMENTED, option_ptr,
                               (uint)tmp->option_length, MYF(MY_FAE));
    } else {
      tmp->string = my_strndup(PSI_NOT_INSTRUMENTED, ptr,
                               (size_t)(retstr - ptr), MYF(MY_FAE));
      tmp->length = (size_t)(retstr - ptr);
    }

    ptr += retstr - ptr + 1;
    if (isspace(*ptr)) ptr++;
    count++;
  }

  if (ptr != origin + length) {
    const char *origin_ptr;

    /*
      Return an error if the length of the any of the comma seprated value
      exceeds HUGE_STRING_LENGTH.
    */
    if (strlen(ptr) > HUGE_STRING_LENGTH) return -1;

    if ((origin_ptr = strchr(ptr, ':'))) {
      const char *option_ptr;

      tmp->length = (size_t)(origin_ptr - ptr);
      tmp->string =
          my_strndup(PSI_NOT_INSTRUMENTED, origin, tmp->length, MYF(MY_FAE));

      option_ptr = ptr + 1 + tmp->length;

      /* Move past the : and the first string */
      tmp->option_length = strlen(option_ptr);
      tmp->option = my_strndup(PSI_NOT_INSTRUMENTED, option_ptr,
                               tmp->option_length, MYF(MY_FAE));
    } else {
      tmp->length = strlen(ptr);
      tmp->string =
          my_strndup(PSI_NOT_INSTRUMENTED, ptr, tmp->length, MYF(MY_FAE));
    }

    count++;
  }

  return count;
}

uint parse_delimiter(const char *script, statement **stmt, char delm) {
  const char *retstr;
  const char *ptr = script;
  statement **sptr = stmt;
  statement *tmp;
  size_t length = strlen(script);
  uint count = 0; /* We know that there is always one */

  for (tmp = *sptr =
           (statement *)my_malloc(PSI_NOT_INSTRUMENTED, sizeof(statement),
                                  MYF(MY_ZEROFILL | MY_FAE | MY_WME));
       (retstr = strchr(ptr, delm));
       tmp->next =
           (statement *)my_malloc(PSI_NOT_INSTRUMENTED, sizeof(statement),
                                  MYF(MY_ZEROFILL | MY_FAE | MY_WME)),
      tmp = tmp->next) {
    count++;
    tmp->string = my_strndup(PSI_NOT_INSTRUMENTED, ptr, (uint)(retstr - ptr),
                             MYF(MY_FAE));
    tmp->length = (size_t)(retstr - ptr);
    ptr += retstr - ptr + 1;
    if (isspace(*ptr)) ptr++;
  }

  if (ptr != script + length) {
    tmp->string = my_strndup(PSI_NOT_INSTRUMENTED, ptr,
                             (uint)((script + length) - ptr), MYF(MY_FAE));
    tmp->length = (size_t)((script + length) - ptr);
    count++;
  }

  return count;
}

uint parse_comma(const char *string, uint **range) {
  uint count = 1, x; /* We know that there is always one */
  const char *retstr;
  const char *ptr = string;
  uint *nptr;

  for (; *ptr; ptr++)
    if (*ptr == ',') count++;

  /* One extra spot for the NULL */
  nptr = *range =
      (uint *)my_malloc(PSI_NOT_INSTRUMENTED, sizeof(uint) * (count + 1),
                        MYF(MY_ZEROFILL | MY_FAE | MY_WME));

  ptr = string;
  x = 0;
  while ((retstr = strchr(ptr, ','))) {
    nptr[x++] = atoi(ptr);
    ptr += retstr - ptr + 1;
  }
  nptr[x++] = atoi(ptr);

  return count;
}

void print_conclusions(conclusions *con) {
  printf("Benchmark\n");
  if (con->engine) printf("\tRunning for engine %s\n", con->engine);
  printf("\tAverage number of seconds to run all queries: %ld.%03ld seconds\n",
         con->avg_timing / 1000, con->avg_timing % 1000);
  printf("\tMinimum number of seconds to run all queries: %ld.%03ld seconds\n",
         con->min_timing / 1000, con->min_timing % 1000);
  printf("\tMaximum number of seconds to run all queries: %ld.%03ld seconds\n",
         con->max_timing / 1000, con->max_timing % 1000);
  printf("\tNumber of clients running queries: %d\n", con->users);
  printf("\tAverage number of queries per client: %llu\n", con->avg_rows);
  printf("\n");
}

void print_conclusions_csv(conclusions *con) {
  char buffer[HUGE_STRING_LENGTH];
  const char *ptr = auto_generate_sql_type ? auto_generate_sql_type : "query";
  snprintf(buffer, HUGE_STRING_LENGTH,
           "%s,%s,%ld.%03ld,%ld.%03ld,%ld.%03ld,%d,%llu\n",
           con->engine ? con->engine : "", /* Storage engine we ran against */
           ptr,                            /* Load type */
           con->avg_timing / 1000, con->avg_timing % 1000, /* Time to load */
           con->min_timing / 1000, con->min_timing % 1000, /* Min time */
           con->max_timing / 1000, con->max_timing % 1000, /* Max time */
           con->users,                                     /* Children used */
           con->avg_rows                                   /* Queries run */
  );
  my_write(csv_file, (uchar *)buffer, (uint)strlen(buffer), MYF(0));
}

void generate_stats(conclusions *con, option_string *eng, stats *sptr) {
  stats *ptr;
  unsigned int x;

  con->min_timing = sptr->timing;
  con->max_timing = sptr->timing;
  con->min_rows = sptr->rows;
  con->max_rows = sptr->rows;

  /* At the moment we assume uniform */
  con->users = sptr->users;
  con->avg_rows = sptr->rows;

  /* With no next, we know it is the last element that was malloced */
  for (ptr = sptr, x = 0; x < iterations; ptr++, x++) {
    con->avg_timing += ptr->timing;

    if (ptr->timing > con->max_timing) con->max_timing = ptr->timing;
    if (ptr->timing < con->min_timing) con->min_timing = ptr->timing;
  }
  con->avg_timing = con->avg_timing / iterations;

  if (eng && eng->string)
    con->engine = eng->string;
  else
    con->engine = nullptr;
}

void option_cleanup(option_string *stmt) {
  option_string *ptr, *nptr;
  if (!stmt) return;

  for (ptr = stmt; ptr; ptr = nptr) {
    nptr = ptr->next;
    my_free(ptr->string);
    my_free(ptr->option);
    my_free(ptr);
  }
}

void statement_cleanup(statement *stmt) {
  statement *ptr, *nptr;
  if (!stmt) return;

  for (ptr = stmt; ptr; ptr = nptr) {
    nptr = ptr->next;
    my_free(ptr->string);
    my_free(ptr);
  }
}

int slap_connect(MYSQL *mysql) {
  /* Connect to server */
  static ulong connection_retry_sleep = 100000; /* Microseconds */
  int x, connect_error = 1;
  for (x = 0; x < 10; x++) {
    if (mysql_real_connect(mysql, host, user, opt_password,
                           create_schema_string, opt_mysql_port,
                           opt_mysql_unix_port, connect_flags)) {
      /* Connect suceeded */
      connect_error = 0;
      break;
    }
    my_sleep(connection_retry_sleep);
  }
  if (connect_error) {
    fprintf(stderr, "%s: Error when connecting to server: %d %s\n", my_progname,
            mysql_errno(mysql), mysql_error(mysql));
    return 1;
  }

  return 0;
}
