/* Copyright (c) 2002, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include <mysql.h>
#include <mysql/client_plugin.h>
#include <mysqld_error.h>
#include <openssl/ssl.h>

#include <algorithm>

#include "errmsg.h"
#include "m_ctype.h"
#include "m_string.h"
#include "my_alloc.h"
#include "my_default.h"
#include "my_getopt.h"
#include "my_sys.h"
#include "mysql/service_mysql_alloc.h"
#include "print_version.h"
#include "sql_common.h"
#include "violite.h"
#include "welcome_copyright_notice.h" /* ORACLE_WELCOME_COPYRIGHT_NOTICE */

#define MAX_TEST_QUERY_LENGTH 300 /* MAX QUERY BUFFER LENGTH */
#define MAX_KEY MAX_INDEXES

/* set default options */
static int opt_testcase = 0;
static char *opt_db = nullptr;
static char *opt_user = nullptr;
static char *opt_password = nullptr;
static char *opt_host = nullptr;
static char *opt_unix_socket = nullptr;
#if defined(_WIN32)
static char *shared_memory_base_name = 0;
#endif
static unsigned int opt_port;
static bool tty_password = false;
static int opt_silent = 0;

static MYSQL *mysql = nullptr;
static char current_db[] = "client_test_db";
static unsigned int test_count = 0;
static unsigned int opt_count = 0;
static unsigned int opt_count_read = 0;
static unsigned int iter_count = 0;
static bool have_innodb = false;
static char *opt_plugin_dir = nullptr, *opt_default_auth = nullptr;
static unsigned int opt_drop_db = 1;

static const char *opt_basedir = "./";
static const char *opt_vardir = "mysql-test/var";

static longlong opt_getopt_ll_test = 0;

static int original_argc;
static char **original_argv;

static time_t start_time, end_time;
static double total_time;

const char *default_dbug_option = "d:t:o,/tmp/mysql_client_test.trace";

/*
Read and parse arguments and MySQL options from my.cnf
*/
static const char *client_test_load_default_groups[] = {"client", nullptr};

struct my_tests_st {
  const char *name;
  void (*function)();
};

#define myheader(str)                                                         \
  DBUG_PRINT("test", ("name: %s", str));                                      \
  if (opt_silent < 2) {                                                       \
    fprintf(stdout, "\n\n#####################################\n");           \
    fprintf(stdout, "%u of (%u/%u): %s", test_count++, iter_count, opt_count, \
            str);                                                             \
    fprintf(stdout, "  \n#####################################\n");           \
  }

#define myheader_r(str)                                             \
  DBUG_PRINT("test", ("name: %s", str));                            \
  if (!opt_silent) {                                                \
    fprintf(stdout, "\n\n#####################################\n"); \
    fprintf(stdout, "%s", str);                                     \
    fprintf(stdout, "  \n#####################################\n"); \
  }

static void print_error(MYSQL *l_mysql, const char *msg);
static void print_st_error(MYSQL_STMT *stmt, const char *msg);
static void client_disconnect(MYSQL *mysql);
static void get_options(int *argc, char ***argv);
static void die(const char *file, int line, const char *expr)
    MY_ATTRIBUTE((noreturn));

/*
Abort unless given experssion is non-zero.

SYNOPSIS
DIE_UNLESS(expr)

DESCRIPTION
We can't use any kind of system assert as we need to
preserve tested invariants in release builds as well.
*/

#define DIE_UNLESS(expr) \
  ((void)((expr) ? 0 : (die(__FILE__, __LINE__, #expr), 0)))
#define DIE_IF(expr) ((void)((expr) ? (die(__FILE__, __LINE__, #expr), 0) : 0))
#define DIE(expr) die(__FILE__, __LINE__, #expr)

static void die(const char *file, int line, const char *expr) {
  fflush(stdout);
  fprintf(stderr, "%s:%d: check failed: '%s'\n", file, line, expr);
  fflush(stderr);
  exit(1);
}

#define myerror(msg) print_error(mysql, msg)
#define myerror2(l_mysql, msg) print_error(l_mysql, msg)
#define mysterror(stmt, msg) print_st_error(stmt, msg)

#define myquery(RES)      \
  {                       \
    int r = (RES);        \
    if (r) myerror(NULL); \
    DIE_UNLESS(r == 0);   \
  }

#define myquery2(L_MYSQL, RES)      \
  {                                 \
    int r = (RES);                  \
    if (r) myerror2(L_MYSQL, NULL); \
    DIE_UNLESS(r == 0);             \
  }

#define myquery_r(r)      \
  {                       \
    if (r) myerror(NULL); \
    DIE_UNLESS(r != 0);   \
  }

#define check_execute(stmt, r)    \
  {                               \
    if (r) mysterror(stmt, NULL); \
    DIE_UNLESS(r == 0);           \
  }

#define check_execute_r(stmt, r)  \
  {                               \
    if (r) mysterror(stmt, NULL); \
    DIE_UNLESS(r != 0);           \
  }

#define check_stmt(stmt)          \
  {                               \
    if (stmt == 0) myerror(NULL); \
    DIE_UNLESS(stmt != 0);        \
  }

#define check_stmt_r(stmt)        \
  {                               \
    if (stmt == 0) myerror(NULL); \
    DIE_UNLESS(stmt == 0);        \
  }

#define mytest(x)      \
  if (!(x)) {          \
    myerror(NULL);     \
    DIE_UNLESS(false); \
  }
#define mytest_r(x)    \
  if ((x)) {           \
    myerror(NULL);     \
    DIE_UNLESS(false); \
  }

/* Silence unused function warnings for some of the static functions. */
static int cmp_double(double *a, double *b) MY_ATTRIBUTE((unused));
static void verify_col_data(const char *table, const char *col,
                            const char *exp_data) MY_ATTRIBUTE((unused));
static void do_verify_prepare_field(MYSQL_RES *result, unsigned int no,
                                    const char *name, const char *org_name,
                                    enum enum_field_types type,
                                    const char *table, const char *org_table,
                                    const char *db, unsigned long length,
                                    const char *def, const char *file, int line)
    MY_ATTRIBUTE((unused));
static void verify_st_affected_rows(MYSQL_STMT *stmt, ulonglong exp_count)
    MY_ATTRIBUTE((unused));
static void verify_affected_rows(ulonglong exp_count) MY_ATTRIBUTE((unused));
static void verify_field_count(MYSQL_RES *result, uint exp_count)
    MY_ATTRIBUTE((unused));
static void execute_prepare_query(const char *query, ulonglong exp_count)
    MY_ATTRIBUTE((unused));
static bool thread_query(const char *query) MY_ATTRIBUTE((unused));

/* A workaround for Sun Forte 5.6 on Solaris x86 */

static int cmp_double(double *a, double *b) { return *a == *b; }

/* Print the error message */

static void print_error(MYSQL *l_mysql, const char *msg) {
  if (!opt_silent) {
    if (l_mysql && mysql_errno(l_mysql)) {
      if (l_mysql->server_version)
        fprintf(stdout, "\n [MySQL-%s]", l_mysql->server_version);
      else
        fprintf(stdout, "\n [MySQL]");
      fprintf(stdout, "[%d] %s\n", mysql_errno(l_mysql), mysql_error(l_mysql));
    } else if (msg)
      fprintf(stderr, " [MySQL] %s\n", msg);
  }
}

static void print_st_error(MYSQL_STMT *stmt, const char *msg) {
  if (!opt_silent) {
    if (stmt && mysql_stmt_errno(stmt)) {
      if (stmt->mysql && stmt->mysql->server_version)
        fprintf(stdout, "\n [MySQL-%s]", stmt->mysql->server_version);
      else
        fprintf(stdout, "\n [MySQL]");

      fprintf(stdout, "[%d] %s\n", mysql_stmt_errno(stmt),
              mysql_stmt_error(stmt));
    } else if (msg)
      fprintf(stderr, " [MySQL] %s\n", msg);
  }
}

/*
Enhanced version of mysql_client_init(), which may also set shared memory
base on Windows.
*/
static MYSQL *mysql_client_init(MYSQL *con) {
  MYSQL *res = mysql_init(con);
  uint ssl_mode = SSL_MODE_REQUIRED;
#if defined(_WIN32)
  if (res && shared_memory_base_name)
    mysql_options(res, MYSQL_SHARED_MEMORY_BASE_NAME, shared_memory_base_name);
#endif
  if (opt_plugin_dir && *opt_plugin_dir)
    mysql_options(res, MYSQL_PLUGIN_DIR, opt_plugin_dir);

  if (opt_default_auth && *opt_default_auth)
    mysql_options(res, MYSQL_DEFAULT_AUTH, opt_default_auth);

  mysql_options(res, MYSQL_OPT_SSL_MODE, &ssl_mode);
  return res;
}

/*
Extended version of mysql_client_init(), which also sets client and CA
certificates on the initialized connection object.
 */
static MYSQL *mysql_client_init_certs(MYSQL *con) {
  MYSQL *res;
  char file_name[
#ifdef PATH_MAX
      PATH_MAX
#elif defined(MAX_PATH)
      MAX_PATH
#else
      1024
#endif
  ];
  enum mysql_ssl_mode ssl_mode = SSL_MODE_VERIFY_IDENTITY;
  const char *test_dir = getenv("MYSQL_TEST_DIR");
  const char *ssl_cert_path = "/std_data/client-cert.pem";
  const char *ssl_key_path = "/std_data/client-key.pem";
  const char *ssl_ca_path = "/std_data/cacert.pem";

  res = mysql_client_init(con);

  strxmov(file_name, test_dir, ssl_cert_path, NullS);
  if (!opt_silent) {
    fprintf(stdout, "MYSQL_OPT_SSL_CERT set to: '%s'\n", file_name);
  }
  mysql_options(res, MYSQL_OPT_SSL_CERT, file_name);

  strxmov(file_name, test_dir, ssl_key_path, NullS);
  if (!opt_silent) {
    fprintf(stdout, "MYSQL_OPT_SSL_KEY set to: '%s'\n", file_name);
  }
  mysql_options(res, MYSQL_OPT_SSL_KEY, file_name);

  strxmov(file_name, test_dir, ssl_ca_path, NullS);
  if (!opt_silent) {
    fprintf(stdout, "MYSQL_OPT_SSL_CA set to: '%s'\n", file_name);
  }
  mysql_options(res, MYSQL_OPT_SSL_CA, file_name);
  mysql_options(res, MYSQL_OPT_SSL_MODE, &ssl_mode);

  return res;
}

/*
Helper function that allocates and initializes SSL_CTX structure
ready to be used for connecting to mysqld.
 */
static SSL_CTX *init_ssl_ctx() {
  char file_name[
#ifdef PATH_MAX
      PATH_MAX
#elif defined(MAX_PATH)
      MAX_PATH
#else
      1024
#endif
  ];
  const char *test_dir = getenv("MYSQL_TEST_DIR");
  const char *ssl_cert_path = "/std_data/client-cert.pem";
  const char *ssl_key_path = "/std_data/client-key.pem";
  const char *ssl_ca_path = "/std_data/cacert.pem";

  SSL_CTX *ctx = SSL_CTX_new(
#ifdef HAVE_TLSv13
      TLS_client_method());
#else  /* HAVE_TLSv13 */
      SSLv23_client_method());
#endif /* HAVE_TLSv13 */
  if (!ctx) {
    fprintf(stdout, "SSL_CTX structure initialization failed.\n");
    return NULL;
  }

  /* Load certs from the trusted ca */
  strxmov(file_name, test_dir, ssl_ca_path, NullS);
  if (!opt_silent) {
    fprintf(stdout, "MYSQL_OPT_SSL_CA set to: '%s'\n", file_name);
  }

  if (SSL_CTX_load_verify_locations(ctx, file_name, NULL) <= 0) {
    fprintf(stdout, "SSL_CTX_load_verify_locations() failed\n");

    /* otherwise go use the defaults */
    if (SSL_CTX_set_default_verify_paths(ctx) == 0) {
      fprintf(stdout, "SSL_CTX_set_default_verify_paths() failed\n");
      return NULL;
    }
  }

  strxmov(file_name, test_dir, ssl_cert_path, NullS);
  if (!opt_silent) {
    fprintf(stdout, "MYSQL_OPT_SSL_CERT set to: '%s'\n", file_name);
  }
  if (SSL_CTX_use_certificate_file(ctx, file_name, SSL_FILETYPE_PEM) != 1) {
    fprintf(stdout, "SSL_CTX_use_certificate_file() failed\n");
    return NULL;
  }

  strxmov(file_name, test_dir, ssl_key_path, NullS);
  if (!opt_silent) {
    fprintf(stdout, "MYSQL_OPT_SSL_KEY set to: '%s'\n", file_name);
  }
  if (SSL_CTX_use_PrivateKey_file(ctx, file_name, SSL_FILETYPE_PEM) != 1) {
    fprintf(stdout, "SSL_CTX_use_PrivateKey_file() failed\n");
    return NULL;
  }

  SSL_CTX_set_options(ctx, process_tls_version("TLSv1,TLSv1.1,TLSv1.2"));

  return ctx;
}

/*
Helper function establishing connectio to MySQL using given pre-initialized
MYSQL structure. Returns "1" if the connection was established successfully,
returns "0" otherwise.
 */
static int mysql_try_connect(MYSQL *conn) {
  enum connect_stage cs;

  if (!mysql_real_connect(conn, opt_host, opt_user, opt_password,
                          opt_db ? opt_db : "test", opt_port, opt_unix_socket,
                          0)) {
    if (!opt_silent)
      fprintf(stdout, "mysql_real_connect() failed: '%s'\n", mysql_error(conn));
    return 0;
  } else {
    fprintf(stdout, "mysql_real_connect() succeeded\n");
  }

  cs = mysql_get_connect_stage(conn);
  if (cs != CONNECT_STAGE_COMPLETE) {
    fprintf(stdout,
            "\n Sync:Expected connect_stage as CONNECT_STAGE_COMPLETE, its %d",
            cs);
    return 0;
  }

  return 1;
}

/*
Server cert validation callback matching cert subject string
against the expected pattern.
 */
static int server_cert_verifier(X509 *server_cert, const void * /*context*/,
                                const char **errptr) {
  const char *expected_server_cert_subj =
      "/C=SE/ST=Stockholm/L=Stockholm/O=Oracle/OU=MySQL/CN=localhost";
  char buf[1024];
  char *subj =
      X509_NAME_oneline(X509_get_subject_name(server_cert), buf, sizeof(buf));
  if (!opt_silent) {
    fprintf(stdout, "Server cert subject name: '%s'\n", subj);
  }
  if (strcmp(expected_server_cert_subj, subj) != 0) {
    *errptr = "Incorrect server cert subject";
    return 1;
  } else {
    return 0;
  }
}

/*
Server cert validation callback that expects no context pointer to be specified.
 */
static int server_cert_verifier_no_context(X509 *server_cert,
                                           const void *context,
                                           const char **errptr) {
  if (context != NULL) {
    fprintf(stdout,
            "server_cert_verifier_no_context(): invalid context pointer\n");
    *errptr = "Invalid callback context pointer";
    return 1;
  }

  return server_cert_verifier(server_cert, context, errptr);
}

/*
Server cert validation callback that expects a particular context pointer
to be specified.
 */
static int server_cert_verifier_with_context(X509 *server_cert,
                                             const void *context,
                                             const char **errptr) {
  if (context != reinterpret_cast<const void *>(0x123456)) {
    fprintf(stdout,
            "server_cert_verifier_no_context(): invalid context pointer\n");
    *errptr = "Invalid callback context pointer";
    return 1;
  }

  return server_cert_verifier(server_cert, context, errptr);
}

/*
Server cert validation callback for negative testing
 */
static int server_cert_verifier_fail(X509 * /*server_cert*/,
                                     const void * /*context*/,
                                     const char ** /*errptr*/) {
  /* Always fail the check */
  if (!opt_silent) {
    fprintf(stdout, "Failing server cert validation (expected)");
  }
  return 1;
}

/*
Disable direct calls of mysql_init, as it disregards  shared memory base.
*/
#define mysql_init(A) Please use mysql_client_init instead of mysql_init

/* Check if the connection has InnoDB tables */

static bool check_have_innodb(MYSQL *conn) {
  MYSQL_RES *res;
  MYSQL_ROW row;
  int rc;
  bool result = false;

  rc = mysql_query(
      conn,
      "SELECT (support = 'YES' or support = 'DEFAULT' or support = 'ENABLED') "
      "AS `TRUE` FROM information_schema.engines WHERE engine = 'innodb'");
  myquery(rc);
  res = mysql_use_result(conn);
  DIE_UNLESS(res);

  row = mysql_fetch_row(res);
  DIE_UNLESS(row);

  if (row[0] && row[1]) result = strcmp(row[1], "1") == 0;
  mysql_free_result(res);
  return result;
}

/*
This is to be what mysql_query() is for mysql_real_query(), for
mysql_simple_prepare(): a variant without the 'length' parameter.
*/

static MYSQL_STMT *STDCALL mysql_simple_prepare(MYSQL *mysql_arg,
                                                const char *query) {
  MYSQL_STMT *stmt = mysql_stmt_init(mysql_arg);
  if (stmt && mysql_stmt_prepare(stmt, query, (ulong)strlen(query))) {
    mysql_stmt_close(stmt);
    return nullptr;
  }
  return stmt;
}

/**
Connect to the server with options given by arguments to this application,
stored in global variables opt_host, opt_user, opt_password, opt_db,
opt_port and opt_unix_socket.

@param flag           client_flag passed on to mysql_real_connect
@param protocol       MYSQL_PROTOCOL_* to use for this connection
@param auto_reconnect set to 1 for auto reconnect

@return pointer to initialized and connected MYSQL object
*/
static MYSQL *client_connect(ulong flag, uint protocol, bool auto_reconnect) {
  MYSQL *mysql;
  int rc;
  static char query[MAX_TEST_QUERY_LENGTH];
  myheader_r("client_connect");

  if (!opt_silent)
    fprintf(stdout, "\n Establishing a connection to '%s' ...",
            opt_host ? opt_host : "");

  if (!(mysql = mysql_client_init(nullptr))) {
    opt_silent = 0;
    myerror("mysql_client_init() failed");
    exit(1);
  }
  /* enable local infile, in non-binary builds often disabled by default */
  mysql_options(mysql, MYSQL_OPT_LOCAL_INFILE, nullptr);
  mysql_options(mysql, MYSQL_OPT_PROTOCOL, &protocol);
  if (opt_plugin_dir && *opt_plugin_dir)
    mysql_options(mysql, MYSQL_PLUGIN_DIR, opt_plugin_dir);

  if (opt_default_auth && *opt_default_auth)
    mysql_options(mysql, MYSQL_DEFAULT_AUTH, opt_default_auth);

  if (!(mysql_real_connect(mysql, opt_host, opt_user, opt_password,
                           opt_db ? opt_db : "test", opt_port, opt_unix_socket,
                           flag))) {
    opt_silent = 0;
    myerror("connection failed");
    mysql_close(mysql);
    fprintf(stdout, "\n Check the connection options using --help or -?\n");
    exit(1);
  }
  mysql->reconnect = auto_reconnect;

  if (!opt_silent) fprintf(stdout, "OK");

  /* set AUTOCOMMIT to ON*/
  mysql_autocommit(mysql, true);

  if (!opt_silent) {
    fprintf(stdout, "\nConnected to MySQL server version: %s (%lu)\n",
            mysql_get_server_info(mysql),
            (ulong)mysql_get_server_version(mysql));
    fprintf(stdout, "\n Creating a test database '%s' ...", current_db);
  }
  strxmov(query, "CREATE DATABASE IF NOT EXISTS ", current_db, NullS);

  rc = mysql_query(mysql, query);
  myquery(rc);

  strxmov(query, "USE ", current_db, NullS);
  rc = mysql_query(mysql, query);
  myquery(rc);
  have_innodb = check_have_innodb(mysql);

  if (!opt_silent) fprintf(stdout, "OK");

  return mysql;
}

/* Close the connection */

static void client_disconnect(MYSQL *mysql) {
  static char query[MAX_TEST_QUERY_LENGTH];

  myheader_r("client_disconnect");

  if (mysql) {
    if (opt_drop_db) {
      if (!opt_silent)
        fprintf(stdout, "\n dropping the test database '%s' ...", current_db);
      strxmov(query, "DROP DATABASE IF EXISTS ", current_db, NullS);

      mysql_query(mysql, query);
      if (!opt_silent) fprintf(stdout, "OK");
    }

    if (!opt_silent) fprintf(stdout, "\n closing the connection ...");
    mysql_close(mysql);
    if (!opt_silent) fprintf(stdout, "OK\n");
  }
}

/* Print dashes */

static void my_print_dashes(MYSQL_RES *result) {
  MYSQL_FIELD *field;
  unsigned int i, j;

  mysql_field_seek(result, 0);
  fputc('\t', stdout);
  fputc('+', stdout);

  for (i = 0; i < mysql_num_fields(result); i++) {
    field = mysql_fetch_field(result);
    for (j = 0; j < field->max_length + 2; j++) fputc('-', stdout);
    fputc('+', stdout);
  }
  fputc('\n', stdout);
}

/* Print resultset metadata information */

static void my_print_result_metadata(MYSQL_RES *result) {
  MYSQL_FIELD *field;
  unsigned int i;
  size_t j;
  unsigned int field_count;

  mysql_field_seek(result, 0);
  if (!opt_silent) {
    fputc('\n', stdout);
    fputc('\n', stdout);
  }

  field_count = mysql_num_fields(result);
  for (i = 0; i < field_count; i++) {
    field = mysql_fetch_field(result);
    j = strlen(field->name);
    if (j < field->max_length) j = field->max_length;
    if (j < 4 && !IS_NOT_NULL(field->flags)) j = 4;
    field->max_length = (unsigned long)j;
  }
  if (!opt_silent) {
    my_print_dashes(result);
    fputc('\t', stdout);
    fputc('|', stdout);
  }

  mysql_field_seek(result, 0);
  for (i = 0; i < field_count; i++) {
    field = mysql_fetch_field(result);
    if (!opt_silent)
      fprintf(stdout, " %-*s |", (int)field->max_length, field->name);
  }
  if (!opt_silent) {
    fputc('\n', stdout);
    my_print_dashes(result);
  }
}

/* Process the result set */

static int my_process_result_set(MYSQL_RES *result) {
  MYSQL_ROW row;
  MYSQL_FIELD *field;
  unsigned int i;
  unsigned int row_count = 0;

  if (!result) return 0;

  my_print_result_metadata(result);

  while ((row = mysql_fetch_row(result)) != nullptr) {
    mysql_field_seek(result, 0);
    if (!opt_silent) {
      fputc('\t', stdout);
      fputc('|', stdout);
    }

    for (i = 0; i < mysql_num_fields(result); i++) {
      field = mysql_fetch_field(result);
      if (!opt_silent) {
        if (row[i] == nullptr)
          fprintf(stdout, " %-*s |", (int)field->max_length, "NULL");
        else if (IS_NUM(field->type))
          fprintf(stdout, " %*s |", (int)field->max_length, row[i]);
        else
          fprintf(stdout, " %-*s |", (int)field->max_length, row[i]);
      }
    }
    if (!opt_silent) {
      fputc('\t', stdout);
      fputc('\n', stdout);
    }
    row_count++;
  }
  if (!opt_silent) {
    if (row_count) my_print_dashes(result);

    if (mysql_errno(mysql) != 0)
      fprintf(stderr, "\n\tmysql_fetch_row() failed\n");
    else
      fprintf(stdout, "\n\t%d %s returned\n", row_count,
              row_count == 1 ? "row" : "rows");
  }
  return row_count;
}

static int my_process_result(MYSQL *mysql_arg) {
  MYSQL_RES *result;
  int row_count;

  if (!(result = mysql_store_result(mysql_arg))) return 0;

  row_count = my_process_result_set(result);

  mysql_free_result(result);
  return row_count;
}

/* Process the statement result set */

#define MAX_RES_FIELDS 50
#define MAX_FIELD_DATA_SIZE 255

static int my_process_stmt_result(MYSQL_STMT *stmt) {
  int field_count;
  int row_count = 0;
  MYSQL_BIND buffer[MAX_RES_FIELDS];
  MYSQL_FIELD *field;
  MYSQL_RES *result;
  char data[MAX_RES_FIELDS][MAX_FIELD_DATA_SIZE];
  ulong length[MAX_RES_FIELDS];
  bool is_null[MAX_RES_FIELDS];
  int rc, i;

  if (!(result = mysql_stmt_result_metadata(stmt))) /* No meta info */
  {
    while (!mysql_stmt_fetch(stmt)) row_count++;
    return row_count;
  }

  field_count = std::min(mysql_num_fields(result), unsigned{MAX_RES_FIELDS});

  memset(buffer, 0, sizeof(buffer));
  memset(length, 0, sizeof(length));
  memset(is_null, 0, sizeof(is_null));

  for (i = 0; i < field_count; i++) {
    buffer[i].buffer_type = MYSQL_TYPE_STRING;
    buffer[i].buffer_length = MAX_FIELD_DATA_SIZE;
    buffer[i].length = &length[i];
    buffer[i].buffer = (void *)data[i];
    buffer[i].is_null = &is_null[i];
  }

  rc = mysql_stmt_bind_result(stmt, buffer);
  check_execute(stmt, rc);

  rc = 1;
  mysql_stmt_attr_set(stmt, STMT_ATTR_UPDATE_MAX_LENGTH, (void *)&rc);
  rc = mysql_stmt_store_result(stmt);
  check_execute(stmt, rc);
  my_print_result_metadata(result);

  mysql_field_seek(result, 0);
  while ((rc = mysql_stmt_fetch(stmt)) == 0) {
    if (!opt_silent) {
      fputc('\t', stdout);
      fputc('|', stdout);
    }
    mysql_field_seek(result, 0);
    for (i = 0; i < field_count; i++) {
      field = mysql_fetch_field(result);
      if (!opt_silent) {
        if (is_null[i])
          fprintf(stdout, " %-*s |", (int)field->max_length, "NULL");
        else if (length[i] == 0) {
          data[i][0] = '\0'; /* unmodified buffer */
          fprintf(stdout, " %*s |", (int)field->max_length, data[i]);
        } else if (IS_NUM(field->type))
          fprintf(stdout, " %*s |", (int)field->max_length, data[i]);
        else
          fprintf(stdout, " %-*s |", (int)field->max_length, data[i]);
      }
    }
    if (!opt_silent) {
      fputc('\t', stdout);
      fputc('\n', stdout);
    }
    row_count++;
  }
  DIE_UNLESS(rc == MYSQL_NO_DATA);
  if (!opt_silent) {
    if (row_count) my_print_dashes(result);
    fprintf(stdout, "\n\t%d %s returned\n", row_count,
            row_count == 1 ? "row" : "rows");
  }
  mysql_free_result(result);
  return row_count;
}

/* Prepare statement, execute, and process result set for given query */

int my_stmt_result(const char *buff) {
  MYSQL_STMT *stmt;
  int row_count;
  int rc;

  if (!opt_silent) fprintf(stdout, "\n\n %s", buff);
  stmt = mysql_simple_prepare(mysql, buff);
  check_stmt(stmt);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  row_count = my_process_stmt_result(stmt);
  mysql_stmt_close(stmt);

  return row_count;
}

/* Print the total number of warnings and the warnings themselves.  */

void my_process_warnings(MYSQL *conn, unsigned expected_warning_count) {
  MYSQL_RES *result;
  int rc;

  if (!opt_silent)
    fprintf(stdout, "\n total warnings: %u (expected: %u)\n",
            mysql_warning_count(conn), expected_warning_count);

  DIE_UNLESS(mysql_warning_count(mysql) == expected_warning_count);

  rc = mysql_query(conn, "SHOW WARNINGS");
  DIE_UNLESS(rc == 0);

  result = mysql_store_result(conn);
  mytest(result);

  rc = my_process_result_set(result);
  mysql_free_result(result);
}

/* Utility function to verify a particular column data */

static void verify_col_data(const char *table, const char *col,
                            const char *exp_data) {
  static char query[MAX_TEST_QUERY_LENGTH];
  MYSQL_RES *result;
  MYSQL_ROW row;
  int rc, field = 1;

  if (table && col) {
    strxmov(query, "SELECT ", col, " FROM ", table, " LIMIT 1", NullS);
    if (!opt_silent) fprintf(stdout, "\n %s", query);
    rc = mysql_query(mysql, query);
    myquery(rc);

    field = 0;
  }

  result = mysql_use_result(mysql);
  mytest(result);

  if (!(row = mysql_fetch_row(result)) || !row[field]) {
    fprintf(stdout, "\n *** ERROR: FAILED TO GET THE RESULT ***");
    exit(1);
  }
  if (strcmp(row[field], exp_data)) {
    fprintf(stdout, "\n obtained: `%s` (expected: `%s`)", row[field], exp_data);
    DIE_UNLESS(false);
  }
  mysql_free_result(result);
}

/* Utility function to verify the field members */

#define verify_prepare_field(result, no, name, org_name, type, table,          \
                             org_table, db, length, def)                       \
  do_verify_prepare_field((result), (no), (name), (org_name), (type), (table), \
                          (org_table), (db), (length), (def), __FILE__,        \
                          __LINE__)

static void do_verify_prepare_field(MYSQL_RES *result, unsigned int no,
                                    const char *name, const char *org_name,
                                    enum enum_field_types type,
                                    const char *table, const char *org_table,
                                    const char *db, unsigned long length,
                                    const char *def, const char *file,
                                    int line) {
  MYSQL_FIELD *field;
  CHARSET_INFO *cs;
  ulonglong expected_field_length;

  if (!(field = mysql_fetch_field_direct(result, no))) {
    fprintf(stdout, "\n *** ERROR: FAILED TO GET THE RESULT ***");
    exit(1);
  }
  cs = get_charset(field->charsetnr, 0);
  DIE_UNLESS(cs);
  if ((expected_field_length = (ulonglong)length * cs->mbmaxlen) > UINT_MAX32)
    expected_field_length = UINT_MAX32;
  if (!opt_silent) {
    fprintf(stdout, "\n field[%d]:", no);
    fprintf(stdout, "\n    name     :`%s`\t(expected: `%s`)", field->name,
            name);
    fprintf(stdout, "\n    org_name :`%s`\t(expected: `%s`)", field->org_name,
            org_name);
    fprintf(stdout, "\n    type     :`%d`\t(expected: `%d`)", field->type,
            type);
    if (table)
      fprintf(stdout, "\n    table    :`%s`\t(expected: `%s`)", field->table,
              table);
    if (org_table)
      fprintf(stdout, "\n    org_table:`%s`\t(expected: `%s`)",
              field->org_table, org_table);
    fprintf(stdout, "\n    database :`%s`\t(expected: `%s`)", field->db, db);
    fprintf(stdout, "\n    length   :`%lu`\t(expected: `%llu`)", field->length,
            expected_field_length);
    fprintf(stdout, "\n    maxlength:`%ld`", field->max_length);
    fprintf(stdout, "\n    charsetnr:`%d`", field->charsetnr);
    fprintf(stdout, "\n    default  :`%s`\t(expected: `%s`)",
            field->def ? field->def : "(null)", def ? def : "(null)");
    fprintf(stdout, "\n");
  }
  DIE_UNLESS(strcmp(field->name, name) == 0);
  DIE_UNLESS(strcmp(field->org_name, org_name) == 0);
  /*
  XXX: silent column specification change works based on number of
  bytes a column occupies. So CHAR -> VARCHAR upgrade is possible even
  for CHAR(2) column if its character set is multibyte.
  VARCHAR -> CHAR downgrade won't work for VARCHAR(3) as one would
  expect.
  */
  if (cs->mbmaxlen == 1) {
    if (field->type != type) {
      fprintf(stderr,
              "Expected field type: %d,  got type: %d in file %s, line %d\n",
              (int)type, (int)field->type, file, line);
      DIE_UNLESS(field->type == type);
    }
  }
  if (table) DIE_UNLESS(strcmp(field->table, table) == 0);
  if (org_table) DIE_UNLESS(strcmp(field->org_table, org_table) == 0);
  DIE_UNLESS(strcmp(field->db, db) == 0);
  /*
  Character set should be taken into account for multibyte encodings, such
  as utf8. Field length is calculated as number of characters * maximum
  number of bytes a character can occupy.
  */
  if (length && (field->length != expected_field_length)) {
    fprintf(stderr, "Expected field length: %llu,  got length: %lu\n",
            expected_field_length, field->length);
    DIE_UNLESS(field->length == expected_field_length);
  }
  if (def) DIE_UNLESS(strcmp(field->def, def) == 0);
}

/* Utility function to verify the parameter count */

static void verify_param_count(MYSQL_STMT *stmt, long exp_count) {
  long param_count = mysql_stmt_param_count(stmt);
  if (!opt_silent)
    fprintf(stdout, "\n total parameters in stmt: `%ld` (expected: `%ld`)",
            param_count, exp_count);
  DIE_UNLESS(param_count == exp_count);
}

/* Utility function to verify the total affected rows */

static void verify_st_affected_rows(MYSQL_STMT *stmt, ulonglong exp_count) {
  ulonglong affected_rows = mysql_stmt_affected_rows(stmt);
  if (!opt_silent)
    fprintf(stdout, "\n total affected rows: `%ld` (expected: `%ld`)",
            (long)affected_rows, (long)exp_count);
  DIE_UNLESS(affected_rows == exp_count);
}

/* Utility function to verify the total affected rows */

static void verify_affected_rows(ulonglong exp_count) {
  ulonglong affected_rows = mysql_affected_rows(mysql);
  if (!opt_silent)
    fprintf(stdout, "\n total affected rows: `%ld` (expected: `%ld`)",
            (long)affected_rows, (long)exp_count);
  DIE_UNLESS(affected_rows == exp_count);
}

/* Utility function to verify the total fields count */

static void verify_field_count(MYSQL_RES *result, uint exp_count) {
  uint field_count = mysql_num_fields(result);
  if (!opt_silent)
    fprintf(stdout, "\n total fields in the result set: `%d` (expected: `%d`)",
            field_count, exp_count);
  DIE_UNLESS(field_count == exp_count);
}

/* Utility function to execute a query using prepare-execute */

static void execute_prepare_query(const char *query, ulonglong exp_count) {
  MYSQL_STMT *stmt;
  ulonglong affected_rows;
  int rc;

  stmt = mysql_simple_prepare(mysql, query);
  check_stmt(stmt);

  rc = mysql_stmt_execute(stmt);
  myquery(rc);

  affected_rows = mysql_stmt_affected_rows(stmt);
  if (!opt_silent)
    fprintf(stdout, "\n total affected rows: `%ld` (expected: `%ld`)",
            (long)affected_rows, (long)exp_count);

  DIE_UNLESS(affected_rows == exp_count);
  mysql_stmt_close(stmt);
}

/*
Accepts arbitrary number of queries and runs them against the database.
Used to fill tables for each test.
*/

void fill_tables(const char **query_list, unsigned query_count) {
  int rc;
  const char **query;
  DBUG_TRACE;
  for (query = query_list; query < query_list + query_count; ++query) {
    rc = mysql_query(mysql, *query);
    myquery(rc);
  }
}

/*
All state of fetch from one statement: statement handle, out buffers,
fetch position.
See fetch_n for for the only use case.
*/

enum { MAX_COLUMN_LENGTH = 255 };

struct Stmt_fetch {
  const char *query;
  unsigned stmt_no;
  MYSQL_STMT *handle;
  bool is_open;
  MYSQL_BIND *bind_array;
  char **out_data;
  unsigned long *out_data_length;
  unsigned column_count;
  unsigned row_count;
};

/*
Create statement handle, prepare it with statement, execute and allocate
fetch buffers.
*/

static void stmt_fetch_init(Stmt_fetch *fetch, unsigned stmt_no_arg,
                            const char *query_arg) {
  unsigned long type = CURSOR_TYPE_READ_ONLY;
  int rc;
  unsigned i;
  MYSQL_RES *metadata;
  DBUG_TRACE;

  /* Save query and statement number for error messages */
  fetch->stmt_no = stmt_no_arg;
  fetch->query = query_arg;

  fetch->handle = mysql_stmt_init(mysql);

  rc = mysql_stmt_prepare(fetch->handle, fetch->query,
                          (ulong)strlen(fetch->query));
  check_execute(fetch->handle, rc);

  /*
  The attribute is sent to server on execute and asks to open read-only
  for result set
  */
  mysql_stmt_attr_set(fetch->handle, STMT_ATTR_CURSOR_TYPE,
                      (const void *)&type);

  rc = mysql_stmt_execute(fetch->handle);
  check_execute(fetch->handle, rc);

  /* Find out total number of columns in result set */
  metadata = mysql_stmt_result_metadata(fetch->handle);
  fetch->column_count = mysql_num_fields(metadata);
  mysql_free_result(metadata);

  /*
  Now allocate bind handles and buffers for output data:
  calloc memory to reduce number of MYSQL_BIND members we need to
  set up.
  */

  fetch->bind_array =
      (MYSQL_BIND *)calloc(1, sizeof(MYSQL_BIND) * fetch->column_count);
  fetch->out_data = (char **)calloc(1, sizeof(char *) * fetch->column_count);
  fetch->out_data_length =
      (ulong *)calloc(1, sizeof(ulong) * fetch->column_count);
  for (i = 0; i < fetch->column_count; ++i) {
    fetch->out_data[i] = (char *)calloc(1, MAX_COLUMN_LENGTH);
    fetch->bind_array[i].buffer_type = MYSQL_TYPE_STRING;
    fetch->bind_array[i].buffer = fetch->out_data[i];
    fetch->bind_array[i].buffer_length = MAX_COLUMN_LENGTH;
    fetch->bind_array[i].length = fetch->out_data_length + i;
  }

  mysql_stmt_bind_result(fetch->handle, fetch->bind_array);

  fetch->row_count = 0;
  fetch->is_open = true;

  /* Ready for reading rows */
}

/* Fetch and print one row from cursor */

static int stmt_fetch_fetch_row(Stmt_fetch *fetch) {
  int rc;
  unsigned i;
  DBUG_TRACE;

  if ((rc = mysql_stmt_fetch(fetch->handle)) == 0) {
    ++fetch->row_count;
    if (!opt_silent)
      printf("Stmt %d fetched row %d:\n", fetch->stmt_no, fetch->row_count);
    for (i = 0; i < fetch->column_count; ++i) {
      fetch->out_data[i][fetch->out_data_length[i]] = '\0';
      if (!opt_silent) printf("column %d: %s\n", i + 1, fetch->out_data[i]);
    }
  } else
    fetch->is_open = false;
  return rc;
}

static void stmt_fetch_close(Stmt_fetch *fetch) {
  unsigned i;
  DBUG_TRACE;

  for (i = 0; i < fetch->column_count; ++i) free(fetch->out_data[i]);
  free(fetch->out_data);
  free(fetch->out_data_length);
  free(fetch->bind_array);
  mysql_stmt_close(fetch->handle);
}

/*
For given array of queries, open query_count cursors and fetch
from them in simultaneous manner.
In case there was an error in one of the cursors, continue
reading from the rest.
*/

enum fetch_type { USE_ROW_BY_ROW_FETCH = 0, USE_STORE_RESULT = 1 };

bool fetch_n(const char **query_list, unsigned query_count,
             enum fetch_type fetch_type) {
  unsigned open_statements = query_count;
  int rc, error_count = 0;
  Stmt_fetch *fetch_array =
      (Stmt_fetch *)calloc(1, sizeof(Stmt_fetch) * query_count);
  Stmt_fetch *fetch;
  DBUG_TRACE;

  for (fetch = fetch_array; fetch < fetch_array + query_count; ++fetch) {
    /* Init will exit(1) in case of error */
    stmt_fetch_init(fetch, fetch - fetch_array,
                    query_list[fetch - fetch_array]);
  }

  if (fetch_type == USE_STORE_RESULT) {
    for (fetch = fetch_array; fetch < fetch_array + query_count; ++fetch) {
      rc = mysql_stmt_store_result(fetch->handle);
      check_execute(fetch->handle, rc);
    }
  }

  while (open_statements) {
    for (fetch = fetch_array; fetch < fetch_array + query_count; ++fetch) {
      if (fetch->is_open && (rc = stmt_fetch_fetch_row(fetch))) {
        open_statements--;
        /*
        We try to fetch from the rest of the statements in case of
        error
        */
        if (rc != MYSQL_NO_DATA) {
          fprintf(stderr,
                  "Got error reading rows from statement %d,\n"
                  "query is: %s,\n"
                  "error message: %s",
                  (int)(fetch - fetch_array), fetch->query,
                  mysql_stmt_error(fetch->handle));
          error_count++;
        }
      }
    }
  }
  if (error_count)
    fprintf(stderr, "Fetch FAILED");
  else {
    unsigned total_row_count = 0;
    for (fetch = fetch_array; fetch < fetch_array + query_count; ++fetch)
      total_row_count += fetch->row_count;
    if (!opt_silent)
      printf("Success, total rows fetched: %d\n", total_row_count);
  }
  for (fetch = fetch_array; fetch < fetch_array + query_count; ++fetch)
    stmt_fetch_close(fetch);
  free(fetch_array);
  return error_count != 0;
}

/* Separate thread query to test some cases */

static bool thread_query(const char *query) {
  MYSQL *l_mysql;
  bool error;

  error = false;
  if (!opt_silent) fprintf(stdout, "\n in thread_query(%s)", query);
  if (!(l_mysql = mysql_client_init(nullptr))) {
    myerror("mysql_client_init() failed");
    return true;
  }
  if (!(mysql_real_connect(l_mysql, opt_host, opt_user, opt_password,
                           current_db, opt_port, opt_unix_socket, 0))) {
    myerror("connection failed");
    error = true;
    goto end;
  }
  l_mysql->reconnect = true;
  if (mysql_query(l_mysql, query)) {
    fprintf(stderr, "Query failed (%s)\n", mysql_error(l_mysql));
    error = true;
    goto end;
  }
  mysql_commit(l_mysql);
end:
  mysql_close(l_mysql);
  return error;
}

static struct my_option client_test_long_options[] = {
    {"basedir", 'b', "Basedir for tests.", &opt_basedir, &opt_basedir, nullptr,
     GET_STR, REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"count", 't', "Number of times test to be executed", &opt_count_read,
     &opt_count_read, nullptr, GET_UINT, REQUIRED_ARG, 1, 0, 0, nullptr, 0,
     nullptr},
    {"database", 'D', "Database to use", &opt_db, &opt_db, nullptr,
     GET_STR_ALLOC, REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"do-not-drop-database", 'd', "Do not drop database while disconnecting",
     nullptr, nullptr, nullptr, GET_NO_ARG, NO_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
    {"debug", '#', "Output debug log", &default_dbug_option,
     &default_dbug_option, nullptr, GET_STR, OPT_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
    {"help", '?', "Display this help and exit", nullptr, nullptr, nullptr,
     GET_NO_ARG, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"host", 'h', "Connect to host", &opt_host, &opt_host, nullptr,
     GET_STR_ALLOC, REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"password", 'p',
     "Password to use when connecting to server. If password is not given it's "
     "asked from the tty.",
     nullptr, nullptr, nullptr, GET_STR, OPT_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"port", 'P',
     "Port number to use for connection or 0 for default to, in "
     "order of preference, my.cnf, $MYSQL_TCP_PORT, "
#if MYSQL_PORT_DEFAULT == 0
     "/etc/services, "
#endif
     "built-in default (" STRINGIFY_ARG(MYSQL_PORT) ").",
     &opt_port, &opt_port, nullptr, GET_UINT, REQUIRED_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
    {"show-tests", 'T', "Show all tests' names", nullptr, nullptr, nullptr,
     GET_NO_ARG, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"silent", 's', "Be more silent", nullptr, nullptr, nullptr, GET_NO_ARG,
     NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
#if defined(_WIN32)
    {"shared-memory-base-name", 'm', "Base name of shared memory.",
     &shared_memory_base_name, (uchar **)&shared_memory_base_name, 0, GET_STR,
     REQUIRED_ARG, 0, 0, 0, 0, 0, 0},
#endif
    {"socket", 'S', "Socket file to use for connection", &opt_unix_socket,
     &opt_unix_socket, nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
    {"testcase", 'c',
     "May disable some code when runs as mysql-test-run testcase.", nullptr,
     nullptr, nullptr, GET_NO_ARG, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"user", 'u', "User for login if not current user", &opt_user, &opt_user,
     nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"vardir", 'v', "Data dir for tests.", &opt_vardir, &opt_vardir, nullptr,
     GET_STR, REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"getopt-ll-test", 'g', "Option for testing bug in getopt library",
     &opt_getopt_ll_test, &opt_getopt_ll_test, nullptr, GET_LL, REQUIRED_ARG, 0,
     0, LLONG_MAX, nullptr, 0, nullptr},
    {"plugin_dir", 0, "Directory for client-side plugins.", &opt_plugin_dir,
     &opt_plugin_dir, nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
    {"default_auth", 0, "Default authentication client-side plugin to use.",
     &opt_default_auth, &opt_default_auth, nullptr, GET_STR, REQUIRED_ARG, 0, 0,
     0, nullptr, 0, nullptr},
    {nullptr, 0, nullptr, nullptr, nullptr, nullptr, GET_NO_ARG, NO_ARG, 0, 0,
     0, nullptr, 0, nullptr}};

static void usage(void) {
  /* show the usage string when the user asks for this */
  print_version();
  puts(ORACLE_WELCOME_COPYRIGHT_NOTICE("2002"));
  printf("Usage: %s [OPTIONS] [TESTNAME1 TESTNAME2...]\n", my_progname);
  my_print_help(client_test_long_options);
  print_defaults("my", client_test_load_default_groups);
  my_print_variables(client_test_long_options);
}

static struct my_tests_st *get_my_tests(); /* To be defined in main .c file */

static struct my_tests_st *my_testlist = nullptr;

static bool get_one_option(int optid,
                           const struct my_option *opt MY_ATTRIBUTE((unused)),
                           char *argument) {
  switch (optid) {
    case '#':
      DBUG_PUSH(argument ? argument : default_dbug_option);
      break;
    case 'c':
      opt_testcase = 1;
      break;
    case 'p':
      if (argument) {
        char *start = argument;
        my_free(opt_password);
        opt_password = my_strdup(PSI_NOT_INSTRUMENTED, argument, MYF(MY_FAE));
        while (*argument) *argument++ = 'x'; /* Destroy argument */
        if (*start) start[1] = 0;
      } else
        tty_password = true;
      break;
    case 's':
      if (argument == disabled_my_option)
        opt_silent = 0;
      else
        opt_silent++;
      break;
    case 'd':
      opt_drop_db = 0;
      break;
    case 'T': {
      struct my_tests_st *fptr;

      printf("All possible test names:\n\n");
      for (fptr = my_testlist; fptr->name; fptr++) printf("%s\n", fptr->name);
      exit(0);
      break;
    }
    case '?':
    case 'I': /* Info */
      usage();
      exit(0);
      break;
  }
  return false;
}

static void get_options(int *argc, char ***argv) {
  int ho_error;

  /* reset --silent option */
  opt_silent = 0;

  if ((ho_error = handle_options(argc, argv, client_test_long_options,
                                 get_one_option)))
    exit(ho_error);

  if (tty_password) opt_password = get_tty_password(NullS);
  return;
}

/*
Print the test output on successful execution before exiting
*/

static void print_test_output() {
  if (opt_silent < 3) {
    fprintf(stdout, "\n\n");
    fprintf(stdout, "All '%d' tests were successful (in '%d' iterations)",
            test_count - 1, opt_count);
    if (!opt_silent) {
      fprintf(stdout, "\n  Total execution time: %g SECS", total_time);
      if (opt_count > 1)
        fprintf(stdout, " (Avg: %g SECS)", total_time / opt_count);
    }

    fprintf(stdout, "\n\n!!! SUCCESS !!!\n");
  }
}

/***************************************************************************
main routine
***************************************************************************/

int main(int argc, char **argv) {
  int i;
  char **tests_to_run = nullptr, **curr_test;
  struct my_tests_st *fptr;
  my_testlist = get_my_tests();

  MY_INIT(argv[0]);

  /* Copy the original arguments, so it can be reused for restarting. */
  original_argc = argc;
  original_argv = (char **)malloc(argc * sizeof(char *));
  if (argc && !original_argv) exit(1);
  for (i = 0; i < argc; i++) original_argv[i] = strdup(argv[i]);

  MEM_ROOT alloc{PSI_NOT_INSTRUMENTED, 512};
  if (load_defaults("my", client_test_load_default_groups, &argc, &argv,
                    &alloc))
    return 1;

  get_options(&argc, &argv);

  /* Set main opt_count. */
  opt_count = opt_count_read;

  /* If there are any arguments left (named tests), save them. */
  if (argc) {
    tests_to_run = (char **)malloc((argc + 1) * sizeof(char *));
    if (!tests_to_run) exit(1);
    for (i = 0; i < argc; i++) tests_to_run[i] = strdup(argv[i]);
    tests_to_run[i] = nullptr;
  }

  if (mysql_server_init(0, nullptr, nullptr))
    DIE("Can't initialize MySQL server");

  /* connect to server with no flags, default protocol, auto reconnect true */
  mysql = client_connect(0, MYSQL_PROTOCOL_DEFAULT, true);

  total_time = 0;
  for (iter_count = 1; iter_count <= opt_count; iter_count++) {
    /* Start of tests */
    test_count = 1;
    start_time = time((time_t *)nullptr);
    if (!tests_to_run) {
      for (fptr = my_testlist; fptr->name; fptr++) (*fptr->function)();
    } else {
      for (curr_test = tests_to_run; *curr_test; curr_test++) {
        for (fptr = my_testlist; fptr->name; fptr++) {
          if (!strcmp(fptr->name, *curr_test)) {
            (*fptr->function)();
            break;
          }
        }
        if (!fptr->name) {
          fprintf(stderr, "\n\nGiven test not found: '%s'\n", *argv);
          fprintf(stderr, "See legal test names with %s -T\n\nAborting!\n",
                  my_progname);
          client_disconnect(mysql);
          mysql_server_end();
          exit(1);
        }
      }
    }

    end_time = time((time_t *)nullptr);
    total_time += difftime(end_time, start_time);

    /* End of tests */
  }

  client_disconnect(mysql); /* disconnect from server */

  print_test_output();

  mysql_server_end();

  my_end(0);

  for (i = 0; i < original_argc; i++) free(original_argv[i]);
  if (original_argc) free(original_argv);
  if (tests_to_run) {
    for (curr_test = tests_to_run; *curr_test; curr_test++) free(*curr_test);
    free(tests_to_run);
  }
  my_free(opt_password);
  my_free(opt_host);
  return 0;
}
