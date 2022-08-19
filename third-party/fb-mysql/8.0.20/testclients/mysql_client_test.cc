/* Copyright (c) 2002, 2020, Oracle and/or its affiliates. All rights reserved.

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

/*
  XXX: PLEASE RUN THIS PROGRAM UNDER VALGRIND AND VERIFY THAT YOUR TEST
  DOESN'T CONTAIN WARNINGS/ERRORS BEFORE YOU PUSH.
*/

/*
  The fw.c file includes all the mysql_client_test framework; this file
  contains only the actual tests, plus the list of test functions to call.
*/

#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "my_byteorder.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_double2ulonglong.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "my_macros.h"
#include "my_time.h"  // SECS_PER_HOUR, SECS_PER_MIN
#include "mysql_client_fw.cc"

#include <list>
#include <string>
#include <vector>

static void mct_log(const char *format, ...)
    MY_ATTRIBUTE((format(printf, 1, 2)));

/* Query processing */

static void client_query() {
  int rc;

  myheader("client_query");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS t1");
  myquery(rc);

  rc = mysql_query(mysql,
                   "CREATE TABLE t1("
                   "id int primary key auto_increment, "
                   "name varchar(20))");
  myquery(rc);

  rc = mysql_query(mysql, "CREATE TABLE t1(id int, name varchar(20))");
  myquery_r(rc);

  rc = mysql_query(mysql, "INSERT INTO t1(name) VALUES('mysql')");
  myquery(rc);

  rc = mysql_query(mysql, "INSERT INTO t1(name) VALUES('monty')");
  myquery(rc);

  rc = mysql_query(mysql, "INSERT INTO t1(name) VALUES('venu')");
  myquery(rc);

  rc = mysql_query(mysql, "INSERT INTO t1(name) VALUES('deleted')");
  myquery(rc);

  rc = mysql_query(mysql, "INSERT INTO t1(name) VALUES('deleted')");
  myquery(rc);

  rc = mysql_query(mysql,
                   "UPDATE t1 SET name= 'updated' "
                   "WHERE name= 'deleted'");
  myquery(rc);

  rc = mysql_query(mysql, "UPDATE t1 SET id= 3 WHERE name= 'updated'");
  myquery_r(rc);

  myquery(mysql_query(mysql, "drop table t1"));
}

/* Store result processing */

static void client_store_result() {
  MYSQL_RES *result;
  int rc;

  myheader("client_store_result");

  rc = mysql_query(mysql, "SELECT * FROM t1");
  myquery(rc);

  /* get the result */
  result = mysql_store_result(mysql);
  mytest(result);

  (void)my_process_result_set(result);
  mysql_free_result(result);
}

/* Fetch the results */

static void client_use_result() {
  MYSQL_RES *result;
  int rc;
  myheader("client_use_result");

  rc = mysql_query(mysql, "SELECT * FROM t1");
  myquery(rc);

  /* get the result */
  result = mysql_use_result(mysql);
  mytest(result);

  (void)my_process_result_set(result);
  mysql_free_result(result);
}

/* Query processing */

static void test_debug_example() {
  int rc;
  MYSQL_RES *result;

  myheader("test_debug_example");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_debug_example");
  myquery(rc);

  rc = mysql_query(mysql,
                   "CREATE TABLE test_debug_example("
                   "id INT PRIMARY KEY AUTO_INCREMENT, "
                   "name VARCHAR(20), xxx INT)");
  myquery(rc);

  rc = mysql_query(mysql,
                   "INSERT INTO test_debug_example (name) "
                   "VALUES ('mysql')");
  myquery(rc);

  rc = mysql_query(mysql,
                   "UPDATE test_debug_example SET name='updated' "
                   "WHERE name='deleted'");
  myquery(rc);

  rc =
      mysql_query(mysql, "SELECT * FROM test_debug_example where name='mysql'");
  myquery(rc);

  result = mysql_use_result(mysql);
  mytest(result);

  (void)my_process_result_set(result);
  mysql_free_result(result);

  rc = mysql_query(mysql, "DROP TABLE test_debug_example");
  myquery(rc);
}

/* Test autocommit feature for BDB tables */

static void test_tran_bdb() {
  MYSQL_RES *result;
  MYSQL_ROW row;
  int rc;

  myheader("test_tran_bdb");

  /* set AUTOCOMMIT to OFF */
  rc = mysql_autocommit(mysql, false);
  myquery(rc);

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS my_demo_transaction");
  myquery(rc);

  /* create the table 'mytran_demo' of type BDB' or 'InnoDB' */
  rc = mysql_query(mysql,
                   "CREATE TABLE my_demo_transaction( "
                   "col1 int , col2 varchar(30)) ENGINE= BDB");
  myquery(rc);

  /* insert a row and commit the transaction */
  rc = mysql_query(mysql, "INSERT INTO my_demo_transaction VALUES(10, 'venu')");
  myquery(rc);

  rc = mysql_commit(mysql);
  myquery(rc);

  /* now insert the second row, and roll back the transaction */
  rc =
      mysql_query(mysql, "INSERT INTO my_demo_transaction VALUES(20, 'mysql')");
  myquery(rc);

  rc = mysql_rollback(mysql);
  myquery(rc);

  /* delete first row, and roll it back */
  rc = mysql_query(mysql, "DELETE FROM my_demo_transaction WHERE col1= 10");
  myquery(rc);

  rc = mysql_rollback(mysql);
  myquery(rc);

  /* test the results now, only one row should exist */
  rc = mysql_query(mysql, "SELECT * FROM my_demo_transaction");
  myquery(rc);

  /* get the result */
  result = mysql_store_result(mysql);
  mytest(result);

  (void)my_process_result_set(result);
  mysql_free_result(result);

  /* test the results now, only one row should exist */
  rc = mysql_query(mysql, "SELECT * FROM my_demo_transaction");
  myquery(rc);

  /* get the result */
  result = mysql_use_result(mysql);
  mytest(result);

  row = mysql_fetch_row(result);
  mytest(row);

  row = mysql_fetch_row(result);
  mytest_r(row);

  mysql_free_result(result);
  mysql_autocommit(mysql, true);
}

/* Test autocommit feature for InnoDB tables */

static void test_tran_innodb() {
  MYSQL_RES *result;
  MYSQL_ROW row;
  int rc;

  myheader("test_tran_innodb");

  /* set AUTOCOMMIT to OFF */
  rc = mysql_autocommit(mysql, false);
  myquery(rc);

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS my_demo_transaction");
  myquery(rc);

  /* create the table 'mytran_demo' of type BDB' or 'InnoDB' */
  rc = mysql_query(mysql,
                   "CREATE TABLE my_demo_transaction(col1 int, "
                   "col2 varchar(30)) ENGINE= InnoDB");
  myquery(rc);

  /* insert a row and commit the transaction */
  rc = mysql_query(mysql, "INSERT INTO my_demo_transaction VALUES(10, 'venu')");
  myquery(rc);

  rc = mysql_commit(mysql);
  myquery(rc);

  /* now insert the second row, and roll back the transaction */
  rc =
      mysql_query(mysql, "INSERT INTO my_demo_transaction VALUES(20, 'mysql')");
  myquery(rc);

  rc = mysql_rollback(mysql);
  myquery(rc);

  /* delete first row, and roll it back */
  rc = mysql_query(mysql, "DELETE FROM my_demo_transaction WHERE col1= 10");
  myquery(rc);

  rc = mysql_rollback(mysql);
  myquery(rc);

  /* test the results now, only one row should exist */
  rc = mysql_query(mysql, "SELECT * FROM my_demo_transaction");
  myquery(rc);

  /* get the result */
  result = mysql_store_result(mysql);
  mytest(result);

  (void)my_process_result_set(result);
  mysql_free_result(result);

  /* test the results now, only one row should exist */
  rc = mysql_query(mysql, "SELECT * FROM my_demo_transaction");
  myquery(rc);

  /* get the result */
  result = mysql_use_result(mysql);
  mytest(result);

  row = mysql_fetch_row(result);
  mytest(row);

  row = mysql_fetch_row(result);
  mytest_r(row);

  mysql_free_result(result);
  mysql_autocommit(mysql, true);
}

/* Test for BUG#7242 */

static void test_prepare_insert_update() {
  MYSQL_STMT *stmt;
  int rc;
  int i;
  const char *testcase[] = {
      "CREATE TABLE t1 (a INT, b INT, c INT, UNIQUE (A), UNIQUE(B))",
      "INSERT t1 VALUES (1,2,10), (3,4,20)",
      "INSERT t1 VALUES (5,6,30), (7,4,40), (8,9,60) ON DUPLICATE KEY UPDATE "
      "c=c+100",
      "SELECT * FROM t1",
      "INSERT t1 SET a=5 ON DUPLICATE KEY UPDATE b=0",
      "SELECT * FROM t1",
      "INSERT t1 VALUES (2,1,11), (7,4,40) ON DUPLICATE KEY UPDATE "
      "c=c+VALUES(a)",
      nullptr};
  const char **cur_query;

  myheader("test_prepare_insert_update");

  for (cur_query = testcase; *cur_query; cur_query++) {
    char query[MAX_TEST_QUERY_LENGTH];
    printf("\nRunning query: %s", *cur_query);
    my_stpcpy(query, *cur_query);
    stmt = mysql_simple_prepare(mysql, query);
    check_stmt(stmt);

    verify_param_count(stmt, 0);
    rc = mysql_stmt_execute(stmt);

    check_execute(stmt, rc);
    /* try the last query several times */
    if (!cur_query[1]) {
      for (i = 0; i < 3; i++) {
        printf("\nExecuting last statement again");
        rc = mysql_stmt_execute(stmt);
        check_execute(stmt, rc);
        rc = mysql_stmt_execute(stmt);
        check_execute(stmt, rc);
      }
    }
    mysql_stmt_close(stmt);
  }

  rc = mysql_commit(mysql);
  myquery(rc);
}

/* Test simple prepares of all DML statements */

static void test_prepare_simple() {
  MYSQL_STMT *stmt;
  int rc;
  char query[MAX_TEST_QUERY_LENGTH];

  myheader("test_prepare_simple");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_prepare_simple");
  myquery(rc);

  rc = mysql_query(mysql,
                   "CREATE TABLE test_prepare_simple("
                   "id int, name varchar(50))");
  myquery(rc);

  /* insert */
  my_stpcpy(query, "INSERT INTO test_prepare_simple VALUES(?, ?)");
  stmt = mysql_simple_prepare(mysql, query);
  check_stmt(stmt);

  verify_param_count(stmt, 2);
  mysql_stmt_close(stmt);

  /* update */
  my_stpcpy(query,
            "UPDATE test_prepare_simple SET id=? "
            "WHERE id=? AND CONVERT(name USING utf8)= ?");
  stmt = mysql_simple_prepare(mysql, query);
  check_stmt(stmt);

  verify_param_count(stmt, 3);
  mysql_stmt_close(stmt);

  /* delete */
  my_stpcpy(query, "DELETE FROM test_prepare_simple WHERE id=10");
  stmt = mysql_simple_prepare(mysql, query);
  check_stmt(stmt);

  verify_param_count(stmt, 0);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);
  mysql_stmt_close(stmt);

  /* delete */
  my_stpcpy(query, "DELETE FROM test_prepare_simple WHERE id=?");
  stmt = mysql_simple_prepare(mysql, query);
  check_stmt(stmt);

  verify_param_count(stmt, 1);

  mysql_stmt_close(stmt);

  /* select */
  my_stpcpy(query,
            "SELECT * FROM test_prepare_simple WHERE id=? "
            "AND CONVERT(name USING utf8)= ?");
  stmt = mysql_simple_prepare(mysql, query);
  check_stmt(stmt);

  verify_param_count(stmt, 2);

  mysql_stmt_close(stmt);

  /* now fetch the results ..*/
  rc = mysql_commit(mysql);
  myquery(rc);
}

/************************************************************************/

#define FILE_PATH_SIZE 4096

char mct_log_file_path[FILE_PATH_SIZE];
FILE *mct_log_file = nullptr;

static void mct_start_logging(const char *test_case_name) {
  const char *tmp_dir = getenv("MYSQL_TMP_DIR");

  if (!tmp_dir) {
    printf("Warning: MYSQL_TMP_DIR is not set. Logging is disabled.\n");
    return;
  }

  if (mct_log_file) {
    printf(
        "Warning: can not start logging for test case '%s' "
        "because log is already open\n",
        (const char *)test_case_name);
    return;
  }

  /*
    Path is: <tmp_dir>/<test_case_name>.out.log
    10 is length of '/' + '.out.log' + \0
  */

  if (strlen(tmp_dir) + strlen(test_case_name) + 10 > FILE_PATH_SIZE) {
    printf("Warning: MYSQL_TMP_DIR is too long. Logging is disabled.\n");
    return;
  }

  snprintf(mct_log_file_path, FILE_PATH_SIZE, "%s/%s.out.log",
           (const char *)tmp_dir, (const char *)test_case_name);

  mct_log_file =
      my_fopen(mct_log_file_path, O_WRONLY | MY_FOPEN_BINARY, MYF(MY_WME));

  if (!mct_log_file) {
    printf("Warning: can not open log file (%s): %s. Logging is disabled.\n",
           (const char *)mct_log_file_path, (const char *)strerror(errno));
    return;
  }
}

static void mct_log(const char *format, ...) {
  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);

  if (mct_log_file) {
    va_list mct_args;
    va_start(mct_args, format);
    vfprintf(mct_log_file, format, mct_args);
    va_end(mct_args);
  }
}

static void mct_close_log() {
  if (!mct_log_file) return;

  my_fclose(mct_log_file, MYF(0));
  mct_log_file = nullptr;
}

#define WL4435_NUM_PARAMS 10
#define WL4435_STRING_SIZE 30

static void test_wl4435() {
  MYSQL_STMT *stmt;
  int query_rc;
  char query[MAX_TEST_QUERY_LENGTH];

  char str_data[20][WL4435_STRING_SIZE];
  double dbl_data[20];
  char dec_data[20][WL4435_STRING_SIZE];
  int int_data[20];
  ulong str_length = WL4435_STRING_SIZE;
  bool is_null;
  MYSQL_BIND ps_params[WL4435_NUM_PARAMS];

  int exec_counter;

  memset(str_data, 0, sizeof(str_data));
  memset(dbl_data, 0, sizeof(dbl_data));
  memset(dec_data, 0, sizeof(dec_data));
  memset(int_data, 0, sizeof(int_data));

  myheader("test_wl4435");
  mct_start_logging("test_wl4435");

  query_rc = mysql_query(mysql, "DROP PROCEDURE IF EXISTS p1");
  myquery(query_rc);

  query_rc = mysql_query(mysql, "DROP PROCEDURE IF EXISTS p2");
  myquery(query_rc);

  query_rc = mysql_query(mysql, "DROP TABLE IF EXISTS t1");
  myquery(query_rc);

  query_rc = mysql_query(mysql, "DROP TABLE IF EXISTS t2");
  myquery(query_rc);

  query_rc = mysql_query(mysql,
                         "CREATE TABLE t1(a1 INT, a2 CHAR(32), "
                         "  a3 DOUBLE(4, 2), a4 DECIMAL(3, 1))");
  myquery(query_rc);

  query_rc = mysql_query(mysql,
                         "CREATE TABLE t2(b0 INT, b1 INT, b2 CHAR(32), "
                         "  b3 DOUBLE(4, 2), b4 DECIMAL(3, 1))");
  myquery(query_rc);

  query_rc = mysql_query(mysql,
                         "INSERT INTO t1 VALUES"
                         "(1, '11', 12.34, 56.7), "
                         "(2, '12', 56.78, 90.1), "
                         "(3, '13', 23.45, 67.8)");
  myquery(query_rc);

  query_rc = mysql_query(mysql,
                         "INSERT INTO t2 VALUES"
                         "(100, 10, '110', 70.70, 10.1), "
                         "(200, 20, '120', 80.80, 20.2), "
                         "(300, 30, '130', 90.90, 30.3)");
  myquery(query_rc);

  query_rc = mysql_query(mysql,
                         "CREATE PROCEDURE p1("
                         "   IN v0 INT, "
                         "   OUT v_str_1 CHAR(32), "
                         "   OUT v_dbl_1 DOUBLE(4, 2), "
                         "   OUT v_dec_1 DECIMAL(6, 3), "
                         "   OUT v_int_1 INT, "
                         "   IN v1 INT, "
                         "   INOUT v_str_2 CHAR(64), "
                         "   INOUT v_dbl_2 DOUBLE(5, 3), "
                         "   INOUT v_dec_2 DECIMAL(7, 4), "
                         "   INOUT v_int_2 INT)"
                         "BEGIN "
                         "   SET v0 = -1; "
                         "   SET v1 = -1; "
                         "   SET v_str_1 = 'test_1'; "
                         "   SET v_dbl_1 = 12.34; "
                         "   SET v_dec_1 = 567.891; "
                         "   SET v_int_1 = 2345; "
                         "   SET v_str_2 = 'test_2'; "
                         "   SET v_dbl_2 = 67.891; "
                         "   SET v_dec_2 = 234.6789; "
                         "   SET v_int_2 = 6789; "
                         "   SELECT * FROM t1; "
                         "   SELECT * FROM t2; "
                         "END");
  myquery(query_rc);

  query_rc = mysql_query(mysql,
                         "CREATE PROCEDURE p2("
                         "   IN i1 VARCHAR(255) CHARACTER SET koi8r, "
                         "   OUT o1 VARCHAR(255) CHARACTER SET cp1251, "
                         "   OUT o2 VARBINARY(255)) "
                         "BEGIN "
                         "   SET o1 = i1; "
                         "   SET o2 = i1; "
                         "END");
  myquery(query_rc);

  my_stpcpy(query, "CALL p1(?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
  stmt = mysql_simple_prepare(mysql, query);
  check_stmt(stmt);

  /* Init PS-parameters. */

  memset(ps_params, 0, sizeof(ps_params));

  /* - v0 -- INT */

  ps_params[0].buffer_type = MYSQL_TYPE_LONG;
  ps_params[0].buffer = (char *)&int_data[0];
  ps_params[0].length = nullptr;
  ps_params[0].is_null = nullptr;

  /* - v_str_1 -- CHAR(32) */

  ps_params[1].buffer_type = MYSQL_TYPE_STRING;
  ps_params[1].buffer = (char *)str_data[0];
  ps_params[1].buffer_length = WL4435_STRING_SIZE;
  ps_params[1].length = &str_length;
  ps_params[1].is_null = nullptr;

  /* - v_dbl_1 -- DOUBLE */

  ps_params[2].buffer_type = MYSQL_TYPE_DOUBLE;
  ps_params[2].buffer = (char *)&dbl_data[0];
  ps_params[2].length = nullptr;
  ps_params[2].is_null = nullptr;

  /* - v_dec_1 -- DECIMAL */

  ps_params[3].buffer_type = MYSQL_TYPE_NEWDECIMAL;
  ps_params[3].buffer = (char *)dec_data[0];
  ps_params[3].buffer_length = WL4435_STRING_SIZE;
  ps_params[3].length = nullptr;
  ps_params[3].is_null = nullptr;

  /* - v_int_1 -- INT */

  ps_params[4].buffer_type = MYSQL_TYPE_LONG;
  ps_params[4].buffer = (char *)&int_data[0];
  ps_params[4].length = nullptr;
  ps_params[4].is_null = nullptr;

  /* - v1 -- INT */

  ps_params[5].buffer_type = MYSQL_TYPE_LONG;
  ps_params[5].buffer = (char *)&int_data[0];
  ps_params[5].length = nullptr;
  ps_params[5].is_null = nullptr;

  /* - v_str_2 -- CHAR(32) */

  ps_params[6].buffer_type = MYSQL_TYPE_STRING;
  ps_params[6].buffer = (char *)str_data[0];
  ps_params[6].buffer_length = WL4435_STRING_SIZE;
  ps_params[6].length = &str_length;
  ps_params[6].is_null = nullptr;

  /* - v_dbl_2 -- DOUBLE */

  ps_params[7].buffer_type = MYSQL_TYPE_DOUBLE;
  ps_params[7].buffer = (char *)&dbl_data[0];
  ps_params[7].length = nullptr;
  ps_params[7].is_null = nullptr;

  /* - v_dec_2 -- DECIMAL */

  ps_params[8].buffer_type = MYSQL_TYPE_DECIMAL;
  ps_params[8].buffer = (char *)dec_data[0];
  ps_params[8].buffer_length = WL4435_STRING_SIZE;
  ps_params[8].length = nullptr;
  ps_params[8].is_null = nullptr;

  /* - v_int_2 -- INT */

  ps_params[9].buffer_type = MYSQL_TYPE_LONG;
  ps_params[9].buffer = (char *)&int_data[0];
  ps_params[9].length = nullptr;
  ps_params[9].is_null = nullptr;

  /* Bind parameters. */

  query_rc = mysql_stmt_bind_param(stmt, ps_params);
  check_execute(stmt, query_rc);

  /* Execute! */

  for (exec_counter = 0; exec_counter < 3; ++exec_counter) {
    int i;
    int num_fields;
    MYSQL_BIND *rs_bind;

    mct_log("\nexec_counter: %d\n", (int)exec_counter);

    query_rc = mysql_stmt_execute(stmt);
    check_execute(stmt, query_rc);

    while (true) {
      MYSQL_FIELD *fields;

      MYSQL_RES *rs_metadata = mysql_stmt_result_metadata(stmt);

      num_fields = mysql_stmt_field_count(stmt);
      fields = mysql_fetch_fields(rs_metadata);

      rs_bind = (MYSQL_BIND *)malloc(sizeof(MYSQL_BIND) * num_fields);
      memset(rs_bind, 0, sizeof(MYSQL_BIND) * num_fields);

      mct_log("num_fields: %d\n", (int)num_fields);

      for (i = 0; i < num_fields; ++i) {
        mct_log(
            "  - %d: name: '%s'/'%s'; table: '%s'/'%s'; "
            "db: '%s'; catalog: '%s'; length: %d; max_length: %d; "
            "type: %d; decimals: %d\n",
            (int)i, (const char *)fields[i].name,
            (const char *)fields[i].org_name, (const char *)fields[i].table,
            (const char *)fields[i].org_table, (const char *)fields[i].db,
            (const char *)fields[i].catalog, (int)fields[i].length,
            (int)fields[i].max_length, (int)fields[i].type,
            (int)fields[i].decimals);

        rs_bind[i].buffer_type = fields[i].type;
        rs_bind[i].is_null = &is_null;

        switch (fields[i].type) {
          case MYSQL_TYPE_LONG:
            rs_bind[i].buffer = (char *)&(int_data[i]);
            rs_bind[i].buffer_length = (ulong)sizeof(int_data);
            break;

          case MYSQL_TYPE_STRING:
            rs_bind[i].buffer = (char *)str_data[i];
            rs_bind[i].buffer_length = WL4435_STRING_SIZE;
            rs_bind[i].length = &str_length;
            break;

          case MYSQL_TYPE_DOUBLE:
            rs_bind[i].buffer = (char *)&dbl_data[i];
            rs_bind[i].buffer_length = (ulong)sizeof(dbl_data);
            break;

          case MYSQL_TYPE_NEWDECIMAL:
            rs_bind[i].buffer = (char *)dec_data[i];
            rs_bind[i].buffer_length = WL4435_STRING_SIZE;
            rs_bind[i].length = &str_length;
            break;

          default:
            fprintf(stderr, "ERROR: unexpected type: %d.\n", fields[i].type);
            exit(1);
        }
      }

      query_rc = mysql_stmt_bind_result(stmt, rs_bind);
      check_execute(stmt, query_rc);

      mct_log("Data:\n");

      while (true) {
        int rc = mysql_stmt_fetch(stmt);

        if (rc == 1 || rc == MYSQL_NO_DATA) break;

        mct_log(" ");

        for (i = 0; i < num_fields; ++i) {
          switch (rs_bind[i].buffer_type) {
            case MYSQL_TYPE_LONG:
              mct_log(" int: %ld;", (long)*((int *)rs_bind[i].buffer));
              break;

            case MYSQL_TYPE_STRING:
              mct_log(" str: '%s';", (char *)rs_bind[i].buffer);
              break;

            case MYSQL_TYPE_DOUBLE:
              mct_log(" dbl: %lf;", (double)*((double *)rs_bind[i].buffer));
              break;

            case MYSQL_TYPE_NEWDECIMAL:
              mct_log(" dec: '%s';", (char *)rs_bind[i].buffer);
              break;

            default:
              printf("  unexpected type (%d)\n", rs_bind[i].buffer_type);
          }
        }
        mct_log("\n");
      }

      mct_log("EOF\n");

      query_rc = mysql_stmt_next_result(stmt);
      mct_log("mysql_stmt_next_result(): %d; field_count: %d\n", query_rc,
              (int)mysql->field_count);

      free(rs_bind);
      mysql_free_result(rs_metadata);

      if (query_rc > 0) {
        printf("Error: %s (errno: %d)\n", mysql_stmt_error(stmt),
               mysql_stmt_errno(stmt));
        DIE(query_rc > 0);
      }

      if (query_rc) break;

      if (!mysql->field_count) {
        /* This is the last OK-packet. No more resultsets. */
        break;
      }
    }
  }

  mysql_stmt_close(stmt);

  mct_close_log();

  query_rc = mysql_commit(mysql);
  myquery(query_rc);

  /* i18n part of test case. */

  {
    char str_koi8r[] =
        "\xee\xd5\x2c\x20\xda\xc1\x20\xd2\xd9\xc2\xc1\xcc\xcb\xd5";
    const char *str_cp1251 =
        "\xcd\xf3\x2c\x20\xe7\xe0\x20\xf0\xfb\xe1\xe0\xeb\xea\xf3";
    char o1_buffer[255];
    ulong o1_length;
    char o2_buffer[255];
    ulong o2_length;

    MYSQL_BIND rs_bind[2];

    my_stpcpy(query, "CALL p2(?, ?, ?)");
    stmt = mysql_simple_prepare(mysql, query);
    check_stmt(stmt);

    /* Init PS-parameters. */

    memset(ps_params, 0, sizeof(ps_params));

    ps_params[0].buffer_type = MYSQL_TYPE_STRING;
    ps_params[0].buffer = str_koi8r;
    ps_params[0].buffer_length = (ulong)strlen(str_koi8r);

    ps_params[1].buffer_type = MYSQL_TYPE_STRING;
    ps_params[1].buffer = o1_buffer;
    ps_params[1].buffer_length = 0;

    ps_params[2].buffer_type = MYSQL_TYPE_STRING;
    ps_params[2].buffer = o2_buffer;
    ps_params[2].buffer_length = 0;

    /* Bind parameters. */

    query_rc = mysql_stmt_bind_param(stmt, ps_params);
    check_execute(stmt, query_rc);

    /* Prevent converting to character_set_results. */

    query_rc = mysql_query(mysql, "SET NAMES binary");
    myquery(query_rc);

    /* Execute statement. */

    query_rc = mysql_stmt_execute(stmt);
    check_execute(stmt, query_rc);

    /* Bind result. */

    memset(rs_bind, 0, sizeof(rs_bind));

    rs_bind[0].buffer_type = MYSQL_TYPE_STRING;
    rs_bind[0].buffer = o1_buffer;
    rs_bind[0].buffer_length = (ulong)sizeof(o1_buffer);
    rs_bind[0].length = &o1_length;

    rs_bind[1].buffer_type = MYSQL_TYPE_BLOB;
    rs_bind[1].buffer = o2_buffer;
    rs_bind[1].buffer_length = (ulong)sizeof(o2_buffer);
    rs_bind[1].length = &o2_length;

    query_rc = mysql_stmt_bind_result(stmt, rs_bind);
    check_execute(stmt, query_rc);

    /* Fetch result. */

    query_rc = mysql_stmt_fetch(stmt);
    check_execute(stmt, query_rc);

    /* Check result. */

    DIE_UNLESS(o1_length == strlen(str_cp1251));
    DIE_UNLESS(o2_length == strlen(str_koi8r));
    DIE_UNLESS(!memcmp(o1_buffer, str_cp1251, o1_length));
    DIE_UNLESS(!memcmp(o2_buffer, str_koi8r, o2_length));

    query_rc = mysql_stmt_fetch(stmt);
    DIE_UNLESS(query_rc == MYSQL_NO_DATA);

    query_rc = mysql_stmt_next_result(stmt);
    DIE_UNLESS(query_rc == 0 && mysql->field_count == 0);

    mysql_stmt_close(stmt);

    query_rc = mysql_commit(mysql);
    myquery(query_rc);
  }
}

static void test_wl4435_2() {
  MYSQL_STMT *stmt;
  int i;
  int rc;
  char query[MAX_TEST_QUERY_LENGTH];

  myheader("test_wl4435_2");
  mct_start_logging("test_wl4435_2");

  /*
    Do a few iterations so that we catch any problem with incorrect
    handling/flushing prepared statement results.
  */

  for (i = 0; i < 10; ++i) {
    /*
      Prepare a procedure. That can be moved out of the loop, but it was
      left in the loop for the sake of having as many statements as
      possible.
    */

    rc = mysql_query(mysql, "DROP PROCEDURE IF EXISTS p1");
    myquery(rc);

    rc = mysql_query(mysql,
                     "CREATE PROCEDURE p1()"
                     "BEGIN "
                     "  SELECT 1; "
                     "  SELECT 2, 3 UNION SELECT 4, 5; "
                     "  SELECT 6, 7, 8; "
                     "END");
    myquery(rc);

    /* Invoke a procedure, that returns several result sets. */

    my_stpcpy(query, "CALL p1()");
    stmt = mysql_simple_prepare(mysql, query);
    check_stmt(stmt);

    /* Execute! */

    rc = mysql_stmt_execute(stmt);
    check_execute(stmt, rc);

    /* Flush all the results. */

    mysql_stmt_close(stmt);

    /* Clean up. */
    rc = mysql_commit(mysql);
    myquery(rc);

    rc = mysql_query(mysql, "DROP PROCEDURE p1");
    myquery(rc);
  }
  mct_close_log();
}

#define WL4435_TEST(sql_type, sql_value, c_api_in_type, c_api_out_type, \
                    c_type, c_type_ext, printf_args, assert_condition)  \
                                                                        \
  do {                                                                  \
    int rc;                                                             \
    MYSQL_STMT *ps;                                                     \
    MYSQL_BIND psp;                                                     \
    MYSQL_RES *rs_metadata;                                             \
    MYSQL_FIELD *fields;                                                \
    c_type pspv c_type_ext;                                             \
    bool psp_null = false;                                              \
                                                                        \
    memset(&pspv, 0, sizeof(pspv));                                     \
                                                                        \
    rc = mysql_query(mysql, "DROP PROCEDURE IF EXISTS p1");             \
    myquery(rc);                                                        \
                                                                        \
    rc = mysql_query(mysql, "CREATE PROCEDURE p1(OUT v " sql_type       \
                            ") SET v = " sql_value ";");                \
    myquery(rc);                                                        \
                                                                        \
    ps = mysql_simple_prepare(mysql, "CALL p1(?)");                     \
    check_stmt(ps);                                                     \
    memset(&psp, 0, sizeof(psp));                                       \
    psp.buffer_type = c_api_in_type;                                    \
    psp.is_null = &psp_null;                                            \
    psp.buffer = (char *)&pspv;                                         \
    psp.buffer_length = (ulong)sizeof(psp);                             \
                                                                        \
    rc = mysql_stmt_bind_param(ps, &psp);                               \
    check_execute(ps, rc);                                              \
                                                                        \
    rc = mysql_stmt_execute(ps);                                        \
    check_execute(ps, rc);                                              \
                                                                        \
    if (!(mysql->server_capabilities & CLIENT_DEPRECATE_EOF))           \
      DIE_UNLESS(mysql->server_status &SERVER_PS_OUT_PARAMS);           \
    DIE_UNLESS(mysql_stmt_field_count(ps) == 1);                        \
                                                                        \
    rs_metadata = mysql_stmt_result_metadata(ps);                       \
    fields = mysql_fetch_fields(rs_metadata);                           \
                                                                        \
    rc = mysql_stmt_bind_result(ps, &psp);                              \
    check_execute(ps, rc);                                              \
                                                                        \
    rc = mysql_stmt_fetch(ps);                                          \
    DIE_UNLESS(rc == 0);                                                \
                                                                        \
    DIE_UNLESS(fields[0].type == c_api_out_type);                       \
    printf printf_args;                                                 \
    printf("; in type: %d; out type: %d\n", (int)c_api_in_type,         \
           (int)c_api_out_type);                                        \
                                                                        \
    rc = mysql_stmt_fetch(ps);                                          \
    DIE_UNLESS(rc == MYSQL_NO_DATA);                                    \
                                                                        \
    rc = mysql_stmt_next_result(ps);                                    \
    DIE_UNLESS(rc == 0);                                                \
                                                                        \
    mysql_free_result(rs_metadata);                                     \
    mysql_stmt_free_result(ps);                                         \
    mysql_stmt_close(ps);                                               \
                                                                        \
    DIE_UNLESS(assert_condition);                                       \
                                                                        \
  } while (0)

static void test_wl4435_3() {
  char tmp[255];

  puts("");

  /*
    The following types are not supported:
     - ENUM
     - SET

    The following types are supported but can not be used for
    OUT-parameters:
     - MEDIUMINT;
     - BIT(..);

    The problem is that those types are not supported for IN-parameters,
    and OUT-parameters should be bound as IN-parameters before execution

    The following types should not be used:
     - MYSQL_TYPE_YEAR (use MYSQL_TYPE_SHORT instead);
     - MYSQL_TYPE_TINY_BLOB, MYSQL_TYPE_MEDIUM_BLOB, MYSQL_TYPE_LONG_BLOB
       (use MYSQL_TYPE_BLOB instead);
  */

  WL4435_TEST("TINYINT", "127", MYSQL_TYPE_TINY, MYSQL_TYPE_TINY, char, ,
              ("  - TINYINT / char / MYSQL_TYPE_TINY:\t\t\t %d", (int)pspv),
              pspv == 127);

  WL4435_TEST("SMALLINT", "32767", MYSQL_TYPE_SHORT, MYSQL_TYPE_SHORT, short, ,
              ("  - SMALLINT / short / MYSQL_TYPE_SHORT:\t\t %d", (int)pspv),
              pspv == 32767);

  WL4435_TEST("INT", "2147483647", MYSQL_TYPE_LONG, MYSQL_TYPE_LONG, int, ,
              ("  - INT / int / MYSQL_TYPE_LONG:\t\t\t %d", pspv),
              pspv == 2147483647l);

  WL4435_TEST("BIGINT", "9223372036854775807", MYSQL_TYPE_LONGLONG,
              MYSQL_TYPE_LONGLONG, long long, ,
              ("  - BIGINT / long long / MYSQL_TYPE_LONGLONG:\t\t %lld", pspv),
              pspv == 9223372036854775807ll);

  WL4435_TEST("TIMESTAMP", "'2007-11-18 15:01:02'", MYSQL_TYPE_TIMESTAMP,
              MYSQL_TYPE_TIMESTAMP, MYSQL_TIME, ,
              ("  - TIMESTAMP / MYSQL_TIME / MYSQL_TYPE_TIMESTAMP:\t "
               "%.4d-%.2d-%.2d %.2d:%.2d:%.2d",
               (int)pspv.year, (int)pspv.month, (int)pspv.day, (int)pspv.hour,
               (int)pspv.minute, (int)pspv.second),
              pspv.year == 2007 && pspv.month == 11 && pspv.day == 18 &&
                  pspv.hour == 15 && pspv.minute == 1 && pspv.second == 2);

  WL4435_TEST("DATETIME", "'1234-11-12 12:34:59'", MYSQL_TYPE_DATETIME,
              MYSQL_TYPE_DATETIME, MYSQL_TIME, ,
              ("  - DATETIME / MYSQL_TIME / MYSQL_TYPE_DATETIME:\t "
               "%.4d-%.2d-%.2d %.2d:%.2d:%.2d",
               (int)pspv.year, (int)pspv.month, (int)pspv.day, (int)pspv.hour,
               (int)pspv.minute, (int)pspv.second),
              pspv.year == 1234 && pspv.month == 11 && pspv.day == 12 &&
                  pspv.hour == 12 && pspv.minute == 34 && pspv.second == 59);

  WL4435_TEST("TIME", "'123:45:01'", MYSQL_TYPE_TIME, MYSQL_TYPE_TIME,
              MYSQL_TIME, ,
              ("  - TIME / MYSQL_TIME / MYSQL_TYPE_TIME:\t\t "
               "%.3d:%.2d:%.2d",
               (int)pspv.hour, (int)pspv.minute, (int)pspv.second),
              pspv.hour == 123 && pspv.minute == 45 && pspv.second == 1);

  WL4435_TEST("DATE", "'1234-11-12'", MYSQL_TYPE_DATE, MYSQL_TYPE_DATE,
              MYSQL_TIME, ,
              ("  - DATE / MYSQL_TIME / MYSQL_TYPE_DATE:\t\t "
               "%.4d-%.2d-%.2d",
               (int)pspv.year, (int)pspv.month, (int)pspv.day),
              pspv.year == 1234 && pspv.month == 11 && pspv.day == 12);

  WL4435_TEST("YEAR", "'2010'", MYSQL_TYPE_SHORT, MYSQL_TYPE_YEAR, short, ,
              ("  - YEAR / short / MYSQL_TYPE_SHORT:\t\t\t %.4d", (int)pspv),
              pspv == 2010);

  WL4435_TEST("FLOAT(7, 4)", "123.4567", MYSQL_TYPE_FLOAT, MYSQL_TYPE_FLOAT,
              float, ,
              ("  - FLOAT / float / MYSQL_TYPE_FLOAT:\t\t\t %g", (double)pspv),
              pspv - 123.4567 < 0.0001);

  WL4435_TEST("DOUBLE(8, 5)", "123.45678", MYSQL_TYPE_DOUBLE, MYSQL_TYPE_DOUBLE,
              double, ,
              ("  - DOUBLE / double / MYSQL_TYPE_DOUBLE:\t\t %g", (double)pspv),
              pspv - 123.45678 < 0.00001);

  WL4435_TEST(
      "DECIMAL(9, 6)", "123.456789", MYSQL_TYPE_NEWDECIMAL,
      MYSQL_TYPE_NEWDECIMAL, char, [255],
      ("  - DECIMAL / char[] / MYSQL_TYPE_NEWDECIMAL:\t\t '%s'", (char *)pspv),
      !strcmp(pspv, "123.456789"));

  WL4435_TEST(
      "CHAR(32)", "REPEAT('C', 16)", MYSQL_TYPE_STRING, MYSQL_TYPE_STRING,
      char, [255],
      ("  - CHAR(32) / char[] / MYSQL_TYPE_STRING:\t\t '%s'", (char *)pspv),
      !strcmp(pspv, "CCCCCCCCCCCCCCCC"));

  WL4435_TEST("VARCHAR(32)", "REPEAT('V', 16)", MYSQL_TYPE_VAR_STRING,
              MYSQL_TYPE_VAR_STRING, char, [255],
              ("  - VARCHAR(32) / char[] / MYSQL_TYPE_VAR_STRING:\t '%s'",
               (char *)pspv),
              !strcmp(pspv, "VVVVVVVVVVVVVVVV"));

  WL4435_TEST(
      "TINYTEXT", "REPEAT('t', 16)", MYSQL_TYPE_TINY_BLOB, MYSQL_TYPE_BLOB,
      char, [255],
      ("  - TINYTEXT / char[] / MYSQL_TYPE_TINY_BLOB:\t\t '%s'", (char *)pspv),
      !strcmp(pspv, "tttttttttttttttt"));

  WL4435_TEST("TEXT", "REPEAT('t', 16)", MYSQL_TYPE_BLOB, MYSQL_TYPE_BLOB,
              char, [255],
              ("  - TEXT / char[] / MYSQL_TYPE_BLOB:\t\t\t '%s'", (char *)pspv),
              !strcmp(pspv, "tttttttttttttttt"));

  WL4435_TEST("MEDIUMTEXT", "REPEAT('t', 16)", MYSQL_TYPE_MEDIUM_BLOB,
              MYSQL_TYPE_BLOB, char, [255],
              ("  - MEDIUMTEXT / char[] / MYSQL_TYPE_MEDIUM_BLOB:\t '%s'",
               (char *)pspv),
              !strcmp(pspv, "tttttttttttttttt"));

  WL4435_TEST(
      "LONGTEXT", "REPEAT('t', 16)", MYSQL_TYPE_LONG_BLOB, MYSQL_TYPE_BLOB,
      char, [255],
      ("  - LONGTEXT / char[] / MYSQL_TYPE_LONG_BLOB:\t\t '%s'", (char *)pspv),
      !strcmp(pspv, "tttttttttttttttt"));

  WL4435_TEST(
      "BINARY(32)", "REPEAT('\1', 16)", MYSQL_TYPE_STRING, MYSQL_TYPE_STRING,
      char, [255],
      ("  - BINARY(32) / char[] / MYSQL_TYPE_STRING:\t\t '%s'", (char *)pspv),
      memset(tmp, 1, 16) && !memcmp(tmp, pspv, 16));

  WL4435_TEST("VARBINARY(32)", "REPEAT('\1', 16)", MYSQL_TYPE_VAR_STRING,
              MYSQL_TYPE_VAR_STRING, char, [255],
              ("  - VARBINARY(32) / char[] / MYSQL_TYPE_VAR_STRING:\t '%s'",
               (char *)pspv),
              memset(tmp, 1, 16) && !memcmp(tmp, pspv, 16));

  WL4435_TEST(
      "TINYBLOB", "REPEAT('\2', 16)", MYSQL_TYPE_TINY_BLOB, MYSQL_TYPE_BLOB,
      char, [255],
      ("  - TINYBLOB / char[] / MYSQL_TYPE_TINY_BLOB:\t\t '%s'", (char *)pspv),
      memset(tmp, 2, 16) && !memcmp(tmp, pspv, 16));

  WL4435_TEST("BLOB", "REPEAT('\2', 16)", MYSQL_TYPE_BLOB, MYSQL_TYPE_BLOB,
              char, [255],
              ("  - BLOB / char[] / MYSQL_TYPE_BLOB:\t\t\t '%s'", (char *)pspv),
              memset(tmp, 2, 16) && !memcmp(tmp, pspv, 16));

  WL4435_TEST("MEDIUMBLOB", "REPEAT('\2', 16)", MYSQL_TYPE_MEDIUM_BLOB,
              MYSQL_TYPE_BLOB, char, [255],
              ("  - MEDIUMBLOB / char[] / MYSQL_TYPE_MEDIUM_BLOB:\t '%s'",
               (char *)pspv),
              memset(tmp, 2, 16) && !memcmp(tmp, pspv, 16));

  WL4435_TEST(
      "LONGBLOB", "REPEAT('\2', 16)", MYSQL_TYPE_LONG_BLOB, MYSQL_TYPE_BLOB,
      char, [255],
      ("  - LONGBLOB / char[] / MYSQL_TYPE_LONG_BLOB:\t\t '%s'", (char *)pspv),
      memset(tmp, 2, 16) && !memcmp(tmp, pspv, 16));
}

/* Test simple prepare field results */

static void test_prepare_field_result() {
  MYSQL_STMT *stmt;
  MYSQL_RES *result;
  int rc;
  char query[MAX_TEST_QUERY_LENGTH];

  myheader("test_prepare_field_result");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_prepare_field_result");
  myquery(rc);

  rc = mysql_query(mysql,
                   "CREATE TABLE test_prepare_field_result(int_c int, "
                   "var_c varchar(50), ts_c timestamp, "
                   "char_c char(4), date_c date, extra tinyint)");
  myquery(rc);

  /* insert */
  my_stpcpy(query,
            "SELECT int_c, var_c, date_c as date, ts_c, char_c FROM "
            " test_prepare_field_result as t1 WHERE int_c=?");
  stmt = mysql_simple_prepare(mysql, query);
  check_stmt(stmt);

  verify_param_count(stmt, 1);

  result = mysql_stmt_result_metadata(stmt);
  mytest(result);

  my_print_result_metadata(result);

  if (!opt_silent) fprintf(stdout, "\n\n field attributes:\n");
  verify_prepare_field(result, 0, "int_c", "int_c", MYSQL_TYPE_LONG, "t1",
                       "test_prepare_field_result", current_db, 11, 0);
  verify_prepare_field(result, 1, "var_c", "var_c", MYSQL_TYPE_VAR_STRING, "t1",
                       "test_prepare_field_result", current_db, 50, 0);
  verify_prepare_field(result, 2, "date", "date_c", MYSQL_TYPE_DATE, "t1",
                       "test_prepare_field_result", current_db, 10, 0);
  verify_prepare_field(result, 3, "ts_c", "ts_c", MYSQL_TYPE_TIMESTAMP, "t1",
                       "test_prepare_field_result", current_db, 19, 0);
  verify_prepare_field(
      result, 4, "char_c", "char_c",
      (mysql_get_server_version(mysql) <= 50000 ? MYSQL_TYPE_VAR_STRING
                                                : MYSQL_TYPE_STRING),
      "t1", "test_prepare_field_result", current_db, 4, 0);

  verify_field_count(result, 5);
  mysql_free_result(result);
  mysql_stmt_close(stmt);
}

/* Test simple prepare field results */

static void test_prepare_syntax() {
  MYSQL_STMT *stmt;
  int rc;
  char query[MAX_TEST_QUERY_LENGTH];

  myheader("test_prepare_syntax");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_prepare_syntax");
  myquery(rc);

  rc = mysql_query(mysql,
                   "CREATE TABLE test_prepare_syntax("
                   "id int, name varchar(50), extra int)");
  myquery(rc);

  my_stpcpy(query, "INSERT INTO test_prepare_syntax VALUES(?");
  stmt = mysql_simple_prepare(mysql, query);
  check_stmt_r(stmt);

  my_stpcpy(query,
            "SELECT id, name FROM test_prepare_syntax WHERE id=? AND WHERE");
  stmt = mysql_simple_prepare(mysql, query);
  check_stmt_r(stmt);

  /* now fetch the results ..*/
  rc = mysql_commit(mysql);
  myquery(rc);
}

/* Test a simple prepare */

static void test_prepare() {
  MYSQL_STMT *stmt;
  int rc, i;
  int int_data, o_int_data;
  char str_data[50], data[50];
  char tiny_data, o_tiny_data;
  short small_data, o_small_data;
  longlong big_data, o_big_data;
  float real_data, o_real_data;
  double double_data, o_double_data;
  ulong length[7], len;
  bool is_null[7];
  char llbuf[22];
  MYSQL_BIND my_bind[7];
  char query[MAX_TEST_QUERY_LENGTH];

  myheader("test_prepare");

  rc = mysql_autocommit(mysql, true);
  myquery(rc);

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS my_prepare");
  myquery(rc);

  rc = mysql_query(mysql,
                   "CREATE TABLE my_prepare(col1 tinyint, "
                   "col2 varchar(15), col3 int, "
                   "col4 smallint, col5 bigint, "
                   "col6 float, col7 double )");
  myquery(rc);

  /* insert by prepare */
  strxmov(query, "INSERT INTO my_prepare VALUES(?, ?, ?, ?, ?, ?, ?)", NullS);
  stmt = mysql_simple_prepare(mysql, query);
  check_stmt(stmt);

  verify_param_count(stmt, 7);

  memset(my_bind, 0, sizeof(my_bind));

  /* tinyint */
  my_bind[0].buffer_type = MYSQL_TYPE_TINY;
  my_bind[0].buffer = (void *)&tiny_data;
  /* string */
  my_bind[1].buffer_type = MYSQL_TYPE_STRING;
  my_bind[1].buffer = (void *)str_data;
  my_bind[1].buffer_length = 1000; /* Max string length */
  /* integer */
  my_bind[2].buffer_type = MYSQL_TYPE_LONG;
  my_bind[2].buffer = (void *)&int_data;
  /* short */
  my_bind[3].buffer_type = MYSQL_TYPE_SHORT;
  my_bind[3].buffer = (void *)&small_data;
  /* bigint */
  my_bind[4].buffer_type = MYSQL_TYPE_LONGLONG;
  my_bind[4].buffer = (void *)&big_data;
  /* float */
  my_bind[5].buffer_type = MYSQL_TYPE_FLOAT;
  my_bind[5].buffer = (void *)&real_data;
  /* double */
  my_bind[6].buffer_type = MYSQL_TYPE_DOUBLE;
  my_bind[6].buffer = (void *)&double_data;

  for (i = 0; i < (int)array_elements(my_bind); i++) {
    my_bind[i].length = &length[i];
    my_bind[i].is_null = &is_null[i];
    is_null[i] = false;
  }

  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  int_data = 320;
  small_data = 1867;
  big_data = 1000;
  real_data = 2;
  double_data = 6578.001;

  /* now, execute the prepared statement to insert 10 records.. */
  for (tiny_data = 0; tiny_data < 100; tiny_data++) {
    length[1] = sprintf(str_data, "MySQL%d", int_data);
    rc = mysql_stmt_execute(stmt);
    check_execute(stmt, rc);
    int_data += 25;
    small_data += 10;
    big_data += 100;
    real_data += 1;
    double_data += 10.09;
  }

  mysql_stmt_close(stmt);

  /* now fetch the results ..*/
  rc = mysql_commit(mysql);
  myquery(rc);

  /* test the results now, only one row should exist */
  rc = my_stmt_result("SELECT * FROM my_prepare");
  DIE_UNLESS(tiny_data == (char)rc);

  stmt = mysql_simple_prepare(mysql, "SELECT * FROM my_prepare");
  check_stmt(stmt);

  rc = mysql_stmt_bind_result(stmt, my_bind);
  check_execute(stmt, rc);

  /* get the result */
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  o_int_data = 320;
  o_small_data = 1867;
  o_big_data = 1000;
  o_real_data = 2;
  o_double_data = 6578.001;

  /* now, execute the prepared statement to insert 10 records.. */
  for (o_tiny_data = 0; o_tiny_data < 100; o_tiny_data++) {
    len = sprintf(data, "MySQL%d", o_int_data);

    rc = mysql_stmt_fetch(stmt);
    check_execute(stmt, rc);

    if (!opt_silent) {
      fprintf(stdout, "\n");
      fprintf(stdout, "\n\t tiny   : %d (%lu)", tiny_data, length[0]);
      fprintf(stdout, "\n\t short  : %d (%lu)", small_data, length[3]);
      fprintf(stdout, "\n\t int    : %d (%lu)", int_data, length[2]);
      fprintf(stdout, "\n\t big    : %s (%lu)", llstr(big_data, llbuf),
              length[4]);

      fprintf(stdout, "\n\t float  : %f (%lu)", real_data, length[5]);
      fprintf(stdout, "\n\t double : %f (%lu)", double_data, length[6]);

      fprintf(stdout, "\n\t str    : %s (%lu)", str_data, length[1]);
    }

    DIE_UNLESS(tiny_data == o_tiny_data);
    DIE_UNLESS(is_null[0] == 0);
    DIE_UNLESS(length[0] == 1);

    DIE_UNLESS(int_data == o_int_data);
    DIE_UNLESS(length[2] == 4);

    DIE_UNLESS(small_data == o_small_data);
    DIE_UNLESS(length[3] == 2);

    DIE_UNLESS(big_data == o_big_data);
    DIE_UNLESS(length[4] == 8);

    DIE_UNLESS(real_data == o_real_data);
    DIE_UNLESS(length[5] == 4);

    DIE_UNLESS(cmp_double(&double_data, &o_double_data));
    DIE_UNLESS(length[6] == 8);

    DIE_UNLESS(strcmp(data, str_data) == 0);
    DIE_UNLESS(length[1] == len);

    o_int_data += 25;
    o_small_data += 10;
    o_big_data += 100;
    o_real_data += 1;
    o_double_data += 10.09;
  }

  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == MYSQL_NO_DATA);

  mysql_stmt_close(stmt);
}

/* Test double comparision */

static void test_double_compare() {
  MYSQL_STMT *stmt;
  int rc;
  char real_data[10], tiny_data;
  double double_data;
  MYSQL_RES *result;
  MYSQL_BIND my_bind[3];
  ulong length[3];
  char query[MAX_TEST_QUERY_LENGTH];

  myheader("test_double_compare");

  rc = mysql_autocommit(mysql, true);
  myquery(rc);

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_double_compare");
  myquery(rc);

  rc = mysql_query(mysql,
                   "CREATE TABLE test_double_compare(col1 tinyint, "
                   " col2 float, col3 double )");
  myquery(rc);

  rc = mysql_query(mysql,
                   "INSERT INTO test_double_compare "
                   "VALUES (1, 10.2, 34.5)");
  myquery(rc);

  my_stpcpy(query,
            "UPDATE test_double_compare SET col1=100 "
            "WHERE col1 = ? AND col2 = ? AND COL3 = ?");
  stmt = mysql_simple_prepare(mysql, query);
  check_stmt(stmt);

  verify_param_count(stmt, 3);

  /* Always memset bind array because there can be internal members */
  memset(my_bind, 0, sizeof(my_bind));
  memset(real_data, 0, sizeof(real_data));

  /* tinyint */
  my_bind[0].buffer_type = MYSQL_TYPE_TINY;
  my_bind[0].buffer = (void *)&tiny_data;

  /* string->float */
  my_bind[1].buffer_type = MYSQL_TYPE_STRING;
  my_bind[1].buffer = (void *)&real_data;
  my_bind[1].buffer_length = (ulong)sizeof(real_data);
  my_bind[1].length = &length[1];
  length[1] = 10;

  /* double */
  my_bind[2].buffer_type = MYSQL_TYPE_DOUBLE;
  my_bind[2].buffer = (void *)&double_data;

  tiny_data = 1;
  my_stpcpy(real_data, "10.2");
  double_data = 34.5;
  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  verify_affected_rows(0);

  mysql_stmt_close(stmt);

  /* now fetch the results ..*/
  rc = mysql_commit(mysql);
  myquery(rc);

  /* test the results now, only one row should exist */
  rc = mysql_query(mysql, "SELECT * FROM test_double_compare");
  myquery(rc);

  /* get the result */
  result = mysql_store_result(mysql);
  mytest(result);

  rc = my_process_result_set(result);
  DIE_UNLESS((int)tiny_data == rc);
  mysql_free_result(result);
}

/* Test simple null */

static void test_null() {
  MYSQL_STMT *stmt;
  int rc;
  uint nData;
  MYSQL_BIND my_bind[2];
  bool is_null[2];
  char query[MAX_TEST_QUERY_LENGTH];

  myheader("test_null");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_null");
  myquery(rc);

  rc = mysql_query(mysql, "CREATE TABLE test_null(col1 int, col2 varchar(50))");
  myquery(rc);

  /* insert by prepare, wrong column name */
  my_stpcpy(query, "INSERT INTO test_null(col3, col2) VALUES(?, ?)");
  stmt = mysql_simple_prepare(mysql, query);
  check_stmt_r(stmt);

  my_stpcpy(query, "INSERT INTO test_null(col1, col2) VALUES(?, ?)");
  stmt = mysql_simple_prepare(mysql, query);
  check_stmt(stmt);

  verify_param_count(stmt, 2);

  /* Always memset all members of bind parameter */
  memset(my_bind, 0, sizeof(my_bind));

  my_bind[0].buffer_type = MYSQL_TYPE_LONG;
  my_bind[0].is_null = &is_null[0];
  is_null[0] = true;
  my_bind[1] = my_bind[0];

  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  /* now, execute the prepared statement to insert 10 records.. */
  for (nData = 0; nData < 10; nData++) {
    rc = mysql_stmt_execute(stmt);
    check_execute(stmt, rc);
  }

  /* Re-bind with MYSQL_TYPE_NULL */
  my_bind[0].buffer_type = MYSQL_TYPE_NULL;
  is_null[0] = false; /* reset */
  my_bind[1] = my_bind[0];

  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  for (nData = 0; nData < 10; nData++) {
    rc = mysql_stmt_execute(stmt);
    check_execute(stmt, rc);
  }

  mysql_stmt_close(stmt);

  /* now fetch the results ..*/
  rc = mysql_commit(mysql);
  myquery(rc);

  nData *= 2;
  rc = my_stmt_result("SELECT * FROM test_null");
  ;
  DIE_UNLESS((int)nData == rc);

  /* Fetch results */
  my_bind[0].buffer_type = MYSQL_TYPE_LONG;
  my_bind[0].buffer = (void *)&nData; /* this buffer won't be altered */
  my_bind[0].length = nullptr;
  my_bind[1] = my_bind[0];
  my_bind[0].is_null = &is_null[0];
  my_bind[1].is_null = &is_null[1];

  stmt = mysql_simple_prepare(mysql, "SELECT * FROM test_null");
  check_stmt(stmt);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_bind_result(stmt, my_bind);
  check_execute(stmt, rc);

  rc = 0;
  is_null[0] = is_null[1] = false;
  while (mysql_stmt_fetch(stmt) != MYSQL_NO_DATA) {
    DIE_UNLESS(is_null[0]);
    DIE_UNLESS(is_null[1]);
    rc++;
    is_null[0] = is_null[1] = false;
  }
  DIE_UNLESS(rc == (int)nData);
  mysql_stmt_close(stmt);
}

/* Test for NULL as PS parameter (BUG#3367, BUG#3371) */

static void test_ps_null_param() {
  MYSQL_STMT *stmt;
  int rc;

  MYSQL_BIND in_bind;
  bool in_is_null;
  long int in_long;

  MYSQL_BIND out_bind;
  ulong out_length;
  bool out_is_null;
  char out_str_data[20];

  const char *queries[] = {"select ?", "select ?+1",
                           "select col1 from test_ps_nulls where col1 <=> ?",
                           nullptr};
  const char **cur_query = queries;

  myheader("test_null_ps_param_in_result");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_ps_nulls");
  myquery(rc);

  rc = mysql_query(mysql, "CREATE TABLE test_ps_nulls(col1 int)");
  myquery(rc);

  rc = mysql_query(mysql, "INSERT INTO test_ps_nulls values (1), (null)");
  myquery(rc);

  /* Always memset all members of bind parameter */
  memset(&in_bind, 0, sizeof(in_bind));
  memset(&out_bind, 0, sizeof(out_bind));

  in_bind.buffer_type = MYSQL_TYPE_LONG;
  in_bind.is_null = &in_is_null;
  in_bind.length = nullptr;
  in_bind.buffer = (void *)&in_long;
  in_is_null = true;
  in_long = 1;

  out_bind.buffer_type = MYSQL_TYPE_STRING;
  out_bind.is_null = &out_is_null;
  out_bind.length = &out_length;
  out_bind.buffer = out_str_data;
  out_bind.buffer_length = array_elements(out_str_data);

  /* Execute several queries, all returning NULL in result. */
  for (cur_query = queries; *cur_query; cur_query++) {
    char query[MAX_TEST_QUERY_LENGTH];
    my_stpcpy(query, *cur_query);
    stmt = mysql_simple_prepare(mysql, query);
    check_stmt(stmt);
    verify_param_count(stmt, 1);

    rc = mysql_stmt_bind_param(stmt, &in_bind);
    check_execute(stmt, rc);
    rc = mysql_stmt_bind_result(stmt, &out_bind);
    check_execute(stmt, rc);
    rc = mysql_stmt_execute(stmt);
    check_execute(stmt, rc);
    rc = mysql_stmt_fetch(stmt);
    DIE_UNLESS(rc != MYSQL_NO_DATA);
    DIE_UNLESS(out_is_null);
    rc = mysql_stmt_fetch(stmt);
    DIE_UNLESS(rc == MYSQL_NO_DATA);
    mysql_stmt_close(stmt);
  }
}

/* Test fetch null */

static void test_fetch_null() {
  MYSQL_STMT *stmt;
  int rc;
  int i, nData;
  MYSQL_BIND my_bind[11];
  ulong length[11];
  bool is_null[11];
  char query[MAX_TEST_QUERY_LENGTH];

  myheader("test_fetch_null");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_fetch_null");
  myquery(rc);

  rc = mysql_query(mysql,
                   "CREATE TABLE test_fetch_null("
                   " col1 tinyint, col2 smallint, "
                   " col3 int, col4 bigint, "
                   " col5 float, col6 double, "
                   " col7 date, col8 time, "
                   " col9 varbinary(10), "
                   " col10 varchar(50), "
                   " col11 char(20))");
  myquery(rc);

  rc = mysql_query(mysql,
                   "INSERT INTO test_fetch_null (col11) "
                   "VALUES (1000), (88), (389789)");
  myquery(rc);

  rc = mysql_commit(mysql);
  myquery(rc);

  /* fetch */
  memset(my_bind, 0, sizeof(my_bind));
  for (i = 0; i < (int)array_elements(my_bind); i++) {
    my_bind[i].buffer_type = MYSQL_TYPE_LONG;
    my_bind[i].is_null = &is_null[i];
    my_bind[i].length = &length[i];
  }
  my_bind[i - 1].buffer = (void *)&nData; /* Last column is not null */

  my_stpcpy((char *)query, "SELECT * FROM test_fetch_null");

  rc = my_stmt_result(query);
  DIE_UNLESS(rc == 3);

  stmt = mysql_simple_prepare(mysql, query);
  check_stmt(stmt);

  rc = mysql_stmt_bind_result(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = 0;
  while (mysql_stmt_fetch(stmt) != MYSQL_NO_DATA) {
    rc++;
    for (i = 0; i < 10; i++) {
      if (!opt_silent)
        fprintf(stdout, "\n data[%d] : %s", i,
                is_null[i] ? "NULL" : "NOT NULL");
      DIE_UNLESS(is_null[i]);
    }
    if (!opt_silent) fprintf(stdout, "\n data[%d]: %d", i, nData);
    DIE_UNLESS(nData == 1000 || nData == 88 || nData == 389789);
    DIE_UNLESS(is_null[i] == 0);
    DIE_UNLESS(length[i] == 4);
  }
  DIE_UNLESS(rc == 3);
  mysql_stmt_close(stmt);
}

/* Test simple select */

static void test_select_version() {
  MYSQL_STMT *stmt;
  int rc;

  myheader("test_select_version");

  stmt = mysql_simple_prepare(mysql, "SELECT @@version");
  check_stmt(stmt);

  verify_param_count(stmt, 0);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  my_process_stmt_result(stmt);
  mysql_stmt_close(stmt);
}

/* Test simple show */

static void test_select_show_table() {
  MYSQL_STMT *stmt;
  int rc, i;

  myheader("test_select_show_table");

  stmt = mysql_simple_prepare(mysql, "SHOW TABLES FROM mysql");
  check_stmt(stmt);

  verify_param_count(stmt, 0);

  for (i = 1; i < 3; i++) {
    rc = mysql_stmt_execute(stmt);
    check_execute(stmt, rc);
  }

  my_process_stmt_result(stmt);
  mysql_stmt_close(stmt);
}

/* Test simple select to debug */

static void test_select_direct() {
  int rc;
  MYSQL_RES *result;

  myheader("test_select_direct");

  rc = mysql_autocommit(mysql, true);
  myquery(rc);

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_select");
  myquery(rc);

  rc = mysql_query(mysql,
                   "CREATE TABLE test_select(id int, id1 tinyint, "
                   " id2 float, "
                   " id3 double, "
                   " name varchar(50))");
  myquery(rc);

  /* insert a row and commit the transaction */
  rc = mysql_query(mysql,
                   "INSERT INTO test_select VALUES(10, 5, 2.3, 4.5, 'venu')");
  myquery(rc);

  rc = mysql_commit(mysql);
  myquery(rc);

  rc = mysql_query(mysql, "SELECT * FROM test_select");
  myquery(rc);

  /* get the result */
  result = mysql_store_result(mysql);
  mytest(result);

  (void)my_process_result_set(result);
  mysql_free_result(result);
}

/* Test simple select with prepare */

static void test_select_prepare() {
  int rc;
  MYSQL_STMT *stmt;

  myheader("test_select_prepare");

  rc = mysql_autocommit(mysql, true);
  myquery(rc);

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_select");
  myquery(rc);

  rc = mysql_query(mysql, "CREATE TABLE test_select(id int, name varchar(50))");
  myquery(rc);

  /* insert a row and commit the transaction */
  rc = mysql_query(mysql, "INSERT INTO test_select VALUES(10, 'venu')");
  myquery(rc);

  rc = mysql_commit(mysql);
  myquery(rc);

  stmt = mysql_simple_prepare(mysql, "SELECT * FROM test_select");
  check_stmt(stmt);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = my_process_stmt_result(stmt);
  DIE_UNLESS(rc == 1);
  mysql_stmt_close(stmt);

  rc = mysql_query(mysql, "DROP TABLE test_select");
  myquery(rc);

  rc = mysql_query(mysql,
                   "CREATE TABLE test_select(id tinyint, id1 int, "
                   "  id2 float, id3 float, "
                   "  name varchar(50))");
  myquery(rc);

  /* insert a row and commit the transaction */
  rc = mysql_query(
      mysql,
      "INSERT INTO test_select(id, id1, id2, name) VALUES(10, 5, 2.3, 'venu')");
  myquery(rc);

  rc = mysql_commit(mysql);
  myquery(rc);

  stmt = mysql_simple_prepare(mysql, "SELECT * FROM test_select");
  check_stmt(stmt);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = my_process_stmt_result(stmt);
  DIE_UNLESS(rc == 1);
  mysql_stmt_close(stmt);
}

/* Test simple select */

static void test_select() {
  MYSQL_STMT *stmt;
  int rc;
  char szData[25];
  int nData = 1;
  MYSQL_BIND my_bind[2];
  ulong length[2];
  char query[MAX_TEST_QUERY_LENGTH];

  myheader("test_select");

  rc = mysql_autocommit(mysql, true);
  myquery(rc);

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_select");
  myquery(rc);

  rc = mysql_query(mysql, "CREATE TABLE test_select(id int, name varchar(50))");
  myquery(rc);

  /* insert a row and commit the transaction */
  rc = mysql_query(mysql, "INSERT INTO test_select VALUES(10, 'venu')");
  myquery(rc);

  /* now insert the second row, and roll back the transaction */
  rc = mysql_query(mysql, "INSERT INTO test_select VALUES(20, 'mysql')");
  myquery(rc);

  rc = mysql_commit(mysql);
  myquery(rc);

  my_stpcpy(query,
            "SELECT * FROM test_select WHERE id= ? "
            "AND CONVERT(name USING utf8) =?");
  stmt = mysql_simple_prepare(mysql, query);
  check_stmt(stmt);

  verify_param_count(stmt, 2);

  /* Always memset all members of bind parameter */
  memset(my_bind, 0, sizeof(my_bind));

  /* string data */
  nData = 10;
  my_stpcpy(szData, "venu");
  my_bind[1].buffer_type = MYSQL_TYPE_STRING;
  my_bind[1].buffer = (void *)szData;
  my_bind[1].buffer_length = 4;
  my_bind[1].length = &length[1];
  length[1] = 4;

  my_bind[0].buffer = (void *)&nData;
  my_bind[0].buffer_type = MYSQL_TYPE_LONG;

  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = my_process_stmt_result(stmt);
  DIE_UNLESS(rc == 1);

  mysql_stmt_close(stmt);
}

/*
  Test for BUG#3420 ("select id1, value1 from t where id= ? or value= ?"
  returns all rows in the table)
*/

static void test_ps_conj_select() {
  MYSQL_STMT *stmt;
  int rc;
  MYSQL_BIND my_bind[2];
  int32 int_data;
  char str_data[32];
  ulong str_length;
  char query[MAX_TEST_QUERY_LENGTH];
  myheader("test_ps_conj_select");

  rc = mysql_query(mysql, "drop table if exists t1");
  myquery(rc);

  rc = mysql_query(mysql,
                   "create table t1 (id1 int(11) NOT NULL default '0', "
                   "value2 varchar(100), value1 varchar(100))");
  myquery(rc);

  rc = mysql_query(mysql,
                   "insert into t1 values (1, 'hh', 'hh'), "
                   "(2, 'hh', 'hh'), (1, 'ii', 'ii'), (2, 'ii', 'ii')");
  myquery(rc);

  my_stpcpy(query,
            "select id1, value1 from t1 where id1= ? or "
            "CONVERT(value1 USING utf8)= ?");
  stmt = mysql_simple_prepare(mysql, query);
  check_stmt(stmt);

  verify_param_count(stmt, 2);

  /* Always memset all members of bind parameter */
  memset(my_bind, 0, sizeof(my_bind));

  my_bind[0].buffer_type = MYSQL_TYPE_LONG;
  my_bind[0].buffer = (void *)&int_data;

  my_bind[1].buffer_type = MYSQL_TYPE_VAR_STRING;
  my_bind[1].buffer = (void *)str_data;
  my_bind[1].buffer_length = array_elements(str_data);
  my_bind[1].length = &str_length;

  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  int_data = 1;
  my_stpcpy(str_data, "hh");
  str_length = (ulong)strlen(str_data);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = my_process_stmt_result(stmt);
  DIE_UNLESS(rc == 3);

  mysql_stmt_close(stmt);
}

/* Test BUG#1115 (incorrect string parameter value allocation) */

static void test_bug1115() {
  MYSQL_STMT *stmt;
  int rc;
  MYSQL_BIND my_bind[1];
  ulong length[1];
  char szData[11];
  char query[MAX_TEST_QUERY_LENGTH];

  myheader("test_bug1115");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_select");
  myquery(rc);

  rc = mysql_query(mysql,
                   "CREATE TABLE test_select(\
session_id  char(9) NOT NULL, \
    a       int(8) unsigned NOT NULL, \
    b        int(5) NOT NULL, \
    c      int(5) NOT NULL, \
    d  datetime NOT NULL)");
  myquery(rc);
  rc = mysql_query(mysql,
                   "INSERT INTO test_select VALUES "
                   "(\"abc\", 1, 2, 3, 2003-08-30), "
                   "(\"abd\", 1, 2, 3, 2003-08-30), "
                   "(\"abf\", 1, 2, 3, 2003-08-30), "
                   "(\"abg\", 1, 2, 3, 2003-08-30), "
                   "(\"abh\", 1, 2, 3, 2003-08-30), "
                   "(\"abj\", 1, 2, 3, 2003-08-30), "
                   "(\"abk\", 1, 2, 3, 2003-08-30), "
                   "(\"abl\", 1, 2, 3, 2003-08-30), "
                   "(\"abq\", 1, 2, 3, 2003-08-30) ");
  myquery(rc);
  rc = mysql_query(mysql,
                   "INSERT INTO test_select VALUES "
                   "(\"abw\", 1, 2, 3, 2003-08-30), "
                   "(\"abe\", 1, 2, 3, 2003-08-30), "
                   "(\"abr\", 1, 2, 3, 2003-08-30), "
                   "(\"abt\", 1, 2, 3, 2003-08-30), "
                   "(\"aby\", 1, 2, 3, 2003-08-30), "
                   "(\"abu\", 1, 2, 3, 2003-08-30), "
                   "(\"abi\", 1, 2, 3, 2003-08-30), "
                   "(\"abo\", 1, 2, 3, 2003-08-30), "
                   "(\"abp\", 1, 2, 3, 2003-08-30), "
                   "(\"abz\", 1, 2, 3, 2003-08-30), "
                   "(\"abx\", 1, 2, 3, 2003-08-30)");
  myquery(rc);

  my_stpcpy(query,
            "SELECT * FROM test_select WHERE "
            "CONVERT(session_id USING utf8)= ?");
  stmt = mysql_simple_prepare(mysql, query);
  check_stmt(stmt);

  verify_param_count(stmt, 1);

  /* Always memset all members of bind parameter */
  memset(my_bind, 0, sizeof(my_bind));

  my_stpcpy(szData, "abc");
  my_bind[0].buffer_type = MYSQL_TYPE_STRING;
  my_bind[0].buffer = (void *)szData;
  my_bind[0].buffer_length = 10;
  my_bind[0].length = &length[0];
  length[0] = 3;

  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = my_process_stmt_result(stmt);
  DIE_UNLESS(rc == 1);

  my_stpcpy(szData, "venu");
  my_bind[0].buffer_type = MYSQL_TYPE_STRING;
  my_bind[0].buffer = (void *)szData;
  my_bind[0].buffer_length = 10;
  my_bind[0].length = &length[0];
  length[0] = 4;
  my_bind[0].is_null = nullptr;

  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = my_process_stmt_result(stmt);
  DIE_UNLESS(rc == 0);

  my_stpcpy(szData, "abc");
  my_bind[0].buffer_type = MYSQL_TYPE_STRING;
  my_bind[0].buffer = (void *)szData;
  my_bind[0].buffer_length = 10;
  my_bind[0].length = &length[0];
  length[0] = 3;
  my_bind[0].is_null = nullptr;

  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = my_process_stmt_result(stmt);
  DIE_UNLESS(rc == 1);

  mysql_stmt_close(stmt);
}

/* Test BUG#1180 (optimized away part of WHERE clause) */

static void test_bug1180() {
  MYSQL_STMT *stmt;
  int rc;
  MYSQL_BIND my_bind[1];
  ulong length[1];
  char szData[11];
  char query[MAX_TEST_QUERY_LENGTH];

  myheader("test_select_bug");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_select");
  myquery(rc);

  rc = mysql_query(mysql,
                   "CREATE TABLE test_select(session_id  char(9) NOT NULL)");
  myquery(rc);
  rc = mysql_query(mysql, "INSERT INTO test_select VALUES (\"abc\")");
  myquery(rc);

  my_stpcpy(query,
            "SELECT * FROM test_select WHERE ?= \"1111\" and "
            "session_id= \"abc\"");
  stmt = mysql_simple_prepare(mysql, query);
  check_stmt(stmt);

  verify_param_count(stmt, 1);

  /* Always memset all members of bind parameter */
  memset(my_bind, 0, sizeof(my_bind));

  my_stpcpy(szData, "abc");
  my_bind[0].buffer_type = MYSQL_TYPE_STRING;
  my_bind[0].buffer = (void *)szData;
  my_bind[0].buffer_length = 10;
  my_bind[0].length = &length[0];
  length[0] = 3;
  my_bind[0].is_null = nullptr;

  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = my_process_stmt_result(stmt);
  DIE_UNLESS(rc == 0);

  my_stpcpy(szData, "1111");
  my_bind[0].buffer_type = MYSQL_TYPE_STRING;
  my_bind[0].buffer = (void *)szData;
  my_bind[0].buffer_length = 10;
  my_bind[0].length = &length[0];
  length[0] = 4;
  my_bind[0].is_null = nullptr;

  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = my_process_stmt_result(stmt);
  DIE_UNLESS(rc == 1);

  my_stpcpy(szData, "abc");
  my_bind[0].buffer_type = MYSQL_TYPE_STRING;
  my_bind[0].buffer = (void *)szData;
  my_bind[0].buffer_length = 10;
  my_bind[0].length = &length[0];
  length[0] = 3;
  my_bind[0].is_null = nullptr;

  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = my_process_stmt_result(stmt);
  DIE_UNLESS(rc == 0);

  mysql_stmt_close(stmt);
}

/*
  Test BUG#1644 (Insertion of more than 3 NULL columns with parameter
  binding fails)
*/

static void test_bug1644() {
  MYSQL_STMT *stmt;
  MYSQL_RES *result;
  MYSQL_ROW row;
  MYSQL_BIND my_bind[4];
  int num;
  bool isnull;
  int rc, i;
  char query[MAX_TEST_QUERY_LENGTH];

  myheader("test_bug1644");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS foo_dfr");
  myquery(rc);

  rc = mysql_query(
      mysql, "CREATE TABLE foo_dfr(col1 int, col2 int, col3 int, col4 int);");
  myquery(rc);

  my_stpcpy(query, "INSERT INTO foo_dfr VALUES (?, ?, ?, ? )");
  stmt = mysql_simple_prepare(mysql, query);
  check_stmt(stmt);

  verify_param_count(stmt, 4);

  /* Always memset all members of bind parameter */
  memset(my_bind, 0, sizeof(my_bind));

  num = 22;
  isnull = false;
  for (i = 0; i < 4; i++) {
    my_bind[i].buffer_type = MYSQL_TYPE_LONG;
    my_bind[i].buffer = (void *)&num;
    my_bind[i].is_null = &isnull;
  }

  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  isnull = true;
  for (i = 0; i < 4; i++) my_bind[i].is_null = &isnull;

  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  isnull = false;
  num = 88;
  for (i = 0; i < 4; i++) my_bind[i].is_null = &isnull;

  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  mysql_stmt_close(stmt);

  rc = mysql_query(mysql, "SELECT * FROM foo_dfr");
  myquery(rc);

  result = mysql_store_result(mysql);
  mytest(result);

  rc = my_process_result_set(result);
  DIE_UNLESS(rc == 3);

  mysql_data_seek(result, 0);

  row = mysql_fetch_row(result);
  mytest(row);
  for (i = 0; i < 4; i++) {
    DIE_UNLESS(strcmp(row[i], "22") == 0);
  }
  row = mysql_fetch_row(result);
  mytest(row);
  for (i = 0; i < 4; i++) {
    DIE_UNLESS(row[i] == nullptr);
  }
  row = mysql_fetch_row(result);
  mytest(row);
  for (i = 0; i < 4; i++) {
    DIE_UNLESS(strcmp(row[i], "88") == 0);
  }
  row = mysql_fetch_row(result);
  mytest_r(row);

  mysql_free_result(result);
}

/* Test simple select show */

static void test_select_show() {
  MYSQL_STMT *stmt;
  int rc;
  char query[MAX_TEST_QUERY_LENGTH];

  myheader("test_select_show");

  mysql_autocommit(mysql, true);

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_show");
  myquery(rc);

  rc = mysql_query(mysql,
                   "CREATE TABLE test_show(id int(4) NOT NULL primary "
                   " key, name char(2))");
  myquery(rc);

  stmt = mysql_simple_prepare(mysql, "show columns from test_show");
  check_stmt(stmt);

  verify_param_count(stmt, 0);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  my_process_stmt_result(stmt);
  mysql_stmt_close(stmt);

  stmt = mysql_simple_prepare(mysql, "show tables from mysql like ?");
  check_stmt_r(stmt);

  strxmov(query, "show tables from ", current_db, " like \'test_show\'", NullS);
  stmt = mysql_simple_prepare(mysql, query);
  check_stmt(stmt);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  my_process_stmt_result(stmt);
  mysql_stmt_close(stmt);

  stmt = mysql_simple_prepare(mysql, "describe test_show");
  check_stmt(stmt);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  my_process_stmt_result(stmt);
  mysql_stmt_close(stmt);

  stmt = mysql_simple_prepare(mysql, "show keys from test_show");
  check_stmt(stmt);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = my_process_stmt_result(stmt);
  DIE_UNLESS(rc == 1);
  mysql_stmt_close(stmt);
}

/* Test simple update */

static void test_simple_update() {
  MYSQL_STMT *stmt;
  int rc;
  char szData[25];
  int nData = 1;
  MYSQL_RES *result;
  MYSQL_BIND my_bind[2];
  ulong length[2];
  char query[MAX_TEST_QUERY_LENGTH];

  myheader("test_simple_update");

  rc = mysql_autocommit(mysql, true);
  myquery(rc);

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_update");
  myquery(rc);

  rc = mysql_query(mysql,
                   "CREATE TABLE test_update(col1 int, "
                   " col2 varchar(50), col3 int )");
  myquery(rc);

  rc = mysql_query(mysql, "INSERT INTO test_update VALUES(1, 'MySQL', 100)");
  myquery(rc);

  verify_affected_rows(1);

  rc = mysql_commit(mysql);
  myquery(rc);

  /* insert by prepare */
  my_stpcpy(query, "UPDATE test_update SET col2= ? WHERE col1= ?");
  stmt = mysql_simple_prepare(mysql, query);
  check_stmt(stmt);

  verify_param_count(stmt, 2);

  /* Always memset all members of bind parameter */
  memset(my_bind, 0, sizeof(my_bind));

  nData = 1;
  my_bind[0].buffer_type = MYSQL_TYPE_STRING;
  my_bind[0].buffer = szData; /* string data */
  my_bind[0].buffer_length = (ulong)sizeof(szData);
  my_bind[0].length = &length[0];
  length[0] = sprintf(szData, "updated-data");

  my_bind[1].buffer = (void *)&nData;
  my_bind[1].buffer_type = MYSQL_TYPE_LONG;

  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);
  verify_affected_rows(1);

  mysql_stmt_close(stmt);

  /* now fetch the results ..*/
  rc = mysql_commit(mysql);
  myquery(rc);

  /* test the results now, only one row should exist */
  rc = mysql_query(mysql, "SELECT * FROM test_update");
  myquery(rc);

  /* get the result */
  result = mysql_store_result(mysql);
  mytest(result);

  rc = my_process_result_set(result);
  DIE_UNLESS(rc == 1);
  mysql_free_result(result);
}

/* Test simple long data handling */

static void test_long_data() {
  MYSQL_STMT *stmt;
  int rc, int_data;
  const char *data = nullptr;
  MYSQL_RES *result;
  MYSQL_BIND my_bind[3];
  char query[MAX_TEST_QUERY_LENGTH];

  myheader("test_long_data");

  rc = mysql_autocommit(mysql, true);
  myquery(rc);

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_long_data");
  myquery(rc);

  rc = mysql_query(mysql,
                   "CREATE TABLE test_long_data(col1 int, "
                   "      col2 long varchar, col3 long varbinary)");
  myquery(rc);

  my_stpcpy(query, "INSERT INTO test_long_data(col1, col2) VALUES(?)");
  stmt = mysql_simple_prepare(mysql, query);
  check_stmt_r(stmt);

  my_stpcpy(query,
            "INSERT INTO test_long_data(col1, col2, col3) VALUES(?, ?, ?)");
  stmt = mysql_simple_prepare(mysql, query);
  check_stmt(stmt);

  verify_param_count(stmt, 3);

  /* Always memset all members of bind parameter */
  memset(my_bind, 0, sizeof(my_bind));

  my_bind[0].buffer = (void *)&int_data;
  my_bind[0].buffer_type = MYSQL_TYPE_LONG;

  my_bind[1].buffer_type = MYSQL_TYPE_STRING;

  my_bind[2] = my_bind[1];
  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  int_data = 999;
  data = "Michael";

  /* supply data in pieces */
  rc = mysql_stmt_send_long_data(stmt, 1, data, (ulong)strlen(data));
  data = " 'Monty' Widenius";
  rc = mysql_stmt_send_long_data(stmt, 1, data, (ulong)strlen(data));
  check_execute(stmt, rc);
  rc = mysql_stmt_send_long_data(stmt, 2, "Venu (venu@mysql.com)", 4);
  check_execute(stmt, rc);

  /* execute */
  rc = mysql_stmt_execute(stmt);
  if (!opt_silent) fprintf(stdout, " mysql_stmt_execute() returned %d\n", rc);
  check_execute(stmt, rc);

  rc = mysql_commit(mysql);
  myquery(rc);

  /* now fetch the results ..*/
  rc = mysql_query(mysql, "SELECT * FROM test_long_data");
  myquery(rc);

  /* get the result */
  result = mysql_store_result(mysql);
  mytest(result);

  rc = my_process_result_set(result);
  DIE_UNLESS(rc == 1);
  mysql_free_result(result);

  verify_col_data("test_long_data", "col1", "999");
  verify_col_data("test_long_data", "col2", "Michael 'Monty' Widenius");
  verify_col_data("test_long_data", "col3", "Venu");
  mysql_stmt_close(stmt);
}

/* Test long data (string) handling */

static void test_long_data_str() {
  MYSQL_STMT *stmt;
  int rc, i;
  char data[255];
  long length;
  ulong length1;
  MYSQL_RES *result;
  MYSQL_BIND my_bind[2];
  bool is_null[2];
  char query[MAX_TEST_QUERY_LENGTH];

  myheader("test_long_data_str");

  rc = mysql_autocommit(mysql, true);
  myquery(rc);

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_long_data_str");
  myquery(rc);

  rc = mysql_query(
      mysql, "CREATE TABLE test_long_data_str(id int, longstr long varchar)");
  myquery(rc);

  my_stpcpy(query, "INSERT INTO test_long_data_str VALUES(?, ?)");
  stmt = mysql_simple_prepare(mysql, query);
  check_stmt(stmt);

  verify_param_count(stmt, 2);

  /* Always memset all members of bind parameter */
  memset(my_bind, 0, sizeof(my_bind));

  my_bind[0].buffer = (void *)&length;
  my_bind[0].buffer_type = MYSQL_TYPE_LONG;
  my_bind[0].is_null = &is_null[0];
  is_null[0] = false;
  length = 0;

  my_bind[1].buffer = data; /* string data */
  my_bind[1].buffer_type = MYSQL_TYPE_STRING;
  my_bind[1].length = &length1;
  my_bind[1].is_null = &is_null[1];
  is_null[1] = false;
  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  length = 40;
  my_stpcpy(data, "MySQL AB");

  /* supply data in pieces */
  for (i = 0; i < 4; i++) {
    rc = mysql_stmt_send_long_data(stmt, 1, (char *)data, 5);
    check_execute(stmt, rc);
  }
  /* execute */
  rc = mysql_stmt_execute(stmt);
  if (!opt_silent) fprintf(stdout, " mysql_stmt_execute() returned %d\n", rc);
  check_execute(stmt, rc);

  mysql_stmt_close(stmt);

  rc = mysql_commit(mysql);
  myquery(rc);

  /* now fetch the results ..*/
  rc = mysql_query(mysql,
                   "SELECT LENGTH(longstr), longstr FROM test_long_data_str");
  myquery(rc);

  /* get the result */
  result = mysql_store_result(mysql);
  mytest(result);

  rc = my_process_result_set(result);
  DIE_UNLESS(rc == 1);
  mysql_free_result(result);

  sprintf(data, "%d", i * 5);
  verify_col_data("test_long_data_str", "LENGTH(longstr)", data);
  data[0] = '\0';
  while (i--) strxmov(data, data, "MySQL", NullS);
  verify_col_data("test_long_data_str", "longstr", data);

  rc = mysql_query(mysql, "DROP TABLE test_long_data_str");
  myquery(rc);
}

/* Test long data (string) handling */

static void test_long_data_str1() {
  MYSQL_STMT *stmt;
  int rc, i;
  char data[255];
  long length;
  ulong max_blob_length, blob_length, length1;
  bool true_value;
  MYSQL_RES *result;
  MYSQL_BIND my_bind[2];
  MYSQL_FIELD *field;
  char query[MAX_TEST_QUERY_LENGTH];

  myheader("test_long_data_str1");

  rc = mysql_autocommit(mysql, true);
  myquery(rc);

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_long_data_str");
  myquery(rc);

  rc = mysql_query(mysql,
                   "CREATE TABLE test_long_data_str(longstr long varchar, blb "
                   "long varbinary)");
  myquery(rc);

  my_stpcpy(query, "INSERT INTO test_long_data_str VALUES(?, ?)");
  stmt = mysql_simple_prepare(mysql, query);
  check_stmt(stmt);

  verify_param_count(stmt, 2);

  /* Always memset all members of bind parameter */
  memset(my_bind, 0, sizeof(my_bind));

  my_bind[0].buffer = data; /* string data */
  my_bind[0].buffer_length = (ulong)sizeof(data);
  my_bind[0].length = &length1;
  my_bind[0].buffer_type = MYSQL_TYPE_STRING;
  length1 = 0;

  my_bind[1] = my_bind[0];
  my_bind[1].buffer_type = MYSQL_TYPE_BLOB;

  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);
  length = sprintf(data, "MySQL AB");

  /* supply data in pieces */
  for (i = 0; i < 3; i++) {
    rc = mysql_stmt_send_long_data(stmt, 0, data, length);
    check_execute(stmt, rc);

    rc = mysql_stmt_send_long_data(stmt, 1, data, 2);
    check_execute(stmt, rc);
  }

  /* execute */
  rc = mysql_stmt_execute(stmt);
  if (!opt_silent) fprintf(stdout, " mysql_stmt_execute() returned %d\n", rc);
  check_execute(stmt, rc);

  mysql_stmt_close(stmt);

  rc = mysql_commit(mysql);
  myquery(rc);

  /* now fetch the results ..*/
  rc = mysql_query(mysql,
                   "SELECT LENGTH(longstr), longstr, LENGTH(blb), blb FROM "
                   "test_long_data_str");
  myquery(rc);

  /* get the result */
  result = mysql_store_result(mysql);

  mysql_field_seek(result, 1);
  field = mysql_fetch_field(result);
  max_blob_length = field->max_length;

  mytest(result);

  rc = my_process_result_set(result);
  DIE_UNLESS(rc == 1);
  mysql_free_result(result);

  sprintf(data, "%ld", (long)i * length);
  verify_col_data("test_long_data_str", "length(longstr)", data);

  sprintf(data, "%d", i * 2);
  verify_col_data("test_long_data_str", "length(blb)", data);

  /* Test length of field->max_length */
  stmt = mysql_simple_prepare(mysql, "SELECT * from test_long_data_str");
  check_stmt(stmt);
  verify_param_count(stmt, 0);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_store_result(stmt);
  check_execute(stmt, rc);

  result = mysql_stmt_result_metadata(stmt);
  field = mysql_fetch_fields(result);

  /* First test what happens if STMT_ATTR_UPDATE_MAX_LENGTH is not used */
  DIE_UNLESS(field->max_length == 0);
  mysql_free_result(result);

  /* Enable updating of field->max_length */
  true_value = true;
  mysql_stmt_attr_set(stmt, STMT_ATTR_UPDATE_MAX_LENGTH, (void *)&true_value);
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_store_result(stmt);
  check_execute(stmt, rc);

  result = mysql_stmt_result_metadata(stmt);
  field = mysql_fetch_fields(result);

  DIE_UNLESS(field->max_length == max_blob_length);

  /* Fetch results into a data buffer that is smaller than data */
  memset(my_bind, 0, sizeof(*my_bind));
  my_bind[0].buffer_type = MYSQL_TYPE_BLOB;
  my_bind[0].buffer = (void *)&data; /* this buffer won't be altered */
  my_bind[0].buffer_length = 16;
  my_bind[0].length = &blob_length;
  my_bind[0].error = &my_bind[0].error_value;
  rc = mysql_stmt_bind_result(stmt, my_bind);
  data[16] = 0;

  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == MYSQL_DATA_TRUNCATED);
  DIE_UNLESS(my_bind[0].error_value);
  DIE_UNLESS(strlen(data) == 16);
  DIE_UNLESS(blob_length == max_blob_length);

  /* Fetch all data */
  memset((my_bind + 1), 0, sizeof(*my_bind));
  my_bind[1].buffer_type = MYSQL_TYPE_BLOB;
  my_bind[1].buffer = (void *)&data; /* this buffer won't be altered */
  my_bind[1].buffer_length = (ulong)sizeof(data);
  my_bind[1].length = &blob_length;
  memset(data, 0, sizeof(data));
  mysql_stmt_fetch_column(stmt, my_bind + 1, 0, 0);
  DIE_UNLESS(strlen(data) == max_blob_length);

  mysql_free_result(result);
  mysql_stmt_close(stmt);

  /* Drop created table */
  rc = mysql_query(mysql, "DROP TABLE test_long_data_str");
  myquery(rc);
}

/* Test long data (binary) handling */

static void test_long_data_bin() {
  MYSQL_STMT *stmt;
  int rc;
  char data[255];
  long length;
  MYSQL_RES *result;
  MYSQL_BIND my_bind[2];
  char query[MAX_TEST_QUERY_LENGTH];

  myheader("test_long_data_bin");

  rc = mysql_autocommit(mysql, true);
  myquery(rc);

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_long_data_bin");
  myquery(rc);

  rc = mysql_query(
      mysql, "CREATE TABLE test_long_data_bin(id int, longbin long varbinary)");
  myquery(rc);

  my_stpcpy(query, "INSERT INTO test_long_data_bin VALUES(?, ?)");
  stmt = mysql_simple_prepare(mysql, query);
  check_stmt(stmt);

  verify_param_count(stmt, 2);

  /* Always memset all members of bind parameter */
  memset(my_bind, 0, sizeof(my_bind));

  my_bind[0].buffer = (void *)&length;
  my_bind[0].buffer_type = MYSQL_TYPE_LONG;
  length = 0;

  my_bind[1].buffer = data; /* string data */
  my_bind[1].buffer_type = MYSQL_TYPE_LONG_BLOB;
  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  length = 10;
  my_stpcpy(data, "MySQL AB");

  /* supply data in pieces */
  {
    int i;
    for (i = 0; i < 100; i++) {
      rc = mysql_stmt_send_long_data(stmt, 1, (char *)data, 4);
      check_execute(stmt, rc);
    }
  }
  /* execute */
  rc = mysql_stmt_execute(stmt);
  if (!opt_silent) fprintf(stdout, " mysql_stmt_execute() returned %d\n", rc);
  check_execute(stmt, rc);

  mysql_stmt_close(stmt);

  rc = mysql_commit(mysql);
  myquery(rc);

  /* now fetch the results ..*/
  rc = mysql_query(mysql,
                   "SELECT LENGTH(longbin), longbin FROM test_long_data_bin");
  myquery(rc);

  /* get the result */
  result = mysql_store_result(mysql);
  mytest(result);

  rc = my_process_result_set(result);
  DIE_UNLESS(rc == 1);
  mysql_free_result(result);
}

/* Test simple delete */

static void test_simple_delete() {
  MYSQL_STMT *stmt;
  int rc;
  char szData[30] = {0};
  int nData = 1;
  MYSQL_RES *result;
  MYSQL_BIND my_bind[2];
  ulong length[2];
  char query[MAX_TEST_QUERY_LENGTH];

  myheader("test_simple_delete");

  rc = mysql_autocommit(mysql, true);
  myquery(rc);

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_simple_delete");
  myquery(rc);

  rc = mysql_query(mysql,
                   "CREATE TABLE test_simple_delete(col1 int, \
                                col2 varchar(50), col3 int )");
  myquery(rc);

  rc = mysql_query(mysql,
                   "INSERT INTO test_simple_delete VALUES(1, 'MySQL', 100)");
  myquery(rc);

  verify_affected_rows(1);

  rc = mysql_commit(mysql);
  myquery(rc);

  /* insert by prepare */
  my_stpcpy(query,
            "DELETE FROM test_simple_delete WHERE col1= ? AND "
            "CONVERT(col2 USING utf8)= ? AND col3= 100");
  stmt = mysql_simple_prepare(mysql, query);
  check_stmt(stmt);

  verify_param_count(stmt, 2);

  /* Always memset all members of bind parameter */
  memset(my_bind, 0, sizeof(my_bind));

  nData = 1;
  my_stpcpy(szData, "MySQL");
  my_bind[1].buffer_type = MYSQL_TYPE_STRING;
  my_bind[1].buffer = szData; /* string data */
  my_bind[1].buffer_length = (ulong)sizeof(szData);
  my_bind[1].length = &length[1];
  length[1] = 5;

  my_bind[0].buffer = (void *)&nData;
  my_bind[0].buffer_type = MYSQL_TYPE_LONG;

  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  verify_affected_rows(1);

  mysql_stmt_close(stmt);

  /* now fetch the results ..*/
  rc = mysql_commit(mysql);
  myquery(rc);

  /* test the results now, only one row should exist */
  rc = mysql_query(mysql, "SELECT * FROM test_simple_delete");
  myquery(rc);

  /* get the result */
  result = mysql_store_result(mysql);
  mytest(result);

  rc = my_process_result_set(result);
  DIE_UNLESS(rc == 0);
  mysql_free_result(result);
}

/* Test simple update */

static void test_update() {
  MYSQL_STMT *stmt;
  int rc;
  char szData[25];
  int nData = 1;
  MYSQL_RES *result;
  MYSQL_BIND my_bind[2];
  ulong length[2];
  char query[MAX_TEST_QUERY_LENGTH];

  myheader("test_update");

  rc = mysql_autocommit(mysql, true);
  myquery(rc);

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_update");
  myquery(rc);

  rc = mysql_query(mysql,
                   "CREATE TABLE test_update("
                   "col1 int primary key auto_increment, "
                   "col2 varchar(50), col3 int )");
  myquery(rc);

  my_stpcpy(query, "INSERT INTO test_update(col2, col3) VALUES(?, ?)");
  stmt = mysql_simple_prepare(mysql, query);
  check_stmt(stmt);

  verify_param_count(stmt, 2);

  /* Always memset all members of bind parameter */
  memset(my_bind, 0, sizeof(my_bind));

  /* string data */
  my_bind[0].buffer_type = MYSQL_TYPE_STRING;
  my_bind[0].buffer = szData;
  my_bind[0].buffer_length = (ulong)sizeof(szData);
  my_bind[0].length = &length[0];
  length[0] = sprintf(szData, "inserted-data");

  my_bind[1].buffer = (void *)&nData;
  my_bind[1].buffer_type = MYSQL_TYPE_LONG;

  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  nData = 100;
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  verify_affected_rows(1);
  mysql_stmt_close(stmt);

  my_stpcpy(query, "UPDATE test_update SET col2= ? WHERE col3= ?");
  stmt = mysql_simple_prepare(mysql, query);
  check_stmt(stmt);

  verify_param_count(stmt, 2);
  nData = 100;

  /* Always memset all members of bind parameter */
  memset(my_bind, 0, sizeof(my_bind));

  my_bind[0].buffer_type = MYSQL_TYPE_STRING;
  my_bind[0].buffer = szData;
  my_bind[0].buffer_length = (ulong)sizeof(szData);
  my_bind[0].length = &length[0];
  length[0] = sprintf(szData, "updated-data");

  my_bind[1].buffer = (void *)&nData;
  my_bind[1].buffer_type = MYSQL_TYPE_LONG;

  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);
  verify_affected_rows(1);

  mysql_stmt_close(stmt);

  /* now fetch the results ..*/
  rc = mysql_commit(mysql);
  myquery(rc);

  /* test the results now, only one row should exist */
  rc = mysql_query(mysql, "SELECT * FROM test_update");
  myquery(rc);

  /* get the result */
  result = mysql_store_result(mysql);
  mytest(result);

  rc = my_process_result_set(result);
  DIE_UNLESS(rc == 1);
  mysql_free_result(result);
}

/* Test prepare without parameters */

static void test_prepare_noparam() {
  MYSQL_STMT *stmt;
  int rc;
  MYSQL_RES *result;
  char query[MAX_TEST_QUERY_LENGTH];

  myheader("test_prepare_noparam");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS my_prepare");
  myquery(rc);

  rc =
      mysql_query(mysql, "CREATE TABLE my_prepare(col1 int, col2 varchar(50))");
  myquery(rc);

  /* insert by prepare */
  my_stpcpy(query, "INSERT INTO my_prepare VALUES(10, 'venu')");
  stmt = mysql_simple_prepare(mysql, query);
  check_stmt(stmt);

  verify_param_count(stmt, 0);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  mysql_stmt_close(stmt);

  /* now fetch the results ..*/
  rc = mysql_commit(mysql);
  myquery(rc);

  /* test the results now, only one row should exist */
  rc = mysql_query(mysql, "SELECT * FROM my_prepare");
  myquery(rc);

  /* get the result */
  result = mysql_store_result(mysql);
  mytest(result);

  rc = my_process_result_set(result);
  DIE_UNLESS(rc == 1);
  mysql_free_result(result);
}

/**
  C++ wrapper for MYSQL_TIME. We need to zero out the bind structure because
  mysql_stmt_bind_param() checks all its members.
*/
struct MysqlBind : public MYSQL_BIND {
  MysqlBind()
      : MYSQL_BIND{nullptr, nullptr,
                   nullptr, nullptr,
                   nullptr, nullptr,
                   nullptr, nullptr,
                   0,       0,
                   0,       0,
                   0,       MYSQL_TYPE_DECIMAL,
                   false,   false,
                   false,   false,
                   nullptr} {}
};

/**
  Prepares a statement and executes it with the parameter.
  @param ps The statement to prepare.
  @param mt The parameter, passed by-value since we need a non-const pointer to
  it.
*/
static void prepare_and_execute(const char *ps, MYSQL_TIME mt) {
  MYSQL_STMT *stmt = mysql_simple_prepare(mysql, ps);
  check_stmt(stmt);

  ulong length = 30;
  bool is_null = false;

  MysqlBind my_bind;
  my_bind.buffer_type = MYSQL_TYPE_TIMESTAMP;
  my_bind.buffer = &mt;
  my_bind.is_null = &is_null;
  my_bind.length = &length;
  my_bind.buffer_length = 30;

  int rc = mysql_stmt_bind_param(stmt, &my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = mysql_commit(mysql);
  myquery(rc);

  mysql_stmt_close(stmt);
}

static void test_time_zone() {
  myheader("test_time_zone");

  myquery(mysql_query(mysql, "SET time_zone = '+00:00';"));
  myquery(mysql_query(mysql, "CREATE TABLE ttz ( a TIMESTAMP )"));
  myquery(mysql_query(mysql, "CREATE TABLE tdt ( a DATETIME )"));

  MYSQL_TIME mt{2011,
                02,
                03,
                04,
                05,
                06,
                123456,
                false,
                MYSQL_TIMESTAMP_DATETIME_TZ,
                12 * SECS_PER_HOUR + 34 * SECS_PER_MIN};

  prepare_and_execute("INSERT INTO ttz ( a ) VALUES ( ? )", mt);
  prepare_and_execute("INSERT INTO tdt ( a ) VALUES ( ? )", mt);

  verify_col_data("ttz", "a", "2011-02-02 15:31:06");
  verify_col_data("tdt", "a", "2011-02-02 15:31:06");
}

/* Test simple bind result */

static void test_bind_result() {
  MYSQL_STMT *stmt;
  int rc;
  int nData;
  ulong length1;
  char szData[100];
  MYSQL_BIND my_bind[2];
  bool is_null[2];

  myheader("test_bind_result");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_bind_result");
  myquery(rc);

  rc = mysql_query(
      mysql, "CREATE TABLE test_bind_result(col1 int , col2 varchar(50))");
  myquery(rc);

  rc = mysql_query(mysql, "INSERT INTO test_bind_result VALUES(10, 'venu')");
  myquery(rc);

  rc = mysql_query(mysql, "INSERT INTO test_bind_result VALUES(20, 'MySQL')");
  myquery(rc);

  rc = mysql_query(mysql, "INSERT INTO test_bind_result(col2) VALUES('monty')");
  myquery(rc);

  rc = mysql_commit(mysql);
  myquery(rc);

  /* fetch */

  memset(my_bind, 0, sizeof(my_bind));
  my_bind[0].buffer_type = MYSQL_TYPE_LONG;
  my_bind[0].buffer = (void *)&nData; /* integer data */
  my_bind[0].is_null = &is_null[0];

  my_bind[1].buffer_type = MYSQL_TYPE_STRING;
  my_bind[1].buffer = szData; /* string data */
  my_bind[1].buffer_length = (ulong)sizeof(szData);
  my_bind[1].length = &length1;
  my_bind[1].is_null = &is_null[1];

  stmt = mysql_simple_prepare(mysql, "SELECT * FROM test_bind_result");
  check_stmt(stmt);

  rc = mysql_stmt_bind_result(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  if (!opt_silent)
    fprintf(stdout, "\n row 1: %d, %s(%lu)", nData, szData, length1);
  DIE_UNLESS(nData == 10);
  DIE_UNLESS(strcmp(szData, "venu") == 0);
  DIE_UNLESS(length1 == 4);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  if (!opt_silent)
    fprintf(stdout, "\n row 2: %d, %s(%lu)", nData, szData, length1);
  DIE_UNLESS(nData == 20);
  DIE_UNLESS(strcmp(szData, "MySQL") == 0);
  DIE_UNLESS(length1 == 5);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  if (!opt_silent && is_null[0])
    fprintf(stdout, "\n row 3: NULL, %s(%lu)", szData, length1);
  DIE_UNLESS(is_null[0]);
  DIE_UNLESS(strcmp(szData, "monty") == 0);
  DIE_UNLESS(length1 == 5);

  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == MYSQL_NO_DATA);

  mysql_stmt_close(stmt);
}

/* Test ext bind result */

static void test_bind_result_ext() {
  MYSQL_STMT *stmt;
  int rc, i;
  uchar t_data;
  short s_data;
  int i_data;
  longlong b_data;
  float f_data;
  double d_data;
  char szData[20], bData[20];
  ulong szLength, bLength;
  MYSQL_BIND my_bind[8];
  ulong length[8];
  bool is_null[8];
  char llbuf[22];
  myheader("test_bind_result_ext");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_bind_result");
  myquery(rc);

  rc = mysql_query(mysql,
                   "CREATE TABLE test_bind_result(c1 tinyint, "
                   " c2 smallint, "
                   " c3 int, c4 bigint, "
                   " c5 float, c6 double, "
                   " c7 varbinary(10), "
                   " c8 varchar(50))");
  myquery(rc);

  rc = mysql_query(mysql,
                   "INSERT INTO test_bind_result "
                   "VALUES (19, 2999, 3999, 4999999, "
                   " 2345.6, 5678.89563, 'venu', 'mysql')");
  myquery(rc);

  rc = mysql_commit(mysql);
  myquery(rc);

  memset(my_bind, 0, sizeof(my_bind));
  for (i = 0; i < (int)array_elements(my_bind); i++) {
    my_bind[i].length = &length[i];
    my_bind[i].is_null = &is_null[i];
  }

  my_bind[0].buffer_type = MYSQL_TYPE_TINY;
  my_bind[0].buffer = (void *)&t_data;

  my_bind[1].buffer_type = MYSQL_TYPE_SHORT;
  my_bind[2].buffer_type = MYSQL_TYPE_LONG;

  my_bind[3].buffer_type = MYSQL_TYPE_LONGLONG;
  my_bind[1].buffer = (void *)&s_data;

  my_bind[2].buffer = (void *)&i_data;
  my_bind[3].buffer = (void *)&b_data;

  my_bind[4].buffer_type = MYSQL_TYPE_FLOAT;
  my_bind[4].buffer = (void *)&f_data;

  my_bind[5].buffer_type = MYSQL_TYPE_DOUBLE;
  my_bind[5].buffer = (void *)&d_data;

  my_bind[6].buffer_type = MYSQL_TYPE_STRING;
  my_bind[6].buffer = (void *)szData;
  my_bind[6].buffer_length = (ulong)sizeof(szData);
  my_bind[6].length = &szLength;

  my_bind[7].buffer_type = MYSQL_TYPE_TINY_BLOB;
  my_bind[7].buffer = (void *)&bData;
  my_bind[7].length = &bLength;
  my_bind[7].buffer_length = (ulong)sizeof(bData);

  stmt = mysql_simple_prepare(mysql, "select * from test_bind_result");
  check_stmt(stmt);

  rc = mysql_stmt_bind_result(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  if (!opt_silent) {
    fprintf(stdout, "\n data (tiny)   : %d", t_data);
    fprintf(stdout, "\n data (short)  : %d", s_data);
    fprintf(stdout, "\n data (int)    : %d", i_data);
    fprintf(stdout, "\n data (big)    : %s", llstr(b_data, llbuf));

    fprintf(stdout, "\n data (float)  : %f", f_data);
    fprintf(stdout, "\n data (double) : %f", d_data);

    fprintf(stdout, "\n data (str)    : %s(%lu)", szData, szLength);

    bData[bLength] = '\0'; /* bData is binary */
    fprintf(stdout, "\n data (bin)    : %s(%lu)", bData, bLength);
  }

  DIE_UNLESS(t_data == 19);
  DIE_UNLESS(s_data == 2999);
  DIE_UNLESS(i_data == 3999);
  DIE_UNLESS(b_data == 4999999);
  /*DIE_UNLESS(f_data == 2345.60);*/
  /*DIE_UNLESS(d_data == 5678.89563);*/
  DIE_UNLESS(strcmp(szData, "venu") == 0);
  DIE_UNLESS(strncmp(bData, "mysql", 5) == 0);
  DIE_UNLESS(szLength == 4);
  DIE_UNLESS(bLength == 5);

  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == MYSQL_NO_DATA);

  mysql_stmt_close(stmt);
}

/* Test ext bind result */

static void test_bind_result_ext1() {
  MYSQL_STMT *stmt;
  uint i;
  int rc;
  char t_data[20];
  float s_data;
  short i_data;
  uchar b_data;
  int f_data;
  long bData = 0;
  char d_data[20];
  double szData;
  MYSQL_BIND my_bind[8];
  ulong length[8];
  bool is_null[8];
  myheader("test_bind_result_ext1");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_bind_result");
  myquery(rc);

  rc = mysql_query(mysql,
                   "CREATE TABLE test_bind_result(c1 tinyint, c2 smallint, \
                                                        c3 int, c4 bigint, \
                                                        c5 float, c6 double, \
                                                        c7 varbinary(10), \
                                                        c8 varchar(10))");
  myquery(rc);

  rc = mysql_query(mysql,
                   "INSERT INTO test_bind_result VALUES(120, 2999, 3999, 54, \
                                                              2.6, 58.89, \
                                                              '206', '6.7')");
  myquery(rc);

  rc = mysql_commit(mysql);
  myquery(rc);

  memset(my_bind, 0, sizeof(my_bind));
  my_bind[0].buffer_type = MYSQL_TYPE_STRING;
  my_bind[0].buffer = (void *)t_data;
  my_bind[0].buffer_length = (ulong)sizeof(t_data);
  my_bind[0].error = &my_bind[0].error_value;

  my_bind[1].buffer_type = MYSQL_TYPE_FLOAT;
  my_bind[1].buffer = (void *)&s_data;
  my_bind[1].buffer_length = 0;
  my_bind[1].error = &my_bind[1].error_value;

  my_bind[2].buffer_type = MYSQL_TYPE_SHORT;
  my_bind[2].buffer = (void *)&i_data;
  my_bind[2].buffer_length = 0;
  my_bind[2].error = &my_bind[2].error_value;

  my_bind[3].buffer_type = MYSQL_TYPE_TINY;
  my_bind[3].buffer = (void *)&b_data;
  my_bind[3].buffer_length = 0;
  my_bind[3].error = &my_bind[3].error_value;

  my_bind[4].buffer_type = MYSQL_TYPE_LONG;
  my_bind[4].buffer = (void *)&f_data;
  my_bind[4].buffer_length = 0;
  my_bind[4].error = &my_bind[4].error_value;

  my_bind[5].buffer_type = MYSQL_TYPE_STRING;
  my_bind[5].buffer = (void *)d_data;
  my_bind[5].buffer_length = (ulong)sizeof(d_data);
  my_bind[5].error = &my_bind[5].error_value;

  my_bind[6].buffer_type = MYSQL_TYPE_LONG;
  my_bind[6].buffer = (void *)&bData;
  my_bind[6].buffer_length = 0;
  my_bind[6].error = &my_bind[6].error_value;

  my_bind[7].buffer_type = MYSQL_TYPE_DOUBLE;
  my_bind[7].buffer = (void *)&szData;
  my_bind[7].buffer_length = 0;
  my_bind[7].error = &my_bind[7].error_value;

  for (i = 0; i < array_elements(my_bind); i++) {
    my_bind[i].is_null = &is_null[i];
    my_bind[i].length = &length[i];
  }

  stmt = mysql_simple_prepare(mysql, "select * from test_bind_result");
  check_stmt(stmt);

  rc = mysql_stmt_bind_result(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);
  printf("rc=%d\n", rc);
  DIE_UNLESS(rc == 0);

  if (!opt_silent) {
    fprintf(stdout, "\n data (tiny)   : %s(%lu)", t_data, length[0]);
    fprintf(stdout, "\n data (short)  : %f(%lu)", s_data, length[1]);
    fprintf(stdout, "\n data (int)    : %d(%lu)", i_data, length[2]);
    fprintf(stdout, "\n data (big)    : %d(%lu)", b_data, length[3]);

    fprintf(stdout, "\n data (float)  : %d(%lu)", f_data, length[4]);
    fprintf(stdout, "\n data (double) : %s(%lu)", d_data, length[5]);

    fprintf(stdout, "\n data (bin)    : %ld(%lu)", bData, length[6]);
    fprintf(stdout, "\n data (str)    : %g(%lu)", szData, length[7]);
  }

  DIE_UNLESS(strcmp(t_data, "120") == 0);
  DIE_UNLESS(i_data == 3999);
  DIE_UNLESS(f_data == 2);
  DIE_UNLESS(strcmp(d_data, "58.89") == 0);
  DIE_UNLESS(b_data == 54);

  DIE_UNLESS(length[0] == 3);
  DIE_UNLESS(length[1] == 4);
  DIE_UNLESS(length[2] == 2);
  DIE_UNLESS(length[3] == 1);
  DIE_UNLESS(length[4] == 4);
  DIE_UNLESS(length[5] == 5);
  DIE_UNLESS(length[6] == 4);
  DIE_UNLESS(length[7] == 8);

  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == MYSQL_NO_DATA);

  mysql_stmt_close(stmt);
}

/* Generalized fetch conversion routine for all basic types */

static void bind_fetch(int row_count) {
  MYSQL_STMT *stmt;
  int rc, i, count = row_count;
  int32 data[10];
  int8 i8_data;
  int16 i16_data;
  int32 i32_data;
  longlong i64_data;
  float f_data;
  double d_data;
  char s_data[10];
  ulong length[10];
  MYSQL_BIND my_bind[7];
  bool is_null[7];

  stmt = mysql_simple_prepare(mysql,
                              "INSERT INTO test_bind_fetch VALUES "
                              "(?, ?, ?, ?, ?, ?, ?)");
  check_stmt(stmt);

  verify_param_count(stmt, 7);

  /* Always memset all members of bind parameter */
  memset(my_bind, 0, sizeof(my_bind));

  for (i = 0; i < (int)array_elements(my_bind); i++) {
    my_bind[i].buffer_type = MYSQL_TYPE_LONG;
    my_bind[i].buffer = (void *)&data[i];
  }
  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  while (count--) {
    rc = 10 + count;
    for (i = 0; i < (int)array_elements(my_bind); i++) {
      data[i] = rc + i;
      rc += 12;
    }
    rc = mysql_stmt_execute(stmt);
    check_execute(stmt, rc);
  }

  rc = mysql_commit(mysql);
  myquery(rc);

  mysql_stmt_close(stmt);

  rc = my_stmt_result("SELECT * FROM test_bind_fetch");
  DIE_UNLESS(row_count == rc);

  stmt = mysql_simple_prepare(mysql, "SELECT * FROM test_bind_fetch");
  check_stmt(stmt);

  for (i = 0; i < (int)array_elements(my_bind); i++) {
    my_bind[i].buffer = (void *)&data[i];
    my_bind[i].length = &length[i];
    my_bind[i].is_null = &is_null[i];
  }

  my_bind[0].buffer_type = MYSQL_TYPE_TINY;
  my_bind[0].buffer = (void *)&i8_data;

  my_bind[1].buffer_type = MYSQL_TYPE_SHORT;
  my_bind[1].buffer = (void *)&i16_data;

  my_bind[2].buffer_type = MYSQL_TYPE_LONG;
  my_bind[2].buffer = (void *)&i32_data;

  my_bind[3].buffer_type = MYSQL_TYPE_LONGLONG;
  my_bind[3].buffer = (void *)&i64_data;

  my_bind[4].buffer_type = MYSQL_TYPE_FLOAT;
  my_bind[4].buffer = (void *)&f_data;

  my_bind[5].buffer_type = MYSQL_TYPE_DOUBLE;
  my_bind[5].buffer = (void *)&d_data;

  my_bind[6].buffer_type = MYSQL_TYPE_STRING;
  my_bind[6].buffer = (void *)&s_data;
  my_bind[6].buffer_length = (ulong)sizeof(s_data);

  rc = mysql_stmt_bind_result(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_store_result(stmt);
  check_execute(stmt, rc);

  while (row_count--) {
    rc = mysql_stmt_fetch(stmt);
    check_execute(stmt, rc);

    if (!opt_silent) {
      fprintf(stdout, "\n");
      fprintf(stdout, "\n tiny     : %ld(%lu)", (ulong)i8_data, length[0]);
      fprintf(stdout, "\n short    : %ld(%lu)", (ulong)i16_data, length[1]);
      fprintf(stdout, "\n int      : %ld(%lu)", (ulong)i32_data, length[2]);
      fprintf(stdout, "\n longlong : %ld(%lu)", (ulong)i64_data, length[3]);
      fprintf(stdout, "\n float    : %f(%lu)", f_data, length[4]);
      fprintf(stdout, "\n double   : %g(%lu)", d_data, length[5]);
      fprintf(stdout, "\n char     : %s(%lu)", s_data, length[6]);
    }
    rc = 10 + row_count;

    /* TINY */
    DIE_UNLESS((int)i8_data == rc);
    DIE_UNLESS(length[0] == 1);
    rc += 13;

    /* SHORT */
    DIE_UNLESS((int)i16_data == rc);
    DIE_UNLESS(length[1] == 2);
    rc += 13;

    /* LONG */
    DIE_UNLESS((int)i32_data == rc);
    DIE_UNLESS(length[2] == 4);
    rc += 13;

    /* LONGLONG */
    DIE_UNLESS((int)i64_data == rc);
    DIE_UNLESS(length[3] == 8);
    rc += 13;

    /* FLOAT */
    DIE_UNLESS((int)f_data == rc);
    DIE_UNLESS(length[4] == 4);
    rc += 13;

    /* DOUBLE */
    DIE_UNLESS((int)d_data == rc);
    DIE_UNLESS(length[5] == 8);
    rc += 13;

    /* CHAR */
    {
      char buff[20];
      long len = sprintf(buff, "%d", rc);
      DIE_UNLESS(strcmp(s_data, buff) == 0);
      DIE_UNLESS(length[6] == (ulong)len);
    }
  }
  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == MYSQL_NO_DATA);

  mysql_stmt_close(stmt);
}

/* Test fetching of date, time and ts */

static void test_fetch_date() {
  MYSQL_STMT *stmt;
  uint i;
  int rc, year;
  char date[25], my_time[25], ts[25], ts_4[25], ts_6[20], dt[20];
  ulong d_length, t_length, ts_length, ts4_length, ts6_length, dt_length,
      y_length;
  MYSQL_BIND my_bind[8];
  bool is_null[8];
  ulong length[8];

  myheader("test_fetch_date");

  /* Will not work if sql_mode is STRICT (implicit if TRADITIONAL) */
  rc = mysql_query(mysql, "SET SQL_MODE=''");
  myquery(rc);

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_bind_result");
  myquery(rc);

  rc = mysql_query(mysql,
                   "CREATE TABLE test_bind_result(c1 date, c2 time, \
                                                        c3 timestamp, \
                                                        c4 year, \
                                                        c5 datetime, \
                                                        c6 timestamp, \
                                                        c7 timestamp)");
  myquery(rc);

  rc = mysql_query(mysql, "SET SQL_MODE=''");
  rc = mysql_query(mysql,
                   "INSERT INTO test_bind_result VALUES('2002-01-02', \
                                                              '12:49:00', \
                                                              '2002-01-02 17:46:59', \
                                                              2010, \
                                                              '2010-07-10', \
                                                              '2020', '1999-12-29')");
  myquery(rc);

  rc = mysql_commit(mysql);
  myquery(rc);

  memset(my_bind, 0, sizeof(my_bind));
  for (i = 0; i < array_elements(my_bind); i++) {
    my_bind[i].is_null = &is_null[i];
    my_bind[i].length = &length[i];
  }

  my_bind[0].buffer_type = MYSQL_TYPE_STRING;
  my_bind[1] = my_bind[2] = my_bind[0];

  my_bind[0].buffer = (void *)&date;
  my_bind[0].buffer_length = (ulong)sizeof(date);
  my_bind[0].length = &d_length;

  my_bind[1].buffer = (void *)&my_time;
  my_bind[1].buffer_length = (ulong)sizeof(my_time);
  my_bind[1].length = &t_length;

  my_bind[2].buffer = (void *)&ts;
  my_bind[2].buffer_length = (ulong)sizeof(ts);
  my_bind[2].length = &ts_length;

  my_bind[3].buffer_type = MYSQL_TYPE_LONG;
  my_bind[3].buffer = (void *)&year;
  my_bind[3].length = &y_length;

  my_bind[4].buffer_type = MYSQL_TYPE_STRING;
  my_bind[4].buffer = (void *)&dt;
  my_bind[4].buffer_length = (ulong)sizeof(dt);
  my_bind[4].length = &dt_length;

  my_bind[5].buffer_type = MYSQL_TYPE_STRING;
  my_bind[5].buffer = (void *)&ts_4;
  my_bind[5].buffer_length = (ulong)sizeof(ts_4);
  my_bind[5].length = &ts4_length;

  my_bind[6].buffer_type = MYSQL_TYPE_STRING;
  my_bind[6].buffer = (void *)&ts_6;
  my_bind[6].buffer_length = (ulong)sizeof(ts_6);
  my_bind[6].length = &ts6_length;

  rc = my_stmt_result("SELECT * FROM test_bind_result");
  DIE_UNLESS(rc == 1);

  stmt = mysql_simple_prepare(mysql, "SELECT * FROM test_bind_result");
  check_stmt(stmt);

  rc = mysql_stmt_bind_result(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  ts_4[0] = '\0';
  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  if (!opt_silent) {
    fprintf(stdout, "\n date   : %s(%lu)", date, d_length);
    fprintf(stdout, "\n time   : %s(%lu)", my_time, t_length);
    fprintf(stdout, "\n ts     : %s(%lu)", ts, ts_length);
    fprintf(stdout, "\n year   : %d(%lu)", year, y_length);
    fprintf(stdout, "\n dt     : %s(%lu)", dt, dt_length);
    fprintf(stdout, "\n ts(4)  : %s(%lu)", ts_4, ts4_length);
    fprintf(stdout, "\n ts(6)  : %s(%lu)", ts_6, ts6_length);
  }

  DIE_UNLESS(strcmp(date, "2002-01-02") == 0);
  DIE_UNLESS(d_length == 10);

  DIE_UNLESS(strcmp(my_time, "12:49:00") == 0);
  DIE_UNLESS(t_length == 8);

  DIE_UNLESS(strcmp(ts, "2002-01-02 17:46:59") == 0);
  DIE_UNLESS(ts_length == 19);

  DIE_UNLESS(year == 2010);
  DIE_UNLESS(y_length == 4);

  DIE_UNLESS(strcmp(dt, "2010-07-10 00:00:00") == 0);
  DIE_UNLESS(dt_length == 19);

  DIE_UNLESS(strcmp(ts_4, "0000-00-00 00:00:00") == 0);
  DIE_UNLESS(ts4_length == strlen("0000-00-00 00:00:00"));

  DIE_UNLESS(strcmp(ts_6, "1999-12-29 00:00:00") == 0);
  DIE_UNLESS(ts6_length == 19);

  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == MYSQL_NO_DATA);

  mysql_stmt_close(stmt);
}

/* Test fetching of str to all types */

static void test_fetch_str() {
  int rc;

  myheader("test_fetch_str");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_bind_fetch");
  myquery(rc);

  rc = mysql_query(mysql,
                   "CREATE TABLE test_bind_fetch(c1 char(10), \
                                                     c2 char(10), \
                                                     c3 char(20), \
                                                     c4 char(20), \
                                                     c5 char(30), \
                                                     c6 char(40), \
                                                     c7 char(20))");
  myquery(rc);

  bind_fetch(3);
}

/* Test fetching of long to all types */

static void test_fetch_long() {
  int rc;

  myheader("test_fetch_long");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_bind_fetch");
  myquery(rc);

  rc = mysql_query(mysql,
                   "CREATE TABLE test_bind_fetch(c1 int unsigned, \
                                                     c2 int unsigned, \
                                                     c3 int, \
                                                     c4 int, \
                                                     c5 int, \
                                                     c6 int unsigned, \
                                                     c7 int)");
  myquery(rc);

  bind_fetch(4);
}

/* Test fetching of short to all types */

static void test_fetch_short() {
  int rc;

  myheader("test_fetch_short");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_bind_fetch");
  myquery(rc);

  rc = mysql_query(mysql,
                   "CREATE TABLE test_bind_fetch(c1 smallint unsigned, \
                                                     c2 smallint, \
                                                     c3 smallint unsigned, \
                                                     c4 smallint, \
                                                     c5 smallint, \
                                                     c6 smallint, \
                                                     c7 smallint unsigned)");
  myquery(rc);

  bind_fetch(5);
}

/* Test fetching of tiny to all types */

static void test_fetch_tiny() {
  int rc;

  myheader("test_fetch_tiny");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_bind_fetch");
  myquery(rc);

  rc = mysql_query(mysql,
                   "CREATE TABLE test_bind_fetch(c1 tinyint unsigned, \
                                                     c2 tinyint, \
                                                     c3 tinyint unsigned, \
                                                     c4 tinyint, \
                                                     c5 tinyint, \
                                                     c6 tinyint, \
                                                     c7 tinyint unsigned)");
  myquery(rc);

  bind_fetch(3);
}

/* Test fetching of longlong to all types */

static void test_fetch_bigint() {
  int rc;

  myheader("test_fetch_bigint");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_bind_fetch");
  myquery(rc);

  rc = mysql_query(mysql,
                   "CREATE TABLE test_bind_fetch(c1 bigint, \
                                                     c2 bigint, \
                                                     c3 bigint unsigned, \
                                                     c4 bigint unsigned, \
                                                     c5 bigint unsigned, \
                                                     c6 bigint unsigned, \
                                                     c7 bigint unsigned)");
  myquery(rc);

  bind_fetch(2);
}

/* Test fetching of float to all types */

static void test_fetch_float() {
  int rc;

  myheader("test_fetch_float");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_bind_fetch");
  myquery(rc);

  rc = mysql_query(mysql,
                   "CREATE TABLE test_bind_fetch(c1 float(3), \
                                                     c2 float, \
                                                     c3 float unsigned, \
                                                     c4 float, \
                                                     c5 float, \
                                                     c6 float, \
                                                     c7 float(10) unsigned)");
  myquery(rc);

  bind_fetch(2);
}

/* Test fetching of double to all types */

static void test_fetch_double() {
  int rc;

  myheader("test_fetch_double");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_bind_fetch");
  myquery(rc);

  rc = mysql_query(mysql,
                   "CREATE TABLE test_bind_fetch(c1 double(5, 2), "
                   "c2 double unsigned, c3 double unsigned, "
                   "c4 double unsigned, c5 double unsigned, "
                   "c6 double unsigned, c7 double unsigned)");
  myquery(rc);

  bind_fetch(3);
}

/* Test simple prepare with all possible types */

static void test_prepare_ext() {
  MYSQL_STMT *stmt;
  int rc;
  const char *sql;
  int nData = 1;
  char tData = 1;
  short sData = 10;
  longlong bData = 20;
  MYSQL_BIND my_bind[6];
  char query[MAX_TEST_QUERY_LENGTH];
  myheader("test_prepare_ext");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_prepare_ext");
  myquery(rc);

  sql =
      "CREATE TABLE test_prepare_ext"
      "("
      " c1  tinyint,"
      " c2  smallint,"
      " c3  mediumint,"
      " c4  int,"
      " c5  integer,"
      " c6  bigint,"
      " c7  float,"
      " c8  double,"
      " c9  double precision,"
      " c10 real,"
      " c11 decimal(7, 4),"
      " c12 numeric(8, 4),"
      " c13 date,"
      " c14 datetime,"
      " c15 timestamp,"
      " c16 time,"
      " c17 year,"
      " c18 bit,"
      " c19 bool,"
      " c20 char,"
      " c21 char(10),"
      " c22 varchar(30),"
      " c23 tinyblob,"
      " c24 tinytext,"
      " c25 blob,"
      " c26 text,"
      " c27 mediumblob,"
      " c28 mediumtext,"
      " c29 longblob,"
      " c30 longtext,"
      " c31 enum('one', 'two', 'three'),"
      " c32 set('monday', 'tuesday', 'wednesday'))";

  rc = mysql_query(mysql, sql);
  myquery(rc);

  /* insert by prepare - all integers */
  my_stpcpy(query,
            "INSERT INTO test_prepare_ext(c1, c2, c3, c4, c5, c6) VALUES(?, ?, "
            "?, ?, ?, ?)");
  stmt = mysql_simple_prepare(mysql, query);
  check_stmt(stmt);

  verify_param_count(stmt, 6);

  /* Always memset all members of bind parameter */
  memset(my_bind, 0, sizeof(my_bind));

  /*tinyint*/
  my_bind[0].buffer_type = MYSQL_TYPE_TINY;
  my_bind[0].buffer = (void *)&tData;

  /*smallint*/
  my_bind[1].buffer_type = MYSQL_TYPE_SHORT;
  my_bind[1].buffer = (void *)&sData;

  /*mediumint*/
  my_bind[2].buffer_type = MYSQL_TYPE_LONG;
  my_bind[2].buffer = (void *)&nData;

  /*int*/
  my_bind[3].buffer_type = MYSQL_TYPE_LONG;
  my_bind[3].buffer = (void *)&nData;

  /*integer*/
  my_bind[4].buffer_type = MYSQL_TYPE_LONG;
  my_bind[4].buffer = (void *)&nData;

  /*bigint*/
  my_bind[5].buffer_type = MYSQL_TYPE_LONGLONG;
  my_bind[5].buffer = (void *)&bData;

  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  /*
   *  integer to integer
   */
  for (nData = 0; nData < 10; nData++, tData++, sData++, bData++) {
    rc = mysql_stmt_execute(stmt);
    check_execute(stmt, rc);
  }
  mysql_stmt_close(stmt);

  /* now fetch the results ..*/

  stmt = mysql_simple_prepare(mysql,
                              "SELECT c1, c2, c3, c4, c5, c6 "
                              "FROM test_prepare_ext");
  check_stmt(stmt);

  /* get the result */
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = my_process_stmt_result(stmt);
  DIE_UNLESS(nData == rc);

  mysql_stmt_close(stmt);
}

/* Test real and alias names */

static void test_field_names() {
  int rc;
  MYSQL_RES *result;

  myheader("test_field_names");

  if (!opt_silent)
    fprintf(stdout, "\n %d, %d, %d", MYSQL_TYPE_DECIMAL, MYSQL_TYPE_NEWDATE,
            MYSQL_TYPE_ENUM);
  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_field_names1");
  myquery(rc);

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_field_names2");
  myquery(rc);

  rc = mysql_query(mysql,
                   "CREATE TABLE test_field_names1(id int, name varchar(50))");
  myquery(rc);

  rc = mysql_query(mysql,
                   "CREATE TABLE test_field_names2(id int, name varchar(50))");
  myquery(rc);

  /* with table name included with TRUE column name */
  rc = mysql_query(mysql, "SELECT id as 'id-alias' FROM test_field_names1");
  myquery(rc);

  result = mysql_use_result(mysql);
  mytest(result);

  rc = my_process_result_set(result);
  DIE_UNLESS(rc == 0);
  mysql_free_result(result);

  /* with table name included with TRUE column name */
  rc = mysql_query(mysql,
                   "SELECT t1.id as 'id-alias', test_field_names2.name FROM "
                   "test_field_names1 t1, test_field_names2");
  myquery(rc);

  result = mysql_use_result(mysql);
  mytest(result);

  rc = my_process_result_set(result);
  DIE_UNLESS(rc == 0);
  mysql_free_result(result);
}

/* Test warnings */

static void test_warnings() {
  int rc;
  MYSQL_RES *result;

  myheader("test_warnings");

  mysql_query(mysql, "DROP TABLE if exists test_non_exists");

  rc = mysql_query(mysql, "DROP TABLE if exists test_non_exists");
  myquery(rc);

  if (!opt_silent)
    fprintf(stdout, "\n total warnings: %d", mysql_warning_count(mysql));
  rc = mysql_query(mysql, "SHOW WARNINGS");
  myquery(rc);

  result = mysql_store_result(mysql);
  mytest(result);

  rc = my_process_result_set(result);
  DIE_UNLESS(rc == 1);
  mysql_free_result(result);
}

/* Test errors */

static void test_errors() {
  int rc;
  MYSQL_RES *result;

  myheader("test_errors");

  mysql_query(mysql, "DROP TABLE if exists test_non_exists");

  rc = mysql_query(mysql, "DROP TABLE test_non_exists");
  myquery_r(rc);

  rc = mysql_query(mysql, "SHOW ERRORS");
  myquery(rc);

  result = mysql_store_result(mysql);
  mytest(result);

  (void)my_process_result_set(result);
  mysql_free_result(result);
}

/* Test simple prepare-insert */

static void test_insert() {
  MYSQL_STMT *stmt;
  int rc;
  char str_data[50];
  char tiny_data;
  MYSQL_RES *result;
  MYSQL_BIND my_bind[2];
  ulong length;

  myheader("test_insert");

  rc = mysql_autocommit(mysql, true);
  myquery(rc);

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_prep_insert");
  myquery(rc);

  rc = mysql_query(mysql,
                   "CREATE TABLE test_prep_insert(col1 tinyint, \
                                col2 varchar(50))");
  myquery(rc);

  /* insert by prepare */
  stmt =
      mysql_simple_prepare(mysql, "INSERT INTO test_prep_insert VALUES(?, ?)");
  check_stmt(stmt);

  verify_param_count(stmt, 2);

  /*
    We need to memset bind structure because mysql_stmt_bind_param checks all
    its members.
  */
  memset(my_bind, 0, sizeof(my_bind));

  /* tinyint */
  my_bind[0].buffer_type = MYSQL_TYPE_TINY;
  my_bind[0].buffer = (void *)&tiny_data;

  /* string */
  my_bind[1].buffer_type = MYSQL_TYPE_STRING;
  my_bind[1].buffer = str_data;
  my_bind[1].buffer_length = (ulong)sizeof(str_data);
  my_bind[1].length = &length;

  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  /* now, execute the prepared statement to insert 10 records.. */
  for (tiny_data = 0; tiny_data < 3; tiny_data++) {
    length = sprintf(str_data, "MySQL%d", tiny_data);
    rc = mysql_stmt_execute(stmt);
    check_execute(stmt, rc);
  }

  mysql_stmt_close(stmt);

  /* now fetch the results ..*/
  rc = mysql_commit(mysql);
  myquery(rc);

  /* test the results now, only one row should exist */
  rc = mysql_query(mysql, "SELECT * FROM test_prep_insert");
  myquery(rc);

  /* get the result */
  result = mysql_store_result(mysql);
  mytest(result);

  rc = my_process_result_set(result);
  DIE_UNLESS((int)tiny_data == rc);
  mysql_free_result(result);
}

/* Test simple prepare-resultset info */

static void test_prepare_resultset() {
  MYSQL_STMT *stmt;
  int rc;
  MYSQL_RES *result;

  myheader("test_prepare_resultset");

  rc = mysql_autocommit(mysql, true);
  myquery(rc);

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_prepare_resultset");
  myquery(rc);

  rc = mysql_query(mysql,
                   "CREATE TABLE test_prepare_resultset(id int, \
                                name varchar(50), extra double)");
  myquery(rc);

  stmt = mysql_simple_prepare(mysql, "SELECT * FROM test_prepare_resultset");
  check_stmt(stmt);

  verify_param_count(stmt, 0);

  result = mysql_stmt_result_metadata(stmt);
  mytest(result);
  my_print_result_metadata(result);
  mysql_free_result(result);
  mysql_stmt_close(stmt);
}

/* Test field flags (verify .NET provider) */

static void test_field_flags() {
  int rc;
  MYSQL_RES *result;
  MYSQL_FIELD *field;
  unsigned int i;

  myheader("test_field_flags");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_field_flags");
  myquery(rc);

  rc = mysql_query(
      mysql,
      "CREATE TABLE test_field_flags(id int NOT NULL AUTO_INCREMENT PRIMARY KEY, \
                                                        id1 int NOT NULL, \
                                                        id2 int UNIQUE, \
                                                        id3 int, \
                                                        id4 int NOT NULL, \
                                                        id5 int, \
                                                        KEY(id3, id4))");
  myquery(rc);

  /* with table name included with TRUE column name */
  rc = mysql_query(mysql, "SELECT * FROM test_field_flags");
  myquery(rc);

  result = mysql_use_result(mysql);
  mytest(result);

  mysql_field_seek(result, 0);
  if (!opt_silent) fputc('\n', stdout);

  for (i = 0; i < mysql_num_fields(result); i++) {
    field = mysql_fetch_field(result);
    if (!opt_silent) {
      fprintf(stdout, "\n field:%d", i);
      if (field->flags & NOT_NULL_FLAG) fprintf(stdout, "\n  NOT_NULL_FLAG");
      if (field->flags & PRI_KEY_FLAG) fprintf(stdout, "\n  PRI_KEY_FLAG");
      if (field->flags & UNIQUE_KEY_FLAG)
        fprintf(stdout, "\n  UNIQUE_KEY_FLAG");
      if (field->flags & MULTIPLE_KEY_FLAG)
        fprintf(stdout, "\n  MULTIPLE_KEY_FLAG");
      if (field->flags & AUTO_INCREMENT_FLAG)
        fprintf(stdout, "\n  AUTO_INCREMENT_FLAG");
    }
  }
  mysql_free_result(result);
}

/* Test mysql_stmt_close for open stmts */

static void test_stmt_close() {
  MYSQL *lmysql;
  MYSQL_STMT *stmt1, *stmt2, *stmt3, *stmt_x;
  MYSQL_BIND my_bind[1];
  MYSQL_RES *result;
  unsigned int count;
  int rc;
  char query[MAX_TEST_QUERY_LENGTH];

  myheader("test_stmt_close");

  if (!opt_silent) fprintf(stdout, "\n Establishing a test connection ...");
  if (!(lmysql = mysql_client_init(nullptr))) {
    myerror("mysql_client_init() failed");
    exit(1);
  }
  if (!(mysql_real_connect(lmysql, opt_host, opt_user, opt_password, current_db,
                           opt_port, opt_unix_socket, 0))) {
    myerror("connection failed");
    exit(1);
  }
  lmysql->reconnect = true;
  if (!opt_silent) fprintf(stdout, "OK");

  /* set AUTOCOMMIT to ON*/
  mysql_autocommit(lmysql, true);

  rc = mysql_query(lmysql, "SET SQL_MODE = ''");
  myquery(rc);

  rc = mysql_query(lmysql, "DROP TABLE IF EXISTS test_stmt_close");
  myquery(rc);

  rc = mysql_query(lmysql, "CREATE TABLE test_stmt_close(id int)");
  myquery(rc);

  my_stpcpy(query, "DO \"nothing\"");
  stmt1 = mysql_simple_prepare(lmysql, query);
  check_stmt(stmt1);

  verify_param_count(stmt1, 0);

  my_stpcpy(query, "INSERT INTO test_stmt_close(id) VALUES(?)");
  stmt_x = mysql_simple_prepare(mysql, query);
  check_stmt(stmt_x);

  verify_param_count(stmt_x, 1);

  my_stpcpy(query, "UPDATE test_stmt_close SET id= ? WHERE id= ?");
  stmt3 = mysql_simple_prepare(lmysql, query);
  check_stmt(stmt3);

  verify_param_count(stmt3, 2);

  my_stpcpy(query, "SELECT * FROM test_stmt_close WHERE id= ?");
  stmt2 = mysql_simple_prepare(lmysql, query);
  check_stmt(stmt2);

  verify_param_count(stmt2, 1);

  rc = mysql_stmt_close(stmt1);
  if (!opt_silent) fprintf(stdout, "\n mysql_close_stmt(1) returned: %d", rc);
  DIE_UNLESS(rc == 0);

  /*
    Originally we were going to close all statements automatically in
    mysql_close(). This proved to not work well - users weren't able to
    close statements by hand once mysql_close() had been called.
    Now mysql_close() doesn't free any statements, so this test doesn't
    serve its original designation any more.
    Here we free stmt2 and stmt3 by hand to avoid memory leaks.
  */
  mysql_stmt_close(stmt2);
  mysql_stmt_close(stmt3);
  mysql_close(lmysql);

  /*
    We need to memset bind structure because mysql_stmt_bind_param checks all
    its members.
  */
  memset(my_bind, 0, sizeof(my_bind));

  my_bind[0].buffer = (void *)&count;
  my_bind[0].buffer_type = MYSQL_TYPE_LONG;
  count = 100;

  rc = mysql_stmt_bind_param(stmt_x, my_bind);
  check_execute(stmt_x, rc);

  rc = mysql_stmt_execute(stmt_x);
  check_execute(stmt_x, rc);

  verify_st_affected_rows(stmt_x, 1);

  rc = mysql_stmt_close(stmt_x);
  if (!opt_silent) fprintf(stdout, "\n mysql_close_stmt(x) returned: %d", rc);
  DIE_UNLESS(rc == 0);

  rc = mysql_query(mysql, "SELECT id FROM test_stmt_close");
  myquery(rc);

  result = mysql_store_result(mysql);
  mytest(result);

  rc = my_process_result_set(result);
  DIE_UNLESS(rc == 1);
  mysql_free_result(result);
}

/* Test simple set variable prepare */

static void test_set_variable() {
  MYSQL_STMT *stmt, *stmt1;
  int rc;
  int set_count, def_count, get_count;
  ulong length;
  char var[NAME_LEN + 1];
  MYSQL_BIND set_bind[1], get_bind[2];

  myheader("test_set_variable");

  mysql_autocommit(mysql, true);

  stmt1 = mysql_simple_prepare(mysql, "show variables like 'max_error_count'");
  check_stmt(stmt1);

  /*
    We need to memset bind structure because mysql_stmt_bind_param checks all
    its members.
  */
  memset(get_bind, 0, sizeof(get_bind));

  get_bind[0].buffer_type = MYSQL_TYPE_STRING;
  get_bind[0].buffer = (void *)var;
  get_bind[0].length = &length;
  get_bind[0].buffer_length = (ulong)NAME_LEN;
  length = NAME_LEN;

  get_bind[1].buffer_type = MYSQL_TYPE_LONG;
  get_bind[1].buffer = (void *)&get_count;

  rc = mysql_stmt_execute(stmt1);
  check_execute(stmt1, rc);

  rc = mysql_stmt_bind_result(stmt1, get_bind);
  check_execute(stmt1, rc);

  rc = mysql_stmt_fetch(stmt1);
  check_execute(stmt1, rc);

  if (!opt_silent)
    fprintf(stdout, "\n max_error_count(default): %d", get_count);
  def_count = get_count;

  DIE_UNLESS(strcmp(var, "max_error_count") == 0);
  rc = mysql_stmt_fetch(stmt1);
  DIE_UNLESS(rc == MYSQL_NO_DATA);

  stmt = mysql_simple_prepare(mysql, "set max_error_count= ?");
  check_stmt(stmt);

  memset(set_bind, 0, sizeof(set_bind));

  set_bind[0].buffer_type = MYSQL_TYPE_LONG;
  set_bind[0].buffer = (void *)&set_count;

  rc = mysql_stmt_bind_param(stmt, set_bind);
  check_execute(stmt, rc);

  set_count = 31;
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  mysql_commit(mysql);

  rc = mysql_stmt_execute(stmt1);
  check_execute(stmt1, rc);

  rc = mysql_stmt_fetch(stmt1);
  check_execute(stmt1, rc);

  if (!opt_silent)
    fprintf(stdout, "\n max_error_count         : %d", get_count);
  DIE_UNLESS(get_count == set_count);

  rc = mysql_stmt_fetch(stmt1);
  DIE_UNLESS(rc == MYSQL_NO_DATA);

  /* restore back to default */
  set_count = def_count;
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt1);
  check_execute(stmt1, rc);

  rc = mysql_stmt_fetch(stmt1);
  check_execute(stmt1, rc);

  if (!opt_silent)
    fprintf(stdout, "\n max_error_count(default): %d", get_count);
  DIE_UNLESS(get_count == set_count);

  rc = mysql_stmt_fetch(stmt1);
  DIE_UNLESS(rc == MYSQL_NO_DATA);

  mysql_stmt_close(stmt);
  mysql_stmt_close(stmt1);
}

/* Test FUNCTION field info / DATE_FORMAT() table_name . */

static void test_func_fields() {
  int rc;
  MYSQL_RES *result;
  MYSQL_FIELD *field;

  myheader("test_func_fields");

  rc = mysql_autocommit(mysql, true);
  myquery(rc);

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_dateformat");
  myquery(rc);

  rc = mysql_query(mysql,
                   "CREATE TABLE test_dateformat(id int, \
                                                       ts timestamp)");
  myquery(rc);

  rc = mysql_query(mysql, "INSERT INTO test_dateformat(id) values(10)");
  myquery(rc);

  rc = mysql_query(mysql, "SELECT ts FROM test_dateformat");
  myquery(rc);

  result = mysql_store_result(mysql);
  mytest(result);

  field = mysql_fetch_field(result);
  mytest(field);
  if (!opt_silent)
    fprintf(stdout, "\n table name: `%s` (expected: `%s`)", field->table,
            "test_dateformat");
  DIE_UNLESS(strcmp(field->table, "test_dateformat") == 0);

  field = mysql_fetch_field(result);
  mytest_r(field); /* no more fields */

  mysql_free_result(result);

  /* DATE_FORMAT */
  rc = mysql_query(
      mysql, "SELECT DATE_FORMAT(ts, '%Y') AS 'venu' FROM test_dateformat");
  myquery(rc);

  result = mysql_store_result(mysql);
  mytest(result);

  field = mysql_fetch_field(result);
  mytest(field);
  if (!opt_silent)
    fprintf(stdout, "\n table name: `%s` (expected: `%s`)", field->table, "");
  DIE_UNLESS(field->table[0] == '\0');

  field = mysql_fetch_field(result);
  mytest_r(field); /* no more fields */

  mysql_free_result(result);

  /* FIELD ALIAS TEST */
  rc = mysql_query(
      mysql, "SELECT DATE_FORMAT(ts, '%Y')  AS 'YEAR' FROM test_dateformat");
  myquery(rc);

  result = mysql_store_result(mysql);
  mytest(result);

  field = mysql_fetch_field(result);
  mytest(field);
  if (!opt_silent) {
    printf("\n field name: `%s` (expected: `%s`)", field->name, "YEAR");
    printf("\n field org name: `%s` (expected: `%s`)", field->org_name, "");
  }
  DIE_UNLESS(strcmp(field->name, "YEAR") == 0);
  DIE_UNLESS(field->org_name[0] == '\0');

  field = mysql_fetch_field(result);
  mytest_r(field); /* no more fields */

  mysql_free_result(result);
}

/* Multiple stmts .. */

static void test_multi_stmt() {
  MYSQL_STMT *stmt, *stmt1, *stmt2;
  int rc;
  uint32 id;
  char name[50];
  MYSQL_BIND my_bind[2];
  ulong length[2];
  bool is_null[2];
  myheader("test_multi_stmt");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_multi_table");
  myquery(rc);

  rc = mysql_query(mysql,
                   "CREATE TABLE test_multi_table(id int, name char(20))");
  myquery(rc);

  rc = mysql_query(mysql, "INSERT INTO test_multi_table values(10, 'mysql')");
  myquery(rc);

  stmt = mysql_simple_prepare(mysql,
                              "SELECT * FROM test_multi_table "
                              "WHERE id= ?");
  check_stmt(stmt);

  stmt2 = mysql_simple_prepare(mysql,
                               "UPDATE test_multi_table "
                               "SET name='updated' WHERE id=10");
  check_stmt(stmt2);

  verify_param_count(stmt, 1);

  /*
    We need to memset bind structure because mysql_stmt_bind_param checks all
    its members.
  */
  memset(my_bind, 0, sizeof(my_bind));

  my_bind[0].buffer_type = MYSQL_TYPE_LONG;
  my_bind[0].buffer = (void *)&id;
  my_bind[0].is_null = &is_null[0];
  my_bind[0].length = &length[0];
  is_null[0] = false;
  length[0] = 0;

  my_bind[1].buffer_type = MYSQL_TYPE_STRING;
  my_bind[1].buffer = (void *)name;
  my_bind[1].buffer_length = (ulong)sizeof(name);
  my_bind[1].length = &length[1];
  my_bind[1].is_null = &is_null[1];

  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_bind_result(stmt, my_bind);
  check_execute(stmt, rc);

  id = 10;
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  id = 999;
  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  if (!opt_silent) {
    fprintf(stdout, "\n int_data: %lu(%lu)", (ulong)id, length[0]);
    fprintf(stdout, "\n str_data: %s(%lu)", name, length[1]);
  }
  DIE_UNLESS(id == 10);
  DIE_UNLESS(strcmp(name, "mysql") == 0);

  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == MYSQL_NO_DATA);

  /* alter the table schema now */
  stmt1 = mysql_simple_prepare(mysql,
                               "DELETE FROM test_multi_table "
                               "WHERE id= ? AND "
                               "CONVERT(name USING utf8)=?");
  check_stmt(stmt1);

  verify_param_count(stmt1, 2);

  rc = mysql_stmt_bind_param(stmt1, my_bind);
  check_execute(stmt1, rc);

  rc = mysql_stmt_execute(stmt2);
  check_execute(stmt2, rc);

  verify_st_affected_rows(stmt2, 1);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  if (!opt_silent) {
    fprintf(stdout, "\n int_data: %lu(%lu)", (ulong)id, length[0]);
    fprintf(stdout, "\n str_data: %s(%lu)", name, length[1]);
  }
  DIE_UNLESS(id == 10);
  DIE_UNLESS(strcmp(name, "updated") == 0);

  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == MYSQL_NO_DATA);

  rc = mysql_stmt_execute(stmt1);
  check_execute(stmt1, rc);

  verify_st_affected_rows(stmt1, 1);

  mysql_stmt_close(stmt1);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == MYSQL_NO_DATA);

  rc = my_stmt_result("SELECT * FROM test_multi_table");
  DIE_UNLESS(rc == 0);

  mysql_stmt_close(stmt);
  mysql_stmt_close(stmt2);
}

/* Test simple sample - manual */

static void test_manual_sample() {
  unsigned int param_count;
  MYSQL_STMT *stmt;
  short small_data;
  int int_data;
  int rc;
  char str_data[50];
  ulonglong affected_rows;
  MYSQL_BIND my_bind[3];
  bool is_null;
  char query[MAX_TEST_QUERY_LENGTH];

  myheader("test_manual_sample");

  /*
    Sample which is incorporated directly in the manual under Prepared
    statements section (Example from mysql_stmt_execute()
  */

  mysql_autocommit(mysql, true);
  if (mysql_query(mysql, "DROP TABLE IF EXISTS test_table")) {
    fprintf(stderr, "\n drop table failed");
    fprintf(stderr, "\n %s", mysql_error(mysql));
    exit(1);
  }
  if (mysql_query(mysql,
                  "CREATE TABLE test_table(col1 int, col2 varchar(50), \
                                                 col3 smallint, \
                                                 col4 timestamp)")) {
    fprintf(stderr, "\n create table failed");
    fprintf(stderr, "\n %s", mysql_error(mysql));
    exit(1);
  }

  /* Prepare a insert query with 3 parameters */
  my_stpcpy(query, "INSERT INTO test_table(col1, col2, col3) values(?, ?, ?)");
  if (!(stmt = mysql_simple_prepare(mysql, query))) {
    fprintf(stderr, "\n prepare, insert failed");
    fprintf(stderr, "\n %s", mysql_error(mysql));
    exit(1);
  }
  if (!opt_silent) fprintf(stdout, "\n prepare, insert successful");

  /* Get the parameter count from the statement */
  param_count = mysql_stmt_param_count(stmt);

  if (!opt_silent)
    fprintf(stdout, "\n total parameters in insert: %d", param_count);
  if (param_count != 3) /* validate parameter count */
  {
    fprintf(stderr, "\n invalid parameter count returned by MySQL");
    exit(1);
  }

  /* Bind the data for the parameters */

  /*
    We need to memset bind structure because mysql_stmt_bind_param checks all
    its members.
  */
  memset(my_bind, 0, sizeof(my_bind));
  memset(str_data, 0, sizeof(str_data));

  /* INTEGER PART */
  my_bind[0].buffer_type = MYSQL_TYPE_LONG;
  my_bind[0].buffer = (void *)&int_data;

  /* STRING PART */
  my_bind[1].buffer_type = MYSQL_TYPE_VAR_STRING;
  my_bind[1].buffer = (void *)str_data;
  my_bind[1].buffer_length = (ulong)sizeof(str_data);

  /* SMALLINT PART */
  my_bind[2].buffer_type = MYSQL_TYPE_SHORT;
  my_bind[2].buffer = (void *)&small_data;
  my_bind[2].is_null = &is_null;
  is_null = false;

  /* Bind the buffers */
  if (mysql_stmt_bind_param(stmt, my_bind)) {
    fprintf(stderr, "\n param bind failed");
    fprintf(stderr, "\n %s", mysql_stmt_error(stmt));
    exit(1);
  }

  /* Specify the data */
  int_data = 10;                /* integer */
  my_stpcpy(str_data, "MySQL"); /* string  */

  /* INSERT SMALLINT data as NULL */
  is_null = true;

  /* Execute the insert statement - 1*/
  if (mysql_stmt_execute(stmt)) {
    fprintf(stderr, "\n execute 1 failed");
    fprintf(stderr, "\n %s", mysql_stmt_error(stmt));
    exit(1);
  }

  /* Get the total rows affected */
  affected_rows = mysql_stmt_affected_rows(stmt);

  if (!opt_silent)
    fprintf(stdout, "\n total affected rows: %ld", (ulong)affected_rows);
  if (affected_rows != 1) /* validate affected rows */
  {
    fprintf(stderr, "\n invalid affected rows by MySQL");
    exit(1);
  }

  /* Re-execute the insert, by changing the values */
  int_data = 1000;
  my_stpcpy(str_data, "The most popular open source database");
  small_data = 1000; /* smallint */
  is_null = false;   /* reset */

  /* Execute the insert statement - 2*/
  if (mysql_stmt_execute(stmt)) {
    fprintf(stderr, "\n execute 2 failed");
    fprintf(stderr, "\n %s", mysql_stmt_error(stmt));
    exit(1);
  }

  /* Get the total rows affected */
  affected_rows = mysql_stmt_affected_rows(stmt);

  if (!opt_silent)
    fprintf(stdout, "\n total affected rows: %ld", (ulong)affected_rows);
  if (affected_rows != 1) /* validate affected rows */
  {
    fprintf(stderr, "\n invalid affected rows by MySQL");
    exit(1);
  }

  /* Close the statement */
  if (mysql_stmt_close(stmt)) {
    fprintf(stderr, "\n failed while closing the statement");
    fprintf(stderr, "\n %s", mysql_stmt_error(stmt));
    exit(1);
  }
  rc = my_stmt_result("SELECT * FROM test_table");
  DIE_UNLESS(rc == 2);

  /* DROP THE TABLE */
  if (mysql_query(mysql, "DROP TABLE test_table")) {
    fprintf(stderr, "\n drop table failed");
    fprintf(stderr, "\n %s", mysql_error(mysql));
    exit(1);
  }
  if (!opt_silent) fprintf(stdout, "Success !!!");
}

/* Test alter table scenario in the middle of prepare */

static void test_prepare_alter() {
  MYSQL_STMT *stmt;
  int rc, id;
  MYSQL_BIND my_bind[1];
  bool is_null;

  myheader("test_prepare_alter");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_prep_alter");
  myquery(rc);

  rc =
      mysql_query(mysql, "CREATE TABLE test_prep_alter(id int, name char(20))");
  myquery(rc);

  rc = mysql_query(
      mysql, "INSERT INTO test_prep_alter values(10, 'venu'), (20, 'mysql')");
  myquery(rc);

  stmt = mysql_simple_prepare(mysql,
                              "INSERT INTO test_prep_alter VALUES(?, 'monty')");
  check_stmt(stmt);

  verify_param_count(stmt, 1);

  /*
    We need to memset bind structure because mysql_stmt_bind_param checks all
    its members.
  */
  memset(my_bind, 0, sizeof(my_bind));

  is_null = false;
  my_bind[0].buffer_type = MYSQL_TYPE_SHORT;
  my_bind[0].buffer = (void *)&id;
  my_bind[0].is_null = &is_null;

  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  id = 30;
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  if (thread_query("ALTER TABLE test_prep_alter change id id_new varchar(20)"))
    exit(1);

  is_null = true;
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = my_stmt_result("SELECT * FROM test_prep_alter");
  DIE_UNLESS(rc == 4);

  mysql_stmt_close(stmt);
}

/* Test the support of multi-statement executions */

static void test_multi_statements() {
  MYSQL *mysql_local;
  MYSQL_RES *result;
  int rc;

  const char *query =
      "\
DROP TABLE IF EXISTS test_multi_tab;\
CREATE TABLE test_multi_tab(id int, name char(20));\
INSERT INTO test_multi_tab(id) VALUES(10), (20);\
INSERT INTO test_multi_tab VALUES(20, 'insert;comma');\
SELECT * FROM test_multi_tab;\
UPDATE test_multi_tab SET name='new;name' WHERE id=20;\
DELETE FROM test_multi_tab WHERE name='new;name';\
SELECT * FROM test_multi_tab;\
DELETE FROM test_multi_tab WHERE id=10;\
SELECT * FROM test_multi_tab;\
DROP TABLE test_multi_tab;\
select 1;\
DROP TABLE IF EXISTS test_multi_tab";
  uint count, exp_value;
  uint rows[] = {0, 0, 2, 1, 3, 2, 2, 1, 1, 0, 0, 1, 0};

  myheader("test_multi_statements");

  /*
    First test that we get an error for multi statements
    (Because default connection is not opened with CLIENT_MULTI_STATEMENTS)
  */
  rc = mysql_query(mysql, query); /* syntax error */
  myquery_r(rc);

  rc = mysql_next_result(mysql);
  DIE_UNLESS(rc == -1);
  rc = mysql_more_results(mysql);
  DIE_UNLESS(rc == 0);

  if (!(mysql_local = mysql_client_init(nullptr))) {
    fprintf(stdout, "\n mysql_client_init() failed");
    exit(1);
  }

  /* Create connection that supports multi statements */
  if (!(mysql_real_connect(mysql_local, opt_host, opt_user, opt_password,
                           current_db, opt_port, opt_unix_socket,
                           CLIENT_MULTI_STATEMENTS))) {
    fprintf(stdout, "\n connection failed(%s)", mysql_error(mysql_local));
    exit(1);
  }
  mysql_local->reconnect = true;

  rc = mysql_query(mysql_local, query);
  myquery(rc);

  for (count = 0; count < array_elements(rows); count++) {
    if (!opt_silent) fprintf(stdout, "\n Query %d: ", count);
    if ((result = mysql_store_result(mysql_local))) {
      (void)my_process_result_set(result);
      mysql_free_result(result);
    } else if (!opt_silent)
      fprintf(stdout, "OK, %ld row(s) affected, %ld warning(s)\n",
              (ulong)mysql_affected_rows(mysql_local),
              (ulong)mysql_warning_count(mysql_local));

    exp_value = (uint)mysql_affected_rows(mysql_local);
    if (rows[count] != exp_value) {
      fprintf(stderr, "row %d  had affected rows: %d, should be %d\n", count,
              exp_value, rows[count]);
      exit(1);
    }
    if (count != array_elements(rows) - 1) {
      if (!(rc = mysql_more_results(mysql_local))) {
        fprintf(stdout,
                "mysql_more_result returned wrong value: %d for row %d\n", rc,
                count);
        exit(1);
      }
      if ((rc = mysql_next_result(mysql_local))) {
        exp_value = mysql_errno(mysql_local);

        exit(1);
      }
    } else {
      rc = mysql_more_results(mysql_local);
      DIE_UNLESS(rc == 0);
      rc = mysql_next_result(mysql_local);
      DIE_UNLESS(rc == -1);
    }
  }

  /* check that errors abort multi statements */

  rc = mysql_query(mysql_local, "select 1+1+a;select 1+1");
  myquery_r(rc);
  rc = mysql_more_results(mysql_local);
  DIE_UNLESS(rc == 0);
  rc = mysql_next_result(mysql_local);
  DIE_UNLESS(rc == -1);

  rc = mysql_query(mysql_local, "select 1+1;select 1+1+a;select 1");
  myquery(rc);
  result = mysql_store_result(mysql_local);
  mytest(result);
  mysql_free_result(result);
  rc = mysql_more_results(mysql_local);
  DIE_UNLESS(rc == 1);
  rc = mysql_next_result(mysql_local);
  DIE_UNLESS(rc > 0);

  /*
    Ensure that we can now do a simple query (this checks that the server is
    not trying to send us the results for the last 'select 1'
  */
  rc = mysql_query(mysql_local, "select 1+1+1");
  myquery(rc);
  result = mysql_store_result(mysql_local);
  mytest(result);
  (void)my_process_result_set(result);
  mysql_free_result(result);

  /*
    Check if errors in one of the queries handled properly.
  */
  rc = mysql_query(mysql_local, "select 1; select * from not_existing_table");
  myquery(rc);
  result = mysql_store_result(mysql_local);
  mysql_free_result(result);

  rc = mysql_next_result(mysql_local);
  DIE_UNLESS(rc > 0);

  rc = mysql_next_result(mysql_local);
  DIE_UNLESS(rc < 0);

  mysql_close(mysql_local);
}

/*
  Check that Prepared statement cannot contain several
  SQL statements
*/

static void test_prepare_multi_statements() {
  MYSQL *mysql_local;
  MYSQL_STMT *stmt;
  char query[MAX_TEST_QUERY_LENGTH];
  myheader("test_prepare_multi_statements");

  if (!(mysql_local = mysql_client_init(nullptr))) {
    fprintf(stderr, "\n mysql_client_init() failed");
    exit(1);
  }

  if (!(mysql_real_connect(mysql_local, opt_host, opt_user, opt_password,
                           current_db, opt_port, opt_unix_socket,
                           CLIENT_MULTI_STATEMENTS))) {
    fprintf(stderr, "\n connection failed(%s)", mysql_error(mysql_local));
    exit(1);
  }
  mysql_local->reconnect = true;
  my_stpcpy(query, "select 1; select 'another value'");
  stmt = mysql_simple_prepare(mysql_local, query);
  check_stmt_r(stmt);
  mysql_close(mysql_local);
}

/* Test simple bind store result */

static void test_store_result() {
  MYSQL_STMT *stmt;
  int rc;
  int32 nData;
  char szData[100];
  MYSQL_BIND my_bind[2];
  ulong length, length1;
  bool is_null[2];

  myheader("test_store_result");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_store_result");
  myquery(rc);

  rc = mysql_query(
      mysql, "CREATE TABLE test_store_result(col1 int , col2 varchar(50))");
  myquery(rc);

  rc = mysql_query(
      mysql, "INSERT INTO test_store_result VALUES(10, 'venu'), (20, 'mysql')");
  myquery(rc);

  rc =
      mysql_query(mysql, "INSERT INTO test_store_result(col2) VALUES('monty')");
  myquery(rc);

  rc = mysql_commit(mysql);
  myquery(rc);

  /* fetch */
  memset(my_bind, 0, sizeof(my_bind));
  my_bind[0].buffer_type = MYSQL_TYPE_LONG;
  my_bind[0].buffer = (void *)&nData; /* integer data */
  my_bind[0].length = &length;
  my_bind[0].is_null = &is_null[0];

  length = 0;
  my_bind[1].buffer_type = MYSQL_TYPE_STRING;
  my_bind[1].buffer = szData; /* string data */
  my_bind[1].buffer_length = (ulong)sizeof(szData);
  my_bind[1].length = &length1;
  my_bind[1].is_null = &is_null[1];
  length1 = 0;

  stmt = mysql_simple_prepare(mysql, "SELECT * FROM test_store_result");
  check_stmt(stmt);

  rc = mysql_stmt_bind_result(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_store_result(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  if (!opt_silent)
    fprintf(stdout, "\n row 1: %ld, %s(%lu)", (long)nData, szData, length1);
  DIE_UNLESS(nData == 10);
  DIE_UNLESS(strcmp(szData, "venu") == 0);
  DIE_UNLESS(length1 == 4);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  if (!opt_silent)
    fprintf(stdout, "\n row 2: %ld, %s(%lu)", (long)nData, szData, length1);
  DIE_UNLESS(nData == 20);
  DIE_UNLESS(strcmp(szData, "mysql") == 0);
  DIE_UNLESS(length1 == 5);

  length = 99;
  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  if (!opt_silent && is_null[0])
    fprintf(stdout, "\n row 3: NULL, %s(%lu)", szData, length1);
  DIE_UNLESS(is_null[0]);
  DIE_UNLESS(strcmp(szData, "monty") == 0);
  DIE_UNLESS(length1 == 5);

  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == MYSQL_NO_DATA);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_store_result(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  if (!opt_silent)
    fprintf(stdout, "\n row 1: %ld, %s(%lu)", (long)nData, szData, length1);
  DIE_UNLESS(nData == 10);
  DIE_UNLESS(strcmp(szData, "venu") == 0);
  DIE_UNLESS(length1 == 4);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  if (!opt_silent)
    fprintf(stdout, "\n row 2: %ld, %s(%lu)", (long)nData, szData, length1);
  DIE_UNLESS(nData == 20);
  DIE_UNLESS(strcmp(szData, "mysql") == 0);
  DIE_UNLESS(length1 == 5);

  length = 99;
  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  if (!opt_silent && is_null[0])
    fprintf(stdout, "\n row 3: NULL, %s(%lu)", szData, length1);
  DIE_UNLESS(is_null[0]);
  DIE_UNLESS(strcmp(szData, "monty") == 0);
  DIE_UNLESS(length1 == 5);

  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == MYSQL_NO_DATA);

  mysql_stmt_close(stmt);
}

/* Test simple bind store result */

static void test_store_result1() {
  MYSQL_STMT *stmt;
  int rc;

  myheader("test_store_result1");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_store_result");
  myquery(rc);

  rc = mysql_query(
      mysql, "CREATE TABLE test_store_result(col1 int , col2 varchar(50))");
  myquery(rc);

  rc = mysql_query(
      mysql, "INSERT INTO test_store_result VALUES(10, 'venu'), (20, 'mysql')");
  myquery(rc);

  rc =
      mysql_query(mysql, "INSERT INTO test_store_result(col2) VALUES('monty')");
  myquery(rc);

  rc = mysql_commit(mysql);
  myquery(rc);

  stmt = mysql_simple_prepare(mysql, "SELECT * FROM test_store_result");
  check_stmt(stmt);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_store_result(stmt);
  check_execute(stmt, rc);

  rc = 0;
  while (mysql_stmt_fetch(stmt) != MYSQL_NO_DATA) rc++;
  if (!opt_silent) fprintf(stdout, "\n total rows: %d", rc);
  DIE_UNLESS(rc == 3);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_store_result(stmt);
  check_execute(stmt, rc);

  rc = 0;
  while (mysql_stmt_fetch(stmt) != MYSQL_NO_DATA) rc++;
  if (!opt_silent) fprintf(stdout, "\n total rows: %d", rc);
  DIE_UNLESS(rc == 3);

  mysql_stmt_close(stmt);
}

/* Another test for bind and store result */

static void test_store_result2() {
  MYSQL_STMT *stmt;
  int rc;
  int nData;
  ulong length;
  MYSQL_BIND my_bind[1];
  char query[MAX_TEST_QUERY_LENGTH];

  myheader("test_store_result2");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_store_result");
  myquery(rc);

  rc = mysql_query(
      mysql, "CREATE TABLE test_store_result(col1 int , col2 varchar(50))");
  myquery(rc);

  rc = mysql_query(
      mysql, "INSERT INTO test_store_result VALUES(10, 'venu'), (20, 'mysql')");
  myquery(rc);

  rc =
      mysql_query(mysql, "INSERT INTO test_store_result(col2) VALUES('monty')");
  myquery(rc);

  rc = mysql_commit(mysql);
  myquery(rc);

  /*
    We need to memset bind structure because mysql_stmt_bind_param checks all
    its members.
  */
  memset(my_bind, 0, sizeof(my_bind));

  my_bind[0].buffer_type = MYSQL_TYPE_LONG;
  my_bind[0].buffer = (void *)&nData; /* integer data */
  my_bind[0].length = &length;
  my_bind[0].is_null = nullptr;

  my_stpcpy((char *)query, "SELECT col1 FROM test_store_result where col1= ?");
  stmt = mysql_simple_prepare(mysql, query);
  check_stmt(stmt);

  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_bind_result(stmt, my_bind);
  check_execute(stmt, rc);

  nData = 10;
  length = 0;
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  nData = 0;
  rc = mysql_stmt_store_result(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  if (!opt_silent) fprintf(stdout, "\n row 1: %d", nData);
  DIE_UNLESS(nData == 10);

  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == MYSQL_NO_DATA);

  nData = 20;
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  nData = 0;
  rc = mysql_stmt_store_result(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  if (!opt_silent) fprintf(stdout, "\n row 1: %d", nData);
  DIE_UNLESS(nData == 20);

  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == MYSQL_NO_DATA);
  mysql_stmt_close(stmt);
}

/* Test simple subselect prepare */

static void test_subselect() {
  MYSQL_STMT *stmt;
  int rc, id;
  MYSQL_BIND my_bind[1];
  DBUG_TRACE;

  myheader("test_subselect");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_sub1");
  myquery(rc);

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_sub2");
  myquery(rc);

  rc = mysql_query(mysql, "CREATE TABLE test_sub1(id int)");
  myquery(rc);

  rc = mysql_query(mysql, "CREATE TABLE test_sub2(id int, id1 int)");
  myquery(rc);

  rc = mysql_query(mysql, "INSERT INTO test_sub1 values(2)");
  myquery(rc);

  rc = mysql_query(mysql, "INSERT INTO test_sub2 VALUES(1, 7), (2, 7)");
  myquery(rc);

  rc = mysql_commit(mysql);
  myquery(rc);

  /* fetch */
  /*
    We need to memset bind structure because mysql_stmt_bind_param checks all
    its members.
  */
  memset(my_bind, 0, sizeof(my_bind));

  my_bind[0].buffer_type = MYSQL_TYPE_LONG;
  my_bind[0].buffer = (void *)&id;
  my_bind[0].length = nullptr;
  my_bind[0].is_null = nullptr;

  stmt = mysql_simple_prepare(
      mysql, "INSERT INTO test_sub2(id) SELECT * FROM test_sub1 WHERE id= ?");
  check_stmt(stmt);

  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  id = 2;
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  verify_st_affected_rows(stmt, 1);

  id = 9;
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  verify_st_affected_rows(stmt, 0);

  mysql_stmt_close(stmt);

  rc = my_stmt_result("SELECT * FROM test_sub2");
  DIE_UNLESS(rc == 3);

  rc = my_stmt_result(
      "SELECT ROW(1, 7) IN (select id, id1 "
      "from test_sub2 WHERE id1= 8)");
  DIE_UNLESS(rc == 1);
  rc = my_stmt_result(
      "SELECT ROW(1, 7) IN (select id, id1 "
      "from test_sub2 WHERE id1= 7)");
  DIE_UNLESS(rc == 1);

  stmt = mysql_simple_prepare(mysql, ("SELECT ROW(1, 7) IN (select id, id1 "
                                      "from test_sub2 WHERE id1= ?)"));
  check_stmt(stmt);

  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_bind_result(stmt, my_bind);
  check_execute(stmt, rc);

  id = 7;
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  if (!opt_silent) fprintf(stdout, "\n row 1: %d", id);
  DIE_UNLESS(id == 1);

  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == MYSQL_NO_DATA);

  id = 8;
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  if (!opt_silent) fprintf(stdout, "\n row 1: %d", id);
  DIE_UNLESS(id == 0);

  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == MYSQL_NO_DATA);

  mysql_stmt_close(stmt);
}

/*
  Generalized conversion routine to handle DATE, TIME and DATETIME
  conversion using MYSQL_TIME structure
*/

static void bind_date_conv(uint row_count, bool preserveFractions) {
  MYSQL_STMT *stmt = nullptr;
  uint rc, i, count = row_count;
  ulong length[4];
  MYSQL_BIND my_bind[4];
  bool is_null[4] = {false};
  MYSQL_TIME tm[4];
  ulong second_part;
  uint year, month, day, hour, minute, sec;
  uint now_year = 1990, now_month = 3, now_day = 13;

  rc = mysql_query(mysql, "SET timestamp=UNIX_TIMESTAMP('1990-03-13')");
  myquery(rc);

  stmt =
      mysql_simple_prepare(mysql, "INSERT INTO test_date VALUES(?, ?, ?, ?)");
  check_stmt(stmt);

  verify_param_count(stmt, 4);

  /*
    We need to memset bind structure because mysql_stmt_bind_param checks all
    its members.
  */
  memset(my_bind, 0, sizeof(my_bind));

  my_bind[0].buffer_type = MYSQL_TYPE_TIMESTAMP;
  my_bind[1].buffer_type = MYSQL_TYPE_TIME;
  my_bind[2].buffer_type = MYSQL_TYPE_DATETIME;
  my_bind[3].buffer_type = MYSQL_TYPE_DATE;

  for (i = 0; i < (uint)array_elements(my_bind); i++) {
    my_bind[i].buffer = (void *)&tm[i];
    my_bind[i].is_null = &is_null[i];
    my_bind[i].length = &length[i];
    my_bind[i].buffer_length = 30;
    length[i] = 20;
  }

  second_part = 0;

  year = 2000;
  month = 01;
  day = 10;

  hour = 11;
  minute = 16;
  sec = 20;

  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  for (count = 0; count < row_count; count++) {
    for (i = 0; i < (uint)array_elements(my_bind); i++) {
      tm[i].neg = false;
      tm[i].second_part = second_part + count;
      if (my_bind[i].buffer_type != MYSQL_TYPE_TIME) {
        tm[i].year = year + count;
        tm[i].month = month + count;
        tm[i].day = day + count;
      } else
        tm[i].year = tm[i].month = tm[i].day = 0;
      if (my_bind[i].buffer_type != MYSQL_TYPE_DATE) {
        tm[i].hour = hour + count;
        tm[i].minute = minute + count;
        tm[i].second = sec + count;
      } else
        tm[i].hour = tm[i].minute = tm[i].second = 0;
      tm[i].time_type = MYSQL_TIMESTAMP_NONE;
    }
    rc = mysql_stmt_execute(stmt);
    check_execute(stmt, rc);
  }

  rc = mysql_commit(mysql);
  myquery(rc);

  mysql_stmt_close(stmt);

  rc = my_stmt_result("SELECT * FROM test_date");
  DIE_UNLESS(row_count == rc);

  stmt = mysql_simple_prepare(mysql, "SELECT * FROM test_date");
  check_stmt(stmt);

  rc = mysql_stmt_bind_result(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_store_result(stmt);
  check_execute(stmt, rc);

  for (count = 0; count < row_count; count++) {
    rc = mysql_stmt_fetch(stmt);
    DIE_UNLESS(rc == 0 || rc == MYSQL_DATA_TRUNCATED);

    if (!opt_silent) fprintf(stdout, "\n");
    for (i = 0; i < array_elements(my_bind); i++) {
      if (!opt_silent)
        fprintf(stdout, "\ntime[%d]: %02d-%02d-%02d %02d:%02d:%02d.%06lu", i,
                tm[i].year, tm[i].month, tm[i].day, tm[i].hour, tm[i].minute,
                tm[i].second, tm[i].second_part);
      DIE_UNLESS(tm[i].year == 0 || tm[i].year == year + count ||
                 (tm[i].year == now_year &&
                  my_bind[i].buffer_type == MYSQL_TYPE_TIME));
      DIE_UNLESS(tm[i].month == 0 || tm[i].month == month + count ||
                 (tm[i].month == now_month &&
                  my_bind[i].buffer_type == MYSQL_TYPE_TIME));
      DIE_UNLESS(
          tm[i].day == 0 || tm[i].day == day + count ||
          (tm[i].day == now_day && my_bind[i].buffer_type == MYSQL_TYPE_TIME));

      DIE_UNLESS(tm[i].hour == 0 || tm[i].hour == hour + count);
      DIE_UNLESS(tm[i].minute == 0 || tm[i].minute == minute + count);
      DIE_UNLESS(tm[i].second == 0 || tm[i].second == sec + count);
      if (preserveFractions) {
        if (i == 3) { /* Dates dont have fractions */
          DIE_UNLESS(tm[i].second_part == 0);
        } else {
          DIE_UNLESS(tm[i].second_part == second_part + count);
        }
      } else {
        DIE_UNLESS((tm[i].second_part == 0) ||
                   tm[i].second_part == second_part + count);
      }
    }
  }
  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == MYSQL_NO_DATA);

  mysql_stmt_close(stmt);

  /* set the timestamp back to default */
  rc = mysql_query(mysql, "SET timestamp=DEFAULT");
  myquery(rc);
}

/* Test DATE, TIME, DATETIME and TS with MYSQL_TIME conversion */

static void test_date() {
  int rc;

  myheader("test_date");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_date");
  myquery(rc);

  rc = mysql_query(mysql,
                   "CREATE TABLE test_date(c1 TIMESTAMP, \
                                                 c2 TIME, \
                                                 c3 DATETIME, \
                                                 c4 DATE)");

  myquery(rc);

  bind_date_conv(5, false);
}

/* Test DATE, TIME(6), DATETIME(6) and TS(6) with MYSQL_TIME conversion */

static void test_date_frac() {
  int rc;

  myheader("test_date");

  if (mysql_get_server_version(mysql) < 50604) {
    if (!opt_silent)
      fprintf(stdout,
              "Skipping test_date_frac: fractinal seconds implemented "
              "in MySQL 5.6.4\n");
    return;
  }

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_date");
  myquery(rc);

  rc = mysql_query(mysql,
                   "CREATE TABLE test_date(c1 TIMESTAMP(6), \
                                                 c2 TIME(6), \
                                                 c3 DATETIME(6), \
                                                 c4 DATE)");

  myquery(rc);

  bind_date_conv(5, true);
}

/* Test all time types to DATE and DATE to all types */

static void test_date_date() {
  int rc;

  myheader("test_date_date");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_date");
  myquery(rc);

  rc = mysql_query(mysql,
                   "CREATE TABLE test_date(c1 DATE, \
                                                 c2 DATE, \
                                                 c3 DATE, \
                                                 c4 DATE)");

  myquery(rc);

  bind_date_conv(3, false);
}

/* Test all time types to TIME and TIME to all types */

static void test_date_time() {
  int rc;

  myheader("test_date_time");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_date");
  myquery(rc);

  rc = mysql_query(mysql,
                   "CREATE TABLE test_date(c1 TIME, \
                                                 c2 TIME, \
                                                 c3 TIME, \
                                                 c4 TIME)");

  myquery(rc);

  bind_date_conv(3, false);
}

/* Test all time types to TIMESTAMP and TIMESTAMP to all types */

static void test_date_ts() {
  int rc;

  myheader("test_date_ts");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_date");
  myquery(rc);

  rc = mysql_query(mysql,
                   "CREATE TABLE test_date(c1 TIMESTAMP, \
                                                 c2 TIMESTAMP, \
                                                 c3 TIMESTAMP, \
                                                 c4 TIMESTAMP)");

  myquery(rc);

  bind_date_conv(2, false);
}

/* Test all time types to DATETIME and DATETIME to all types */

static void test_date_dt() {
  int rc;

  myheader("test_date_dt");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_date");
  myquery(rc);

  rc = mysql_query(mysql,
                   "CREATE TABLE test_date(c1 datetime, "
                   " c2 datetime, c3 datetime, c4 date)");
  myquery(rc);

  bind_date_conv(2, false);
}

/*
  Test TIME/DATETIME parameters to cover the following methods:
    Item_param::val_int()
    Item_param::val_real()
    Item_param::val_decimal()
*/
static void test_temporal_param() {
#define N_PARAMS 3
  MYSQL_STMT *stmt = nullptr;
  uint rc;
  ulong length[N_PARAMS], length2[N_PARAMS];
  MYSQL_BIND my_bind[N_PARAMS], my_bind2[N_PARAMS];
  bool is_null[N_PARAMS], is_null2[N_PARAMS];
  MYSQL_TIME tm;
  longlong bigint = 123;
  double real = 123;
  char dec[40];

  if (mysql_get_server_version(mysql) < 50600) {
    if (!opt_silent)
      fprintf(stdout,
              "Skipping test_temporal_param: this test cannot be "
              "executed on servers prior to 5.6 until bug#16328037 is fixed\n");
    return;
  }

  /* Initialize param/fetch buffers for data, null flags, lengths */
  memset(&my_bind, 0, sizeof(my_bind));
  memset(&my_bind2, 0, sizeof(my_bind2));
  memset(&length, 0, sizeof(length));
  memset(&length2, 0, sizeof(length2));
  memset(&is_null, 0, sizeof(is_null));
  memset(&is_null2, 0, sizeof(is_null2));

  /* Initialize the first input parameter */
  my_bind[0].buffer_type = MYSQL_TYPE_TIMESTAMP;
  my_bind[0].buffer = (void *)&tm;
  my_bind[0].is_null = &is_null[0];
  my_bind[0].length = &length[0];
  my_bind[0].buffer_length = (ulong)sizeof(tm);

  /* Clone the second and the third input parameter */
  my_bind[2] = my_bind[1] = my_bind[0];

  /* Initialize fetch parameters */
  my_bind2[0].buffer_type = MYSQL_TYPE_LONGLONG;
  my_bind2[0].length = &length2[0];
  my_bind2[0].is_null = &is_null2[0];
  my_bind2[0].buffer_length = (ulong)sizeof(bigint);
  my_bind2[0].buffer = (void *)&bigint;

  my_bind2[1].buffer_type = MYSQL_TYPE_DOUBLE;
  my_bind2[1].length = &length2[1];
  my_bind2[1].is_null = &is_null2[1];
  my_bind2[1].buffer_length = (ulong)sizeof(real);
  my_bind2[1].buffer = (void *)&real;

  my_bind2[2].buffer_type = MYSQL_TYPE_STRING;
  my_bind2[2].length = &length2[2];
  my_bind2[2].is_null = &is_null2[2];
  my_bind2[2].buffer_length = (ulong)sizeof(dec);
  my_bind2[2].buffer = (void *)&dec;

  /* Prepare and bind input and output parameters */
  stmt = mysql_simple_prepare(mysql, "SELECT CAST(? AS SIGNED), ?+0e0, ?+0.0");
  check_stmt(stmt);
  verify_param_count(stmt, N_PARAMS);

  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_bind_result(stmt, my_bind2);
  check_execute(stmt, rc);

  /* Initialize DATETIME value */
  tm.neg = false;
  tm.time_type = MYSQL_TIMESTAMP_DATETIME;
  tm.year = 2001;
  tm.month = 10;
  tm.day = 20;
  tm.hour = 10;
  tm.minute = 10;
  tm.second = 59;
  tm.second_part = 500000;

  /* Execute and fetch */
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_store_result(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  if (!opt_silent) printf("\n%lld %f '%s'\n", bigint, real, dec);

  /* Check values.  */
  DIE_UNLESS(bigint == 20011020101100LL);
  DIE_UNLESS(real == 20011020101059.5);
  DIE_UNLESS(!strcmp(dec, "20011020101059.500000"));

  mysql_stmt_close(stmt);

  /* Re-initialize input parameters to TIME data type */
  my_bind[0].buffer_type = my_bind[1].buffer_type = my_bind[2].buffer_type =
      MYSQL_TYPE_TIME;

  /* Prepare and bind intput and output parameters */
  stmt = mysql_simple_prepare(mysql, "SELECT CAST(? AS SIGNED), ?+0e0, ?+0.0");
  check_stmt(stmt);
  verify_param_count(stmt, N_PARAMS);

  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_bind_result(stmt, my_bind2);
  check_execute(stmt, rc);

  /* Initialize TIME value */
  tm.neg = false;
  tm.time_type = MYSQL_TIMESTAMP_TIME;
  tm.year = tm.month = tm.day = 0;
  tm.hour = 10;
  tm.minute = 10;
  tm.second = 59;
  tm.second_part = 500000;

  /* Execute and fetch */
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_store_result(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  if (!opt_silent) printf("\n%lld %f '%s'\n", bigint, real, dec);

  /* Check returned values */
  DIE_UNLESS(bigint == 101100);
  DIE_UNLESS(real == 101059.5);
  DIE_UNLESS(!strcmp(dec, "101059.500000"));

  mysql_stmt_close(stmt);
}

/* Misc tests to keep pure coverage happy */

static void test_pure_coverage() {
  MYSQL_STMT *stmt;
  MYSQL_BIND my_bind[2];
  int rc;
  ulong length;

  myheader("test_pure_coverage");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_pure");
  myquery(rc);

  rc = mysql_query(mysql, "CREATE TABLE test_pure(c1 int, c2 varchar(20))");
  myquery(rc);

  stmt =
      mysql_simple_prepare(mysql, "insert into test_pure(c67788) values(10)");
  check_stmt_r(stmt);

  /* Query without params and result should allow to bind 0 arrays */
  stmt = mysql_simple_prepare(mysql, "insert into test_pure(c2) values(10)");
  check_stmt(stmt);

  rc = mysql_stmt_bind_param(stmt, (MYSQL_BIND *)nullptr);
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_bind_result(stmt, (MYSQL_BIND *)nullptr);
  DIE_UNLESS(rc == 1);

  mysql_stmt_close(stmt);

  stmt = mysql_simple_prepare(mysql, "insert into test_pure(c2) values(?)");
  check_stmt(stmt);

  /*
    We need to memset bind structure because mysql_stmt_bind_param checks all
    its members.
  */
  memset(my_bind, 0, sizeof(my_bind));

  my_bind[0].length = &length;
  my_bind[0].is_null = nullptr;
  my_bind[0].buffer_length = 0;

  my_bind[0].buffer_type = MYSQL_TYPE_GEOMETRY;
  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute_r(stmt, rc); /* unsupported buffer type */

  my_bind[0].buffer_type = MYSQL_TYPE_STRING;
  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_store_result(stmt);
  check_execute(stmt, rc);

  mysql_stmt_close(stmt);

  stmt = mysql_simple_prepare(mysql, "select * from test_pure");
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  // NOTE: stmt now has two columns, but only my_bind[0] is initialized.
  my_bind[0].buffer_type = MYSQL_TYPE_GEOMETRY;
  rc = mysql_stmt_bind_result(stmt, my_bind);
  check_execute_r(stmt, rc); /* unsupported buffer type */

  rc = mysql_stmt_store_result(stmt);
  DIE_UNLESS(rc);

  rc = mysql_stmt_store_result(stmt);
  DIE_UNLESS(rc); /* Old error must be reset first */

  mysql_stmt_close(stmt);

  mysql_query(mysql, "DROP TABLE test_pure");
}

/* Test for string buffer fetch */

static void test_buffers() {
  MYSQL_STMT *stmt;
  // The test_pure table has two columns.
  MYSQL_BIND my_bind[2];
  int rc;
  ulong length;
  bool is_null;
  char buffer[20];

  myheader("test_buffers");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_buffer");
  myquery(rc);

  rc = mysql_query(mysql, "CREATE TABLE test_buffer(str varchar(20))");
  myquery(rc);

  rc = mysql_query(mysql,
                   "insert into test_buffer values('MySQL')\
                          , ('Database'), ('Open-Source'), ('Popular')");
  myquery(rc);

  stmt = mysql_simple_prepare(mysql, "select str from test_buffer");
  check_stmt(stmt);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  memset(buffer, 0, sizeof(buffer)); /* Avoid overruns in printf() */

  memset(my_bind, 0, sizeof(my_bind));
  my_bind[0].length = &length;
  my_bind[0].is_null = &is_null;
  my_bind[0].buffer_length = 1;
  my_bind[0].buffer_type = MYSQL_TYPE_STRING;
  my_bind[0].buffer = (void *)buffer;
  my_bind[0].error = &my_bind[0].error_value;

  rc = mysql_stmt_bind_result(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_store_result(stmt);
  check_execute(stmt, rc);

  buffer[1] = 'X';
  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == MYSQL_DATA_TRUNCATED);
  DIE_UNLESS(my_bind[0].error_value);
  if (!opt_silent) fprintf(stdout, "\n data: %s (%lu)", buffer, length);
  DIE_UNLESS(buffer[0] == 'M');
  DIE_UNLESS(buffer[1] == 'X');
  DIE_UNLESS(length == 5);

  my_bind[0].buffer_length = 8;
  rc = mysql_stmt_bind_result(stmt, my_bind); /* re-bind */
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);
  if (!opt_silent) fprintf(stdout, "\n data: %s (%lu)", buffer, length);
  DIE_UNLESS(strncmp(buffer, "Database", 8) == 0);
  DIE_UNLESS(length == 8);

  my_bind[0].buffer_length = 12;
  rc = mysql_stmt_bind_result(stmt, my_bind); /* re-bind */
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);
  if (!opt_silent) fprintf(stdout, "\n data: %s (%lu)", buffer, length);
  DIE_UNLESS(strcmp(buffer, "Open-Source") == 0);
  DIE_UNLESS(length == 11);

  my_bind[0].buffer_length = 6;
  rc = mysql_stmt_bind_result(stmt, my_bind); /* re-bind */
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == MYSQL_DATA_TRUNCATED);
  DIE_UNLESS(my_bind[0].error_value);
  if (!opt_silent) fprintf(stdout, "\n data: %s (%lu)", buffer, length);
  DIE_UNLESS(strncmp(buffer, "Popula", 6) == 0);
  DIE_UNLESS(length == 7);

  mysql_stmt_close(stmt);
}

/* Test the direct query execution in the middle of open stmts */

static void test_open_direct() {
  MYSQL_STMT *stmt;
  MYSQL_RES *result;
  int rc;

  myheader("test_open_direct");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_open_direct");
  myquery(rc);

  rc =
      mysql_query(mysql, "CREATE TABLE test_open_direct(id int, name char(6))");
  myquery(rc);

  stmt = mysql_simple_prepare(
      mysql, "INSERT INTO test_open_direct values(10, 'mysql')");
  check_stmt(stmt);

  rc = mysql_query(mysql, "SELECT * FROM test_open_direct");
  myquery(rc);

  result = mysql_store_result(mysql);
  mytest(result);

  rc = my_process_result_set(result);
  DIE_UNLESS(rc == 0);
  mysql_free_result(result);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  verify_st_affected_rows(stmt, 1);

  rc = mysql_query(mysql, "SELECT * FROM test_open_direct");
  myquery(rc);

  result = mysql_store_result(mysql);
  mytest(result);

  rc = my_process_result_set(result);
  DIE_UNLESS(rc == 1);
  mysql_free_result(result);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  verify_st_affected_rows(stmt, 1);

  rc = mysql_query(mysql, "SELECT * FROM test_open_direct");
  myquery(rc);

  result = mysql_store_result(mysql);
  mytest(result);

  rc = my_process_result_set(result);
  DIE_UNLESS(rc == 2);
  mysql_free_result(result);

  mysql_stmt_close(stmt);

  /* run a direct query in the middle of a fetch */
  stmt = mysql_simple_prepare(mysql, "SELECT * FROM test_open_direct");
  check_stmt(stmt);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  rc = mysql_query(mysql, "INSERT INTO test_open_direct(id) VALUES(20)");
  myquery_r(rc);

  rc = mysql_stmt_close(stmt);
  check_execute(stmt, rc);

  rc = mysql_query(mysql, "INSERT INTO test_open_direct(id) VALUES(20)");
  myquery(rc);

  /* run a direct query with store result */
  stmt = mysql_simple_prepare(mysql, "SELECT * FROM test_open_direct");
  check_stmt(stmt);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_store_result(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  rc = mysql_query(mysql, "drop table test_open_direct");
  myquery(rc);

  rc = mysql_stmt_close(stmt);
  check_execute(stmt, rc);
}

/* Test fetch without prior bound buffers */

static void test_fetch_nobuffs() {
  MYSQL_STMT *stmt;
  MYSQL_BIND my_bind[4];
  char str[4][50];
  int rc;

  myheader("test_fetch_nobuffs");

  stmt = mysql_simple_prepare(mysql,
                              "SELECT DATABASE(), CURRENT_USER(), \
                              CURRENT_DATE(), CURRENT_TIME()");
  check_stmt(stmt);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = 0;
  while (mysql_stmt_fetch(stmt) != MYSQL_NO_DATA) rc++;

  if (!opt_silent) fprintf(stdout, "\n total rows        : %d", rc);
  DIE_UNLESS(rc == 1);

  memset(my_bind, 0, sizeof(MYSQL_BIND));
  my_bind[0].buffer_type = MYSQL_TYPE_STRING;
  my_bind[0].buffer = (void *)str[0];
  my_bind[0].buffer_length = (ulong)sizeof(str[0]);
  my_bind[1] = my_bind[2] = my_bind[3] = my_bind[0];
  my_bind[1].buffer = (void *)str[1];
  my_bind[2].buffer = (void *)str[2];
  my_bind[3].buffer = (void *)str[3];

  rc = mysql_stmt_bind_result(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = 0;
  while (mysql_stmt_fetch(stmt) != MYSQL_NO_DATA) {
    rc++;
    if (!opt_silent) {
      fprintf(stdout, "\n CURRENT_DATABASE(): %s", str[0]);
      fprintf(stdout, "\n CURRENT_USER()    : %s", str[1]);
      fprintf(stdout, "\n CURRENT_DATE()    : %s", str[2]);
      fprintf(stdout, "\n CURRENT_TIME()    : %s", str[3]);
    }
  }
  if (!opt_silent) fprintf(stdout, "\n total rows        : %d", rc);
  DIE_UNLESS(rc == 1);

  mysql_stmt_close(stmt);
}

/* Test a misc bug */

static void test_ushort_bug() {
  MYSQL_STMT *stmt;
  MYSQL_BIND my_bind[4];
  ushort short_value;
  uint32 long_value;
  ulong s_length, l_length, ll_length, t_length;
  ulonglong longlong_value;
  int rc;
  uchar tiny_value;
  char llbuf[22];
  myheader("test_ushort_bug");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_ushort");
  myquery(rc);

  rc = mysql_query(mysql,
                   "CREATE TABLE test_ushort(a smallint unsigned, \
                                                  b smallint unsigned, \
                                                  c smallint unsigned, \
                                                  d smallint unsigned)");
  myquery(rc);

  rc = mysql_query(mysql,
                   "INSERT INTO test_ushort VALUES(35999, 35999, 35999, 200)");
  myquery(rc);

  stmt = mysql_simple_prepare(mysql, "SELECT * FROM test_ushort");
  check_stmt(stmt);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  memset(my_bind, 0, sizeof(my_bind));
  my_bind[0].buffer_type = MYSQL_TYPE_SHORT;
  my_bind[0].buffer = (void *)&short_value;
  my_bind[0].is_unsigned = true;
  my_bind[0].length = &s_length;

  my_bind[1].buffer_type = MYSQL_TYPE_LONG;
  my_bind[1].buffer = (void *)&long_value;
  my_bind[1].length = &l_length;

  my_bind[2].buffer_type = MYSQL_TYPE_LONGLONG;
  my_bind[2].buffer = (void *)&longlong_value;
  my_bind[2].length = &ll_length;

  my_bind[3].buffer_type = MYSQL_TYPE_TINY;
  my_bind[3].buffer = (void *)&tiny_value;
  my_bind[3].is_unsigned = true;
  my_bind[3].length = &t_length;

  rc = mysql_stmt_bind_result(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  if (!opt_silent) {
    fprintf(stdout, "\n ushort   : %d (%ld)", short_value, (long)s_length);
    fprintf(stdout, "\n ulong    : %lu (%ld)", (ulong)long_value,
            (long)l_length);
    fprintf(stdout, "\n longlong : %s (%ld)", llstr(longlong_value, llbuf),
            (long)ll_length);
    fprintf(stdout, "\n tinyint  : %d   (%ld)", tiny_value, (long)t_length);
  }

  DIE_UNLESS(short_value == 35999);
  DIE_UNLESS(s_length == 2);

  DIE_UNLESS(long_value == 35999);
  DIE_UNLESS(l_length == 4);

  DIE_UNLESS(longlong_value == 35999);
  DIE_UNLESS(ll_length == 8);

  DIE_UNLESS(tiny_value == 200);
  DIE_UNLESS(t_length == 1);

  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == MYSQL_NO_DATA);

  mysql_stmt_close(stmt);
}

/* Test a misc smallint-signed conversion bug */

static void test_sshort_bug() {
  MYSQL_STMT *stmt;
  MYSQL_BIND my_bind[4];
  short short_value;
  int32 long_value;
  ulong s_length, l_length, ll_length, t_length;
  ulonglong longlong_value;
  int rc;
  uchar tiny_value;
  char llbuf[22];

  myheader("test_sshort_bug");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_sshort");
  myquery(rc);

  rc = mysql_query(mysql,
                   "CREATE TABLE test_sshort(a smallint signed, \
                                                  b smallint signed, \
                                                  c smallint unsigned, \
                                                  d smallint unsigned)");
  myquery(rc);

  rc = mysql_query(mysql,
                   "INSERT INTO test_sshort VALUES(-5999, -5999, 35999, 200)");
  myquery(rc);

  stmt = mysql_simple_prepare(mysql, "SELECT * FROM test_sshort");
  check_stmt(stmt);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  memset(my_bind, 0, sizeof(my_bind));
  my_bind[0].buffer_type = MYSQL_TYPE_SHORT;
  my_bind[0].buffer = (void *)&short_value;
  my_bind[0].length = &s_length;

  my_bind[1].buffer_type = MYSQL_TYPE_LONG;
  my_bind[1].buffer = (void *)&long_value;
  my_bind[1].length = &l_length;

  my_bind[2].buffer_type = MYSQL_TYPE_LONGLONG;
  my_bind[2].buffer = (void *)&longlong_value;
  my_bind[2].length = &ll_length;

  my_bind[3].buffer_type = MYSQL_TYPE_TINY;
  my_bind[3].buffer = (void *)&tiny_value;
  my_bind[3].is_unsigned = true;
  my_bind[3].length = &t_length;

  rc = mysql_stmt_bind_result(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  if (!opt_silent) {
    fprintf(stdout, "\n sshort   : %d (%ld)", short_value, (long)s_length);
    fprintf(stdout, "\n slong    : %ld (%ld)", (long)long_value,
            (long)l_length);
    fprintf(stdout, "\n longlong : %s (%ld)", llstr(longlong_value, llbuf),
            (long)ll_length);
    fprintf(stdout, "\n tinyint  : %d   (%ld)", tiny_value, (long)t_length);
  }

  DIE_UNLESS(short_value == -5999);
  DIE_UNLESS(s_length == 2);

  DIE_UNLESS(long_value == -5999);
  DIE_UNLESS(l_length == 4);

  DIE_UNLESS(longlong_value == 35999);
  DIE_UNLESS(ll_length == 8);

  DIE_UNLESS(tiny_value == 200);
  DIE_UNLESS(t_length == 1);

  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == MYSQL_NO_DATA);

  mysql_stmt_close(stmt);
}

/* Test a misc tinyint-signed conversion bug */

static void test_stiny_bug() {
  MYSQL_STMT *stmt;
  MYSQL_BIND my_bind[4];
  short short_value;
  int32 long_value;
  ulong s_length, l_length, ll_length, t_length;
  ulonglong longlong_value;
  int rc;
  uchar tiny_value;
  char llbuf[22];

  myheader("test_stiny_bug");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_stiny");
  myquery(rc);

  rc = mysql_query(mysql,
                   "CREATE TABLE test_stiny(a tinyint signed, \
                                                  b tinyint signed, \
                                                  c tinyint unsigned, \
                                                  d tinyint unsigned)");
  myquery(rc);

  rc = mysql_query(mysql, "INSERT INTO test_stiny VALUES(-128, -127, 255, 0)");
  myquery(rc);

  stmt = mysql_simple_prepare(mysql, "SELECT * FROM test_stiny");
  check_stmt(stmt);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  memset(my_bind, 0, sizeof(my_bind));
  my_bind[0].buffer_type = MYSQL_TYPE_SHORT;
  my_bind[0].buffer = (void *)&short_value;
  my_bind[0].length = &s_length;

  my_bind[1].buffer_type = MYSQL_TYPE_LONG;
  my_bind[1].buffer = (void *)&long_value;
  my_bind[1].length = &l_length;

  my_bind[2].buffer_type = MYSQL_TYPE_LONGLONG;
  my_bind[2].buffer = (void *)&longlong_value;
  my_bind[2].length = &ll_length;

  my_bind[3].buffer_type = MYSQL_TYPE_TINY;
  my_bind[3].buffer = (void *)&tiny_value;
  my_bind[3].length = &t_length;

  rc = mysql_stmt_bind_result(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  if (!opt_silent) {
    fprintf(stdout, "\n sshort   : %d (%ld)", short_value, (long)s_length);
    fprintf(stdout, "\n slong    : %ld (%ld)", (long)long_value,
            (long)l_length);
    fprintf(stdout, "\n longlong : %s  (%ld)", llstr(longlong_value, llbuf),
            (long)ll_length);
    fprintf(stdout, "\n tinyint  : %d    (%ld)", tiny_value, (long)t_length);
  }

  DIE_UNLESS(short_value == -128);
  DIE_UNLESS(s_length == 2);

  DIE_UNLESS(long_value == -127);
  DIE_UNLESS(l_length == 4);

  DIE_UNLESS(longlong_value == 255);
  DIE_UNLESS(ll_length == 8);

  DIE_UNLESS(tiny_value == 0);
  DIE_UNLESS(t_length == 1);

  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == MYSQL_NO_DATA);

  mysql_stmt_close(stmt);
}

/* Test misc field information, bug: #74 */

static void test_field_misc() {
  MYSQL_STMT *stmt;
  MYSQL_RES *result;
  int rc;

  myheader("test_field_misc");

  rc = mysql_query(mysql, "SELECT @@autocommit");
  myquery(rc);

  result = mysql_store_result(mysql);
  mytest(result);

  rc = my_process_result_set(result);
  DIE_UNLESS(rc == 1);

  verify_prepare_field(result, 0, "@@autocommit",
                       "",                  /* field and its org name */
                       MYSQL_TYPE_LONGLONG, /* field type */
                       "", "",              /* table and its org name */
                       "", 1, 0);           /* db name, length(its bool flag)*/

  mysql_free_result(result);

  stmt = mysql_simple_prepare(mysql, "SELECT @@autocommit");
  check_stmt(stmt);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  result = mysql_stmt_result_metadata(stmt);
  mytest(result);

  rc = my_process_stmt_result(stmt);
  DIE_UNLESS(rc == 1);

  verify_prepare_field(result, 0, "@@autocommit",
                       "",                  /* field and its org name */
                       MYSQL_TYPE_LONGLONG, /* field type */
                       "", "",              /* table and its org name */
                       "", 1, 0);           /* db name, length(its bool flag)*/

  mysql_free_result(result);
  mysql_stmt_close(stmt);

  stmt = mysql_simple_prepare(mysql, "SELECT @@max_error_count");
  check_stmt(stmt);

  result = mysql_stmt_result_metadata(stmt);
  mytest(result);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = my_process_stmt_result(stmt);
  DIE_UNLESS(rc == 1);

  verify_prepare_field(result, 0, "@@max_error_count",
                       "",                  /* field and its org name */
                       MYSQL_TYPE_LONGLONG, /* field type */
                       "", "",              /* table and its org name */
                       /* db name, length */
                       "", MY_INT64_NUM_DECIMAL_DIGITS, 0);

  mysql_free_result(result);
  mysql_stmt_close(stmt);

  stmt = mysql_simple_prepare(mysql, "SELECT @@max_allowed_packet");
  check_stmt(stmt);

  result = mysql_stmt_result_metadata(stmt);
  mytest(result);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  DIE_UNLESS(1 == my_process_stmt_result(stmt));

  verify_prepare_field(result, 0, "@@max_allowed_packet",
                       "",                  /* field and its org name */
                       MYSQL_TYPE_LONGLONG, /* field type */
                       "", "",              /* table and its org name */
                       /* db name, length */
                       "", MY_INT64_NUM_DECIMAL_DIGITS, 0);

  mysql_free_result(result);
  mysql_stmt_close(stmt);

  stmt = mysql_simple_prepare(mysql, "SELECT @@sql_warnings");
  check_stmt(stmt);

  result = mysql_stmt_result_metadata(stmt);
  mytest(result);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = my_process_stmt_result(stmt);
  DIE_UNLESS(rc == 1);

  verify_prepare_field(result, 0, "@@sql_warnings",
                       "",                  /* field and its org name */
                       MYSQL_TYPE_LONGLONG, /* field type */
                       "", "",              /* table and its org name */
                       "", 1, 0);           /* db name, length */

  mysql_free_result(result);
  mysql_stmt_close(stmt);
}

/*
  Test SET feature with prepare stmts, bug #85
*/

static void test_set_option() {
  MYSQL_STMT *stmt;
  MYSQL_RES *result;
  int rc;

  myheader("test_set_option");

  mysql_autocommit(mysql, true);

  /* LIMIT the rows count to 2 */
  rc = mysql_query(mysql, "SET SQL_SELECT_LIMIT= 2");
  myquery(rc);

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_limit");
  myquery(rc);

  rc = mysql_query(mysql, "CREATE TABLE test_limit(a tinyint)");
  myquery(rc);

  rc =
      mysql_query(mysql, "INSERT INTO test_limit VALUES(10), (20), (30), (40)");
  myquery(rc);

  if (!opt_silent) fprintf(stdout, "\n with SQL_SELECT_LIMIT= 2 (direct)");
  rc = mysql_query(mysql, "SELECT * FROM test_limit");
  myquery(rc);

  result = mysql_store_result(mysql);
  mytest(result);

  rc = my_process_result_set(result);
  DIE_UNLESS(rc == 2);

  mysql_free_result(result);

  if (!opt_silent) fprintf(stdout, "\n with SQL_SELECT_LIMIT=2 (prepare)");
  stmt = mysql_simple_prepare(mysql, "SELECT * FROM test_limit");
  check_stmt(stmt);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = my_process_stmt_result(stmt);
  DIE_UNLESS(rc == 2);

  mysql_stmt_close(stmt);

  /* RESET the LIMIT the rows count to 0 */
  if (!opt_silent)
    fprintf(stdout, "\n with SQL_SELECT_LIMIT=DEFAULT (prepare)");
  rc = mysql_query(mysql, "SET SQL_SELECT_LIMIT=DEFAULT");
  myquery(rc);

  stmt = mysql_simple_prepare(mysql, "SELECT * FROM test_limit");
  check_stmt(stmt);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = my_process_stmt_result(stmt);
  DIE_UNLESS(rc == 4);

  mysql_stmt_close(stmt);
}

/*
  Test a misc GRANT option, bug #89
*/

static void test_prepare_grant() {
  int rc;
  char query[MAX_TEST_QUERY_LENGTH];

  myheader("test_prepare_grant");

  mysql_autocommit(mysql, true);

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_grant");
  myquery(rc);

  rc = mysql_query(
      mysql, "CREATE TABLE test_grant(a tinyint primary key auto_increment)");
  myquery(rc);

  strxmov(query, "CREATE USER 'test_grant'@",
          opt_host ? opt_host : "'localhost'", NullS);
  rc = mysql_query(mysql, query);
  myquery(rc);

  strxmov(query, "GRANT INSERT, UPDATE, SELECT ON ", current_db,
          ".test_grant TO 'test_grant'@", opt_host ? opt_host : "'localhost'",
          NullS);

  if (mysql_query(mysql, query)) {
    myerror("GRANT failed");

    /*
       If server started with --skip-grant-tables, skip this test, else
       exit to indicate an error

       ER_UNKNOWN_COM_ERROR= 1047
     */
    if (mysql_errno(mysql) != 1047) exit(1);
  } else {
    MYSQL *org_mysql = mysql, *lmysql;
    MYSQL_STMT *stmt;

    if (!opt_silent) fprintf(stdout, "\n Establishing a test connection ...");
    if (!(lmysql = mysql_client_init(nullptr))) {
      myerror("mysql_client_init() failed");
      exit(1);
    }
    if (!(mysql_real_connect(lmysql, opt_host, "test_grant", "", current_db,
                             opt_port, opt_unix_socket, 0))) {
      myerror("connection failed");
      mysql_close(lmysql);
      exit(1);
    }
    lmysql->reconnect = true;
    if (!opt_silent) fprintf(stdout, "OK");

    mysql = lmysql;
    rc = mysql_query(mysql, "INSERT INTO test_grant VALUES(NULL)");
    myquery(rc);

    rc = mysql_query(mysql, "INSERT INTO test_grant(a) VALUES(NULL)");
    myquery(rc);

    execute_prepare_query("INSERT INTO test_grant(a) VALUES(NULL)", 1);
    execute_prepare_query("INSERT INTO test_grant VALUES(NULL)", 1);
    execute_prepare_query("UPDATE test_grant SET a=9 WHERE a=1", 1);
    rc = my_stmt_result("SELECT a FROM test_grant");
    DIE_UNLESS(rc == 4);

    /* Both DELETE expected to fail as user does not have DELETE privs */

    rc = mysql_query(mysql, "DELETE FROM test_grant");
    myquery_r(rc);

    stmt = mysql_simple_prepare(mysql, "DELETE FROM test_grant");
    check_stmt_r(stmt);

    rc = my_stmt_result("SELECT * FROM test_grant");
    DIE_UNLESS(rc == 4);

    mysql_close(lmysql);
    mysql = org_mysql;

    rc = mysql_query(mysql, "delete from mysql.user where User='test_grant'");
    myquery(rc);
    DIE_UNLESS(1 == mysql_affected_rows(mysql));

    rc = mysql_query(mysql,
                     "delete from mysql.tables_priv where User='test_grant'");
    myquery(rc);
    DIE_UNLESS(1 == mysql_affected_rows(mysql));
  }
}

/* Test DECIMAL conversion */

static void test_decimal_bug() {
  MYSQL_STMT *stmt;
  MYSQL_BIND my_bind[1];
  char data[30];
  int rc;
  bool is_null;

  myheader("test_decimal_bug");

  mysql_autocommit(mysql, true);

  rc = mysql_query(mysql, "drop table if exists test_decimal_bug");
  myquery(rc);

  rc = mysql_query(mysql, "create table test_decimal_bug(c1 decimal(10, 2))");
  myquery(rc);

  rc = mysql_query(mysql,
                   "insert into test_decimal_bug value(8), (10.22), (5.61)");
  myquery(rc);

  stmt = mysql_simple_prepare(mysql,
                              "select c1 from test_decimal_bug where c1= ?");
  check_stmt(stmt);

  /*
    We need to memset bind structure because mysql_stmt_bind_param checks all
    its members.
  */
  memset(my_bind, 0, sizeof(my_bind));
  memset(data, 0, sizeof(data));

  my_bind[0].buffer_type = MYSQL_TYPE_NEWDECIMAL;
  my_bind[0].buffer = (void *)data;
  my_bind[0].buffer_length = 25;
  my_bind[0].is_null = &is_null;

  is_null = false;
  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  my_stpcpy(data, "8.0");
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  data[0] = 0;
  rc = mysql_stmt_bind_result(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  if (!opt_silent) fprintf(stdout, "\n data: %s", data);
  DIE_UNLESS(strcmp(data, "8.00") == 0);

  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == MYSQL_NO_DATA);

  my_stpcpy(data, "5.61");
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  data[0] = 0;
  rc = mysql_stmt_bind_result(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  if (!opt_silent) fprintf(stdout, "\n data: %s", data);
  DIE_UNLESS(strcmp(data, "5.61") == 0);

  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == MYSQL_NO_DATA);

  is_null = true;
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == MYSQL_NO_DATA);

  my_stpcpy(data, "10.22");
  is_null = false;
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  data[0] = 0;
  rc = mysql_stmt_bind_result(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  if (!opt_silent) fprintf(stdout, "\n data: %s", data);
  DIE_UNLESS(strcmp(data, "10.22") == 0);

  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == MYSQL_NO_DATA);

  mysql_stmt_close(stmt);
}

/* Test EXPLAIN bug, bug #115 */

static void test_explain_bug() {
  MYSQL_STMT *stmt;
  MYSQL_RES *result;
  int rc;
  int no;

  myheader("test_explain_bug");

  mysql_autocommit(mysql, true);

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_explain");
  myquery(rc);

  rc = mysql_query(mysql, "CREATE TABLE test_explain(id int, name char(2))");
  myquery(rc);

  stmt = mysql_simple_prepare(mysql, "explain test_explain");
  check_stmt(stmt);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = my_process_stmt_result(stmt);
  DIE_UNLESS(rc == 2);

  result = mysql_stmt_result_metadata(stmt);
  mytest(result);

  if (!opt_silent)
    fprintf(stdout, "\n total fields in the result: %d",
            mysql_num_fields(result));
  DIE_UNLESS(6 == mysql_num_fields(result));

  verify_prepare_field(result, 0, "Field", "Field",
                       mysql_get_server_version(mysql) <= 50000
                           ? MYSQL_TYPE_STRING
                           : MYSQL_TYPE_VAR_STRING,
                       0, 0, "", 64, 0);

  verify_prepare_field(result, 1, "Type", "Type", MYSQL_TYPE_BLOB, 0, 0,
                       "mysql", 0, 0);

  verify_prepare_field(result, 2, "Null", "Null",
                       mysql_get_server_version(mysql) <= 50000
                           ? MYSQL_TYPE_STRING
                           : MYSQL_TYPE_VAR_STRING,
                       0, 0, "", 3, 0);

  verify_prepare_field(result, 3, "Key", "Key", MYSQL_TYPE_STRING, 0, 0,
                       "mysql", 3, 0);

  if (mysql_get_server_version(mysql) >= 50027) {
    /*  The patch for bug#23037 changes column type of DEAULT to blob */
    verify_prepare_field(result, 4, "Default", "Default", MYSQL_TYPE_BLOB, 0, 0,
                         "mysql", 0, 0);
  } else {
    verify_prepare_field(
        result, 4, "Default", "Default",
        mysql_get_server_version(mysql) >= 50027
            ? MYSQL_TYPE_BLOB
            : mysql_get_server_version(mysql) <= 50000 ? MYSQL_TYPE_STRING
                                                       : MYSQL_TYPE_VAR_STRING,
        0, 0, "mysql", mysql_get_server_version(mysql) >= 50027 ? 0 : 64, 0);
  }

  verify_prepare_field(
      result, 5, "Extra", "Extra",
      mysql_get_server_version(mysql) <= 50000 ? MYSQL_TYPE_STRING
                                               : MYSQL_TYPE_VAR_STRING,
      0, 0, "", mysql_get_server_version(mysql) <= 50602 ? 27 : 256, 0);

  mysql_free_result(result);
  mysql_stmt_close(stmt);

  stmt =
      mysql_simple_prepare(mysql, "explain select id, name FROM test_explain");
  check_stmt(stmt);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = my_process_stmt_result(stmt);
  DIE_UNLESS(rc == 1);

  result = mysql_stmt_result_metadata(stmt);
  mytest(result);

  if (!opt_silent)
    fprintf(stdout, "\n total fields in the result: %d",
            mysql_num_fields(result));
  if (mysql_get_server_version(mysql) <= 50702)
    DIE_UNLESS(10 == mysql_num_fields(result));
  else
    DIE_UNLESS(12 == mysql_num_fields(result));

  no = 0;

  verify_prepare_field(result, no++, "id", "", MYSQL_TYPE_LONGLONG, "", "", "",
                       3, 0);

  verify_prepare_field(result, no++, "select_type", "", MYSQL_TYPE_VAR_STRING,
                       "", "", "", 19, 0);

  verify_prepare_field(result, no++, "table", "", MYSQL_TYPE_VAR_STRING, "", "",
                       "", NAME_CHAR_LEN, 0);

  if (mysql_get_server_version(mysql) > 50702) no++;

  verify_prepare_field(result, no++, "type", "", MYSQL_TYPE_VAR_STRING, "", "",
                       "", 10, 0);

  verify_prepare_field(result, no++, "possible_keys", "", MYSQL_TYPE_VAR_STRING,
                       "", "", "", NAME_CHAR_LEN * MAX_KEY, 0);

  verify_prepare_field(result, no++, "key", "", MYSQL_TYPE_VAR_STRING, "", "",
                       "", NAME_CHAR_LEN, 0);

  if (mysql_get_server_version(mysql) <= 50000) {
    verify_prepare_field(result, no++, "key_len", "", MYSQL_TYPE_LONGLONG, "",
                         "", "", 3, 0);
  } else {
    verify_prepare_field(result, no++, "key_len", "", MYSQL_TYPE_VAR_STRING, "",
                         "", "", NAME_CHAR_LEN * MAX_KEY, 0);
  }

  verify_prepare_field(result, no++, "ref", "", MYSQL_TYPE_VAR_STRING, "", "",
                       "", NAME_CHAR_LEN * 16, 0);

  verify_prepare_field(result, no++, "rows", "", MYSQL_TYPE_LONGLONG, "", "",
                       "", 10, 0);

  if (mysql_get_server_version(mysql) > 50702) no++;

  verify_prepare_field(result, no++, "Extra", "", MYSQL_TYPE_VAR_STRING, "", "",
                       "", 255, 0);

  mysql_free_result(result);
  mysql_stmt_close(stmt);
}

/* Test warnings for truncated rows */

static void test_truncated_rows() {
  int rc, count;
  MYSQL_RES *result;

  myheader("test_truncated_rows");

  mysql_query(mysql, "DROP TABLE if exists t1");
  mysql_query(mysql, "DROP TABLE if exists t2");

  rc = mysql_query(mysql, "CREATE TABLE t1(c1 tinyint)");
  myquery(rc);

  rc = mysql_query(mysql, "CREATE TABLE t2(c1 int not null)");
  myquery(rc);

  rc = mysql_query(mysql, "INSERT INTO t1 values(10), (NULL), (NULL)");
  myquery(rc);

  count = mysql_warning_count(mysql);
  if (!opt_silent) fprintf(stdout, "\n total warnings: %d", count);
  DIE_UNLESS(count == 0);

  rc = mysql_query(mysql, "INSERT INTO t2 SELECT * FROM t1");
  myquery(rc);

  count = mysql_warning_count(mysql);
  if (!opt_silent) fprintf(stdout, "\n total warnings: %d", count);
  // Number of warnings changed in mysql-5.7
  DIE_UNLESS(count == (mysql_get_server_version(mysql) < 50700 ? 2 : 1));

  rc = mysql_query(mysql, "SHOW WARNINGS");
  myquery(rc);

  result = mysql_store_result(mysql);
  mytest(result);

  rc = my_process_result_set(result);
  // Number of warnings changed in mysql-5.7
  DIE_UNLESS(rc == (mysql_get_server_version(mysql) < 50700 ? 2 : 1));
  mysql_free_result(result);

  rc = mysql_query(mysql, "INSERT INTO t1 VALUES('junk'), (876789)");
  myquery(rc);

  count = mysql_warning_count(mysql);
  if (!opt_silent) fprintf(stdout, "\n total warnings: %d", count);
  DIE_UNLESS(count == 2);

  rc = mysql_query(mysql, "SHOW WARNINGS");
  myquery(rc);

  result = mysql_store_result(mysql);
  mytest(result);

  rc = my_process_result_set(result);
  DIE_UNLESS(rc == 2);
  mysql_free_result(result);
}

/* Test update/binary logs */

static void test_logs() {
  MYSQL_STMT *stmt;
  MYSQL_BIND my_bind[2];
  char data[255];
  ulong length;
  int rc;
  short id;

  myheader("test_logs");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_logs");
  myquery(rc);

  rc = mysql_query(mysql,
                   "CREATE TABLE test_logs(id smallint, name varchar(20))");
  myquery(rc);

  my_stpcpy((char *)data, "INSERT INTO test_logs VALUES(?, ?)");
  stmt = mysql_simple_prepare(mysql, data);
  check_stmt(stmt);

  /*
    We need to memset bind structure because mysql_stmt_bind_param checks all
    its members.
  */
  memset(my_bind, 0, sizeof(my_bind));

  my_bind[0].buffer_type = MYSQL_TYPE_SHORT;
  my_bind[0].buffer = (void *)&id;

  my_bind[1].buffer_type = MYSQL_TYPE_STRING;
  my_bind[1].buffer = (void *)&data;
  my_bind[1].buffer_length = 255;
  my_bind[1].length = &length;

  id = 9876;
  length =
      (ulong)(my_stpcpy((char *)data, "MySQL - Open Source Database") - data);

  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  my_stpcpy((char *)data, "'");
  length = 1;

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  my_stpcpy((char *)data, "\"");
  length = 1;

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  length = (ulong)(my_stpcpy((char *)data, "my\'sql\'") - data);
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  length = (ulong)(my_stpcpy((char *)data, "my\"sql\"") - data);
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  mysql_stmt_close(stmt);

  my_stpcpy((char *)data, "INSERT INTO test_logs VALUES(20, 'mysql')");
  stmt = mysql_simple_prepare(mysql, data);
  check_stmt(stmt);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  mysql_stmt_close(stmt);

  my_stpcpy((char *)data, "SELECT * FROM test_logs WHERE id=?");
  stmt = mysql_simple_prepare(mysql, data);
  check_stmt(stmt);

  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  my_bind[1].buffer_length = 255;
  rc = mysql_stmt_bind_result(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  if (!opt_silent) {
    fprintf(stdout, "id    : %d\n", id);
    fprintf(stdout, "name  : %s(%ld)\n", data, (long)length);
  }

  DIE_UNLESS(id == 9876);
  DIE_UNLESS(length == 19 || length == 20); /* Due to VARCHAR(20) */
  DIE_UNLESS(is_prefix(data, "MySQL - Open Source") == 1);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  if (!opt_silent) fprintf(stdout, "\n name  : %s(%ld)", data, (long)length);

  DIE_UNLESS(length == 1);
  DIE_UNLESS(strcmp(data, "'") == 0);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  if (!opt_silent) fprintf(stdout, "\n name  : %s(%ld)", data, (long)length);

  DIE_UNLESS(length == 1);
  DIE_UNLESS(strcmp(data, "\"") == 0);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  if (!opt_silent) fprintf(stdout, "\n name  : %s(%ld)", data, (long)length);

  DIE_UNLESS(length == 7);
  DIE_UNLESS(strcmp(data, "my\'sql\'") == 0);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  if (!opt_silent) fprintf(stdout, "\n name  : %s(%ld)", data, (long)length);

  DIE_UNLESS(length == 7);
  /*DIE_UNLESS(strcmp(data, "my\"sql\"") == 0); */

  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == MYSQL_NO_DATA);

  mysql_stmt_close(stmt);

  rc = mysql_query(mysql, "DROP TABLE test_logs");
  myquery(rc);
}

/* Test 'n' statements create and close */

static void test_nstmts() {
  MYSQL_STMT *stmt;
  char query[255];
  int rc;
  static uint i, total_stmts = 2000;
  MYSQL_BIND my_bind[1];

  myheader("test_nstmts");

  mysql_autocommit(mysql, true);

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_nstmts");
  myquery(rc);

  rc = mysql_query(mysql, "CREATE TABLE test_nstmts(id int)");
  myquery(rc);

  /*
    We need to memset bind structure because mysql_stmt_bind_param checks all
    its members.
  */
  memset(my_bind, 0, sizeof(my_bind));

  my_bind[0].buffer = (void *)&i;
  my_bind[0].buffer_type = MYSQL_TYPE_LONG;

  for (i = 0; i < total_stmts; i++) {
    if (!opt_silent) fprintf(stdout, "\r stmt: %d", i);

    my_stpcpy(query, "insert into test_nstmts values(?)");
    stmt = mysql_simple_prepare(mysql, query);
    check_stmt(stmt);

    rc = mysql_stmt_bind_param(stmt, my_bind);
    check_execute(stmt, rc);

    rc = mysql_stmt_execute(stmt);
    check_execute(stmt, rc);

    mysql_stmt_close(stmt);
  }

  stmt = mysql_simple_prepare(mysql, " select count(*) from test_nstmts");
  check_stmt(stmt);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  i = 0;
  rc = mysql_stmt_bind_result(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);
  if (!opt_silent) fprintf(stdout, "\n total rows: %d", i);
  DIE_UNLESS(i == total_stmts);

  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == MYSQL_NO_DATA);

  mysql_stmt_close(stmt);

  rc = mysql_query(mysql, "DROP TABLE test_nstmts");
  myquery(rc);
}

/* Test stmt seek() functions */

static void test_fetch_seek() {
  MYSQL_STMT *stmt;
  MYSQL_BIND my_bind[3];
  MYSQL_ROW_OFFSET row;
  int rc;
  int32 c1;
  char c2[11], c3[20];

  myheader("test_fetch_seek");
  rc = mysql_query(mysql, "drop table if exists t1");

  myquery(rc);

  rc = mysql_query(mysql,
                   "create table t1(c1 int primary key auto_increment, c2 "
                   "char(10), c3 timestamp)");
  myquery(rc);

  rc = mysql_query(mysql,
                   "insert into t1(c2, c3) values('venu', CURRENT_TIMESTAMP), \
                                                       ('mysql', CURRENT_TIMESTAMP), \
                                                       ('open', CURRENT_TIMESTAMP), \
                                                       ('source', CURRENT_TIMESTAMP)");

  myquery(rc);

  stmt = mysql_simple_prepare(mysql, "select * from t1");
  check_stmt(stmt);

  memset(my_bind, 0, sizeof(my_bind));
  my_bind[0].buffer_type = MYSQL_TYPE_LONG;
  my_bind[0].buffer = (void *)&c1;

  my_bind[1].buffer_type = MYSQL_TYPE_STRING;
  my_bind[1].buffer = (void *)c2;
  my_bind[1].buffer_length = (ulong)sizeof(c2);

  my_bind[2] = my_bind[1];
  my_bind[2].buffer = (void *)c3;
  my_bind[2].buffer_length = (ulong)sizeof(c3);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_bind_result(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_store_result(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  if (!opt_silent) fprintf(stdout, "\n row 0: %ld, %s, %s", (long)c1, c2, c3);

  row = mysql_stmt_row_tell(stmt);

  row = mysql_stmt_row_seek(stmt, row);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  if (!opt_silent) fprintf(stdout, "\n row 2: %ld, %s, %s", (long)c1, c2, c3);

  row = mysql_stmt_row_seek(stmt, row);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  if (!opt_silent) fprintf(stdout, "\n row 2: %ld, %s, %s", (long)c1, c2, c3);

  mysql_stmt_data_seek(stmt, 0);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  if (!opt_silent) fprintf(stdout, "\n row 0: %ld, %s, %s", (long)c1, c2, c3);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == MYSQL_NO_DATA);

  mysql_stmt_close(stmt);
  myquery(mysql_query(mysql, "drop table t1"));
}

/* Test mysql_stmt_fetch_column() with offset */

static void test_fetch_offset() {
  MYSQL_STMT *stmt;
  MYSQL_BIND my_bind[1];
  char data[11];
  ulong length;
  int rc;
  bool is_null;

  myheader("test_fetch_offset");

  rc = mysql_query(mysql, "drop table if exists t1");
  myquery(rc);

  rc = mysql_query(mysql, "create table t1(a char(10))");
  myquery(rc);

  rc = mysql_query(mysql, "insert into t1 values('abcdefghij'), (null)");
  myquery(rc);

  stmt = mysql_simple_prepare(mysql, "select * from t1");
  check_stmt(stmt);

  memset(my_bind, 0, sizeof(my_bind));
  my_bind[0].buffer_type = MYSQL_TYPE_STRING;
  my_bind[0].buffer = (void *)data;
  my_bind[0].buffer_length = 11;
  my_bind[0].is_null = &is_null;
  my_bind[0].length = &length;

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch_column(stmt, my_bind, 0, 0);
  check_execute_r(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_bind_result(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_store_result(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  data[0] = '\0';
  rc = mysql_stmt_fetch_column(stmt, my_bind, 0, 0);
  check_execute(stmt, rc);
  if (!opt_silent) fprintf(stdout, "\n col 1: %s (%ld)", data, (long)length);
  DIE_UNLESS(strncmp(data, "abcd", 4) == 0 && length == 10);

  rc = mysql_stmt_fetch_column(stmt, my_bind, 0, 5);
  check_execute(stmt, rc);
  if (!opt_silent) fprintf(stdout, "\n col 1: %s (%ld)", data, (long)length);
  DIE_UNLESS(strncmp(data, "fg", 2) == 0 && length == 10);

  rc = mysql_stmt_fetch_column(stmt, my_bind, 0, 9);
  check_execute(stmt, rc);
  if (!opt_silent) fprintf(stdout, "\n col 0: %s (%ld)", data, (long)length);
  DIE_UNLESS(strncmp(data, "j", 1) == 0 && length == 10);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  is_null = false;

  rc = mysql_stmt_fetch_column(stmt, my_bind, 0, 0);
  check_execute(stmt, rc);

  DIE_UNLESS(is_null == 1);

  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == MYSQL_NO_DATA);

  rc = mysql_stmt_fetch_column(stmt, my_bind, 1, 0);
  check_execute_r(stmt, rc);

  mysql_stmt_close(stmt);

  myquery(mysql_query(mysql, "drop table t1"));
}

/* Test mysql_stmt_fetch_column() */

static void test_fetch_column() {
  MYSQL_STMT *stmt;
  MYSQL_BIND my_bind[2];
  char c2[20], bc2[20];
  ulong l1, l2, bl1, bl2;
  int rc, c1, bc1;

  myheader("test_fetch_column");

  rc = mysql_query(mysql, "drop table if exists t1");
  myquery(rc);

  rc = mysql_query(
      mysql, "create table t1(c1 int primary key auto_increment, c2 char(10))");
  myquery(rc);

  rc = mysql_query(mysql, "insert into t1(c2) values('venu'), ('mysql')");
  myquery(rc);

  stmt = mysql_simple_prepare(mysql, "select * from t1 order by c2 desc");
  check_stmt(stmt);

  memset(my_bind, 0, sizeof(my_bind));
  my_bind[0].buffer_type = MYSQL_TYPE_LONG;
  my_bind[0].buffer = (void *)&bc1;
  my_bind[0].buffer_length = 0;
  my_bind[0].is_null = nullptr;
  my_bind[0].length = &bl1;
  my_bind[1].buffer_type = MYSQL_TYPE_STRING;
  my_bind[1].buffer = (void *)bc2;
  my_bind[1].buffer_length = 7;
  my_bind[1].is_null = nullptr;
  my_bind[1].length = &bl2;

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_bind_result(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_store_result(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch_column(stmt, my_bind, 1, 0); /* No-op at this point */
  check_execute_r(stmt, rc);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  if (!opt_silent) fprintf(stdout, "\n row 0: %d, %s", bc1, bc2);

  c2[0] = '\0';
  l2 = 0;
  my_bind[0].buffer_type = MYSQL_TYPE_STRING;
  my_bind[0].buffer = (void *)c2;
  my_bind[0].buffer_length = 7;
  my_bind[0].is_null = nullptr;
  my_bind[0].length = &l2;

  rc = mysql_stmt_fetch_column(stmt, my_bind, 1, 0);
  check_execute(stmt, rc);
  if (!opt_silent) fprintf(stdout, "\n col 1: %s(%ld)", c2, (long)l2);
  DIE_UNLESS(strncmp(c2, "venu", 4) == 0 && l2 == 4);

  c2[0] = '\0';
  l2 = 0;
  rc = mysql_stmt_fetch_column(stmt, my_bind, 1, 0);
  check_execute(stmt, rc);
  if (!opt_silent) fprintf(stdout, "\n col 1: %s(%ld)", c2, (long)l2);
  DIE_UNLESS(strcmp(c2, "venu") == 0 && l2 == 4);

  c1 = 0;
  my_bind[0].buffer_type = MYSQL_TYPE_LONG;
  my_bind[0].buffer = (void *)&c1;
  my_bind[0].buffer_length = 0;
  my_bind[0].is_null = nullptr;
  my_bind[0].length = &l1;

  rc = mysql_stmt_fetch_column(stmt, my_bind, 0, 0);
  check_execute(stmt, rc);
  if (!opt_silent) fprintf(stdout, "\n col 0: %d(%ld)", c1, (long)l1);
  DIE_UNLESS(c1 == 1 && l1 == 4);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  if (!opt_silent) fprintf(stdout, "\n row 1: %d, %s", bc1, bc2);

  c2[0] = '\0';
  l2 = 0;
  my_bind[0].buffer_type = MYSQL_TYPE_STRING;
  my_bind[0].buffer = (void *)c2;
  my_bind[0].buffer_length = 7;
  my_bind[0].is_null = nullptr;
  my_bind[0].length = &l2;

  rc = mysql_stmt_fetch_column(stmt, my_bind, 1, 0);
  check_execute(stmt, rc);
  if (!opt_silent) fprintf(stdout, "\n col 1: %s(%ld)", c2, (long)l2);
  DIE_UNLESS(strncmp(c2, "mysq", 4) == 0 && l2 == 5);

  c2[0] = '\0';
  l2 = 0;
  rc = mysql_stmt_fetch_column(stmt, my_bind, 1, 0);
  check_execute(stmt, rc);
  if (!opt_silent) fprintf(stdout, "\n col 1: %si(%ld)", c2, (long)l2);
  DIE_UNLESS(strcmp(c2, "mysql") == 0 && l2 == 5);

  c1 = 0;
  my_bind[0].buffer_type = MYSQL_TYPE_LONG;
  my_bind[0].buffer = (void *)&c1;
  my_bind[0].buffer_length = 0;
  my_bind[0].is_null = nullptr;
  my_bind[0].length = &l1;

  rc = mysql_stmt_fetch_column(stmt, my_bind, 0, 0);
  check_execute(stmt, rc);
  if (!opt_silent) fprintf(stdout, "\n col 0: %d(%ld)", c1, (long)l1);
  DIE_UNLESS(c1 == 2 && l1 == 4);

  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == MYSQL_NO_DATA);

  rc = mysql_stmt_fetch_column(stmt, my_bind, 1, 0);
  check_execute_r(stmt, rc);

  mysql_stmt_close(stmt);
  myquery(mysql_query(mysql, "drop table t1"));
}

/* Test mysql_list_fields() */

static void test_list_fields() {
  MYSQL_RES *result;
  int rc;
  myheader("test_list_fields");

  rc = mysql_query(mysql, "drop table if exists t1");
  myquery(rc);

  rc = mysql_query(mysql,
                   "create table t1(c1 int primary key auto_increment, c2 "
                   "char(10) default 'mysql')");
  myquery(rc);

  result = mysql_list_fields(mysql, "t1", nullptr);
  mytest(result);

  rc = my_process_result_set(result);
  DIE_UNLESS(rc == 0);

  verify_prepare_field(result, 0, "c1", "c1", MYSQL_TYPE_LONG, "t1", "t1",
                       current_db, 11, "0");

  verify_prepare_field(result, 1, "c2", "c2", MYSQL_TYPE_STRING, "t1", "t1",
                       current_db, 10, "mysql");

  mysql_free_result(result);
  myquery(mysql_query(mysql, "drop table t1"));
}

static void test_bug19671() {
  MYSQL_RES *result;
  int rc;
  myheader("test_bug19671");

  mysql_query(mysql, "set sql_mode=''");
  rc = mysql_query(mysql, "drop table if exists t1");
  myquery(rc);

  rc = mysql_query(mysql, "drop view if exists v1");
  myquery(rc);

  rc = mysql_query(mysql, "create table t1(f1 int)");
  myquery(rc);

  rc = mysql_query(mysql, "create view v1 as select va.* from t1 va");
  myquery(rc);

  result = mysql_list_fields(mysql, "v1", nullptr);
  mytest(result);

  rc = my_process_result_set(result);
  DIE_UNLESS(rc == 0);

  verify_prepare_field(result, 0, "f1", "f1", MYSQL_TYPE_LONG, "v1", "v1",
                       current_db, 11, "0");

  mysql_free_result(result);
  myquery(mysql_query(mysql, "drop view v1"));
  myquery(mysql_query(mysql, "drop table t1"));
}

/* Test a memory ovverun bug */

static void test_mem_overun() {
  char buffer[10000], field[10];
  MYSQL_STMT *stmt;
  MYSQL_RES *field_res;
  int rc;
  ulong length;

  myheader("test_mem_overun");

  /*
    Test a memory ovverun bug when a table had 1000 fields with
    a row of data
  */
  rc = mysql_query(mysql, "drop table if exists t_mem_overun");
  myquery(rc);

  strxmov(buffer, "create table t_mem_overun(", NullS);
  for (unsigned i = 0; i < 1000; i++) {
    sprintf(field, "c%d int", i);
    strxmov(buffer, buffer, field, ", ", NullS);
  }
  length = (ulong)strlen(buffer);
  buffer[length - 2] = ')';
  buffer[--length] = '\0';

  strcat(buffer, " ENGINE = MyISAM ");
  length = (ulong)strlen(buffer);
  rc = mysql_real_query(mysql, buffer, length);
  myquery(rc);

  strxmov(buffer, "insert into t_mem_overun values(", NullS);
  for (int i = 0; i < 1000; i++) {
    strxmov(buffer, buffer, "1, ", NullS);
  }
  length = (ulong)strlen(buffer);
  buffer[length - 2] = ')';
  buffer[--length] = '\0';

  rc = mysql_real_query(mysql, buffer, length);
  myquery(rc);

  rc = mysql_query(mysql, "select * from t_mem_overun");
  myquery(rc);

  rc = my_process_result(mysql);
  DIE_UNLESS(rc == 1);

  stmt = mysql_simple_prepare(mysql, "select * from t_mem_overun");
  check_stmt(stmt);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  field_res = mysql_stmt_result_metadata(stmt);
  mytest(field_res);

  if (!opt_silent)
    fprintf(stdout, "\n total fields : %d", mysql_num_fields(field_res));
  DIE_UNLESS(1000 == mysql_num_fields(field_res));

  rc = mysql_stmt_store_result(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == MYSQL_NO_DATA);

  mysql_free_result(field_res);

  mysql_stmt_close(stmt);
}

/* Test mysql_stmt_free_result() */

static void test_free_result() {
  MYSQL_STMT *stmt;
  MYSQL_BIND my_bind[1];
  char c2[5];
  ulong bl1, l2;
  int rc, c1, bc1;

  myheader("test_free_result");

  rc = mysql_query(mysql, "drop table if exists test_free_result");
  myquery(rc);

  rc = mysql_query(mysql,
                   "create table test_free_result("
                   "c1 int primary key auto_increment)");
  myquery(rc);

  rc = mysql_query(mysql, "insert into test_free_result values(), (), ()");
  myquery(rc);

  stmt = mysql_simple_prepare(mysql, "select * from test_free_result");
  check_stmt(stmt);

  memset(my_bind, 0, sizeof(my_bind));
  my_bind[0].buffer_type = MYSQL_TYPE_LONG;
  my_bind[0].buffer = (void *)&bc1;
  my_bind[0].length = &bl1;

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_bind_result(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  c2[0] = '\0';
  l2 = 0;
  my_bind[0].buffer_type = MYSQL_TYPE_STRING;
  my_bind[0].buffer = (void *)c2;
  my_bind[0].buffer_length = 7;
  my_bind[0].is_null = nullptr;
  my_bind[0].length = &l2;

  rc = mysql_stmt_fetch_column(stmt, my_bind, 0, 0);
  check_execute(stmt, rc);
  if (!opt_silent) fprintf(stdout, "\n col 0: %s(%ld)", c2, (long)l2);
  DIE_UNLESS(strncmp(c2, "1", 1) == 0 && l2 == 1);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  c1 = 0, l2 = 0;
  my_bind[0].buffer_type = MYSQL_TYPE_LONG;
  my_bind[0].buffer = (void *)&c1;
  my_bind[0].buffer_length = 0;
  my_bind[0].is_null = nullptr;
  my_bind[0].length = &l2;

  rc = mysql_stmt_fetch_column(stmt, my_bind, 0, 0);
  check_execute(stmt, rc);
  if (!opt_silent) fprintf(stdout, "\n col 0: %d(%ld)", c1, (long)l2);
  DIE_UNLESS(c1 == 2 && l2 == 4);

  rc = mysql_query(mysql, "drop table test_free_result");
  myquery_r(rc); /* error should be, COMMANDS OUT OF SYNC */

  rc = mysql_stmt_free_result(stmt);
  check_execute(stmt, rc);

  rc = mysql_query(mysql, "drop table test_free_result");
  myquery(rc); /* should be successful */

  mysql_stmt_close(stmt);
}

/* Test mysql_stmt_store_result() */

static void test_free_store_result() {
  MYSQL_STMT *stmt;
  MYSQL_BIND my_bind[1];
  char c2[5];
  ulong bl1, l2;
  int rc, c1, bc1;

  myheader("test_free_store_result");

  rc = mysql_query(mysql, "drop table if exists test_free_result");
  myquery(rc);

  rc = mysql_query(
      mysql,
      "create table test_free_result(c1 int primary key auto_increment)");
  myquery(rc);

  rc = mysql_query(mysql, "insert into test_free_result values(), (), ()");
  myquery(rc);

  stmt = mysql_simple_prepare(mysql, "select * from test_free_result");
  check_stmt(stmt);

  memset(my_bind, 0, sizeof(my_bind));
  my_bind[0].buffer_type = MYSQL_TYPE_LONG;
  my_bind[0].buffer = (void *)&bc1;
  my_bind[0].buffer_length = 0;
  my_bind[0].is_null = nullptr;
  my_bind[0].length = &bl1;

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_bind_result(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_store_result(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  c2[0] = '\0';
  l2 = 0;
  my_bind[0].buffer_type = MYSQL_TYPE_STRING;
  my_bind[0].buffer = (void *)c2;
  my_bind[0].buffer_length = 7;
  my_bind[0].is_null = nullptr;
  my_bind[0].length = &l2;

  rc = mysql_stmt_fetch_column(stmt, my_bind, 0, 0);
  check_execute(stmt, rc);
  if (!opt_silent) fprintf(stdout, "\n col 1: %s(%ld)", c2, (long)l2);
  DIE_UNLESS(strncmp(c2, "1", 1) == 0 && l2 == 1);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  c1 = 0, l2 = 0;
  my_bind[0].buffer_type = MYSQL_TYPE_LONG;
  my_bind[0].buffer = (void *)&c1;
  my_bind[0].buffer_length = 0;
  my_bind[0].is_null = nullptr;
  my_bind[0].length = &l2;

  rc = mysql_stmt_fetch_column(stmt, my_bind, 0, 0);
  check_execute(stmt, rc);
  if (!opt_silent) fprintf(stdout, "\n col 0: %d(%ld)", c1, (long)l2);
  DIE_UNLESS(c1 == 2 && l2 == 4);

  rc = mysql_stmt_free_result(stmt);
  check_execute(stmt, rc);

  rc = mysql_query(mysql, "drop table test_free_result");
  myquery(rc);

  mysql_stmt_close(stmt);
}

/* Test SQLmode */

static void test_sqlmode() {
  MYSQL_STMT *stmt;
  MYSQL_BIND my_bind[2];
  char c1[5], c2[5];
  int rc;
  char query[MAX_TEST_QUERY_LENGTH];

  myheader("test_sqlmode");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_piping");
  myquery(rc);

  rc = mysql_query(mysql, "CREATE TABLE test_piping(name varchar(10))");
  myquery(rc);

  /* PIPES_AS_CONCAT */
  my_stpcpy(query, "SET SQL_MODE= \"PIPES_AS_CONCAT\"");
  if (!opt_silent) fprintf(stdout, "\n With %s", query);
  rc = mysql_query(mysql, query);
  myquery(rc);

  my_stpcpy(query, "INSERT INTO test_piping VALUES(?||?)");
  if (!opt_silent) fprintf(stdout, "\n  query: %s", query);
  stmt = mysql_simple_prepare(mysql, query);
  check_stmt(stmt);

  if (!opt_silent)
    fprintf(stdout, "\n  total parameters: %ld", mysql_stmt_param_count(stmt));

  /*
    We need to memset bind structure because mysql_stmt_bind_param checks all
    its members.
  */
  memset(my_bind, 0, sizeof(my_bind));

  my_bind[0].buffer_type = MYSQL_TYPE_STRING;
  my_bind[0].buffer = (void *)c1;
  my_bind[0].buffer_length = 2;

  my_bind[1].buffer_type = MYSQL_TYPE_STRING;
  my_bind[1].buffer = (void *)c2;
  my_bind[1].buffer_length = 3;

  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  my_stpcpy(c1, "My");
  my_stpcpy(c2, "SQL");
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);
  mysql_stmt_close(stmt);

  verify_col_data("test_piping", "name", "MySQL");

  rc = mysql_query(mysql, "DELETE FROM test_piping");
  myquery(rc);

  my_stpcpy(query, "SELECT connection_id    ()");
  if (!opt_silent) fprintf(stdout, "\n  query: %s", query);
  stmt = mysql_simple_prepare(mysql, query);
  check_stmt(stmt);
  mysql_stmt_close(stmt);

  /* ANSI */
  my_stpcpy(query, "SET SQL_MODE= \"ANSI\"");
  if (!opt_silent) fprintf(stdout, "\n With %s", query);
  rc = mysql_query(mysql, query);
  myquery(rc);

  my_stpcpy(query, "INSERT INTO test_piping VALUES(?||?)");
  if (!opt_silent) fprintf(stdout, "\n  query: %s", query);
  stmt = mysql_simple_prepare(mysql, query);
  check_stmt(stmt);
  if (!opt_silent)
    fprintf(stdout, "\n  total parameters: %ld", mysql_stmt_param_count(stmt));

  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  my_stpcpy(c1, "My");
  my_stpcpy(c2, "SQL");
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  mysql_stmt_close(stmt);
  verify_col_data("test_piping", "name", "MySQL");

  /* ANSI mode spaces ... */
  my_stpcpy(query, "SELECT connection_id    ()");
  if (!opt_silent) fprintf(stdout, "\n  query: %s", query);
  stmt = mysql_simple_prepare(mysql, query);
  check_stmt(stmt);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == MYSQL_NO_DATA);
  if (!opt_silent) fprintf(stdout, "\n  returned 1 row\n");

  mysql_stmt_close(stmt);

  /* IGNORE SPACE MODE */
  my_stpcpy(query, "SET SQL_MODE= \"IGNORE_SPACE\"");
  if (!opt_silent) fprintf(stdout, "\n With %s", query);
  rc = mysql_query(mysql, query);
  myquery(rc);

  my_stpcpy(query, "SELECT connection_id    ()");
  if (!opt_silent) fprintf(stdout, "\n  query: %s", query);
  stmt = mysql_simple_prepare(mysql, query);
  check_stmt(stmt);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == MYSQL_NO_DATA);
  if (!opt_silent) fprintf(stdout, "\n  returned 1 row");

  mysql_stmt_close(stmt);
}

/* Test for timestamp handling */

static void test_ts() {
  MYSQL_STMT *stmt;
  MYSQL_BIND my_bind[6];
  MYSQL_TIME ts;
  MYSQL_RES *prep_res;
  char strts[30];
  ulong length;
  int rc, field_count;
  char name;
  char query[MAX_TEST_QUERY_LENGTH];
  const char *queries[3] = {
      "SELECT a, b, c FROM test_ts WHERE %c=?",
      "SELECT a, b, c FROM test_ts WHERE %c=?",
      "SELECT a, b, c FROM test_ts WHERE %c=CAST(? AS DATE)"};
  myheader("test_ts");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_ts");
  myquery(rc);

  rc = mysql_query(mysql, "CREATE TABLE test_ts(a DATE, b TIME, c TIMESTAMP)");
  myquery(rc);

  stmt = mysql_simple_prepare(mysql,
                              "INSERT INTO test_ts VALUES(?, ?, ?), (?, ?, ?)");
  check_stmt(stmt);

  ts.year = 2003;
  ts.month = 07;
  ts.day = 12;
  ts.hour = 21;
  ts.minute = 07;
  ts.second = 46;
  ts.second_part = 0;
  ts.time_type = MYSQL_TIMESTAMP_DATETIME;
  length = (long)(my_stpcpy(strts, "2003-07-12 21:07:46") - strts);

  /*
    We need to memset bind structure because mysql_stmt_bind_param checks all
    its members.
  */
  memset(my_bind, 0, sizeof(my_bind));

  my_bind[0].buffer_type = MYSQL_TYPE_TIMESTAMP;
  my_bind[0].buffer = (void *)&ts;
  my_bind[0].buffer_length = (ulong)sizeof(ts);

  my_bind[2] = my_bind[1] = my_bind[0];

  my_bind[3].buffer_type = MYSQL_TYPE_STRING;
  my_bind[3].buffer = (void *)strts;
  my_bind[3].buffer_length = (ulong)sizeof(strts);
  my_bind[3].length = &length;

  my_bind[5] = my_bind[4] = my_bind[3];

  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  mysql_stmt_close(stmt);

  verify_col_data("test_ts", "a", "2003-07-12");
  verify_col_data("test_ts", "b", "21:07:46");
  verify_col_data("test_ts", "c", "2003-07-12 21:07:46");

  stmt = mysql_simple_prepare(mysql, "SELECT * FROM test_ts");
  check_stmt(stmt);

  prep_res = mysql_stmt_result_metadata(stmt);
  mytest(prep_res);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = my_process_stmt_result(stmt);
  DIE_UNLESS(rc == 2);
  field_count = mysql_num_fields(prep_res);

  mysql_free_result(prep_res);
  mysql_stmt_close(stmt);

  for (name = 'a'; field_count--; name++) {
    int row_count = 0;

    sprintf(query, queries[field_count], name);

    if (!opt_silent) fprintf(stdout, "\n  %s", query);
    stmt = mysql_simple_prepare(mysql, query);
    check_stmt(stmt);

    rc = mysql_stmt_bind_param(stmt, my_bind);
    check_execute(stmt, rc);

    rc = mysql_stmt_execute(stmt);
    check_execute(stmt, rc);

    while (mysql_stmt_fetch(stmt) == 0) row_count++;

    if (!opt_silent) fprintf(stdout, "\n   returned '%d' rows", row_count);
    DIE_UNLESS(row_count == 2);
    mysql_stmt_close(stmt);
  }
}

/* Test for bug #1500. */

static void test_bug1500() {
  MYSQL_STMT *stmt;
  MYSQL_BIND my_bind[3];
  int rc;
  int32 int_data[3] = {2, 3, 4};
  const char *data;

  myheader("test_bug1500");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_bg1500");
  myquery(rc);

  rc = mysql_query(mysql, "CREATE TABLE test_bg1500 (i INT)");
  myquery(rc);

  rc = mysql_query(mysql, "INSERT INTO test_bg1500 VALUES (1), (2)");
  myquery(rc);

  rc = mysql_commit(mysql);
  myquery(rc);

  stmt = mysql_simple_prepare(mysql,
                              "SELECT i FROM test_bg1500 WHERE i IN (?, ?, ?)");
  check_stmt(stmt);
  verify_param_count(stmt, 3);

  /*
    We need to memset bind structure because mysql_stmt_bind_param checks all
    its members.
  */
  memset(my_bind, 0, sizeof(my_bind));

  my_bind[0].buffer = (void *)int_data;
  my_bind[0].buffer_type = MYSQL_TYPE_LONG;
  my_bind[2] = my_bind[1] = my_bind[0];
  my_bind[1].buffer = (void *)(int_data + 1);
  my_bind[2].buffer = (void *)(int_data + 2);

  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = my_process_stmt_result(stmt);
  DIE_UNLESS(rc == 1);

  mysql_stmt_close(stmt);

  rc = mysql_query(mysql, "DROP TABLE test_bg1500");
  myquery(rc);

  rc = mysql_query(
      mysql,
      "CREATE TABLE test_bg1500 (s VARCHAR(25), FULLTEXT(s)) engine=MyISAM");
  myquery(rc);

  rc = mysql_query(mysql,
                   "INSERT INTO test_bg1500 VALUES ('Gravedigger'), ('Greed'), "
                   "('Hollow Dogs')");
  myquery(rc);

  rc = mysql_commit(mysql);
  myquery(rc);

  stmt = mysql_simple_prepare(
      mysql, "SELECT s FROM test_bg1500 WHERE MATCH (s) AGAINST (?)");
  check_stmt(stmt);

  verify_param_count(stmt, 1);

  data = "Dogs";
  my_bind[0].buffer_type = MYSQL_TYPE_STRING;
  my_bind[0].buffer = const_cast<char *>(data);
  my_bind[0].buffer_length = (ulong)strlen(data);
  my_bind[0].is_null = nullptr;
  my_bind[0].length = nullptr;

  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = my_process_stmt_result(stmt);
  DIE_UNLESS(rc == 1);

  mysql_stmt_close(stmt);

  /* This should work too */
  stmt = mysql_simple_prepare(mysql,
                              "SELECT s FROM test_bg1500 WHERE MATCH (s) "
                              "AGAINST (CONCAT(?, 'digger'))");
  check_stmt(stmt);

  verify_param_count(stmt, 1);

  data = "Grave";
  my_bind[0].buffer_type = MYSQL_TYPE_STRING;
  my_bind[0].buffer = const_cast<char *>(data);
  my_bind[0].buffer_length = (ulong)strlen(data);

  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = my_process_stmt_result(stmt);
  DIE_UNLESS(rc == 1);

  mysql_stmt_close(stmt);
}

static void test_bug1946() {
  MYSQL_STMT *stmt;
  int rc;
  const char *query = "INSERT INTO prepare_command VALUES (?)";

  myheader("test_bug1946");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS prepare_command");
  myquery(rc);

  rc = mysql_query(mysql, "CREATE TABLE prepare_command(ID INT)");
  myquery(rc);

  stmt = mysql_simple_prepare(mysql, query);
  check_stmt(stmt);
  rc = mysql_real_query(mysql, query, (ulong)strlen(query));
  DIE_UNLESS(rc != 0);
  if (!opt_silent) fprintf(stdout, "Got error (as expected):\n");
  myerror(nullptr);

  mysql_stmt_close(stmt);
  rc = mysql_query(mysql, "DROP TABLE prepare_command");
}

static void test_parse_error_and_bad_length() {
  MYSQL_STMT *stmt;
  int rc;

  /* check that we get 4 syntax errors over the 4 calls */
  myheader("test_parse_error_and_bad_length");

  rc = mysql_query(mysql, "SHOW DATABAAAA");
  DIE_UNLESS(rc);
  if (!opt_silent)
    fprintf(stdout, "Got error (as expected): '%s'\n", mysql_error(mysql));
  rc = mysql_real_query(mysql, "SHOW DATABASES", 12);  // Incorrect length.
  DIE_UNLESS(rc);
  if (!opt_silent)
    fprintf(stdout, "Got error (as expected): '%s'\n", mysql_error(mysql));

  stmt = mysql_simple_prepare(mysql, "SHOW DATABAAAA");
  DIE_UNLESS(!stmt);
  if (!opt_silent)
    fprintf(stdout, "Got error (as expected): '%s'\n", mysql_error(mysql));
  stmt = mysql_stmt_init(mysql);
  DIE_UNLESS(stmt);
  rc = mysql_stmt_prepare(stmt, "SHOW DATABASES", 12);  // Incorrect length.
  DIE_UNLESS(rc != 0);
  if (!opt_silent)
    fprintf(stdout, "Got error (as expected): '%s'\n", mysql_stmt_error(stmt));
  mysql_stmt_close(stmt);
}

static void test_bug2247() {
  MYSQL_STMT *stmt;
  MYSQL_RES *res;
  int rc;
  int i;
  const char *create = "CREATE TABLE bug2247(id INT UNIQUE AUTO_INCREMENT)";
  const char *insert = "INSERT INTO bug2247 VALUES (NULL)";
  const char *SELECT = "SELECT id FROM bug2247";
  const char *update = "UPDATE bug2247 SET id=id+10";
  const char *drop = "DROP TABLE IF EXISTS bug2247";
  ulonglong exp_count;
  enum { NUM_ROWS = 5 };

  myheader("test_bug2247");

  if (!opt_silent)
    fprintf(stdout,
            "\nChecking if stmt_affected_rows is not affected by\n"
            "mysql_query ... ");
  /* create table and insert few rows */
  rc = mysql_query(mysql, drop);
  myquery(rc);

  rc = mysql_query(mysql, create);
  myquery(rc);

  stmt = mysql_simple_prepare(mysql, insert);
  check_stmt(stmt);
  for (i = 0; i < NUM_ROWS; ++i) {
    rc = mysql_stmt_execute(stmt);
    check_execute(stmt, rc);
  }
  exp_count = mysql_stmt_affected_rows(stmt);
  DIE_UNLESS(exp_count == 1);

  rc = mysql_query(mysql, SELECT);
  myquery(rc);
  /*
    mysql_store_result overwrites mysql->affected_rows. Check that
    mysql_stmt_affected_rows() returns the same value, whereas
    mysql_affected_rows() value is correct.
  */
  res = mysql_store_result(mysql);
  mytest(res);

  DIE_UNLESS(mysql_affected_rows(mysql) == NUM_ROWS);
  DIE_UNLESS(exp_count == mysql_stmt_affected_rows(stmt));

  rc = mysql_query(mysql, update);
  myquery(rc);
  DIE_UNLESS(mysql_affected_rows(mysql) == NUM_ROWS);
  DIE_UNLESS(exp_count == mysql_stmt_affected_rows(stmt));

  mysql_free_result(res);
  mysql_stmt_close(stmt);

  /* check that mysql_stmt_store_result modifies mysql_stmt_affected_rows */
  stmt = mysql_simple_prepare(mysql, SELECT);
  check_stmt(stmt);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);
  rc = mysql_stmt_store_result(stmt);
  check_execute(stmt, rc);
  exp_count = mysql_stmt_affected_rows(stmt);
  DIE_UNLESS(exp_count == NUM_ROWS);

  rc = mysql_query(mysql, insert);
  myquery(rc);
  DIE_UNLESS(mysql_affected_rows(mysql) == 1);
  DIE_UNLESS(mysql_stmt_affected_rows(stmt) == exp_count);

  mysql_stmt_close(stmt);
  if (!opt_silent) fprintf(stdout, "OK");
}

static void test_subqueries() {
  MYSQL_STMT *stmt;
  int rc, i;
  const char *query =
      "SELECT (SELECT SUM(a+b) FROM t2 where t1.b=t2.b GROUP BY t1.a LIMIT 1) "
      "as scalar_s, exists (select 1 from t2 where t2.a/2=t1.a) as exists_s, a "
      "in (select a+3 from t2) as in_s, (a-1, b-1) in (select a, b from t2) as "
      "in_row_s FROM t1, (select a x, b y from t2) tt WHERE x=a";

  myheader("test_subqueries");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS t1, t2");
  myquery(rc);

  rc = mysql_query(mysql, "CREATE TABLE t1 (a int , b int);");
  myquery(rc);

  rc = mysql_query(
      mysql, "insert into t1 values (1, 1), (2, 2), (3, 3), (4, 4), (5, 5);");
  myquery(rc);

  rc = mysql_query(mysql, "create table t2 select * from t1;");
  myquery(rc);

  stmt = mysql_simple_prepare(mysql, query);
  check_stmt(stmt);
  for (i = 0; i < 3; i++) {
    rc = mysql_stmt_execute(stmt);
    check_execute(stmt, rc);
    rc = my_process_stmt_result(stmt);
    DIE_UNLESS(rc == 5);
  }
  mysql_stmt_close(stmt);

  rc = mysql_query(mysql, "DROP TABLE t1, t2");
  myquery(rc);
}

static void test_bad_union() {
  MYSQL_STMT *stmt;
  const char *query = "SELECT 1, 2 union SELECT 1";

  myheader("test_bad_union");

  stmt = mysql_simple_prepare(mysql, query);
  DIE_UNLESS(stmt == nullptr);
  myerror(nullptr);
}

static void test_distinct() {
  MYSQL_STMT *stmt;
  int rc, i;
  const char *query =
      "SELECT 2+count(distinct b), group_concat(a) FROM t1 group by a";

  myheader("test_distinct");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS t1");
  myquery(rc);

  rc = mysql_query(mysql, "CREATE TABLE t1 (a int , b int);");
  myquery(rc);

  rc = mysql_query(
      mysql,
      "insert into t1 values (1, 1), (2, 2), (3, 3), (4, 4), (5, 5), \
(1, 10), (2, 20), (3, 30), (4, 40), (5, 50);");
  myquery(rc);

  for (i = 0; i < 3; i++) {
    stmt = mysql_simple_prepare(mysql, query);
    check_stmt(stmt);
    rc = mysql_stmt_execute(stmt);
    check_execute(stmt, rc);
    rc = my_process_stmt_result(stmt);
    DIE_UNLESS(rc == 5);
    mysql_stmt_close(stmt);
  }

  rc = mysql_query(mysql, "DROP TABLE t1");
  myquery(rc);
}

/*
  Test for bug#2248 "mysql_fetch without prior mysql_stmt_execute hangs"
*/

static void test_bug2248() {
  MYSQL_STMT *stmt;
  int rc;
  const char *query1 = "SELECT DATABASE()";
  const char *query2 = "INSERT INTO test_bug2248 VALUES (10)";

  myheader("test_bug2248");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_bug2248");
  myquery(rc);

  rc = mysql_query(mysql, "CREATE TABLE test_bug2248 (id int)");
  myquery(rc);

  stmt = mysql_simple_prepare(mysql, query1);
  check_stmt(stmt);

  /* This should not hang */
  rc = mysql_stmt_fetch(stmt);
  check_execute_r(stmt, rc);

  /* And this too */
  rc = mysql_stmt_store_result(stmt);
  check_execute_r(stmt, rc);

  mysql_stmt_close(stmt);

  stmt = mysql_simple_prepare(mysql, query2);
  check_stmt(stmt);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  /* This too should not hang but should return proper error */
  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == 1);

  /* This too should not hang but should not bark */
  rc = mysql_stmt_store_result(stmt);
  check_execute(stmt, rc);

  /* This should return proper error */
  rc = mysql_stmt_fetch(stmt);
  check_execute_r(stmt, rc);
  DIE_UNLESS(rc == 1);

  mysql_stmt_close(stmt);

  rc = mysql_query(mysql, "DROP TABLE test_bug2248");
  myquery(rc);
}

static void test_subqueries_ref() {
  MYSQL_STMT *stmt;
  int rc, i;
  const char *query =
      "SELECT a as ccc from t1 outr where a+1=(SELECT 1+outr.a from t1 where "
      "outr.a+1=a+1 and a=1)";

  myheader("test_subqueries_ref");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS t1");
  myquery(rc);

  rc = mysql_query(mysql, "CREATE TABLE t1 (a int);");
  myquery(rc);

  rc = mysql_query(mysql, "insert into t1 values (1), (2), (3), (4), (5);");
  myquery(rc);

  stmt = mysql_simple_prepare(mysql, query);
  check_stmt(stmt);
  for (i = 0; i < 3; i++) {
    rc = mysql_stmt_execute(stmt);
    check_execute(stmt, rc);
    rc = my_process_stmt_result(stmt);
    DIE_UNLESS(rc == 1);
  }
  mysql_stmt_close(stmt);

  rc = mysql_query(mysql, "DROP TABLE t1");
  myquery(rc);
}

static void test_union() {
  MYSQL_STMT *stmt;
  int rc;

  myheader("test_union");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS t1, t2");
  myquery(rc);

  rc = mysql_query(mysql,
                   "CREATE TABLE t1 "
                   "(id INTEGER NOT NULL PRIMARY KEY, "
                   " name VARCHAR(20) NOT NULL)");
  myquery(rc);
  rc = mysql_query(mysql,
                   "INSERT INTO t1 (id, name) VALUES "
                   "(2, 'Ja'), (3, 'Ede'), "
                   "(4, 'Haag'), (5, 'Kabul'), "
                   "(6, 'Almere'), (7, 'Utrecht'), "
                   "(8, 'Qandahar'), (9, 'Amsterdam'), "
                   "(10, 'Amersfoort'), (11, 'Constantine')");
  myquery(rc);
  rc = mysql_query(mysql,
                   "CREATE TABLE t2 "
                   "(id INTEGER NOT NULL PRIMARY KEY, "
                   " name VARCHAR(20) NOT NULL)");
  myquery(rc);
  rc = mysql_query(mysql,
                   "INSERT INTO t2 (id, name) VALUES "
                   "(4, 'Guam'), (5, 'Aruba'), "
                   "(6, 'Angola'), (7, 'Albania'), "
                   "(8, 'Anguilla'), (9, 'Argentina'), "
                   "(10, 'Azerbaijan'), (11, 'Afghanistan'), "
                   "(12, 'Burkina Faso'), (13, 'Faroe Islands')");
  myquery(rc);

  stmt = mysql_simple_prepare(mysql,
                              "SELECT t1.name FROM t1 UNION "
                              "SELECT t2.name FROM t2");
  check_stmt(stmt);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);
  rc = my_process_stmt_result(stmt);
  DIE_UNLESS(rc == 20);
  mysql_stmt_close(stmt);

  rc = mysql_query(mysql, "DROP TABLE t1, t2");
  myquery(rc);
}

static void test_bug3117() {
  MYSQL_STMT *stmt;
  MYSQL_BIND buffer;
  longlong lii;
  ulong length;
  bool is_null;
  int rc;

  myheader("test_bug3117");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS t1");
  myquery(rc);

  rc =
      mysql_query(mysql, "CREATE TABLE t1 (id int auto_increment primary key)");
  myquery(rc);

  stmt = mysql_simple_prepare(mysql, "SELECT LAST_INSERT_ID()");
  check_stmt(stmt);

  rc = mysql_query(mysql, "INSERT INTO t1 VALUES (NULL)");
  myquery(rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  memset(&buffer, 0, sizeof(buffer));
  buffer.buffer_type = MYSQL_TYPE_LONGLONG;
  buffer.buffer_length = (ulong)sizeof(lii);
  buffer.buffer = (void *)&lii;
  buffer.length = &length;
  buffer.is_null = &is_null;

  rc = mysql_stmt_bind_result(stmt, &buffer);
  check_execute(stmt, rc);

  rc = mysql_stmt_store_result(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  DIE_UNLESS(is_null == 0 && lii == 1);
  if (!opt_silent) fprintf(stdout, "\n\tLAST_INSERT_ID()= 1 ok\n");

  rc = mysql_query(mysql, "INSERT INTO t1 VALUES (NULL)");
  myquery(rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  DIE_UNLESS(is_null == 0 && lii == 2);
  if (!opt_silent) fprintf(stdout, "\tLAST_INSERT_ID()= 2 ok\n");

  mysql_stmt_close(stmt);

  rc = mysql_query(mysql, "DROP TABLE t1");
  myquery(rc);
}

static void test_join() {
  MYSQL_STMT *stmt;
  int rc, i, j;
  const char *query[] = {"SELECT * FROM t2 join t1 on (t1.a=t2.a)",
                         "SELECT * FROM t2 natural join t1",
                         "SELECT * FROM t2 join t1 using(a)",
                         "SELECT * FROM t2 left join t1 on(t1.a=t2.a)",
                         "SELECT * FROM t2 natural left join t1",
                         "SELECT * FROM t2 left join t1 using(a)",
                         "SELECT * FROM t2 right join t1 on(t1.a=t2.a)",
                         "SELECT * FROM t2 natural right join t1",
                         "SELECT * FROM t2 right join t1 using(a)"};

  myheader("test_join");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS t1, t2");
  myquery(rc);

  rc = mysql_query(mysql, "CREATE TABLE t1 (a int , b int);");
  myquery(rc);

  rc = mysql_query(
      mysql, "insert into t1 values (1, 1), (2, 2), (3, 3), (4, 4), (5, 5);");
  myquery(rc);

  rc = mysql_query(mysql, "CREATE TABLE t2 (a int , c int);");
  myquery(rc);

  rc = mysql_query(
      mysql, "insert into t2 values (1, 1), (2, 2), (3, 3), (4, 4), (5, 5);");
  myquery(rc);

  for (j = 0; j < 9; j++) {
    stmt = mysql_simple_prepare(mysql, query[j]);
    check_stmt(stmt);
    for (i = 0; i < 3; i++) {
      rc = mysql_stmt_execute(stmt);
      check_execute(stmt, rc);
      rc = my_process_stmt_result(stmt);
      DIE_UNLESS(rc == 5);
    }
    mysql_stmt_close(stmt);
  }

  rc = mysql_query(mysql, "DROP TABLE t1, t2");
  myquery(rc);
}

static void test_selecttmp() {
  MYSQL_STMT *stmt;
  int rc, i;
  const char *query =
      "select a, (select count(distinct t1.b) as sum from t1, t2 where "
      "t1.a=t2.a and t2.b > 0 and t1.a <= t3.b group by t1.a order by sum "
      "limit 1) from t3";

  myheader("test_select_tmp");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS t1, t2, t3");
  myquery(rc);

  rc = mysql_query(mysql, "CREATE TABLE t1 (a int , b int);");
  myquery(rc);

  rc = mysql_query(mysql, "create table t2 (a int, b int);");
  myquery(rc);

  rc = mysql_query(mysql, "create table t3 (a int, b int);");
  myquery(rc);

  rc = mysql_query(
      mysql,
      "insert into t1 values (0, 100), (1, 2), (1, 3), (2, 2), (2, 7), \
(2, -1), (3, 10);");
  myquery(rc);
  rc = mysql_query(
      mysql, "insert into t2 values (0, 0), (1, 1), (2, 1), (3, 1), (4, 1);");
  myquery(rc);
  rc = mysql_query(mysql, "insert into t3 values (3, 3), (2, 2), (1, 1);");
  myquery(rc);

  stmt = mysql_simple_prepare(mysql, query);
  check_stmt(stmt);
  for (i = 0; i < 3; i++) {
    rc = mysql_stmt_execute(stmt);
    check_execute(stmt, rc);
    rc = my_process_stmt_result(stmt);
    DIE_UNLESS(rc == 3);
  }
  mysql_stmt_close(stmt);

  rc = mysql_query(mysql, "DROP TABLE t1, t2, t3");
  myquery(rc);
}

static void test_create_drop() {
  MYSQL_STMT *stmt_create, *stmt_drop, *stmt_select, *stmt_create_select;
  const char *query;
  int rc, i;
  myheader("test_table_manipulation");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS t1, t2");
  myquery(rc);

  rc = mysql_query(mysql, "create table t2 (a int);");
  myquery(rc);

  rc = mysql_query(mysql, "create table t1 (a int);");
  myquery(rc);

  rc = mysql_query(mysql, "insert into t2 values (3), (2), (1);");
  myquery(rc);

  query = "create table t1 (a int)";
  stmt_create = mysql_simple_prepare(mysql, query);
  check_stmt(stmt_create);

  query = "drop table t1";
  stmt_drop = mysql_simple_prepare(mysql, query);
  check_stmt(stmt_drop);

  query = "select a in (select a from t2) from t1";
  stmt_select = mysql_simple_prepare(mysql, query);
  check_stmt(stmt_select);

  rc = mysql_query(mysql, "DROP TABLE t1");
  myquery(rc);

  query = "create table t1 select a from t2";
  stmt_create_select = mysql_simple_prepare(mysql, query);
  check_stmt(stmt_create_select);

  for (i = 0; i < 3; i++) {
    rc = mysql_stmt_execute(stmt_create);
    check_execute(stmt_create, rc);
    if (!opt_silent) fprintf(stdout, "created %i\n", i);

    rc = mysql_stmt_execute(stmt_select);
    check_execute(stmt_select, rc);
    rc = my_process_stmt_result(stmt_select);
    DIE_UNLESS(rc == 0);

    rc = mysql_stmt_execute(stmt_drop);
    check_execute(stmt_drop, rc);
    if (!opt_silent) fprintf(stdout, "dropped %i\n", i);

    rc = mysql_stmt_execute(stmt_create_select);
    check_execute(stmt_create, rc);
    if (!opt_silent) fprintf(stdout, "created select %i\n", i);

    rc = mysql_stmt_execute(stmt_select);
    check_execute(stmt_select, rc);
    rc = my_process_stmt_result(stmt_select);
    DIE_UNLESS(rc == 3);

    rc = mysql_stmt_execute(stmt_drop);
    check_execute(stmt_drop, rc);
    if (!opt_silent) fprintf(stdout, "dropped %i\n", i);
  }

  mysql_stmt_close(stmt_create);
  mysql_stmt_close(stmt_drop);
  mysql_stmt_close(stmt_select);
  mysql_stmt_close(stmt_create_select);

  rc = mysql_query(mysql, "DROP TABLE t2");
  myquery(rc);
}

static void test_rename() {
  MYSQL_STMT *stmt;
  const char *query = "rename table t1 to t2, t3 to t4";
  int rc;
  myheader("test_table_rename");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS t1, t2, t3, t4");
  myquery(rc);

  stmt = mysql_simple_prepare(mysql, query);
  check_stmt(stmt);

  rc = mysql_query(mysql, "create table t1 (a int)");
  myquery(rc);

  rc = mysql_stmt_execute(stmt);
  check_execute_r(stmt, rc);
  if (!opt_silent) fprintf(stdout, "rename without t3\n");

  rc = mysql_query(mysql, "create table t3 (a int)");
  myquery(rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);
  if (!opt_silent) fprintf(stdout, "rename with t3\n");

  rc = mysql_stmt_execute(stmt);
  check_execute_r(stmt, rc);
  if (!opt_silent) fprintf(stdout, "rename renamed\n");

  rc = mysql_query(mysql, "rename table t2 to t1, t4 to t3");
  myquery(rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);
  if (!opt_silent) fprintf(stdout, "rename reverted\n");

  mysql_stmt_close(stmt);

  rc = mysql_query(mysql, "DROP TABLE t2, t4");
  myquery(rc);
}

static void test_do_set() {
  MYSQL_STMT *stmt_do, *stmt_set;
  const char *query;
  int rc, i;
  myheader("test_do_set");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS t1");
  myquery(rc);

  rc = mysql_query(mysql, "create table t1 (a int)");
  myquery(rc);

  query = "do @var:=(1 in (select * from t1))";
  stmt_do = mysql_simple_prepare(mysql, query);
  check_stmt(stmt_do);

  query = "set @var=(1 in (select * from t1))";
  stmt_set = mysql_simple_prepare(mysql, query);
  check_stmt(stmt_set);

  for (i = 0; i < 3; i++) {
    rc = mysql_stmt_execute(stmt_do);
    check_execute(stmt_do, rc);
    if (!opt_silent) fprintf(stdout, "do %i\n", i);
    rc = mysql_stmt_execute(stmt_set);
    check_execute(stmt_set, rc);
    if (!opt_silent) fprintf(stdout, "set %i\n", i);
  }

  mysql_stmt_close(stmt_do);
  mysql_stmt_close(stmt_set);
}

static void test_multi() {
  MYSQL_STMT *stmt_delete, *stmt_update, *stmt_select1, *stmt_select2;
  const char *query;
  MYSQL_BIND my_bind[1];
  int rc, i;
  int32 param = 1;
  ulong length = 1;
  myheader("test_multi");

  /*
    We need to memset bind structure because mysql_stmt_bind_param checks all
    its members.
  */
  memset(my_bind, 0, sizeof(my_bind));

  my_bind[0].buffer_type = MYSQL_TYPE_LONG;
  my_bind[0].buffer = (void *)&param;
  my_bind[0].length = &length;

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS t1, t2");
  myquery(rc);

  rc = mysql_query(mysql, "create table t1 (a int, b int)");
  myquery(rc);

  rc = mysql_query(mysql, "create table t2 (a int, b int)");
  myquery(rc);

  rc = mysql_query(mysql, "insert into t1 values (3, 3), (2, 2), (1, 1)");
  myquery(rc);

  rc = mysql_query(mysql, "insert into t2 values (3, 3), (2, 2), (1, 1)");
  myquery(rc);

  query = "delete t1, t2 from t1, t2 where t1.a=t2.a and t1.b=10";
  stmt_delete = mysql_simple_prepare(mysql, query);
  check_stmt(stmt_delete);

  query = "update t1, t2 set t1.b=10, t2.b=10 where t1.a=t2.a and t1.b=?";
  stmt_update = mysql_simple_prepare(mysql, query);
  check_stmt(stmt_update);

  query = "select * from t1";
  stmt_select1 = mysql_simple_prepare(mysql, query);
  check_stmt(stmt_select1);

  query = "select * from t2";
  stmt_select2 = mysql_simple_prepare(mysql, query);
  check_stmt(stmt_select2);

  for (i = 0; i < 3; i++) {
    rc = mysql_stmt_bind_param(stmt_update, my_bind);
    check_execute(stmt_update, rc);

    rc = mysql_stmt_execute(stmt_update);
    check_execute(stmt_update, rc);
    if (!opt_silent) fprintf(stdout, "update %ld\n", (long)param);

    rc = mysql_stmt_execute(stmt_delete);
    check_execute(stmt_delete, rc);
    if (!opt_silent) fprintf(stdout, "delete %ld\n", (long)param);

    rc = mysql_stmt_execute(stmt_select1);
    check_execute(stmt_select1, rc);
    rc = my_process_stmt_result(stmt_select1);
    DIE_UNLESS(rc == 3 - param);

    rc = mysql_stmt_execute(stmt_select2);
    check_execute(stmt_select2, rc);
    rc = my_process_stmt_result(stmt_select2);
    DIE_UNLESS(rc == 3 - param);

    param++;
  }

  mysql_stmt_close(stmt_delete);
  mysql_stmt_close(stmt_update);
  mysql_stmt_close(stmt_select1);
  mysql_stmt_close(stmt_select2);
  rc = mysql_query(mysql, "drop table t1, t2");
  myquery(rc);
}

static void test_insert_select() {
  MYSQL_STMT *stmt_insert, *stmt_select;
  const char *query;
  int rc;
  uint i;
  myheader("test_insert_select");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS t1, t2");
  myquery(rc);

  rc = mysql_query(mysql, "create table t1 (a int)");
  myquery(rc);

  rc = mysql_query(mysql, "create table t2 (a int)");
  myquery(rc);

  rc = mysql_query(mysql, "insert into t2 values (1)");
  myquery(rc);

  query = "insert into t1 select a from t2";
  stmt_insert = mysql_simple_prepare(mysql, query);
  check_stmt(stmt_insert);

  query = "select * from t1";
  stmt_select = mysql_simple_prepare(mysql, query);
  check_stmt(stmt_select);

  for (i = 0; i < 3; i++) {
    rc = mysql_stmt_execute(stmt_insert);
    check_execute(stmt_insert, rc);
    if (!opt_silent) fprintf(stdout, "insert %u\n", i);

    rc = mysql_stmt_execute(stmt_select);
    check_execute(stmt_select, rc);
    rc = my_process_stmt_result(stmt_select);
    DIE_UNLESS(rc == (int)(i + 1));
  }

  mysql_stmt_close(stmt_insert);
  mysql_stmt_close(stmt_select);
  rc = mysql_query(mysql, "drop table t1, t2");
  myquery(rc);
}

static void test_bind_nagative() {
  MYSQL_STMT *stmt_insert;
  const char *query;
  int rc;
  MYSQL_BIND my_bind[1];
  int32 my_val = 0;
  ulong my_length = 0L;
  bool my_null = false;
  myheader("test_insert_select");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS t1");
  myquery(rc);

  rc = mysql_query(mysql, "create temporary table t1 (c1 int unsigned)");
  myquery(rc);

  rc = mysql_query(mysql, "INSERT INTO t1 VALUES (1), (-1)");
  myquery(rc);

  query = "INSERT INTO t1 VALUES (?)";
  stmt_insert = mysql_simple_prepare(mysql, query);
  check_stmt(stmt_insert);

  /* bind parameters */
  memset(my_bind, 0, sizeof(my_bind));

  my_bind[0].buffer_type = MYSQL_TYPE_LONG;
  my_bind[0].buffer = (void *)&my_val;
  my_bind[0].length = &my_length;
  my_bind[0].is_null = &my_null;

  rc = mysql_stmt_bind_param(stmt_insert, my_bind);
  check_execute(stmt_insert, rc);

  my_val = -1;
  rc = mysql_stmt_execute(stmt_insert);
  check_execute(stmt_insert, rc);

  mysql_stmt_close(stmt_insert);
  rc = mysql_query(mysql, "drop table t1");
  myquery(rc);
}

static void test_derived() {
  MYSQL_STMT *stmt;
  int rc, i;
  MYSQL_BIND my_bind[1];
  int32 my_val = 0;
  ulong my_length = 0L;
  bool my_null = false;
  const char *query =
      "select count(1) from (select f.id from t1 f where f.id=?) as x";

  myheader("test_derived");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS t1");
  myquery(rc);

  rc = mysql_query(mysql,
                   "create table t1 (id  int(8), primary key (id)) \
ENGINE=InnoDB DEFAULT CHARSET=utf8");
  myquery(rc);

  rc = mysql_query(mysql, "insert into t1 values (1)");
  myquery(rc);

  stmt = mysql_simple_prepare(mysql, query);
  check_stmt(stmt);
  /*
    We need to memset bind structure because mysql_stmt_bind_param checks all
    its members.
  */
  memset(my_bind, 0, sizeof(my_bind));

  my_bind[0].buffer_type = MYSQL_TYPE_LONG;
  my_bind[0].buffer = (void *)&my_val;
  my_bind[0].length = &my_length;
  my_bind[0].is_null = (bool *)&my_null;
  my_val = 1;
  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  for (i = 0; i < 3; i++) {
    rc = mysql_stmt_execute(stmt);
    check_execute(stmt, rc);
    rc = my_process_stmt_result(stmt);
    DIE_UNLESS(rc == 1);
  }
  mysql_stmt_close(stmt);

  rc = mysql_query(mysql, "DROP TABLE t1");
  myquery(rc);
}

static void test_xjoin() {
  MYSQL_STMT *stmt;
  int rc, i;
  const char *query =
      "select t.id, p1.value, n1.value, p2.value, n2.value from t3 t LEFT JOIN "
      "t1 p1 ON (p1.id=t.param1_id) LEFT JOIN t2 p2 ON (p2.id=t.param2_id) "
      "LEFT JOIN t4 n1 ON (n1.id=p1.name_id) LEFT JOIN t4 n2 ON "
      "(n2.id=p2.name_id) where t.id=1";

  myheader("test_xjoin");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS t1, t2, t3, t4");
  myquery(rc);

  rc = mysql_query(mysql,
                   "create table t3 (id int(8), param1_id int(8), param2_id "
                   "int(8)) ENGINE=InnoDB DEFAULT CHARSET=utf8");
  myquery(rc);

  rc = mysql_query(mysql,
                   "create table t1 ( id int(8), name_id int(8), value "
                   "varchar(10)) ENGINE=InnoDB DEFAULT CHARSET=utf8");
  myquery(rc);

  rc = mysql_query(mysql,
                   "create table t2 (id int(8), name_id int(8), value "
                   "varchar(10)) ENGINE=InnoDB DEFAULT CHARSET=utf8;");
  myquery(rc);

  rc = mysql_query(mysql,
                   "create table t4(id int(8), value varchar(10)) "
                   "ENGINE=InnoDB DEFAULT CHARSET=utf8");
  myquery(rc);

  rc = mysql_query(mysql, "insert into t3 values (1, 1, 1), (2, 2, null)");
  myquery(rc);

  rc = mysql_query(mysql,
                   "insert into t1 values (1, 1, 'aaa'), (2, null, 'bbb')");
  myquery(rc);

  rc = mysql_query(mysql, "insert into t2 values (1, 2, 'ccc')");
  myquery(rc);

  rc = mysql_query(mysql, "insert into t4 values (1, 'Name1'), (2, null)");
  myquery(rc);

  stmt = mysql_simple_prepare(mysql, query);
  check_stmt(stmt);

  for (i = 0; i < 3; i++) {
    rc = mysql_stmt_execute(stmt);
    check_execute(stmt, rc);
    rc = my_process_stmt_result(stmt);
    DIE_UNLESS(rc == 1);
  }
  mysql_stmt_close(stmt);

  rc = mysql_query(mysql, "DROP TABLE t1, t2, t3, t4");
  myquery(rc);
}

static void test_bug3035() {
  MYSQL_STMT *stmt;
  int rc;
  MYSQL_BIND bind_array[12], *my_bind = bind_array, *bind_end = my_bind + 12;
  int8 int8_val;
  uint8 uint8_val;
  int16 int16_val;
  uint16 uint16_val;
  int32 int32_val;
  uint32 uint32_val;
  longlong int64_val;
  ulonglong uint64_val;
  double double_val, udouble_val, double_tmp;
  char longlong_as_string[22], ulonglong_as_string[22];

  /* mins and maxes */
  const int8 int8_min = -128;
  const int8 int8_max = 127;
  const uint8 uint8_min = 0;
  const uint8 uint8_max = 255;

  const int16 int16_min = -32768;
  const int16 int16_max = 32767;
  const uint16 uint16_min = 0;
  const uint16 uint16_max = 65535;

  const int32 int32_max = 2147483647L;
  const int32 int32_min = -int32_max - 1;
  const uint32 uint32_min = 0;
  const uint32 uint32_max = 4294967295U;

  /* it might not work okay everyplace */
  const longlong int64_max = 9223372036854775807LL;
  const longlong int64_min = -int64_max - 1;

  const ulonglong uint64_min = 0U;
  const ulonglong uint64_max = 18446744073709551615ULL;

  const char *stmt_text;

  myheader("test_bug3035");

  stmt_text = "DROP TABLE IF EXISTS t1";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);

  stmt_text =
      "CREATE TABLE t1 (i8 TINYINT, ui8 TINYINT UNSIGNED, "
      "i16 SMALLINT, ui16 SMALLINT UNSIGNED, "
      "i32 INT, ui32 INT UNSIGNED, "
      "i64 BIGINT, ui64 BIGINT UNSIGNED, "
      "id INTEGER NOT NULL PRIMARY KEY AUTO_INCREMENT)";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);

  memset(bind_array, 0, sizeof(bind_array));

  for (my_bind = bind_array; my_bind < bind_end; my_bind++)
    my_bind->error = &my_bind->error_value;

  bind_array[0].buffer_type = MYSQL_TYPE_TINY;
  bind_array[0].buffer = (void *)&int8_val;

  bind_array[1].buffer_type = MYSQL_TYPE_TINY;
  bind_array[1].buffer = (void *)&uint8_val;
  bind_array[1].is_unsigned = true;

  bind_array[2].buffer_type = MYSQL_TYPE_SHORT;
  bind_array[2].buffer = (void *)&int16_val;

  bind_array[3].buffer_type = MYSQL_TYPE_SHORT;
  bind_array[3].buffer = (void *)&uint16_val;
  bind_array[3].is_unsigned = true;

  bind_array[4].buffer_type = MYSQL_TYPE_LONG;
  bind_array[4].buffer = (void *)&int32_val;

  bind_array[5].buffer_type = MYSQL_TYPE_LONG;
  bind_array[5].buffer = (void *)&uint32_val;
  bind_array[5].is_unsigned = true;

  bind_array[6].buffer_type = MYSQL_TYPE_LONGLONG;
  bind_array[6].buffer = (void *)&int64_val;

  bind_array[7].buffer_type = MYSQL_TYPE_LONGLONG;
  bind_array[7].buffer = (void *)&uint64_val;
  bind_array[7].is_unsigned = true;

  stmt = mysql_stmt_init(mysql);
  check_stmt(stmt);

  stmt_text =
      "INSERT INTO t1 (i8, ui8, i16, ui16, i32, ui32, i64, ui64) "
      "VALUES (?, ?, ?, ?, ?, ?, ?, ?)";
  rc = mysql_stmt_prepare(stmt, stmt_text, (ulong)strlen(stmt_text));
  check_execute(stmt, rc);

  mysql_stmt_bind_param(stmt, bind_array);

  int8_val = int8_min;
  uint8_val = uint8_min;
  int16_val = int16_min;
  uint16_val = uint16_min;
  int32_val = int32_min;
  uint32_val = uint32_min;
  int64_val = int64_min;
  uint64_val = uint64_min;

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  int8_val = int8_max;
  uint8_val = uint8_max;
  int16_val = int16_max;
  uint16_val = uint16_max;
  int32_val = int32_max;
  uint32_val = uint32_max;
  int64_val = int64_max;
  uint64_val = uint64_max;

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  stmt_text =
      "SELECT i8, ui8, i16, ui16, i32, ui32, i64, ui64, ui64, "
      "cast(ui64 as signed), ui64, cast(ui64 as signed)"
      "FROM t1 ORDER BY id ASC";

  rc = mysql_stmt_prepare(stmt, stmt_text, (ulong)strlen(stmt_text));
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  bind_array[8].buffer_type = MYSQL_TYPE_DOUBLE;
  bind_array[8].buffer = (void *)&udouble_val;

  bind_array[9].buffer_type = MYSQL_TYPE_DOUBLE;
  bind_array[9].buffer = (void *)&double_val;

  bind_array[10].buffer_type = MYSQL_TYPE_STRING;
  bind_array[10].buffer = (void *)&ulonglong_as_string;
  bind_array[10].buffer_length = (ulong)sizeof(ulonglong_as_string);

  bind_array[11].buffer_type = MYSQL_TYPE_STRING;
  bind_array[11].buffer = (void *)&longlong_as_string;
  bind_array[11].buffer_length = (ulong)sizeof(longlong_as_string);

  mysql_stmt_bind_result(stmt, bind_array);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  DIE_UNLESS(int8_val == int8_min);
  DIE_UNLESS(uint8_val == uint8_min);
  DIE_UNLESS(int16_val == int16_min);
  DIE_UNLESS(uint16_val == uint16_min);
  DIE_UNLESS(int32_val == int32_min);
  DIE_UNLESS(uint32_val == uint32_min);
  DIE_UNLESS(int64_val == int64_min);
  DIE_UNLESS(uint64_val == uint64_min);
  DIE_UNLESS(double_val == (longlong)uint64_min);
  double_tmp = ulonglong2double(uint64_val);
  DIE_UNLESS(cmp_double(&udouble_val, &double_tmp));
  DIE_UNLESS(!strcmp(longlong_as_string, "0"));
  DIE_UNLESS(!strcmp(ulonglong_as_string, "0"));

  rc = mysql_stmt_fetch(stmt);

  if (!opt_silent) {
    printf("Truncation mask: ");
    for (my_bind = bind_array; my_bind < bind_end; my_bind++)
      printf("%d", (int)my_bind->error_value);
    printf("\n");
  }
  DIE_UNLESS(rc == MYSQL_DATA_TRUNCATED || rc == 0);

  DIE_UNLESS(int8_val == int8_max);
  DIE_UNLESS(uint8_val == uint8_max);
  DIE_UNLESS(int16_val == int16_max);
  DIE_UNLESS(uint16_val == uint16_max);
  DIE_UNLESS(int32_val == int32_max);
  DIE_UNLESS(uint32_val == uint32_max);
  DIE_UNLESS(int64_val == int64_max);
  DIE_UNLESS(uint64_val == uint64_max);
  DIE_UNLESS(double_val == (longlong)uint64_val);
  double_tmp = ulonglong2double(uint64_val);
  DIE_UNLESS(cmp_double(&udouble_val, &double_tmp));
  DIE_UNLESS(!strcmp(longlong_as_string, "-1"));
  DIE_UNLESS(!strcmp(ulonglong_as_string, "18446744073709551615"));

  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == MYSQL_NO_DATA);

  mysql_stmt_close(stmt);

  stmt_text = "DROP TABLE t1";
  mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
}

static void test_union2() {
  MYSQL_STMT *stmt;
  int rc, i;

  myheader("test_union2");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS t1");
  myquery(rc);

  rc = mysql_query(mysql,
                   "CREATE TABLE t1(col1 INT, \
                                         col2 VARCHAR(40),      \
                                         col3 SMALLINT, \
                                         col4 TIMESTAMP)");
  myquery(rc);

  stmt = mysql_simple_prepare(mysql,
                              "select col1 FROM t1 where col1=1 union distinct "
                              "select col1 FROM t1 where col1=2");
  check_stmt(stmt);

  for (i = 0; i < 3; i++) {
    rc = mysql_stmt_execute(stmt);
    check_execute(stmt, rc);
    rc = my_process_stmt_result(stmt);
    DIE_UNLESS(rc == 0);
  }

  mysql_stmt_close(stmt);

  rc = mysql_query(mysql, "DROP TABLE t1");
  myquery(rc);
}

/*
  This tests for various mysql_stmt_send_long_data bugs described in #1664
*/

static void test_bug1664() {
  MYSQL_STMT *stmt;
  int rc, int_data;
  const char *data;
  const char *str_data = "Simple string";
  MYSQL_BIND my_bind[2];
  const char *query = "INSERT INTO test_long_data(col2, col1) VALUES(?, ?)";

  myheader("test_bug1664");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS test_long_data");
  myquery(rc);

  rc = mysql_query(mysql,
                   "CREATE TABLE test_long_data(col1 int, col2 long varchar)");
  myquery(rc);

  stmt = mysql_stmt_init(mysql);
  check_stmt(stmt);
  rc = mysql_stmt_prepare(stmt, query, (ulong)strlen(query));
  check_execute(stmt, rc);

  verify_param_count(stmt, 2);

  memset(my_bind, 0, sizeof(my_bind));

  my_bind[0].buffer_type = MYSQL_TYPE_STRING;
  my_bind[0].buffer = const_cast<char *>(str_data);
  my_bind[0].buffer_length = (ulong)strlen(str_data);

  my_bind[1].buffer = (void *)&int_data;
  my_bind[1].buffer_type = MYSQL_TYPE_LONG;

  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  int_data = 1;

  /*
    Let us supply empty long_data. This should work and should
    not break following execution.
  */
  data = "";
  rc = mysql_stmt_send_long_data(stmt, 0, data, (ulong)strlen(data));
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  verify_col_data("test_long_data", "col1", "1");
  verify_col_data("test_long_data", "col2", "");

  rc = mysql_query(mysql, "DELETE FROM test_long_data");
  myquery(rc);

  /* This should pass OK */
  data = "Data";
  rc = mysql_stmt_send_long_data(stmt, 0, data, (ulong)strlen(data));
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  verify_col_data("test_long_data", "col1", "1");
  verify_col_data("test_long_data", "col2", "Data");

  /* clean up */
  rc = mysql_query(mysql, "DELETE FROM test_long_data");
  myquery(rc);

  /*
    Now we are changing int parameter and don't do anything
    with first parameter. Second mysql_stmt_execute() should run
    OK treating this first parameter as string parameter.
  */

  int_data = 2;
  /* execute */
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  verify_col_data("test_long_data", "col1", "2");
  verify_col_data("test_long_data", "col2", str_data);

  /* clean up */
  rc = mysql_query(mysql, "DELETE FROM test_long_data");
  myquery(rc);

  /*
    Now we are sending other long data. It should not be
    concatened to previous.
  */

  data = "SomeOtherData";
  rc = mysql_stmt_send_long_data(stmt, 0, data, (ulong)strlen(data));
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  verify_col_data("test_long_data", "col1", "2");
  verify_col_data("test_long_data", "col2", "SomeOtherData");

  mysql_stmt_close(stmt);

  /* clean up */
  rc = mysql_query(mysql, "DELETE FROM test_long_data");
  myquery(rc);

  /* Now let us test how mysql_stmt_reset works. */
  stmt = mysql_stmt_init(mysql);
  check_stmt(stmt);
  rc = mysql_stmt_prepare(stmt, query, (ulong)strlen(query));
  check_execute(stmt, rc);
  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  data = "SomeData";
  rc = mysql_stmt_send_long_data(stmt, 0, data, (ulong)strlen(data));
  check_execute(stmt, rc);

  rc = mysql_stmt_reset(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  verify_col_data("test_long_data", "col1", "2");
  verify_col_data("test_long_data", "col2", str_data);

  mysql_stmt_close(stmt);

  /* Final clean up */
  rc = mysql_query(mysql, "DROP TABLE test_long_data");
  myquery(rc);
}

static void test_order_param() {
  MYSQL_STMT *stmt;
  int rc;

  myheader("test_order_param");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS t1");
  myquery(rc);

  rc = mysql_query(mysql, "CREATE TABLE t1(a INT, b char(10))");
  myquery(rc);

  stmt = mysql_simple_prepare(mysql,
                              "select sum(a) + 200, 1 from t1 "
                              " union distinct "
                              "select sum(a) + 200, 1 from t1 group by b ");
  check_stmt(stmt);
  mysql_stmt_close(stmt);

  stmt = mysql_simple_prepare(mysql,
                              "select sum(a) + 200, ? from t1 group by b "
                              " union distinct "
                              "select sum(a) + 200, 1 from t1 group by b ");
  check_stmt(stmt);
  mysql_stmt_close(stmt);

  stmt = mysql_simple_prepare(mysql,
                              "select sum(a) + 200, ? from t1 "
                              " union distinct "
                              "select sum(a) + 200, 1 from t1 group by b ");
  check_stmt(stmt);
  mysql_stmt_close(stmt);

  rc = mysql_query(mysql, "DROP TABLE t1");
  myquery(rc);
}

static void test_union_param() {
  MYSQL_STMT *stmt;
  const char *query;
  int rc, i;
  MYSQL_BIND my_bind[2];
  char my_val[4];
  ulong my_length = 3L;
  bool my_null = false;
  myheader("test_union_param");

  my_stpcpy(my_val, "abc");

  query = "select ? as my_col union distinct select ?";
  stmt = mysql_simple_prepare(mysql, query);
  check_stmt(stmt);

  /*
    We need to memset bind structure because mysql_stmt_bind_param checks all
    its members.
  */
  memset(my_bind, 0, sizeof(my_bind));

  /* bind parameters */
  my_bind[0].buffer_type = MYSQL_TYPE_STRING;
  my_bind[0].buffer = (char *)&my_val;
  my_bind[0].buffer_length = 4;
  my_bind[0].length = &my_length;
  my_bind[0].is_null = &my_null;
  my_bind[1].buffer_type = MYSQL_TYPE_STRING;
  my_bind[1].buffer = (char *)&my_val;
  my_bind[1].buffer_length = 4;
  my_bind[1].length = &my_length;
  my_bind[1].is_null = &my_null;

  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  for (i = 0; i < 3; i++) {
    rc = mysql_stmt_execute(stmt);
    check_execute(stmt, rc);
    rc = my_process_stmt_result(stmt);
    DIE_UNLESS(rc == 1);
  }

  mysql_stmt_close(stmt);
}

static void test_ps_i18n() {
  MYSQL_STMT *stmt;
  int rc;
  const char *stmt_text;
  MYSQL_BIND bind_array[2];

  /* Represented as numbers to keep UTF8 tools from clobbering them. */
  const char *koi8 = "\xee\xd5\x2c\x20\xda\xc1\x20\xd2\xd9\xc2\xc1\xcc\xcb\xd5";
  const char *cp1251 =
      "\xcd\xf3\x2c\x20\xe7\xe0\x20\xf0\xfb\xe1\xe0\xeb\xea\xf3";
  char buf1[16], buf2[16];
  ulong buf1_len, buf2_len;

  myheader("test_ps_i18n");

  stmt_text = "DROP TABLE IF EXISTS t1";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);

  /*
    Create table with binary columns, set session character set to cp1251,
    client character set to koi8, and make sure that there is conversion
    on insert and no conversion on select
  */

  stmt_text = "CREATE TABLE t1 (c1 VARBINARY(255), c2 VARBINARY(255))";

  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);

  stmt_text =
      "SET CHARACTER_SET_CLIENT=koi8r, "
      "CHARACTER_SET_CONNECTION=cp1251, "
      "CHARACTER_SET_RESULTS=koi8r";

  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);

  memset(bind_array, 0, sizeof(bind_array));

  bind_array[0].buffer_type = MYSQL_TYPE_STRING;
  bind_array[0].buffer = const_cast<char *>(koi8);
  bind_array[0].buffer_length = (ulong)strlen(koi8);

  bind_array[1].buffer_type = MYSQL_TYPE_STRING;
  bind_array[1].buffer = const_cast<char *>(koi8);
  bind_array[1].buffer_length = (ulong)strlen(koi8);

  stmt = mysql_stmt_init(mysql);
  check_stmt(stmt);

  stmt_text = "INSERT INTO t1 (c1, c2) VALUES (?, ?)";

  rc = mysql_stmt_prepare(stmt, stmt_text, (ulong)strlen(stmt_text));
  check_execute(stmt, rc);

  mysql_stmt_bind_param(stmt, bind_array);

  mysql_stmt_send_long_data(stmt, 0, koi8, (ulong)strlen(koi8));

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  stmt_text = "SELECT c1, c2 FROM t1";

  /* c1 and c2 are binary so no conversion will be done on select */
  rc = mysql_stmt_prepare(stmt, stmt_text, (ulong)strlen(stmt_text));
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  bind_array[0].buffer = buf1;
  bind_array[0].buffer_length = (ulong)sizeof(buf1);
  bind_array[0].length = &buf1_len;

  bind_array[1].buffer = buf2;
  bind_array[1].buffer_length = (ulong)sizeof(buf2);
  bind_array[1].length = &buf2_len;

  mysql_stmt_bind_result(stmt, bind_array);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  DIE_UNLESS(buf1_len == strlen(cp1251));
  DIE_UNLESS(buf2_len == strlen(cp1251));
  DIE_UNLESS(!memcmp(buf1, cp1251, buf1_len));
  DIE_UNLESS(!memcmp(buf2, cp1251, buf1_len));

  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == MYSQL_NO_DATA);

  stmt_text = "DROP TABLE IF EXISTS t1";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);

  /*
    Now create table with two cp1251 columns, set client character
    set to koi8 and supply columns of one row as string and another as
    binary data. Binary data must not be converted on insert, and both
    columns must be converted to client character set on select.
  */

  stmt_text =
      "CREATE TABLE t1 (c1 VARCHAR(255) CHARACTER SET cp1251, "
      "c2 VARCHAR(255) CHARACTER SET cp1251)";

  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);

  stmt_text = "INSERT INTO t1 (c1, c2) VALUES (?, ?)";

  rc = mysql_stmt_prepare(stmt, stmt_text, (ulong)strlen(stmt_text));
  check_execute(stmt, rc);

  /* this data must be converted */
  bind_array[0].buffer_type = MYSQL_TYPE_STRING;
  bind_array[0].buffer = const_cast<char *>(koi8);
  bind_array[0].buffer_length = (ulong)strlen(koi8);

  bind_array[1].buffer_type = MYSQL_TYPE_STRING;
  bind_array[1].buffer = const_cast<char *>(koi8);
  bind_array[1].buffer_length = (ulong)strlen(koi8);

  mysql_stmt_bind_param(stmt, bind_array);

  mysql_stmt_send_long_data(stmt, 0, koi8, (ulong)strlen(koi8));

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  /* this data must not be converted */
  bind_array[0].buffer_type = MYSQL_TYPE_BLOB;
  bind_array[0].buffer = const_cast<char *>(cp1251);
  bind_array[0].buffer_length = (ulong)strlen(cp1251);

  bind_array[1].buffer_type = MYSQL_TYPE_BLOB;
  bind_array[1].buffer = const_cast<char *>(cp1251);
  bind_array[1].buffer_length = (ulong)strlen(cp1251);

  mysql_stmt_bind_param(stmt, bind_array);

  mysql_stmt_send_long_data(stmt, 0, cp1251, (ulong)strlen(cp1251));

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  /* Fetch data and verify that rows are in koi8 */

  stmt_text = "SELECT c1, c2 FROM t1";

  /* c1 and c2 are binary so no conversion will be done on select */
  rc = mysql_stmt_prepare(stmt, stmt_text, (ulong)strlen(stmt_text));
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  bind_array[0].buffer = buf1;
  bind_array[0].buffer_length = (ulong)sizeof(buf1);
  bind_array[0].length = &buf1_len;

  bind_array[1].buffer = buf2;
  bind_array[1].buffer_length = (ulong)sizeof(buf2);
  bind_array[1].length = &buf2_len;

  mysql_stmt_bind_result(stmt, bind_array);

  while ((rc = mysql_stmt_fetch(stmt)) == 0) {
    DIE_UNLESS(buf1_len == strlen(koi8));
    DIE_UNLESS(buf2_len == strlen(koi8));
    DIE_UNLESS(!memcmp(buf1, koi8, buf1_len));
    DIE_UNLESS(!memcmp(buf2, koi8, buf1_len));
  }
  DIE_UNLESS(rc == MYSQL_NO_DATA);
  mysql_stmt_close(stmt);

  stmt_text = "DROP TABLE t1";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);
  stmt_text = "SET NAMES DEFAULT";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);
}

static void test_bug3796() {
  MYSQL_STMT *stmt;
  MYSQL_BIND my_bind[1];
  const char *concat_arg0 = "concat_with_";
  enum { OUT_BUFF_SIZE = 30 };
  char out_buff[OUT_BUFF_SIZE];
  char canonical_buff[OUT_BUFF_SIZE];
  ulong out_length;
  const char *stmt_text;
  int rc;

  myheader("test_bug3796");

  /* Create and fill test table */
  stmt_text = "DROP TABLE IF EXISTS t1";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);

  stmt_text = "CREATE TABLE t1 (a INT, b VARCHAR(30))";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);

  stmt_text = "INSERT INTO t1 VALUES(1, 'ONE'), (2, 'TWO')";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);

  /* Create statement handle and prepare it with select */
  stmt = mysql_stmt_init(mysql);
  stmt_text = "SELECT concat(?, b) FROM t1";

  rc = mysql_stmt_prepare(stmt, stmt_text, (ulong)strlen(stmt_text));
  check_execute(stmt, rc);

  /* Bind input buffers */
  memset(my_bind, 0, sizeof(my_bind));

  my_bind[0].buffer_type = MYSQL_TYPE_STRING;
  my_bind[0].buffer = const_cast<char *>(concat_arg0);
  my_bind[0].buffer_length = (ulong)strlen(concat_arg0);

  mysql_stmt_bind_param(stmt, my_bind);

  /* Execute the select statement */
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  my_bind[0].buffer = (void *)out_buff;
  my_bind[0].buffer_length = OUT_BUFF_SIZE;
  my_bind[0].length = &out_length;

  mysql_stmt_bind_result(stmt, my_bind);

  rc = mysql_stmt_fetch(stmt);
  if (!opt_silent) printf("Concat result: '%s'\n", out_buff);
  check_execute(stmt, rc);
  my_stpcpy(canonical_buff, concat_arg0);
  strcat(canonical_buff, "ONE");
  DIE_UNLESS(strlen(canonical_buff) == out_length &&
             strncmp(out_buff, canonical_buff, out_length) == 0);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);
  my_stpcpy(canonical_buff + strlen(concat_arg0), "TWO");
  DIE_UNLESS(strlen(canonical_buff) == out_length &&
             strncmp(out_buff, canonical_buff, out_length) == 0);
  if (!opt_silent) printf("Concat result: '%s'\n", out_buff);

  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == MYSQL_NO_DATA);

  mysql_stmt_close(stmt);

  stmt_text = "DROP TABLE IF EXISTS t1";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);
}

static void test_bug4026() {
  MYSQL_STMT *stmt;
  MYSQL_BIND my_bind[2];
  MYSQL_TIME time_in, time_out;
  MYSQL_TIME datetime_in, datetime_out;
  const char *stmt_text;
  int rc;

  myheader("test_bug4026");

  /* Check that microseconds are inserted and selected successfully */

  /* Create a statement handle and prepare it with select */
  stmt = mysql_stmt_init(mysql);
  stmt_text = "SELECT ?, ?";

  rc = mysql_stmt_prepare(stmt, stmt_text, (ulong)strlen(stmt_text));
  check_execute(stmt, rc);

  /* Bind input buffers */
  memset(my_bind, 0, sizeof(my_bind));
  memset(&time_in, 0, sizeof(time_in));
  memset(&time_out, 0, sizeof(time_out));
  memset(&datetime_in, 0, sizeof(datetime_in));
  memset(&datetime_out, 0, sizeof(datetime_out));

  my_bind[0].buffer_type = MYSQL_TYPE_TIME;
  my_bind[0].buffer = (void *)&time_in;
  my_bind[1].buffer_type = MYSQL_TYPE_DATETIME;
  my_bind[1].buffer = (void *)&datetime_in;

  time_in.hour = 23;
  time_in.minute = 59;
  time_in.second = 59;
  time_in.second_part = 123456;
  /*
    This is not necessary, just to make DIE_UNLESS below work: this field
    is filled in when time is received from server
  */
  time_in.time_type = MYSQL_TIMESTAMP_TIME;

  datetime_in = time_in;
  datetime_in.year = 2003;
  datetime_in.month = 12;
  datetime_in.day = 31;
  datetime_in.time_type = MYSQL_TIMESTAMP_DATETIME;

  mysql_stmt_bind_param(stmt, my_bind);

  /* Execute the select statement */
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  my_bind[0].buffer = (void *)&time_out;
  my_bind[1].buffer = (void *)&datetime_out;

  mysql_stmt_bind_result(stmt, my_bind);

  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == 0);
  if (!opt_silent) {
    printf("%d:%d:%d.%lu\n", time_out.hour, time_out.minute, time_out.second,
           time_out.second_part);
    printf("%d-%d-%d %d:%d:%d.%lu\n", datetime_out.year, datetime_out.month,
           datetime_out.day, datetime_out.hour, datetime_out.minute,
           datetime_out.second, datetime_out.second_part);
  }
  DIE_UNLESS(memcmp(&time_in, &time_out, sizeof(time_in)) == 0);
  DIE_UNLESS(memcmp(&datetime_in, &datetime_out, sizeof(datetime_in)) == 0);
  mysql_stmt_close(stmt);
}

static void test_bug4079() {
  MYSQL_STMT *stmt;
  MYSQL_BIND my_bind[1];
  const char *stmt_text;
  uint32 res;
  int rc;

  myheader("test_bug4079");

  /* Create and fill table */
  mysql_query(mysql, "DROP TABLE IF EXISTS t1");
  mysql_query(mysql, "CREATE TABLE t1 (a int)");
  mysql_query(mysql, "INSERT INTO t1 VALUES (1), (2)");

  /* Prepare erroneous statement */
  stmt = mysql_stmt_init(mysql);
  stmt_text = "SELECT 1 < (SELECT a FROM t1)";

  rc = mysql_stmt_prepare(stmt, stmt_text, (ulong)strlen(stmt_text));
  check_execute(stmt, rc);

  /* Execute the select statement */
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  /* Bind input buffers */
  memset(my_bind, 0, sizeof(my_bind));

  my_bind[0].buffer_type = MYSQL_TYPE_LONG;
  my_bind[0].buffer = (void *)&res;

  mysql_stmt_bind_result(stmt, my_bind);

  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc != 0 && rc != MYSQL_NO_DATA);
  if (!opt_silent)
    printf("Got error from mysql_stmt_fetch (as expected):\n%s\n",
           mysql_stmt_error(stmt));
  /* buggy version of libmysql hanged up here */
  mysql_stmt_close(stmt);
}

static void test_bug4236() {
  MYSQL_STMT *stmt;
  const char *stmt_text;
  int rc;
  MYSQL_STMT backup;

  myheader("test_bug4236");

  stmt = mysql_stmt_init(mysql);

  /* mysql_stmt_execute() of statement with statement id= 0 crashed server */
  stmt_text = "SELECT 1";
  /* We need to prepare statement to pass by possible check in libmysql */
  rc = mysql_stmt_prepare(stmt, stmt_text, (ulong)strlen(stmt_text));
  check_execute(stmt, rc);
  /* Hack to check that server works OK if statement wasn't found */
  backup.stmt_id = stmt->stmt_id;
  stmt->stmt_id = 0;
  rc = mysql_stmt_execute(stmt);
  DIE_UNLESS(rc);
  /* Restore original statement id to be able to reprepare it */
  stmt->stmt_id = backup.stmt_id;

  mysql_stmt_close(stmt);
}

static void test_bug4030() {
  MYSQL_STMT *stmt;
  MYSQL_BIND my_bind[3];
  MYSQL_TIME time_canonical, time_out;
  MYSQL_TIME date_canonical, date_out;
  MYSQL_TIME datetime_canonical, datetime_out;
  const char *stmt_text;
  int rc;

  myheader("test_bug4030");

  /* Check that microseconds are inserted and selected successfully */

  /* Execute a query with time values in prepared mode */
  stmt = mysql_stmt_init(mysql);
  stmt_text =
      "SELECT '23:59:59.123456', '2003-12-31', "
      "'2003-12-31 23:59:59.123456'";
  rc = mysql_stmt_prepare(stmt, stmt_text, (ulong)strlen(stmt_text));
  check_execute(stmt, rc);
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  /* Bind output buffers */
  memset(my_bind, 0, sizeof(my_bind));
  memset(&time_canonical, 0, sizeof(time_canonical));
  memset(&time_out, 0, sizeof(time_out));
  memset(&date_canonical, 0, sizeof(date_canonical));
  memset(&date_out, 0, sizeof(date_out));
  memset(&datetime_canonical, 0, sizeof(datetime_canonical));
  memset(&datetime_out, 0, sizeof(datetime_out));

  my_bind[0].buffer_type = MYSQL_TYPE_TIME;
  my_bind[0].buffer = (void *)&time_out;
  my_bind[1].buffer_type = MYSQL_TYPE_DATE;
  my_bind[1].buffer = (void *)&date_out;
  my_bind[2].buffer_type = MYSQL_TYPE_DATETIME;
  my_bind[2].buffer = (void *)&datetime_out;

  time_canonical.hour = 23;
  time_canonical.minute = 59;
  time_canonical.second = 59;
  time_canonical.second_part = 123456;
  time_canonical.time_type = MYSQL_TIMESTAMP_TIME;

  date_canonical.year = 2003;
  date_canonical.month = 12;
  date_canonical.day = 31;
  date_canonical.time_type = MYSQL_TIMESTAMP_DATE;

  datetime_canonical = time_canonical;
  datetime_canonical.year = 2003;
  datetime_canonical.month = 12;
  datetime_canonical.day = 31;
  datetime_canonical.time_type = MYSQL_TIMESTAMP_DATETIME;

  mysql_stmt_bind_result(stmt, my_bind);

  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == 0);
  if (!opt_silent) {
    printf("%d:%d:%d.%lu\n", time_out.hour, time_out.minute, time_out.second,
           time_out.second_part);
    printf("%d-%d-%d\n", date_out.year, date_out.month, date_out.day);
    printf("%d-%d-%d %d:%d:%d.%lu\n", datetime_out.year, datetime_out.month,
           datetime_out.day, datetime_out.hour, datetime_out.minute,
           datetime_out.second, datetime_out.second_part);
  }
  DIE_UNLESS(memcmp(&time_canonical, &time_out, sizeof(time_out)) == 0);
  DIE_UNLESS(memcmp(&date_canonical, &date_out, sizeof(date_out)) == 0);
  DIE_UNLESS(memcmp(&datetime_canonical, &datetime_out, sizeof(datetime_out)) ==
             0);
  mysql_stmt_close(stmt);
}

static void test_view() {
  MYSQL_STMT *stmt;
  int rc, i;
  MYSQL_BIND my_bind[1];
  char str_data[50];
  ulong length = 0L;
  bool is_null = false;
  const char *query = "SELECT COUNT(*) FROM v1 WHERE SERVERNAME=?";

  myheader("test_view");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS t1,t2,t3,v1");
  myquery(rc);

  rc = mysql_query(mysql, "DROP VIEW IF EXISTS v1,t1,t2,t3");
  myquery(rc);
  rc = mysql_query(mysql,
                   "CREATE TABLE t1 ("
                   " SERVERGRP varchar(20) NOT NULL default '', "
                   " DBINSTANCE varchar(20) NOT NULL default '', "
                   " PRIMARY KEY  (SERVERGRP)) "
                   " CHARSET=latin1 collate=latin1_bin");
  myquery(rc);
  rc = mysql_query(mysql,
                   "CREATE TABLE t2 ("
                   " SERVERNAME varchar(20) NOT NULL, "
                   " SERVERGRP varchar(20) NOT NULL, "
                   " PRIMARY KEY (SERVERNAME)) "
                   " CHARSET=latin1 COLLATE latin1_bin");
  myquery(rc);
  rc = mysql_query(mysql,
                   "CREATE TABLE t3 ("
                   " SERVERGRP varchar(20) BINARY NOT NULL, "
                   " TABNAME varchar(30) NOT NULL, MAPSTATE char(1) NOT NULL, "
                   " ACTSTATE char(1) NOT NULL , "
                   " LOCAL_NAME varchar(30) NOT NULL, "
                   " CHG_DATE varchar(8) NOT NULL default '00000000', "
                   " CHG_TIME varchar(6) NOT NULL default '000000', "
                   " MXUSER varchar(12) NOT NULL default '', "
                   " PRIMARY KEY (SERVERGRP, TABNAME, MAPSTATE, ACTSTATE, "
                   " LOCAL_NAME)) CHARSET=latin1 COLLATE latin1_bin");
  myquery(rc);
  rc = mysql_query(mysql,
                   "CREATE VIEW v1 AS select sql_no_cache"
                   " T0001.SERVERNAME AS SERVERNAME, T0003.TABNAME AS"
                   " TABNAME,T0003.LOCAL_NAME AS LOCAL_NAME,T0002.DBINSTANCE AS"
                   " DBINSTANCE from t2 T0001 join t1 T0002 join t3 T0003 where"
                   " ((T0002.SERVERGRP = T0001.SERVERGRP) and"
                   " (T0002.SERVERGRP = T0003.SERVERGRP)"
                   " and (T0003.MAPSTATE = _latin1'A') and"
                   " (T0003.ACTSTATE = _latin1' '))");
  myquery(rc);

  stmt = mysql_stmt_init(mysql);
  rc = mysql_stmt_prepare(stmt, query, (ulong)strlen(query));
  check_execute(stmt, rc);

  my_stpcpy(str_data, "TEST");
  memset(my_bind, 0, sizeof(my_bind));
  my_bind[0].buffer_type = MYSQL_TYPE_STRING;
  my_bind[0].buffer = (char *)&str_data;
  my_bind[0].buffer_length = 50;
  my_bind[0].length = &length;
  length = 4;
  my_bind[0].is_null = &is_null;
  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  for (i = 0; i < 3; i++) {
    rc = mysql_stmt_execute(stmt);
    check_execute(stmt, rc);
    rc = my_process_stmt_result(stmt);
    DIE_UNLESS(1 == rc);
  }
  mysql_stmt_close(stmt);

  rc = mysql_query(mysql, "DROP TABLE t1,t2,t3");
  myquery(rc);
  rc = mysql_query(mysql, "DROP VIEW v1");
  myquery(rc);
}

static void test_view_where() {
  MYSQL_STMT *stmt;
  int rc, i;
  const char *query = "select v1.c,v2.c from v1, v2";

  myheader("test_view_where");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS t1,v1,v2");
  myquery(rc);

  rc = mysql_query(mysql, "DROP VIEW IF EXISTS v1,v2,t1");
  myquery(rc);
  rc = mysql_query(mysql, "CREATE TABLE t1 (a int, b int)");
  myquery(rc);
  rc = mysql_query(mysql,
                   "insert into t1 values (1,2), (1,3), (2,4), (2,5), (3,10)");
  myquery(rc);
  rc = mysql_query(mysql, "create view v1 (c) as select b from t1 where a<3");
  myquery(rc);
  rc = mysql_query(mysql, "create view v2 (c) as select b from t1 where a>=3");
  myquery(rc);

  stmt = mysql_stmt_init(mysql);
  rc = mysql_stmt_prepare(stmt, query, (ulong)strlen(query));
  check_execute(stmt, rc);

  for (i = 0; i < 3; i++) {
    rc = mysql_stmt_execute(stmt);
    check_execute(stmt, rc);
    rc = my_process_stmt_result(stmt);
    DIE_UNLESS(4 == rc);
  }
  mysql_stmt_close(stmt);

  rc = mysql_query(mysql, "DROP TABLE t1");
  myquery(rc);
  rc = mysql_query(mysql, "DROP VIEW v1, v2");
  myquery(rc);
}

static void test_view_2where() {
  MYSQL_STMT *stmt;
  int rc, i;
  MYSQL_BIND my_bind[8];
  char parms[8][100];
  ulong length[8];
  const char *query =
      "select relid, report, handle, log_group, username, variant, type, "
      "version, erfdat, erftime, erfname, aedat, aetime, aename, dependvars, "
      "inactive from V_LTDX where mandt = ? and relid = ? and report = ? and "
      "handle = ? and log_group = ? and username in ( ? , ? ) and type = ?";

  myheader("test_view_2where");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS LTDX");
  myquery(rc);
  rc = mysql_query(mysql, "DROP VIEW IF EXISTS V_LTDX");
  myquery(rc);
  rc = mysql_query(mysql,
                   "CREATE TABLE LTDX (MANDT char(3) NOT NULL default '000', "
                   " RELID char(2) NOT NULL, REPORT varchar(40) NOT NULL,"
                   " HANDLE varchar(4) NOT NULL, LOG_GROUP varchar(4) NOT NULL,"
                   " USERNAME varchar(12) NOT NULL,"
                   " VARIANT varchar(12) NOT NULL,"
                   " TYPE char(1) NOT NULL, SRTF2 int(11) NOT NULL,"
                   " VERSION varchar(6) NOT NULL default '000000',"
                   " ERFDAT varchar(8) NOT NULL default '00000000',"
                   " ERFTIME varchar(6) NOT NULL default '000000',"
                   " ERFNAME varchar(12) NOT NULL,"
                   " AEDAT varchar(8) NOT NULL default '00000000',"
                   " AETIME varchar(6) NOT NULL default '000000',"
                   " AENAME varchar(12) NOT NULL,"
                   " DEPENDVARS varchar(10) NOT NULL,"
                   " INACTIVE char(1) NOT NULL, CLUSTR smallint(6) NOT NULL,"
                   " CLUSTD blob,"
                   " PRIMARY KEY (MANDT, RELID, REPORT, HANDLE, LOG_GROUP, "
                   "USERNAME, VARIANT, TYPE, SRTF2))"
                   " CHARSET=latin1 COLLATE latin1_bin");
  myquery(rc);
  rc = mysql_query(mysql,
                   "CREATE VIEW V_LTDX AS select T0001.MANDT AS "
                   " MANDT,T0001.RELID AS RELID,T0001.REPORT AS "
                   " REPORT,T0001.HANDLE AS HANDLE,T0001.LOG_GROUP AS "
                   " LOG_GROUP,T0001.USERNAME AS USERNAME,T0001.VARIANT AS "
                   " VARIANT,T0001.TYPE AS TYPE,T0001.VERSION AS "
                   " VERSION,T0001.ERFDAT AS ERFDAT,T0001.ERFTIME AS "
                   " ERFTIME,T0001.ERFNAME AS ERFNAME,T0001.AEDAT AS "
                   " AEDAT,T0001.AETIME AS AETIME,T0001.AENAME AS "
                   " AENAME,T0001.DEPENDVARS AS DEPENDVARS,T0001.INACTIVE AS "
                   " INACTIVE from LTDX T0001 where (T0001.SRTF2 = 0)");
  myquery(rc);
  memset(my_bind, 0, sizeof(my_bind));
  for (i = 0; i < 8; i++) {
    my_stpcpy(parms[i], "1");
    my_bind[i].buffer_type = MYSQL_TYPE_VAR_STRING;
    my_bind[i].buffer = (char *)&parms[i];
    my_bind[i].buffer_length = 100;
    my_bind[i].is_null = nullptr;
    my_bind[i].length = &length[i];
    length[i] = 1;
  }
  stmt = mysql_stmt_init(mysql);
  rc = mysql_stmt_prepare(stmt, query, (ulong)strlen(query));
  check_execute(stmt, rc);

  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);
  rc = my_process_stmt_result(stmt);
  DIE_UNLESS(0 == rc);

  mysql_stmt_close(stmt);

  rc = mysql_query(mysql, "DROP VIEW V_LTDX");
  myquery(rc);
  rc = mysql_query(mysql, "DROP TABLE LTDX");
  myquery(rc);
}

static void test_view_star() {
  MYSQL_STMT *stmt;
  int rc, i;
  MYSQL_BIND my_bind[8];
  char parms[8][100];
  ulong length[8];
  const char *query = "SELECT * FROM vt1 WHERE a IN (?,?)";

  myheader("test_view_star");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS t1, vt1");
  myquery(rc);
  rc = mysql_query(mysql, "DROP VIEW IF EXISTS t1, vt1");
  myquery(rc);
  rc = mysql_query(mysql, "CREATE TABLE t1 (a int)");
  myquery(rc);
  rc = mysql_query(mysql, "CREATE VIEW vt1 AS SELECT a FROM t1");
  myquery(rc);
  memset(my_bind, 0, sizeof(my_bind));
  for (i = 0; i < 2; i++) {
    sprintf((char *)&parms[i], "%d", i);
    my_bind[i].buffer_type = MYSQL_TYPE_VAR_STRING;
    my_bind[i].buffer = (char *)&parms[i];
    my_bind[i].buffer_length = 100;
    my_bind[i].is_null = nullptr;
    my_bind[i].length = &length[i];
    length[i] = 1;
  }

  stmt = mysql_stmt_init(mysql);
  rc = mysql_stmt_prepare(stmt, query, (ulong)strlen(query));
  check_execute(stmt, rc);

  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  for (i = 0; i < 3; i++) {
    rc = mysql_stmt_execute(stmt);
    check_execute(stmt, rc);
    rc = my_process_stmt_result(stmt);
    DIE_UNLESS(0 == rc);
  }

  mysql_stmt_close(stmt);

  rc = mysql_query(mysql, "DROP TABLE t1");
  myquery(rc);
  rc = mysql_query(mysql, "DROP VIEW vt1");
  myquery(rc);
}

static void test_view_insert() {
  MYSQL_STMT *insert_stmt, *select_stmt;
  int rc, i;
  MYSQL_BIND my_bind[1];
  int my_val = 0;
  ulong my_length = 0L;
  bool my_null = false;
  const char *query = "insert into v1 values (?)";

  myheader("test_view_insert");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS t1,v1");
  myquery(rc);
  rc = mysql_query(mysql, "DROP VIEW IF EXISTS t1,v1");
  myquery(rc);

  rc = mysql_query(mysql, "create table t1 (a int, primary key (a))");
  myquery(rc);

  rc = mysql_query(mysql, "create view v1 as select a from t1 where a>=1");
  myquery(rc);

  insert_stmt = mysql_stmt_init(mysql);
  rc = mysql_stmt_prepare(insert_stmt, query, (ulong)strlen(query));
  check_execute(insert_stmt, rc);
  query = "select * from t1";
  select_stmt = mysql_stmt_init(mysql);
  rc = mysql_stmt_prepare(select_stmt, query, (ulong)strlen(query));
  check_execute(select_stmt, rc);

  memset(my_bind, 0, sizeof(my_bind));
  my_bind[0].buffer_type = MYSQL_TYPE_LONG;
  my_bind[0].buffer = (char *)&my_val;
  my_bind[0].length = &my_length;
  my_bind[0].is_null = &my_null;
  rc = mysql_stmt_bind_param(insert_stmt, my_bind);
  check_execute(insert_stmt, rc);

  for (i = 0; i < 3; i++) {
    int rowcount = 0;
    my_val = i;

    rc = mysql_stmt_execute(insert_stmt);
    check_execute(insert_stmt, rc);

    rc = mysql_stmt_execute(select_stmt);
    check_execute(select_stmt, rc);
    rowcount = (int)my_process_stmt_result(select_stmt);
    DIE_UNLESS((i + 1) == rowcount);
  }
  mysql_stmt_close(insert_stmt);
  mysql_stmt_close(select_stmt);

  rc = mysql_query(mysql, "DROP VIEW v1");
  myquery(rc);
  rc = mysql_query(mysql, "DROP TABLE t1");
  myquery(rc);
}

static void test_left_join_view() {
  MYSQL_STMT *stmt;
  int rc, i;
  const char *query = "select t1.a, v1.x from t1 left join v1 on (t1.a= v1.x);";

  myheader("test_left_join_view");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS t1,v1");
  myquery(rc);

  rc = mysql_query(mysql, "DROP VIEW IF EXISTS v1,t1");
  myquery(rc);
  rc = mysql_query(mysql, "CREATE TABLE t1 (a int)");
  myquery(rc);
  rc = mysql_query(mysql, "insert into t1 values (1), (2), (3)");
  myquery(rc);
  rc = mysql_query(mysql, "create view v1 (x) as select a from t1 where a > 1");
  myquery(rc);
  stmt = mysql_stmt_init(mysql);
  rc = mysql_stmt_prepare(stmt, query, (ulong)strlen(query));
  check_execute(stmt, rc);

  for (i = 0; i < 3; i++) {
    rc = mysql_stmt_execute(stmt);
    check_execute(stmt, rc);
    rc = my_process_stmt_result(stmt);
    DIE_UNLESS(3 == rc);
  }
  mysql_stmt_close(stmt);

  rc = mysql_query(mysql, "DROP VIEW v1");
  myquery(rc);
  rc = mysql_query(mysql, "DROP TABLE t1");
  myquery(rc);
}

static void test_view_insert_fields() {
  MYSQL_STMT *stmt;
  char parm[11][1000];
  ulong l[11];
  int rc, i;
  MYSQL_BIND my_bind[11];
  const char *query =
      "INSERT INTO `v1` ( `K1C4` ,`K2C4` ,`K3C4` ,`K4N4` ,`F1C4` ,`F2I4` "
      ",`F3N5` ,`F7F8` ,`F6N4` ,`F5C8` ,`F9D8` ) VALUES( ? , ? , ? , ? , ? , ? "
      ", ? , ? , ? , ? , ? )";

  myheader("test_view_insert_fields");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS t1, v1");
  myquery(rc);
  rc = mysql_query(mysql, "DROP VIEW IF EXISTS t1, v1");
  myquery(rc);
  rc =
      mysql_query(mysql,
                  "CREATE TABLE t1 (K1C4 varchar(4) NOT NULL,"
                  "K2C4 varchar(4) NOT NULL, K3C4 varchar(4) NOT NULL,"
                  "K4N4 varchar(4) NOT NULL default '0000',"
                  "F1C4 varchar(4) NOT NULL, F2I4 int(11) NOT NULL,"
                  "F3N5 varchar(5) NOT NULL default '00000',"
                  "F4I4 int(11) NOT NULL default '0', F5C8 varchar(8) NOT NULL,"
                  "F6N4 varchar(4) NOT NULL default '0000',"
                  "F7F8 double NOT NULL default '0',"
                  "F8F8 double NOT NULL default '0',"
                  "F9D8 decimal(8,2) NOT NULL default '0.00',"
                  "PRIMARY KEY (K1C4,K2C4,K3C4,K4N4)) "
                  "CHARSET=latin1 COLLATE latin1_bin");
  myquery(rc);
  rc = mysql_query(mysql,
                   "CREATE VIEW v1 AS select "
                   " K1C4 AS K1C4, K2C4 AS K2C4, K3C4 AS K3C4, K4N4 AS K4N4, "
                   " F1C4 AS F1C4, F2I4 AS F2I4, F3N5 AS F3N5,"
                   " F7F8 AS F7F8, F6N4 AS F6N4, F5C8 AS F5C8, F9D8 AS F9D8"
                   " from t1 T0001");

  memset(my_bind, 0, sizeof(my_bind));
  memset(parm, 0, sizeof(parm));
  for (i = 0; i < 11; i++) {
    l[i] = 20;
    my_bind[i].buffer_type = MYSQL_TYPE_STRING;
    my_bind[i].is_null = nullptr;
    my_bind[i].buffer = (char *)&parm[i];

    my_stpcpy(parm[i], "1");
    my_bind[i].buffer_length = 2;
    my_bind[i].length = &l[i];
  }
  stmt = mysql_stmt_init(mysql);
  rc = mysql_stmt_prepare(stmt, query, (ulong)strlen(query));
  check_execute(stmt, rc);
  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);
  mysql_stmt_close(stmt);

  query = "select * from t1";
  stmt = mysql_stmt_init(mysql);
  rc = mysql_stmt_prepare(stmt, query, (ulong)strlen(query));
  check_execute(stmt, rc);
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);
  rc = my_process_stmt_result(stmt);
  DIE_UNLESS(1 == rc);

  mysql_stmt_close(stmt);
  rc = mysql_query(mysql, "DROP VIEW v1");
  myquery(rc);
  rc = mysql_query(mysql, "DROP TABLE t1");
  myquery(rc);
}

static void test_bug5126() {
  MYSQL_STMT *stmt;
  MYSQL_BIND my_bind[2];
  int32 c1, c2;
  const char *stmt_text;
  int rc;

  myheader("test_bug5126");

  stmt_text = "DROP TABLE IF EXISTS t1";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);

  stmt_text = "CREATE TABLE t1 (a mediumint, b int)";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);

  stmt_text = "INSERT INTO t1 VALUES (8386608, 1)";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);

  stmt = mysql_stmt_init(mysql);
  stmt_text = "SELECT a, b FROM t1";
  rc = mysql_stmt_prepare(stmt, stmt_text, (ulong)strlen(stmt_text));
  check_execute(stmt, rc);
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  /* Bind output buffers */
  memset(my_bind, 0, sizeof(my_bind));

  my_bind[0].buffer_type = MYSQL_TYPE_LONG;
  my_bind[0].buffer = &c1;
  my_bind[1].buffer_type = MYSQL_TYPE_LONG;
  my_bind[1].buffer = &c2;

  mysql_stmt_bind_result(stmt, my_bind);

  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == 0);
  DIE_UNLESS(c1 == 8386608 && c2 == 1);
  if (!opt_silent) printf("%ld, %ld\n", (long)c1, (long)c2);
  mysql_stmt_close(stmt);
}

static void test_bug4231() {
  MYSQL_STMT *stmt;
  MYSQL_BIND my_bind[2];
  MYSQL_TIME tm[2];
  const char *stmt_text;
  int rc;

  myheader("test_bug4231");

  stmt_text = "DROP TABLE IF EXISTS t1";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);

  stmt_text = "CREATE TABLE t1 (a int)";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);

  stmt_text = "INSERT INTO t1 VALUES (1)";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);

  stmt = mysql_stmt_init(mysql);
  stmt_text = "SELECT a FROM t1 WHERE ? = ?";
  rc = mysql_stmt_prepare(stmt, stmt_text, (ulong)strlen(stmt_text));
  check_execute(stmt, rc);

  /* Bind input buffers */
  memset(my_bind, 0, sizeof(my_bind));
  memset(tm, 0, sizeof(tm));

  my_bind[0].buffer_type = MYSQL_TYPE_DATE;
  my_bind[0].buffer = &tm[0];
  my_bind[1].buffer_type = MYSQL_TYPE_DATE;
  my_bind[1].buffer = &tm[1];

  mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  /*
    First set server-side params to some non-zero non-equal values:
    then we will check that they are not used when client sends
    new (zero) times.
  */
  tm[0].time_type = MYSQL_TIMESTAMP_DATE;
  tm[0].year = 2000;
  tm[0].month = 1;
  tm[0].day = 1;
  tm[1] = tm[0];
  --tm[1].year; /* tm[0] != tm[1] */

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);

  /* binds are unequal, no rows should be returned */
  DIE_UNLESS(rc == MYSQL_NO_DATA);

  /* Set one of the dates to zero */
  tm[0].year = tm[0].month = tm[0].day = 0;
  tm[1] = tm[0];
  mysql_stmt_execute(stmt);
  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == 0);

  mysql_stmt_close(stmt);
  stmt_text = "DROP TABLE t1";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);
}

static void test_bug5399() {
/*
  Ascii 97 is 'a', which gets mapped to Ascii 65 'A' unless internal
  statement id hash in the server uses binary collation.
*/
#define NUM_OF_USED_STMT 97
  MYSQL_STMT *stmt_list[NUM_OF_USED_STMT];
  MYSQL_STMT **stmt;
  MYSQL_BIND my_bind[1];
  char buff[600];
  int rc;
  int32 no;

  myheader("test_bug5399");

  memset(my_bind, 0, sizeof(my_bind));
  my_bind[0].buffer_type = MYSQL_TYPE_LONG;
  my_bind[0].buffer = &no;

  for (stmt = stmt_list; stmt != stmt_list + NUM_OF_USED_STMT; ++stmt) {
    sprintf(buff, "select %d", (int)(stmt - stmt_list));
    *stmt = mysql_stmt_init(mysql);
    rc = mysql_stmt_prepare(*stmt, buff, (ulong)strlen(buff));
    check_execute(*stmt, rc);
    mysql_stmt_bind_result(*stmt, my_bind);
  }
  if (!opt_silent) printf("%d statements prepared.\n", NUM_OF_USED_STMT);

  for (stmt = stmt_list; stmt != stmt_list + NUM_OF_USED_STMT; ++stmt) {
    rc = mysql_stmt_execute(*stmt);
    check_execute(*stmt, rc);
    rc = mysql_stmt_store_result(*stmt);
    check_execute(*stmt, rc);
    rc = mysql_stmt_fetch(*stmt);
    DIE_UNLESS(rc == 0);
    DIE_UNLESS((int32)(stmt - stmt_list) == no);
  }

  for (stmt = stmt_list; stmt != stmt_list + NUM_OF_USED_STMT; ++stmt)
    mysql_stmt_close(*stmt);
#undef NUM_OF_USED_STMT
}

static void test_bug5194() {
  MYSQL_STMT *stmt;
  MYSQL_BIND *my_bind;
  char *query;
  char *param_str;
  ulong param_str_length;
  const char *stmt_text;
  int rc;
  float float_array[250] = {
      0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,
      0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,
      0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,
      0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,
      0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,
      0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,
      0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,
      0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,
      0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,
      0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,
      0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.25, 0.25,
      0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25,
      0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25,
      0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25,
      0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25,
      0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25,
      0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25,
      0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25,
      0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25,
      0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25,
      0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25};
  float *fa_ptr = float_array;
  /* Number of columns per row */
  const int COLUMN_COUNT = sizeof(float_array) / sizeof(*float_array);
  /* Number of rows per bulk insert to start with */
  const int MIN_ROWS_PER_INSERT = 262;
  /* Max number of rows per bulk insert to end with */
  const int MAX_ROWS_PER_INSERT = 300;
  const int MAX_PARAM_COUNT = COLUMN_COUNT * MAX_ROWS_PER_INSERT;
  const char *query_template = "insert into t1 values %s";
  const int CHARS_PER_PARAM = 5; /* space needed to place ", ?" in the query */
  const int uint16_max = 65535;
  int nrows, i;

  myheader("test_bug5194");

  stmt_text = "drop table if exists t1";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));

  stmt_text =
      "create table if not exists t1"
      "(c1 float, c2 float, c3 float, c4 float, c5 float, c6 float, "
      "c7 float, c8 float, c9 float, c10 float, c11 float, c12 float, "
      "c13 float, c14 float, c15 float, c16 float, c17 float, c18 float, "
      "c19 float, c20 float, c21 float, c22 float, c23 float, c24 float, "
      "c25 float, c26 float, c27 float, c28 float, c29 float, c30 float, "
      "c31 float, c32 float, c33 float, c34 float, c35 float, c36 float, "
      "c37 float, c38 float, c39 float, c40 float, c41 float, c42 float, "
      "c43 float, c44 float, c45 float, c46 float, c47 float, c48 float, "
      "c49 float, c50 float, c51 float, c52 float, c53 float, c54 float, "
      "c55 float, c56 float, c57 float, c58 float, c59 float, c60 float, "
      "c61 float, c62 float, c63 float, c64 float, c65 float, c66 float, "
      "c67 float, c68 float, c69 float, c70 float, c71 float, c72 float, "
      "c73 float, c74 float, c75 float, c76 float, c77 float, c78 float, "
      "c79 float, c80 float, c81 float, c82 float, c83 float, c84 float, "
      "c85 float, c86 float, c87 float, c88 float, c89 float, c90 float, "
      "c91 float, c92 float, c93 float, c94 float, c95 float, c96 float, "
      "c97 float, c98 float, c99 float, c100 float, c101 float, c102 float, "
      "c103 float, c104 float, c105 float, c106 float, c107 float, c108 float, "
      "c109 float, c110 float, c111 float, c112 float, c113 float, c114 float, "
      "c115 float, c116 float, c117 float, c118 float, c119 float, c120 float, "
      "c121 float, c122 float, c123 float, c124 float, c125 float, c126 float, "
      "c127 float, c128 float, c129 float, c130 float, c131 float, c132 float, "
      "c133 float, c134 float, c135 float, c136 float, c137 float, c138 float, "
      "c139 float, c140 float, c141 float, c142 float, c143 float, c144 float, "
      "c145 float, c146 float, c147 float, c148 float, c149 float, c150 float, "
      "c151 float, c152 float, c153 float, c154 float, c155 float, c156 float, "
      "c157 float, c158 float, c159 float, c160 float, c161 float, c162 float, "
      "c163 float, c164 float, c165 float, c166 float, c167 float, c168 float, "
      "c169 float, c170 float, c171 float, c172 float, c173 float, c174 float, "
      "c175 float, c176 float, c177 float, c178 float, c179 float, c180 float, "
      "c181 float, c182 float, c183 float, c184 float, c185 float, c186 float, "
      "c187 float, c188 float, c189 float, c190 float, c191 float, c192 float, "
      "c193 float, c194 float, c195 float, c196 float, c197 float, c198 float, "
      "c199 float, c200 float, c201 float, c202 float, c203 float, c204 float, "
      "c205 float, c206 float, c207 float, c208 float, c209 float, c210 float, "
      "c211 float, c212 float, c213 float, c214 float, c215 float, c216 float, "
      "c217 float, c218 float, c219 float, c220 float, c221 float, c222 float, "
      "c223 float, c224 float, c225 float, c226 float, c227 float, c228 float, "
      "c229 float, c230 float, c231 float, c232 float, c233 float, c234 float, "
      "c235 float, c236 float, c237 float, c238 float, c239 float, c240 float, "
      "c241 float, c242 float, c243 float, c244 float, c245 float, c246 float, "
      "c247 float, c248 float, c249 float, c250 float)";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);

  my_bind = (MYSQL_BIND *)malloc(MAX_PARAM_COUNT * sizeof(MYSQL_BIND));
  query = (char *)malloc(strlen(query_template) +
                         MAX_PARAM_COUNT * CHARS_PER_PARAM + 1);
  param_str = (char *)malloc(COLUMN_COUNT * CHARS_PER_PARAM);

  if (my_bind == nullptr || query == nullptr || param_str == nullptr) {
    fprintf(stderr, "Can't allocate enough memory for query structs\n");
    if (my_bind) free(my_bind);
    if (query) free(query);
    if (param_str) free(param_str);
    return;
  }

  stmt = mysql_stmt_init(mysql);

  /* setup a template for one row of parameters */
  sprintf(param_str, "(");
  for (i = 1; i < COLUMN_COUNT; ++i) strcat(param_str, "?, ");
  strcat(param_str, "?)");
  param_str_length = (ulong)strlen(param_str);

  /* setup bind array */
  memset(my_bind, 0, MAX_PARAM_COUNT * sizeof(MYSQL_BIND));
  for (i = 0; i < MAX_PARAM_COUNT; ++i) {
    my_bind[i].buffer_type = MYSQL_TYPE_FLOAT;
    my_bind[i].buffer = fa_ptr;
    if (++fa_ptr == float_array + COLUMN_COUNT) fa_ptr = float_array;
  }

  /*
    Test each number of rows per bulk insert, so that we can see where
    MySQL fails.
  */
  for (nrows = MIN_ROWS_PER_INSERT; nrows <= MAX_ROWS_PER_INSERT; ++nrows) {
    char *query_ptr;
    /* Create statement text for current number of rows */
    sprintf(query, query_template, param_str);
    query_ptr = query + strlen(query);
    for (i = 1; i < nrows; ++i) {
      memcpy(query_ptr, ", ", 2);
      query_ptr += 2;
      memcpy(query_ptr, param_str, param_str_length);
      query_ptr += param_str_length;
    }
    *query_ptr = '\0';

    rc = mysql_stmt_prepare(stmt, query, (ulong)(query_ptr - query));
    if (rc && nrows * COLUMN_COUNT > uint16_max) {
      if (!opt_silent)
        printf(
            "Failed to prepare a statement with %d placeholders "
            "(as expected).\n",
            nrows * COLUMN_COUNT);
      break;
    } else
      check_execute(stmt, rc);

    if (!opt_silent)
      printf("Insert: query length= %d, row count= %d, param count= %lu\n",
             (int)strlen(query), nrows, mysql_stmt_param_count(stmt));

    /* bind the parameter array and execute the query */
    rc = mysql_stmt_bind_param(stmt, my_bind);
    check_execute(stmt, rc);

    rc = mysql_stmt_execute(stmt);
    check_execute(stmt, rc);
    mysql_stmt_reset(stmt);
  }

  mysql_stmt_close(stmt);
  free(my_bind);
  free(query);
  free(param_str);
  stmt_text = "drop table t1";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);
}

static void test_bug5315() {
  MYSQL_STMT *stmt;
  const char *stmt_text;
  int rc;

  myheader("test_bug5315");

  stmt_text = "SELECT 1";
  stmt = mysql_stmt_init(mysql);
  rc = mysql_stmt_prepare(stmt, stmt_text, (ulong)strlen(stmt_text));
  DIE_UNLESS(rc == 0);
  if (!opt_silent) printf("Excuting mysql_change_user\n");
  mysql_change_user(mysql, opt_user, opt_password, current_db);
  if (!opt_silent) printf("Excuting mysql_stmt_execute\n");
  rc = mysql_stmt_execute(stmt);
  DIE_UNLESS(rc != 0);
  if (rc) {
    if (!opt_silent)
      printf("Got error (as expected): '%s'\n", mysql_stmt_error(stmt));
  }
  /* check that connection is OK */
  if (!opt_silent) printf("Excuting mysql_stmt_close\n");
  mysql_stmt_close(stmt);
  if (!opt_silent) printf("Excuting mysql_stmt_init\n");
  stmt = mysql_stmt_init(mysql);
  rc = mysql_stmt_prepare(stmt, stmt_text, (ulong)strlen(stmt_text));
  DIE_UNLESS(rc == 0);
  rc = mysql_stmt_execute(stmt);
  DIE_UNLESS(rc == 0);
  mysql_stmt_close(stmt);
}

static void test_bug6049() {
  MYSQL_STMT *stmt;
  MYSQL_BIND my_bind[1];
  MYSQL_RES *res;
  MYSQL_ROW row;
  const char *stmt_text;
  char buffer[30];
  ulong length;
  int rc;

  myheader("test_bug6049");

  if (mysql_get_server_version(mysql) < 50600) {
    if (!opt_silent)
      fprintf(stdout,
              "Skipping test_bug6049: this test cannot be executed "
              "on servers prior to 5.6 until bug#16433596 is fixed\n");
    return;
  }

  stmt_text = "SELECT MAKETIME(-25, 12, 12)";

  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);
  res = mysql_store_result(mysql);
  row = mysql_fetch_row(res);

  stmt = mysql_stmt_init(mysql);
  rc = mysql_stmt_prepare(stmt, stmt_text, (ulong)strlen(stmt_text));
  check_execute(stmt, rc);
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  memset(my_bind, 0, sizeof(my_bind));
  my_bind[0].buffer_type = MYSQL_TYPE_STRING;
  my_bind[0].buffer = &buffer;
  my_bind[0].buffer_length = (ulong)sizeof(buffer);
  my_bind[0].length = &length;

  mysql_stmt_bind_result(stmt, my_bind);
  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == 0);

  if (!opt_silent) {
    printf("Result from query: %s\n", row[0]);
    printf("Result from prepared statement: %s\n", (char *)buffer);
  }

  DIE_UNLESS(strcmp(row[0], (char *)buffer) == 0);

  mysql_free_result(res);
  mysql_stmt_close(stmt);
}

static void test_bug6058() {
  MYSQL_STMT *stmt;
  MYSQL_BIND my_bind[1];
  MYSQL_RES *res;
  MYSQL_ROW row;
  const char *stmt_text;
  char buffer[30];
  ulong length;
  int rc;

  myheader("test_bug6058");

  rc = mysql_query(mysql, "SET SQL_MODE=''");
  myquery(rc);

  stmt_text = "SELECT CAST('0000-00-00' AS DATE)";

  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);
  res = mysql_store_result(mysql);
  row = mysql_fetch_row(res);

  stmt = mysql_stmt_init(mysql);
  rc = mysql_stmt_prepare(stmt, stmt_text, (ulong)strlen(stmt_text));
  check_execute(stmt, rc);
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  memset(my_bind, 0, sizeof(my_bind));
  my_bind[0].buffer_type = MYSQL_TYPE_STRING;
  my_bind[0].buffer = &buffer;
  my_bind[0].buffer_length = (ulong)sizeof(buffer);
  my_bind[0].length = &length;

  mysql_stmt_bind_result(stmt, my_bind);
  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == 0);

  if (!opt_silent) {
    printf("Result from query: %s\n", row[0]);
    printf("Result from prepared statement: %s\n", buffer);
  }

  DIE_UNLESS(strcmp(row[0], buffer) == 0);

  mysql_free_result(res);
  mysql_stmt_close(stmt);
}

static void test_bug6059() {
  MYSQL_STMT *stmt;
  const char *stmt_text;

  myheader("test_bug6059");

  stmt_text = "SELECT 'foo' INTO OUTFILE 'x.3'";

  stmt = mysql_stmt_init(mysql);
  (void)mysql_stmt_prepare(stmt, stmt_text, (ulong)strlen(stmt_text));
  DIE_UNLESS(mysql_stmt_field_count(stmt) == 0);
  mysql_stmt_close(stmt);
}

static void test_bug6046() {
  MYSQL_STMT *stmt;
  const char *stmt_text;
  int rc;
  short b = 1;
  MYSQL_BIND my_bind[1];

  myheader("test_bug6046");

  stmt_text = "DROP TABLE IF EXISTS t1";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);
  stmt_text = "CREATE TABLE t1 (a int, b int)";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);
  stmt_text = "INSERT INTO t1 VALUES (1,1),(2,2),(3,1),(4,2)";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);

  stmt = mysql_stmt_init(mysql);

  stmt_text =
      "SELECT t1.a FROM t1 NATURAL JOIN t1 as X1 "
      "WHERE t1.b > ? ORDER BY t1.a";

  rc = mysql_stmt_prepare(stmt, stmt_text, (ulong)strlen(stmt_text));
  check_execute(stmt, rc);

  b = 1;
  memset(my_bind, 0, sizeof(my_bind));
  my_bind[0].buffer = &b;
  my_bind[0].buffer_type = MYSQL_TYPE_SHORT;

  mysql_stmt_bind_param(stmt, my_bind);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);
  mysql_stmt_store_result(stmt);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  mysql_stmt_close(stmt);
}

static void test_basic_cursors() {
  const char *basic_tables[] = {
      "DROP TABLE IF EXISTS t1, t2",

      "CREATE TABLE t1 "
      "(id INTEGER NOT NULL PRIMARY KEY, "
      " name VARCHAR(20) NOT NULL)",

      "INSERT INTO t1 (id, name) VALUES "
      "  (2, 'Ja'), (3, 'Ede'), "
      "  (4, 'Haag'), (5, 'Kabul'), "
      "  (6, 'Almere'), (7, 'Utrecht'), "
      "  (8, 'Qandahar'), (9, 'Amsterdam'), "
      "  (10, 'Amersfoort'), (11, 'Constantine')",

      "CREATE TABLE t2 "
      "(id INTEGER NOT NULL PRIMARY KEY, "
      " name VARCHAR(20) NOT NULL)",

      "INSERT INTO t2 (id, name) VALUES "
      "  (4, 'Guam'), (5, 'Aruba'), "
      "  (6, 'Angola'), (7, 'Albania'), "
      "  (8, 'Anguilla'), (9, 'Argentina'), "
      "  (10, 'Azerbaijan'), (11, 'Afghanistan'), "
      "  (12, 'Burkina Faso'), (13, 'Faroe Islands')"};
  const char *queries[] = {"SELECT * FROM t1", "SELECT * FROM t2"};

  DBUG_TRACE;
  myheader("test_basic_cursors");

  fill_tables(basic_tables, sizeof(basic_tables) / sizeof(*basic_tables));

  fetch_n(queries, sizeof(queries) / sizeof(*queries), USE_ROW_BY_ROW_FETCH);
  fetch_n(queries, sizeof(queries) / sizeof(*queries), USE_STORE_RESULT);
}

static void test_cursors_with_union() {
  const char *queries[] = {
      "SELECT t1.name FROM t1 UNION SELECT t2.name FROM t2",
      "SELECT t1.id FROM t1 WHERE t1.id < 5"};
  myheader("test_cursors_with_union");
  fetch_n(queries, sizeof(queries) / sizeof(*queries), USE_ROW_BY_ROW_FETCH);
  fetch_n(queries, sizeof(queries) / sizeof(*queries), USE_STORE_RESULT);
}

/*
  Altough mysql_create_db(), mysql_rm_db() are deprecated since 4.0 they
  should not crash server and should not hang in case of errors.

  Since those functions can't be seen in modern API we use simple_command()
  macro.
*/
static void test_bug6081() {
  int rc;
  myheader("test_bug6081");

  rc = simple_command(mysql, COM_DROP_DB, (uchar *)current_db,
                      (ulong)strlen(current_db), 0);
  if (rc == 0 && mysql_errno(mysql) != ER_UNKNOWN_COM_ERROR) {
    myerror(nullptr);                              /* purecov: inspected */
    die(__FILE__, __LINE__, "COM_DROP_DB failed"); /* purecov: inspected */
  }
  rc = simple_command(mysql, COM_DROP_DB, (uchar *)current_db,
                      (ulong)strlen(current_db), 0);
  myquery_r(rc);
  rc = simple_command(mysql, COM_CREATE_DB, (uchar *)current_db,
                      (ulong)strlen(current_db), 0);
  if (rc == 0 && mysql_errno(mysql) != ER_UNKNOWN_COM_ERROR) {
    myerror(nullptr);                                /* purecov: inspected */
    die(__FILE__, __LINE__, "COM_CREATE_DB failed"); /* purecov: inspected */
  }
  rc = simple_command(mysql, COM_CREATE_DB, (uchar *)current_db,
                      (ulong)strlen(current_db), 0);
  myquery_r(rc);
  rc = mysql_select_db(mysql, current_db);
  myquery(rc);
}

static void test_bug6096() {
  MYSQL_STMT *stmt;
  MYSQL_RES *query_result, *stmt_metadata;
  const char *stmt_text;
  MYSQL_BIND my_bind[12];
  MYSQL_FIELD *query_field_list, *stmt_field_list;
  ulong query_field_count, stmt_field_count;
  int rc;
  bool update_max_length = true;
  uint i;

  myheader("test_bug6096");

  stmt_text = "drop table if exists t1";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);

  mysql_query(mysql, "set sql_mode=''");
  stmt_text =
      "create table t1 (c_tinyint tinyint, c_smallint smallint, "
      " c_mediumint mediumint, c_int int, "
      " c_bigint bigint, c_float float, "
      " c_double double, c_varchar varchar(20), "
      " c_char char(20), c_time time, c_date date, "
      " c_datetime datetime)";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);
  stmt_text =
      "insert into t1  values (-100, -20000, 30000000, 4, 8, 1.0, "
      "2.0, 'abc', 'def', now(), now(), now())";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);

  stmt_text = "select * from t1";

  /* Run select in prepared and non-prepared mode and compare metadata */
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);
  query_result = mysql_store_result(mysql);
  query_field_list = mysql_fetch_fields(query_result);
  query_field_count = mysql_num_fields(query_result);

  stmt = mysql_stmt_init(mysql);
  rc = mysql_stmt_prepare(stmt, stmt_text, (ulong)strlen(stmt_text));
  check_execute(stmt, rc);
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);
  mysql_stmt_attr_set(stmt, STMT_ATTR_UPDATE_MAX_LENGTH,
                      (void *)&update_max_length);
  mysql_stmt_store_result(stmt);
  stmt_metadata = mysql_stmt_result_metadata(stmt);
  stmt_field_list = mysql_fetch_fields(stmt_metadata);
  stmt_field_count = mysql_num_fields(stmt_metadata);
  DIE_UNLESS(stmt_field_count == query_field_count);

  /* Print out and check the metadata */

  if (!opt_silent) {
    printf(" ------------------------------------------------------------\n");
    printf("             |                     Metadata \n");
    printf(" ------------------------------------------------------------\n");
    printf("             |         Query          |   Prepared statement \n");
    printf(" ------------------------------------------------------------\n");
    printf(" field name  |  length   | max_length |  length   |  max_length\n");
    printf(" ------------------------------------------------------------\n");

    for (i = 0; i < query_field_count; ++i) {
      MYSQL_FIELD *f1 = &query_field_list[i], *f2 = &stmt_field_list[i];
      printf(" %-11s | %9lu | %10lu | %9lu | %10lu \n", f1->name, f1->length,
             f1->max_length, f2->length, f2->max_length);
      DIE_UNLESS(f1->length == f2->length);
    }
    printf(
        " ---------------------------------------------------------------\n");
  }

  /* Bind and fetch the data */

  memset(my_bind, 0, sizeof(my_bind));
  for (i = 0; i < stmt_field_count; ++i) {
    my_bind[i].buffer_type = MYSQL_TYPE_STRING;
    my_bind[i].buffer_length = stmt_field_list[i].max_length + 1;
    my_bind[i].buffer = malloc(my_bind[i].buffer_length);
  }
  mysql_stmt_bind_result(stmt, my_bind);
  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);
  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == MYSQL_NO_DATA);

  /* Clean up */

  for (i = 0; i < stmt_field_count; ++i) free(my_bind[i].buffer);
  mysql_stmt_close(stmt);
  mysql_free_result(query_result);
  mysql_free_result(stmt_metadata);
  stmt_text = "drop table t1";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);
}

/*
  Test of basic checks that are performed in server for components
  of MYSQL_TIME parameters.
*/

static void test_datetime_ranges() {
  const char *stmt_text;
  int rc, i;
  MYSQL_STMT *stmt;
  MYSQL_BIND my_bind[6];
  MYSQL_TIME tm[6];

  myheader("test_datetime_ranges");

  stmt_text = "drop table if exists t1";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);

  stmt_text =
      "create table t1 (year datetime, month datetime, day datetime, "
      "hour datetime, min datetime, sec datetime)";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);

  stmt =
      mysql_simple_prepare(mysql, "INSERT INTO t1 VALUES (?, ?, ?, ?, ?, ?)");
  check_stmt(stmt);
  verify_param_count(stmt, 6);

  memset(my_bind, 0, sizeof(my_bind));
  for (i = 0; i < 6; i++) {
    my_bind[i].buffer_type = MYSQL_TYPE_DATETIME;
    my_bind[i].buffer = &tm[i];
  }
  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  tm[0].year = 2004;
  tm[0].month = 11;
  tm[0].day = 10;
  tm[0].hour = 12;
  tm[0].minute = 30;
  tm[0].second = 30;
  tm[0].second_part = 0;
  tm[0].neg = false;
  tm[0].time_type = MYSQL_TIMESTAMP_DATETIME;

  tm[5] = tm[4] = tm[3] = tm[2] = tm[1] = tm[0];
  tm[0].year = 10000;
  tm[1].month = 13;
  tm[2].day = 32;
  tm[3].hour = 24;
  tm[4].minute = 60;
  tm[5].second = 60;

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);
  /* behaviour changed by WL#5928 */
  my_process_warnings(mysql, mysql_get_server_version(mysql) < 50702 ? 12 : 6);

  verify_col_data("t1", "year", "0000-00-00 00:00:00");
  verify_col_data("t1", "month", "0000-00-00 00:00:00");
  verify_col_data("t1", "day", "0000-00-00 00:00:00");
  verify_col_data("t1", "hour", "0000-00-00 00:00:00");
  verify_col_data("t1", "min", "0000-00-00 00:00:00");
  verify_col_data("t1", "sec", "0000-00-00 00:00:00");

  mysql_stmt_close(stmt);

  stmt_text = "delete from t1";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);

  stmt = mysql_simple_prepare(mysql,
                              "INSERT INTO t1 (year, month, day) "
                              "VALUES (?, ?, ?)");
  check_stmt(stmt);
  verify_param_count(stmt, 3);

  /*
    We reuse contents of bind and tm arrays left from previous part of test.
  */
  for (i = 0; i < 3; i++) my_bind[i].buffer_type = MYSQL_TYPE_DATE;

  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);
  /* behaviour changed by WL#5928 */
  my_process_warnings(mysql, mysql_get_server_version(mysql) < 50702 ? 6 : 3);

  verify_col_data("t1", "year", "0000-00-00 00:00:00");
  verify_col_data("t1", "month", "0000-00-00 00:00:00");
  verify_col_data("t1", "day", "0000-00-00 00:00:00");

  mysql_stmt_close(stmt);

  stmt_text = "drop table t1";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);

  stmt_text =
      "create table t1 (day_ovfl time, day time, hour time, min time, sec "
      "time)";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);

  stmt = mysql_simple_prepare(mysql, "INSERT INTO t1 VALUES (?, ?, ?, ?, ?)");
  check_stmt(stmt);
  verify_param_count(stmt, 5);

  /*
    Again we reuse what we can from previous part of test.
  */
  for (i = 0; i < 5; i++) my_bind[i].buffer_type = MYSQL_TYPE_TIME;

  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  tm[0].year = 0;
  tm[0].month = 0;
  tm[0].day = 10;
  tm[0].hour = 12;
  tm[0].minute = 30;
  tm[0].second = 30;
  tm[0].second_part = 0;
  tm[0].neg = false;

  tm[4] = tm[3] = tm[2] = tm[1] = tm[0];
  tm[0].day = 35;
  tm[1].day = 34;
  tm[2].hour = 30;
  tm[3].minute = 60;
  tm[4].second = 60;

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);
  /* behaviour changed by WL#5928 */
  my_process_warnings(mysql, mysql_get_server_version(mysql) < 50702 ? 2 : 0);

  verify_col_data("t1", "day_ovfl", "838:59:59");
  verify_col_data("t1", "day", "828:30:30");
  verify_col_data("t1", "hour", "270:30:30");
  verify_col_data("t1", "min", "00:00:00");
  verify_col_data("t1", "sec", "00:00:00");

  mysql_stmt_close(stmt);

  stmt_text = "drop table t1";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);
}

static void test_bug4172() {
  MYSQL_STMT *stmt;
  MYSQL_BIND my_bind[3];
  const char *stmt_text;
  MYSQL_RES *res;
  MYSQL_ROW row;
  int rc;
  char f[100], d[100], e[100];
  ulong f_len, d_len, e_len;

  myheader("test_bug4172");

  mysql_query(mysql, "DROP TABLE IF EXISTS t1");
  mysql_query(mysql, "CREATE TABLE t1 (f float, d double, e decimal(10,4))");
  mysql_query(mysql,
              "INSERT INTO t1 VALUES (12345.1234, 123456.123456, "
              "123456.1234)");

  stmt = mysql_stmt_init(mysql);
  stmt_text = "SELECT f, d, e FROM t1";

  rc = mysql_stmt_prepare(stmt, stmt_text, (ulong)strlen(stmt_text));
  check_execute(stmt, rc);
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  memset(my_bind, 0, sizeof(my_bind));
  my_bind[0].buffer_type = MYSQL_TYPE_STRING;
  my_bind[0].buffer = f;
  my_bind[0].buffer_length = (ulong)sizeof(f);
  my_bind[0].length = &f_len;
  my_bind[1].buffer_type = MYSQL_TYPE_STRING;
  my_bind[1].buffer = d;
  my_bind[1].buffer_length = (ulong)sizeof(d);
  my_bind[1].length = &d_len;
  my_bind[2].buffer_type = MYSQL_TYPE_STRING;
  my_bind[2].buffer = e;
  my_bind[2].buffer_length = (ulong)sizeof(e);
  my_bind[2].length = &e_len;

  mysql_stmt_bind_result(stmt, my_bind);

  mysql_stmt_store_result(stmt);
  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);
  res = mysql_store_result(mysql);
  row = mysql_fetch_row(res);

  if (!opt_silent) {
    printf("Binary protocol: float=%s, double=%s, decimal(10,4)=%s\n", f, d, e);
    printf("Text protocol:   float=%s, double=%s, decimal(10,4)=%s\n", row[0],
           row[1], row[2]);
  }
  DIE_UNLESS(!strcmp(f, row[0]) && !strcmp(d, row[1]) && !strcmp(e, row[2]));

  mysql_free_result(res);
  mysql_stmt_close(stmt);
}

static void test_conversion() {
  MYSQL_STMT *stmt;
  const char *stmt_text;
  int rc;
  MYSQL_BIND my_bind[1];
  char buff[4];
  ulong length;

  myheader("test_conversion");

  stmt_text = "DROP TABLE IF EXISTS t1";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);
  stmt_text = "CREATE TABLE t1 (a TEXT) DEFAULT CHARSET latin1";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);
  stmt_text =
      "SET character_set_connection=utf8, character_set_client=utf8, "
      " character_set_results=latin1";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);

  stmt = mysql_stmt_init(mysql);

  stmt_text = "INSERT INTO t1 (a) VALUES (?)";
  rc = mysql_stmt_prepare(stmt, stmt_text, (ulong)strlen(stmt_text));
  check_execute(stmt, rc);

  memset(my_bind, 0, sizeof(my_bind));
  my_bind[0].buffer = buff;
  my_bind[0].length = &length;
  my_bind[0].buffer_type = MYSQL_TYPE_STRING;

  mysql_stmt_bind_param(stmt, my_bind);

  buff[0] = (uchar)0xC3;
  buff[1] = (uchar)0xA0;
  length = 2;

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  stmt_text = "SELECT a FROM t1";
  rc = mysql_stmt_prepare(stmt, stmt_text, (ulong)strlen(stmt_text));
  check_execute(stmt, rc);
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  my_bind[0].buffer_length = (ulong)sizeof(buff);
  mysql_stmt_bind_result(stmt, my_bind);

  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == 0);
  DIE_UNLESS(length == 1);
  DIE_UNLESS((uchar)buff[0] == 0xE0);
  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == MYSQL_NO_DATA);

  mysql_stmt_close(stmt);
  stmt_text = "DROP TABLE t1";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);
  stmt_text = "SET NAMES DEFAULT";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);
}

static void test_rewind(void) {
  MYSQL_STMT *stmt;
  MYSQL_BIND my_bind;
  int rc = 0;
  const char *stmt_text;
  ulong length = 4;
  long unsigned int Data = 0;
  bool isnull = false;

  myheader("test_rewind");

  stmt_text = "CREATE TABLE t1 (a int)";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);
  stmt_text = "INSERT INTO t1 VALUES(2),(3),(4)";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);

  stmt = mysql_stmt_init(mysql);

  stmt_text = "SELECT * FROM t1";
  rc = mysql_stmt_prepare(stmt, stmt_text, (ulong)strlen(stmt_text));
  check_execute(stmt, rc);

  memset(&my_bind, 0, sizeof(MYSQL_BIND));
  my_bind.buffer_type = MYSQL_TYPE_LONG;
  my_bind.buffer = (void *)&Data; /* this buffer won't be altered */
  my_bind.length = &length;
  my_bind.is_null = &isnull;

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_store_result(stmt);
  DIE_UNLESS(rc == 0);

  rc = mysql_stmt_bind_result(stmt, &my_bind);
  DIE_UNLESS(rc == 0);

  /* retreive all result sets till we are at the end */
  while (!mysql_stmt_fetch(stmt))
    if (!opt_silent) printf("fetched result:%ld\n", Data);

  DIE_UNLESS(rc != MYSQL_NO_DATA);

  /* seek to the first row */
  mysql_stmt_data_seek(stmt, 0);

  /* now we should be able to fetch the results again */
  /* but mysql_stmt_fetch returns MYSQL_NO_DATA */
  while (!(rc = mysql_stmt_fetch(stmt)))
    if (!opt_silent) printf("fetched result after seek:%ld\n", Data);

  DIE_UNLESS(rc == MYSQL_NO_DATA);

  stmt_text = "DROP TABLE t1";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);
  rc = mysql_stmt_free_result(stmt);
  rc = mysql_stmt_close(stmt);
}

static void test_truncation() {
  MYSQL_STMT *stmt;
  const char *stmt_text;
  int rc;
  uint bind_count;
  MYSQL_BIND *bind_array, *my_bind;

  myheader("test_truncation");

  /* Prepare the test table */
  rc = mysql_query(mysql, "drop table if exists t1");
  myquery(rc);

  stmt_text =
      "create table t1 ("
      "i8 tinyint, ui8 tinyint unsigned, "
      "i16 smallint, i16_1 smallint, "
      "ui16 smallint unsigned, i32 int, i32_1 int, "
      "d double, d_1 double, ch char(30), ch_1 char(30), "
      "tx text, tx_1 text, ch_2 char(30) "
      ")";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);

  {
    const char insert_text[] =
        "insert into t1 VALUES ("
        "-10, "                             /* i8 */
        "200, "                             /* ui8 */
        "32000, "                           /* i16 */
        "-32767, "                          /* i16_1 */
        "64000, "                           /* ui16 */
        "1073741824, "                      /* i32 */
        "1073741825, "                      /* i32_1 */
        "123.456, "                         /* d */
        "-12345678910, "                    /* d_1 */
        "'111111111111111111111111111111'," /* ch */
        "'abcdef', "                        /* ch_1 */
        "'12345 	      ', "          /* tx */
        "'12345.67 	      ', "          /* tx_1 */
        "'12345.67abc'"                     /* ch_2 */
        ")";
    rc = mysql_real_query(mysql, insert_text, (ulong)strlen(insert_text));
    myquery(rc);
  }

  stmt_text =
      "select i8 c1, i8 c2, ui8 c3, i16_1 c4, ui16 c5, "
      "       i16 c6, ui16 c7, i32 c8, i32_1 c9, i32_1 c10, "
      "       d c11, d_1 c12, d_1 c13, ch c14, ch_1 c15, tx c16, "
      "       tx_1 c17, ch_2 c18 "
      "from t1";

  stmt = mysql_stmt_init(mysql);
  rc = mysql_stmt_prepare(stmt, stmt_text, (ulong)strlen(stmt_text));
  check_execute(stmt, rc);
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);
  bind_count = (uint)mysql_stmt_field_count(stmt);

  /*************** Fill in the bind structure and bind it **************/
  bind_array = (MYSQL_BIND *)malloc(sizeof(MYSQL_BIND) * bind_count);
  memset(bind_array, 0, sizeof(MYSQL_BIND) * bind_count);
  for (my_bind = bind_array; my_bind < bind_array + bind_count; my_bind++)
    my_bind->error = &my_bind->error_value;
  my_bind = bind_array;

  my_bind->buffer = malloc(sizeof(uint8));
  my_bind->buffer_type = MYSQL_TYPE_TINY;
  my_bind->is_unsigned = true;

  DIE_UNLESS(my_bind++ < bind_array + bind_count);
  my_bind->buffer = malloc(sizeof(uint32));
  my_bind->buffer_type = MYSQL_TYPE_LONG;
  my_bind->is_unsigned = true;

  DIE_UNLESS(my_bind++ < bind_array + bind_count);
  my_bind->buffer = malloc(sizeof(int8));
  my_bind->buffer_type = MYSQL_TYPE_TINY;

  DIE_UNLESS(my_bind++ < bind_array + bind_count);
  my_bind->buffer = malloc(sizeof(uint16));
  my_bind->buffer_type = MYSQL_TYPE_SHORT;
  my_bind->is_unsigned = true;

  DIE_UNLESS(my_bind++ < bind_array + bind_count);
  my_bind->buffer = malloc(sizeof(int16));
  my_bind->buffer_type = MYSQL_TYPE_SHORT;

  DIE_UNLESS(my_bind++ < bind_array + bind_count);
  my_bind->buffer = malloc(sizeof(uint16));
  my_bind->buffer_type = MYSQL_TYPE_SHORT;
  my_bind->is_unsigned = true;

  DIE_UNLESS(my_bind++ < bind_array + bind_count);
  my_bind->buffer = malloc(sizeof(int8));
  my_bind->buffer_type = MYSQL_TYPE_TINY;
  my_bind->is_unsigned = true;

  DIE_UNLESS(my_bind++ < bind_array + bind_count);
  my_bind->buffer = malloc(sizeof(float));
  my_bind->buffer_type = MYSQL_TYPE_FLOAT;

  DIE_UNLESS(my_bind++ < bind_array + bind_count);
  my_bind->buffer = malloc(sizeof(float));
  my_bind->buffer_type = MYSQL_TYPE_FLOAT;

  DIE_UNLESS(my_bind++ < bind_array + bind_count);
  my_bind->buffer = malloc(sizeof(double));
  my_bind->buffer_type = MYSQL_TYPE_DOUBLE;

  DIE_UNLESS(my_bind++ < bind_array + bind_count);
  my_bind->buffer = malloc(sizeof(longlong));
  my_bind->buffer_type = MYSQL_TYPE_LONGLONG;

  DIE_UNLESS(my_bind++ < bind_array + bind_count);
  my_bind->buffer = malloc(sizeof(ulonglong));
  my_bind->buffer_type = MYSQL_TYPE_LONGLONG;
  my_bind->is_unsigned = true;

  DIE_UNLESS(my_bind++ < bind_array + bind_count);
  my_bind->buffer = malloc(sizeof(longlong));
  my_bind->buffer_type = MYSQL_TYPE_LONGLONG;

  DIE_UNLESS(my_bind++ < bind_array + bind_count);
  my_bind->buffer = malloc(sizeof(longlong));
  my_bind->buffer_type = MYSQL_TYPE_LONGLONG;

  DIE_UNLESS(my_bind++ < bind_array + bind_count);
  my_bind->buffer = malloc(sizeof(longlong));
  my_bind->buffer_type = MYSQL_TYPE_LONGLONG;

  DIE_UNLESS(my_bind++ < bind_array + bind_count);
  my_bind->buffer = malloc(sizeof(longlong));
  my_bind->buffer_type = MYSQL_TYPE_LONGLONG;

  DIE_UNLESS(my_bind++ < bind_array + bind_count);
  my_bind->buffer = malloc(sizeof(double));
  my_bind->buffer_type = MYSQL_TYPE_DOUBLE;

  DIE_UNLESS(my_bind++ < bind_array + bind_count);
  my_bind->buffer = malloc(sizeof(double));
  my_bind->buffer_type = MYSQL_TYPE_DOUBLE;

  rc = mysql_stmt_bind_result(stmt, bind_array);
  check_execute(stmt, rc);
  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == MYSQL_DATA_TRUNCATED);

  /*************** Verify truncation results ***************************/
  my_bind = bind_array;

  /* signed tiny -> tiny */
  DIE_UNLESS(*my_bind->error && *(int8 *)my_bind->buffer == -10);

  /* signed tiny -> uint32 */
  DIE_UNLESS(my_bind++ < bind_array + bind_count);
  DIE_UNLESS(*my_bind->error && *(int32 *)my_bind->buffer == -10);

  /* unsigned tiny -> tiny */
  DIE_UNLESS(my_bind++ < bind_array + bind_count);
  DIE_UNLESS(*my_bind->error && *(uint8 *)my_bind->buffer == 200);

  /* short -> ushort */
  DIE_UNLESS(my_bind++ < bind_array + bind_count);
  DIE_UNLESS(*my_bind->error && *(int16 *)my_bind->buffer == -32767);

  /* ushort -> short */
  DIE_UNLESS(my_bind++ < bind_array + bind_count);
  DIE_UNLESS(*my_bind->error && *(uint16 *)my_bind->buffer == 64000);

  /* short -> ushort (no truncation, data is in the range of target type) */
  DIE_UNLESS(my_bind++ < bind_array + bind_count);
  DIE_UNLESS(!*my_bind->error && *(uint16 *)my_bind->buffer == 32000);

  /* ushort -> utiny */
  DIE_UNLESS(my_bind++ < bind_array + bind_count);
  DIE_UNLESS(*my_bind->error && *(int8 *)my_bind->buffer == 0);

  /* int -> float: no truncation, the number is a power of two */
  DIE_UNLESS(my_bind++ < bind_array + bind_count);
  DIE_UNLESS(!*my_bind->error && *(float *)my_bind->buffer == 1073741824);

  /* int -> float: truncation, not enough bits in float */
  DIE_UNLESS(my_bind++ < bind_array + bind_count);
  DIE_UNLESS(*my_bind->error);

  /* int -> double: no truncation */
  DIE_UNLESS(my_bind++ < bind_array + bind_count);
  DIE_UNLESS(!*my_bind->error && *(double *)my_bind->buffer == 1073741825);

  /* double -> longlong: fractional part is lost */
  DIE_UNLESS(my_bind++ < bind_array + bind_count);

  /* double -> ulonglong, negative fp number to unsigned integer */
  DIE_UNLESS(my_bind++ < bind_array + bind_count);
  /* Value in the buffer is not defined: don't test it */
  DIE_UNLESS(*my_bind->error);

  /* double -> longlong, negative fp number to signed integer: no loss */
  DIE_UNLESS(my_bind++ < bind_array + bind_count);
  DIE_UNLESS(!*my_bind->error &&
             *(longlong *)my_bind->buffer == -12345678910LL);

  /* big numeric string -> number */
  DIE_UNLESS(my_bind++ < bind_array + bind_count);
  DIE_UNLESS(*my_bind->error);

  /* junk string -> number */
  DIE_UNLESS(my_bind++ < bind_array + bind_count);
  DIE_UNLESS(*my_bind->error && *(longlong *)my_bind->buffer == 0);

  /* string with trailing spaces -> number */
  DIE_UNLESS(my_bind++ < bind_array + bind_count);
  DIE_UNLESS(!*my_bind->error && *(longlong *)my_bind->buffer == 12345);

  /* string with trailing spaces -> double */
  DIE_UNLESS(my_bind++ < bind_array + bind_count);
  DIE_UNLESS(!*my_bind->error && *(double *)my_bind->buffer == 12345.67);

  /* string with trailing junk -> double */
  DIE_UNLESS(my_bind++ < bind_array + bind_count);
  /*
    XXX: There must be a truncation error: but it's not the way the server
    behaves, so let's leave it for now.
  */
  DIE_UNLESS(*(double *)my_bind->buffer == 12345.67);
  /*
    TODO: string -> double,  double -> time, double -> string (truncation
          errors are not supported here yet)
          longlong -> time/date/datetime
          date -> time, date -> timestamp, date -> number
          time -> string, time -> date, time -> timestamp,
          number -> date string -> date
  */
  /*************** Cleanup *********************************************/

  mysql_stmt_close(stmt);

  for (my_bind = bind_array; my_bind < bind_array + bind_count; my_bind++)
    free(my_bind->buffer);
  free(bind_array);

  rc = mysql_query(mysql, "drop table t1");
  myquery(rc);
}

static void test_truncation_option() {
  MYSQL_STMT *stmt;
  const char *stmt_text;
  int rc;
  uint8 buf;
  bool option = false;
  bool error;
  MYSQL_BIND my_bind;

  myheader("test_truncation_option");

  /* Prepare the test table */
  stmt_text = "select -1";

  stmt = mysql_stmt_init(mysql);
  rc = mysql_stmt_prepare(stmt, stmt_text, (ulong)strlen(stmt_text));
  check_execute(stmt, rc);
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  memset(&my_bind, 0, sizeof(my_bind));

  my_bind.buffer = (void *)&buf;
  my_bind.buffer_type = MYSQL_TYPE_TINY;
  my_bind.is_unsigned = true;
  my_bind.error = &error;

  rc = mysql_stmt_bind_result(stmt, &my_bind);
  check_execute(stmt, rc);
  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == MYSQL_DATA_TRUNCATED);
  DIE_UNLESS(error);
  rc = mysql_options(mysql, MYSQL_REPORT_DATA_TRUNCATION, (char *)&option);
  myquery(rc);
  /* need to rebind for the new setting to take effect */
  rc = mysql_stmt_bind_result(stmt, &my_bind);
  check_execute(stmt, rc);
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);
  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);
  /* The only change is rc - error pointers are still filled in */
  DIE_UNLESS(error == 1);
  /* restore back the defaults */
  option = true;
  mysql_options(mysql, MYSQL_REPORT_DATA_TRUNCATION, (char *)&option);

  mysql_stmt_close(stmt);
}

/* Bug#6761 - mysql_list_fields doesn't work */

static void test_bug6761(void) {
  const char *stmt_text;
  MYSQL_RES *res;
  int rc;
  myheader("test_bug6761");

  stmt_text = "CREATE TABLE t1 (a int, b char(255), c decimal)";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);

  res = mysql_list_fields(mysql, "t1", "%");
  DIE_UNLESS(res && mysql_num_fields(res) == 3);
  mysql_free_result(res);

  stmt_text = "DROP TABLE t1";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);
}

/* Bug#8330 - mysql_stmt_execute crashes (libmysql) */

static void test_bug8330() {
  const char *stmt_text;
  MYSQL_STMT *stmt[2];
  int i, rc;
  const char *query = "select a,b from t1 where a=?";
  MYSQL_BIND my_bind[2];
  long lval[2];

  myheader("test_bug8330");

  stmt_text = "drop table if exists t1";
  /* in case some previos test failed */
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);
  stmt_text = "create table t1 (a int, b int)";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);

  memset(my_bind, 0, sizeof(my_bind));
  memset(lval, 0, sizeof(lval));
  for (i = 0; i < 2; i++) {
    stmt[i] = mysql_stmt_init(mysql);
    rc = mysql_stmt_prepare(stmt[i], query, (ulong)strlen(query));
    check_execute(stmt[i], rc);

    my_bind[i].buffer_type = MYSQL_TYPE_LONG;
    my_bind[i].buffer = (void *)&lval[i];
    my_bind[i].is_null = nullptr;
    mysql_stmt_bind_param(stmt[i], &my_bind[i]);
  }

  rc = mysql_stmt_execute(stmt[0]);
  check_execute(stmt[0], rc);

  rc = mysql_stmt_execute(stmt[1]);
  DIE_UNLESS(rc && mysql_stmt_errno(stmt[1]) == CR_COMMANDS_OUT_OF_SYNC);
  rc = mysql_stmt_execute(stmt[0]);
  check_execute(stmt[0], rc);

  mysql_stmt_close(stmt[0]);
  mysql_stmt_close(stmt[1]);

  stmt_text = "drop table t1";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);
}

/* Bug#7990 - mysql_stmt_close doesn't reset mysql->net.last_error */

static void test_bug7990() {
  MYSQL_STMT *stmt;
  int rc;
  myheader("test_bug7990");

  stmt = mysql_stmt_init(mysql);
  rc = mysql_stmt_prepare(stmt, "foo", 3);
  /*
    XXX: the fact that we store errno both in STMT and in
    MYSQL is not documented and is subject to change in 5.0
  */
  DIE_UNLESS(rc && mysql_stmt_errno(stmt) && mysql_errno(mysql));
  mysql_stmt_close(stmt);
  DIE_UNLESS(!mysql_errno(mysql));
}

/*
  Bug #15518 - Reusing a stmt that has failed during prepare
  does not clear error
*/

static void test_bug15518() {
  MYSQL_STMT *stmt;
  MYSQL *mysql1;
  int rc;
  myheader("test_bug15518");

  mysql1 = mysql_client_init(nullptr);

  if (!mysql_real_connect(mysql1, opt_host, opt_user, opt_password,
                          opt_db ? opt_db : "test", opt_port, opt_unix_socket,
                          CLIENT_MULTI_STATEMENTS)) {
    fprintf(stderr, "Failed to connect to the database\n");
    DIE_UNLESS(0);
  }

  stmt = mysql_stmt_init(mysql1);

  /*
    The prepare of foo should fail with errno 1064 since
    it's not a valid query
  */
  rc = mysql_stmt_prepare(stmt, "foo", 3);
  if (!opt_silent)
    fprintf(stdout, "rc: %d, mysql_stmt_errno: %d, mysql_errno: %d\n", rc,
            mysql_stmt_errno(stmt), mysql_errno(mysql1));
  DIE_UNLESS(rc && mysql_stmt_errno(stmt) && mysql_errno(mysql1));

  /*
    Use the same stmt and reprepare with another query that
    suceeds
  */
  rc = mysql_stmt_prepare(stmt, "SHOW STATUS", 12);
  if (!opt_silent)
    fprintf(stdout, "rc: %d, mysql_stmt_errno: %d, mysql_errno: %d\n", rc,
            mysql_stmt_errno(stmt), mysql_errno(mysql1));
  DIE_UNLESS(!rc || mysql_stmt_errno(stmt) || mysql_errno(mysql1));

  mysql_stmt_close(stmt);
  DIE_UNLESS(!mysql_errno(mysql1));

  /*
    part2, when connection to server has been closed
    after first prepare
  */
  stmt = mysql_stmt_init(mysql1);
  rc = mysql_stmt_prepare(stmt, "foo", 3);
  if (!opt_silent)
    fprintf(stdout, "rc: %d, mysql_stmt_errno: %d, mysql_errno: %d\n", rc,
            mysql_stmt_errno(stmt), mysql_errno(mysql1));
  DIE_UNLESS(rc && mysql_stmt_errno(stmt) && mysql_errno(mysql1));

  /* Close connection to server */
  mysql_close(mysql1);

  /*
    Use the same stmt and reprepare with another query that
    suceeds. The prepare should fail with error 2013 since
    connection to server has been closed.
  */
  rc = mysql_stmt_prepare(stmt, "SHOW STATUS", 12);
  if (!opt_silent)
    fprintf(stdout, "rc: %d, mysql_stmt_errno: %d\n", rc,
            mysql_stmt_errno(stmt));
  DIE_UNLESS(rc && mysql_stmt_errno(stmt));

  mysql_stmt_close(stmt);
}

static void disable_query_logs() {
  int rc;
  rc = mysql_query(mysql, "set @@global.general_log=off");
  myquery(rc);
  rc = mysql_query(mysql, "set @@global.slow_query_log=off");
  myquery(rc);
}

static void enable_query_logs(int truncate) {
  int rc;

  rc = mysql_query(mysql, "set @save_global_general_log=@@global.general_log");
  myquery(rc);

  rc = mysql_query(mysql,
                   "set @save_global_slow_query_log=@@global.slow_query_log");
  myquery(rc);

  rc = mysql_query(mysql, "set @@global.general_log=on");
  myquery(rc);

  rc = mysql_query(mysql, "set @@global.slow_query_log=on");
  myquery(rc);

  if (truncate) {
    rc = mysql_query(mysql, "truncate mysql.general_log");
    myquery(rc);

    rc = mysql_query(mysql, "truncate mysql.slow_log");
    myquery(rc);
  }
}

static void restore_query_logs() {
  int rc;
  rc = mysql_query(mysql, "set @@global.general_log=@save_global_general_log");
  myquery(rc);

  rc = mysql_query(mysql,
                   "set @@global.slow_query_log=@save_global_slow_query_log");
  myquery(rc);
}

static void test_view_sp_list_fields() {
  int rc;
  MYSQL_RES *res;

  myheader("test_view_sp_list_fields");

  rc = mysql_query(mysql, "DROP FUNCTION IF EXISTS f1");
  myquery(rc);
  rc = mysql_query(mysql, "DROP TABLE IF EXISTS v1, t1, t2");
  myquery(rc);
  rc = mysql_query(mysql, "DROP VIEW IF EXISTS v1, t1, t2");
  myquery(rc);
  rc = mysql_query(mysql, "create function f1 () returns int return 5");
  myquery(rc);
  rc = mysql_query(mysql, "create table t1 (s1 char,s2 char)");
  myquery(rc);
  rc = mysql_query(mysql, "create table t2 (s1 int);");
  myquery(rc);
  rc = mysql_query(mysql,
                   "create view v1 as select s2,sum(s1) - \
count(s2) as vx from t1 group by s2 having sum(s1) - count(s2) < (select f1() \
from t2);");
  myquery(rc);
  res = mysql_list_fields(mysql, "v1", NullS);
  DIE_UNLESS(res != nullptr && mysql_num_fields(res) != 0);
  rc = mysql_query(mysql, "DROP FUNCTION f1");
  myquery(rc);
  rc = mysql_query(mysql, "DROP VIEW v1");
  myquery(rc);
  rc = mysql_query(mysql, "DROP TABLE t1, t2");
  mysql_free_result(res);
  myquery(rc);
}

/*
 Test mysql_real_escape_string_quote() with gbk charset

 The important part is that 0x27 (') is the second-byte in a invalid
 two-byte GBK character here. But 0xbf5c is a valid GBK character, so
 it needs to be escaped as 0x5cbf27
*/
#define TEST_BUG8378_IN "\xef\xbb\xbf\x27\xbf\x10"
#define TEST_BUG8378_OUT "\xef\xbb\x5c\xbf\x5c\x27\x5c\xbf\x10"

static void test_bug8378() {
  MYSQL *lmysql;
  char out[9]; /* strlen(TEST_BUG8378)*2+1 */
  char buf[256];
  int len, rc;

  myheader("test_bug8378");

  if (!opt_silent) fprintf(stdout, "\n Establishing a test connection ...");
  if (!(lmysql = mysql_client_init(nullptr))) {
    myerror("mysql_client_init() failed");
    exit(1);
  }
  if (mysql_options(lmysql, MYSQL_SET_CHARSET_NAME, "gbk")) {
    myerror("mysql_options() failed");
    exit(1);
  }
  if (!(mysql_real_connect(lmysql, opt_host, opt_user, opt_password, current_db,
                           opt_port, opt_unix_socket, 0))) {
    myerror("connection failed");
    exit(1);
  }
  if (!opt_silent) fprintf(stdout, "OK");

  rc = mysql_query(lmysql, "SET SQL_MODE=''");
  myquery(rc);

  len = mysql_real_escape_string_quote(lmysql, out, TEST_BUG8378_IN, 4, '\'');

  /* No escaping should have actually happened. */
  DIE_UNLESS(memcmp(out, TEST_BUG8378_OUT, len) == 0);

  sprintf(buf, "SELECT '%s'", out);

  rc = mysql_real_query(lmysql, buf, (ulong)strlen(buf));
  myquery(rc);

  mysql_close(lmysql);
}

static void test_bug8722() {
  MYSQL_STMT *stmt;
  int rc;
  const char *stmt_text;

  myheader("test_bug8722");
  /* Prepare test data */
  stmt_text = "drop table if exists t1, v1";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);
  stmt_text =
      "CREATE TABLE t1 (c1 varchar(10), c2 varchar(10), c3 varchar(10),"
      " c4 varchar(10), c5 varchar(10), c6 varchar(10),"
      " c7 varchar(10), c8 varchar(10), c9 varchar(10),"
      "c10 varchar(10))";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);
  stmt_text = "INSERT INTO t1 VALUES (1,2,3,4,5,6,7,8,9,10)";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);
  stmt_text = "CREATE VIEW v1 AS SELECT * FROM t1";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);
  /* Note: if you uncomment following block everything works fine */
  /*
    rc= mysql_query(mysql, "sellect * from v1");
    myquery(rc);
    mysql_free_result(mysql_store_result(mysql));
  */

  stmt = mysql_stmt_init(mysql);
  stmt_text = "select * from v1";
  rc = mysql_stmt_prepare(stmt, stmt_text, (ulong)strlen(stmt_text));
  check_execute(stmt, rc);
  mysql_stmt_close(stmt);
  stmt_text = "drop table if exists t1, v1";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);
}

static MYSQL_STMT *open_cursor(const char *query) {
  int rc;
  const ulong type = (ulong)CURSOR_TYPE_READ_ONLY;

  MYSQL_STMT *stmt = mysql_stmt_init(mysql);
  rc = mysql_stmt_prepare(stmt, query, (ulong)strlen(query));
  check_execute(stmt, rc);

  mysql_stmt_attr_set(stmt, STMT_ATTR_CURSOR_TYPE, &type);
  return stmt;
}

static void test_bug8880() {
  MYSQL_STMT *stmt_list[2], **stmt;
  MYSQL_STMT **stmt_list_end = (MYSQL_STMT **)stmt_list + 2;
  int rc;

  myheader("test_bug8880");

  mysql_query(mysql, "drop table if exists t1");
  mysql_query(mysql, "create table t1 (a int not null primary key, b int)");
  rc = mysql_query(mysql, "insert into t1 values (1,1)");
  myquery(rc); /* one check is enough */
  /*
    when inserting 2 rows everything works well
    mysql_query(mysql, "INSERT INTO t1 VALUES (1,1),(2,2)");
  */
  for (stmt = stmt_list; stmt < stmt_list_end; stmt++)
    *stmt = open_cursor("select a from t1");
  for (stmt = stmt_list; stmt < stmt_list_end; stmt++) {
    rc = mysql_stmt_execute(*stmt);
    check_execute(*stmt, rc);
  }
  for (stmt = stmt_list; stmt < stmt_list_end; stmt++) mysql_stmt_close(*stmt);
}

static void test_bug9159() {
  MYSQL_STMT *stmt;
  int rc;
  const char *stmt_text = "select a, b from t1";
  const unsigned long type = CURSOR_TYPE_READ_ONLY;

  myheader("test_bug9159");

  mysql_query(mysql, "drop table if exists t1");
  mysql_query(mysql, "create table t1 (a int not null primary key, b int)");
  rc = mysql_query(mysql, "insert into t1 values (1,1)");
  myquery(rc);

  stmt = mysql_stmt_init(mysql);
  mysql_stmt_prepare(stmt, stmt_text, (ulong)strlen(stmt_text));
  mysql_stmt_attr_set(stmt, STMT_ATTR_CURSOR_TYPE, (const void *)&type);

  mysql_stmt_execute(stmt);
  mysql_stmt_close(stmt);
  rc = mysql_query(mysql, "drop table if exists t1");
  myquery(rc);
}

/* Crash when opening a cursor to a query with DISTICNT and no key */

static void test_bug9520() {
  MYSQL_STMT *stmt;
  MYSQL_BIND my_bind[1];
  char a[6];
  ulong a_len;
  int rc, row_count = 0;

  myheader("test_bug9520");

  mysql_query(mysql, "drop table if exists t1");
  mysql_query(mysql,
              "create table t1 (a char(5), b char(5), c char(5),"
              " primary key (a, b, c))");
  rc = mysql_query(mysql,
                   "insert into t1 values ('x', 'y', 'z'), "
                   " ('a', 'b', 'c'), ('k', 'l', 'm')");
  myquery(rc);

  stmt = open_cursor("select distinct b from t1");

  /*
    Not crashes with:
    stmt= open_cursor("select distinct a from t1");
  */

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  memset(my_bind, 0, sizeof(my_bind));
  my_bind[0].buffer_type = MYSQL_TYPE_STRING;
  my_bind[0].buffer = (char *)a;
  my_bind[0].buffer_length = (ulong)sizeof(a);
  my_bind[0].length = &a_len;

  mysql_stmt_bind_result(stmt, my_bind);

  while (!(rc = mysql_stmt_fetch(stmt))) row_count++;

  DIE_UNLESS(rc == MYSQL_NO_DATA);

  if (!opt_silent) printf("Fetched %d rows\n", row_count);
  DBUG_ASSERT(row_count == 3);

  mysql_stmt_close(stmt);

  rc = mysql_query(mysql, "drop table t1");
  myquery(rc);
}

/*
  We can't have more than one cursor open for a prepared statement.
  Test re-executions of a PS with cursor; mysql_stmt_reset must close
  the cursor attached to the statement, if there is one.
*/

static void test_bug9478() {
  MYSQL_STMT *stmt;
  MYSQL_BIND my_bind[1];
  char a[6];
  ulong a_len;
  int rc, i;
  DBUG_TRACE;

  myheader("test_bug9478");

  mysql_query(mysql, "drop table if exists t1");
  mysql_query(mysql,
              "create table t1 (id integer not null primary key, "
              " name varchar(20) not null)");
  rc = mysql_query(mysql,
                   "insert into t1 (id, name) values "
                   " (1, 'aaa'), (2, 'bbb'), (3, 'ccc')");
  myquery(rc);

  stmt = open_cursor("select name from t1 where id=2");

  memset(my_bind, 0, sizeof(my_bind));
  memset(a, 0, sizeof(a));
  my_bind[0].buffer_type = MYSQL_TYPE_STRING;
  my_bind[0].buffer = (char *)a;
  my_bind[0].buffer_length = (ulong)sizeof(a);
  my_bind[0].length = &a_len;
  mysql_stmt_bind_result(stmt, my_bind);

  for (i = 0; i < 5; i++) {
    rc = mysql_stmt_execute(stmt);
    check_execute(stmt, rc);
    rc = mysql_stmt_fetch(stmt);
    check_execute(stmt, rc);
    if (!opt_silent && i == 0) printf("Fetched row: %s\n", a);

    /*
      The query above is a one-row result set. Therefore, there is no
      cursor associated with it, as the server won't bother with opening
      a cursor for a one-row result set. The first row was read from the
      server in the fetch above. But there is eof packet pending in the
      network. mysql_stmt_execute will flush the packet and successfully
      execute the statement.
    */

    rc = mysql_stmt_execute(stmt);
    check_execute(stmt, rc);

    rc = mysql_stmt_fetch(stmt);
    check_execute(stmt, rc);
    if (!opt_silent && i == 0) printf("Fetched row: %s\n", a);
    rc = mysql_stmt_fetch(stmt);
    DIE_UNLESS(rc == MYSQL_NO_DATA);

    {
      uchar buff[8];
      memset(buff, 0, sizeof(buff));
      /* Fill in the fetch packet */
      int4store(buff, stmt->stmt_id);
      buff[4] = 1; /* prefetch rows */
      rc = ((*mysql->methods->advanced_command)(mysql, COM_STMT_FETCH,
                                                (uchar *)buff, sizeof(buff),
                                                nullptr, 0, true, nullptr) ||
            (*mysql->methods->read_query_result)(mysql));
      DIE_UNLESS(rc);
      if (!opt_silent && i == 0)
        printf("Got error (as expected): %s\n", mysql_error(mysql));
    }

    rc = mysql_stmt_execute(stmt);
    check_execute(stmt, rc);

    rc = mysql_stmt_fetch(stmt);
    check_execute(stmt, rc);
    if (!opt_silent && i == 0) printf("Fetched row: %s\n", a);

    rc = mysql_stmt_reset(stmt);
    check_execute(stmt, rc);
    rc = mysql_stmt_fetch(stmt);
    DIE_UNLESS(rc && mysql_stmt_errno(stmt));
    if (!opt_silent && i == 0)
      printf("Got error (as expected): %s\n", mysql_stmt_error(stmt));
  }
  rc = mysql_stmt_close(stmt);
  DIE_UNLESS(rc == 0);

  /* Test the case with a server side cursor */
  stmt = open_cursor("select name from t1");

  mysql_stmt_bind_result(stmt, my_bind);

  for (i = 0; i < 5; i++) {
    DBUG_PRINT("loop", ("i: %d", i));
    rc = mysql_stmt_execute(stmt);
    check_execute(stmt, rc);
    rc = mysql_stmt_fetch(stmt);
    check_execute(stmt, rc);
    if (!opt_silent && i == 0) printf("Fetched row: %s\n", a);
    rc = mysql_stmt_execute(stmt);
    check_execute(stmt, rc);

    while (!(rc = mysql_stmt_fetch(stmt))) {
      if (!opt_silent && i == 0) printf("Fetched row: %s\n", a);
    }
    DIE_UNLESS(rc == MYSQL_NO_DATA);

    rc = mysql_stmt_execute(stmt);
    check_execute(stmt, rc);

    rc = mysql_stmt_fetch(stmt);
    check_execute(stmt, rc);
    if (!opt_silent && i == 0) printf("Fetched row: %s\n", a);

    rc = mysql_stmt_reset(stmt);
    check_execute(stmt, rc);
    rc = mysql_stmt_fetch(stmt);
    DIE_UNLESS(rc && mysql_stmt_errno(stmt));
    if (!opt_silent && i == 0)
      printf("Got error (as expected): %s\n", mysql_stmt_error(stmt));
  }

  rc = mysql_stmt_close(stmt);
  DIE_UNLESS(rc == 0);

  rc = mysql_query(mysql, "drop table t1");
  myquery(rc);
}

/*
  Error message is returned for unsupported features.
  Test also cursors with non-default PREFETCH_ROWS
*/

static void test_bug9643() {
  MYSQL_STMT *stmt;
  MYSQL_BIND my_bind[1];
  int32 a;
  int rc;
  const char *stmt_text;
  int num_rows = 0;
  ulong type;
  ulong prefetch_rows = 5;

  myheader("test_bug9643");

  mysql_query(mysql, "drop table if exists t1");
  mysql_query(mysql, "create table t1 (id integer not null primary key)");
  rc = mysql_query(mysql,
                   "insert into t1 (id) values "
                   " (1), (2), (3), (4), (5), (6), (7), (8), (9)");
  myquery(rc);

  stmt = mysql_stmt_init(mysql);
  /* Not implemented in 5.0 */
  type = (ulong)CURSOR_TYPE_SCROLLABLE;
  rc = mysql_stmt_attr_set(stmt, STMT_ATTR_CURSOR_TYPE, (void *)&type);
  DIE_UNLESS(rc);
  if (!opt_silent)
    printf("Got error (as expected): %s\n", mysql_stmt_error(stmt));

  type = (ulong)CURSOR_TYPE_READ_ONLY;
  rc = mysql_stmt_attr_set(stmt, STMT_ATTR_CURSOR_TYPE, (void *)&type);
  check_execute(stmt, rc);
  rc = mysql_stmt_attr_set(stmt, STMT_ATTR_PREFETCH_ROWS,
                           (void *)&prefetch_rows);
  check_execute(stmt, rc);
  stmt_text = "select * from t1";
  rc = mysql_stmt_prepare(stmt, stmt_text, (ulong)strlen(stmt_text));
  check_execute(stmt, rc);

  memset(my_bind, 0, sizeof(my_bind));
  my_bind[0].buffer_type = MYSQL_TYPE_LONG;
  my_bind[0].buffer = (void *)&a;
  my_bind[0].buffer_length = (ulong)sizeof(a);
  mysql_stmt_bind_result(stmt, my_bind);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  while ((rc = mysql_stmt_fetch(stmt)) == 0) ++num_rows;
  DIE_UNLESS(num_rows == 9);

  rc = mysql_stmt_close(stmt);
  DIE_UNLESS(rc == 0);

  rc = mysql_query(mysql, "drop table t1");
  myquery(rc);
}

/*
  Bug#11111: fetch from view returns wrong data
*/

static void test_bug11111() {
  MYSQL_STMT *stmt;
  MYSQL_BIND my_bind[2];
  char buf[2][20];
  ulong len[2];
  int i;
  int rc;
  const char *query = "SELECT DISTINCT f1,ff2 FROM v1";

  myheader("test_bug11111");

  rc = mysql_query(mysql, "drop table if exists t1, t2, v1");
  myquery(rc);
  rc = mysql_query(mysql, "drop view if exists t1, t2, v1");
  myquery(rc);
  rc = mysql_query(mysql, "create table t1 (f1 int, f2 int)");
  myquery(rc);
  rc = mysql_query(mysql, "create table t2 (ff1 int, ff2 int)");
  myquery(rc);
  rc =
      mysql_query(mysql, "create view v1 as select * from t1, t2 where f1=ff1");
  myquery(rc);
  rc = mysql_query(mysql, "insert into t1 values (1,1), (2,2), (3,3)");
  myquery(rc);
  rc = mysql_query(mysql, "insert into t2 values (1,1), (2,2), (3,3)");
  myquery(rc);

  stmt = mysql_stmt_init(mysql);

  mysql_stmt_prepare(stmt, query, (ulong)strlen(query));
  mysql_stmt_execute(stmt);

  memset(my_bind, 0, sizeof(my_bind));
  for (i = 0; i < 2; i++) {
    my_bind[i].buffer_type = MYSQL_TYPE_STRING;
    my_bind[i].buffer = (uchar **)&buf[i];
    my_bind[i].buffer_length = 20;
    my_bind[i].length = &len[i];
  }

  rc = mysql_stmt_bind_result(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);
  if (!opt_silent) printf("return: %s", buf[1]);
  DIE_UNLESS(!strcmp(buf[1], "1"));
  mysql_stmt_close(stmt);
  rc = mysql_query(mysql, "drop view v1");
  myquery(rc);
  rc = mysql_query(mysql, "drop table t1, t2");
  myquery(rc);
}

/*
  Check that proper cleanups are done for prepared statement when
  fetching thorugh a cursor.
*/

static void test_bug10729() {
  MYSQL_STMT *stmt;
  MYSQL_BIND my_bind[1];
  char a[21];
  int rc;
  const char *stmt_text;
  int i = 0;
  const char *name_array[3] = {"aaa", "bbb", "ccc"};
  ulong type;

  myheader("test_bug10729");

  mysql_query(mysql, "drop table if exists t1");
  mysql_query(mysql,
              "create table t1 (id integer not null primary key,"
              "name VARCHAR(20) NOT NULL)");
  rc = mysql_query(mysql,
                   "insert into t1 (id, name) values "
                   "(1, 'aaa'), (2, 'bbb'), (3, 'ccc')");
  myquery(rc);

  stmt = mysql_stmt_init(mysql);

  type = (ulong)CURSOR_TYPE_READ_ONLY;
  rc = mysql_stmt_attr_set(stmt, STMT_ATTR_CURSOR_TYPE, (void *)&type);
  check_execute(stmt, rc);
  stmt_text = "select name from t1";
  rc = mysql_stmt_prepare(stmt, stmt_text, (ulong)strlen(stmt_text));
  check_execute(stmt, rc);

  memset(my_bind, 0, sizeof(my_bind));
  my_bind[0].buffer_type = MYSQL_TYPE_STRING;
  my_bind[0].buffer = (void *)a;
  my_bind[0].buffer_length = (ulong)sizeof(a);
  mysql_stmt_bind_result(stmt, my_bind);

  for (i = 0; i < 3; i++) {
    int row_no = 0;
    rc = mysql_stmt_execute(stmt);
    check_execute(stmt, rc);
    while ((rc = mysql_stmt_fetch(stmt)) == 0) {
      DIE_UNLESS(strcmp(a, name_array[row_no]) == 0);
      if (!opt_silent) printf("%d: %s\n", row_no, a);
      ++row_no;
    }
    DIE_UNLESS(rc == MYSQL_NO_DATA);
  }
  rc = mysql_stmt_close(stmt);
  DIE_UNLESS(rc == 0);

  rc = mysql_query(mysql, "drop table t1");
  myquery(rc);
}

/*
  Check that mysql_next_result works properly in case when one of
  the statements used in a multi-statement query is erroneous
*/

static void test_bug9992() {
  MYSQL *mysql1;
  MYSQL_RES *res;
  int rc;

  myheader("test_bug9992");

  if (!opt_silent)
    printf("Establishing a connection with option CLIENT_MULTI_STATEMENTS..\n");

  mysql1 = mysql_client_init(nullptr);

  if (!mysql_real_connect(mysql1, opt_host, opt_user, opt_password,
                          opt_db ? opt_db : "test", opt_port, opt_unix_socket,
                          CLIENT_MULTI_STATEMENTS)) {
    fprintf(stderr, "Failed to connect to the database\n");
    DIE_UNLESS(0);
  }

  /* Sic: SHOW DATABASE is incorrect syntax. */
  rc = mysql_query(mysql1, "SHOW TABLES; SHOW DATABASE; SELECT 1;");

  if (rc) {
    fprintf(stderr, "[%d] %s\n", mysql_errno(mysql1), mysql_error(mysql1));
    DIE_UNLESS(0);
  }

  if (!opt_silent) printf("Testing mysql_store_result/mysql_next_result..\n");

  res = mysql_store_result(mysql1);
  DIE_UNLESS(res);
  mysql_free_result(res);
  rc = mysql_next_result(mysql1);
  DIE_UNLESS(rc == 1); /* Got errors, as expected */

  if (!opt_silent)
    fprintf(stdout, "Got error, as expected:\n [%d] %s\n", mysql_errno(mysql1),
            mysql_error(mysql1));

  mysql_close(mysql1);
}

/* Bug#10736: cursors and subqueries, memroot management */

static void test_bug10736() {
  MYSQL_STMT *stmt;
  MYSQL_BIND my_bind[1];
  char a[21];
  int rc;
  const char *stmt_text;
  int i = 0;
  ulong type;

  myheader("test_bug10736");

  mysql_query(mysql, "drop table if exists t1");
  mysql_query(mysql,
              "create table t1 (id integer not null primary key,"
              "name VARCHAR(20) NOT NULL)");
  rc = mysql_query(mysql,
                   "insert into t1 (id, name) values "
                   "(1, 'aaa'), (2, 'bbb'), (3, 'ccc')");
  myquery(rc);

  stmt = mysql_stmt_init(mysql);

  type = (ulong)CURSOR_TYPE_READ_ONLY;
  rc = mysql_stmt_attr_set(stmt, STMT_ATTR_CURSOR_TYPE, (void *)&type);
  check_execute(stmt, rc);
  stmt_text = "select name from t1 where name=(select name from t1 where id=2)";
  rc = mysql_stmt_prepare(stmt, stmt_text, (ulong)strlen(stmt_text));
  check_execute(stmt, rc);

  memset(my_bind, 0, sizeof(my_bind));
  my_bind[0].buffer_type = MYSQL_TYPE_STRING;
  my_bind[0].buffer = (void *)a;
  my_bind[0].buffer_length = (ulong)sizeof(a);
  mysql_stmt_bind_result(stmt, my_bind);

  for (i = 0; i < 3; i++) {
    int row_no = 0;
    rc = mysql_stmt_execute(stmt);
    check_execute(stmt, rc);
    while ((rc = mysql_stmt_fetch(stmt)) == 0) {
      if (!opt_silent) printf("%d: %s\n", row_no, a);
      ++row_no;
    }
    DIE_UNLESS(rc == MYSQL_NO_DATA);
  }
  rc = mysql_stmt_close(stmt);
  DIE_UNLESS(rc == 0);

  rc = mysql_query(mysql, "drop table t1");
  myquery(rc);
}

/* Bug#10794: cursors, packets out of order */

static void test_bug10794() {
  MYSQL_STMT *stmt, *stmt1;
  MYSQL_BIND my_bind[2];
  char a[21];
  int id_val;
  ulong a_len;
  int rc;
  const char *stmt_text;
  int i = 0;
  ulong type;

  myheader("test_bug10794");

  mysql_query(mysql, "drop table if exists t1");
  mysql_query(mysql,
              "create table t1 (id integer not null primary key,"
              "name varchar(20) not null)");
  stmt = mysql_stmt_init(mysql);
  stmt_text = "insert into t1 (id, name) values (?, ?)";
  rc = mysql_stmt_prepare(stmt, stmt_text, (ulong)strlen(stmt_text));
  check_execute(stmt, rc);
  memset(my_bind, 0, sizeof(my_bind));
  my_bind[0].buffer_type = MYSQL_TYPE_LONG;
  my_bind[0].buffer = (void *)&id_val;
  my_bind[1].buffer_type = MYSQL_TYPE_STRING;
  my_bind[1].buffer = (void *)a;
  my_bind[1].length = &a_len;
  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);
  for (i = 0; i < 42; i++) {
    id_val = (i + 1) * 10;
    sprintf(a, "a%d", i);
    a_len = (ulong)strlen(a); /* safety against broken sprintf */
    rc = mysql_stmt_execute(stmt);
    check_execute(stmt, rc);
  }
  stmt_text = "select name from t1";
  rc = mysql_stmt_prepare(stmt, stmt_text, (ulong)strlen(stmt_text));
  type = (ulong)CURSOR_TYPE_READ_ONLY;
  mysql_stmt_attr_set(stmt, STMT_ATTR_CURSOR_TYPE, (const void *)&type);
  stmt1 = mysql_stmt_init(mysql);
  mysql_stmt_attr_set(stmt1, STMT_ATTR_CURSOR_TYPE, (const void *)&type);
  memset(my_bind, 0, sizeof(my_bind));
  my_bind[0].buffer_type = MYSQL_TYPE_STRING;
  my_bind[0].buffer = (void *)a;
  my_bind[0].buffer_length = (ulong)sizeof(a);
  my_bind[0].length = &a_len;
  rc = mysql_stmt_bind_result(stmt, my_bind);
  check_execute(stmt, rc);
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);
  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);
  if (!opt_silent) printf("Fetched row from stmt: %s\n", a);
  /* Don't optimize: an attribute of the original test case */
  mysql_stmt_free_result(stmt);
  mysql_stmt_reset(stmt);
  stmt_text = "select name from t1 where id=10";
  rc = mysql_stmt_prepare(stmt1, stmt_text, (ulong)strlen(stmt_text));
  check_execute(stmt1, rc);
  rc = mysql_stmt_bind_result(stmt1, my_bind);
  check_execute(stmt1, rc);
  rc = mysql_stmt_execute(stmt1);
  while (true) {
    rc = mysql_stmt_fetch(stmt1);
    if (rc == MYSQL_NO_DATA) {
      if (!opt_silent) printf("End of data in stmt1\n");
      break;
    }
    check_execute(stmt1, rc);
    if (!opt_silent) printf("Fetched row from stmt1: %s\n", a);
  }
  mysql_stmt_close(stmt);
  mysql_stmt_close(stmt1);

  rc = mysql_query(mysql, "drop table t1");
  myquery(rc);
}

/* Bug#11172: cursors, crash on a fetch from a datetime column */

static void test_bug11172() {
  MYSQL_STMT *stmt;
  MYSQL_BIND bind_in[1], bind_out[2];
  MYSQL_TIME hired;
  int rc;
  const char *stmt_text;
  int i = 0, id;
  ulong type;

  myheader("test_bug11172");

  mysql_query(mysql, "drop table if exists t1");
  mysql_query(mysql,
              "create table t1 (id integer not null primary key,"
              "hired date not null)");
  rc = mysql_query(mysql,
                   "insert into t1 (id, hired) values (1, '1933-08-24'), "
                   "(2, '1965-01-01'), (3, '1949-08-17'), (4, '1945-07-07'), "
                   "(5, '1941-05-15'), (6, '1978-09-15'), (7, '1936-03-28')");
  myquery(rc);
  stmt = mysql_stmt_init(mysql);
  stmt_text = "SELECT id, hired FROM t1 WHERE hired=?";
  rc = mysql_stmt_prepare(stmt, stmt_text, (ulong)strlen(stmt_text));
  check_execute(stmt, rc);

  type = (ulong)CURSOR_TYPE_READ_ONLY;
  mysql_stmt_attr_set(stmt, STMT_ATTR_CURSOR_TYPE, (const void *)&type);

  memset(bind_in, 0, sizeof(bind_in));
  memset(bind_out, 0, sizeof(bind_out));
  memset(&hired, 0, sizeof(hired));
  hired.year = 1965;
  hired.month = 1;
  hired.day = 1;
  bind_in[0].buffer_type = MYSQL_TYPE_DATE;
  bind_in[0].buffer = (void *)&hired;
  bind_in[0].buffer_length = (ulong)sizeof(hired);
  bind_out[0].buffer_type = MYSQL_TYPE_LONG;
  bind_out[0].buffer = (void *)&id;
  bind_out[1] = bind_in[0];

  for (i = 0; i < 3; i++) {
    rc = mysql_stmt_bind_param(stmt, bind_in);
    check_execute(stmt, rc);
    rc = mysql_stmt_bind_result(stmt, bind_out);
    check_execute(stmt, rc);
    rc = mysql_stmt_execute(stmt);
    check_execute(stmt, rc);
    while ((rc = mysql_stmt_fetch(stmt)) == 0) {
      if (!opt_silent)
        printf("fetched data %d:%d-%d-%d\n", id, hired.year, hired.month,
               hired.day);
    }
    DIE_UNLESS(rc == MYSQL_NO_DATA);
    if (!mysql_stmt_free_result(stmt)) mysql_stmt_reset(stmt);
  }
  mysql_stmt_close(stmt);
  mysql_rollback(mysql);
  mysql_rollback(mysql);

  rc = mysql_query(mysql, "drop table t1");
  myquery(rc);
}

/* Bug#11656: cursors, crash on a fetch from a query with distinct. */

static void test_bug11656() {
  MYSQL_STMT *stmt;
  MYSQL_BIND my_bind[2];
  int rc;
  const char *stmt_text;
  char buf[2][20];
  int i = 0;
  ulong type;

  myheader("test_bug11656");

  mysql_query(mysql, "drop table if exists t1");

  rc = mysql_query(mysql,
                   "create table t1 ("
                   "server varchar(40) not null, "
                   "test_kind varchar(1) not null, "
                   "test_id varchar(30) not null , "
                   "primary key (server,test_kind,test_id))");
  myquery(rc);

  stmt_text =
      "select distinct test_kind, test_id from t1 "
      "where server in (?, ?)";
  stmt = mysql_stmt_init(mysql);
  rc = mysql_stmt_prepare(stmt, stmt_text, (ulong)strlen(stmt_text));
  check_execute(stmt, rc);
  type = (ulong)CURSOR_TYPE_READ_ONLY;
  mysql_stmt_attr_set(stmt, STMT_ATTR_CURSOR_TYPE, (const void *)&type);

  memset(my_bind, 0, sizeof(my_bind));
  my_stpcpy(buf[0], "pcint502_MY2");
  my_stpcpy(buf[1], "*");
  for (i = 0; i < 2; i++) {
    my_bind[i].buffer_type = MYSQL_TYPE_STRING;
    my_bind[i].buffer = (uchar **)&buf[i];
    my_bind[i].buffer_length = (ulong)strlen(buf[i]);
  }
  mysql_stmt_bind_param(stmt, my_bind);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == MYSQL_NO_DATA);

  mysql_stmt_close(stmt);
  rc = mysql_query(mysql, "drop table t1");
  myquery(rc);
}

/*
  Check that the server signals when NO_BACKSLASH_ESCAPES mode is in effect,
  and mysql_real_escape_string_quote() does the right thing as a result.
*/

static void test_bug10214() {
  int len;
  char out[8];

  myheader("test_bug10214");

  DIE_UNLESS(!(mysql->server_status & SERVER_STATUS_NO_BACKSLASH_ESCAPES));

  len = mysql_real_escape_string_quote(mysql, out, "a'b\\c", 5, '\'');
  DIE_UNLESS(memcmp(out, "a\\'b\\\\c", len) == 0);

  mysql_query(mysql, "set sql_mode='NO_BACKSLASH_ESCAPES'");
  DIE_UNLESS(mysql->server_status & SERVER_STATUS_NO_BACKSLASH_ESCAPES);

  len = mysql_real_escape_string_quote(mysql, out, "a'b\\c", 5, '\'');
  DIE_UNLESS(memcmp(out, "a''b\\c", len) == 0);

  mysql_query(mysql, "set sql_mode=''");
}

/*
  Check that the server signals when NO_BACKSLASH_ESCAPES mode is in effect,
  a deprecated mysql_real_escape_string() function exits with error and the
  mysql_real_escape_string_quote() does the right thing as a result.
*/

static void test_bug21246() {
  int len;
  char out[11];

  myheader("test_bug21246");

  DIE_UNLESS(!(mysql->server_status & SERVER_STATUS_NO_BACKSLASH_ESCAPES));

  len = mysql_real_escape_string(mysql, out, "a'b\\c", 5);
  DIE_UNLESS(len == 7);
  DIE_UNLESS(memcmp(out, "a\\'b\\\\c", len) == 0);

  len = mysql_real_escape_string_quote(mysql, out, "a'b\\c", 5, '\'');
  DIE_UNLESS(len == 7);
  DIE_UNLESS(memcmp(out, "a\\'b\\\\c", len) == 0);

  len = mysql_real_escape_string_quote(mysql, out, "`a'b\\c`", 7, '\'');
  DIE_UNLESS(len == 9);
  DIE_UNLESS(memcmp(out, "`a\\'b\\\\c`", len) == 0);

  len = mysql_real_escape_string_quote(mysql, out, "`a'b\\c`", 7, '`');
  DIE_UNLESS(len == 9);
  DIE_UNLESS(memcmp(out, "``a'b\\c``", len) == 0);

  len = mysql_real_escape_string_quote(mysql, out, "`a'b\\c\"", 7, '"');
  DIE_UNLESS(len == 10);
  DIE_UNLESS(memcmp(out, "`a\\'b\\\\c\\\"", len) == 0);

  mysql_query(mysql, "set sql_mode='NO_BACKSLASH_ESCAPES'");
  DIE_UNLESS(mysql->server_status & SERVER_STATUS_NO_BACKSLASH_ESCAPES);

  len = mysql_real_escape_string(mysql, out, "a'b\\c", 5);
  DIE_UNLESS(len == -1);

  len = mysql_real_escape_string_quote(mysql, out, "a'b\"c", 5, '\'');
  DIE_UNLESS(len == 6);
  DIE_UNLESS(memcmp(out, "a''b\"c", len) == 0);

  len = mysql_real_escape_string_quote(mysql, out, "a'b\"c", 5, '\"');
  DIE_UNLESS(len == 6);
  DIE_UNLESS(memcmp(out, "a'b\"\"c", len) == 0);

  len = mysql_real_escape_string_quote(mysql, out, "`a'b\"c`\"", 8, '\"');
  DIE_UNLESS(len == 10);
  DIE_UNLESS(memcmp(out, "`a'b\"\"c`\"\"", len) == 0);

  len = mysql_real_escape_string_quote(mysql, out, "`a'b\"c`\"", 8, '`');
  DIE_UNLESS(len == 10);
  DIE_UNLESS(memcmp(out, "``a'b\"c``\"", len) == 0);

  len = mysql_real_escape_string_quote(mysql, out, "\"a'b\"c\"\"", 8, '`');
  DIE_UNLESS(len == 8);
  DIE_UNLESS(memcmp(out, "\"a'b\"c\"\"", len) == 0);

  mysql_query(mysql, "set sql_mode=''");
}

static void test_client_character_set() {
  MY_CHARSET_INFO cs;
  const char *csname = "utf8";
  const char *csdefault = mysql_character_set_name(mysql);
  int rc;

  myheader("test_client_character_set");

  rc = mysql_set_character_set(mysql, csname);
  DIE_UNLESS(rc == 0);

  mysql_get_character_set_info(mysql, &cs);
  DIE_UNLESS(!strcmp(cs.csname, "utf8"));
  DIE_UNLESS(!strcmp(cs.name, "utf8_general_ci"));
  /* Restore the default character set */
  rc = mysql_set_character_set(mysql, csdefault);
  myquery(rc);
}

/* Test correct max length for MEDIUMTEXT and LONGTEXT columns */

static void test_bug9735() {
  MYSQL_RES *res;
  int rc;

  myheader("test_bug9735");

  rc = mysql_query(mysql, "drop table if exists t1");
  myquery(rc);
  rc = mysql_query(mysql,
                   "create table t1 (a mediumtext, b longtext) "
                   "character set latin1");
  myquery(rc);
  rc = mysql_query(mysql, "select * from t1");
  myquery(rc);
  res = mysql_store_result(mysql);
  verify_prepare_field(res, 0, "a", "a", MYSQL_TYPE_BLOB, "t1", "t1",
                       current_db, (1U << 24) - 1, 0);
  verify_prepare_field(res, 1, "b", "b", MYSQL_TYPE_BLOB, "t1", "t1",
                       current_db, ~0U, 0);
  mysql_free_result(res);
  rc = mysql_query(mysql, "drop table t1");
  myquery(rc);
}

/* Bug#11183 "mysql_stmt_reset() doesn't reset information about error" */

static void test_bug11183() {
  int rc;
  MYSQL_STMT *stmt;
  char bug_statement[] = "insert into t1 values (1)";

  myheader("test_bug11183");

  mysql_query(mysql, "drop table t1 if exists");
  mysql_query(mysql, "create table t1 (a int)");

  stmt = mysql_stmt_init(mysql);
  DIE_UNLESS(stmt != nullptr);

  rc = mysql_stmt_prepare(stmt, bug_statement, (ulong)strlen(bug_statement));
  check_execute(stmt, rc);

  rc = mysql_query(mysql, "drop table t1");
  myquery(rc);

  /* Trying to execute statement that should fail on execute stage */
  rc = mysql_stmt_execute(stmt);
  DIE_UNLESS(rc);

  mysql_stmt_reset(stmt);
  DIE_UNLESS(mysql_stmt_errno(stmt) == 0);

  mysql_query(mysql, "create table t1 (a int)");

  /* Trying to execute statement that should pass ok */
  if (mysql_stmt_execute(stmt)) {
    mysql_stmt_reset(stmt);
    DIE_UNLESS(mysql_stmt_errno(stmt) == 0);
  }

  mysql_stmt_close(stmt);

  rc = mysql_query(mysql, "drop table t1");
  myquery(rc);
}

static void test_bug11037() {
  MYSQL_STMT *stmt;
  int rc;
  const char *stmt_text;

  myheader("test_bug11037");

  mysql_query(mysql, "drop table if exists t1");

  rc = mysql_query(mysql, "create table t1 (id int not null)");
  myquery(rc);

  rc = mysql_query(mysql, "insert into t1 values (1)");
  myquery(rc);

  stmt_text = "select id FROM t1";
  stmt = mysql_stmt_init(mysql);
  rc = mysql_stmt_prepare(stmt, stmt_text, (ulong)strlen(stmt_text));

  /* expected error */
  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == 1);
  if (!opt_silent)
    fprintf(stdout, "Got error, as expected:\n [%d] %s\n",
            mysql_stmt_errno(stmt), mysql_stmt_error(stmt));

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == 0);

  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == MYSQL_NO_DATA);

  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == MYSQL_NO_DATA);

  mysql_stmt_close(stmt);
  rc = mysql_query(mysql, "drop table t1");
  myquery(rc);
}

/* Bug#10760: cursors, crash in a fetch after rollback. */
static void test_bug10760() {
  MYSQL_STMT *stmt;
  MYSQL_BIND my_bind[1];
  int rc;
  const char *stmt_text;
  char id_buf[20];
  ulong id_len;
  int i = 0;
  ulong type;

  myheader("test_bug10760");

  mysql_query(mysql, "drop table if exists t1, t2");

  /* create tables */
  rc = mysql_query(mysql,
                   "create table t1 (id integer not null primary key)"
                   " engine=MyISAM");
  myquery(rc);
  for (; i < 42; ++i) {
    char buf[100];
    sprintf(buf, "insert into t1 (id) values (%d)", i + 1);
    rc = mysql_query(mysql, buf);
    myquery(rc);
  }
  mysql_autocommit(mysql, false);
  /* create statement */
  stmt = mysql_stmt_init(mysql);
  type = (ulong)CURSOR_TYPE_READ_ONLY;
  mysql_stmt_attr_set(stmt, STMT_ATTR_CURSOR_TYPE, (const void *)&type);

  /*
    1: check that a deadlock within the same connection
    is resolved and an error is returned. The deadlock is modelled
    as follows:
    con1: open cursor for select * from t1;
    con1: insert into t1 (id) values (1)
  */
  stmt_text = "select id from t1 order by 1";
  rc = mysql_stmt_prepare(stmt, stmt_text, (ulong)strlen(stmt_text));
  check_execute(stmt, rc);
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);
  rc = mysql_query(mysql, "update t1 set id=id+100");
  /*
    If cursors are not materialized, the update will return an error;
    we mainly test that it won't deadlock.
  */
  if (rc && !opt_silent)
    printf("Got error (as expected): %s\n", mysql_error(mysql));
  /*
    2: check that MyISAM tables used in cursors survive
    COMMIT/ROLLBACK.
  */
  rc = mysql_rollback(mysql); /* should not close the cursor */
  myquery(rc);
  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  /*
    3: check that cursors to InnoDB tables are closed (for now) by
    COMMIT/ROLLBACK.
  */
  stmt_text = "select id from t1 order by 1";
  rc = mysql_stmt_prepare(stmt, stmt_text, (ulong)strlen(stmt_text));
  check_execute(stmt, rc);

  rc = mysql_query(mysql, "alter table t1 engine=InnoDB");
  myquery(rc);

  memset(my_bind, 0, sizeof(my_bind));
  my_bind[0].buffer_type = MYSQL_TYPE_STRING;
  my_bind[0].buffer = (void *)id_buf;
  my_bind[0].buffer_length = (ulong)sizeof(id_buf);
  my_bind[0].length = &id_len;
  check_execute(stmt, rc);
  mysql_stmt_bind_result(stmt, my_bind);

  rc = mysql_stmt_execute(stmt);
  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == 0);
  if (!opt_silent) printf("Fetched row %s\n", id_buf);
  rc = mysql_rollback(mysql); /* should close the cursor */
  myquery(rc);
#if 0
    rc= mysql_stmt_fetch(stmt);
    DIE_UNLESS(rc);
    if (!opt_silent)
      printf("Got error (as expected): %s\n", mysql_error(mysql));
#endif

  mysql_stmt_close(stmt);
  rc = mysql_query(mysql, "drop table t1");
  myquery(rc);
  mysql_autocommit(mysql, true); /* restore default */
}

static void test_bug12001() {
  MYSQL *mysql_local;
  MYSQL_RES *result;
  const char *query =
      "DROP TABLE IF EXISTS test_table;"
      "CREATE TABLE test_table(id INT);"
      "INSERT INTO test_table VALUES(10);"
      "UPDATE test_table SET id=20 WHERE id=10;"
      "SELECT * FROM test_table;"
      "INSERT INTO non_existent_table VALUES(11);";
  int rc, res;

  myheader("test_bug12001");

  if (!(mysql_local = mysql_client_init(nullptr))) {
    fprintf(stdout, "\n mysql_client_init() failed");
    exit(1);
  }

  /* Create connection that supports multi statements */
  if (!mysql_real_connect(mysql_local, opt_host, opt_user, opt_password,
                          current_db, opt_port, opt_unix_socket,
                          CLIENT_MULTI_STATEMENTS)) {
    fprintf(stdout, "\n mysql_real_connect() failed");
    exit(1);
  }

  rc = mysql_query(mysql_local, query);
  myquery(rc);

  do {
    if (mysql_field_count(mysql_local) &&
        (result = mysql_use_result(mysql_local))) {
      mysql_free_result(result);
    }
  } while (!(res = mysql_next_result(mysql_local)));

  rc = mysql_query(mysql_local, "DROP TABLE IF EXISTS test_table");
  myquery(rc);

  mysql_close(mysql_local);
  DIE_UNLESS(res == 1);
}

/* Bug#11909: wrong metadata if fetching from two cursors */

static void test_bug11909() {
  MYSQL_STMT *stmt1, *stmt2;
  MYSQL_BIND my_bind[7];
  int rc;
  char firstname[20], midinit[20], lastname[20], workdept[20];
  ulong firstname_len, midinit_len, lastname_len, workdept_len;
  uint32 empno;
  double salary;
  float bonus;
  const char *stmt_text;

  myheader("test_bug11909");

  stmt_text = "drop table if exists t1";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);

  stmt_text =
      "create table t1 ("
      "  empno int(11) not null, firstname varchar(20) not null,"
      "  midinit varchar(20) not null, lastname varchar(20) not null,"
      "  workdept varchar(6) not null, salary double not null,"
      "  bonus float not null, primary key (empno)"
      ") default charset=latin1 collate=latin1_bin";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);

  stmt_text =
      "insert into t1 values "
      "(10, 'CHRISTINE', 'I', 'HAAS',     'A00', 52750, 1000), "
      "(20, 'MICHAEL',   'L', 'THOMPSON', 'B01', 41250, 800),"
      "(30, 'SALLY',     'A', 'KWAN',     'C01', 38250, 800),"
      "(50, 'JOHN',      'B', 'GEYER',    'E01', 40175, 800), "
      "(60, 'IRVING',    'F', 'STERN',    'D11', 32250, 500)";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);

  /* ****** Begin of trace ****** */

  stmt1 = open_cursor(
      "SELECT empno, firstname, midinit, lastname,"
      "workdept, salary, bonus FROM t1");

  memset(my_bind, 0, sizeof(my_bind));
  my_bind[0].buffer_type = MYSQL_TYPE_LONG;
  my_bind[0].buffer = (void *)&empno;

  my_bind[1].buffer_type = MYSQL_TYPE_VAR_STRING;
  my_bind[1].buffer = (void *)firstname;
  my_bind[1].buffer_length = (ulong)sizeof(firstname);
  my_bind[1].length = &firstname_len;

  my_bind[2].buffer_type = MYSQL_TYPE_VAR_STRING;
  my_bind[2].buffer = (void *)midinit;
  my_bind[2].buffer_length = (ulong)sizeof(midinit);
  my_bind[2].length = &midinit_len;

  my_bind[3].buffer_type = MYSQL_TYPE_VAR_STRING;
  my_bind[3].buffer = (void *)lastname;
  my_bind[3].buffer_length = (ulong)sizeof(lastname);
  my_bind[3].length = &lastname_len;

  my_bind[4].buffer_type = MYSQL_TYPE_VAR_STRING;
  my_bind[4].buffer = (void *)workdept;
  my_bind[4].buffer_length = (ulong)sizeof(workdept);
  my_bind[4].length = &workdept_len;

  my_bind[5].buffer_type = MYSQL_TYPE_DOUBLE;
  my_bind[5].buffer = (void *)&salary;

  my_bind[6].buffer_type = MYSQL_TYPE_FLOAT;
  my_bind[6].buffer = (void *)&bonus;
  rc = mysql_stmt_bind_result(stmt1, my_bind);
  check_execute(stmt1, rc);

  rc = mysql_stmt_execute(stmt1);
  check_execute(stmt1, rc);

  rc = mysql_stmt_fetch(stmt1);
  DIE_UNLESS(rc == 0);
  DIE_UNLESS(empno == 10);
  DIE_UNLESS(strcmp(firstname, "CHRISTINE") == 0);
  DIE_UNLESS(strcmp(midinit, "I") == 0);
  DIE_UNLESS(strcmp(lastname, "HAAS") == 0);
  DIE_UNLESS(strcmp(workdept, "A00") == 0);
  DIE_UNLESS(salary == (double)52750.0);
  DIE_UNLESS(bonus == (float)1000.0);

  stmt2 = open_cursor("SELECT empno, firstname FROM t1");
  rc = mysql_stmt_bind_result(stmt2, my_bind);
  check_execute(stmt2, rc);

  rc = mysql_stmt_execute(stmt2);
  check_execute(stmt2, rc);

  rc = mysql_stmt_fetch(stmt2);
  DIE_UNLESS(rc == 0);

  DIE_UNLESS(empno == 10);
  DIE_UNLESS(strcmp(firstname, "CHRISTINE") == 0);

  rc = mysql_stmt_reset(stmt2);
  check_execute(stmt2, rc);

  /* ERROR: next statement should return 0 */

  rc = mysql_stmt_fetch(stmt1);
  DIE_UNLESS(rc == 0);

  mysql_stmt_close(stmt1);
  mysql_stmt_close(stmt2);
  rc = mysql_rollback(mysql);
  myquery(rc);

  rc = mysql_query(mysql, "drop table t1");
  myquery(rc);
}

/* Cursors: opening a cursor to a compilicated query with ORDER BY */

static void test_bug11901() {
  /*  MYSQL_STMT *stmt;
    MYSQL_BIND my_bind[2]; */
  int rc;
  /*  char workdept[20];
    ulong workdept_len;
    uint32 empno; */
  const char *stmt_text;

  myheader("test_bug11901");

  stmt_text = "drop table if exists t1, t2";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);

  stmt_text =
      "create table t1 ("
      "  empno int(11) not null, firstname varchar(20) not null,"
      "  midinit varchar(20) not null, lastname varchar(20) not null,"
      "  workdept varchar(6) not null, salary double not null,"
      "  bonus float not null, primary key (empno), "
      " unique key (workdept, empno) "
      ") default charset=latin1 collate=latin1_bin";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);

  stmt_text =
      "insert into t1 values "
      "(10,  'CHRISTINE', 'I', 'HAAS',      'A00', 52750, 1000),"
      "(20,  'MICHAEL',   'L', 'THOMPSON',  'B01', 41250, 800), "
      "(30,  'SALLY',     'A', 'KWAN',      'C01', 38250, 800), "
      "(50,  'JOHN',      'B', 'GEYER',     'E01', 40175, 800), "
      "(60,  'IRVING',    'F', 'STERN',     'D11', 32250, 500), "
      "(70,  'EVA',       'D', 'PULASKI',   'D21', 36170, 700), "
      "(90,  'EILEEN',    'W', 'HENDERSON', 'E11', 29750, 600), "
      "(100, 'THEODORE',  'Q', 'SPENSER',   'E21', 26150, 500), "
      "(110, 'VINCENZO',  'G', 'LUCCHESSI', 'A00', 46500, 900), "
      "(120, 'SEAN',      '',  'O\\'CONNELL', 'A00', 29250, 600), "
      "(130, 'DOLORES',   'M', 'QUINTANA',  'C01', 23800, 500), "
      "(140, 'HEATHER',   'A', 'NICHOLLS',  'C01', 28420, 600), "
      "(150, 'BRUCE',     '',  'ADAMSON',   'D11', 25280, 500), "
      "(160, 'ELIZABETH', 'R', 'PIANKA',    'D11', 22250, 400), "
      "(170, 'MASATOSHI', 'J', 'YOSHIMURA', 'D11', 24680, 500), "
      "(180, 'MARILYN',   'S', 'SCOUTTEN',  'D11', 21340, 500), "
      "(190, 'JAMES',     'H', 'WALKER',    'D11', 20450, 400), "
      "(200, 'DAVID',     '',  'BROWN',     'D11', 27740, 600), "
      "(210, 'WILLIAM',   'T', 'JONES',     'D11', 18270, 400), "
      "(220, 'JENNIFER',  'K', 'LUTZ',      'D11', 29840, 600), "
      "(230, 'JAMES',     'J', 'JEFFERSON', 'D21', 22180, 400), "
      "(240, 'SALVATORE', 'M', 'MARINO',    'D21', 28760, 600), "
      "(250, 'DANIEL',    'S', 'SMITH',     'D21', 19180, 400), "
      "(260, 'SYBIL',     'P', 'JOHNSON',   'D21', 17250, 300), "
      "(270, 'MARIA',     'L', 'PEREZ',     'D21', 27380, 500), "
      "(280, 'ETHEL',     'R', 'SCHNEIDER', 'E11', 26250, 500), "
      "(290, 'JOHN',      'R', 'PARKER',    'E11', 15340, 300), "
      "(300, 'PHILIP',    'X', 'SMITH',     'E11', 17750, 400), "
      "(310, 'MAUDE',     'F', 'SETRIGHT',  'E11', 15900, 300), "
      "(320, 'RAMLAL',    'V', 'MEHTA',     'E21', 19950, 400), "
      "(330, 'WING',      '',  'LEE',       'E21', 25370, 500), "
      "(340, 'JASON',     'R', 'GOUNOT',    'E21', 23840, 500)";

  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);

  stmt_text =
      "create table t2 ("
      " deptno varchar(6) not null, deptname varchar(20) not null,"
      " mgrno int(11) not null, location varchar(20) not null,"
      " admrdept varchar(6) not null, refcntd int(11) not null,"
      " refcntu int(11) not null, primary key (deptno)"
      ") default charset=latin1 collate=latin1_bin";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);

  stmt_text =
      "insert into t2 values "
      "('A00', 'SPIFFY COMPUTER SERV', 10, '', 'A00', 0, 0), "
      "('B01', 'PLANNING',             20, '', 'A00', 0, 0), "
      "('C01', 'INFORMATION CENTER',   30, '', 'A00', 0, 0), "
      "('D01', 'DEVELOPMENT CENTER',   0,  '', 'A00', 0, 0),"
      "('D11', 'MANUFACTURING SYSTEM', 60, '', 'D01', 0, 0), "
      "('D21', 'ADMINISTRATION SYSTE', 70, '', 'D01', 0, 0), "
      "('E01', 'SUPPORT SERVICES',     50, '', 'A00', 0, 0), "
      "('E11', 'OPERATIONS',           90, '', 'E01', 0, 0), "
      "('E21', 'SOFTWARE SUPPORT',     100,'', 'E01', 0, 0)";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);

  /* ****** Begin of trace ****** */
  /* WL#1110 - disabled test case failure - crash. */
  /*
    stmt= open_cursor("select t1.emp, t1.workdept "
                      "from (t1 left join t2 on t2.deptno = t1.workdept) "
                      "where t2.deptno in "
                      "   (select t2.deptno "
                      "    from (t1 left join t2 on t2.deptno = t1.workdept) "
                      "    where t1.empno = ?) "
                      "order by 1");
    memset(my_bind, 0, sizeof(my_bind));

    my_bind[0].buffer_type= MYSQL_TYPE_LONG;
    my_bind[0].buffer= &empno;
    rc= mysql_stmt_bind_param(stmt, my_bind);
    check_execute(stmt, rc);

    my_bind[1].buffer_type= MYSQL_TYPE_VAR_STRING;
    my_bind[1].buffer= (void*) workdept;
    my_bind[1].buffer_length= sizeof(workdept);
    my_bind[1].length= &workdept_len;

    rc= mysql_stmt_bind_result(stmt, my_bind);
    check_execute(stmt, rc);

    empno= 10;
  */
  /* ERROR: next statement causes a server crash */
  /*
    rc= mysql_stmt_execute(stmt);
    check_execute(stmt, rc);

    mysql_stmt_close(stmt);

    rc= mysql_query(mysql, "drop table t1, t2");
    myquery(rc);
  */
}

/* Bug#11904: mysql_stmt_attr_set CURSOR_TYPE_READ_ONLY grouping wrong result */

static void test_bug11904() {
  MYSQL_STMT *stmt1;
  int rc;
  const char *stmt_text;
  const ulong type = (ulong)CURSOR_TYPE_READ_ONLY;
  MYSQL_BIND my_bind[2];
  int country_id = 0;
  char row_data[11] = {0};

  myheader("test_bug11904");

  /* create tables */
  rc = mysql_query(mysql, "DROP TABLE IF EXISTS bug11904b");
  myquery(rc);
  rc = mysql_query(
      mysql,
      "CREATE TABLE bug11904b (id int, name char(10), primary key(id, name))");
  myquery(rc);

  rc = mysql_query(mysql,
                   "INSERT INTO bug11904b VALUES (1, 'sofia'), (1,'plovdiv'),"
                   " (1,'varna'), (2,'LA'), (2,'new york'), (3,'heidelberg'),"
                   " (3,'berlin'), (3, 'frankfurt')");

  myquery(rc);
  mysql_commit(mysql);
  /* create statement */
  stmt1 = mysql_stmt_init(mysql);
  mysql_stmt_attr_set(stmt1, STMT_ATTR_CURSOR_TYPE, (const void *)&type);

  stmt_text = "SELECT id, MIN(name) FROM bug11904b GROUP BY id";

  rc = mysql_stmt_prepare(stmt1, stmt_text, (ulong)strlen(stmt_text));
  check_execute(stmt1, rc);

  memset(my_bind, 0, sizeof(my_bind));
  my_bind[0].buffer_type = MYSQL_TYPE_LONG;
  my_bind[0].buffer = &country_id;
  my_bind[0].buffer_length = 0;
  my_bind[0].length = nullptr;

  my_bind[1].buffer_type = MYSQL_TYPE_STRING;
  my_bind[1].buffer = &row_data;
  my_bind[1].buffer_length = (ulong)(sizeof(row_data) - 1);
  my_bind[1].length = nullptr;

  rc = mysql_stmt_bind_result(stmt1, my_bind);
  check_execute(stmt1, rc);

  rc = mysql_stmt_execute(stmt1);
  check_execute(stmt1, rc);

  rc = mysql_stmt_fetch(stmt1);
  check_execute(stmt1, rc);
  DIE_UNLESS(country_id == 1);
  DIE_UNLESS(memcmp(row_data, "plovdiv", 7) == 0);

  rc = mysql_stmt_fetch(stmt1);
  check_execute(stmt1, rc);
  DIE_UNLESS(country_id == 2);
  DIE_UNLESS(memcmp(row_data, "LA", 2) == 0);

  rc = mysql_stmt_fetch(stmt1);
  check_execute(stmt1, rc);
  DIE_UNLESS(country_id == 3);
  DIE_UNLESS(memcmp(row_data, "berlin", 6) == 0);

  rc = mysql_stmt_close(stmt1);
  check_execute(stmt1, rc);

  rc = mysql_query(mysql, "drop table bug11904b");
  myquery(rc);
}

/* Bug#12243: multiple cursors, crash in a fetch after commit. */

static void test_bug12243() {
  MYSQL_STMT *stmt1, *stmt2;
  int rc;
  const char *stmt_text;
  ulong type;

  myheader("test_bug12243");

  /* create tables */
  mysql_query(mysql, "drop table if exists t1");
  mysql_query(mysql, "create table t1 (a int) engine=InnoDB");
  rc = mysql_query(mysql, "insert into t1 (a) values (1), (2)");
  myquery(rc);
  mysql_autocommit(mysql, false);
  /* create statement */
  stmt1 = mysql_stmt_init(mysql);
  stmt2 = mysql_stmt_init(mysql);
  type = (ulong)CURSOR_TYPE_READ_ONLY;
  mysql_stmt_attr_set(stmt1, STMT_ATTR_CURSOR_TYPE, (const void *)&type);
  mysql_stmt_attr_set(stmt2, STMT_ATTR_CURSOR_TYPE, (const void *)&type);

  stmt_text = "select a from t1";

  rc = mysql_stmt_prepare(stmt1, stmt_text, (ulong)strlen(stmt_text));
  check_execute(stmt1, rc);
  rc = mysql_stmt_execute(stmt1);
  check_execute(stmt1, rc);
  rc = mysql_stmt_fetch(stmt1);
  check_execute(stmt1, rc);

  rc = mysql_stmt_prepare(stmt2, stmt_text, (ulong)strlen(stmt_text));
  check_execute(stmt2, rc);
  rc = mysql_stmt_execute(stmt2);
  check_execute(stmt2, rc);
  rc = mysql_stmt_fetch(stmt2);
  check_execute(stmt2, rc);

  rc = mysql_stmt_close(stmt1);
  check_execute(stmt1, rc);
  rc = mysql_commit(mysql);
  myquery(rc);
  rc = mysql_stmt_fetch(stmt2);
  check_execute(stmt2, rc);

  mysql_stmt_close(stmt2);
  rc = mysql_query(mysql, "drop table t1");
  myquery(rc);
  mysql_autocommit(mysql, true); /* restore default */
}

/*
  Bug#11718: query with function, join and order by returns wrong type
*/

static void test_bug11718() {
  MYSQL_RES *res;
  int rc;
  const char *query =
      "select str_to_date(concat(f3),'%Y%m%d') from t1,t2 "
      "where f1=f2 order by f1";

  myheader("test_bug11718");

  rc = mysql_query(mysql, "drop table if exists t1, t2");
  myquery(rc);
  rc = mysql_query(mysql, "create table t1 (f1 int)");
  myquery(rc);
  rc = mysql_query(mysql, "create table t2 (f2 int, f3 numeric(8))");
  myquery(rc);
  rc = mysql_query(mysql, "insert into t1 values (1), (2)");
  myquery(rc);
  rc = mysql_query(mysql, "insert into t2 values (1,20050101), (2,20050202)");
  myquery(rc);
  rc = mysql_query(mysql, query);
  myquery(rc);
  res = mysql_store_result(mysql);

  if (!opt_silent)
    printf("return type: %s",
           (res->fields[0].type == MYSQL_TYPE_DATE) ? "DATE" : "not DATE");
  DIE_UNLESS(res->fields[0].type == MYSQL_TYPE_DATE);
  mysql_free_result(res);
  rc = mysql_query(mysql, "drop table t1, t2");
  myquery(rc);
}

/*
  Bug #12925: Bad handling of maximum values in getopt
*/
static void test_bug12925() {
  myheader("test_bug12925");
  if (opt_getopt_ll_test)
    DIE_UNLESS(opt_getopt_ll_test == 25600 * 1024 * 1024LL);
}

/*
  Bug#14210 "Simple query with > operator on large table gives server
  crash"
*/

static void test_bug14210() {
  MYSQL_STMT *stmt;
  int rc, i;
  const char *stmt_text;
  ulong type;

  myheader("test_bug14210");

  mysql_query(mysql, "drop table if exists t1");
  /*
    To trigger the problem the table must be InnoDB, although the problem
    itself is not InnoDB related. In case the table is MyISAM this test
    is harmless.
  */
  mysql_query(mysql, "create table t1 (a varchar(255)) engine=InnoDB");
  rc = mysql_query(mysql, "insert into t1 (a) values (repeat('a', 256))");
  myquery(rc);
  rc = mysql_query(mysql, "set @@session.max_heap_table_size=16384");
  /* Create a big enough table (more than max_heap_table_size) */
  for (i = 0; i < 8; i++) {
    rc = mysql_query(mysql, "insert into t1 (a) select a from t1");
    myquery(rc);
  }
  /* create statement */
  stmt = mysql_stmt_init(mysql);
  type = (ulong)CURSOR_TYPE_READ_ONLY;
  mysql_stmt_attr_set(stmt, STMT_ATTR_CURSOR_TYPE, (const void *)&type);

  stmt_text = "select a from t1";

  rc = mysql_stmt_prepare(stmt, stmt_text, (ulong)strlen(stmt_text));
  check_execute(stmt, rc);
  rc = mysql_stmt_execute(stmt);
  while ((rc = mysql_stmt_fetch(stmt)) == 0)
    ;
  DIE_UNLESS(rc == MYSQL_NO_DATA);

  rc = mysql_stmt_close(stmt);

  rc = mysql_query(mysql, "drop table t1");
  myquery(rc);
  rc = mysql_query(mysql, "set @@session.max_heap_table_size=default");
  myquery(rc);
}

/* Bug#13488: wrong column metadata when fetching from cursor */

static void test_bug13488() {
  MYSQL_BIND my_bind[3];
  MYSQL_STMT *stmt1;
  int rc, f1, f2, f3, i;
  const ulong type = CURSOR_TYPE_READ_ONLY;
  const char *query = "select * from t1 left join t2 on f1=f2 where f1=1";

  myheader("test_bug13488");

  rc = mysql_query(mysql, "drop table if exists t1, t2");
  myquery(rc);
  rc = mysql_query(mysql, "create table t1 (f1 int not null primary key)");
  myquery(rc);
  rc = mysql_query(mysql,
                   "create table t2 (f2 int not null primary key, "
                   "f3 int not null)");
  myquery(rc);
  rc = mysql_query(mysql, "insert into t1 values (1), (2)");
  myquery(rc);
  rc = mysql_query(mysql, "insert into t2 values (1,2), (2,4)");
  myquery(rc);

  memset(my_bind, 0, sizeof(my_bind));
  for (i = 0; i < 3; i++) {
    my_bind[i].buffer_type = MYSQL_TYPE_LONG;
    my_bind[i].buffer_length = 4;
    my_bind[i].length = nullptr;
  }
  my_bind[0].buffer = &f1;
  my_bind[1].buffer = &f2;
  my_bind[2].buffer = &f3;

  stmt1 = mysql_stmt_init(mysql);
  rc = mysql_stmt_attr_set(stmt1, STMT_ATTR_CURSOR_TYPE, (const void *)&type);
  check_execute(stmt1, rc);

  rc = mysql_stmt_prepare(stmt1, query, (ulong)strlen(query));
  check_execute(stmt1, rc);

  rc = mysql_stmt_execute(stmt1);
  check_execute(stmt1, rc);

  rc = mysql_stmt_bind_result(stmt1, my_bind);
  check_execute(stmt1, rc);

  rc = mysql_stmt_fetch(stmt1);
  check_execute(stmt1, rc);

  rc = mysql_stmt_free_result(stmt1);
  check_execute(stmt1, rc);

  rc = mysql_stmt_reset(stmt1);
  check_execute(stmt1, rc);

  rc = mysql_stmt_close(stmt1);
  check_execute(stmt1, rc);

  if (!opt_silent) {
    printf("data: f1: %d; f2: %d; f3: %d\n", f1, f2, f3);
    printf("data is: %s\n", (f1 == 1 && f2 == 1 && f3 == 2) ? "OK" : "wrong");
  }
  DIE_UNLESS(f1 == 1 && f2 == 1 && f3 == 2);
  rc = mysql_query(mysql, "drop table t1, t2");
  myquery(rc);
}

/*
  Bug#13524: warnings of a previous command are not reset when fetching
  from a cursor.
*/

static void test_bug13524() {
  MYSQL_STMT *stmt;
  int rc;
  unsigned int warning_count;
  const ulong type = CURSOR_TYPE_READ_ONLY;
  const char *query = "select * from t1";

  myheader("test_bug13524");

  rc = mysql_query(mysql, "drop table if exists t1, t2");
  myquery(rc);
  rc = mysql_query(mysql, "create table t1 (a int not null primary key)");
  myquery(rc);
  rc = mysql_query(mysql, "insert into t1 values (1), (2), (3), (4)");
  myquery(rc);

  stmt = mysql_stmt_init(mysql);
  rc = mysql_stmt_attr_set(stmt, STMT_ATTR_CURSOR_TYPE, (const void *)&type);
  check_execute(stmt, rc);

  rc = mysql_stmt_prepare(stmt, query, (ulong)strlen(query));
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  warning_count = mysql_warning_count(mysql);
  DIE_UNLESS(warning_count == 0);

  /* Check that DROP TABLE produced a warning (no such table) */
  rc = mysql_query(mysql, "drop table if exists t2");
  myquery(rc);
  warning_count = mysql_warning_count(mysql);
  DIE_UNLESS(warning_count == 1);

  /*
    Check that fetch from a cursor cleared the warning from the previous
    command.
  */
  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);
  warning_count = mysql_warning_count(mysql);
  DIE_UNLESS(warning_count == 0);

  /* Cleanup */
  mysql_stmt_close(stmt);
  rc = mysql_query(mysql, "drop table t1");
  myquery(rc);
}

/*
  Bug#14845 "mysql_stmt_fetch returns MYSQL_NO_DATA when COUNT(*) is 0"
*/

static void test_bug14845() {
  MYSQL_STMT *stmt;
  int rc;
  const ulong type = CURSOR_TYPE_READ_ONLY;
  const char *query = "select count(*) from t1 where 1 = 0";

  myheader("test_bug14845");

  rc = mysql_query(mysql, "drop table if exists t1");
  myquery(rc);
  rc = mysql_query(mysql,
                   "create table t1 (id int(11) default null, "
                   "name varchar(20) default null)"
                   "engine=MyISAM DEFAULT CHARSET=utf8");
  myquery(rc);
  rc = mysql_query(mysql, "insert into t1 values (1,'abc'),(2,'def')");
  myquery(rc);

  stmt = mysql_stmt_init(mysql);
  rc = mysql_stmt_attr_set(stmt, STMT_ATTR_CURSOR_TYPE, (const void *)&type);
  check_execute(stmt, rc);

  rc = mysql_stmt_prepare(stmt, query, (ulong)strlen(query));
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == 0);

  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == MYSQL_NO_DATA);

  /* Cleanup */
  mysql_stmt_close(stmt);
  rc = mysql_query(mysql, "drop table t1");
  myquery(rc);
}

/*
  Bug #15510: mysql_warning_count returns 0 after mysql_stmt_fetch which
  should warn
*/
static void test_bug15510() {
  MYSQL_STMT *stmt;
  int rc;
  const char *query = "select 1 from dual where 1/0";

  myheader("test_bug15510");

  rc = mysql_query(mysql, "set @@sql_mode='ERROR_FOR_DIVISION_BY_ZERO'");
  myquery(rc);

  stmt = mysql_stmt_init(mysql);

  rc = mysql_stmt_prepare(stmt, query, (ulong)strlen(query));
  check_execute(stmt, rc);
  DIE_UNLESS(mysql_warning_count(mysql));

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);

  /* Cleanup */
  mysql_stmt_close(stmt);
  rc = mysql_query(mysql, "set @@sql_mode=''");
  myquery(rc);
}

/* Test MYSQL_OPT_RECONNECT, Bug#15719 */

static void test_opt_reconnect() {
  MYSQL *lmysql;
  bool my_true = true;

  myheader("test_opt_reconnect");

  if (!(lmysql = mysql_client_init(nullptr))) {
    myerror("mysql_client_init() failed");
    exit(1);
  }

  if (!opt_silent)
    fprintf(stdout, "reconnect before mysql_options: %d\n", lmysql->reconnect);
  DIE_UNLESS(lmysql->reconnect == 0);

  if (mysql_options(lmysql, MYSQL_OPT_RECONNECT, &my_true)) {
    myerror("mysql_options failed: unknown option MYSQL_OPT_RECONNECT\n");
    DIE_UNLESS(0);
  }

  /* reconnect should be 1 */
  if (!opt_silent)
    fprintf(stdout, "reconnect after mysql_options: %d\n", lmysql->reconnect);
  DIE_UNLESS(lmysql->reconnect == 1);

  if (!(mysql_real_connect(lmysql, opt_host, opt_user, opt_password, current_db,
                           opt_port, opt_unix_socket, 0))) {
    myerror("connection failed");
    DIE_UNLESS(0);
  }

  /* reconnect should still be 1 */
  if (!opt_silent)
    fprintf(stdout, "reconnect after mysql_real_connect: %d\n",
            lmysql->reconnect);
  DIE_UNLESS(lmysql->reconnect == 1);

  mysql_close(lmysql);

  if (!(lmysql = mysql_client_init(nullptr))) {
    myerror("mysql_client_init() failed");
    DIE_UNLESS(0);
  }

  if (!opt_silent)
    fprintf(stdout, "reconnect before mysql_real_connect: %d\n",
            lmysql->reconnect);
  DIE_UNLESS(lmysql->reconnect == 0);

  if (!(mysql_real_connect(lmysql, opt_host, opt_user, opt_password, current_db,
                           opt_port, opt_unix_socket, 0))) {
    myerror("connection failed");
    DIE_UNLESS(0);
  }

  /* reconnect should still be 0 */
  if (!opt_silent)
    fprintf(stdout, "reconnect after mysql_real_connect: %d\n",
            lmysql->reconnect);
  DIE_UNLESS(lmysql->reconnect == 0);

  mysql_close(lmysql);
}

static void test_bug12744() {
  MYSQL_STMT *prep_stmt = nullptr;
  MYSQL *lmysql;
  int rc;
  myheader("test_bug12744");

  lmysql = mysql_client_init(nullptr);
  DIE_UNLESS(lmysql);

  if (!mysql_real_connect(lmysql, opt_host, opt_user, opt_password, current_db,
                          opt_port, opt_unix_socket, 0)) {
    fprintf(stderr, "Failed to connect to the database\n");
    DIE_UNLESS(0);
  }

  prep_stmt = mysql_stmt_init(lmysql);
  rc = mysql_stmt_prepare(prep_stmt, "SELECT 1", 8);
  DIE_UNLESS(rc == 0);

  mysql_close(lmysql);

  rc = mysql_stmt_execute(prep_stmt);
  DIE_UNLESS(rc);
  rc = mysql_stmt_reset(prep_stmt);
  DIE_UNLESS(rc);
  rc = mysql_stmt_close(prep_stmt);
  DIE_UNLESS(rc == 0);
}

/* Bug #16143: mysql_stmt_sqlstate returns an empty string instead of '00000' */

static void test_bug16143() {
  MYSQL_STMT *stmt;
  myheader("test_bug16143");

  stmt = mysql_stmt_init(mysql);
  /* Check mysql_stmt_sqlstate return "no error" */
  DIE_UNLESS(strcmp(mysql_stmt_sqlstate(stmt), "00000") == 0);

  mysql_stmt_close(stmt);
}

/* Bug #16144: mysql_stmt_attr_get type error */

static void test_bug16144() {
  const bool flag_orig = true;
  bool flag = flag_orig;
  MYSQL_STMT *stmt;
  myheader("test_bug16144");

  /* Check that attr_get returns correct data on little and big endian CPUs */
  stmt = mysql_stmt_init(mysql);
  mysql_stmt_attr_set(stmt, STMT_ATTR_UPDATE_MAX_LENGTH, (const void *)&flag);
  mysql_stmt_attr_get(stmt, STMT_ATTR_UPDATE_MAX_LENGTH, (void *)&flag);
  DIE_UNLESS(flag == flag_orig);

  mysql_stmt_close(stmt);
}

/*
  Bug #15613: "libmysqlclient API function mysql_stmt_prepare returns wrong
  field length"
*/

static void test_bug15613() {
  MYSQL_STMT *stmt;
  const char *stmt_text;
  MYSQL_RES *metadata;
  MYSQL_FIELD *field;
  int rc;
  myheader("test_bug15613");

  /* I. Prepare the table */
  rc = mysql_query(mysql, "set names latin1");
  myquery(rc);
  mysql_query(mysql, "drop table if exists t1");
  rc = mysql_query(mysql,
                   "create table t1 (t text character set utf8, "
                   "tt tinytext character set utf8, "
                   "mt mediumtext character set utf8, "
                   "lt longtext character set utf8, "
                   "vl varchar(255) character set latin1,"
                   "vb varchar(255) character set binary,"
                   "vu varchar(255) character set utf8)");
  myquery(rc);

  stmt = mysql_stmt_init(mysql);

  /* II. Check SELECT metadata */
  stmt_text = ("select t, tt, mt, lt, vl, vb, vu from t1");
  rc = mysql_stmt_prepare(stmt, stmt_text, (ulong)strlen(stmt_text));
  metadata = mysql_stmt_result_metadata(stmt);
  field = mysql_fetch_fields(metadata);
  if (!opt_silent) {
    printf(
        "Field lengths (client character set is latin1):\n"
        "text character set utf8:\t\t%lu\n"
        "tinytext character set utf8:\t\t%lu\n"
        "mediumtext character set utf8:\t\t%lu\n"
        "longtext character set utf8:\t\t%lu\n"
        "varchar(255) character set latin1:\t%lu\n"
        "varchar(255) character set binary:\t%lu\n"
        "varchar(255) character set utf8:\t%lu\n",
        field[0].length, field[1].length, field[2].length, field[3].length,
        field[4].length, field[5].length, field[6].length);
  }
  DIE_UNLESS(field[0].length == 65535);
  DIE_UNLESS(field[1].length == 255);
  DIE_UNLESS(field[2].length == 16777215);
  DIE_UNLESS(field[3].length == 4294967295UL);
  DIE_UNLESS(field[4].length == 255);
  DIE_UNLESS(field[5].length == 255);
  DIE_UNLESS(field[6].length == 255);
  mysql_free_result(metadata);
  mysql_stmt_free_result(stmt);

  /* III. Cleanup */
  rc = mysql_query(mysql, "drop table t1");
  myquery(rc);
  rc = mysql_query(mysql, "set names default");
  myquery(rc);
  mysql_stmt_close(stmt);
}

/*
  Bug#17667: An attacker has the opportunity to bypass query logging.

  Note! Also tests Bug#21813, where prepared statements are used to
  run queries
*/
static void test_bug17667() {
  int rc;
  MYSQL_STMT *stmt;
  enum query_type { QT_NORMAL, QT_PREPARED };
  struct buffer_and_length {
    enum query_type qt;
    const char *buffer;
    const ulong length;
  } statements[] = {
      {QT_NORMAL, "drop table if exists bug17667", 29},
      {QT_NORMAL, "create table bug17667 (c varchar(20))", 37},
      {QT_NORMAL,
       "insert into bug17667 (c) values ('regular') /* NUL=\0 with comment */",
       68},
      {
          QT_PREPARED,
          "insert into bug17667 (c) values ('prepared') /* NUL=\0 with comment "
          "*/",
          69,
      },
      {QT_NORMAL, "insert into bug17667 (c) values ('NUL=\0 in value')", 50},
      {QT_NORMAL, "insert into bug17667 (c) values ('5 NULs=\0\0\0\0\0')", 48},
      {QT_PREPARED, "insert into bug17667 (c) values ('6 NULs=\0\0\0\0\0\0')",
       50},
      {QT_NORMAL,
       "/* NUL=\0 with comment */ insert into bug17667 (c) values ('encore')",
       67},
      {QT_NORMAL, "drop table bug17667", 19},
      {QT_NORMAL, nullptr, 0}};

  struct buffer_and_length *statement_cursor;
  FILE *log_file;
  char *master_log_filename;

  myheader("test_bug17667");

  master_log_filename =
      (char *)malloc(strlen(opt_vardir) + strlen("/log/master.log") + 1);
  strxmov(master_log_filename, opt_vardir, "/log/master.log", NullS);
  if (!opt_silent) printf("Opening '%s'\n", master_log_filename);
  log_file =
      my_fopen(master_log_filename, (int)(O_RDONLY | MY_FOPEN_BINARY), MYF(0));
  free(master_log_filename);

  if (log_file == nullptr) {
    if (!opt_silent) {
      printf(
          "Could not find the log file, VARDIR/log/master.log, so "
          "test_bug17667 is not run.\n"
          "Run test from the mysql-test/mysql-test-run* program to set up "
          "correct environment for this test.\n\n");
    }
    return;
  }

  enable_query_logs(1);

  for (statement_cursor = statements; statement_cursor->buffer != nullptr;
       statement_cursor++) {
    if (statement_cursor->qt == QT_NORMAL) {
      /* Run statement as normal query */
      rc = mysql_real_query(mysql, statement_cursor->buffer,
                            statement_cursor->length);
      myquery(rc);
    } else if (statement_cursor->qt == QT_PREPARED) {
      /*
        Run as prepared statement

        NOTE! All these queries should be in the log twice,
        one time for prepare and one time for execute
      */
      stmt = mysql_stmt_init(mysql);

      rc = mysql_stmt_prepare(stmt, statement_cursor->buffer,
                              statement_cursor->length);
      check_execute(stmt, rc);

      rc = mysql_stmt_execute(stmt);
      check_execute(stmt, rc);

      mysql_stmt_close(stmt);
    } else {
      DIE_UNLESS(0 == 1);
    }
  }

  /* Make sure the server has written the logs to disk before reading it */
  rc = mysql_query(mysql, "flush logs");
  myquery(rc);

  for (statement_cursor = statements; statement_cursor->buffer != nullptr;
       statement_cursor++) {
    int expected_hits = 1, hits = 0;
    char line_buffer[MAX_TEST_QUERY_LENGTH * 2];
    /* more than enough room for the query and some marginalia. */

    /* Prepared statments always occurs twice in log */
    if (statement_cursor->qt == QT_PREPARED) expected_hits++;

    /* Loop until we found expected number of log entries */
    do {
      /* Loop until statement is found in log */
      do {
        memset(line_buffer, '/', MAX_TEST_QUERY_LENGTH * 2);

        if (fgets(line_buffer, MAX_TEST_QUERY_LENGTH * 2, log_file) ==
            nullptr) {
          /* If fgets returned NULL, it indicates either error or EOF */
          if (feof(log_file))
            DIE("Found EOF before all statements where found");

          fprintf(stderr, "Got error %d while reading from file\n",
                  ferror(log_file));
          DIE("Read error");
        }

      } while (strstr(line_buffer, statement_cursor->buffer) == nullptr);
      hits++;
    } while (hits < expected_hits);

    if (!opt_silent)
      printf("Found statement starting with \"%s\"\n",
             statement_cursor->buffer);
  }

  restore_query_logs();

  if (!opt_silent) printf("success.  All queries found intact in the log.\n");

  my_fclose(log_file, MYF(0));
}

/*
  Bug#14169: type of group_concat() result changed to blob if tmp_table was
  used
*/
#if 0
static void test_bug14169()
{
  MYSQL_STMT *stmt;
  const char *stmt_text;
  MYSQL_RES *res;
  MYSQL_FIELD *field;
  int rc;

  myheader("test_bug14169");

  rc= mysql_query(mysql, "drop table if exists t1");
  myquery(rc);
  rc= mysql_query(mysql, "set session group_concat_max_len=1024");
  myquery(rc);
  rc= mysql_query(mysql, "create table t1 (f1 int unsigned, f2 varchar(250))");
  myquery(rc);
  rc= mysql_query(mysql, "insert into t1 values (1,repeat('a',255)),"
                         "(2,repeat('b',255))");
  myquery(rc);
  stmt= mysql_stmt_init(mysql);
  stmt_text= "select f2,group_concat(f1) from t1 group by f2";
  rc= mysql_stmt_prepare(stmt, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);
  res= mysql_stmt_result_metadata(stmt);
  field= mysql_fetch_fields(res);
  if (!opt_silent)
    printf("GROUP_CONCAT() result type %i", field[1].type);
  DIE_UNLESS(field[1].type == MYSQL_TYPE_BLOB);
  mysql_free_result(res);
  mysql_stmt_free_result(stmt);
  mysql_stmt_close(stmt);

  rc= mysql_query(mysql, "drop table t1");
  myquery(rc);
}
#endif
/*
   Test that mysql_insert_id() behaves as documented in our manual
*/
static void test_mysql_insert_id() {
  uint64_t res;
  int rc;

  myheader("test_mysql_insert_id");

  rc = mysql_query(mysql, "drop table if exists t1");
  myquery(rc);
  /* table without auto_increment column */
  rc = mysql_query(mysql, "create table t1 (f1 int, f2 varchar(25), key(f1))");
  myquery(rc);
  rc = mysql_query(mysql, "insert into t1 values (1,'a')");
  myquery(rc);
  res = mysql_insert_id(mysql);
  DIE_UNLESS(res == 0);
  rc = mysql_query(mysql, "insert into t1 values (null,'b')");
  myquery(rc);
  res = mysql_insert_id(mysql);
  DIE_UNLESS(res == 0);
  rc = mysql_query(mysql, "insert into t1 select 5,'c'");
  myquery(rc);
  res = mysql_insert_id(mysql);
  DIE_UNLESS(res == 0);

  /*
    Test for bug #34889: mysql_client_test::test_mysql_insert_id test fails
    sporadically
  */
  rc = mysql_query(mysql,
                   "create table t2 (f1 int not null primary key "
                   "auto_increment, f2 varchar(255))");
  myquery(rc);
  rc = mysql_query(mysql, "insert into t2 values (null,'b')");
  myquery(rc);
  rc = mysql_query(mysql, "insert into t1 select 5,'c'");
  myquery(rc);
  res = mysql_insert_id(mysql);
  DIE_UNLESS(res == 0);
  rc = mysql_query(mysql, "drop table t2");
  myquery(rc);

  rc = mysql_query(mysql, "insert into t1 select null,'d'");
  myquery(rc);
  res = mysql_insert_id(mysql);
  DIE_UNLESS(res == 0);
  rc = mysql_query(mysql, "insert into t1 values (null,last_insert_id(300))");
  myquery(rc);
  res = mysql_insert_id(mysql);
  DIE_UNLESS(res == 300);
  rc = mysql_query(mysql, "insert into t1 select null,last_insert_id(400)");
  myquery(rc);
  res = mysql_insert_id(mysql);
  /*
    Behaviour change: old code used to return 0; but 400 is consistent
    with INSERT VALUES, and the manual's section of mysql_insert_id() does not
    say INSERT SELECT should be different.
  */
  DIE_UNLESS(res == 400);

  /* table with auto_increment column */
  rc = mysql_query(mysql,
                   "create table t2 (f1 int not null primary key "
                   "auto_increment, f2 varchar(255)) ENGINE = MyISAM");
  myquery(rc);
  rc = mysql_query(mysql, "insert into t2 values (1,'a')");
  myquery(rc);
  res = mysql_insert_id(mysql);
  DIE_UNLESS(res == 1);
  /* this should not influence next INSERT if it doesn't have auto_inc */
  rc = mysql_query(mysql, "insert into t1 values (10,'e')");
  myquery(rc);
  res = mysql_insert_id(mysql);
  DIE_UNLESS(res == 0);

  rc = mysql_query(mysql, "insert into t2 values (null,'b')");
  myquery(rc);
  res = mysql_insert_id(mysql);
  DIE_UNLESS(res == 2);
  rc = mysql_query(mysql, "insert into t2 select 5,'c'");
  myquery(rc);
  res = mysql_insert_id(mysql);
  /*
    Manual says that for multirow insert this should have been 5, but does not
    say for INSERT SELECT. This is a behaviour change: old code used to return
    0. We try to be consistent with INSERT VALUES.
  */
  DIE_UNLESS(res == 5);
  rc = mysql_query(mysql, "insert into t2 select null,'d'");
  myquery(rc);
  res = mysql_insert_id(mysql);
  DIE_UNLESS(res == 6);
  /* with more than one row */
  rc = mysql_query(mysql, "insert into t2 values (10,'a'),(11,'b')");
  myquery(rc);
  res = mysql_insert_id(mysql);
  DIE_UNLESS(res == 11);
  rc = mysql_query(mysql, "insert into t2 select 12,'a' union select 13,'b'");
  myquery(rc);
  res = mysql_insert_id(mysql);
  /*
    Manual says that for multirow insert this should have been 13, but does
    not say for INSERT SELECT. This is a behaviour change: old code used to
    return 0. We try to be consistent with INSERT VALUES.
  */
  DIE_UNLESS(res == 13);
  rc = mysql_query(mysql, "insert into t2 values (null,'a'),(null,'b')");
  myquery(rc);
  res = mysql_insert_id(mysql);
  DIE_UNLESS(res == 14);
  rc = mysql_query(mysql,
                   "insert into t2 select null,'a' union select null,'b'");
  myquery(rc);
  res = mysql_insert_id(mysql);
  DIE_UNLESS(res == 16);
  rc = mysql_query(mysql, "insert into t2 select 12,'a' union select 13,'b'");
  myquery_r(rc);
  rc = mysql_query(mysql,
                   "insert ignore into t2 select 12,'a' union select 13,'b'");
  myquery(rc);
  res = mysql_insert_id(mysql);
  DIE_UNLESS(res == 0);
  rc = mysql_query(mysql, "insert into t2 values (12,'a'),(13,'b')");
  myquery_r(rc);
  res = mysql_insert_id(mysql);
  DIE_UNLESS(res == 0);
  rc = mysql_query(mysql, "insert ignore into t2 values (12,'a'),(13,'b')");
  myquery(rc);
  res = mysql_insert_id(mysql);
  DIE_UNLESS(res == 0);
  /* mixing autogenerated and explicit values */
  rc = mysql_query(mysql, "insert into t2 values (null,'e'),(12,'a'),(13,'b')");
  myquery_r(rc);
  rc = mysql_query(
      mysql, "insert into t2 values (null,'e'),(12,'a'),(13,'b'),(25,'g')");
  myquery_r(rc);
  rc = mysql_query(mysql, "insert into t2 values (null,last_insert_id(300))");
  myquery(rc);
  res = mysql_insert_id(mysql);
  /*
    according to the manual, this might be 20 or 300, but it looks like
    auto_increment column takes priority over last_insert_id().
  */
  DIE_UNLESS(res == 20);
  /* If first autogenerated number fails and 2nd works: */
  rc = mysql_query(mysql, "drop table t2");
  myquery(rc);
  rc = mysql_query(
      mysql,
      "create table t2 (f1 int not null primary key "
      "auto_increment, f2 varchar(25), unique (f2)) ENGINE = MyISAM");
  myquery(rc);
  rc = mysql_query(mysql, "insert into t2 values (null,'e')");
  res = mysql_insert_id(mysql);
  DIE_UNLESS(res == 1);
  rc = mysql_query(
      mysql, "insert ignore into t2 values (null,'e'),(null,'a'),(null,'e')");
  myquery(rc);
  res = mysql_insert_id(mysql);
  DIE_UNLESS(res == 2);
  /* If autogenerated fails and explicit works: */
  rc = mysql_query(
      mysql, "insert ignore into t2 values (null,'e'),(12,'c'),(null,'d')");
  myquery(rc);
  res = mysql_insert_id(mysql);
  /*
    Behaviour change: old code returned 3 (first autogenerated, even if it
    fails); we now return first successful autogenerated.
  */
  DIE_UNLESS(res == 13);
  /* UPDATE may update mysql_insert_id() if it uses LAST_INSERT_ID(#) */
  rc = mysql_query(mysql, "update t2 set f1=14 where f1=12");
  myquery(rc);
  res = mysql_insert_id(mysql);
  DIE_UNLESS(res == 0);
  rc = mysql_query(mysql, "update t2 set f1=0 where f1=14");
  myquery(rc);
  res = mysql_insert_id(mysql);
  DIE_UNLESS(res == 0);
  rc = mysql_query(mysql, "update t2 set f2=last_insert_id(372) where f1=0");
  myquery(rc);
  res = mysql_insert_id(mysql);
  DIE_UNLESS(res == 372);
  /* check that LAST_INSERT_ID() does not update mysql_insert_id(): */
  rc = mysql_query(mysql, "insert into t2 values (null,'g')");
  myquery(rc);
  res = mysql_insert_id(mysql);
  DIE_UNLESS(res == 15);
  rc = mysql_query(mysql,
                   "update t2 set f2=(@li:=last_insert_id()) where f1=15");
  myquery(rc);
  res = mysql_insert_id(mysql);
  DIE_UNLESS(res == 0);
  /*
    Behaviour change: now if ON DUPLICATE KEY UPDATE updates a row,
    mysql_insert_id() returns the id of the row, instead of not being
    affected.
  */
  rc = mysql_query(mysql,
                   "insert into t2 values (null,@li) on duplicate key "
                   "update f2=concat('we updated ',f2)");
  myquery(rc);
  res = mysql_insert_id(mysql);
  DIE_UNLESS(res == 15);

  rc = mysql_query(mysql, "drop table t1,t2");
  myquery(rc);
}

/*
  Test for bug#22028117: MYSQLCLIENT DOES NOT RETURN CORRECT
  MYSQL_INSERT_ID VIA DATABASE HANDLE
*/

static void test_bug22028117() {
  uint64_t res;
  int rc;
  MYSQL_STMT *stmt;

  myheader("test_bug22028117");

  rc = mysql_query(mysql, "USE test");
  myquery(rc);
  rc = mysql_query(mysql, "DROP TABLE IF EXISTS t1");
  myquery(rc);
  rc = mysql_query(mysql,
                   "CREATE TABLE t1 ("
                   "f1 INT NOT NULL PRIMARY KEY AUTO_INCREMENT,"
                   "f2 VARCHAR(255))");
  myquery(rc);
  res = mysql_insert_id(mysql);
  DIE_UNLESS(res == 0);

  rc = mysql_query(mysql, "INSERT INTO t1 (f2) VALUES ('a')");
  myquery(rc);
  res = mysql_insert_id(mysql);
  DIE_UNLESS(res == 1);

  rc = mysql_query(mysql, "INSERT INTO t1 (f2) VALUES ('b')");
  myquery(rc);
  res = mysql_insert_id(mysql);
  DIE_UNLESS(res == 2);

  /* Make sure that the value of insert_id is not lost after SELECT */
  stmt = mysql_simple_prepare(mysql, "SELECT MAX(f1) FROM t1");
  check_stmt(stmt);
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);
  rc = my_process_stmt_result(stmt);
  DIE_UNLESS(rc == 1);
  mysql_stmt_close(stmt);

  res = mysql_insert_id(mysql);
  DIE_UNLESS(res == 2);
  /* insert_id will be reset to 0 after a new table is created */
  rc = mysql_query(mysql, "DROP TABLE IF EXISTS t2");
  myquery(rc);

  rc = mysql_query(mysql,
                   "CREATE TABLE t2 ("
                   "f1 INT NOT NULL PRIMARY KEY AUTO_INCREMENT,"
                   "f2 VARCHAR(255))");
  myquery(rc);
  res = mysql_insert_id(mysql);
  DIE_UNLESS(res == 0);

  /*
    mysql_insert_id() should return expr when the INSERT query contains
    last_insert_id(expr)
  */
  rc = mysql_query(mysql, "INSERT INTO t1 (f1) VALUES (last_insert_id(100))");
  myquery(rc);
  res = mysql_insert_id(mysql);
  DIE_UNLESS(res == 100);

  rc = mysql_query(mysql, "DROP TABLE t1,t2");
  myquery(rc);
}

/*
  Bug#20152: mysql_stmt_execute() writes to MYSQL_TYPE_DATE buffer
*/

static void test_bug20152() {
  MYSQL_BIND my_bind[1];
  MYSQL_STMT *stmt;
  MYSQL_TIME tm;
  int rc;
  const char *query = "INSERT INTO t1 (f1) VALUES (?)";

  myheader("test_bug20152");

  memset(my_bind, 0, sizeof(my_bind));
  my_bind[0].buffer_type = MYSQL_TYPE_DATE;
  my_bind[0].buffer = (void *)&tm;

  tm.year = 2006;
  tm.month = 6;
  tm.day = 18;
  tm.hour = 14;
  tm.minute = 9;
  tm.second = 42;
  tm.time_type = MYSQL_TIMESTAMP_DATETIME;

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS t1");
  myquery(rc);
  rc = mysql_query(mysql, "CREATE TABLE t1 (f1 DATE)");
  myquery(rc);

  stmt = mysql_stmt_init(mysql);
  rc = mysql_stmt_prepare(stmt, query, (ulong)strlen(query));
  check_execute(stmt, rc);
  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);
  rc = mysql_stmt_close(stmt);
  check_execute(stmt, rc);
  rc = mysql_query(mysql, "DROP TABLE t1");
  myquery(rc);

  if (tm.hour == 14 && tm.minute == 9 && tm.second == 42) {
    if (!opt_silent) printf("OK!");
  } else {
    printf("[14:09:42] != [%02d:%02d:%02d]\n", tm.hour, tm.minute, tm.second);
    DIE_UNLESS(0 == 1);
  }
}

/* Bug#15752 "Lost connection to MySQL server when calling a SP from C API" */

static void test_bug15752() {
  MYSQL mysql_local;
  int rc, i;
  const int ITERATION_COUNT = 100;
  const char *query = "CALL p1()";

  myheader("test_bug15752");

  rc = mysql_query(mysql, "drop procedure if exists p1");
  myquery(rc);
  rc = mysql_query(mysql, "create procedure p1() select 1");
  myquery(rc);

  mysql_client_init(&mysql_local);
  if (!mysql_real_connect(&mysql_local, opt_host, opt_user, opt_password,
                          current_db, opt_port, opt_unix_socket,
                          CLIENT_MULTI_STATEMENTS)) {
    printf("Unable connect to MySQL server: %s\n", mysql_error(&mysql_local));
    DIE_UNLESS(0);
  }
  rc = mysql_real_query(&mysql_local, query, (ulong)strlen(query));
  myquery(rc);
  mysql_free_result(mysql_store_result(&mysql_local));

  rc = mysql_real_query(&mysql_local, query, (ulong)strlen(query));
  DIE_UNLESS(rc && mysql_errno(&mysql_local) == CR_COMMANDS_OUT_OF_SYNC);

  if (!opt_silent)
    printf("Got error (as expected): %s\n", mysql_error(&mysql_local));

  /* Check some other commands too */

  DIE_UNLESS(mysql_next_result(&mysql_local) == 0);
  mysql_free_result(mysql_store_result(&mysql_local));
  DIE_UNLESS(mysql_next_result(&mysql_local) == -1);

  /* The second problem is not reproducible: add the test case */
  for (i = 0; i < ITERATION_COUNT; i++) {
    if (mysql_real_query(&mysql_local, query, (ulong)strlen(query))) {
      printf("\ni=%d %s failed: %s\n", i, query, mysql_error(&mysql_local));
      break;
    }
    mysql_free_result(mysql_store_result(&mysql_local));
    DIE_UNLESS(mysql_next_result(&mysql_local) == 0);
    mysql_free_result(mysql_store_result(&mysql_local));
    DIE_UNLESS(mysql_next_result(&mysql_local) == -1);
  }
  mysql_close(&mysql_local);
  rc = mysql_query(mysql, "drop procedure p1");
  myquery(rc);
}

/*
  Bug#21206: memory corruption when too many cursors are opened at once

  Memory corruption happens when more than 1024 cursors are open
  simultaneously.
*/
static void test_bug21206() {
  const size_t cursor_count = 1025;

  const char *create_table[] = {"DROP TABLE IF EXISTS t1",
                                "CREATE TABLE t1 (i INT)",
                                "INSERT INTO t1 VALUES (1), (2), (3)"};
  const char *query = "SELECT * FROM t1";

  Stmt_fetch *fetch_array =
      (Stmt_fetch *)calloc(cursor_count, sizeof(Stmt_fetch));

  Stmt_fetch *fetch;

  DBUG_TRACE;
  myheader("test_bug21206");

  fill_tables(create_table, sizeof(create_table) / sizeof(*create_table));

  for (fetch = fetch_array; fetch < fetch_array + cursor_count; ++fetch) {
    /* Init will exit(1) in case of error */
    stmt_fetch_init(fetch, fetch - fetch_array, query);
  }

  for (fetch = fetch_array; fetch < fetch_array + cursor_count; ++fetch)
    stmt_fetch_close(fetch);

  free(fetch_array);
}

/*
  Ensure we execute the status code while testing
*/

static void test_status() {
  const char *status;
  DBUG_TRACE;
  myheader("test_status");

  if (!(status = mysql_stat(mysql))) {
    myerror("mysql_stat failed");                 /* purecov: inspected */
    die(__FILE__, __LINE__, "mysql_stat failed"); /* purecov: inspected */
  }
}

/*
  Bug#21726: Incorrect result with multiple invocations of
  LAST_INSERT_ID

  Test that client gets updated value of insert_id on UPDATE that uses
  LAST_INSERT_ID(expr).
  select_query added to test for bug
    #26921 Problem in mysql_insert_id() Embedded C API function
*/
static void test_bug21726() {
  const char *create_table[] = {
      "DROP TABLE IF EXISTS t1",
      "CREATE TABLE t1 (i INT)",
      "INSERT INTO t1 VALUES (1)",
  };
  const char *update_query = "UPDATE t1 SET i= LAST_INSERT_ID(i + 1)";
  int rc;
  uint64_t insert_id;
  const char *select_query = "SELECT * FROM t1";
  MYSQL_RES *result;

  DBUG_TRACE;
  myheader("test_bug21726");

  fill_tables(create_table, sizeof(create_table) / sizeof(*create_table));

  rc = mysql_query(mysql, update_query);
  myquery(rc);
  insert_id = mysql_insert_id(mysql);
  DIE_UNLESS(insert_id == 2);

  rc = mysql_query(mysql, update_query);
  myquery(rc);
  insert_id = mysql_insert_id(mysql);
  DIE_UNLESS(insert_id == 3);

  rc = mysql_query(mysql, select_query);
  myquery(rc);
  insert_id = mysql_insert_id(mysql);
  DIE_UNLESS(insert_id == 3);
  result = mysql_store_result(mysql);
  mysql_free_result(result);
}

/*
  BUG#23383: mysql_affected_rows() returns different values than
  mysql_stmt_affected_rows()

  Test that both mysql_affected_rows() and mysql_stmt_affected_rows()
  return -1 on error, 0 when no rows were affected, and (positive) row
  count when some rows were affected.
*/
static void test_bug23383() {
  const char *insert_query = "INSERT INTO t1 VALUES (1), (2)";
  const char *update_query = "UPDATE t1 SET i= 4 WHERE i = 3";
  MYSQL_STMT *stmt;
  uint64_t row_count;
  int rc;

  DBUG_TRACE;
  myheader("test_bug23383");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS t1");
  myquery(rc);

  rc = mysql_query(mysql, "CREATE TABLE t1 (i INT UNIQUE)");
  myquery(rc);

  rc = mysql_query(mysql, insert_query);
  myquery(rc);
  row_count = mysql_affected_rows(mysql);
  DIE_UNLESS(row_count == 2);

  rc = mysql_query(mysql, insert_query);
  DIE_UNLESS(rc != 0);
  row_count = mysql_affected_rows(mysql);
  DIE_UNLESS(row_count == (uint64_t)-1);

  rc = mysql_query(mysql, update_query);
  myquery(rc);
  row_count = mysql_affected_rows(mysql);
  DIE_UNLESS(row_count == 0);

  rc = mysql_query(mysql, "DELETE FROM t1");
  myquery(rc);

  stmt = mysql_stmt_init(mysql);
  DIE_UNLESS(stmt != nullptr);

  rc = mysql_stmt_prepare(stmt, insert_query, (ulong)strlen(insert_query));
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);
  row_count = mysql_stmt_affected_rows(stmt);
  DIE_UNLESS(row_count == 2);

  rc = mysql_stmt_execute(stmt);
  DIE_UNLESS(rc != 0);
  row_count = mysql_stmt_affected_rows(stmt);
  DIE_UNLESS(row_count == (uint64_t)-1);

  rc = mysql_stmt_prepare(stmt, update_query, (ulong)strlen(update_query));
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);
  row_count = mysql_stmt_affected_rows(stmt);
  DIE_UNLESS(row_count == 0);

  rc = mysql_stmt_close(stmt);
  check_execute(stmt, rc);

  rc = mysql_query(mysql, "DROP TABLE t1");
  myquery(rc);
}

/*
  BUG#21635: MYSQL_FIELD struct's member strings seem to misbehave for
  expression cols

  Check that for MIN(), MAX(), COUNT() only MYSQL_FIELD::name is set
  to either expression or its alias, and db, org_table, table,
  org_name fields are empty strings.
*/
static void test_bug21635() {
  const char *expr[] = {
      "MIN(i)",   "MIN(i)",   "MIN(i) AS A1",   "A1",
      "MAX(i)",   "MAX(i)",   "MAX(i) AS A2",   "A2",
      "COUNT(i)", "COUNT(i)", "COUNT(i) AS A3", "A3",
  };
  char query[MAX_TEST_QUERY_LENGTH];
  char *query_end;
  MYSQL_RES *result;
  MYSQL_FIELD *field;
  unsigned int field_count, i, j;
  int rc;

  DBUG_TRACE;
  myheader("test_bug21635");

  query_end = strxmov(query, "SELECT ", NullS);
  for (i = 0; i < sizeof(expr) / sizeof(*expr) / 2; ++i)
    query_end = strxmov(query_end, expr[i * 2], ", ", NullS);
  query_end = strxmov(query_end - 2, " FROM t1 GROUP BY i", NullS);
  DIE_UNLESS(query_end - query < MAX_TEST_QUERY_LENGTH);

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS t1");
  myquery(rc);
  rc = mysql_query(mysql, "CREATE TABLE t1 (i INT)");
  myquery(rc);
  /*
    We need this loop to ensure correct behavior with both constant and
    non-constant tables.
  */
  for (j = 0; j < 2; j++) {
    rc = mysql_query(mysql, "INSERT INTO t1 VALUES (1)");
    myquery(rc);

    rc = mysql_real_query(mysql, query, (ulong)(query_end - query));
    myquery(rc);

    result = mysql_use_result(mysql);
    DIE_UNLESS(result);

    field_count = mysql_field_count(mysql);
    for (i = 0; i < field_count; ++i) {
      field = mysql_fetch_field_direct(result, i);
      if (!opt_silent)
        if (!opt_silent) printf("%s -> %s ... ", expr[i * 2], field->name);
      fflush(stdout);
      DIE_UNLESS(field->db[0] == 0 && field->org_table[0] == 0 &&
                 field->table[0] == 0 && field->org_name[0] == 0);
      DIE_UNLESS(strcmp(field->name, expr[i * 2 + 1]) == 0);
      if (!opt_silent)
        if (!opt_silent) puts("OK");
    }

    mysql_free_result(result);
  }
  rc = mysql_query(mysql, "DROP TABLE t1");
  myquery(rc);
}

/*
  Bug#24179 "select b into $var" fails with --cursor_protocol"
  The failure is correct, check that the returned message is meaningful.
*/

static void test_bug24179() {
  int rc;
  MYSQL_STMT *stmt;

  DBUG_TRACE;
  myheader("test_bug24179");

  stmt = open_cursor("select 1 into @a");
  rc = mysql_stmt_execute(stmt);
  DIE_UNLESS(rc);
  if (!opt_silent) {
    printf("Got error (as expected): %d %s\n", mysql_stmt_errno(stmt),
           mysql_stmt_error(stmt));
  }
  DIE_UNLESS(mysql_stmt_errno(stmt) == 1323);
  mysql_stmt_close(stmt);
}

/**
  Bug#32265 Server returns different metadata if prepared statement is used
*/

static void test_bug32265() {
  int rc;
  MYSQL_STMT *stmt;
  MYSQL_FIELD *field;
  MYSQL_RES *metadata;

  DBUG_TRACE;
  myheader("test_bug32265");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS t1");
  myquery(rc);
  rc = mysql_query(mysql, "CREATE  TABLE t1 (a INTEGER)");
  myquery(rc);
  rc = mysql_query(mysql, "INSERT INTO t1 VALUES (1)");
  myquery(rc);
  rc = mysql_query(mysql, "CREATE VIEW v1 AS SELECT * FROM t1");
  myquery(rc);

  stmt = open_cursor("SELECT * FROM t1");
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  metadata = mysql_stmt_result_metadata(stmt);
  field = mysql_fetch_field(metadata);
  DIE_UNLESS(field);
  DIE_UNLESS(strcmp(field->table, "t1") == 0);
  DIE_UNLESS(strcmp(field->org_table, "t1") == 0);
  DIE_UNLESS(strcmp(field->db, "client_test_db") == 0);
  mysql_free_result(metadata);
  mysql_stmt_close(stmt);

  stmt = open_cursor("SELECT a '' FROM t1 ``");
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  metadata = mysql_stmt_result_metadata(stmt);
  field = mysql_fetch_field(metadata);
  DIE_UNLESS(strcmp(field->table, "") == 0);
  DIE_UNLESS(strcmp(field->org_table, "t1") == 0);
  DIE_UNLESS(strcmp(field->db, "client_test_db") == 0);
  mysql_free_result(metadata);
  mysql_stmt_close(stmt);

  stmt = open_cursor("SELECT a '' FROM t1 ``");
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  metadata = mysql_stmt_result_metadata(stmt);
  field = mysql_fetch_field(metadata);
  DIE_UNLESS(strcmp(field->table, "") == 0);
  DIE_UNLESS(strcmp(field->org_table, "t1") == 0);
  DIE_UNLESS(strcmp(field->db, "client_test_db") == 0);
  mysql_free_result(metadata);
  mysql_stmt_close(stmt);

  stmt = open_cursor("SELECT * FROM v1");
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  metadata = mysql_stmt_result_metadata(stmt);
  field = mysql_fetch_field(metadata);
  DIE_UNLESS(strcmp(field->table, "v1") == 0);
  DIE_UNLESS(strcmp(field->org_table, "v1") == 0);
  DIE_UNLESS(strcmp(field->db, "client_test_db") == 0);
  mysql_free_result(metadata);
  mysql_stmt_close(stmt);

  stmt = open_cursor("SELECT * FROM v1 /* SIC */ GROUP BY 1");
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  metadata = mysql_stmt_result_metadata(stmt);
  field = mysql_fetch_field(metadata);
  DIE_UNLESS(strcmp(field->table, "v1") == 0);
  DIE_UNLESS(strcmp(field->org_table, "v1") == 0);
  DIE_UNLESS(strcmp(field->db, "client_test_db") == 0);
  mysql_free_result(metadata);
  mysql_stmt_close(stmt);

  rc = mysql_query(mysql, "DROP VIEW v1");
  myquery(rc);
  rc = mysql_query(mysql, "DROP TABLE t1");
  myquery(rc);
}

/*
  Bug#28075 "COM_DEBUG crashes mysqld"
*/

static void test_bug28075() {
  int rc;

  DBUG_TRACE;
  myheader("test_bug28075");

  rc = mysql_dump_debug_info(mysql);
  DIE_UNLESS(rc == 0);

  rc = mysql_ping(mysql);
  DIE_UNLESS(rc == 0);
}

/*
  Bug#27876 (SF with cyrillic variable name fails during execution (regression))
*/

static void test_bug27876() {
  int rc;
  MYSQL_RES *result;

  uchar utf8_func[] = {0xd1, 0x84, 0xd1, 0x83, 0xd0, 0xbd, 0xd0,
                       0xba, 0xd1, 0x86, 0xd0, 0xb8, 0xd0, 0xb9,
                       0xd0, 0xba, 0xd0, 0xb0, 0x00};

  uchar utf8_param[] = {0xd0, 0xbf, 0xd0, 0xb0, 0xd1, 0x80, 0xd0, 0xb0,
                        0xd0, 0xbc, 0xd0, 0xb5, 0xd1, 0x82, 0xd1, 0x8a,
                        0xd1, 0x80, 0x5f, 0xd0, 0xb2, 0xd0, 0xb5, 0xd1,
                        0x80, 0xd1, 0x81, 0xd0, 0xb8, 0xd1, 0x8f, 0x00};

  char query[500];

  DBUG_TRACE;
  myheader("test_bug27876");

  rc = mysql_query(mysql, "set names utf8");
  myquery(rc);

  rc = mysql_query(mysql, "select version()");
  myquery(rc);
  result = mysql_store_result(mysql);
  mytest(result);
  mysql_free_result(result);

  sprintf(query, "DROP FUNCTION IF EXISTS %s", (char *)utf8_func);
  rc = mysql_query(mysql, query);
  myquery(rc);

  sprintf(query,
          "CREATE FUNCTION %s( %s VARCHAR(25))"
          " RETURNS VARCHAR(25) DETERMINISTIC RETURN %s",
          (char *)utf8_func, (char *)utf8_param, (char *)utf8_param);
  rc = mysql_query(mysql, query);
  myquery(rc);
  sprintf(query, "SELECT %s(VERSION())", (char *)utf8_func);
  rc = mysql_query(mysql, query);
  myquery(rc);
  result = mysql_store_result(mysql);
  mytest(result);
  mysql_free_result(result);

  sprintf(query, "DROP FUNCTION %s", (char *)utf8_func);
  rc = mysql_query(mysql, query);
  myquery(rc);

  rc = mysql_query(mysql, "set names default");
  myquery(rc);
}

/*
  Bug#28505: mysql_affected_rows() returns wrong value if CLIENT_FOUND_ROWS
  flag is set.
*/

static void test_bug28505() {
  uint64_t res;

  myquery(mysql_query(mysql, "drop table if exists t1"));
  myquery(mysql_query(mysql, "create table t1(f1 int primary key)"));
  myquery(mysql_query(mysql, "insert into t1 values(1)"));
  myquery(mysql_query(mysql,
                      "insert into t1 values(1) on duplicate key update f1=1"));
  res = mysql_affected_rows(mysql);
  DIE_UNLESS(!res);
  myquery(mysql_query(mysql, "drop table t1"));
}

/*
  Bug#28934: server crash when receiving malformed com_execute packets
*/

static void test_bug28934() {
  bool error = false;
  MYSQL_BIND bind[5];
  MYSQL_STMT *stmt;
  int cnt;

  myquery(mysql_query(mysql, "drop table if exists t1"));
  myquery(mysql_query(mysql, "create table t1(id int)"));

  myquery(mysql_query(mysql, "insert into t1 values(1),(2),(3),(4),(5)"));
  stmt = mysql_simple_prepare(mysql, "select * from t1 where id in(?,?,?,?,?)");
  check_stmt(stmt);

  memset(&bind, 0, sizeof(bind));
  for (cnt = 0; cnt < 5; cnt++) {
    bind[cnt].buffer_type = MYSQL_TYPE_LONG;
    bind[cnt].buffer = (char *)&cnt;
    bind[cnt].buffer_length = 0;
  }
  myquery(mysql_stmt_bind_param(stmt, bind));

  stmt->param_count = 2;
  error = mysql_stmt_execute(stmt);
  DIE_UNLESS(error != 0);
  myerror(nullptr);
  mysql_stmt_close(stmt);

  myquery(mysql_query(mysql, "drop table t1"));
}

/*
  Test mysql_change_user() C API and COM_CHANGE_USER
*/

static void reconnect(MYSQL **mysql) {
  mysql_close(*mysql);
  *mysql = mysql_client_init(nullptr);
  DIE_UNLESS(*mysql != nullptr);
  *mysql = mysql_real_connect(*mysql, opt_host, opt_user, opt_password,
                              current_db, opt_port, opt_unix_socket, 0);
  DIE_UNLESS(*mysql != nullptr);
}

static void test_change_user() {
  char buff[256];
  const char *user_pw = "mysqltest_pw";
  const char *user_no_pw = "mysqltest_no_pw";
  const char *pw = "password";
  const char *db = "mysqltest_user_test_database";
  int rc;
  MYSQL *l_mysql;

  DBUG_TRACE;
  myheader("test_change_user");

  l_mysql = mysql_client_init(nullptr);
  DIE_UNLESS(l_mysql != nullptr);

  l_mysql = mysql_real_connect(l_mysql, opt_host, opt_user, opt_password,
                               current_db, opt_port, opt_unix_socket, 0);
  DIE_UNLESS(l_mysql != nullptr);

  /* Prepare environment */
  sprintf(buff, "drop database if exists %s", db);
  rc = mysql_query(l_mysql, buff);
  myquery2(l_mysql, rc);

  sprintf(buff, "create database %s", db);
  rc = mysql_query(l_mysql, buff);
  myquery2(l_mysql, rc);

  sprintf(buff, "create user %s@'%%' identified by '%s'", user_pw, pw);
  rc = mysql_query(l_mysql, buff);
  myquery2(l_mysql, rc);

  sprintf(buff, "grant select on %s.* to %s@'%%'", db, user_pw);
  rc = mysql_query(l_mysql, buff);
  myquery2(l_mysql, rc);

  sprintf(buff, "create user %s@'localhost' identified by '%s'", user_pw, pw);
  rc = mysql_query(l_mysql, buff);
  myquery2(l_mysql, rc);

  sprintf(buff, "grant select on %s.* to %s@'localhost'", db, user_pw);
  rc = mysql_query(l_mysql, buff);
  myquery2(l_mysql, rc);

  sprintf(buff, "create user %s@'%%'", user_no_pw);
  rc = mysql_query(l_mysql, buff);
  myquery2(l_mysql, rc);

  sprintf(buff, "grant select on %s.* to %s@'%%'", db, user_no_pw);
  rc = mysql_query(l_mysql, buff);
  myquery2(l_mysql, rc);

  sprintf(buff, "create user %s@'localhost'", user_no_pw);
  rc = mysql_query(l_mysql, buff);
  myquery2(l_mysql, rc);

  sprintf(buff, "grant select on %s.* to %s@'localhost'", db, user_no_pw);
  rc = mysql_query(l_mysql, buff);
  myquery2(l_mysql, rc);

  /* Try some combinations */
  rc = mysql_change_user(l_mysql, nullptr, nullptr, nullptr);
  DIE_UNLESS(rc);
  if (!opt_silent)
    printf("Got error (as expected): %s\n", mysql_error(l_mysql));
  reconnect(&l_mysql);

  rc = mysql_change_user(l_mysql, "", nullptr, nullptr);
  DIE_UNLESS(rc);
  if (!opt_silent)
    printf("Got error (as expected): %s\n", mysql_error(l_mysql));
  reconnect(&l_mysql);

  rc = mysql_change_user(l_mysql, "", "", nullptr);
  DIE_UNLESS(rc);
  if (!opt_silent)
    printf("Got error (as expected): %s\n", mysql_error(l_mysql));
  reconnect(&l_mysql);

  rc = mysql_change_user(l_mysql, "", "", "");
  DIE_UNLESS(rc);
  if (!opt_silent)
    printf("Got error (as expected): %s\n", mysql_error(l_mysql));
  reconnect(&l_mysql);

  rc = mysql_change_user(l_mysql, nullptr, "", "");
  DIE_UNLESS(rc);
  if (!opt_silent)
    printf("Got error (as expected): %s\n", mysql_error(l_mysql));
  reconnect(&l_mysql);

  rc = mysql_change_user(l_mysql, nullptr, nullptr, "");
  DIE_UNLESS(rc);
  if (!opt_silent)
    printf("Got error (as expected): %s\n", mysql_error(l_mysql));
  reconnect(&l_mysql);

  rc = mysql_change_user(l_mysql, "", nullptr, "");
  DIE_UNLESS(rc);
  if (!opt_silent)
    printf("Got error (as expected): %s\n", mysql_error(l_mysql));
  reconnect(&l_mysql);

  rc = mysql_change_user(l_mysql, user_pw, nullptr, "");
  DIE_UNLESS(rc);
  if (!opt_silent)
    printf("Got error (as expected): %s\n", mysql_error(l_mysql));
  reconnect(&l_mysql);

  rc = mysql_change_user(l_mysql, user_pw, "", "");
  DIE_UNLESS(rc);
  if (!opt_silent)
    printf("Got error (as expected): %s\n", mysql_error(l_mysql));
  reconnect(&l_mysql);

  rc = mysql_change_user(l_mysql, user_pw, "", nullptr);
  DIE_UNLESS(rc);
  if (!opt_silent)
    printf("Got error (as expected): %s\n", mysql_error(l_mysql));
  reconnect(&l_mysql);

  rc = mysql_change_user(l_mysql, user_pw, nullptr, nullptr);
  DIE_UNLESS(rc);
  if (!opt_silent)
    printf("Got error (as expected): %s\n", mysql_error(l_mysql));
  reconnect(&l_mysql);

  rc = mysql_change_user(l_mysql, user_pw, "", db);
  DIE_UNLESS(rc);
  if (!opt_silent)
    printf("Got error (as expected): %s\n", mysql_error(l_mysql));
  reconnect(&l_mysql);

  rc = mysql_change_user(l_mysql, user_pw, nullptr, db);
  DIE_UNLESS(rc);
  if (!opt_silent)
    printf("Got error (as expected): %s\n", mysql_error(l_mysql));
  reconnect(&l_mysql);

  rc = mysql_change_user(l_mysql, user_pw, pw, db);
  myquery2(l_mysql, rc);

  rc = mysql_change_user(l_mysql, user_pw, pw, nullptr);
  myquery2(l_mysql, rc);

  rc = mysql_change_user(l_mysql, user_pw, pw, "");
  myquery2(l_mysql, rc);

  rc = mysql_change_user(l_mysql, user_no_pw, pw, db);
  DIE_UNLESS(rc);
  if (!opt_silent)
    printf("Got error (as expected): %s\n", mysql_error(l_mysql));
  reconnect(&l_mysql);

  rc = mysql_change_user(l_mysql, user_no_pw, pw, "");
  DIE_UNLESS(rc);
  if (!opt_silent)
    printf("Got error (as expected): %s\n", mysql_error(l_mysql));
  reconnect(&l_mysql);

  rc = mysql_change_user(l_mysql, user_no_pw, pw, nullptr);
  DIE_UNLESS(rc);
  if (!opt_silent)
    printf("Got error (as expected): %s\n", mysql_error(l_mysql));
  reconnect(&l_mysql);

  rc = mysql_change_user(l_mysql, user_no_pw, "", nullptr);
  myquery2(l_mysql, rc);

  rc = mysql_change_user(l_mysql, user_no_pw, "", "");
  myquery2(l_mysql, rc);

  rc = mysql_change_user(l_mysql, user_no_pw, "", db);
  myquery2(l_mysql, rc);

  rc = mysql_change_user(l_mysql, user_no_pw, nullptr, db);
  myquery2(l_mysql, rc);

  rc = mysql_change_user(l_mysql, "", pw, db);
  DIE_UNLESS(rc);
  if (!opt_silent)
    printf("Got error (as expected): %s\n", mysql_error(l_mysql));
  reconnect(&l_mysql);

  rc = mysql_change_user(l_mysql, "", pw, "");
  DIE_UNLESS(rc);
  if (!opt_silent)
    printf("Got error (as expected): %s\n", mysql_error(l_mysql));
  reconnect(&l_mysql);

  rc = mysql_change_user(l_mysql, "", pw, nullptr);
  DIE_UNLESS(rc);
  if (!opt_silent)
    printf("Got error (as expected): %s\n", mysql_error(l_mysql));
  reconnect(&l_mysql);

  rc = mysql_change_user(l_mysql, nullptr, pw, nullptr);
  DIE_UNLESS(rc);
  if (!opt_silent)
    printf("Got error (as expected): %s\n", mysql_error(l_mysql));
  reconnect(&l_mysql);

  rc = mysql_change_user(l_mysql, nullptr, nullptr, db);
  DIE_UNLESS(rc);
  if (!opt_silent)
    printf("Got error (as expected): %s\n", mysql_error(l_mysql));
  reconnect(&l_mysql);

  rc = mysql_change_user(l_mysql, nullptr, "", db);
  DIE_UNLESS(rc);
  if (!opt_silent)
    printf("Got error (as expected): %s\n", mysql_error(l_mysql));
  reconnect(&l_mysql);

  rc = mysql_change_user(l_mysql, "", "", db);
  DIE_UNLESS(rc);
  if (!opt_silent)
    printf("Got error (as expected): %s\n", mysql_error(l_mysql));
  reconnect(&l_mysql);

  /* Cleanup the environment */

  mysql_close(l_mysql);

  sprintf(buff, "drop database %s", db);
  rc = mysql_query(mysql, buff);
  myquery(rc);

  sprintf(buff, "drop user %s@'%%'", user_pw);
  rc = mysql_query(mysql, buff);
  myquery(rc);

  sprintf(buff, "drop user %s@'%%'", user_no_pw);
  rc = mysql_query(mysql, buff);
  myquery(rc);

  sprintf(buff, "drop user %s@'localhost'", user_pw);
  rc = mysql_query(mysql, buff);
  myquery(rc);

  sprintf(buff, "drop user %s@'localhost'", user_no_pw);
  rc = mysql_query(mysql, buff);
  myquery(rc);
}

/*
  Bug#27592 (stack overrun when storing datetime value using prepared
  statements)
*/

static void test_bug27592() {
  const int NUM_ITERATIONS = 40;
  int i;
  int rc;
  MYSQL_STMT *stmt = nullptr;
  MYSQL_BIND bind[1];
  MYSQL_TIME time_val;

  DBUG_TRACE;
  myheader("test_bug27592");

  mysql_query(mysql, "DROP TABLE IF EXISTS t1");
  mysql_query(mysql, "CREATE TABLE t1(c2 DATETIME)");

  stmt = mysql_simple_prepare(mysql, "INSERT INTO t1 VALUES (?)");
  DIE_UNLESS(stmt);

  memset(bind, 0, sizeof(bind));

  bind[0].buffer_type = MYSQL_TYPE_DATETIME;
  bind[0].buffer = (char *)&time_val;
  bind[0].length = nullptr;

  for (i = 0; i < NUM_ITERATIONS; i++) {
    time_val.year = 2007;
    time_val.month = 6;
    time_val.day = 7;
    time_val.hour = 18;
    time_val.minute = 41;
    time_val.second = 3;

    time_val.second_part = 0;
    time_val.neg = false;
    time_val.time_type = MYSQL_TIMESTAMP_NONE;

    rc = mysql_stmt_bind_param(stmt, bind);
    check_execute(stmt, rc);

    rc = mysql_stmt_execute(stmt);
    check_execute(stmt, rc);
  }

  mysql_stmt_close(stmt);
}

/*
  Bug#29687 mysql_stmt_store_result memory leak in libmysqld
*/

static void test_bug29687() {
  const int NUM_ITERATIONS = 40;
  int i;
  int rc;
  MYSQL_STMT *stmt = nullptr;

  DBUG_TRACE;
  myheader("test_bug29687");

  stmt = mysql_simple_prepare(mysql, "SELECT 1 FROM dual WHERE 0=2");
  DIE_UNLESS(stmt);

  for (i = 0; i < NUM_ITERATIONS; i++) {
    rc = mysql_stmt_execute(stmt);
    check_execute(stmt, rc);
    mysql_stmt_store_result(stmt);
    while (mysql_stmt_fetch(stmt) == 0)
      ;
    mysql_stmt_free_result(stmt);
  }

  mysql_stmt_close(stmt);
}

/*
  Bug #29692  	Single row inserts can incorrectly report a huge number of
  row insertions
*/

static void test_bug29692() {
  MYSQL *conn;

  if (!(conn = mysql_client_init(nullptr))) {
    myerror("test_bug29692 init failed");
    exit(1);
  }

  if (!(mysql_real_connect(conn, opt_host, opt_user, opt_password,
                           opt_db ? opt_db : "test", opt_port, opt_unix_socket,
                           CLIENT_FOUND_ROWS))) {
    myerror("test_bug29692 connection failed");
    mysql_close(mysql);
    exit(1);
  }
  myquery(mysql_query(conn, "drop table if exists t1"));
  myquery(mysql_query(conn, "create table t1(f1 int)"));
  myquery(mysql_query(conn, "insert into t1 values(1)"));
  DIE_UNLESS(1 == mysql_affected_rows(conn));
  myquery(mysql_query(conn, "drop table t1"));
  mysql_close(conn);
}

/**
  Bug#29306 Truncated data in MS Access with decimal (3,1) columns in a VIEW
*/

static void test_bug29306() {
  MYSQL_FIELD *field;
  int rc;
  MYSQL_RES *res;

  DBUG_TRACE;
  myheader("test_bug29306");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS tab17557");
  myquery(rc);
  rc = mysql_query(mysql, "DROP VIEW IF EXISTS view17557");
  myquery(rc);
  rc = mysql_query(mysql, "CREATE TABLE tab17557 (dd decimal (3,1))");
  myquery(rc);
  rc = mysql_query(mysql, "CREATE VIEW view17557 as SELECT dd FROM tab17557");
  myquery(rc);
  rc = mysql_query(mysql, "INSERT INTO tab17557 VALUES (7.6)");
  myquery(rc);

  /* Checking the view */
  res = mysql_list_fields(mysql, "view17557", nullptr);
  while ((field = mysql_fetch_field(res))) {
    if (!opt_silent) {
      printf("field name %s\n", field->name);
      printf("field table %s\n", field->table);
      printf("field decimals %d\n", field->decimals);
      if (field->decimals < 1) printf("Error! No decimals! \n");
      printf("\n\n");
    }
    DIE_UNLESS(field->decimals == 1);
  }
  mysql_free_result(res);

  rc = mysql_query(mysql, "DROP TABLE tab17557");
  myquery(rc);
  rc = mysql_query(mysql, "DROP VIEW view17557");
  myquery(rc);
}
/*
  Bug#30472: libmysql doesn't reset charset, insert_id after succ.
  mysql_change_user() call row insertions.
*/

static void bug30472_retrieve_charset_info(MYSQL *con, char *character_set_name,
                                           char *character_set_client,
                                           char *character_set_results,
                                           char *collation_connection) {
  MYSQL_RES *rs;
  MYSQL_ROW row;

  /* Get the cached client character set name. */

  strcpy(character_set_name, mysql_character_set_name(con));

  /* Retrieve server character set information. */

  DIE_IF(mysql_query(con, "SHOW VARIABLES LIKE 'character_set_client'"));
  DIE_UNLESS(rs = mysql_store_result(con));
  DIE_UNLESS(row = mysql_fetch_row(rs));
  strcpy(character_set_client, row[1]);
  mysql_free_result(rs);

  DIE_IF(mysql_query(con, "SHOW VARIABLES LIKE 'character_set_results'"));
  DIE_UNLESS(rs = mysql_store_result(con));
  DIE_UNLESS(row = mysql_fetch_row(rs));
  strcpy(character_set_results, row[1]);
  mysql_free_result(rs);

  DIE_IF(mysql_query(con, "SHOW VARIABLES LIKE 'collation_connection'"));
  DIE_UNLESS(rs = mysql_store_result(con));
  DIE_UNLESS(row = mysql_fetch_row(rs));
  strcpy(collation_connection, row[1]);
  mysql_free_result(rs);
}

static void test_bug30472() {
  MYSQL con;

  char character_set_name_1[MY_CS_NAME_SIZE];
  char character_set_client_1[MY_CS_NAME_SIZE];
  char character_set_results_1[MY_CS_NAME_SIZE];
  char collation_connnection_1[MY_CS_NAME_SIZE];

  char character_set_name_2[MY_CS_NAME_SIZE];
  char character_set_client_2[MY_CS_NAME_SIZE];
  char character_set_results_2[MY_CS_NAME_SIZE];
  char collation_connnection_2[MY_CS_NAME_SIZE];

  char character_set_name_3[MY_CS_NAME_SIZE];
  char character_set_client_3[MY_CS_NAME_SIZE];
  char character_set_results_3[MY_CS_NAME_SIZE];
  char collation_connnection_3[MY_CS_NAME_SIZE];

  char character_set_name_4[MY_CS_NAME_SIZE];
  char character_set_client_4[MY_CS_NAME_SIZE];
  char character_set_results_4[MY_CS_NAME_SIZE];
  char collation_connnection_4[MY_CS_NAME_SIZE];

  /* Create a new connection. */

  DIE_UNLESS(mysql_client_init(&con));

  DIE_UNLESS(mysql_real_connect(&con, opt_host, opt_user, opt_password,
                                opt_db ? opt_db : "test", opt_port,
                                opt_unix_socket, CLIENT_FOUND_ROWS));

  /* Retrieve character set information. */

  bug30472_retrieve_charset_info(
      &con, character_set_name_1, character_set_client_1,
      character_set_results_1, collation_connnection_1);

  /* Switch client character set. */

  DIE_IF(mysql_set_character_set(&con, "utf8"));

  /* Retrieve character set information. */

  bug30472_retrieve_charset_info(
      &con, character_set_name_2, character_set_client_2,
      character_set_results_2, collation_connnection_2);

  /*
    Check that
      1) character set has been switched and
      2) new character set is different from the original one.
  */

  DIE_UNLESS(strcmp(character_set_name_2, "utf8") == 0);
  DIE_UNLESS(strcmp(character_set_client_2, "utf8") == 0);
  DIE_UNLESS(strcmp(character_set_results_2, "utf8") == 0);
  DIE_UNLESS(strcmp(collation_connnection_2, "utf8_general_ci") == 0);

  DIE_UNLESS(strcmp(character_set_name_1, character_set_name_2) != 0);
  DIE_UNLESS(strcmp(character_set_client_1, character_set_client_2) != 0);
  DIE_UNLESS(strcmp(character_set_results_1, character_set_results_2) != 0);
  DIE_UNLESS(strcmp(collation_connnection_1, collation_connnection_2) != 0);

  /* Call mysql_change_user() with the same username, password, database. */

  DIE_IF(mysql_change_user(&con, opt_user, opt_password,
                           opt_db ? opt_db : "test"));

  /* Retrieve character set information. */

  bug30472_retrieve_charset_info(
      &con, character_set_name_3, character_set_client_3,
      character_set_results_3, collation_connnection_3);

  /* Check that character set information has been reset. */

  DIE_UNLESS(strcmp(character_set_name_1, character_set_name_3) == 0);
  DIE_UNLESS(strcmp(character_set_client_1, character_set_client_3) == 0);
  DIE_UNLESS(strcmp(character_set_results_1, character_set_results_3) == 0);
  DIE_UNLESS(strcmp(collation_connnection_1, collation_connnection_3) == 0);

  /* Change connection-default character set in the client. */

  mysql_options(&con, MYSQL_SET_CHARSET_NAME, "utf8");

  /*
    Call mysql_change_user() in order to check that new connection will
    have UTF8 character set on the client and on the server.
  */

  DIE_IF(mysql_change_user(&con, opt_user, opt_password,
                           opt_db ? opt_db : "test"));

  /* Retrieve character set information. */

  bug30472_retrieve_charset_info(
      &con, character_set_name_4, character_set_client_4,
      character_set_results_4, collation_connnection_4);

  /* Check that we have UTF8 on the server and on the client. */

  DIE_UNLESS(strcmp(character_set_name_4, "utf8") == 0);
  DIE_UNLESS(strcmp(character_set_client_4, "utf8") == 0);
  DIE_UNLESS(strcmp(character_set_results_4, "utf8") == 0);
  DIE_UNLESS(strcmp(collation_connnection_4, "utf8_general_ci") == 0);

  /* That's it. Cleanup. */

  mysql_close(&con);
}

static void bug20023_change_user(MYSQL *con) {
  DIE_IF(
      mysql_change_user(con, opt_user, opt_password, opt_db ? opt_db : "test"));
}

static bool query_str_variable(MYSQL *con, const char *var_name, char *str,
                               size_t len) {
  MYSQL_RES *rs;
  MYSQL_ROW row;

  char query_buffer[MAX_TEST_QUERY_LENGTH];

  bool is_null;

  snprintf(query_buffer, sizeof(query_buffer), "SELECT %s", var_name);

  DIE_IF(mysql_query(con, query_buffer));
  DIE_UNLESS(rs = mysql_store_result(con));
  DIE_UNLESS(row = mysql_fetch_row(rs));

  is_null = row[0] == nullptr;

  if (!is_null) snprintf(str, len, "%s", row[0]);

  mysql_free_result(rs);

  return is_null;
}

static bool query_int_variable(MYSQL *con, const char *var_name,
                               int *var_value) {
  char str[32];
  bool is_null = query_str_variable(con, var_name, str, sizeof(str));

  if (!is_null) *var_value = atoi(str);

  return is_null;
}

static void test_bug20023() {
  MYSQL con;

  int sql_big_selects_orig = 0;
  /*
    Type of max_join_size is ha_rows, which might be ulong or off_t
    depending on the platform or configure options. Preserve the string
    to avoid type overflow pitfalls.
  */
  char max_join_size_orig[32];

  int sql_big_selects_2 = 0;
  int sql_big_selects_3 = 0;
  int sql_big_selects_4 = 0;
  int sql_big_selects_5 = 0;

  char query_buffer[MAX_TEST_QUERY_LENGTH];

  /* Create a new connection. */

  DIE_UNLESS(mysql_client_init(&con));

  DIE_UNLESS(mysql_real_connect(&con, opt_host, opt_user, opt_password,
                                opt_db ? opt_db : "test", opt_port,
                                opt_unix_socket, CLIENT_FOUND_ROWS));

  /***********************************************************************
    Remember original SQL_BIG_SELECTS, MAX_JOIN_SIZE values.
  ***********************************************************************/

  query_int_variable(&con, "@@session.sql_big_selects", &sql_big_selects_orig);

  query_str_variable(&con, "@@global.max_join_size", max_join_size_orig,
                     sizeof(max_join_size_orig));

  /***********************************************************************
    Test that COM_CHANGE_USER resets the SQL_BIG_SELECTS to the initial value.
  ***********************************************************************/

  /* Issue COM_CHANGE_USER. */

  bug20023_change_user(&con);

  /* Query SQL_BIG_SELECTS. */

  query_int_variable(&con, "@@session.sql_big_selects", &sql_big_selects_2);

  /* Check that SQL_BIG_SELECTS is reset properly. */

  DIE_UNLESS(sql_big_selects_orig == sql_big_selects_2);

  /***********************************************************************
    Test that if MAX_JOIN_SIZE set to non-default value,
    SQL_BIG_SELECTS will be 0.
  ***********************************************************************/

  /* Set MAX_JOIN_SIZE to some non-default value. */

  DIE_IF(mysql_query(&con, "SET @@global.max_join_size = 10000"));
  DIE_IF(mysql_query(&con, "SET @@session.max_join_size = default"));

  /* Issue COM_CHANGE_USER. */

  bug20023_change_user(&con);

  /* Query SQL_BIG_SELECTS. */

  query_int_variable(&con, "@@session.sql_big_selects", &sql_big_selects_3);

  /* Check that SQL_BIG_SELECTS is 0. */

  DIE_UNLESS(sql_big_selects_3 == 0);

  /***********************************************************************
    Test that if MAX_JOIN_SIZE set to default value,
    SQL_BIG_SELECTS will be 1.
  ***********************************************************************/

  /* Set MAX_JOIN_SIZE to the default value (2^64-1). */

  DIE_IF(mysql_query(&con,
                     "SET @@global.max_join_size = cast(-1 as unsigned int)"));
  DIE_IF(mysql_query(&con, "SET @@session.max_join_size = default"));

  /* Issue COM_CHANGE_USER. */

  bug20023_change_user(&con);

  /* Query SQL_BIG_SELECTS. */

  query_int_variable(&con, "@@session.sql_big_selects", &sql_big_selects_4);

  /* Check that SQL_BIG_SELECTS is 1. */

  DIE_UNLESS(sql_big_selects_4 == 1);

  /***********************************************************************
    Restore MAX_JOIN_SIZE.
    Check that SQL_BIG_SELECTS will be the original one.
  ***********************************************************************/

  /* Restore MAX_JOIN_SIZE. */

  snprintf(query_buffer, sizeof(query_buffer),
           "SET @@global.max_join_size = %s", max_join_size_orig);

  DIE_IF(mysql_query(&con, query_buffer));

  DIE_IF(mysql_query(&con,
                     "SET @@global.max_join_size = cast(-1 as unsigned int)"));
  DIE_IF(mysql_query(&con, "SET @@session.max_join_size = default"));

  /* Issue COM_CHANGE_USER. */

  bug20023_change_user(&con);

  /* Query SQL_BIG_SELECTS. */

  query_int_variable(&con, "@@session.sql_big_selects", &sql_big_selects_5);

  /* Check that SQL_BIG_SELECTS is 1. */

  DIE_UNLESS(sql_big_selects_5 == sql_big_selects_orig);

  /***********************************************************************
    That's it. Cleanup.
  ***********************************************************************/

  mysql_close(&con);
}

static void bug31418_impl() {
  MYSQL con;

  bool is_null;
  int rc = 0;

  /* Create a new connection. */

  DIE_UNLESS(mysql_client_init(&con));

  DIE_UNLESS(mysql_real_connect(&con, opt_host, opt_user, opt_password,
                                opt_db ? opt_db : "test", opt_port,
                                opt_unix_socket, CLIENT_FOUND_ROWS));

  /***********************************************************************
    Check that lock is free:
      - IS_FREE_LOCK() should return 1;
      - IS_USED_LOCK() should return NULL;
  ***********************************************************************/

  is_null = query_int_variable(&con, "IS_FREE_LOCK('bug31418')", &rc);
  DIE_UNLESS(!is_null && rc);

  is_null = query_int_variable(&con, "IS_USED_LOCK('bug31418')", &rc);
  DIE_UNLESS(is_null);

  /***********************************************************************
    Acquire lock and check the lock status (the lock must be in use):
      - IS_FREE_LOCK() should return 0;
      - IS_USED_LOCK() should return non-zero thread id;
  ***********************************************************************/

  query_int_variable(&con, "GET_LOCK('bug31418', 1)", &rc);
  DIE_UNLESS(rc);

  is_null = query_int_variable(&con, "IS_FREE_LOCK('bug31418')", &rc);
  DIE_UNLESS(!is_null && !rc);

  is_null = query_int_variable(&con, "IS_USED_LOCK('bug31418')", &rc);
  DIE_UNLESS(!is_null && rc);

  /***********************************************************************
    Issue COM_CHANGE_USER command and check the lock status
    (the lock must be free):
      - IS_FREE_LOCK() should return 1;
      - IS_USED_LOCK() should return NULL;
  **********************************************************************/

  bug20023_change_user(&con);

  is_null = query_int_variable(&con, "IS_FREE_LOCK('bug31418')", &rc);
  DIE_UNLESS(!is_null && rc);

  is_null = query_int_variable(&con, "IS_USED_LOCK('bug31418')", &rc);
  DIE_UNLESS(is_null);

  /***********************************************************************
   That's it. Cleanup.
  ***********************************************************************/

  mysql_close(&con);
}

static void test_bug31418() {
  /* Run test case for BUG#31418 for three different connections. */

  bug31418_impl();

  bug31418_impl();

  bug31418_impl();
}

/**
  Bug#31669 Buffer overflow in mysql_change_user()
*/

#define LARGE_BUFFER_SIZE 2048

static void test_bug31669() {
  int rc;
  static char buff[LARGE_BUFFER_SIZE + 1];
  static char user[USERNAME_CHAR_LENGTH + 1];
  static char db[NAME_CHAR_LEN + 1];
  static char query[LARGE_BUFFER_SIZE * 2];
  MYSQL *l_mysql;

  DBUG_TRACE;
  myheader("test_bug31669");

  l_mysql = mysql_client_init(nullptr);
  DIE_UNLESS(l_mysql != nullptr);

  l_mysql = mysql_real_connect(l_mysql, opt_host, opt_user, opt_password,
                               current_db, opt_port, opt_unix_socket, 0);
  DIE_UNLESS(l_mysql != nullptr);

  rc = mysql_change_user(l_mysql, nullptr, nullptr, nullptr);
  DIE_UNLESS(rc);

  reconnect(&l_mysql);

  rc = mysql_change_user(l_mysql, "", "", "");
  DIE_UNLESS(rc);
  reconnect(&l_mysql);

  memset(buff, 'a', sizeof(buff));
  buff[sizeof(buff) - 1] = '\0';

  rc = mysql_change_user(l_mysql, buff, buff, buff);
  DIE_UNLESS(rc);
  reconnect(&l_mysql);

  rc = mysql_change_user(mysql, opt_user, opt_password, current_db);
  DIE_UNLESS(!rc);

  memset(db, 'a', sizeof(db));
  db[NAME_CHAR_LEN] = 0;
  strxmov(query, "CREATE DATABASE IF NOT EXISTS ", db, NullS);
  rc = mysql_query(mysql, query);
  myquery(rc);

  memset(user, 'b', sizeof(user));
  user[USERNAME_CHAR_LENGTH] = 0;
  memset(buff, 'c', sizeof(buff));
  buff[LARGE_BUFFER_SIZE] = 0;

  strxmov(query, "CREATE USER '", user,
          "'@'%' IDENTIFIED WITH 'mysql_native_password' BY '", buff, "'",
          NullS);
  rc = mysql_query(mysql, query);
  myquery(rc);
  strxmov(query, "GRANT ALL PRIVILEGES ON *.* TO '", user,
          "'@'%' WITH GRANT OPTION", NullS);
  rc = mysql_query(mysql, query);
  myquery(rc);

  strxmov(query, "CREATE USER '", user,
          "'@'localhost' IDENTIFIED WITH 'mysql_native_password' BY '", buff,
          "'", NullS);
  rc = mysql_query(mysql, query);
  myquery(rc);
  strxmov(query, "GRANT ALL PRIVILEGES ON *.* TO '", user,
          "'@'localhost' WITH GRANT OPTION", NullS);
  rc = mysql_query(mysql, query);
  myquery(rc);

  rc = mysql_query(mysql, "FLUSH PRIVILEGES");
  myquery(rc);

  rc = mysql_change_user(l_mysql, user, buff, db);
  DIE_UNLESS(!rc);

  user[USERNAME_CHAR_LENGTH - 1] = 'a';
  rc = mysql_change_user(l_mysql, user, buff, db);
  DIE_UNLESS(rc);
  reconnect(&l_mysql);

  user[USERNAME_CHAR_LENGTH - 1] = 'b';
  buff[LARGE_BUFFER_SIZE - 1] = 'd';
  rc = mysql_change_user(l_mysql, user, buff, db);
  DIE_UNLESS(rc);
  reconnect(&l_mysql);

  buff[LARGE_BUFFER_SIZE - 1] = 'c';
  db[NAME_CHAR_LEN - 1] = 'e';
  rc = mysql_change_user(l_mysql, user, buff, db);
  DIE_UNLESS(rc);
  reconnect(&l_mysql);

  db[NAME_CHAR_LEN - 1] = 'a';
  rc = mysql_change_user(l_mysql, user, buff, db);
  DIE_UNLESS(!rc);

  rc = mysql_change_user(l_mysql, user + 1, buff + 1, db + 1);
  DIE_UNLESS(rc);
  reconnect(&l_mysql);

  rc = mysql_change_user(mysql, opt_user, opt_password, current_db);
  DIE_UNLESS(!rc);

  strxmov(query, "DROP DATABASE ", db, NullS);
  rc = mysql_query(mysql, query);
  myquery(rc);

  strxmov(query, "DROP USER '", user, "'@'localhost'", NullS);
  rc = mysql_query(mysql, query);
  myquery(rc);
  DIE_UNLESS(mysql_affected_rows(mysql) == 0);

  strxmov(query, "DROP USER '", user, "'@'%'", NullS);
  rc = mysql_query(mysql, query);
  myquery(rc);
  DIE_UNLESS(mysql_affected_rows(mysql) == 0);

  mysql_close(l_mysql);
}

/**
  Bug#28386 the general log is incomplete
*/

static void test_bug28386() {
  int rc;
  MYSQL_STMT *stmt;
  MYSQL_RES *result;
  MYSQL_ROW row;
  MYSQL_BIND bind;
  const char hello[] = "hello world!";

  DBUG_TRACE;
  myheader("test_bug28386");

  rc = mysql_query(mysql, "select @@global.log_output");
  myquery(rc);

  result = mysql_store_result(mysql);
  DIE_UNLESS(result);

  row = mysql_fetch_row(result);
  if (!strstr(row[0], "TABLE")) {
    mysql_free_result(result);
    if (!opt_silent)
      printf("Skipping the test since logging to tables is not enabled\n");
    /* Log output is not to tables */
    return;
  }
  mysql_free_result(result);

  enable_query_logs(1);

  stmt = mysql_simple_prepare(mysql, "SELECT ?");
  check_stmt(stmt);

  memset(&bind, 0, sizeof(bind));

  bind.buffer_type = MYSQL_TYPE_STRING;
  bind.buffer = const_cast<char *>(hello);
  bind.buffer_length = (ulong)sizeof(hello);

  mysql_stmt_bind_param(stmt, &bind);
  mysql_stmt_send_long_data(stmt, 0, hello, (ulong)sizeof(hello));

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = my_process_stmt_result(stmt);
  DIE_UNLESS(rc == 1);

  rc = mysql_stmt_reset(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_close(stmt);
  DIE_UNLESS(!rc);

  rc = mysql_query(mysql,
                   "select * from mysql.general_log where "
                   "command_type='Close stmt' or "
                   "command_type='Reset stmt' or "
                   "command_type='Long Data'");
  myquery(rc);

  result = mysql_store_result(mysql);
  mytest(result);

  DIE_UNLESS(mysql_num_rows(result) == 3);

  mysql_free_result(result);

  restore_query_logs();
}

static void test_wl4166_1() {
  MYSQL_STMT *stmt;
  int int_data;
  char str_data[50];
  char tiny_data;
  short small_data;
  longlong big_data;
  float real_data;
  double double_data;
  ulong length[7];
  bool is_null[7];
  MYSQL_BIND my_bind[7];
  int rc;
  int i;

  myheader("test_wl4166_1");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS table_4166");
  myquery(rc);

  rc = mysql_query(mysql,
                   "CREATE TABLE table_4166(col1 tinyint NOT NULL, "
                   "col2 varchar(15), col3 int, "
                   "col4 smallint, col5 bigint, "
                   "col6 float, col7 double, "
                   "colX varchar(10) default NULL)");
  myquery(rc);

  stmt = mysql_simple_prepare(
      mysql,
      "INSERT INTO table_4166(col1, col2, col3, col4, col5, col6, col7) "
      "VALUES(?, ?, ?, ?, ?, ?, ?)");
  check_stmt(stmt);

  verify_param_count(stmt, 7);

  memset(my_bind, 0, sizeof(my_bind));
  /* tinyint */
  my_bind[0].buffer_type = MYSQL_TYPE_TINY;
  my_bind[0].buffer = (void *)&tiny_data;
  /* string */
  my_bind[1].buffer_type = MYSQL_TYPE_STRING;
  my_bind[1].buffer = (void *)str_data;
  my_bind[1].buffer_length = 1000; /* Max string length */
  /* integer */
  my_bind[2].buffer_type = MYSQL_TYPE_LONG;
  my_bind[2].buffer = (void *)&int_data;
  /* short */
  my_bind[3].buffer_type = MYSQL_TYPE_SHORT;
  my_bind[3].buffer = (void *)&small_data;
  /* bigint */
  my_bind[4].buffer_type = MYSQL_TYPE_LONGLONG;
  my_bind[4].buffer = (void *)&big_data;
  /* float */
  my_bind[5].buffer_type = MYSQL_TYPE_FLOAT;
  my_bind[5].buffer = (void *)&real_data;
  /* double */
  my_bind[6].buffer_type = MYSQL_TYPE_DOUBLE;
  my_bind[6].buffer = (void *)&double_data;

  for (i = 0; i < (int)array_elements(my_bind); i++) {
    my_bind[i].length = &length[i];
    my_bind[i].is_null = &is_null[i];
    is_null[i] = false;
  }

  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  int_data = 320;
  small_data = 1867;
  big_data = 1000;
  real_data = 2;
  double_data = 6578.001;

  /* now, execute the prepared statement to insert 10 records.. */
  for (tiny_data = 0; tiny_data < 10; tiny_data++) {
    length[1] = sprintf(str_data, "MySQL%d", int_data);
    rc = mysql_stmt_execute(stmt);
    check_execute(stmt, rc);
    int_data += 25;
    small_data += 10;
    big_data += 100;
    real_data += 1;
    double_data += 10.09;
  }

  /* force a re-prepare with some DDL */

  rc = mysql_query(
      mysql,
      "ALTER TABLE table_4166 change colX colX varchar(20) default NULL");
  myquery(rc);

  /*
    execute the prepared statement again,
    without changing the types of parameters already bound.
  */

  for (tiny_data = 50; tiny_data < 60; tiny_data++) {
    length[1] = sprintf(str_data, "MySQL%d", int_data);
    rc = mysql_stmt_execute(stmt);
    check_execute(stmt, rc);
    int_data += 25;
    small_data += 10;
    big_data += 100;
    real_data += 1;
    double_data += 10.09;
  }

  mysql_stmt_close(stmt);

  rc = mysql_query(mysql, "DROP TABLE table_4166");
  myquery(rc);
}

static void test_wl4166_2() {
  MYSQL_STMT *stmt;
  int c_int;
  MYSQL_TIME d_date;
  MYSQL_BIND bind_out[2];
  int rc;

  myheader("test_wl4166_2");

  rc = mysql_query(mysql, "SET SQL_MODE=''");
  myquery(rc);

  rc = mysql_query(mysql, "drop table if exists t1");
  myquery(rc);
  rc = mysql_query(mysql, "create table t1 (c_int int, d_date date)");
  myquery(rc);
  rc = mysql_query(mysql,
                   "insert into t1 (c_int, d_date) values (42, '1948-05-15')");
  myquery(rc);

  stmt = mysql_simple_prepare(mysql, "select * from t1");
  check_stmt(stmt);

  memset(bind_out, 0, sizeof(bind_out));
  bind_out[0].buffer_type = MYSQL_TYPE_LONG;
  bind_out[0].buffer = (void *)&c_int;

  bind_out[1].buffer_type = MYSQL_TYPE_DATE;
  bind_out[1].buffer = (void *)&d_date;

  rc = mysql_stmt_bind_result(stmt, bind_out);
  check_execute(stmt, rc);

  /* int -> varchar transition */

  rc = mysql_query(mysql,
                   "alter table t1 change column c_int c_int varchar(11)");
  myquery(rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  DIE_UNLESS(c_int == 42);
  DIE_UNLESS(d_date.year == 1948);
  DIE_UNLESS(d_date.month == 5);
  DIE_UNLESS(d_date.day == 15);

  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == MYSQL_NO_DATA);

  /* varchar to int retrieval with truncation */

  rc = mysql_query(mysql, "update t1 set c_int='abcde'");
  myquery(rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);
  check_execute_r(stmt, rc);

  DIE_UNLESS(c_int == 0);

  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == MYSQL_NO_DATA);

  /* alter table and increase the number of columns */
  rc = mysql_query(mysql, "alter table t1 add column d_int int");
  myquery(rc);

  rc = mysql_stmt_execute(stmt);
  check_execute_r(stmt, rc);

  rc = mysql_stmt_reset(stmt);
  check_execute(stmt, rc);

  /* decrease the number of columns */
  rc = mysql_query(mysql, "alter table t1 drop d_date, drop d_int");
  myquery(rc);

  rc = mysql_stmt_execute(stmt);
  check_execute_r(stmt, rc);

  mysql_stmt_close(stmt);
  rc = mysql_query(mysql, "drop table t1");
  myquery(rc);
}

/**
  Test how warnings generated during assignment of parameters
  are (currently not) preserve in case of reprepare.
*/

static void test_wl4166_3() {
  int rc;
  MYSQL_STMT *stmt;
  MYSQL_BIND my_bind[1];
  MYSQL_TIME tm[1];

  myheader("test_wl4166_3");

  rc = mysql_query(mysql, "drop table if exists t1");
  myquery(rc);

  rc = mysql_query(mysql, "create table t1 (year datetime)");
  myquery(rc);

  stmt = mysql_simple_prepare(mysql, "insert into t1 (year) values (?)");
  check_stmt(stmt);
  verify_param_count(stmt, 1);

  memset(my_bind, 0, sizeof(my_bind));
  my_bind[0].buffer_type = MYSQL_TYPE_DATETIME;
  my_bind[0].buffer = &tm[0];

  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  tm[0].year = 10000;
  tm[0].month = 1;
  tm[0].day = 1;
  tm[0].hour = 1;
  tm[0].minute = 1;
  tm[0].second = 1;
  tm[0].second_part = 0;
  tm[0].neg = false;
  tm[0].time_type = MYSQL_TIMESTAMP_DATETIME;

  /* Cause a statement reprepare */
  rc = mysql_query(mysql, "alter table t1 add column c int");
  myquery(rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);
  /*
    Sic: only one warning, instead of two. The warning
    about data truncation when assigning a parameter is lost.
    This is a bug.
  */
  my_process_warnings(mysql, 1);

  verify_col_data("t1", "year", "0000-00-00 00:00:00");

  mysql_stmt_close(stmt);

  rc = mysql_query(mysql, "drop table t1");
  myquery(rc);
}

/**
  Test that long data parameters, as well as parameters
  that were originally in a different character set, are
  preserved in case of reprepare.
*/

static void test_wl4166_4() {
  MYSQL_STMT *stmt;
  int rc;
  const char *stmt_text;
  MYSQL_BIND bind_array[2];

  /* Represented as numbers to keep UTF8 tools from clobbering them. */
  const char *koi8 = "\xee\xd5\x2c\x20\xda\xc1\x20\xd2\xd9\xc2\xc1\xcc\xcb\xd5";
  const char *cp1251 =
      "\xcd\xf3\x2c\x20\xe7\xe0\x20\xf0\xfb\xe1\xe0\xeb\xea\xf3";
  char buf1[16], buf2[16];
  ulong buf1_len, buf2_len;

  myheader("test_wl4166_4");

  rc = mysql_query(mysql, "drop table if exists t1");
  myquery(rc);

  /*
    Create table with binary columns, set session character set to cp1251,
    client character set to koi8, and make sure that there is conversion
    on insert and no conversion on select
  */
  rc = mysql_query(mysql,
                   "create table t1 (c1 varbinary(255), c2 varbinary(255))");
  myquery(rc);
  rc = mysql_query(mysql,
                   "set character_set_client=koi8r, "
                   "character_set_connection=cp1251, "
                   "character_set_results=koi8r");
  myquery(rc);

  memset(bind_array, 0, sizeof(bind_array));

  bind_array[0].buffer_type = MYSQL_TYPE_STRING;

  bind_array[1].buffer_type = MYSQL_TYPE_STRING;
  bind_array[1].buffer = const_cast<char *>(koi8);
  bind_array[1].buffer_length = (ulong)strlen(koi8);

  stmt = mysql_stmt_init(mysql);
  check_stmt(stmt);

  stmt_text = "insert into t1 (c1, c2) values (?, ?)";

  rc = mysql_stmt_prepare(stmt, stmt_text, (ulong)strlen(stmt_text));
  check_execute(stmt, rc);

  mysql_stmt_bind_param(stmt, bind_array);

  mysql_stmt_send_long_data(stmt, 0, koi8, (ulong)strlen(koi8));

  /* Cause a reprepare at statement execute */
  rc = mysql_query(mysql, "alter table t1 add column d int");
  myquery(rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  stmt_text = "select c1, c2 from t1";

  /* c1 and c2 are binary so no conversion will be done on select */
  rc = mysql_stmt_prepare(stmt, stmt_text, (ulong)strlen(stmt_text));
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  bind_array[0].buffer = buf1;
  bind_array[0].buffer_length = (ulong)sizeof(buf1);
  bind_array[0].length = &buf1_len;

  bind_array[1].buffer = buf2;
  bind_array[1].buffer_length = (ulong)sizeof(buf2);
  bind_array[1].length = &buf2_len;

  mysql_stmt_bind_result(stmt, bind_array);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  DIE_UNLESS(buf1_len == strlen(cp1251));
  DIE_UNLESS(buf2_len == strlen(cp1251));
  DIE_UNLESS(!memcmp(buf1, cp1251, buf1_len));
  DIE_UNLESS(!memcmp(buf2, cp1251, buf1_len));

  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == MYSQL_NO_DATA);

  mysql_stmt_close(stmt);

  rc = mysql_query(mysql, "drop table t1");
  myquery(rc);
  rc = mysql_query(mysql, "set names default");
  myquery(rc);
}

/**
  Bug#36004 mysql_stmt_prepare resets the list of warnings
*/

static void test_bug36004() {
  int rc, warning_count = 0;
  MYSQL_STMT *stmt;

  DBUG_TRACE;
  myheader("test_bug36004");

  rc = mysql_query(mysql, "drop table if exists inexistant");
  myquery(rc);

  DIE_UNLESS(mysql_warning_count(mysql) == 1);
  query_int_variable(mysql, "@@warning_count", &warning_count);
  DIE_UNLESS(warning_count);

  stmt = mysql_simple_prepare(mysql, "select 1");
  check_stmt(stmt);

  DIE_UNLESS(mysql_warning_count(mysql) == 0);
  query_int_variable(mysql, "@@warning_count", &warning_count);
  /* behaviour changed by WL#5928 */
  if (mysql_get_server_version(mysql) < 50702)
    DIE_UNLESS(warning_count);
  else
    DIE_UNLESS(!warning_count);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  DIE_UNLESS(mysql_warning_count(mysql) == 0);
  mysql_stmt_close(stmt);

  query_int_variable(mysql, "@@warning_count", &warning_count);
  /* behaviour changed by WL#5928 */
  if (mysql_get_server_version(mysql) < 50702)
    DIE_UNLESS(warning_count);
  else
    DIE_UNLESS(!warning_count);

  stmt = mysql_simple_prepare(mysql, "drop table if exists inexistant");
  check_stmt(stmt);

  query_int_variable(mysql, "@@warning_count", &warning_count);
  DIE_UNLESS(warning_count == 0);
  mysql_stmt_close(stmt);
}

/**
  Test that COM_REFRESH issues a implicit commit.
*/

static void test_wl4284_1() {
  int rc;
  MYSQL_ROW row;
  MYSQL_RES *result;

  DBUG_TRACE;
  myheader("test_wl4284_1");

  /* set AUTOCOMMIT to OFF */
  rc = mysql_autocommit(mysql, false);
  myquery(rc);

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS trans");
  myquery(rc);

  rc = mysql_query(mysql, "CREATE TABLE trans (a INT) ENGINE= InnoDB");
  myquery(rc);

  rc = mysql_query(mysql, "INSERT INTO trans VALUES(1)");
  myquery(rc);

  rc = mysql_refresh(mysql, REFRESH_GRANT | REFRESH_TABLES);
  myquery(rc);

  rc = mysql_rollback(mysql);
  myquery(rc);

  rc = mysql_query(mysql, "SELECT * FROM trans");
  myquery(rc);

  result = mysql_use_result(mysql);
  mytest(result);

  row = mysql_fetch_row(result);
  mytest(row);

  mysql_free_result(result);

  /* set AUTOCOMMIT to ON */
  rc = mysql_autocommit(mysql, true);
  myquery(rc);

  rc = mysql_query(mysql, "DROP TABLE trans");
  myquery(rc);
}

static void test_bug38486(void) {
  MYSQL_STMT *stmt;
  const char *stmt_text;
  unsigned long type = CURSOR_TYPE_READ_ONLY;

  DBUG_TRACE;
  myheader("test_bug38486");

  stmt = mysql_stmt_init(mysql);
  mysql_stmt_attr_set(stmt, STMT_ATTR_CURSOR_TYPE, (void *)&type);
  stmt_text = "CREATE TABLE t1 (a INT)";
  mysql_stmt_prepare(stmt, stmt_text, (ulong)strlen(stmt_text));
  mysql_stmt_execute(stmt);
  mysql_stmt_close(stmt);

  stmt = mysql_stmt_init(mysql);
  mysql_stmt_attr_set(stmt, STMT_ATTR_CURSOR_TYPE, (void *)&type);
  stmt_text = "INSERT INTO t1 VALUES (1)";
  mysql_stmt_prepare(stmt, stmt_text, (ulong)strlen(stmt_text));
  mysql_stmt_execute(stmt);
  mysql_stmt_close(stmt);
}

/**
     Bug# 33831 mysql_real_connect() should fail if
     given an already connected MYSQL handle.
*/

static void test_bug33831(void) {
  MYSQL *l_mysql;

  DBUG_TRACE;

  if (!(l_mysql = mysql_client_init(nullptr))) {
    myerror("mysql_client_init() failed");
    DIE_UNLESS(0);
  }
  if (!(mysql_real_connect(l_mysql, opt_host, opt_user, opt_password,
                           current_db, opt_port, opt_unix_socket, 0))) {
    myerror("connection failed");
    DIE_UNLESS(0);
  }

  if (mysql_real_connect(l_mysql, opt_host, opt_user, opt_password, current_db,
                         opt_port, opt_unix_socket, 0)) {
    myerror("connection should have failed");
    DIE_UNLESS(0);
  }

  mysql_close(l_mysql);
}

static void test_bug40365(void) {
  uint rc, i;
  MYSQL_STMT *stmt = nullptr;
  MYSQL_BIND my_bind[2];
  bool is_null[2] = {false};
  MYSQL_TIME tm[2];

  DBUG_TRACE;

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS t1");
  myquery(rc);
  rc = mysql_query(mysql,
                   "CREATE TABLE t1(c1 DATETIME, \
                                          c2 DATE)");
  myquery(rc);

  stmt = mysql_simple_prepare(mysql, "INSERT INTO t1 VALUES(?, ?)");
  check_stmt(stmt);
  verify_param_count(stmt, 2);

  memset(my_bind, 0, sizeof(my_bind));
  my_bind[0].buffer_type = MYSQL_TYPE_DATETIME;
  my_bind[1].buffer_type = MYSQL_TYPE_DATE;
  for (i = 0; i < (uint)array_elements(my_bind); i++) {
    my_bind[i].buffer = (void *)&tm[i];
    my_bind[i].is_null = &is_null[i];
  }

  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);

  for (i = 0; i < array_elements(my_bind); i++) {
    tm[i].time_type = MYSQL_TIMESTAMP_NONE;
    tm[i].neg = false;
    tm[i].second_part = 0;
    tm[i].year = 2009;
    tm[i].month = 2;
    tm[i].day = 29;
    tm[i].hour = 0;
    tm[i].minute = 0;
    tm[i].second = 0;
  }
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = mysql_commit(mysql);
  myquery(rc);
  mysql_stmt_close(stmt);

  stmt = mysql_simple_prepare(mysql, "SELECT * FROM t1");
  check_stmt(stmt);

  rc = mysql_stmt_bind_result(stmt, my_bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_store_result(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  if (!opt_silent) fprintf(stdout, "\n");

  for (i = 0; i < array_elements(my_bind); i++) {
    if (!opt_silent)
      fprintf(stdout, "\ntime[%d]: %02d-%02d-%02d ", i, tm[i].year, tm[i].month,
              tm[i].day);
    DIE_UNLESS(tm[i].year == 0);
    DIE_UNLESS(tm[i].month == 0);
    DIE_UNLESS(tm[i].day == 0);
  }
  mysql_stmt_close(stmt);
  rc = mysql_commit(mysql);
  myquery(rc);
}

/**
  Subtest for Bug#43560. Verifies that a loss of connection on the server side
  is handled well by the mysql_stmt_execute() call, i.e., no SIGSEGV due to
  a vio socket that is cleared upon closed connection.

  Assumes the presence of the close_conn_after_stmt_execute debug feature in
  the server. Verifies that it is connected to a debug server before proceeding
  with the test.
 */
static void test_bug43560(void) {
  MYSQL *conn;
  uint rc;
  MYSQL_STMT *stmt = nullptr;
  MYSQL_BIND bind;
  bool is_null = false;
  char buffer[256];
  const uint BUFSIZE = sizeof(buffer);
  const char *values[] = {"eins", "zwei", "drei", "viele", nullptr};
  const char insert_str[] = "INSERT INTO t1 (c2) VALUES (?)";
  ulong length;
  const unsigned int drop_db = opt_drop_db;

  DBUG_TRACE;
  myheader("test_bug43560");

  /* Make sure we only run against a debug server. */
  if (!strstr(mysql->server_version, "debug")) {
    fprintf(stdout, "Skipping test_bug43560: server not DEBUG version\n");
    return;
  }

  /*
    Set up a separate connection for this test to avoid messing up the
    general MYSQL object used in other subtests. Use TCP protocol to avoid
    problems with the buffer semantics of AF_UNIX, and turn off auto reconnect.
  */
  conn = client_connect(0, MYSQL_PROTOCOL_TCP, false);

  rc = mysql_query(conn, "DROP TABLE IF EXISTS t1");
  myquery(rc);
  rc = mysql_query(
      conn, "CREATE TABLE t1 (c1 INT PRIMARY KEY AUTO_INCREMENT, c2 CHAR(10))");
  myquery(rc);

  stmt = mysql_stmt_init(conn);
  check_stmt(stmt);
  rc = mysql_stmt_prepare(stmt, insert_str, (ulong)strlen(insert_str));
  check_execute(stmt, rc);

  memset(&bind, 0, sizeof(bind));
  bind.buffer_type = MYSQL_TYPE_STRING;
  bind.buffer_length = BUFSIZE;
  bind.buffer = buffer;
  bind.is_null = &is_null;
  bind.length = &length;
  rc = mysql_stmt_bind_param(stmt, &bind);
  check_execute(stmt, rc);

  /* First execute; should succeed. */
  strncpy(buffer, values[0], BUFSIZE);
  length = (ulong)strlen(buffer);
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  /*
    Set up the server to close this session's server-side socket after
    next execution of prep statement.
  */
  rc =
      mysql_query(conn, "SET SESSION debug='+d,close_conn_after_stmt_execute'");
  myquery(rc);

  /* Second execute; should fail due to socket closed during execution. */
  strncpy(buffer, values[1], BUFSIZE);
  length = (ulong)strlen(buffer);
  rc = mysql_stmt_execute(stmt);
  DIE_UNLESS(rc && mysql_stmt_errno(stmt) == CR_SERVER_LOST);

  /*
    Third execute; should fail (connection already closed), or SIGSEGV in
    case of a Bug#43560 type regression in which case the whole test fails.
  */
  strncpy(buffer, values[2], BUFSIZE);
  length = (ulong)strlen(buffer);
  rc = mysql_stmt_execute(stmt);
  DIE_UNLESS(rc && mysql_stmt_errno(stmt) == CR_SERVER_LOST);

  mysql_stmt_close(stmt);

  opt_drop_db = 0;
  client_disconnect(conn);
  rc = mysql_query(mysql, "DROP TABLE t1");
  myquery(rc);
  opt_drop_db = drop_db;
}

/**
  Bug#36326: nested transaction and select
*/

static void test_bug36326() {
  int rc;

  DBUG_TRACE;
  myheader("test_bug36326");

  rc = mysql_autocommit(mysql, true);
  myquery(rc);
  rc = mysql_query(mysql, "DROP TABLE IF EXISTS t1");
  myquery(rc);
  rc = mysql_query(mysql, "CREATE  TABLE t1 (a INTEGER)");
  myquery(rc);
  rc = mysql_query(mysql, "INSERT INTO t1 VALUES (1)");
  myquery(rc);
  DIE_UNLESS(!(mysql->server_status & SERVER_STATUS_IN_TRANS));
  DIE_UNLESS(mysql->server_status & SERVER_STATUS_AUTOCOMMIT);
  rc = mysql_query(mysql, "BEGIN");
  myquery(rc);
  DIE_UNLESS(mysql->server_status & SERVER_STATUS_IN_TRANS);
  rc = mysql_query(mysql, "SELECT * FROM t1");
  myquery(rc);
  rc = my_process_result(mysql);
  DIE_UNLESS(rc == 1);
  rc = mysql_rollback(mysql);
  myquery(rc);
  rc = mysql_query(mysql, "ROLLBACK");
  myquery(rc);
  DIE_UNLESS(!(mysql->server_status & SERVER_STATUS_IN_TRANS));
  rc = mysql_query(mysql, "SELECT * FROM t1");
  myquery(rc);
  DIE_UNLESS(!(mysql->server_status & SERVER_STATUS_IN_TRANS));
  rc = my_process_result(mysql);
  DIE_UNLESS(rc == 1);
  rc = mysql_query(mysql, "DROP TABLE t1");
  myquery(rc);
}

/**
  Bug#41078: With CURSOR_TYPE_READ_ONLY mysql_stmt_fetch() returns short
             string value.
*/

static void test_bug41078(void) {
  uint rc;
  MYSQL_STMT *stmt = nullptr;
  MYSQL_BIND param, result;
  ulong cursor_type = CURSOR_TYPE_READ_ONLY;
  ulong len;
  char str[64];
  const char param_str[] = "abcdefghijklmn";
  bool is_null, error;

  DBUG_TRACE;

  rc = mysql_query(mysql, "SET NAMES UTF8");
  myquery(rc);

  stmt = mysql_simple_prepare(mysql, "SELECT ?");
  check_stmt(stmt);
  verify_param_count(stmt, 1);

  rc = mysql_stmt_attr_set(stmt, STMT_ATTR_CURSOR_TYPE, &cursor_type);
  check_execute(stmt, rc);

  memset(&param, 0, sizeof(param));
  param.buffer_type = MYSQL_TYPE_STRING;
  param.buffer = const_cast<char *>(param_str);
  len = sizeof(param_str) - 1;
  param.length = &len;

  rc = mysql_stmt_bind_param(stmt, &param);
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  memset(&result, 0, sizeof(result));
  result.buffer_type = MYSQL_TYPE_STRING;
  result.buffer = str;
  result.buffer_length = (ulong)sizeof(str);
  result.is_null = &is_null;
  result.length = &len;
  result.error = &error;

  rc = mysql_stmt_bind_result(stmt, &result);
  check_execute(stmt, rc);

  rc = mysql_stmt_store_result(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  DIE_UNLESS(len == sizeof(param_str) - 1 && !strcmp(str, param_str));

  mysql_stmt_close(stmt);
}

/**
  Bug#45010: invalid memory reads during parsing some strange statements
*/
static void test_bug45010() {
  int rc;
  const char query1[] = "select a.\x80", query2[] = "describe `table\xef";

  DBUG_TRACE;
  myheader("test_bug45010");

  rc = mysql_query(mysql, "set names utf8");
  myquery(rc);

  /* \x80 (-128) could be used as a index of ident_map. */
  rc = mysql_real_query(mysql, query1, (ulong)(sizeof(query1) - 1));
  DIE_UNLESS(rc);

  /* \xef (-17) could be used to skip 3 bytes past the buffer end. */
  rc = mysql_real_query(mysql, query2, (ulong)(sizeof(query2) - 1));
  DIE_UNLESS(rc);

  rc = mysql_query(mysql, "set names default");
  myquery(rc);
}

/**
  Bug#44495: Prepared Statement:
             CALL p(<x>) - `thd->get_protocol() == &thd->protocol_text' failed
*/

static void test_bug44495() {
  int rc;
  MYSQL con;
  MYSQL_STMT *stmt;

  DBUG_TRACE;
  myheader("test_44495");

  rc = mysql_query(mysql, "DROP PROCEDURE IF EXISTS p1");
  myquery(rc);

  rc = mysql_query(mysql,
                   "CREATE PROCEDURE p1(IN arg VARCHAR(25))"
                   "  BEGIN SET @stmt = CONCAT('SELECT \"', arg, '\"');"
                   "  PREPARE ps1 FROM @stmt;"
                   "  EXECUTE ps1;"
                   "  DROP PREPARE ps1;"
                   "END;");
  myquery(rc);

  DIE_UNLESS(mysql_client_init(&con));

  DIE_UNLESS(mysql_real_connect(&con, opt_host, opt_user, opt_password,
                                current_db, opt_port, opt_unix_socket,
                                CLIENT_MULTI_RESULTS));

  stmt = mysql_simple_prepare(&con, "CALL p1('abc')");
  check_stmt(stmt);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = my_process_stmt_result(stmt);
  DIE_UNLESS(rc == 1);

  mysql_stmt_close(stmt);

  mysql_close(&con);

  rc = mysql_query(mysql, "DROP PROCEDURE p1");
  myquery(rc);
}

static void test_bug53371() {
  int rc;
  MYSQL_RES *result;

  myheader("test_bug53371");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS t1");
  myquery(rc);
  rc = mysql_query(mysql, "DROP DATABASE IF EXISTS bug53371");
  myquery(rc);
  rc = mysql_query(mysql, "DROP USER 'testbug'@localhost");
  rc = mysql_query(mysql, "CREATE USER 'testbug'@localhost");
  myquery(rc);

  rc = mysql_query(mysql, "CREATE TABLE t1 (a INT)");
  myquery(rc);
  rc = mysql_query(mysql, "CREATE DATABASE bug53371");
  myquery(rc);
  rc = mysql_query(mysql, "GRANT SELECT ON bug53371.* to 'testbug'@localhost");
  myquery(rc);

  rc = mysql_change_user(mysql, "testbug", nullptr, "bug53371");
  myquery(rc);

  rc = mysql_query(mysql, "SHOW COLUMNS FROM client_test_db.t1");
  DIE_UNLESS(rc);
  DIE_UNLESS(mysql_errno(mysql) == 1142);

  result = mysql_list_fields(mysql, "../client_test_db/t1", nullptr);
  DIE_IF(result);

  result = mysql_list_fields(mysql, "#mysql50#/../client_test_db/t1", nullptr);
  DIE_IF(result);

  rc = mysql_change_user(mysql, opt_user, opt_password, current_db);
  myquery(rc);
  rc = mysql_query(mysql, "DROP TABLE t1");
  myquery(rc);
  rc = mysql_query(mysql, "DROP DATABASE bug53371");
  myquery(rc);
  rc = mysql_query(mysql, "DROP USER 'testbug'@localhost");
  myquery(rc);
}

/**
  Bug#42373: libmysql can mess a connection at connect
*/
static void test_bug42373() {
  int rc;
  MYSQL con;
  MYSQL_STMT *stmt;

  DBUG_TRACE;
  myheader("test_42373");

  rc = mysql_query(mysql, "DROP PROCEDURE IF EXISTS p1");
  myquery(rc);

  rc = mysql_query(mysql,
                   "CREATE PROCEDURE p1()"
                   "  BEGIN"
                   "  SELECT 1;"
                   "  INSERT INTO t1 VALUES (2);"
                   "END;");
  myquery(rc);

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS t1");
  myquery(rc);

  rc = mysql_query(mysql, "CREATE TABLE t1 (a INT)");
  myquery(rc);

  /* Try with a stored procedure. */
  DIE_UNLESS(mysql_client_init(&con));

  mysql_options(&con, MYSQL_INIT_COMMAND, "CALL p1()");

  DIE_UNLESS(mysql_real_connect(
      &con, opt_host, opt_user, opt_password, current_db, opt_port,
      opt_unix_socket, CLIENT_MULTI_STATEMENTS | CLIENT_MULTI_RESULTS));

  stmt = mysql_simple_prepare(&con, "SELECT a FROM t1");
  check_stmt(stmt);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = my_process_stmt_result(stmt);
  DIE_UNLESS(rc == 1);

  mysql_stmt_close(stmt);
  mysql_close(&con);

  /* Now try with a multi-statement. */
  DIE_UNLESS(mysql_client_init(&con));

  mysql_options(&con, MYSQL_INIT_COMMAND,
                "SELECT 3; INSERT INTO t1 VALUES (4)");

  DIE_UNLESS(mysql_real_connect(
      &con, opt_host, opt_user, opt_password, current_db, opt_port,
      opt_unix_socket, CLIENT_MULTI_STATEMENTS | CLIENT_MULTI_RESULTS));

  stmt = mysql_simple_prepare(&con, "SELECT a FROM t1");
  check_stmt(stmt);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = my_process_stmt_result(stmt);
  DIE_UNLESS(rc == 2);

  mysql_stmt_close(stmt);
  mysql_close(&con);

  rc = mysql_query(mysql, "DROP TABLE t1");
  myquery(rc);

  rc = mysql_query(mysql, "DROP PROCEDURE p1");
  myquery(rc);
}

/**
  Bug#54041: MySQL 5.0.92 fails when tests from Connector/C suite run
*/

static void test_bug54041_impl() {
  int rc;
  MYSQL_STMT *stmt;
  MYSQL_BIND bind;

  DBUG_TRACE;
  myheader("test_bug54041");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS t1");
  myquery(rc);

  rc = mysql_query(mysql, "CREATE TABLE t1 (a INT)");
  myquery(rc);

  stmt = mysql_simple_prepare(mysql, "SELECT a FROM t1 WHERE a > ?");
  check_stmt(stmt);
  verify_param_count(stmt, 1);

  memset(&bind, 0, sizeof(bind));

  /* Any type that does not support long data handling. */
  bind.buffer_type = MYSQL_TYPE_LONG;

  rc = mysql_stmt_bind_param(stmt, &bind);
  check_execute(stmt, rc);

  /*
    Trick the client API into sending a long data packet for
    the parameter. Long data is only supported for string and
    binary types.
  */
  stmt->params[0].buffer_type = MYSQL_TYPE_STRING;

  rc = mysql_stmt_send_long_data(stmt, 0, "data", 5);
  check_execute(stmt, rc);

  /* Undo API violation. */
  stmt->params[0].buffer_type = MYSQL_TYPE_LONG;

  rc = mysql_stmt_execute(stmt);
  /* Incorrect arguments. */
  check_execute_r(stmt, rc);

  mysql_stmt_close(stmt);

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS t1");
  myquery(rc);
}

/**
  Bug#54041: MySQL 5.0.92 fails when tests from Connector/C suite run
*/

static void test_bug54041() {
  enable_query_logs(0);
  test_bug54041_impl();
  disable_query_logs();
  test_bug54041_impl();
  restore_query_logs();
}

/**
  Bug#47485: mysql_store_result returns a result set for a prepared statement
*/
static void test_bug47485() {
  MYSQL_STMT *stmt;
  MYSQL_RES *res;
  MYSQL_BIND bind[2];
  int rc;
  const char *sql_select = "SELECT 1, 'a'";
  int int_data;
  char str_data[16];
  bool is_null[2];
  bool error[2];
  ulong length[2];

  DBUG_TRACE;
  myheader("test_bug47485");

  stmt = mysql_stmt_init(mysql);
  check_stmt(stmt);
  rc = mysql_stmt_prepare(stmt, sql_select, (ulong)strlen(sql_select));
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  res = mysql_store_result(mysql);
  DIE_UNLESS(res == nullptr);

  mysql_stmt_reset(stmt);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  res = mysql_use_result(mysql);
  DIE_UNLESS(res == nullptr);

  mysql_stmt_reset(stmt);

  memset(bind, 0, sizeof(bind));
  bind[0].buffer_type = MYSQL_TYPE_LONG;
  bind[0].buffer = (char *)&int_data;
  bind[0].is_null = &is_null[0];
  bind[0].length = &length[0];
  bind[0].error = &error[0];

  bind[1].buffer_type = MYSQL_TYPE_STRING;
  bind[1].buffer = (char *)str_data;
  bind[1].buffer_length = (ulong)sizeof(str_data);
  bind[1].is_null = &is_null[1];
  bind[1].length = &length[1];
  bind[1].error = &error[1];

  rc = mysql_stmt_bind_result(stmt, bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  rc = mysql_stmt_store_result(stmt);
  check_execute(stmt, rc);

  while (!(rc = mysql_stmt_fetch(stmt)))
    ;

  DIE_UNLESS(rc == MYSQL_NO_DATA);

  mysql_stmt_reset(stmt);

  memset(bind, 0, sizeof(bind));
  bind[0].buffer_type = MYSQL_TYPE_LONG;
  bind[0].buffer = (char *)&int_data;
  bind[0].is_null = &is_null[0];
  bind[0].length = &length[0];
  bind[0].error = &error[0];

  bind[1].buffer_type = MYSQL_TYPE_STRING;
  bind[1].buffer = (char *)str_data;
  bind[1].buffer_length = (ulong)sizeof(str_data);
  bind[1].is_null = &is_null[1];
  bind[1].length = &length[1];
  bind[1].error = &error[1];

  rc = mysql_stmt_bind_result(stmt, bind);
  check_execute(stmt, rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  while (!(rc = mysql_stmt_fetch(stmt)))
    ;

  DIE_UNLESS(rc == MYSQL_NO_DATA);

  mysql_stmt_close(stmt);
}

/*
  Bug#58036 client utf32, utf16, ucs2 should be disallowed, they crash server
*/
static void test_bug58036() {
  MYSQL *conn;
  enum mysql_ssl_mode ssl_mode = SSL_MODE_DISABLED;
  DBUG_TRACE;
  myheader("test_bug58036");

  /* Part1: try to connect with ucs2 client character set */
  conn = mysql_client_init(nullptr);
  mysql_options(conn, MYSQL_SET_CHARSET_NAME, "ucs2");
  mysql_options(conn, MYSQL_OPT_SSL_MODE, &ssl_mode);
  if (mysql_real_connect(conn, opt_host, opt_user, opt_password,
                         opt_db ? opt_db : "test", opt_port, opt_unix_socket,
                         0)) {
    if (!opt_silent)
      printf("mysql_real_connect() succeeded (failure expected)\n");
    mysql_close(conn);
    DIE("");
  }

  if (!opt_silent)
    printf("Got mysql_real_connect() error (expected): %s (%d)\n",
           mysql_error(conn), mysql_errno(conn));
  DIE_UNLESS(mysql_errno(conn) == ER_WRONG_VALUE_FOR_VAR);
  mysql_close(conn);

  /*
    Part2:
    - connect with latin1
    - then change client character set to ucs2
    - then try mysql_change_user()
  */
  conn = mysql_client_init(nullptr);
  mysql_options(conn, MYSQL_SET_CHARSET_NAME, "latin1");
  if (!mysql_real_connect(conn, opt_host, opt_user, opt_password,
                          opt_db ? opt_db : "test", opt_port, opt_unix_socket,
                          0)) {
    if (!opt_silent)
      printf("mysql_real_connect() failed: %s (%d)\n", mysql_error(conn),
             mysql_errno(conn));
    mysql_close(conn);
    DIE("");
  }

  mysql_options(conn, MYSQL_SET_CHARSET_NAME, "ucs2");
  if (!mysql_change_user(conn, opt_user, opt_password, nullptr)) {
    if (!opt_silent) printf("mysql_change_user() succedded, error expected!");
    mysql_close(conn);
    DIE("");
  }

  if (!opt_silent)
    printf("Got mysql_change_user() error (expected): %s (%d)\n",
           mysql_error(conn), mysql_errno(conn));
  mysql_close(conn);
}

/*
  Bug#49972: Crash in prepared statements.

  The following case lead to a server crash:
    - Use binary protocol;
    - Prepare a statement with OUT-parameter;
    - Execute the statement;
    - Cause re-prepare of the statement (change dependencies);
    - Execute the statement again -- crash here.
*/

static void test_bug49972() {
  int rc;
  MYSQL_STMT *stmt;

  MYSQL_BIND in_param_bind;
  MYSQL_BIND out_param_bind;
  int int_data = 0;
  bool is_null = false;

  DBUG_TRACE;
  myheader("test_bug49972");

  rc = mysql_query(mysql, "DROP FUNCTION IF EXISTS f1");
  myquery(rc);

  rc = mysql_query(mysql, "DROP PROCEDURE IF EXISTS p1");
  myquery(rc);

  rc = mysql_query(mysql, "CREATE FUNCTION f1() RETURNS INT RETURN 1");
  myquery(rc);

  rc = mysql_query(mysql, "CREATE PROCEDURE p1(IN a INT, OUT b INT) SET b = a");
  myquery(rc);

  stmt = mysql_simple_prepare(mysql, "CALL p1((SELECT f1()), ?)");
  check_stmt(stmt);

  memset(&in_param_bind, 0, sizeof(in_param_bind));

  in_param_bind.buffer_type = MYSQL_TYPE_LONG;
  in_param_bind.buffer = (char *)&int_data;
  in_param_bind.length = nullptr;
  in_param_bind.is_null = nullptr;

  rc = mysql_stmt_bind_param(stmt, &in_param_bind);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  {
    memset(&out_param_bind, 0, sizeof(out_param_bind));

    out_param_bind.buffer_type = MYSQL_TYPE_LONG;
    out_param_bind.is_null = &is_null;
    out_param_bind.buffer = &int_data;
    out_param_bind.buffer_length = (ulong)sizeof(int_data);

    rc = mysql_stmt_bind_result(stmt, &out_param_bind);
    check_execute(stmt, rc);

    rc = mysql_stmt_fetch(stmt);
    rc = mysql_stmt_fetch(stmt);
    DBUG_ASSERT(rc == MYSQL_NO_DATA);

    mysql_stmt_next_result(stmt);
    mysql_stmt_fetch(stmt);
  }

  rc = mysql_query(mysql, "DROP FUNCTION f1");
  myquery(rc);

  rc = mysql_query(mysql, "CREATE FUNCTION f1() RETURNS INT RETURN 1");
  myquery(rc);

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  {
    memset(&out_param_bind, 0, sizeof(out_param_bind));

    out_param_bind.buffer_type = MYSQL_TYPE_LONG;
    out_param_bind.is_null = &is_null;
    out_param_bind.buffer = &int_data;
    out_param_bind.buffer_length = (ulong)sizeof(int_data);

    rc = mysql_stmt_bind_result(stmt, &out_param_bind);
    check_execute(stmt, rc);

    rc = mysql_stmt_fetch(stmt);
    rc = mysql_stmt_fetch(stmt);
    DBUG_ASSERT(rc == MYSQL_NO_DATA);

    mysql_stmt_next_result(stmt);
    mysql_stmt_fetch(stmt);
  }

  mysql_stmt_close(stmt);

  rc = mysql_query(mysql, "DROP PROCEDURE p1");
  myquery(rc);

  rc = mysql_query(mysql, "DROP FUNCTION f1");
  myquery(rc);
}

/*
  Bug #56976:   Severe Denial Of Service in prepared statements
*/
static void test_bug56976() {
  MYSQL_STMT *stmt;
  MYSQL_BIND bind[1];
  int rc;
  const char *query = "SELECT LENGTH(?)";
  char *long_buffer;
  unsigned long i, packet_len = 256 * 1024L;
  unsigned long dos_len = 65 * 1024 * 1024L;

  DBUG_TRACE;
  myheader("test_bug56976");

  stmt = mysql_stmt_init(mysql);
  check_stmt(stmt);

  rc = mysql_stmt_prepare(stmt, query, (ulong)strlen(query));
  check_execute(stmt, rc);

  memset(bind, 0, sizeof(bind));
  bind[0].buffer_type = MYSQL_TYPE_TINY_BLOB;

  rc = mysql_stmt_bind_param(stmt, bind);
  check_execute(stmt, rc);

  long_buffer = (char *)my_malloc(PSI_NOT_INSTRUMENTED, packet_len, MYF(0));
  DIE_UNLESS(long_buffer);

  memset(long_buffer, 'a', packet_len);

  for (i = 0; i < dos_len / packet_len; i++) {
    rc = mysql_stmt_send_long_data(stmt, 0, long_buffer, packet_len);
    check_execute(stmt, rc);
  }

  my_free(long_buffer);
  rc = mysql_stmt_execute(stmt);
  DIE_UNLESS(rc && mysql_stmt_errno(stmt) == ER_UNKNOWN_ERROR);

  mysql_stmt_close(stmt);
}

/**
  Bug#57058 SERVER_QUERY_WAS_SLOW not wired up.
*/

static void test_bug57058() {
  MYSQL_RES *res;
  int rc;

  DBUG_TRACE;
  myheader("test_bug57058");

  rc = mysql_query(mysql, "set @@session.long_query_time=0.1");
  myquery(rc);

  DIE_UNLESS(!(mysql->server_status & SERVER_QUERY_WAS_SLOW));

  rc = mysql_query(mysql, "select sleep(1)");
  myquery(rc);

  /*
    Important: the flag is sent in the last EOF packet of
    the query, the one which ends the result. Read the
    result to see the "slow" status.
  */
  res = mysql_store_result(mysql);

  DIE_UNLESS(mysql->server_status & SERVER_QUERY_WAS_SLOW);

  mysql_free_result(res);

  rc = mysql_query(mysql, "set @@session.long_query_time=default");
  myquery(rc);
}

/**
  Bug#11766854: 60075: MYSQL_LOAD_CLIENT_PLUGIN DOESN'T CLEAR ERROR
*/

static void test_bug11766854() {
  struct st_mysql_client_plugin *plugin;

  DBUG_TRACE;
  myheader("test_bug11766854");

  plugin = mysql_load_plugin(mysql, "foo", -1, 0);
  DIE_UNLESS(plugin == nullptr);

  plugin = mysql_load_plugin(mysql, "qa_auth_client", -1, 0);
  DIE_UNLESS(plugin != nullptr);
  DIE_IF(mysql_errno(mysql));
}

/**
  Bug#12337762: 60075: MYSQL_LIST_FIELDS() RETURNS WRONG CHARSET FOR
                       CHAR/VARCHAR/TEXT COLUMNS IN VIEWS
*/
static void test_bug12337762() {
  int rc, i = 0;
  MYSQL_RES *result;
  MYSQL_FIELD *field;
  unsigned int tab_charsetnr[3] = {0};

  DBUG_TRACE;
  myheader("test_bug12337762");

  /*
    Creating table with specific charset.
  */
  rc = mysql_query(mysql, "drop table if exists charset_tab");
  rc = mysql_query(mysql,
                   "create table charset_tab("
                   "txt1 varchar(32) character set Latin1,"
                   "txt2 varchar(32) character set Latin1 collate latin1_bin,"
                   "txt3 varchar(32) character set utf8 collate utf8_bin"
                   ")");

  DIE_UNLESS(rc == 0);
  DIE_IF(mysql_errno(mysql));

  /*
    Creating view from table created earlier.
  */
  rc = mysql_query(mysql, "drop view if exists charset_view");
  rc = mysql_query(mysql,
                   "create view charset_view as "
                   "select * from charset_tab;");
  DIE_UNLESS(rc == 0);
  DIE_IF(mysql_errno(mysql));

  /*
    Checking field information for table.
  */
  result = mysql_list_fields(mysql, "charset_tab", nullptr);
  DIE_IF(mysql_errno(mysql));
  i = 0;
  while ((field = mysql_fetch_field(result))) {
    printf("field name %s\n", field->name);
    printf("field table %s\n", field->table);
    printf("field type %d\n", field->type);
    printf("field charset %d\n", field->charsetnr);
    tab_charsetnr[i++] = field->charsetnr;
    printf("\n");
  }
  mysql_free_result(result);

  /*
    Checking field information for view.
  */
  result = mysql_list_fields(mysql, "charset_view", nullptr);
  DIE_IF(mysql_errno(mysql));
  i = 0;
  while ((field = mysql_fetch_field(result))) {
    printf("field name %s\n", field->name);
    printf("field table %s\n", field->table);
    printf("field type %d\n", field->type);
    printf("field charset %d\n", field->charsetnr);
    printf("\n");
    /*
      charset value for field must be same for both, view and table.
    */
    DIE_UNLESS(field->charsetnr == tab_charsetnr[i++]);
  }
  mysql_free_result(result);
}

/**
  Bug#54790: Use of non-blocking mode for sockets limits performance
*/

static void test_bug54790() {
  int rc;
  MYSQL *lmysql;
  uint timeout = 2;
  enum mysql_ssl_mode ssl_mode = SSL_MODE_DISABLED;

  DBUG_TRACE;
  myheader("test_bug54790");

  lmysql = mysql_client_init(nullptr);
  DIE_UNLESS(lmysql);

  rc = mysql_options(lmysql, MYSQL_OPT_READ_TIMEOUT, &timeout);
  DIE_UNLESS(!rc);

  mysql_options(lmysql, MYSQL_OPT_SSL_MODE, &ssl_mode);
  if (!mysql_real_connect(lmysql, opt_host, opt_user, opt_password,
                          opt_db ? opt_db : "test", opt_port, opt_unix_socket,
                          0)) {
    mysql = lmysql;
    myerror("mysql_real_connect failed");
    mysql_close(lmysql);
    exit(1);
  }

  rc = mysql_query(lmysql, "SELECT SLEEP(100);");
  myquery_r(rc);

  DIE_UNLESS(mysql_errno(lmysql) == CR_NET_READ_INTERRUPTED);

  mysql_close(lmysql);
}

/*
  BUG 11754979 - 46675: ON DUPLICATE KEY UPDATE AND UPDATECOUNT() POSSIBLY WRONG
*/

static void test_bug11754979() {
  MYSQL *conn;
  DBUG_TRACE;

  myheader("test_bug11754979");
  DIE_UNLESS((conn = mysql_client_init(nullptr)));
  DIE_UNLESS(mysql_real_connect(conn, opt_host, opt_user, opt_password,
                                opt_db ? opt_db : "test", opt_port,
                                opt_unix_socket, CLIENT_FOUND_ROWS));
  myquery(mysql_query(conn, "DROP TABLE IF EXISTS t1"));
  myquery(mysql_query(
      conn, "CREATE TABLE t1(id INT, label CHAR(1), PRIMARY KEY(id))"));
  myquery(mysql_query(conn, "INSERT INTO t1(id, label) VALUES (1, 'a')"));
  myquery(mysql_query(conn,
                      "INSERT INTO t1(id, label) VALUES (1, 'a') "
                      "ON DUPLICATE KEY UPDATE id = 4"));
  DIE_UNLESS(mysql_affected_rows(conn) == 2);
  myquery(mysql_query(conn, "DROP TABLE t1"));
  mysql_close(conn);
}

/*
  Bug#13001491: MYSQL_REFRESH CRASHES WHEN STORED ROUTINES ARE RUN CONCURRENTLY.
*/
static void test_bug13001491() {
  int rc;
  char query[MAX_TEST_QUERY_LENGTH];
  MYSQL *c;

  myheader("test_bug13001491");

  snprintf(query, MAX_TEST_QUERY_LENGTH, "CREATE USER mysqltest_u1@%s",
           opt_host ? opt_host : "'localhost'");

  rc = mysql_query(mysql, query);
  myquery(rc);

  snprintf(query, MAX_TEST_QUERY_LENGTH,
           "GRANT ALL PRIVILEGES ON *.* TO mysqltest_u1@%s",
           opt_host ? opt_host : "'localhost'");

  rc = mysql_query(mysql, query);
  myquery(rc);

  snprintf(query, MAX_TEST_QUERY_LENGTH,
           "GRANT RELOAD ON *.* TO mysqltest_u1@%s",
           opt_host ? opt_host : "'localhost'");

  rc = mysql_query(mysql, query);
  myquery(rc);

  c = mysql_client_init(nullptr);

  DIE_UNLESS(mysql_real_connect(
      c, opt_host, "mysqltest_u1", nullptr, current_db, opt_port,
      opt_unix_socket, CLIENT_MULTI_STATEMENTS | CLIENT_MULTI_RESULTS));

  rc = mysql_query(c, "DROP PROCEDURE IF EXISTS p1");
  myquery(rc);

  rc = mysql_query(c,
                   "CREATE PROCEDURE p1() "
                   "BEGIN "
                   " DECLARE CONTINUE HANDLER FOR SQLEXCEPTION BEGIN END; "
                   " SELECT COUNT(*) "
                   " FROM INFORMATION_SCHEMA.PROCESSLIST "
                   " GROUP BY user "
                   " ORDER BY NULL "
                   " INTO @a; "
                   "END");
  myquery(rc);

  rc = mysql_query(c, "CALL p1()");
  myquery(rc);

  mysql_free_result(mysql_store_result(c));

  /* Check that mysql_refresh() succeeds without REFRESH_LOG. */
  rc = mysql_refresh(c, REFRESH_GRANT | REFRESH_TABLES | REFRESH_HOSTS |
                            REFRESH_STATUS | REFRESH_THREADS);
  myquery(rc);

  /*
    Check that mysql_refresh(REFRESH_LOG) does not crash the server even if it
    fails. mysql_refresh(REFRESH_LOG) fails when error log points to unavailable
    location.
  */
  mysql_refresh(c, REFRESH_LOG);

  rc = mysql_query(c, "DROP PROCEDURE p1");
  myquery(rc);

  mysql_close(c);
  c = nullptr;

  snprintf(query, MAX_TEST_QUERY_LENGTH, "DROP USER mysqltest_u1@%s",
           opt_host ? opt_host : "'localhost'");

  rc = mysql_query(mysql, query);
  myquery(rc);
}

/*
  WL#5968: Implement START TRANSACTION READ (WRITE|ONLY);
  Check that the SERVER_STATUS_IN_TRANS_READONLY flag is set properly.
*/
static void test_wl5968() {
  int rc;

  myheader("test_wl5968");

  if (mysql_get_server_version(mysql) < 50600) {
    if (!opt_silent)
      fprintf(stdout,
              "Skipping test_wl5968: "
              "tested feature does not exist in versions before MySQL 5.6\n");
    return;
  }

  rc = mysql_query(mysql, "START TRANSACTION");
  myquery(rc);
  DIE_UNLESS(mysql->server_status & SERVER_STATUS_IN_TRANS);
  DIE_UNLESS(!(mysql->server_status & SERVER_STATUS_IN_TRANS_READONLY));
  rc = mysql_query(mysql, "COMMIT");
  myquery(rc);
  rc = mysql_query(mysql, "START TRANSACTION READ ONLY");
  myquery(rc);
  DIE_UNLESS(mysql->server_status & SERVER_STATUS_IN_TRANS);
  DIE_UNLESS(mysql->server_status & SERVER_STATUS_IN_TRANS_READONLY);
  rc = mysql_query(mysql, "COMMIT");
  myquery(rc);
  DIE_UNLESS(!(mysql->server_status & SERVER_STATUS_IN_TRANS));
  DIE_UNLESS(!(mysql->server_status & SERVER_STATUS_IN_TRANS_READONLY));
  rc = mysql_query(mysql, "START TRANSACTION");
  myquery(rc);
  DIE_UNLESS(mysql->server_status & SERVER_STATUS_IN_TRANS);
  DIE_UNLESS(!(mysql->server_status & SERVER_STATUS_IN_TRANS_READONLY));
  rc = mysql_query(mysql, "COMMIT");
  myquery(rc);
}

/*
  WL#5924: Add connect string processing to mysql
*/
static void test_wl5924() {
  int rc;
  MYSQL *l_mysql;
  MYSQL_RES *res;
  MYSQL_ROW row;

  myheader("test_wl5924");

  if (mysql_get_server_version(mysql) < 50600) {
    if (!opt_silent)
      fprintf(stdout,
              "Skipping test_wl5924: "
              "tested feature does not exist in versions before MySQL 5.6\n");
    return;
  }

  l_mysql = mysql_client_init(nullptr);
  DIE_UNLESS(l_mysql != nullptr);

  /* we want a non-default character set */
  rc = mysql_set_character_set(l_mysql, "cp1251");
  DIE_UNLESS(rc == 0);

  /* put in an attr */
  rc = mysql_options4(l_mysql, MYSQL_OPT_CONNECT_ATTR_ADD, "key1", "value1");
  DIE_UNLESS(rc == 0);

  /* put a second attr */
  rc = mysql_options4(l_mysql, MYSQL_OPT_CONNECT_ATTR_ADD, "key2", "value2");
  DIE_UNLESS(rc == 0);

  /* put the second attr again : should fail */
  rc = mysql_options4(l_mysql, MYSQL_OPT_CONNECT_ATTR_ADD, "key2", "value2");
  DIE_UNLESS(rc != 0);

  /* delete the second attr */
  rc = mysql_options(l_mysql, MYSQL_OPT_CONNECT_ATTR_DELETE, "key2");
  DIE_UNLESS(rc == 0);

  /* put the second attr again : should pass */
  rc = mysql_options4(l_mysql, MYSQL_OPT_CONNECT_ATTR_ADD, "key2", "value2");
  DIE_UNLESS(rc == 0);

  /* full reset */
  rc = mysql_options(l_mysql, MYSQL_OPT_CONNECT_ATTR_RESET, nullptr);
  DIE_UNLESS(rc == 0);

  /* put the second attr again : should pass */
  rc = mysql_options4(l_mysql, MYSQL_OPT_CONNECT_ATTR_ADD, "key2", "value2");
  DIE_UNLESS(rc == 0);

  /* full reset */
  rc = mysql_options(l_mysql, MYSQL_OPT_CONNECT_ATTR_RESET, nullptr);
  DIE_UNLESS(rc == 0);

  /* add a third attr */
  rc = mysql_options4(l_mysql, MYSQL_OPT_CONNECT_ATTR_ADD, "key3", "value3");
  DIE_UNLESS(rc == 0);

  /* add a fourth attr */
  rc = mysql_options4(l_mysql, MYSQL_OPT_CONNECT_ATTR_ADD, "key4", "value4");
  DIE_UNLESS(rc == 0);

  /* add a non-ascii attr */
  /* note : this is Георги, Кодинов in windows-1251 */
  rc = mysql_options4(l_mysql, MYSQL_OPT_CONNECT_ATTR_ADD,
                      "\xc3\xe5\xee\xf0\xe3\xe8",
                      "\xca\xee\xe4\xe8\xed\xee\xe2");
  DIE_UNLESS(rc == 0);

  l_mysql = mysql_real_connect(l_mysql, opt_host, opt_user, opt_password,
                               current_db, opt_port, opt_unix_socket, 0);
  DIE_UNLESS(l_mysql != nullptr);

  rc = mysql_query(l_mysql,
                   "SELECT ATTR_NAME, ATTR_VALUE "
                   " FROM performance_schema.session_account_connect_attrs"
                   " WHERE ATTR_NAME IN ('key1','key2','key3','key4',"
                   "  '\xc3\xe5\xee\xf0\xe3\xe8') AND"
                   "  PROCESSLIST_ID = CONNECTION_ID() ORDER BY ATTR_NAME");
  myquery2(l_mysql, rc);
  res = mysql_use_result(l_mysql);
  DIE_UNLESS(res);

  row = mysql_fetch_row(res);
  DIE_UNLESS(row);
  DIE_UNLESS(0 == strcmp(row[0], "key3"));
  DIE_UNLESS(0 == strcmp(row[1], "value3"));

  row = mysql_fetch_row(res);
  DIE_UNLESS(row);
  DIE_UNLESS(0 == strcmp(row[0], "key4"));
  DIE_UNLESS(0 == strcmp(row[1], "value4"));

  row = mysql_fetch_row(res);
  DIE_UNLESS(row);
  DIE_UNLESS(0 == strcmp(row[0], "\xc3\xe5\xee\xf0\xe3\xe8"));
  DIE_UNLESS(0 == strcmp(row[1], "\xca\xee\xe4\xe8\xed\xee\xe2"));

  mysql_free_result(res);

  l_mysql->reconnect = true;
  rc = mysql_reconnect(l_mysql);
  myquery2(l_mysql, rc);

  mysql_close(l_mysql);
}

/*
  WL#56587: Protocol support for password expiration
*/
static void test_wl6587() {
  int rc;
  MYSQL *l_mysql, *r_mysql;
  bool can;

  myheader("test_wl6587");

  if (mysql_get_server_version(mysql) < 50600) {
    if (!opt_silent)
      fprintf(stdout,
              "Skipping test_wl6587: "
              "tested feature does not exist in versions before MySQL 5.6\n");
    return;
  }

  /* initialize the server user */
  rc = mysql_query(mysql,
                   "CREATE USER wl6587_cli@localhost IDENTIFIED BY 'wl6587'");
  myquery(rc);
  rc = mysql_query(mysql, "ALTER USER wl6587_cli@localhost PASSWORD EXPIRE");
  myquery(rc);

  /* prepare the connection */
  l_mysql = mysql_client_init(nullptr);
  DIE_UNLESS(l_mysql != nullptr);

  /* connect must fail : the flag is off by default */
  r_mysql = mysql_real_connect(l_mysql, opt_host, "wl6587_cli", "wl6587",
                               "test", opt_port, opt_unix_socket, 0);
  DIE_UNLESS(r_mysql == nullptr);
  mysql_close(l_mysql);

  l_mysql = mysql_client_init(nullptr);
  DIE_UNLESS(l_mysql != nullptr);

  /* try the last argument. should work */
  l_mysql = mysql_real_connect(l_mysql, opt_host, "wl6587_cli", "wl6587",
                               "test", opt_port, opt_unix_socket,
                               CLIENT_CAN_HANDLE_EXPIRED_PASSWORDS);
  DIE_UNLESS(l_mysql != nullptr);

  /* must fail : sandbox mode */
  rc = mysql_query(l_mysql, "SELECT USER()");
  myerror2(l_mysql, nullptr);
  DIE_UNLESS(rc != 0);

  mysql_close(l_mysql);

  /* try setting the option */

  l_mysql = mysql_client_init(nullptr);
  DIE_UNLESS(l_mysql != nullptr);

  can = true;
  rc = mysql_options(l_mysql, MYSQL_OPT_CAN_HANDLE_EXPIRED_PASSWORDS, &can);
  DIE_UNLESS(rc == 0);

  l_mysql = mysql_real_connect(l_mysql, opt_host, "wl6587_cli", "wl6587",
                               "test", opt_port, opt_unix_socket, 0);
  DIE_UNLESS(l_mysql != nullptr);

  /* must fail : sandbox mode */
  rc = mysql_query(l_mysql, "SELECT USER()");
  myerror2(l_mysql, nullptr);
  DIE_UNLESS(rc != 0);

  mysql_close(l_mysql);

  /* try change user against an expired account */

  l_mysql = mysql_client_init(nullptr);
  DIE_UNLESS(l_mysql != nullptr);

  can = false;
  rc = mysql_options(l_mysql, MYSQL_OPT_CAN_HANDLE_EXPIRED_PASSWORDS, &can);
  DIE_UNLESS(rc == 0);

  l_mysql = mysql_real_connect(l_mysql, opt_host, opt_user, opt_password,
                               current_db, opt_port, opt_unix_socket, 0);
  DIE_UNLESS(l_mysql != nullptr);

  rc = mysql_change_user(l_mysql, "wl6587_cli", "wl6587", "test");
  DIE_UNLESS(rc == true);

  mysql_close(l_mysql);

  /* cleanup */
  rc = mysql_query(mysql, "DROP USER wl6587_cli@localhost");
  myquery(rc);
}

/*
  Bug #17309863 AUTO RECONNECT DOES NOT WORK WITH 5.6 LIBMYSQLCLIENT
*/
static void test_bug17309863() {
  MYSQL *lmysql;
  unsigned long thread_id;
  char query[MAX_TEST_QUERY_LENGTH];
  int rc;

  myheader("test_bug17309863");

  if (!opt_silent) fprintf(stdout, "\n Establishing a test connection ...");
  if (!(lmysql = mysql_client_init(nullptr))) {
    myerror("mysql_client_init() failed");
    exit(1);
  }
  lmysql->reconnect = true;
  if (!(mysql_real_connect(lmysql, opt_host, opt_user, opt_password, current_db,
                           opt_port, opt_unix_socket, 0))) {
    myerror("connection failed");
    exit(1);
  }
  if (!opt_silent) fprintf(stdout, "OK");

  thread_id = mysql_thread_id(lmysql);
  sprintf(query, "KILL %lu", thread_id);

  /*
    Running the "KILL <thread_id>" query in a separate connection.
  */
  if (thread_query(query)) exit(1);

  /*
    The above KILL statement should have closed our connection. But reconnect
    flag allows to detect this before sending query and re-establish it without
    returning an error.
  */
  rc = mysql_query(lmysql, "SELECT 'bug17309863'");
  myquery(rc);

  mysql_close(lmysql);
}

static void test_wl5928() {
  MYSQL_STMT *stmt;
  int rc;
  MYSQL_RES *result;

  myheader("test_wl5928");

  if (mysql_get_server_version(mysql) < 50702) {
    if (!opt_silent)
      fprintf(stdout,
              "Skipping test_wl5928: "
              "tested feature does not exist in versions before MySQL 5.7.2\n");
    return;
  }

  stmt = mysql_simple_prepare(mysql, "SHOW WARNINGS");
  DIE_UNLESS(stmt == nullptr);
  stmt = mysql_simple_prepare(mysql, "SHOW ERRORS");
  DIE_UNLESS(stmt == nullptr);
  stmt = mysql_simple_prepare(mysql, "SHOW COUNT(*) WARNINGS");
  DIE_UNLESS(stmt == nullptr);
  stmt = mysql_simple_prepare(mysql, "SHOW COUNT(*) ERRORS");
  DIE_UNLESS(stmt == nullptr);
  stmt = mysql_simple_prepare(mysql, "SELECT @@warning_count");
  DIE_UNLESS(stmt == nullptr);
  stmt = mysql_simple_prepare(mysql, "SELECT @@error_count");
  DIE_UNLESS(stmt == nullptr);
  stmt = mysql_simple_prepare(mysql, "GET DIAGNOSTICS");
  DIE_UNLESS(stmt == nullptr);

  rc = mysql_query(mysql, "SET SQL_MODE=''");
  myquery(rc);

  /* PREPARE */

  stmt = mysql_simple_prepare(mysql, "CREATE TABLE t1 (f1 INT) ENGINE=UNKNOWN");
  DIE_UNLESS(mysql_warning_count(mysql) == 2);
  check_stmt(stmt);

  /* SHOW WARNINGS.  (Will keep diagnostics) */
  rc = mysql_query(mysql, "SHOW WARNINGS");
  myquery(rc);
  result = mysql_store_result(mysql);
  mytest(result);
  rc = my_process_result_set(result);
  DIE_UNLESS(rc == 2);
  mysql_free_result(result);

  /* EXEC */
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);
  DIE_UNLESS(mysql_warning_count(mysql) == 0);

  /* SHOW WARNINGS.  (Will keep diagnostics) */
  rc = mysql_query(mysql, "SHOW WARNINGS");
  myquery(rc);
  result = mysql_store_result(mysql);
  mytest(result);
  rc = my_process_result_set(result);
  DIE_UNLESS(rc == 0);
  mysql_free_result(result);

  /* clean up */
  mysql_stmt_close(stmt);

  stmt = mysql_simple_prepare(mysql, "SELECT 1");
  check_stmt(stmt);
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);
  mysql_stmt_close(stmt);

  myquery(rc);
}

static void test_wl6797() {
  MYSQL_STMT *stmt;
  int rc;
  const char *stmt_text;
  uint64_t res;

  myheader("test_wl6797");

  if (mysql_get_server_version(mysql) < 50703) {
    if (!opt_silent)
      fprintf(stdout,
              "Skipping test_wl6797: "
              "tested feature does not exist in versions before MySQL 5.7.3\n");
    return;
  }
  /* clean up the session */
  rc = mysql_reset_connection(mysql);
  DIE_UNLESS(rc == 0);

  /* do prepare of a query */
  mysql_query(mysql, "use test");
  mysql_query(mysql, "DROP TABLE IF EXISTS t1");
  mysql_query(mysql, "CREATE TABLE t1 (a int)");

  stmt = mysql_stmt_init(mysql);
  stmt_text = "INSERT INTO t1 VALUES (1), (2)";

  rc = mysql_stmt_prepare(stmt, stmt_text, (ulong)strlen(stmt_text));
  check_execute(stmt, rc);

  /* Execute the insert statement */
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  /*
   clean the session this should remove the prepare statement
   from the cache.
  */
  rc = mysql_reset_connection(mysql);
  DIE_UNLESS(rc == 0);

  /* this below stmt should report error */
  rc = mysql_stmt_execute(stmt);
  DIE_IF(rc == 0);

  /*
   bug#17653288: MYSQL_RESET_CONNECTION DOES NOT RESET LAST_INSERT_ID
  */

  rc = mysql_query(mysql,
                   "CREATE TABLE t2 (a int NOT NULL PRIMARY KEY"
                   " auto_increment)");
  myquery(rc);
  rc = mysql_query(mysql, "INSERT INTO t2 VALUES (null)");
  myquery(rc);
  res = mysql_insert_id(mysql);
  DIE_UNLESS(res == 1);
  rc = mysql_reset_connection(mysql);
  DIE_UNLESS(rc == 0);
  res = mysql_insert_id(mysql);
  DIE_UNLESS(res == 0);

  rc = mysql_query(mysql, "INSERT INTO t2 VALUES (last_insert_id(100))");
  myquery(rc);
  res = mysql_insert_id(mysql);
  DIE_UNLESS(res == 100);
  rc = mysql_reset_connection(mysql);
  DIE_UNLESS(rc == 0);
  res = mysql_insert_id(mysql);
  DIE_UNLESS(res == 0);

  mysql_query(mysql, "DROP TABLE IF EXISTS t1");
  mysql_query(mysql, "DROP TABLE IF EXISTS t2");
  mysql_stmt_close(stmt);
}

static void test_wl6791() {
  int rc;
  uint idx;
  MYSQL *l_mysql;
  enum mysql_option
      uint_opts[] = {MYSQL_OPT_CONNECT_TIMEOUT, MYSQL_OPT_READ_TIMEOUT,
                     MYSQL_OPT_WRITE_TIMEOUT,   MYSQL_OPT_PROTOCOL,
                     MYSQL_OPT_LOCAL_INFILE,    MYSQL_OPT_SSL_MODE,
                     MYSQL_OPT_TOS},
      bool_opts[] = {MYSQL_OPT_COMPRESS,
                     MYSQL_REPORT_DATA_TRUNCATION,
                     MYSQL_OPT_RECONNECT,
                     MYSQL_ENABLE_CLEARTEXT_PLUGIN,
                     MYSQL_OPT_CAN_HANDLE_EXPIRED_PASSWORDS,
                     MYSQL_OPT_OPTIONAL_RESULTSET_METADATA},
      const_char_opts[] =
  { MYSQL_READ_DEFAULT_FILE,
    MYSQL_READ_DEFAULT_GROUP,
    MYSQL_SET_CHARSET_DIR,
    MYSQL_SET_CHARSET_NAME,
#if defined(_WIN32)
    /* mysql_options() is a no-op on non-supporting platforms. */
    MYSQL_SHARED_MEMORY_BASE_NAME,
#endif
    MYSQL_OPT_BIND,
    MYSQL_PLUGIN_DIR,
    MYSQL_DEFAULT_AUTH,
    MYSQL_OPT_SSL_KEY,
    MYSQL_OPT_SSL_CERT,
    MYSQL_OPT_SSL_CA,
    MYSQL_OPT_SSL_CAPATH,
    MYSQL_OPT_SSL_CIPHER,
    MYSQL_OPT_TLS_CIPHERSUITES,
    MYSQL_OPT_TLS_SNI_SERVERNAME,
    MYSQL_OPT_SSL_CRL,
    MYSQL_OPT_SSL_CRLPATH,
    MYSQL_SERVER_PUBLIC_KEY },
      err_opts[] = {MYSQL_OPT_NAMED_PIPE, MYSQL_OPT_CONNECT_ATTR_RESET,
                    MYSQL_OPT_CONNECT_ATTR_DELETE, MYSQL_INIT_COMMAND},
      pointer_opts[] = {
#ifdef HAVE_OPENSSL
          MYSQL_OPT_TLS_CERT_CALLBACK, MYSQL_OPT_TLS_CERT_CALLBACK_CONTEXT
#endif
      };

  myheader("test_wl6791");

  /* prepare the connection */
  l_mysql = mysql_client_init(nullptr);
  DIE_UNLESS(l_mysql != nullptr);

  for (idx = 0; idx < sizeof(uint_opts) / sizeof(enum mysql_option); idx++) {
    uint opt_before = 1, opt_after = 0;

    if (!opt_silent)
      fprintf(stdout, "testing uint option #%d (%d)\n", idx,
              (int)uint_opts[idx]);
    rc = mysql_options(l_mysql, uint_opts[idx], &opt_before);
    DIE_UNLESS(rc == 0);

    rc = mysql_get_option(l_mysql, uint_opts[idx], &opt_after);
    DIE_UNLESS(rc == 0);

    DIE_UNLESS(opt_before == opt_after);
  }

  for (idx = 0; idx < sizeof(bool_opts) / sizeof(enum mysql_option); idx++) {
    bool opt_before = true, opt_after = false;

    if (!opt_silent)
      fprintf(stdout, "testing bool option #%d (%d)\n", idx,
              (int)bool_opts[idx]);

    rc = mysql_options(l_mysql, bool_opts[idx], &opt_before);
    DIE_UNLESS(rc == 0);

    rc = mysql_get_option(l_mysql, bool_opts[idx], &opt_after);
    DIE_UNLESS(rc == 0);

    DIE_UNLESS(opt_before == opt_after);
  }

  for (idx = 0; idx < sizeof(const_char_opts) / sizeof(enum mysql_option);
       idx++) {
    const char *opt_before = "TEST", *opt_after = nullptr;

    if (!opt_silent)
      fprintf(stdout, "testing const char * option #%d (%d)\n", idx,
              (int)const_char_opts[idx]);

    rc = mysql_options(l_mysql, const_char_opts[idx], opt_before);
    DIE_UNLESS(rc == 0);

    rc = mysql_get_option(l_mysql, const_char_opts[idx], &opt_after);
    DIE_UNLESS(rc == 0);

    DIE_UNLESS(opt_before && opt_after && 0 == strcmp(opt_before, opt_after));
  }

  for (idx = 0; idx < sizeof(err_opts) / sizeof(enum mysql_option); idx++) {
    void *dummy_arg;
    if (!opt_silent)
      fprintf(stdout, "testing invalid option #%d (%d)\n", idx,
              (int)err_opts[idx]);

    rc = mysql_get_option(l_mysql, err_opts[idx], &dummy_arg);
    DIE_UNLESS(rc != 0);
  }

  for (idx = 0; idx < sizeof(pointer_opts) / sizeof(enum mysql_option); idx++) {
    void *opt_before = reinterpret_cast<void *>(0x123456), *opt_after = NULL;

    if (!opt_silent)
      fprintf(stdout, "testing uint option #%d (%d)\n", idx,
              static_cast<int>(pointer_opts[idx]));
    rc = mysql_options(l_mysql, pointer_opts[idx], &opt_before);
    DIE_UNLESS(rc == 0);

    rc = mysql_get_option(l_mysql, pointer_opts[idx], &opt_after);
    DIE_UNLESS(rc == 0);

    DIE_UNLESS(opt_before == opt_after);
  }

  /* clean up */
  mysql_close(l_mysql);
}

#define QUERY_PREPARED_STATEMENTS_INSTANCES_TABLE                    \
  rc = mysql_query(                                                  \
      mysql,                                                         \
      "SELECT STATEMENT_NAME, SQL_TEXT, COUNT_REPREPARE,"            \
      " COUNT_EXECUTE, OWNER_OBJECT_TYPE, OWNER_OBJECT_SCHEMA, "     \
      "OWNER_OBJECT_NAME"                                            \
      " from performance_schema.prepared_statements_instances where" \
      " sql_text like \"%ps_t1%\"");                                 \
  myquery(rc);                                                       \
  result = mysql_store_result(mysql);                                \
  mytest(result);                                                    \
  (void)my_process_result_set(result);                               \
  mysql_free_result(result);

static void test_wl5768() {
  MYSQL_RES *result;
  MYSQL_STMT *stmt, *sp_stmt;
  MYSQL_BIND bind[1];
  long int_data;
  int rc;

  myheader("test_wl5768");

  if (mysql_get_server_version(mysql) < 50704) {
    if (!opt_silent)
      fprintf(stdout,
              "Skipping test_wl5768: "
              "tested feature does not exist in versions before MySQL 5.7.4\n");
    return;
  }

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS ps_t1");
  myquery(rc);

  rc = mysql_query(mysql, "CREATE TABLE ps_t1(Id INT)");
  myquery(rc);

  // Prepare an insert statement.
  stmt = mysql_simple_prepare(mysql, "INSERT INTO ps_t1 VALUES(?)");
  check_stmt(stmt);
  verify_param_count(stmt, 1);

  // Query P_S table.
  QUERY_PREPARED_STATEMENTS_INSTANCES_TABLE;

  memset(bind, 0, sizeof(bind));
  bind[0].buffer_type = MYSQL_TYPE_LONG;
  bind[0].buffer = (long *)&int_data;
  bind[0].length = nullptr;
  bind[0].is_null = nullptr;
  rc = mysql_stmt_bind_param(stmt, bind);
  check_execute(stmt, rc);

  // Set the data to be inserted.
  int_data = 25;

  // Execute the prepared statement.
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  // Query P_S table.
  QUERY_PREPARED_STATEMENTS_INSTANCES_TABLE;

  // execute the prepared statement for 3 more times to check COUNT_EXECUTE
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  int_data = 74;

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  int_data = 123;

  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  // Query P_S table.
  QUERY_PREPARED_STATEMENTS_INSTANCES_TABLE;

  // Deallocate/Close the prepared statement.
  mysql_stmt_close(stmt);

  rc = mysql_query(mysql, "DROP PROCEDURE IF EXISTS proc");
  myquery(rc);

  // Check the instrumentation of the statement prepared in a stored procedure
  rc = mysql_query(
      mysql,
      "CREATE PROCEDURE proc(IN a INT)"
      "BEGIN"
      "  SET @stmt = CONCAT('UPDATE ps_t1 SET Id = ? WHERE Id > 100');"
      "  PREPARE st FROM @stmt;"
      "  EXECUTE st USING @a;"
      "  DEALLOCATE PREPARE st;"
      "END;");
  myquery(rc);

  sp_stmt = mysql_simple_prepare(mysql, "CALL proc(?)");
  check_stmt(sp_stmt);
  verify_param_count(sp_stmt, 1);

  memset(bind, 0, sizeof(bind));
  bind[0].buffer_type = MYSQL_TYPE_LONG;
  bind[0].buffer = (long *)&int_data;
  bind[0].length = nullptr;
  bind[0].is_null = nullptr;
  rc = mysql_stmt_bind_param(sp_stmt, bind);
  check_execute(sp_stmt, rc);

  int_data = 100;

  // Execute the prepared statement.
  rc = mysql_stmt_execute(sp_stmt);
  check_execute(sp_stmt, rc);

  // Query P_S table.
  QUERY_PREPARED_STATEMENTS_INSTANCES_TABLE;

  // Deallocate/Close the prepared statement.
  mysql_stmt_close(sp_stmt);

  rc = mysql_query(mysql, "DROP PROCEDURE IF EXISTS proc");
  myquery(rc);

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS ps_t1");
  myquery(rc);
}

/**
   BUG#17512527: LIST HANDLING INCORRECT IN MYSQL_PRUNE_STMT_LIST()
*/
static void test_bug17512527() {
  MYSQL *conn1, *conn2;
  MYSQL_STMT *stmt1, *stmt2;
  const char *stmt1_txt = "SELECT NOW();";
  const char *stmt2_txt = "SELECT 1;";
  unsigned long thread_id;
  char query[MAX_TEST_QUERY_LENGTH];
  int rc;

  conn1 = client_connect(0, MYSQL_PROTOCOL_DEFAULT, true);
  conn2 = client_connect(0, MYSQL_PROTOCOL_DEFAULT, false);

  stmt1 = mysql_stmt_init(conn1);
  check_stmt(stmt1);
  rc = mysql_stmt_prepare(stmt1, stmt1_txt, strlen(stmt1_txt));
  check_execute(stmt1, rc);

  thread_id = mysql_thread_id(conn1);
  sprintf(query, "KILL %lu", thread_id);
  if (thread_query(query)) exit(1);

  /*
    After the connection is killed, the connection is
    re-established due to the reconnect flag.
  */
  stmt2 = mysql_stmt_init(conn1);
  check_stmt(stmt2);

  rc = mysql_stmt_prepare(stmt2, stmt2_txt, strlen(stmt2_txt));
  check_execute(stmt1, rc);

  mysql_stmt_close(stmt2);
  mysql_stmt_close(stmt1);

  mysql_close(conn1);
  mysql_close(conn2);
}

/**
   WL#8016: Parser for optimizer hints
*/
static void test_wl8016() {
  MYSQL_RES *result;
  int rc;

  myheader("test_wl8016");

  rc = mysql_query(mysql, "SELECT /*+ ");
  DIE_UNLESS(rc);

  rc = mysql_query(mysql, "SELECT /*+ ICP(`test");
  DIE_UNLESS(rc);

  rc = mysql_query(mysql, "SELECT /*+ ICP(`test*/ 1");
  myquery(rc);

  rc = mysql_query(mysql, "SELECT /*+ ICP(`test*/`*/ 1");
  DIE_UNLESS(rc);

  /* get the result */
  result = mysql_store_result(mysql);
  mytest(result);

  (void)my_process_result_set(result);
  mysql_free_result(result);
}

struct execute_test_query {
  const char *create;
  const char *select;
  const char *drop;
};

/**
  test_bug20645725 helper function
*/
static void execute_and_test(struct execute_test_query *query, char quote,
                             int result, const char *string,
                             const char *expected, bool recursive) {
  MYSQL_STMT *stmt;
  const char *stmt_text;
  int rc;
  MYSQL_BIND my_bind[1];
  char query_buffer[100];
  char param_buffer[50];
  char buff[50];
  ulong length;

  sprintf(param_buffer, "%c%s%c", quote, string, quote);
  sprintf(query_buffer, query->create, param_buffer);

  rc = mysql_real_query(mysql, query_buffer, (ulong)strlen(query_buffer));
  DIE_UNLESS(rc == result);
  if (result != 0) return;
  myquery(rc);

  stmt = mysql_stmt_init(mysql);

  memset(my_bind, 0, sizeof(my_bind));
  my_bind[0].buffer = buff;
  my_bind[0].length = &length;
  my_bind[0].buffer_length = (ulong)sizeof(buff);
  my_bind[0].buffer_type = MYSQL_TYPE_STRING;

  mysql_stmt_bind_param(stmt, my_bind);

  stmt_text = query->select;
  rc = mysql_stmt_prepare(stmt, stmt_text, (ulong)strlen(stmt_text));
  check_execute(stmt, rc);
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  mysql_stmt_bind_result(stmt, my_bind);

  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == 0);
  DIE_UNLESS(length == (ulong)strlen(expected));
  DIE_UNLESS(strcmp(buff, expected) == 0);
  rc = mysql_stmt_fetch(stmt);
  DIE_UNLESS(rc == MYSQL_NO_DATA);

  mysql_stmt_close(stmt);

  sprintf(query_buffer, query->drop, param_buffer);

  rc = mysql_real_query(mysql, query_buffer, (ulong)strlen(query_buffer));
  myquery(rc);

  if (recursive != 0) {
    length = mysql_real_escape_string_quote(mysql, param_buffer, expected,
                                            (ulong)strlen(expected), quote);
    DIE_UNLESS(length != (ulong)-1);

    execute_and_test(query, quote, result, param_buffer, expected, false);
  }
}

/**
  BUG#20645725 GRAVE ACCENT CHARACTER (`) IS NOT FOLLOWED WITH BACKSLASH
               WHEN ESCAPING IT
*/
static void test_bug20645725() {
  const char *stmt_text;
  const char *modes[2];
  struct execute_test_query query;
  int rc;
  int i;

  myheader("test_bug20645725");

  stmt_text = "DROP DATABASE IF EXISTS supertest";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);

  stmt_text = "CREATE DATABASE supertest";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);

  stmt_text = "USE supertest";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);

  stmt_text = "DROP TABLE IF EXISTS t1";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);
  stmt_text = "CREATE TABLE t1 (a TEXT)";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);

  modes[0] = "SET sql_mode=''";
  modes[1] = "SET sql_mode='ANSI_QUOTES'";

  query.create = "INSERT INTO t1 VALUES(%s)";
  query.drop = "DELETE FROM t1; -- %s";
  query.select = "SELECT a FROM t1";

  for (i = 0; i < (int)(sizeof(modes) / sizeof(modes[0])); i++) {
    rc = mysql_real_query(mysql, modes[i], (ulong)strlen(modes[i]));
    myquery(rc);

    execute_and_test(&query, '\'', 0, "aaa", "aaa", true);
    execute_and_test(&query, '\'', 0, "a\\'", "a'", true);
    execute_and_test(&query, '\'', 0, "''", "'", true);
    execute_and_test(&query, '\'', 0, "a''", "a'", true);
    execute_and_test(&query, '\'', 0, "a''b", "a'b", true);
    execute_and_test(&query, '\'', 0, "a\\'''\\'", "a'''", true);
    execute_and_test(&query, '\'', 0, "a\\`", "a`", true);
    execute_and_test(&query, '\'', 0, "a\\n", "a\n", true);
    execute_and_test(&query, '\'', 0, "a``", "a``", true);
    execute_and_test(&query, '\'', 0, "a\\``\\`", "a```", true);
    execute_and_test(&query, '\'', 0, "b\"", "b\"", true);
    execute_and_test(&query, '\'', 0, "b\"\"", "b\"\"", true);
    execute_and_test(&query, '\'', 0, "b\"\"", "b\"\"", true);
    execute_and_test(&query, '\'', 0, "b\\$", "b$", true);
    execute_and_test(&query, '\'', 0, "b\\\\\"", "b\\\"", true);
    execute_and_test(&query, '\'', 0, "b\\\"\"", "b\"\"", true);
    execute_and_test(&query, '"', i, "b\\\"\"\"", "b\"\"", true);
    execute_and_test(&query, '"', i, "d\\'e", "d'e", true);
    execute_and_test(&query, '`', 1, "", "", false);
  }

  modes[0] = "SET sql_mode='NO_BACKSLASH_ESCAPES'";
  modes[1] = "SET sql_mode='NO_BACKSLASH_ESCAPES,ANSI_QUOTES'";

  for (i = 0; i < (int)(sizeof(modes) / sizeof(modes[0])); i++) {
    rc = mysql_real_query(mysql, modes[i], (ulong)strlen(modes[i]));
    myquery(rc);

    execute_and_test(&query, '\'', 0, "aaa", "aaa", true);
    execute_and_test(&query, '\'', 1, "a\\'", "", false);
    execute_and_test(&query, '\'', 0, "''", "'", true);
    execute_and_test(&query, '\'', 0, "a''", "a'", true);
    execute_and_test(&query, '\'', 0, "a''b", "a'b", true);
    execute_and_test(&query, '\'', 1, "a\\'''\\'", "", false);
    execute_and_test(&query, '\'', 0, "a\\`", "a\\`", true);
    execute_and_test(&query, '\'', 0, "a\\n", "a\\n", true);
    execute_and_test(&query, '\'', 0, "a``", "a``", true);
    execute_and_test(&query, '\'', 0, "a\\``\\`", "a\\``\\`", true);
    execute_and_test(&query, '\'', 0, "b\"", "b\"", true);
    execute_and_test(&query, '\'', 0, "b\"\"", "b\"\"", true);
    execute_and_test(&query, '\'', 0, "b\\$", "b\\$", true);
    execute_and_test(&query, '\'', 0, "b\\\\\"", "b\\\\\"", true);
    execute_and_test(&query, '\'', 0, "b\\\"\"", "b\\\"\"", true);
    execute_and_test(&query, '"', 1, "b\\\"\"\"", "", false);
    execute_and_test(&query, '"', i, "d\\'e", "d\\'e", true);
    execute_and_test(&query, '`', 1, "", "", true);
  }

  stmt_text = "DROP TABLE t1";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);

  stmt_text = "SET sql_mode=''";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);

  query.create = "CREATE TABLE %s (a INT)";
  query.drop = "DROP TABLE %s";
  query.select = "SHOW TABLES";

  execute_and_test(&query, '`', 0, "ccc", "ccc", true);
  execute_and_test(&query, '`', 0, "c``cc", "c`cc", true);
  execute_and_test(&query, '`', 0, "c'cc", "c'cc", true);
  execute_and_test(&query, '`', 0, "c''cc", "c''cc", true);
  execute_and_test(&query, '`', 1, "c\\`cc", "", false);
  execute_and_test(&query, '`', 0, "c\"cc", "c\"cc", true);
  execute_and_test(&query, '`', 0, "c\\\"cc", "c\\\"cc", true);
  execute_and_test(&query, '`', 0, "c\"\"cc", "c\"\"cc", true);

  stmt_text = "SET sql_mode='ANSI_QUOTES'";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);

  execute_and_test(&query, '"', 0, "a\"\"a", "a\"a", false);
  execute_and_test(&query, '"', 1, "a\\\"b", "", false);
  execute_and_test(&query, '"', 0, "c\\'cc", "c\\'cc", false);

  modes[0] = "SET sql_mode='NO_BACKSLASH_ESCAPES'";
  modes[1] = "SET sql_mode='NO_BACKSLASH_ESCAPES,ANSI_QUOTES'";

  for (i = 0; i < (int)(sizeof(modes) / sizeof(modes[0])); i++) {
    rc = mysql_real_query(mysql, modes[i], (ulong)strlen(modes[i]));
    myquery(rc);

    execute_and_test(&query, '`', 0, "ccc", "ccc", true);
    execute_and_test(&query, '`', 0, "c``cc", "c`cc", true);
    execute_and_test(&query, '`', 0, "c'cc", "c'cc", true);
    execute_and_test(&query, '`', 0, "c''cc", "c''cc", true);
    execute_and_test(&query, '`', 1, "c\\`cc", "", false);
    execute_and_test(&query, '`', 0, "c\"cc", "c\"cc", true);
    execute_and_test(&query, '`', 0, "c\\\"cc", "c\\\"cc", true);
    execute_and_test(&query, '`', 0, "c\"\"cc", "c\"\"cc", true);
  }

  stmt_text = "DROP DATABASE supertest";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);
}

/**
   BUG#19894382 - SERVER SIDE PREPARED STATEMENTS LEADS TO POTENTIAL
   OFF-BY-SECOND TIMESTAMP ON SLAVE
*/
static void test_bug19894382() {
  MYSQL_STMT *stmt1;
  const char *stmt1_txt =
      "INSERT INTO client_test_db.bug19894382 VALUES"
      " ('master', ?, ?, ?, ?, ?, ?);";
  bool is_null = false;
  MYSQL_BIND bind_val[6];
  MYSQL_TIME tm[6];
  MYSQL_TIME tm_common;
  ulong length = sizeof(MYSQL_TIME);
  int ind;
  int rc;

  myheader("test_bug19894382");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS client_test_db.bug19894382;");
  myquery(rc);
  rc = mysql_query(mysql,
                   "CREATE TABLE client_test_db.bug19894382(f1 CHAR(64),"
                   " f2 TIME, f3 TIMESTAMP NULL, f4 DATETIME,"
                   " f5 TIME(3), f6 TIMESTAMP(3) NULL,"
                   " f7 DATETIME(3));");
  myquery(rc);

  stmt1 = mysql_stmt_init(mysql);
  check_stmt(stmt1);

  // Prepare statement
  rc = mysql_stmt_prepare(stmt1, stmt1_txt, strlen(stmt1_txt));
  check_execute(stmt1, rc);

  // Prepare and bind values.
  tm_common.year = 2015;
  tm_common.month = 4;
  tm_common.day = 24;
  tm_common.hour = 7;
  tm_common.minute = 30;
  tm_common.second = 30;
  tm_common.second_part = 5010;
  tm_common.neg = false;
  tm_common.time_type = MYSQL_TIMESTAMP_NONE;

  memset(bind_val, 0, sizeof(MYSQL_BIND) * 6);
  for (ind = 0; ind < 6; ind++) {
    tm[ind] = tm_common;
    bind_val[ind].buffer = (void *)&tm[ind];
    bind_val[ind].is_null = &is_null;
    bind_val[ind].length = &length;
    bind_val[ind].buffer_length = sizeof(MYSQL_TIME);
    switch (ind % 3) {
      case 0:
        tm[ind].year = tm[ind].month = tm[ind].day = 0;
        bind_val[ind].buffer_type = MYSQL_TYPE_TIME;
        tm[ind].time_type = MYSQL_TIMESTAMP_TIME;
        break;
      case 1:
        bind_val[ind].buffer_type = MYSQL_TYPE_TIMESTAMP;
        tm[ind].time_type = MYSQL_TIMESTAMP_DATETIME;
        break;
      case 2:
        bind_val[ind].buffer_type = MYSQL_TYPE_DATETIME;
        tm[ind].time_type = MYSQL_TIMESTAMP_DATETIME;
        break;
    }
  }
  rc = mysql_stmt_bind_param(stmt1, bind_val);
  check_execute(stmt1, rc);

  /* Execute the insert statement */
  rc = mysql_stmt_execute(stmt1);
  check_execute(stmt1, rc);

  for (ind = 0; ind < 6; ind++) {
    tm[ind].second_part = 501900;
  }
  /* Execute the insert statement */
  rc = mysql_stmt_execute(stmt1);
  check_execute(stmt1, rc);

  rc = mysql_commit(mysql);
  myquery(rc);

  mysql_stmt_close(stmt1);
}

/**
  Bug#20444737  STRING::CHOP ASSERTS ON NAUGHTY TABLE NAMES
*/
static void test_bug20444737() {
  char query[MAX_TEST_QUERY_LENGTH];
  FILE *test_file;
  char *master_test_filename;
  ulong length;
  int rc;
  const char *test_dir = getenv("MYSQL_TEST_DIR");
  const char db_query[] = "USE client_test_db";

  myheader("Test_bug20444737");
  master_test_filename = (char *)malloc(
      strlen(test_dir) + strlen("/std_data/bug20444737.sql") + 1);
  strxmov(master_test_filename, test_dir, "/std_data/bug20444737.sql", NullS);
  if (!opt_silent) fprintf(stdout, "Opening '%s'\n", master_test_filename);
  test_file =
      my_fopen(master_test_filename, (int)(O_RDONLY | MY_FOPEN_BINARY), MYF(0));
  if (test_file == nullptr) {
    fprintf(stderr, "Error in opening file");
    free(master_test_filename);
    DIE("File open error");
  } else if (fgets(query, MAX_TEST_QUERY_LENGTH, test_file) == nullptr) {
    free(master_test_filename);
    /* If fgets returned NULL, it indicates either error or EOF */
    if (feof(test_file)) DIE("Found EOF before all statements were found");

    fprintf(stderr, "Got error %d while reading from file\n",
            ferror(test_file));
    DIE("Read error");
  }

  rc = mysql_real_query(mysql, db_query, (ulong)strlen(db_query));
  myquery(rc);
  length = (ulong)strlen(query);
  fprintf(stdout, "Query is %s\n", query);
  rc = mysql_real_query(mysql, query, (ulong)length);
  myquery(rc);

  free(master_test_filename);
  my_fclose(test_file, MYF(0));
}

/**
  Bug#21104470 WL8132:ASSERTION `! IS_SET()' FAILED.
*/
static void test_bug21104470() {
  MYSQL_RES *result;
  int rc;

  myheader("test_bug21104470");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS t1");
  myquery(rc);

  rc = mysql_query(mysql, "CREATE TABLE t1(j1 JSON, j2 JSON NOT NULL)");
  myquery(rc);

  /* This call used to crash the server. */
  result = mysql_list_fields(mysql, "t1", nullptr);
  mytest(result);

  rc = my_process_result_set(result);
  DIE_UNLESS(rc == 0);

  verify_prepare_field(result, 0, "j1", "j1", MYSQL_TYPE_JSON, "t1", "t1",
                       current_db, UINT_MAX32, 0);

  verify_prepare_field(result, 1, "j2", "j2", MYSQL_TYPE_JSON, "t1", "t1",
                       current_db, UINT_MAX32, 0);

  mysql_free_result(result);
  myquery(mysql_query(mysql, "DROP TABLE t1"));
}

/**
  Bug#21293012 ASSERT `!IS_NULL()' FAILED AT FIELD_JSON::VAL_JSON
  ON NEW CONN TO DB WITH VIEW
*/
static void test_bug21293012() {
  MYSQL_RES *result;
  int rc;

  myheader("test_bug21293012");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS t1");
  myquery(rc);

  rc = mysql_query(mysql, "CREATE TABLE t1(j1 JSON, j2 JSON NOT NULL)");
  myquery(rc);

  rc = mysql_query(mysql, "CREATE VIEW v1 AS SELECT * FROM t1");
  myquery(rc);

  /* This call used to crash the server. */
  result = mysql_list_fields(mysql, "v1", nullptr);
  mytest(result);

  rc = my_process_result_set(result);
  DIE_UNLESS(rc == 0);

  verify_prepare_field(result, 0, "j1", "j1", MYSQL_TYPE_JSON, "v1", "v1",
                       current_db, UINT_MAX32, 0);

  verify_prepare_field(result, 1, "j2", "j2", MYSQL_TYPE_JSON, "v1", "v1",
                       current_db, UINT_MAX32, 0);

  mysql_free_result(result);
  myquery(mysql_query(mysql, "DROP VIEW v1"));
  myquery(mysql_query(mysql, "DROP TABLE t1"));
}

static void test_bug21199582() {
  int rc = 0;
  int recCnt[] = {3, 4, 1};
  int i = 0;
  char query[512] = {0};
  MYSQL_BIND in_param_bind;
  MYSQL_BIND out_param_bind2;
  int in_data = 0;
  char cout_data[100] = {0};
  int rows_number = 0;
  int num_fields = 0;
  MYSQL_RES *result = nullptr;
  MYSQL_STMT *stmt;
  unsigned long cur_type = CURSOR_TYPE_READ_ONLY;
  int iLoop = 0;

  myheader("test_bug21199582");

  for (; i < 2; ++i) {
    iLoop = 0;

    myquery(mysql_query(mysql, "drop procedure if exists p3"));
    myquery(mysql_query(mysql, "drop table if exists abcd"));
    myquery(mysql_query(mysql, "create table abcd(id int,c char(4))"));
    myquery(
        mysql_query(mysql, "insert into abcd values(1,'a'),(2,'b'),(3,'c')"));

    if (i) {
      sprintf(query,
              "create procedure p3(INOUT p1 Integer) BEGIN select c from "
              "abcd;insert into abcd values(4,'d');"
              "select id from abcd;SET NAMES 'utf8';set autocommit = ON;SET p1 "
              "= 9999;END");
    } else {
      sprintf(query,
              "create procedure p3(INOUT p1 Integer) BEGIN select id from "
              "abcd;insert into abcd values(4,'d');"
              "select id from abcd;SET NAMES 'utf8';set autocommit = ON;SET p1 "
              "= 9999;END");
    }

    rc = mysql_query(mysql, query);
    DIE_UNLESS(rc == 0);

    stmt = mysql_stmt_init(mysql);
    DIE_UNLESS(stmt);

    rc = mysql_stmt_attr_set(stmt, STMT_ATTR_CURSOR_TYPE, (void *)&cur_type);
    DIE_UNLESS(rc == 0);

    sprintf(query, "call p3(?)");
    rc = mysql_stmt_prepare(stmt, query, strlen(query));
    DIE_UNLESS(rc == 0);

    memset(&in_param_bind, 0, sizeof(in_param_bind));
    memset(&out_param_bind2, 0, sizeof(out_param_bind2));
    in_data = 0;
    in_param_bind.buffer_type = MYSQL_TYPE_LONG;
    in_param_bind.buffer = (char *)&in_data;
    in_param_bind.length = nullptr;
    in_param_bind.is_null = nullptr;

    out_param_bind2.buffer_type = MYSQL_TYPE_STRING;
    out_param_bind2.buffer = (char *)cout_data;
    out_param_bind2.length = nullptr;
    out_param_bind2.is_null = nullptr;
    out_param_bind2.buffer_length = (ulong)sizeof(cout_data);

    rc = mysql_stmt_bind_param(stmt, &in_param_bind);
    check_execute(stmt, rc);

    rc = mysql_stmt_execute(stmt);
    check_execute(stmt, rc);

    rows_number = 0;

    do {
      num_fields = mysql_stmt_field_count(stmt);

      if (num_fields > 0) {
        memset(cout_data, 0, sizeof(cout_data));
        result = mysql_stmt_result_metadata(stmt);
        if (result) {
          rows_number = mysql_stmt_num_rows(stmt);
          DIE_UNLESS(rows_number == recCnt[iLoop]);

          rc = mysql_stmt_bind_result(stmt, &out_param_bind2);
          check_execute(stmt, rc);

          while (!mysql_stmt_fetch(stmt))
            ;

          mysql_free_result(result);
        }
      }
      rc = mysql_stmt_next_result(stmt);
      ++iLoop;
    } while (rc == 0);

    rc = mysql_stmt_free_result(stmt);
    DIE_UNLESS(rc == 0);

    mysql_stmt_close(stmt);
  }
}

/**
  Bug#20821550 ADD MYSQL_GET_PARAMETERS FUNCTIONALITY TO MYSQL_GET_OPTION()
*/
static void test_bug20821550() {
  MYSQL *mysql_ptr = nullptr;
  int ret_val;
  ulong max_allowed_packet_value = 8192 * 2;
  ulong net_buffer_length_value = 8192 * 4;
  ulong save_max_allowed_packet_value = 0;
  ulong save_net_buffer_length_value = 0;
  ulong ret_max_allowed_packet_value = 0;
  ulong ret_net_buffer_length_value = 0;

  myheader("test_bug20821550");

  /* Try setting/getting values without MYSQL object */

  /* Fetch default values and validate */
  ret_val = mysql_get_option(mysql_ptr, MYSQL_OPT_MAX_ALLOWED_PACKET,
                             &save_max_allowed_packet_value);
  DIE_UNLESS(ret_val == 0);
  DIE_UNLESS(save_max_allowed_packet_value == 1024L * 1024L * 1024L);

  ret_val = mysql_get_option(mysql_ptr, MYSQL_OPT_NET_BUFFER_LENGTH,
                             &save_net_buffer_length_value);
  DIE_UNLESS(ret_val == 0);
  DIE_UNLESS(save_net_buffer_length_value == 8192);

  /* Now set global values */
  ret_val = mysql_options(mysql_ptr, MYSQL_OPT_MAX_ALLOWED_PACKET,
                          &max_allowed_packet_value);
  DIE_UNLESS(ret_val == 0);

  ret_val = mysql_options(mysql_ptr, MYSQL_OPT_NET_BUFFER_LENGTH,
                          &net_buffer_length_value);
  DIE_UNLESS(ret_val == 0);

  /* Check that global values are set */
  ret_val = mysql_get_option(mysql_ptr, MYSQL_OPT_MAX_ALLOWED_PACKET,
                             &ret_max_allowed_packet_value);
  DIE_UNLESS(ret_val == 0);
  DIE_UNLESS(ret_max_allowed_packet_value == max_allowed_packet_value);

  ret_val = mysql_get_option(mysql_ptr, MYSQL_OPT_NET_BUFFER_LENGTH,
                             &ret_net_buffer_length_value);
  DIE_UNLESS(ret_val == 0);
  DIE_UNLESS(ret_net_buffer_length_value == net_buffer_length_value);

  /* Restore default values */
  ret_val = mysql_options(mysql_ptr, MYSQL_OPT_MAX_ALLOWED_PACKET,
                          &save_max_allowed_packet_value);
  DIE_UNLESS(ret_val == 0);

  ret_val = mysql_options(mysql_ptr, MYSQL_OPT_NET_BUFFER_LENGTH,
                          &save_net_buffer_length_value);
  DIE_UNLESS(ret_val == 0);

  /* Create MYSQL object */

  if (!(mysql_ptr = mysql_client_init(nullptr))) {
    myerror("mysql_client_init() failed");
    exit(1);
  }

  /* Now try setting/getting values with MYSQL object */

  mysql_ptr = mysql_real_connect(mysql_ptr, opt_host, opt_user, opt_password,
                                 "test", opt_port, opt_unix_socket, 0);

  /* By default, session values of max_allowed_packet is 0 */
  ret_val = mysql_get_option(mysql_ptr, MYSQL_OPT_MAX_ALLOWED_PACKET,
                             &ret_max_allowed_packet_value);
  DIE_UNLESS(ret_val == 0);
  DIE_UNLESS(ret_max_allowed_packet_value == 0);

  ret_val = mysql_get_option(mysql_ptr, MYSQL_OPT_NET_BUFFER_LENGTH,
                             &ret_net_buffer_length_value);
  DIE_UNLESS(ret_val == 0);
  DIE_UNLESS(ret_net_buffer_length_value == save_net_buffer_length_value);

  DIE_UNLESS(mysql_ptr->net.max_packet_size == save_max_allowed_packet_value);

  mysql_close(mysql_ptr);

  if (!(mysql_ptr = mysql_client_init(nullptr))) {
    myerror("mysql_client_init() failed");
    exit(1);
  }

  /* Set session value for max_allowed_packet */
  ret_val = mysql_options(mysql_ptr, MYSQL_OPT_MAX_ALLOWED_PACKET,
                          &max_allowed_packet_value);
  DIE_UNLESS(ret_val == 0);

  /* net_buffer_length only has global value */
  ret_val = mysql_options(mysql_ptr, MYSQL_OPT_NET_BUFFER_LENGTH,
                          &net_buffer_length_value);
  DIE_UNLESS(ret_val == 0);

  /* Get session value of max_allowed_packet */
  ret_val = mysql_get_option(mysql_ptr, MYSQL_OPT_MAX_ALLOWED_PACKET,
                             &ret_max_allowed_packet_value);
  DIE_UNLESS(ret_val == 0);
  DIE_UNLESS(ret_max_allowed_packet_value == max_allowed_packet_value);

  ret_val = mysql_get_option(mysql_ptr, MYSQL_OPT_NET_BUFFER_LENGTH,
                             &ret_net_buffer_length_value);
  DIE_UNLESS(ret_val == 0);
  DIE_UNLESS(ret_net_buffer_length_value == net_buffer_length_value);

  /* mysql_real_connect() prefers session value of max_allowed_packet */
  mysql_ptr = mysql_real_connect(mysql_ptr, opt_host, opt_user, opt_password,
                                 "test", opt_port, opt_unix_socket, 0);

  /* Session value is used for net.max_allowed_size */
  DIE_UNLESS(mysql_ptr->net.max_packet_size == max_allowed_packet_value);

  /* Get global value of max_allowed_packet */
  ret_val = mysql_get_option(nullptr, MYSQL_OPT_MAX_ALLOWED_PACKET,
                             &ret_max_allowed_packet_value);
  DIE_UNLESS(ret_val == 0);
  DIE_UNLESS(ret_max_allowed_packet_value == save_max_allowed_packet_value);

  mysql_close(mysql_ptr);
}

static void check_warning(MYSQL *conn) {
  MYSQL_RES *result;
  int rc;

  rc = mysql_query(conn, "SHOW WARNINGS");
  myquery(rc);
  result = mysql_store_result(conn);
  mytest(result);
  rc = my_process_result_set(result);
  DIE_UNLESS(rc == 1);
  mysql_free_result(result);
}

static void test_wl8754() {
  MYSQL_RES *res;
  MYSQL *conn;
  int rc;
  unsigned long thread_id;
  const char *stmt_text;

  myheader("test_wl8754");

  /* Check that mysql_list_fields reports deprecated warning. */
  rc = mysql_query(mysql, "DROP TABLE IF EXISTS t1");
  myquery(rc);
  stmt_text = "CREATE TABLE t1 (a int, b char(255), c decimal)";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);

  res = mysql_list_fields(mysql, "t1", "%");
  mysql_free_result(res);

  check_warning(mysql);

  stmt_text = "DROP TABLE t1";
  rc = mysql_real_query(mysql, stmt_text, (ulong)strlen(stmt_text));
  myquery(rc);

  /* Check that mysql_refresh() reports deprecated warning. */
  rc = mysql_refresh(mysql, REFRESH_TABLES);
  myquery(rc);

  check_warning(mysql);

  /* Run a dummy query to clear diagnostics. */
  rc = mysql_query(mysql, "SELECT 1");
  myquery(rc);
  /* Get the result. */
  res = mysql_store_result(mysql);
  mytest(res);
  (void)my_process_result_set(res);
  mysql_free_result(res);

  /* Check that mysql_list_processes() reports deprecated warning. */
  res = mysql_list_processes(mysql);
  mysql_free_result(res);

  check_warning(mysql);

  /* Check that mysql_kill() reports deprecated warning. */
  if (!(conn = mysql_client_init(nullptr))) {
    myerror("mysql_client_init() failed");
    exit(1);
  }
  conn->reconnect = true;
  if (!(mysql_real_connect(conn, opt_host, opt_user, opt_password, current_db,
                           opt_port, opt_unix_socket, 0))) {
    myerror("connection failed");
    exit(1);
  }
  thread_id = mysql_thread_id(conn);
  /*
    Kill connection would have killed the existing connection which clears
    the THD state and reconnects with a new THD thus there will be no
    warnings.
  */
  mysql_kill(conn, (unsigned long)thread_id);
  mysql_close(conn);
}

/*
  BUG#17883203: MYSQL EMBEDDED MYSQL_STMT_EXECUTE RETURN
                "MALFORMED COMMUNICATION PACKET" ERROR
*/
#define BUG17883203_STRING_SIZE 100

static void test_bug17883203() {
  MYSQL_STMT *stmt;
  MYSQL_BIND bind;
  char str_data[BUG17883203_STRING_SIZE];
  bool is_null;
  bool error;
  unsigned long length;
  const char stmt_text[] = "SELECT VERSION()";
  int rc;

  myheader("test_bug17883203");

  stmt = mysql_stmt_init(mysql);
  check_stmt(stmt);
  rc = mysql_stmt_prepare(stmt, stmt_text, strlen(stmt_text));
  check_execute(stmt, rc);
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);
  memset(&bind, 0, sizeof(bind));

  bind.buffer_type = MYSQL_TYPE_STRING;
  bind.buffer = (char *)str_data;
  bind.buffer_length = BUG17883203_STRING_SIZE;
  bind.is_null = &is_null;
  bind.length = &length;
  bind.error = &error;

  rc = mysql_stmt_bind_result(stmt, &bind);
  check_execute(stmt, rc);
  rc = mysql_stmt_fetch(stmt);
  check_execute(stmt, rc);

  if (!opt_silent) {
    fprintf(stdout, "\n Version: %s", str_data);
  }
  mysql_stmt_close(stmt);
}

/**
  BUG#22336527: MYSQL_REAL_CONNECT CAN FAIL WITH SYSTEM ERROR: 4
*/

static void test_bug22336527() {
  int rc;
  MYSQL *l_mysql;
  uint opt_before = 10;
  uint opt_after = 0;

  myheader("test_bug22336527");

  /* prepare the connection */
  l_mysql = mysql_client_init(nullptr);
  DIE_UNLESS(l_mysql != nullptr);

  rc = mysql_options(l_mysql, MYSQL_OPT_RETRY_COUNT, &opt_before);
  DIE_UNLESS(rc == 0);

  rc = mysql_get_option(l_mysql, MYSQL_OPT_RETRY_COUNT, &opt_after);
  DIE_UNLESS(rc == 0);

  DIE_UNLESS(opt_before == opt_after);

  /* clean up */
  mysql_close(l_mysql);
}

/**
  Bug#24963580 INFORMATION_SCHEMA:MDL_REQUEST::INIT_WITH_SOURCE
*/
static void test_bug24963580() {
  MYSQL_RES *result;
  int rc;

  myheader("test_bug24963580");

  rc = mysql_query(mysql, "USE information_schema");
  myquery(rc);

  /* This call used to crash the server. */
  result = mysql_list_fields(mysql, "CHARACTER_SETS", nullptr);
  mytest(result);

  rc = my_process_result_set(result);
  DIE_UNLESS(rc == 0);

  mysql_free_result(result);
}

static int test_mysql_binlog_perform(MYSQL *mysql1, const char *binlog_name,
                                     ulong start_position, uint server_id,
                                     uint flags, size_t gtid_set_size,
                                     uchar *gtid_set) {
  MYSQL_RPL rpl;

  if (!opt_silent) {
    fprintf(stdout, "Perform binlog read: '%s' from %lu, flags=%u\n",
            binlog_name, start_position, flags);
  }

  rpl.file_name_length = 0;
  rpl.file_name = binlog_name;
  rpl.start_position = start_position;
  rpl.server_id = server_id;
  rpl.flags = flags;
  rpl.gtid_set_encoded_size = gtid_set_size;
  rpl.gtid_set_arg = gtid_set;
  rpl.fix_gtid_set = nullptr;

  if (mysql_binlog_open(mysql1, &rpl)) {
    if (!opt_silent)
      fprintf(stdout, "mysql_binlog_open() failed: '%s'\n",
              mysql_error(mysql1));
    mysql_binlog_close(mysql1, &rpl);
    return 1;
  }

  for (;;) {
    if (mysql_binlog_fetch(mysql1, &rpl))  // Error
    {
      if (!opt_silent)
        fprintf(stdout, "Error packet: '%s'\n", mysql_error(mysql1));
      mysql_binlog_close(mysql1, &rpl);
      return 2;
    }

    if (rpl.size == 0)  // EOF
    {
      if (!opt_silent) fprintf(stdout, "EOF packet.\n");
      break;
    }

    /* Normal packet. */
    if (!opt_silent) fprintf(stdout, "%lu byte length packet.\n", rpl.size);
  }

  mysql_binlog_close(mysql1, &rpl);

  return 0;
}

static void test_mysql_binlog() {
  int rc;
  MYSQL *mysql1, *mysql2;
  MYSQL_RPL rpl1, rpl2;
  const char *binlog_name = "mysql-binlog.000001";
  bool reconnect = true;

  myheader("test_mysql_binlog");

  /* Check if binary logging enabled. */
  {
    MYSQL_RES *res;
    MYSQL_ROW row;
    DIE_IF(mysql_query(mysql, "SHOW MASTER STATUS"));
    DIE_UNLESS(res = mysql_store_result(mysql));
    if (!(row = mysql_fetch_row(res)) || strcmp(row[0], binlog_name)) {
      if (!opt_silent) fprintf(stdout, "Skipping test_mysql_binlog\n");
      mysql_free_result(res);
      return;
    }
    mysql_free_result(res);
  }

  rc = mysql_query(mysql,
                   "SET @save_global_binlog_checksum=@@global.binlog_checksum");
  myquery(rc);
  rc = mysql_query(mysql, "SET @@global.binlog_checksum=NONE");
  myquery(rc);
  rc = mysql_query(mysql, "RESET MASTER");
  myquery(rc);
  rc = mysql_query(mysql, "CREATE TABLE t1(a INT)");
  myquery(rc);
  rc = mysql_query(mysql, "INSERT INTO t1 VALUES(0)");
  myquery(rc);
  rc = mysql_query(mysql, "DROP TABLE t1");
  myquery(rc);
  rc = mysql_query(mysql, "FLUSH LOGS");
  myquery(rc);

  if (!(mysql1 = mysql_client_init(nullptr))) {
    myerror("mysql_client_init() failed");
    DIE_UNLESS(0);
  }

  /* binlog_open without being connected */
  if (!opt_silent) fprintf(stdout, "binlog_open without being connected\n");
  DIE_UNLESS(-1 == mysql_binlog_open(mysql1, &rpl1));
  if (!opt_silent)
    fprintf(stdout, "binlog_open error: %s\n", mysql_error(mysql1));
  /* binlog_close on without being connected */
  if (!opt_silent) fprintf(stdout, "binlog_close without being connected\n");
  mysql_binlog_close(mysql1, &rpl1);

  mysql_options(mysql1, MYSQL_OPT_RECONNECT, &reconnect);
  mysql1 = mysql_real_connect(mysql1, opt_host, opt_user, opt_password,
                              current_db, opt_port, opt_unix_socket, 0);
  if (!mysql1) {
    if (!opt_silent)
      fprintf(stdout, "mysql_real_connect() failed: '%s'\n",
              mysql_error(mysql1));
    DIE_UNLESS(0);
  }

#define BIN_LOG_HEADER_SIZE 4U

  rc = test_mysql_binlog_perform(mysql1, nullptr, BIN_LOG_HEADER_SIZE, 0, 0, 0,
                                 nullptr);
  DIE_UNLESS(rc == 0);
  rc = test_mysql_binlog_perform(mysql1, "", BIN_LOG_HEADER_SIZE, 0, 0, 0,
                                 nullptr);
  DIE_UNLESS(rc == 0);
  rc = test_mysql_binlog_perform(mysql1, binlog_name, BIN_LOG_HEADER_SIZE, 0, 0,
                                 0, nullptr);
  DIE_UNLESS(rc == 0);
  rc = test_mysql_binlog_perform(mysql1, binlog_name, BIN_LOG_HEADER_SIZE, 0,
                                 MYSQL_RPL_GTID, 0, nullptr);
  DIE_UNLESS(rc == 0);
  rc = test_mysql_binlog_perform(mysql1, binlog_name, BIN_LOG_HEADER_SIZE, 0,
                                 MYSQL_RPL_GTID | MYSQL_RPL_SKIP_HEARTBEAT, 0,
                                 nullptr);
  DIE_UNLESS(rc == 0);

  /* Incorrect start position. */
  rc = test_mysql_binlog_perform(mysql1, binlog_name, BIN_LOG_HEADER_SIZE - 1,
                                 0, 0, 0, nullptr);
  DIE_UNLESS(rc == 2 &&
             mysql_errno(mysql1) == ER_MASTER_FATAL_ERROR_READING_BINLOG);
  rc = test_mysql_binlog_perform(mysql1, binlog_name, BIN_LOG_HEADER_SIZE + 1,
                                 0, 0, 0, nullptr);
  DIE_UNLESS(rc == 2 &&
             mysql_errno(mysql1) == ER_MASTER_FATAL_ERROR_READING_BINLOG);

  /* Non-existing binlog file. */
  rc = test_mysql_binlog_perform(mysql1, "Xfile", BIN_LOG_HEADER_SIZE, 0, 0, 0,
                                 nullptr);
  DIE_UNLESS(rc == 2 &&
             mysql_errno(mysql1) == ER_MASTER_FATAL_ERROR_READING_BINLOG);

  /* Two readers. */
  if (!opt_silent) fprintf(stdout, "Two readers\n");

  if (!(mysql2 = mysql_client_init(nullptr))) {
    myerror("mysql_client_init() failed");
    DIE_UNLESS(0);
  }
  mysql2 = mysql_real_connect(mysql2, opt_host, opt_user, opt_password,
                              current_db, opt_port, opt_unix_socket, 0);
  if (!mysql2) {
    if (!opt_silent)
      fprintf(stdout, "mysql_real_connect() failed: '%s'\n",
              mysql_error(mysql2));
    DIE_UNLESS(0);
  }

  memset(&rpl1, 0, sizeof(rpl1));
  memset(&rpl2, 0, sizeof(rpl2));
  rpl1.start_position = rpl2.start_position = BIN_LOG_HEADER_SIZE;

  if (mysql_binlog_open(mysql1, &rpl1) || mysql_binlog_open(mysql2, &rpl2)) {
    myerror("mysql_binlog_open() failed");
    DIE_UNLESS(0);
  }

  for (;;) {
    int rc1 = mysql_binlog_fetch(mysql1, &rpl1);
    int rc2 = mysql_binlog_fetch(mysql2, &rpl2);
    if (rc1 != 0 || rc2 != 0)  // Error
      DIE_UNLESS(0);
    else if (rpl1.size != rpl2.size)
      DIE_UNLESS(0);
    else if (rpl1.size == 0)  // EOF
      break;
    if (!opt_silent) fprintf(stdout, "%lu byte length packet.\n", rpl1.size);
    DIE_UNLESS(!memcmp(rpl1.buffer, rpl2.buffer, rpl1.size));
  }

  mysql_close(mysql1);
  mysql_close(mysql2);

  rc = mysql_query(mysql,
                   "SET @@global.binlog_checksum=@save_global_binlog_checksum");
  myquery(rc);
}

static void print_no_metadata_row(MYSQL_RES *result, MYSQL_ROW *row) {
  uint i;

  for (i = 0; i < mysql_num_fields(result); i++) {
    if (!opt_silent)
      fprintf(stdout, "field#%u='%s' ", i, (*row)[i] ? (*row)[i] : "NULL");
  }
  if (!opt_silent) fprintf(stdout, "\n");
}

static void print_metadata_row(MYSQL_RES *result, MYSQL_ROW *row) {
  uint i;
  MYSQL_FIELD *field;

  mysql_field_seek(result, 0);
  for (i = 0; i < mysql_num_fields(result); i++) {
    field = mysql_fetch_field(result);
    DIE_UNLESS(field != nullptr);
    if (!opt_silent)
      fprintf(stdout, "%s='%s' ", field->name, (*row)[i] ? (*row)[i] : "NULL");
  }
  if (!opt_silent) fprintf(stdout, "\n");
}

static void perform_query(MYSQL *mysql1, const char *query, bool store_result,
                          bool use_metadata) {
  MYSQL_RES *result;
  MYSQL_ROW row;
  uint row_count;
  int rc;

  if (!opt_silent)
    fprintf(stdout, "\nQuery: '%s' %s\n", query,
            use_metadata ? "(with metadata)" : "(no metadata)");

  DIE_IF(mysql_query(mysql1, query));

  do {
    result =
        store_result ? mysql_store_result(mysql1) : mysql_use_result(mysql1);
    if (mysql_errno(mysql1)) {
      if (!opt_silent)
        fprintf(stdout, "mysql_{store,use}_result() failed: %s\n",
                mysql_error(mysql1));
      DIE_UNLESS(0);
    }
    if (!result) {
      if (!opt_silent) fprintf(stdout, "Query OK\n");
    } else {
      if (!opt_silent) fprintf(stdout, "Result:\n");

      row_count = 0;
      while ((row = mysql_fetch_row(result)) != nullptr) {
        if (use_metadata)
          print_metadata_row(result, &row);
        else
          print_no_metadata_row(result, &row);

        row_count++;
      }

      if (mysql_errno(mysql1) != 0) {
        if (!opt_silent)
          fprintf(stderr, "mysql_fetch_row() failed: %s\n",
                  mysql_error(mysql1));
        DIE_UNLESS(0);
      }
      if (!opt_silent) fprintf(stdout, "%d row(s) returned\n", row_count);

      mysql_free_result(result);
    }

    rc = mysql_next_result(mysql1);
  } while (rc == 0);
  if (rc > 0) {
    if (!opt_silent)
      fprintf(stderr, "mysql_next_result() failed: %s\n", mysql_error(mysql1));
    DIE_UNLESS(0);
  }
}

static void perform_no_metadata_query(MYSQL *mysql1, const char *query,
                                      bool store_result) {
  perform_query(mysql1, query, store_result, false);
}

static void perform_metadata_query(MYSQL *mysql1, const char *query,
                                   bool store_result) {
  perform_query(mysql1, query, store_result, true);
}

static void perform_no_metadata_stmt(MYSQL *mysql1, const char *query,
                                     uint params, uint fields) {
  MYSQL_STMT *stmt = mysql_stmt_init(mysql1);
  int rc;
  MYSQL_RES *result;

  if (!opt_silent) fprintf(stdout, "\nStmt: '%s'", query);

  DIE_UNLESS(stmt);

  rc = mysql_stmt_prepare(stmt, query, (ulong)strlen(query));
  check_execute(stmt, rc);

  verify_param_count(stmt, params);

  result = mysql_stmt_result_metadata(stmt);
  DIE_UNLESS(result);

  verify_field_count(result, fields);
  if (!opt_silent) fprintf(stdout, "\n");

  if (params == 0) {
    rc = mysql_stmt_execute(stmt);
    check_execute(stmt, rc);
  }

  mysql_free_result(result);
  mysql_stmt_close(stmt);
}

static void test_skip_metadata() {
  int rc;
  MYSQL *mysql1;
  bool optional_resultset_metadata = true;
  bool get_optional_resultset_metadata = false;
  MYSQL_RES *result;

  myheader("test_skip_metadata");

  /* Check default resultset_metadata value. */
  perform_metadata_query(mysql, "SELECT @@resultset_metadata", true);

  /* Check if we can set it to FULL. */
  rc = mysql_query(mysql, "SET @@session.resultset_metadata=FULL");
  DIE_UNLESS(rc == 0);

  /* Check if resultset_metadata's local only. */
  rc = mysql_query(mysql, "SET @@global.resultset_metadata=NONE");
  DIE_UNLESS(rc == 1 && mysql_errno(mysql) == ER_LOCAL_VARIABLE);

  /*
    Check if we can set resultset_metadata for connection that doesn't
    have CLIENT_OPTIONAL_RESULTSET_METADATA flag.
  */
  rc = mysql_query(mysql, "SET @@session.resultset_metadata=NONE");
  DIE_UNLESS(rc == 1 && mysql_errno(mysql) == ER_CLIENT_DOES_NOT_SUPPORT);

  /* Check if we can set resultset_metadata to wrong value. */
  rc = mysql_query(mysql, "SET @@session.resultset_metadata=XXX");
  DIE_UNLESS(rc == 1 && mysql_errno(mysql) == ER_WRONG_VALUE_FOR_VAR);

  /* Test MYSQL_OPT_OPTIONAL_RESULTSET_METADATA option. */
  if (!(mysql1 = mysql_client_init(nullptr))) {
    myerror("mysql_client_init() failed");
    DIE_UNLESS(0);
  }
  mysql_options(mysql1, MYSQL_OPT_OPTIONAL_RESULTSET_METADATA,
                &optional_resultset_metadata);
  mysql1 = mysql_real_connect(mysql1, opt_host, opt_user, opt_password,
                              current_db, opt_port, opt_unix_socket, 0);
  if (!mysql1) {
    if (!opt_silent)
      fprintf(stdout, "mysql_real_connect() failed: '%s'\n",
              mysql_error(mysql1));
    DIE_UNLESS(0);
  }

  /* Check CLIENT_OPTIONAL_RESULTSET_METADATA flag. */
  DIE_UNLESS(mysql->client_flag | CLIENT_OPTIONAL_RESULTSET_METADATA);

  /* Check MYSQL_OPT_OPTIONAL_RESULTSET_METADATA option. */
  mysql_get_option(mysql1, MYSQL_OPT_OPTIONAL_RESULTSET_METADATA,
                   &get_optional_resultset_metadata);
  DIE_UNLESS(get_optional_resultset_metadata == true);

  mysql_close(mysql1);

  /* Connect with CLIENT_OPTIONAL_RESULTSET_METADATA flag. */
  if (!(mysql1 = mysql_client_init(nullptr))) {
    myerror("mysql_client_init() failed");
    DIE_UNLESS(0);
  }

  mysql1 = mysql_real_connect(mysql1, opt_host, opt_user, opt_password,
                              current_db, opt_port, opt_unix_socket,
                              CLIENT_MULTI_RESULTS | CLIENT_MULTI_STATEMENTS |
                                  CLIENT_OPTIONAL_RESULTSET_METADATA);
  if (!mysql1) {
    if (!opt_silent)
      fprintf(stdout, "mysql_real_connect() failed: '%s'\n",
              mysql_error(mysql1));
    DIE_UNLESS(0);
  }

  /* Check default resultset_metadata value. */
  perform_metadata_query(mysql1, "SELECT @@resultset_metadata", true);

  /* Check if we can set resultset_metadata to wrong value. */
  rc = mysql_query(mysql1, "SET @@session.resultset_metadata=XXX");
  DIE_UNLESS(rc == 1 && mysql_errno(mysql1) == ER_WRONG_VALUE_FOR_VAR);

  /* Check if resultset_metadata's session only. */
  rc = mysql_query(mysql1, "SET @@global.resultset_metadata=NONE");
  DIE_UNLESS(rc == 1 && mysql_errno(mysql1) == ER_LOCAL_VARIABLE);

  /* Set resultset_metadata to NONE. */
  DIE_IF(mysql_query(mysql1, "SET @@session.resultset_metadata=NONE"));

  /* Check API functions. */
  DIE_IF(mysql_query(mysql1, "SELECT 1, 2, 3"));
  DIE_UNLESS(mysql_field_count(mysql1) == 3);
  result = mysql_store_result(mysql1);
  mytest(result);
  DIE_UNLESS(mysql_result_metadata(result) == RESULTSET_METADATA_NONE);
  DIE_UNLESS(mysql_num_fields(result) == 3);
  DIE_UNLESS(mysql_field_seek(result, 3) == 0);
  DIE_UNLESS(mysql_field_seek(result, 2) == 3);
  DIE_UNLESS(mysql_fetch_field(result) == nullptr);
  DIE_UNLESS(mysql_field_tell(result) == 2);
  DIE_UNLESS(mysql_fetch_field_direct(result, 1) == nullptr);
  DIE_UNLESS(mysql_fetch_fields(result) == nullptr);
  mysql_free_result(result);

  /* Test different datasets using mysql_store_result()/mysql_use_result(). */
  perform_no_metadata_query(mysql1, "SELECT @@resultset_metadata", true);
  perform_no_metadata_query(mysql1, "SELECT 1", false);
  perform_no_metadata_query(mysql1, "SELECT 1/0", false);
  DIE_IF(mysql_query(mysql1, "DROP TABLE IF EXISTS t"));
  DIE_IF(mysql_query(mysql1, "CREATE TABLE t(a INT, b varchar(20))"));
  DIE_IF(mysql_query(
      mysql1, "INSERT INTO t VALUES (0, '00'), (1, '01'), (100, '0100')"));
  perform_no_metadata_query(mysql1, "SELECT * FROM t", true);
  perform_no_metadata_query(mysql1, "SELECT a FROM t", false);
  perform_no_metadata_query(mysql1, "SELECT b FROM t", true);
  perform_no_metadata_query(mysql1, "SELECT b, a FROM t", false);

  /* Tests for prepare statements. */
  perform_no_metadata_stmt(mysql1, "SELECT 1", 0, 1);
  perform_no_metadata_stmt(mysql1, "SELECT * FROM t", 0, 2);
  perform_no_metadata_stmt(mysql1, "SELECT b FROM t WHERE a=?", 1, 1);
  perform_no_metadata_stmt(mysql1, "SELECT * FROM t WHERE a=? AND b=?", 2, 2);

  /* Test for a SP with multiple results. */
  DIE_IF(mysql_query(mysql1, "DROP PROCEDURE IF EXISTS p"));
  DIE_IF(mysql_query(mysql1,
                     "CREATE PROCEDURE p() "
                     "BEGIN "
                     "  SELECT 1; "
                     "  SELECT 1, 2, 3; "
                     "  SELECT * FROM t; "
                     "END"));
  perform_no_metadata_query(mysql1, "CALL p()", true);
  perform_no_metadata_query(mysql1, "CALL p()", false);

  /* Set resultset_metadata back to FULL. */
  DIE_IF(mysql_query(mysql1, "SET @@session.resultset_metadata=FULL"));

  /* Check it's value */
  perform_metadata_query(mysql1, "SELECT @@resultset_metadata", true);

  /* Test mysql_result_metadata(). */
  DIE_IF(mysql_query(mysql1, "SELECT 1, 2, 3"));
  result = mysql_store_result(mysql1);
  mytest(result);
  DIE_UNLESS(mysql_result_metadata(result) == RESULTSET_METADATA_FULL);
  mysql_free_result(result);

  /* Test different datasets again. */
  perform_metadata_query(mysql1, "SELECT 1", false);
  perform_metadata_query(mysql1, "SELECT 1/0", true);
  perform_metadata_query(mysql1, "SELECT * FROM t", false);
  perform_metadata_query(mysql1, "SELECT a FROM t", true);
  perform_metadata_query(mysql1, "SELECT b FROM t", false);
  perform_metadata_query(mysql1, "SELECT b, a FROM t", true);
  perform_metadata_query(mysql1, "CALL p()", true);
  perform_metadata_query(mysql1, "CALL p()", false);

  /* Cleanup. */
  DIE_IF(mysql_query(mysql1, "DROP TABLE t"));
  DIE_IF(mysql_query(mysql1, "DROP PROCEDURE p"));
  mysql_close(mysql1);
}

static void test_bug25701141() {
  MYSQL_STMT *stmt;
  int rc;
  MYSQL_BIND my_bind[2];
  char query[MAX_TEST_QUERY_LENGTH];
  const char *input1 = "abcdefgh";
  const char *input2 = "mnopqrst";
  const ulong type = CURSOR_TYPE_READ_ONLY;

  myheader("test_bug25701141");

  rc = mysql_query(mysql, "DROP TABLE IF EXISTS t1");
  myquery(rc);

  rc = mysql_query(mysql,
                   "CREATE TABLE t1( "
                   "id INT(11) NOT NULL AUTO_INCREMENT PRIMARY KEY,"
                   "serial CHAR(10) NOT NULL,"
                   "pretty CHAR(20) DEFAULT NULL,"
                   "CONSTRAINT UNIQUE KEY unique_serial (serial) USING HASH,"
                   "INDEX pretty_index USING HASH (pretty)"
                   ") ENGINE = InnoDB CHARSET = utf8 COLLATE = utf8_bin");
  myquery(rc);

  my_stpcpy(query, "INSERT IGNORE INTO t1 SET `serial`=?, `pretty`=?");
  stmt = mysql_simple_prepare(mysql, query);
  check_stmt(stmt);
  memset(my_bind, 0, sizeof(my_bind));
  my_bind[0].buffer_type = MYSQL_TYPE_STRING;
  my_bind[0].buffer = const_cast<char *>(input1);
  my_bind[0].buffer_length = (ulong)strlen(input1);

  my_bind[1].buffer_type = MYSQL_TYPE_STRING;
  my_bind[1].buffer = const_cast<char *>(input2);
  my_bind[1].buffer_length = (ulong)strlen(input2);

  rc = mysql_stmt_bind_param(stmt, my_bind);
  check_execute(stmt, rc);
  mysql_stmt_attr_set(stmt, STMT_ATTR_CURSOR_TYPE, &type);

  /* Execute */
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);
  mysql_stmt_close(stmt);

  myquery(mysql_query(mysql, "DROP TABLE t1"));
}

static void test_bug27443252() {
  MYSQL_STMT *stmt;
  MYSQL_BIND my_bind[1];
  int rc;
  int32 a;
  int row_count = 0;
  int column_count = 0;
  MYSQL_RES *metadata = nullptr;

  myheader("test_bug27443252");

  rc = mysql_query(mysql, "drop procedure if exists p1");
  myquery(rc);
  rc = mysql_query(mysql, "drop table if exists p1");
  myquery(rc);
  rc = mysql_query(mysql, "create table t1 (id int)");
  myquery(rc);
  rc = mysql_query(mysql, "create procedure p1() begin select * from t1; end");
  myquery(rc);

  /* Case 1 - Procedure call with empty result set */
  stmt = open_cursor("call p1");
  /* This should not result in hang */
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  metadata = mysql_stmt_result_metadata(stmt);
  if (metadata) {
    column_count = mysql_num_fields(metadata);
    DIE_UNLESS(column_count == 1);
  }

  memset(my_bind, 0, sizeof(my_bind));
  my_bind[0].buffer_type = MYSQL_TYPE_LONG;
  my_bind[0].buffer = (void *)&a;
  my_bind[0].buffer_length = (ulong)sizeof(a);
  rc = mysql_stmt_bind_result(stmt, my_bind);
  check_execute(stmt, rc);

  while (!mysql_stmt_fetch(stmt)) row_count++;
  DIE_UNLESS(row_count == 0);

  rc = mysql_stmt_next_result(stmt);
  check_execute(stmt, rc);
  rc = mysql_stmt_free_result(stmt);
  check_execute(stmt, rc);
  mysql_free_result(metadata);
  mysql_stmt_close(stmt);

  /* Case 2 - SELECT with empty result set */
  row_count = 0;
  stmt = open_cursor("select * from t1");
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  memset(my_bind, 0, sizeof(my_bind));
  my_bind[0].buffer_type = MYSQL_TYPE_LONG;
  my_bind[0].buffer = (void *)&a;
  my_bind[0].buffer_length = (ulong)sizeof(a);

  rc = mysql_stmt_bind_result(stmt, my_bind);
  check_execute(stmt, rc);

  while (!mysql_stmt_fetch(stmt)) row_count++;
  DIE_UNLESS(row_count == 0);
  mysql_stmt_close(stmt);

  /* Case 3 - Procedure call with non-empty result set */
  rc = mysql_query(mysql,
                   "insert into t1 (id) values "
                   " (1), (2), (3)");
  myquery(rc);
  row_count = 0;
  stmt = open_cursor("call p1");
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  memset(my_bind, 0, sizeof(my_bind));
  my_bind[0].buffer_type = MYSQL_TYPE_LONG;
  my_bind[0].buffer = (void *)&a;
  my_bind[0].buffer_length = (ulong)sizeof(a);
  rc = mysql_stmt_bind_result(stmt, my_bind);
  check_execute(stmt, rc);

  while (!mysql_stmt_fetch(stmt)) row_count++;
  DIE_UNLESS(row_count == 3);

  rc = mysql_stmt_next_result(stmt);
  check_execute(stmt, rc);
  rc = mysql_stmt_free_result(stmt);
  check_execute(stmt, rc);
  mysql_stmt_close(stmt);

  /* Case 4 - SELECT with Non-empty result set */
  row_count = 0;
  stmt = open_cursor("select * from t1");
  rc = mysql_stmt_execute(stmt);
  check_execute(stmt, rc);

  memset(my_bind, 0, sizeof(my_bind));
  my_bind[0].buffer_type = MYSQL_TYPE_LONG;
  my_bind[0].buffer = (void *)&a;
  my_bind[0].buffer_length = (ulong)sizeof(a);
  rc = mysql_stmt_bind_result(stmt, my_bind);
  check_execute(stmt, rc);

  while (!mysql_stmt_fetch(stmt)) row_count++;
  DIE_UNLESS(row_count == 3);

  rc = mysql_stmt_free_result(stmt);
  check_execute(stmt, rc);
  mysql_stmt_close(stmt);

  /* Cleanup */
  rc = mysql_query(mysql, "drop table t1");
  myquery(rc);
  rc = mysql_query(mysql, "drop procedure p1");
  myquery(rc);
}

void perform_arithmatic() { fprintf(stdout, "\n Do some other stuff."); }

static void test_wl11381() {
  MYSQL *mysql_local;
  MYSQL_RES *result;
  MYSQL_ROW row;
  net_async_status status;
  const char *stmt_text, *stmt_text1, *insert_stmt, *insert_stmt1;

  myheader("test_wl11381");
  /*make new non blocking connection to do asynchronous operations */
  if (!(mysql_local = mysql_client_init(nullptr))) {
    myerror("mysql_client_init() failed");
    exit(1);
  }
  do {
    status = mysql_real_connect_nonblocking(
        mysql_local, opt_host, opt_user, opt_password, current_db, opt_port,
        opt_unix_socket, CLIENT_MULTI_STATEMENTS);
  } while (status == NET_ASYNC_NOT_READY);
  if (status == NET_ASYNC_ERROR) {
    fprintf(stdout, "\n mysql_real_connect_nonblocking() failed. Error: [%s]",
            mysql_error(mysql_local));
    exit(1);
  } else {
    fprintf(stdout, "\n asynchronous connection estalished");
  }

  mysql_autocommit(mysql_local, true);

  if (mysql_query(mysql_local, "DROP TABLE IF EXISTS test_table")) {
    fprintf(stderr, "\n drop table failed with error %s ",
            mysql_error(mysql_local));
    exit(1);
  }

  if (mysql_query(mysql_local, "CREATE TABLE test_table(col1 int)")) {
    fprintf(stderr, "\n create table failed with error %s ",
            mysql_error(mysql_local));
    exit(1);
  }

  if (mysql_query(mysql_local,
                  "INSERT INTO test_table values(10), (20), (30)")) {
    fprintf(stderr, "\n insert into table failed with error %s ",
            mysql_error(mysql_local));
    exit(1);
  }

  stmt_text = "SELECT * FROM test_table";
  /* run query in asynchronous way */
  status = mysql_real_query_nonblocking(mysql_local, stmt_text,
                                        (ulong)strlen(stmt_text));
  /* do some other task */
  perform_arithmatic();
  while (status == NET_ASYNC_NOT_READY) {
    status = mysql_real_query_nonblocking(mysql_local, stmt_text,
                                          (ulong)strlen(stmt_text));
  }
  if (status == NET_ASYNC_ERROR) {
    fprintf(stdout, "\n mysql_real_query_nonblocking() failed");
    exit(1);
  } else {
    fprintf(stdout, "\n mysql_real_query_nonblocking() passed");
  }
  status = mysql_store_result_nonblocking(mysql_local, &result);
  /* do some other task */
  perform_arithmatic();
  while (status == NET_ASYNC_NOT_READY) {
    status = mysql_store_result_nonblocking(mysql_local, &result);
  }
  if (!result) {
    fprintf(stdout, "\n mysql_store_result_nonblocking() fetched 0 records");
    exit(1);
  } else {
    fprintf(stdout, "\n mysql_store_result_nonblocking() passed");
  }

  row = mysql_fetch_row(result);
  DIE_UNLESS(strcmp(row[0], "10") == 0);
  fprintf(stdout, "\n mysql_fetch_row() passed");

  while ((status = mysql_fetch_row_nonblocking(result, &row)) !=
         NET_ASYNC_COMPLETE)
    ;
  /* 2nd row fetched */
  DIE_UNLESS(strcmp(row[0], "20") == 0);
  fprintf(stdout, "\n mysql_fetch_row_nonblocking() passed");

  status = mysql_fetch_row_nonblocking(result, &row);
  /* do some other task */
  perform_arithmatic();
  if (status == NET_ASYNC_COMPLETE) {
    DIE_UNLESS(strcmp(row[0], "30") == 0);
  } else {
    while ((status = mysql_fetch_row_nonblocking(result, &row)) !=
           NET_ASYNC_COMPLETE)
      ;
    /* 3rd row fetched */
    DIE_UNLESS(strcmp(row[0], "30") == 0);
    fprintf(stdout, "\n mysql_fetch_row_nonblocking() passed");
  }

  while ((status = mysql_free_result_nonblocking(result)) != NET_ASYNC_COMPLETE)
    ;
  fprintf(stdout, "\n mysql_free_result_nonblocking() passed");

  if (mysql_query(mysql_local, "DROP TABLE IF EXISTS test_table1")) {
    fprintf(stderr, "\n drop table failed with error %s ",
            mysql_error(mysql_local));
    exit(1);
  }

  if (mysql_query(mysql_local,
                  "CREATE TABLE test_table1(col1 int, col2 varchar(2048))")) {
    fprintf(stderr, "\n create table failed with error %s ",
            mysql_error(mysql_local));
    exit(1);
  }

  insert_stmt =
      "INSERT INTO test_table1 values(10, repeat('a', 2000)), (2, repeat('bc', "
      "1000)), (3, repeat('def', 600))";
  status = NET_ASYNC_NOT_READY;
  while (status == NET_ASYNC_NOT_READY) {
    status = mysql_real_query_nonblocking(mysql_local, insert_stmt,
                                          (ulong)strlen(insert_stmt));
  }
  if (status == NET_ASYNC_ERROR) {
    fprintf(stdout, "\n INSERT INTO test_table1... failed");
    exit(1);
  }

  insert_stmt1 =
      "INSERT INTO test_table1 SELECT * FROM test_table1;\
                 INSERT INTO test_table1 SELECT * FROM test_table1;\
                 INSERT INTO test_table1 SELECT * FROM test_table1;\
                 INSERT INTO test_table1 SELECT * FROM test_table1;\
                 INSERT INTO test_table1 SELECT * FROM test_table1;\
                 INSERT INTO test_table1 SELECT * FROM test_table1;\
                 INSERT INTO test_table1 SELECT * FROM test_table1;\
                 INSERT INTO test_table1 SELECT * FROM test_table1;\
                 INSERT INTO test_table1 SELECT * FROM test_table1;";
  status = mysql_real_query_nonblocking(mysql_local, insert_stmt1,
                                        (ulong)strlen(insert_stmt1));
  if (status == NET_ASYNC_ERROR) {
    fprintf(stdout, "\n mysql_real_query_nonblocking() failed");
    exit(1);
  }
  /* do some other task */
  perform_arithmatic();
  while (status == NET_ASYNC_NOT_READY) {
    status = mysql_real_query_nonblocking(mysql_local, insert_stmt1,
                                          (ulong)strlen(insert_stmt1));
  }
  if (status == NET_ASYNC_ERROR) {
    fprintf(stderr,
            "\n INSERT INTO test_table1 "
            "SELECT * FROM test_table1 with error; %s ",
            mysql_error(mysql_local));
    exit(1);
  }

  while (mysql_more_results(mysql_local)) {
    status = mysql_next_result_nonblocking(mysql_local);
    if (status == NET_ASYNC_ERROR) {
      fprintf(stdout, "\n mysql_next_result_nonblocking() failed");
      exit(1);
    } else {
      while (status == NET_ASYNC_NOT_READY) {
        status = mysql_store_result_nonblocking(mysql_local, &result);
      }
    }
  }
  stmt_text1 = "SELECT * FROM test_table1";
  /* run query in asynchronous way */
  status = mysql_real_query_nonblocking(mysql_local, stmt_text1,
                                        (ulong)strlen(stmt_text1));
  /* do some other task */
  perform_arithmatic();
  while (status == NET_ASYNC_NOT_READY) {
    status = mysql_real_query_nonblocking(mysql_local, stmt_text1,
                                          (ulong)strlen(stmt_text1));
  }
  if (status == NET_ASYNC_ERROR) {
    fprintf(stdout, "\n SELECT * FROM test_table1 failed");
    exit(1);
  }
  result = mysql_use_result(mysql_local);
  while ((status = mysql_free_result_nonblocking(result)) != NET_ASYNC_COMPLETE)
    ;
  mysql_close(mysql_local);
}

static void test_wl11381_qa() {
  MYSQL *mysql_con1;
  MYSQL *mysql_con2;
  net_async_status mysql_con1_status;
  net_async_status mysql_con2_status;
  const char *stmt_text;
  int counter = 0;

  myheader("test_wl11381_qa");
  /*make new non blocking connection to do asynchronous operations */
  if (!(mysql_con1 = mysql_client_init(nullptr))) {
    myerror("mysql_client_init() failed");
    exit(1);
  }
  /*make new non blocking connection to do asynchronous operations */
  if (!(mysql_con2 = mysql_client_init(nullptr))) {
    myerror("mysql_client_init() failed");
    exit(1);
  }

  mysql_con1_status = mysql_real_connect_nonblocking(
      mysql_con1, opt_host, opt_user, opt_password, current_db, opt_port,
      opt_unix_socket, CLIENT_MULTI_STATEMENTS);

  mysql_con2_status = mysql_real_connect_nonblocking(
      mysql_con2, opt_host, opt_user, opt_password, current_db, opt_port,
      opt_unix_socket, CLIENT_MULTI_STATEMENTS);

  while (mysql_con1_status == NET_ASYNC_NOT_READY) {
    mysql_con1_status = mysql_real_connect_nonblocking(
        mysql_con1, opt_host, opt_user, opt_password, current_db, opt_port,
        opt_unix_socket, CLIENT_MULTI_STATEMENTS);
  }

  while (mysql_con2_status == NET_ASYNC_NOT_READY) {
    mysql_con2_status = mysql_real_connect_nonblocking(
        mysql_con2, opt_host, opt_user, opt_password, current_db, opt_port,
        opt_unix_socket, CLIENT_MULTI_STATEMENTS);
  }

  if (mysql_con1_status == NET_ASYNC_ERROR) {
    fprintf(stdout, "\n mysql_real_connect_nonblocking() failed");
    exit(1);
  } else if (mysql_con1_status == NET_ASYNC_COMPLETE) {
    fprintf(stdout, "\n asynchronous connection 1 estalished");
  }

  if (mysql_con2_status == NET_ASYNC_ERROR) {
    fprintf(stdout, "\n mysql_real_connect_nonblocking() failed");
    exit(1);
  } else if (mysql_con2_status == NET_ASYNC_COMPLETE) {
    fprintf(stdout, "\n asynchronous connection 2 estalished");
  }

  mysql_autocommit(mysql_con1, true);
  mysql_autocommit(mysql_con2, true);

  if (mysql_query(mysql_con1, "DROP TABLE IF EXISTS test_table")) {
    fprintf(stderr, "\n drop table failed with error %s ",
            mysql_error(mysql_con1));
    exit(1);
  }

  if (mysql_query(mysql_con1, "CREATE TABLE test_table(col1 int)")) {
    fprintf(stderr, "\n create table failed with error %s ",
            mysql_error(mysql_con1));
    exit(1);
  }

  if (mysql_query(mysql_con1,
                  "INSERT INTO test_table values(10), (20), (30)")) {
    fprintf(stderr, "\n insert into table failed with error %s ",
            mysql_error(mysql_con1));
    exit(1);
  }
  if (mysql_query(mysql_con1, "LOCK TABLE test_table WRITE")) {
    fprintf(stderr, "\n lock table failed with error %s ",
            mysql_error(mysql_con1));
    exit(1);
  }
  stmt_text = "SELECT * FROM test_table";
  /* run query in asynchronous way */
  mysql_con2_status = mysql_real_query_nonblocking(mysql_con2, stmt_text,
                                                   (ulong)strlen(stmt_text));
  if (mysql_con2_status == NET_ASYNC_NOT_READY)
    printf("\n Query not finished yet.");
  while (mysql_con2_status == NET_ASYNC_NOT_READY) {
    /* do some other task */
    perform_arithmatic();
    fprintf(stdout, "\n Operation pending.");
    counter++;
    if (counter == 4) mysql_query(mysql_con1, "UNLOCK TABLES");
    mysql_con2_status = mysql_real_query_nonblocking(mysql_con2, stmt_text,
                                                     (ulong)strlen(stmt_text));
  }
  if (mysql_con2_status == NET_ASYNC_ERROR) {
    fprintf(stdout, "\n mysql_real_query_nonblocking() failed");
    exit(1);
  } else {
    fprintf(stdout, "\n mysql_real_query_nonblocking() passed");
  }
  mysql_close(mysql_con1);
  mysql_close(mysql_con2);
}

static void test_wl11772() {
  MYSQL *mysql_con1;
  int rc;
  myheader("test_wl11772");

  if (mysql_query(mysql,
                  "CREATE USER u1 IDENTIFIED BY RANDOM PASSWORD,"
                  "u2 IDENTIFIED BY RANDOM PASSWORD,"
                  "u3 IDENTIFIED BY RANDOM PASSWORD")) {
    fprintf(stderr, "\n Create user failed with error %s ", mysql_error(mysql));
    exit(1);
  }
  MYSQL_RES *result = mysql_store_result(mysql);
  DIE_IF(result == nullptr);

  typedef struct {
    char user[32];
    char host[255];
    char password[255];
  } Userhostpass;

  Userhostpass userhostpass[3];
  MYSQL_ROW row;
  for (int i = 0; (row = mysql_fetch_row(result)) != nullptr;) {
    strcpy(userhostpass[i].user, row[0]);
    strcpy(userhostpass[i].host, row[1]);
    strcpy(userhostpass[i].password, row[2]);
    ++i;
  }
  mysql_free_result(result);
  rc = mysql_query(mysql, "GRANT SELECT ON test.* TO u1,u2,u3");
  myquery(rc);
  for (int i = 0; i < 3; ++i) {
    fprintf(stdout, "%d . Connecting to user %s@%s...", i, userhostpass[i].user,
            userhostpass[i].host);
    mysql_con1 = mysql_client_init(nullptr);
    DIE_IF(!(mysql_real_connect(mysql_con1, opt_host, userhostpass[i].user,
                                userhostpass[i].password, "test", opt_port,
                                opt_unix_socket, 0)));
    mysql_close(mysql_con1);
    fprintf(stdout, "OK\n");
  }

  if (mysql_query(mysql, "SET PASSWORD FOR u1 TO RANDOM")) {
    fprintf(stderr, "\n SET PASSWORD failed with error %s ",
            mysql_error(mysql));
    exit(1);
  }
  result = mysql_store_result(mysql);
  DIE_IF(result == nullptr);
  for (int i = 0; (row = mysql_fetch_row(result)) != nullptr;) {
    strcpy(userhostpass[i].user, row[0]);
    strcpy(userhostpass[i].host, row[1]);
    strcpy(userhostpass[i].password, row[2]);
    ++i;
  }
  mysql_free_result(result);

  for (int i = 0; i < 1; ++i) {
    fprintf(stdout, "%d . Connecting to user %s@%s...", i, userhostpass[i].user,
            userhostpass[i].host);
    mysql_con1 = mysql_client_init(nullptr);
    DIE_IF(!(mysql_real_connect(mysql_con1, opt_host, userhostpass[i].user,
                                userhostpass[i].password, "test", opt_port,
                                opt_unix_socket, 0)));
    mysql_close(mysql_con1);
    fprintf(stdout, "OK\n");
  }
  rc = mysql_query(mysql, "DROP USER u1,u2,u3");
  myquery(rc);
}

static void test_wl12475() {
  MYSQL *mysql_local;
  char compress_method[] = "zstd";
  int rc;
  MYSQL_RES *result;

  myheader("test_wl12475");
  if (!(mysql_local = mysql_client_init(nullptr))) {
    myerror("mysql_client_init() failed");
    exit(1);
  }
  rc = mysql_options(mysql_local, MYSQL_OPT_COMPRESSION_ALGORITHMS,
                     compress_method);
  myquery(rc);

  if (!(mysql_real_connect(mysql_local, opt_host, opt_user, opt_password,
                           current_db, opt_port, opt_unix_socket, 0))) {
    myerror("connection failed");
    exit(1);
  }
  rc = mysql_query(mysql, "SELECT REPEAT('A',1024*1024)");
  myquery(rc);

  /* get the result */
  result = mysql_use_result(mysql);
  mytest(result);

  my_process_result_set(result);
  mysql_free_result(result);

  mysql_close(mysql_local);

  if (!(mysql_local = mysql_client_init(nullptr))) {
    myerror("mysql_client_init() failed");
    exit(1);
  }
  rc = mysql_options(mysql_local, MYSQL_OPT_COMPRESSION_ALGORITHMS,
                     compress_method);
  myquery(rc);
  rc = mysql_options(mysql_local, MYSQL_OPT_COMPRESSION_ALGORITHMS, "zlib");
  myquery(rc);
  // connection should fail
  if (!mysql_real_connect(mysql_local, opt_host, opt_user, opt_password,
                          current_db, opt_port, opt_unix_socket, 0)) {
    myerror("connection failed");
    mysql_error(mysql_local);
  }
  mysql_close(mysql_local);
}

static void test_bug30032302() {
  MYSQL_RES *res;

  myheader("test_bug30032302");

  res = mysql_list_processes(mysql);
  mysql_free_result(res);

  check_warning(mysql);
}

static void test_bug31104389() {
  myheader("test_bug31104389");

  MYSQL *mysql_local;

  if (!(mysql_local = mysql_client_init(NULL))) {
    myerror("mysql_client_init() failed");
    exit(1);
  }
  /* make new non blocking connection to an invalid host */
  mysql_real_connect_nonblocking(mysql_local, "0100::", opt_user, opt_password,
                                 current_db, opt_port, opt_unix_socket,
                                 CLIENT_MULTI_STATEMENTS);

  /* this call should not crash and report error */
  mysql_close(mysql_local);
}

static void test_get_connect_stage() {
  MYSQL *mysql_async = NULL, *mysql_sync = NULL;
  net_async_status status;

  myheader("test_get_connect_stage");

  enum connect_stage cs = mysql_get_connect_stage(mysql_async);
  if (cs != CONNECT_STAGE_INVALID) {
    fprintf(stdout,
            "\n Expected connect_stage to be "
            "CONNECT_STAGE_INVALID for uninitialized mysql");
    exit(1);
  }

  /* test stages for a nonblocking conneciton */
  if (!(mysql_async = mysql_client_init(NULL))) {
    myerror("mysql_client_init() failed");
    exit(1);
  }
  cs = mysql_get_connect_stage(mysql_async);
  if (cs != CONNECT_STAGE_NOT_STARTED) {
    fprintf(stdout,
            "\n Expected connect_stage to be "
            "CONNECT_STAGE_NET_BEGIN_CONNECT for just initialized connection"
            "but its %d",
            cs);
    exit(1);
  }

  enum connect_stage cs_prev = cs;
  do {
    status = mysql_real_connect_nonblocking(
        mysql_async, opt_host, opt_user, opt_password, current_db, opt_port,
        opt_unix_socket, CLIENT_MULTI_STATEMENTS);
    cs = mysql_get_connect_stage(mysql_async);
    // Always expect state machine to make forward progress
    if (cs < cs_prev) {
      fprintf(stdout,
              "\n Did not expect connect_stage to be %d, as previous was %d",
              cs, cs_prev);
      exit(1);
    } else if (cs != cs_prev) {
      if (!opt_silent)
        fprintf(stdout,
                "\n Nonblocking connect made transition from stage %d to %d",
                cs_prev, cs);
    }
    cs_prev = cs;
  } while (status == NET_ASYNC_NOT_READY);
  if (status == NET_ASYNC_ERROR) {
    fprintf(stdout, "\n mysql_real_connect_nonblocking() failed. stage:%d", cs);
    exit(1);
  }

  cs = mysql_get_connect_stage(mysql_async);
  if (cs != CONNECT_STAGE_COMPLETE) {
    fprintf(stdout,
            "\n Expected connect_stage as CONNECT_STAGE_COMPLETE, its %d", cs);
    exit(1);
  }
  if (!opt_silent)
    fprintf(stdout, "\n Nonblocking connect successful. Final stage %d", cs);

  mysql_close(mysql_async);

  /* test stages for a blocking conneciton */
  if (!(mysql_sync = mysql_client_init(NULL))) {
    myerror("mysql_client_init() failed");
    exit(1);
  }
  cs = mysql_get_connect_stage(mysql_sync);
  if (cs != CONNECT_STAGE_NOT_STARTED) {
    fprintf(stdout,
            "\n Sync:Expected connect_stage to be "
            "CONNECT_STAGE_NOT_STARTED for just initialized connection"
            "but its %d",
            cs);
    exit(1);
  }

  if (!opt_silent)
    fprintf(stdout, "\n Starting blocking connect. Starting stage %d", cs);
  if (!(mysql_real_connect(mysql_sync, opt_host, opt_user, opt_password,
                           current_db, opt_port, opt_unix_socket, 0))) {
    myerror("connection failed");
    exit(1);
  }
  cs = mysql_get_connect_stage(mysql_sync);
  if (cs != CONNECT_STAGE_COMPLETE) {
    fprintf(stdout,
            "\n Sync:Expected connect_stage as CONNECT_STAGE_COMPLETE, its %d",
            cs);
    exit(1);
  }
  if (!opt_silent)
    fprintf(stdout, "\n Blocking connect successful. Final stage %d", cs);

  mysql_close(mysql_sync);
}

static void test_ssl_connect() {
  MYSQL conn;
  server_cert_validator_ptr validator;
  const void *context = reinterpret_cast<const void *>(0x123456);

  DBUG_ENTER("test_ssl_connect");
  myheader("test_ssl_connect");

  /* Checking that SSL connect works as expected */
  if (!mysql_client_init_certs(&conn)) {
    fprintf(stdout, "mysql_client_init_certs() failed\n");
    DIE_UNLESS(0);
  }

  if (!mysql_try_connect(&conn)) {
    fprintf(stdout, "Failed to connect using SSL\n");
    DIE_UNLESS(0);
  }
  mysql_close(&conn);

  /* Now let's configure a custom server cert validator callback */
  /* No callback context specified so far */
  if (!mysql_client_init_certs(&conn)) {
    fprintf(stdout, "mysql_client_init_certs() failed\n");
    DIE_UNLESS(0);
  }

  validator = server_cert_verifier_no_context;
  mysql_options(&conn, MYSQL_OPT_TLS_CERT_CALLBACK,
                reinterpret_cast<void *>(&validator));

  if (!mysql_try_connect(&conn)) {
    fprintf(stdout,
            "Failed to connect using SSL with cert validator callback\n");
    DIE_UNLESS(0);
  }
  mysql_close(&conn);

  /* Testing callback context pointer propagation */
  if (!mysql_client_init_certs(&conn)) {
    fprintf(stdout, "mysql_client_init_certs() failed\n");
    DIE_UNLESS(0);
  }

  validator = server_cert_verifier_with_context;
  mysql_options(&conn, MYSQL_OPT_TLS_CERT_CALLBACK,
                reinterpret_cast<void *>(&validator));
  mysql_options(&conn, MYSQL_OPT_TLS_CERT_CALLBACK_CONTEXT,
                reinterpret_cast<void *>(&context));

  if (!mysql_try_connect(&conn)) {
    fprintf(stdout,
            "Failed to connect using SSL with cert validator callback and "
            "context\n");
    DIE_UNLESS(0);
  }
  mysql_close(&conn);

  /* Testing callback context pointer propagation */
  if (!mysql_client_init_certs(&conn)) {
    fprintf(stdout, "mysql_client_init_certs() failed\n");
    DIE_UNLESS(0);
  }

  validator = server_cert_verifier_fail;
  mysql_options(&conn, MYSQL_OPT_TLS_CERT_CALLBACK,
                reinterpret_cast<void *>(&validator));

  if (mysql_try_connect(&conn)) {
    fprintf(stdout,
            "Connect succeeded using negative validator callback"
            " - was expected to fail\n");
    DIE_UNLESS(0);
  }
  mysql_close(&conn);

  DBUG_VOID_RETURN;
}

static void test_ssl_connect_ctx() {
  MYSQL conn;
  SSL_CTX *ssl_ctx;
  server_cert_validator_ptr validator;
  const void *context = reinterpret_cast<const void *>(0x123456);

  DBUG_ENTER("test_ssl_connect_ctx");
  myheader("test_ssl_connect_ctx");

  /* Check that we can connect using SSL_CTX */
  if (!mysql_client_init(&conn)) {
    fprintf(stdout, "mysql_client_init() failed\n");
    DIE_UNLESS(0);
  }

  if (!(ssl_ctx = init_ssl_ctx())) {
    fprintf(stdout, "SSL_CTX initialization error\n");
    DIE_UNLESS(0);
  }
  mysql_options(&conn, MYSQL_OPT_SSL_CONTEXT, ssl_ctx);

  if (!mysql_try_connect(&conn)) {
    fprintf(stdout, "Failed to connect using SSL_CTX\n");
    DIE_UNLESS(0);
  }
  mysql_close(&conn);
  SSL_CTX_free(ssl_ctx);

  /* Connect using SSL_CTX and server cert validation callback no context */
  if (!mysql_client_init(&conn)) {
    fprintf(stdout, "mysql_client_init() failed\n");
    DIE_UNLESS(0);
  }

  if (!(ssl_ctx = init_ssl_ctx())) {
    fprintf(stdout, "SSL_CTX initialization error\n");
    DIE_UNLESS(0);
  }
  mysql_options(&conn, MYSQL_OPT_SSL_CONTEXT, ssl_ctx);
  validator = server_cert_verifier_no_context;
  mysql_options(&conn, MYSQL_OPT_TLS_CERT_CALLBACK,
                reinterpret_cast<void *>(&validator));

  if (!mysql_try_connect(&conn)) {
    fprintf(stdout,
            "Failed to connect using SSL_CTX with validation callback no "
            "context\n");
    DIE_UNLESS(0);
  }
  mysql_close(&conn);
  SSL_CTX_free(ssl_ctx);

  /* Connect using SSL_CTX and server cert validation callback with context */
  if (!mysql_client_init(&conn)) {
    fprintf(stdout, "mysql_client_init() failed\n");
    DIE_UNLESS(0);
  }

  if (!(ssl_ctx = init_ssl_ctx())) {
    fprintf(stdout, "SSL_CTX initialization error\n");
    DIE_UNLESS(0);
  }
  mysql_options(&conn, MYSQL_OPT_SSL_CONTEXT, ssl_ctx);
  validator = server_cert_verifier_with_context;
  mysql_options(&conn, MYSQL_OPT_TLS_CERT_CALLBACK,
                reinterpret_cast<void *>(&validator));
  mysql_options(&conn, MYSQL_OPT_TLS_CERT_CALLBACK_CONTEXT,
                reinterpret_cast<void *>(&context));

  if (!mysql_try_connect(&conn)) {
    fprintf(stdout,
            "Failed to connect using SSL_CTX with validation callback with "
            "context\n");
    DIE_UNLESS(0);
  }
  mysql_close(&conn);
  SSL_CTX_free(ssl_ctx);

  /* Negative connection test using SSL_CTX */
  if (!mysql_client_init(&conn)) {
    fprintf(stdout, "mysql_client_init() failed\n");
    DIE_UNLESS(0);
  }

  if (!(ssl_ctx = init_ssl_ctx())) {
    fprintf(stdout, "SSL_CTX initialization error\n");
    DIE_UNLESS(0);
  }
  mysql_options(&conn, MYSQL_OPT_SSL_CONTEXT, ssl_ctx);
  validator = server_cert_verifier_fail;
  mysql_options(&conn, MYSQL_OPT_TLS_CERT_CALLBACK,
                reinterpret_cast<void *>(&validator));
  mysql_options(&conn, MYSQL_OPT_TLS_CERT_CALLBACK_CONTEXT,
                reinterpret_cast<void *>(&context));

  if (mysql_try_connect(&conn)) {
    fprintf(stdout, "Succeeded MySQL connect - was expected to fail.\n");
    DIE_UNLESS(0);
  }
  mysql_close(&conn);
  SSL_CTX_free(ssl_ctx);

  DBUG_VOID_RETURN;
}

static void test_option_tos() {
  MYSQL conn;
  auto ip_proto = IPPROTO_IPV6;
  auto opt_name = IPV6_TCLASS;
  uint mysql_proto_tcp = MYSQL_PROTOCOL_TCP;
  int tos = 0;
  uint tos_len = sizeof(tos);

  DBUG_ENTER("test_option_tos");
  myheader("test_option_tos");

  /* 1. Check the initial ToS = 0 */
  if (!mysql_client_init(&conn)) {
    fprintf(stdout, "mysql_client_init() failed\n");
    DIE_UNLESS(0);
  }
  mysql_options(&conn, MYSQL_OPT_PROTOCOL, &mysql_proto_tcp);
  if (!(mysql_real_connect(&conn, opt_host, opt_user, opt_password,
                           opt_db ? opt_db : "test", opt_port, opt_unix_socket,
                           0))) {
    fprintf(stderr, "\n connection failed(%s)", mysql_error(&conn));
    exit(1);
  }

  // Verify the ToS is 0 from getsockopt
  int fd = conn.net.vio->mysql_socket.fd;
  int rc = getsockopt(fd, ip_proto, opt_name, &tos, &tos_len);
  fprintf(stdout, "ToS is set to %d\n", tos);
  DIE_UNLESS(rc == 0);
  DIE_UNLESS(tos == 0);
  mysql_close(&conn);

  /* 2. Check the ToS is updated after mysql_option */
  if (!mysql_client_init(&conn)) {
    fprintf(stdout, "mysql_client_init() failed\n");
    DIE_UNLESS(0);
  }
  mysql_options(&conn, MYSQL_OPT_PROTOCOL, &mysql_proto_tcp);

  // Set ToS to 140
  uint input_tos = 140;
  mysql_options(&conn, MYSQL_OPT_TOS, &input_tos);

  if (!(mysql_real_connect(&conn, opt_host, opt_user, opt_password,
                           opt_db ? opt_db : "test", opt_port, opt_unix_socket,
                           0))) {
    fprintf(stderr, "\n connection failed(%s)", mysql_error(&conn));
    exit(1);
  }

  // Verify the ToS is set to 140 from getsockopt
  fd = conn.net.vio->mysql_socket.fd;
  rc = getsockopt(fd, ip_proto, opt_name, &tos, &tos_len);
  fprintf(stdout, "ToS is set to %d\n", tos);
  DIE_UNLESS(rc == 0);
  DIE_UNLESS(tos == 140);
  mysql_close(&conn);

  /* 3. Check the max allowed ToS is accepted */
  if (!mysql_client_init(&conn)) {
    fprintf(stdout, "mysql_client_init() failed\n");
    DIE_UNLESS(0);
  }
  mysql_options(&conn, MYSQL_OPT_PROTOCOL, &mysql_proto_tcp);

  // Set ToS to 252
  input_tos = 252;
  mysql_options(&conn, MYSQL_OPT_TOS, &input_tos);

  if (!(mysql_real_connect(&conn, opt_host, opt_user, opt_password,
                           opt_db ? opt_db : "test", opt_port, opt_unix_socket,
                           0))) {
    fprintf(stderr, "\n connection failed(%s)", mysql_error(&conn));
    exit(1);
  }

  // Verify the ToS is set to 252 from getsockopt
  fd = conn.net.vio->mysql_socket.fd;
  rc = getsockopt(fd, ip_proto, opt_name, &tos, &tos_len);
  fprintf(stdout, "ToS is set to %d\n", tos);
  DIE_UNLESS(rc == 0);
  DIE_UNLESS(tos == 252);
  mysql_close(&conn);

  /* 4. Check an invalid ToS is ignored */
  if (!mysql_client_init(&conn)) {
    fprintf(stdout, "mysql_client_init() failed\n");
    DIE_UNLESS(0);
  }
  mysql_options(&conn, MYSQL_OPT_PROTOCOL, &mysql_proto_tcp);

  // Set ToS to 256
  input_tos = 256;
  mysql_options(&conn, MYSQL_OPT_TOS, &input_tos);

  if (!(mysql_real_connect(&conn, opt_host, opt_user, opt_password,
                           opt_db ? opt_db : "test", opt_port, opt_unix_socket,
                           0))) {
    fprintf(stderr, "\n connection failed(%s)", mysql_error(&conn));
    exit(1);
  }

  // Verify the ToS is set to 0 from getsockopt
  fd = conn.net.vio->mysql_socket.fd;
  rc = getsockopt(fd, ip_proto, opt_name, &tos, &tos_len);
  fprintf(stdout, "ToS is set to %d\n", tos);
  DIE_UNLESS(rc == 0);
  DIE_UNLESS(tos == 0);
  mysql_close(&conn);

  DBUG_VOID_RETURN;
}

static struct my_tests_st my_tests[] = {
    {"disable_query_logs", disable_query_logs},
    {"test_view_sp_list_fields", test_view_sp_list_fields},
    {"client_query", client_query},
    {"test_prepare_insert_update", test_prepare_insert_update},
    {"test_fetch_seek", test_fetch_seek},
    {"test_fetch_nobuffs", test_fetch_nobuffs},
    {"test_open_direct", test_open_direct},
    {"test_fetch_null", test_fetch_null},
    {"test_ps_null_param", test_ps_null_param},
    {"test_fetch_date", test_fetch_date},
    {"test_fetch_str", test_fetch_str},
    {"test_fetch_long", test_fetch_long},
    {"test_fetch_short", test_fetch_short},
    {"test_fetch_tiny", test_fetch_tiny},
    {"test_fetch_bigint", test_fetch_bigint},
    {"test_fetch_float", test_fetch_float},
    {"test_fetch_double", test_fetch_double},
    {"test_bind_result_ext", test_bind_result_ext},
    {"test_bind_result_ext1", test_bind_result_ext1},
    {"test_select_direct", test_select_direct},
    {"test_select_prepare", test_select_prepare},
    {"test_select", test_select},
    {"test_select_version", test_select_version},
    {"test_ps_conj_select", test_ps_conj_select},
    {"test_select_show_table", test_select_show_table},
    {"test_func_fields", test_func_fields},
    {"test_long_data", test_long_data},
    {"test_insert", test_insert},
    {"test_set_variable", test_set_variable},
    {"test_select_show", test_select_show},
    {"test_prepare_noparam", test_prepare_noparam},
    {"test_time_zone", test_time_zone},
    {"test_bind_result", test_bind_result},
    {"test_prepare_simple", test_prepare_simple},
    {"test_prepare", test_prepare},
    {"test_null", test_null},
    {"test_debug_example", test_debug_example},
    {"test_update", test_update},
    {"test_simple_update", test_simple_update},
    {"test_simple_delete", test_simple_delete},
    {"test_double_compare", test_double_compare},
    {"client_store_result", client_store_result},
    {"client_use_result", client_use_result},
    {"test_tran_bdb", test_tran_bdb},
    {"test_tran_innodb", test_tran_innodb},
    {"test_prepare_ext", test_prepare_ext},
    {"test_prepare_syntax", test_prepare_syntax},
    {"test_field_names", test_field_names},
    {"test_field_flags", test_field_flags},
    {"test_long_data_str", test_long_data_str},
    {"test_long_data_str1", test_long_data_str1},
    {"test_long_data_bin", test_long_data_bin},
    {"test_warnings", test_warnings},
    {"test_errors", test_errors},
    {"test_prepare_resultset", test_prepare_resultset},
    {"test_stmt_close", test_stmt_close},
    {"test_prepare_field_result", test_prepare_field_result},
    {"test_multi_stmt", test_multi_stmt},
    {"test_multi_statements", test_multi_statements},
    {"test_prepare_multi_statements", test_prepare_multi_statements},
    {"test_store_result", test_store_result},
    {"test_store_result1", test_store_result1},
    {"test_store_result2", test_store_result2},
    {"test_subselect", test_subselect},
    {"test_date", test_date},
    {"test_date_frac", test_date_frac},
    {"test_temporal_param", test_temporal_param},
    {"test_date_date", test_date_date},
    {"test_date_time", test_date_time},
    {"test_date_ts", test_date_ts},
    {"test_date_dt", test_date_dt},
    {"test_prepare_alter", test_prepare_alter},
    {"test_manual_sample", test_manual_sample},
    {"test_pure_coverage", test_pure_coverage},
    {"test_buffers", test_buffers},
    {"test_ushort_bug", test_ushort_bug},
    {"test_sshort_bug", test_sshort_bug},
    {"test_stiny_bug", test_stiny_bug},
    {"test_field_misc", test_field_misc},
    {"test_set_option", test_set_option},
    {"test_prepare_grant", test_prepare_grant},
    {"test_explain_bug", test_explain_bug},
    {"test_decimal_bug", test_decimal_bug},
    {"test_nstmts", test_nstmts},
    {"test_logs;", test_logs},
    {"test_truncated_rows", test_truncated_rows},
    {"test_fetch_offset", test_fetch_offset},
    {"test_fetch_column", test_fetch_column},
    {"test_mem_overun", test_mem_overun},
    {"test_list_fields", test_list_fields},
    {"test_free_result", test_free_result},
    {"test_free_store_result", test_free_store_result},
    {"test_sqlmode", test_sqlmode},
    {"test_ts", test_ts},
    {"test_bug1115", test_bug1115},
    {"test_bug1180", test_bug1180},
    {"test_bug1500", test_bug1500},
    {"test_bug1644", test_bug1644},
    {"test_bug1946", test_bug1946},
    {"test_bug2248", test_bug2248},
    {"test_parse_error_and_bad_length", test_parse_error_and_bad_length},
    {"test_bug2247", test_bug2247},
    {"test_subqueries", test_subqueries},
    {"test_bad_union", test_bad_union},
    {"test_distinct", test_distinct},
    {"test_subqueries_ref", test_subqueries_ref},
    {"test_union", test_union},
    {"test_bug3117", test_bug3117},
    {"test_join", test_join},
    {"test_selecttmp", test_selecttmp},
    {"test_create_drop", test_create_drop},
    {"test_rename", test_rename},
    {"test_do_set", test_do_set},
    {"test_multi", test_multi},
    {"test_insert_select", test_insert_select},
    {"test_bind_nagative", test_bind_nagative},
    {"test_derived", test_derived},
    {"test_xjoin", test_xjoin},
    {"test_bug3035", test_bug3035},
    {"test_union2", test_union2},
    {"test_bug1664", test_bug1664},
    {"test_union_param", test_union_param},
    {"test_order_param", test_order_param},
    {"test_ps_i18n", test_ps_i18n},
    {"test_bug3796", test_bug3796},
    {"test_bug4026", test_bug4026},
    {"test_bug4079", test_bug4079},
    {"test_bug4236", test_bug4236},
    {"test_bug4030", test_bug4030},
    {"test_bug5126", test_bug5126},
    {"test_bug4231", test_bug4231},
    {"test_bug5399", test_bug5399},
    {"test_bug5194", test_bug5194},
    {"test_bug5315", test_bug5315},
    {"test_bug6049", test_bug6049},
    {"test_bug6058", test_bug6058},
    {"test_bug6059", test_bug6059},
    {"test_bug6046", test_bug6046},
    {"test_bug6081", test_bug6081},
    {"test_bug6096", test_bug6096},
    {"test_datetime_ranges", test_datetime_ranges},
    {"test_bug4172", test_bug4172},
    {"test_conversion", test_conversion},
    {"test_rewind", test_rewind},
    {"test_bug6761", test_bug6761},
    {"test_view", test_view},
    {"test_view_where", test_view_where},
    {"test_view_2where", test_view_2where},
    {"test_view_star", test_view_star},
    {"test_view_insert", test_view_insert},
    {"test_left_join_view", test_left_join_view},
    {"test_view_insert_fields", test_view_insert_fields},
    {"test_basic_cursors", test_basic_cursors},
    {"test_cursors_with_union", test_cursors_with_union},
    {"test_truncation", test_truncation},
    {"test_truncation_option", test_truncation_option},
    {"test_client_character_set", test_client_character_set},
    {"test_bug8330", test_bug8330},
    {"test_bug7990", test_bug7990},
    {"test_bug8378", test_bug8378},
    {"test_bug8722", test_bug8722},
    {"test_bug8880", test_bug8880},
    {"test_bug9159", test_bug9159},
    {"test_bug9520", test_bug9520},
    {"test_bug9478", test_bug9478},
    {"test_bug9643", test_bug9643},
    {"test_bug10729", test_bug10729},
    {"test_bug11111", test_bug11111},
    {"test_bug9992", test_bug9992},
    {"test_bug10736", test_bug10736},
    {"test_bug10794", test_bug10794},
    {"test_bug11172", test_bug11172},
    {"test_bug11656", test_bug11656},
    {"test_bug10214", test_bug10214},
    {"test_bug21246", test_bug21246},
    {"test_bug9735", test_bug9735},
    {"test_bug11183", test_bug11183},
    {"test_bug11037", test_bug11037},
    {"test_bug10760", test_bug10760},
    {"test_bug12001", test_bug12001},
    {"test_bug11718", test_bug11718},
    {"test_bug12925", test_bug12925},
    {"test_bug11909", test_bug11909},
    {"test_bug11901", test_bug11901},
    {"test_bug11904", test_bug11904},
    {"test_bug12243", test_bug12243},
    {"test_bug14210", test_bug14210},
    {"test_bug13488", test_bug13488},
    {"test_bug13524", test_bug13524},
    {"test_bug14845", test_bug14845},
    {"test_opt_reconnect", test_opt_reconnect},
    {"test_bug15510", test_bug15510},
    {"test_bug12744", test_bug12744},
    {"test_bug16143", test_bug16143},
    {"test_bug16144", test_bug16144},
    {"test_bug15613", test_bug15613},
    {"test_bug20152", test_bug20152},
    //  { "test_bug14169", test_bug14169 },
    {"test_bug17667", test_bug17667},
    {"test_bug15752", test_bug15752},
    {"test_mysql_insert_id", test_mysql_insert_id},
    {"test_bug19671", test_bug19671},
    {"test_bug21206", test_bug21206},
    {"test_bug21726", test_bug21726},
    {"test_bug15518", test_bug15518},
    {"test_bug23383", test_bug23383},
    {"test_bug32265", test_bug32265},
    {"test_bug21635", test_bug21635},
    {"test_status", test_status},
    {"test_bug24179", test_bug24179},
    {"test_bug28075", test_bug28075},
    {"test_bug27876", test_bug27876},
    {"test_bug28505", test_bug28505},
    {"test_bug28934", test_bug28934},
    {"test_bug27592", test_bug27592},
    {"test_bug29687", test_bug29687},
    {"test_bug29692", test_bug29692},
    {"test_bug29306", test_bug29306},
    {"test_change_user", test_change_user},
    {"test_bug30472", test_bug30472},
    {"test_bug20023", test_bug20023},
    {"test_bug45010", test_bug45010},
    {"test_bug53371", test_bug53371},
    {"test_bug31418", test_bug31418},
    {"test_bug31669", test_bug31669},
    {"test_bug28386", test_bug28386},
    {"test_wl4166_1", test_wl4166_1},
    {"test_wl4166_2", test_wl4166_2},
    {"test_wl4166_3", test_wl4166_3},
    {"test_wl4166_4", test_wl4166_4},
    {"test_bug36004", test_bug36004},
    {"test_wl4284_1", test_wl4284_1},
    {"test_wl4435", test_wl4435},
    {"test_wl4435_2", test_wl4435_2},
    {"test_wl4435_3", test_wl4435_3},
    {"test_bug38486", test_bug38486},
    {"test_bug33831", test_bug33831},
    {"test_bug40365", test_bug40365},
    {"test_bug43560", test_bug43560},
    {"test_bug36326", test_bug36326},
    {"test_bug41078", test_bug41078},
    {"test_bug44495", test_bug44495},
    {"test_bug49972", test_bug49972},
    {"test_bug42373", test_bug42373},
    {"test_bug54041", test_bug54041},
    {"test_bug47485", test_bug47485},
    {"test_bug58036", test_bug58036},
    {"test_bug57058", test_bug57058},
    {"test_bug56976", test_bug56976},
    {"test_bug11766854", test_bug11766854},
    {"test_bug54790", test_bug54790},
    {"test_bug12337762", test_bug12337762},
    {"test_bug11754979", test_bug11754979},
    {"test_bug13001491", test_bug13001491},
    {"test_wl5968", test_wl5968},
    {"test_wl5924", test_wl5924},
    {"test_wl6587", test_wl6587},
    {"test_wl5928", test_wl5928},
    {"test_wl6797", test_wl6797},
    {"test_wl6791", test_wl6791},
    {"test_wl5768", test_wl5768},
    {"test_bug17309863", test_bug17309863},
    {"test_bug17512527", test_bug17512527},
    {"test_wl8016", test_wl8016},
    {"test_bug20645725", test_bug20645725},
    {"test_bug19894382", test_bug19894382},
    {"test_bug20444737", test_bug20444737},
    {"test_bug21104470", test_bug21104470},
    {"test_bug21293012", test_bug21293012},
    {"test_bug21199582", test_bug21199582},
    {"test_bug20821550", test_bug20821550},
    {"test_wl8754", test_wl8754},
    {"test_bug17883203", test_bug17883203},
    {"test_bug22336527", test_bug22336527},
    {"test_bug24963580", test_bug24963580},
    {"test_mysql_binlog", test_mysql_binlog},
    {"test_bug22028117", test_bug22028117},
    {"test_skip_metadata", test_skip_metadata},
    {"test_bug25701141", test_bug25701141},
    {"test_bug27443252", test_bug27443252},
    {"test_wl11381", test_wl11381},
    {"test_wl11381_qa", test_wl11381_qa},
    {"test_wl11772", test_wl11772},
    {"test_wl12475", test_wl12475},
    {"test_bug30032302", test_bug30032302},
    {"test_bug31104389", test_bug31104389},
    {"test_get_connect_stage", test_get_connect_stage},
    {"test_ssl_connect", test_ssl_connect},
    {"test_ssl_connect_ctx", test_ssl_connect_ctx},
    {"test_option_tos", test_option_tos},
    {nullptr, nullptr}};

static struct my_tests_st *get_my_tests() { return my_tests; }
