/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/test/ext/test_ext_mysql.h"
#include "hphp/runtime/ext/ext_mysql.h"
#include "hphp/test/ext/test_mysql_info.h"
#include "errmsg.h"

///////////////////////////////////////////////////////////////////////////////

bool TestExtMysql::RunTests(const std::string &which) {
  // XXX: Disabled until flakiness is resolved: t1135133
  return true;

  bool ret = true;
  mysqlExtension::ReadOnly = false;

  DECLARE_TEST_FUNCTIONS("");

  RUN_TEST(test_mysql_connect);
  RUN_TEST(test_mysql_pconnect);
  RUN_TEST(test_mysql_set_charset);
  RUN_TEST(test_mysql_ping);
  RUN_TEST(test_mysql_escape_string);
  RUN_TEST(test_mysql_real_escape_string);
  RUN_TEST(test_mysql_client_encoding);
  RUN_TEST(test_mysql_close);
  RUN_TEST(test_mysql_errno);
  RUN_TEST(test_mysql_error);
  RUN_TEST(test_mysql_warning_count);
  RUN_TEST(test_mysql_get_client_info);
  RUN_TEST(test_mysql_get_host_info);
  RUN_TEST(test_mysql_get_proto_info);
  RUN_TEST(test_mysql_get_server_info);
  RUN_TEST(test_mysql_info);
  RUN_TEST(test_mysql_insert_id);
  RUN_TEST(test_mysql_stat);
  RUN_TEST(test_mysql_thread_id);
  RUN_TEST(test_mysql_create_db);
  RUN_TEST(test_mysql_select_db);
  RUN_TEST(test_mysql_drop_db);
  RUN_TEST(test_mysql_affected_rows);
  RUN_TEST(test_mysql_set_timeout);
#ifdef MYSQL_MILLISECOND_TIMEOUT
  RUN_TEST(test_mysql_subsecond_timeout);
#endif
  RUN_TEST(test_mysql_query);
  RUN_TEST(test_mysql_unbuffered_query);
  RUN_TEST(test_mysql_db_query);
  RUN_TEST(test_mysql_list_dbs);
  RUN_TEST(test_mysql_list_tables);
  RUN_TEST(test_mysql_list_fields);
  RUN_TEST(test_mysql_list_processes);
  RUN_TEST(test_mysql_db_name);
  RUN_TEST(test_mysql_tablename);
  RUN_TEST(test_mysql_num_fields);
  RUN_TEST(test_mysql_num_rows);
  RUN_TEST(test_mysql_free_result);
  RUN_TEST(test_mysql_data_seek);
  RUN_TEST(test_mysql_fetch_row);
  RUN_TEST(test_mysql_fetch_assoc);
  RUN_TEST(test_mysql_fetch_array);
  RUN_TEST(test_mysql_fetch_lengths);
  RUN_TEST(test_mysql_fetch_object);
  RUN_TEST(test_mysql_result);
  RUN_TEST(test_mysql_fetch_field);
  RUN_TEST(test_mysql_field_seek);
  RUN_TEST(test_mysql_field_name);
  RUN_TEST(test_mysql_field_table);
  RUN_TEST(test_mysql_field_len);
  RUN_TEST(test_mysql_field_type);
  RUN_TEST(test_mysql_field_flags);

  return ret;
}

static bool CreateTestTable() {
  f_mysql_select_db(TEST_DATABASE);
  f_mysql_query("drop table test");
  return f_mysql_query("create table test (id int not null auto_increment,"
                       " name varchar(255) not null, primary key (id)) "
                       "engine=innodb").toBoolean();
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtMysql::test_mysql_connect() {
  Variant conn = f_mysql_connect(TEST_HOSTNAME, TEST_USERNAME, TEST_PASSWORD);
  VERIFY(!same(conn, false));
  return Count(true);
}

bool TestExtMysql::test_mysql_pconnect() {
  Variant conn = f_mysql_pconnect(TEST_HOSTNAME, TEST_USERNAME, TEST_PASSWORD);
  VERIFY(!same(conn, false));
  return Count(true);
}

bool TestExtMysql::test_mysql_set_charset() {
  Variant conn = f_mysql_connect(TEST_HOSTNAME, TEST_USERNAME,
                                 TEST_PASSWORD);
  VERIFY(f_mysql_set_charset("utf8", conn));
  return Count(true);
}

bool TestExtMysql::test_mysql_ping() {
  Variant conn = f_mysql_connect(TEST_HOSTNAME, TEST_USERNAME, TEST_PASSWORD);
  VERIFY(f_mysql_ping());
  return Count(true);
}

bool TestExtMysql::test_mysql_escape_string() {
  String item = "Zak's Laptop";
  String escaped_item = f_mysql_escape_string(item);
  VS(escaped_item, "Zak\\'s Laptop");
  return Count(true);
}

bool TestExtMysql::test_mysql_real_escape_string() {
  {
    Variant conn = f_mysql_connect(TEST_HOSTNAME, TEST_USERNAME,
                                   TEST_PASSWORD);
    String item = "Zak's Laptop";
    String escaped_item = f_mysql_real_escape_string(item);
    VS(escaped_item, "Zak\\'s Laptop");
    f_mysql_close(conn);
    VS(f_mysql_real_escape_string(item), false);
  }
  return Count(true);
}

bool TestExtMysql::test_mysql_client_encoding() {
  Variant conn = f_mysql_connect(TEST_HOSTNAME, TEST_USERNAME, TEST_PASSWORD);
  VS(f_mysql_client_encoding(), "latin1");
  return Count(true);
}

bool TestExtMysql::test_mysql_close() {
  Variant conn = f_mysql_connect(TEST_HOSTNAME, TEST_USERNAME, TEST_PASSWORD);
  VERIFY(f_mysql_close(conn));
  return Count(true);
}

bool TestExtMysql::test_mysql_errno() {
  Variant conn = f_mysql_connect(TEST_HOSTNAME, TEST_USERNAME, TEST_PASSWORD);
  VERIFY(!f_mysql_select_db("nonexistentdb").toBoolean());
  VS(f_mysql_errno(conn), 1049);
  return Count(true);
}

bool TestExtMysql::test_mysql_error() {
  Variant conn = f_mysql_connect(TEST_HOSTNAME, TEST_USERNAME, TEST_PASSWORD);
  VERIFY(!f_mysql_select_db("nonexistentdb").toBoolean());
  VS(f_mysql_error(conn), "Unknown database 'nonexistentdb'");
  return Count(true);
}

bool TestExtMysql::test_mysql_warning_count() {
  Variant conn = f_mysql_connect(TEST_HOSTNAME, TEST_USERNAME, TEST_PASSWORD);
  // No warnings from normal operations.
  VERIFY(CreateTestTable());
  VS(f_mysql_warning_count(conn), 0);
  VS(f_mysql_query("INSERT INTO test (name) VALUES ('test'),('test2')"), true);
  VS(f_mysql_warning_count(conn), 0);

  // Dropping a non-existent table with IF EXISTS generates a warning.
  VS(f_mysql_query("DROP TABLE IF EXISTS no_such_table"), true);
  VS(f_mysql_warning_count(conn), 1);

  // Dropping an existing table generates no warnings.
  VS(f_mysql_query("DROP TABLE IF EXISTS test"), true);
  VS(f_mysql_warning_count(conn), 0);

  return Count(true);
}

bool TestExtMysql::test_mysql_get_client_info() {
  VERIFY(!f_mysql_get_client_info().empty());
  return Count(true);
}

bool TestExtMysql::test_mysql_get_host_info() {
  Variant conn = f_mysql_connect(TEST_HOSTNAME, TEST_USERNAME, TEST_PASSWORD);
  VERIFY(!f_mysql_get_host_info().toString().empty());
  return Count(true);
}

bool TestExtMysql::test_mysql_get_proto_info() {
  Variant conn = f_mysql_connect(TEST_HOSTNAME, TEST_USERNAME, TEST_PASSWORD);
  VS(f_mysql_get_proto_info(), 10);
  return Count(true);
}

bool TestExtMysql::test_mysql_get_server_info() {
  Variant conn = f_mysql_connect(TEST_HOSTNAME, TEST_USERNAME, TEST_PASSWORD);
  VERIFY(!f_mysql_get_server_info().toString().empty());
  return Count(true);
}

bool TestExtMysql::test_mysql_info() {
  Variant conn = f_mysql_connect(TEST_HOSTNAME, TEST_USERNAME, TEST_PASSWORD);
  VERIFY(CreateTestTable());
  VS(f_mysql_query("insert into test (name) values ('test'),('test2')"), true);
  VS(f_mysql_info(), "Records: 2  Duplicates: 0  Warnings: 0");
  return Count(true);
}

bool TestExtMysql::test_mysql_insert_id() {
  Variant conn = f_mysql_connect(TEST_HOSTNAME, TEST_USERNAME, TEST_PASSWORD);
  VERIFY(CreateTestTable());
  VS(f_mysql_query("insert into test (name) values ('test'),('test2')"), true);
  VS(f_mysql_insert_id(), 1);
  return Count(true);
}

bool TestExtMysql::test_mysql_stat() {
  Variant conn = f_mysql_connect(TEST_HOSTNAME, TEST_USERNAME, TEST_PASSWORD);
  VERIFY(!f_mysql_stat().toString().empty());
  return Count(true);
}

bool TestExtMysql::test_mysql_thread_id() {
  Variant conn = f_mysql_connect(TEST_HOSTNAME, TEST_USERNAME, TEST_PASSWORD);
  VERIFY(more(f_mysql_thread_id(), 0));
  return Count(true);
}

bool TestExtMysql::test_mysql_create_db() {
  try {
    f_mysql_create_db("");
  } catch (const NotSupportedException& e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtMysql::test_mysql_select_db() {
  // tested in mysql_info
  return Count(true);
}

bool TestExtMysql::test_mysql_drop_db() {
  try {
    f_mysql_drop_db("");
  } catch (const NotSupportedException& e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtMysql::test_mysql_affected_rows() {
  Variant conn = f_mysql_connect(TEST_HOSTNAME, TEST_USERNAME, TEST_PASSWORD);
  VERIFY(CreateTestTable());
  VS(f_mysql_query("insert into test (name) values ('test'),('test2')"), true);
  VS(f_mysql_affected_rows(), 2);
  return Count(true);
}

bool TestExtMysql::test_mysql_set_timeout() {
  VERIFY(f_mysql_set_timeout(10));
  Variant conn = f_mysql_connect(TEST_HOSTNAME, TEST_USERNAME, TEST_PASSWORD,
                                 true, 0, 20, 50);
  VERIFY(!same(conn, false));
  return Count(true);
}

#ifdef MYSQL_MILLISECOND_TIMEOUT

// A random and hopefully unroutable and/or distant address.
#define UNROUTABLE_DESTINATION "10.76.184.41:1"

bool TestExtMysql::test_mysql_subsecond_timeout() {
  struct timeval before;
  gettimeofday(&before, NULL);
  Variant conn = f_mysql_connect(UNROUTABLE_DESTINATION,
                                 TEST_USERNAME, TEST_PASSWORD,
                                 true, /* new_link */
                                 0, /* client flags */
                                 1, /* connect ms */
                                 1 /* query ms */);

  VS(conn, false);
  // CR_CONN_HOST_ERROR which is a client-generated timeout and what
  // libmysqlclient.so will generate.
  VS(f_mysql_errno(), CR_CONN_HOST_ERROR);

  // Verify our perceived timeout of 1ms wasn't turned into a 1s
  // timeout by checking that the entire connect took less than 10ms.
  struct timeval after;
  gettimeofday(&after, NULL);
  const size_t delta_usec = 1000 * 1000 * (after.tv_sec - before.tv_sec) +
    (after.tv_usec - before.tv_usec);
  if (delta_usec > 10 * 1000) {
    LOG_TEST_ERROR("mysql timeout took too long: %.2f ms", delta_usec / 1000.0);
    Count(false);
  }
  return Count(true);
}
#endif

bool TestExtMysql::test_mysql_query() {
  Variant conn = f_mysql_connect(TEST_HOSTNAME, TEST_USERNAME, TEST_PASSWORD);
  VERIFY(CreateTestTable());
  VS(f_mysql_query("insert into test (name) values ('test'),('test2')"), true);

  Variant res = f_mysql_query("select * from test");
  Variant row = f_mysql_fetch_assoc(res);
  VS(f_print_r(row, true),
     "Array\n"
     "(\n"
     "    [id] => 1\n"
     "    [name] => test\n"
     ")\n");

  row = f_mysql_fetch_assoc(res);
  VS(f_print_r(row, true),
     "Array\n"
     "(\n"
     "    [id] => 2\n"
     "    [name] => test2\n"
     ")\n");

  row = f_mysql_fetch_assoc(res);
  VS(row, false);
  return Count(true);
}

bool TestExtMysql::test_mysql_unbuffered_query() {
  Variant conn = f_mysql_connect(TEST_HOSTNAME, TEST_USERNAME, TEST_PASSWORD);
  VERIFY(CreateTestTable());
  VS(f_mysql_query("insert into test (name) values ('test'),('test2')"), true);

  Variant res = f_mysql_unbuffered_query("select * from test");
  Variant row = f_mysql_fetch_assoc(res);
  VS(f_print_r(row, true),
     "Array\n"
     "(\n"
     "    [id] => 1\n"
     "    [name] => test\n"
     ")\n");

  row = f_mysql_fetch_assoc(res);
  VS(f_print_r(row, true),
     "Array\n"
     "(\n"
     "    [id] => 2\n"
     "    [name] => test2\n"
     ")\n");

  row = f_mysql_fetch_assoc(res);
  VS(row, false);
  return Count(true);
}

bool TestExtMysql::test_mysql_db_query() {
  try {
    f_mysql_db_query("", "");
  } catch (const NotSupportedException& e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtMysql::test_mysql_list_dbs() {
  static const StaticString s_Database("Database");
  Variant conn = f_mysql_connect(TEST_HOSTNAME, TEST_USERNAME, TEST_PASSWORD);
  Variant res = f_mysql_list_dbs();
  Variant db = f_mysql_fetch_assoc(res);
  if (db[s_Database].toString().empty()) {
    return CountSkip();
  }
  return Count(true);
}

bool TestExtMysql::test_mysql_list_tables() {
  Variant conn = f_mysql_connect(TEST_HOSTNAME, TEST_USERNAME, TEST_PASSWORD);
  Variant res = f_mysql_list_tables(TEST_DATABASE);
  Variant table = f_mysql_fetch_assoc(res);
  VERIFY(!table[String("Tables_in_") + TEST_DATABASE].toString().empty());
  return Count(true);
}

bool TestExtMysql::test_mysql_list_fields() {
  try {
    Variant conn = f_mysql_connect(TEST_HOSTNAME, TEST_USERNAME,
                                   TEST_PASSWORD);
    f_mysql_list_fields(TEST_DATABASE, "test");
  } catch (const NotSupportedException& e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtMysql::test_mysql_list_processes() {
  static const StaticString s_Id("Id");
  Variant conn = f_mysql_connect(TEST_HOSTNAME, TEST_USERNAME, TEST_PASSWORD);
  Variant res = f_mysql_list_processes();
  Variant process = f_mysql_fetch_assoc(res);
  VERIFY(!process[s_Id].toString().empty());
  return Count(true);
}

bool TestExtMysql::test_mysql_db_name() {
  Variant conn = f_mysql_connect(TEST_HOSTNAME, TEST_USERNAME, TEST_PASSWORD);
  Variant dbs = f_mysql_list_dbs();
  if (f_mysql_db_name(dbs, 0).toString().empty()) {
    return CountSkip();
  }
  return Count(true);
}

bool TestExtMysql::test_mysql_tablename() {
  Variant conn = f_mysql_connect(TEST_HOSTNAME, TEST_USERNAME, TEST_PASSWORD);
  VERIFY(CreateTestTable());
  Variant tables = f_mysql_list_tables(TEST_DATABASE);
  VERIFY(!f_mysql_tablename(tables, 0).toString().empty());
  return Count(true);
}

bool TestExtMysql::test_mysql_num_fields() {
  Variant conn = f_mysql_connect(TEST_HOSTNAME, TEST_USERNAME, TEST_PASSWORD);
  VERIFY(CreateTestTable());
  VS(f_mysql_query("insert into test (name) values ('test'),('test2')"), true);

  Variant res = f_mysql_query("select * from test");
  VS(f_mysql_num_fields(res), 2);
  return Count(true);
}

bool TestExtMysql::test_mysql_num_rows() {
  Variant conn = f_mysql_connect(TEST_HOSTNAME, TEST_USERNAME, TEST_PASSWORD);
  VERIFY(CreateTestTable());
  VS(f_mysql_query("insert into test (name) values ('test'),('test2')"), true);

  Variant res = f_mysql_query("select * from test");
  VS(f_mysql_num_rows(res), 2);
  return Count(true);
}

bool TestExtMysql::test_mysql_free_result() {
  Variant conn = f_mysql_connect(TEST_HOSTNAME, TEST_USERNAME, TEST_PASSWORD);
  VERIFY(CreateTestTable());
  VS(f_mysql_query("insert into test (name) values ('test'),('test2')"), true);

  Variant res = f_mysql_query("select * from test");
  f_mysql_free_result(res);
  return Count(true);
}

bool TestExtMysql::test_mysql_data_seek() {
  Variant conn = f_mysql_connect(TEST_HOSTNAME, TEST_USERNAME, TEST_PASSWORD);
  VERIFY(CreateTestTable());
  VS(f_mysql_query("insert into test (name) values ('test'),('test2')"), true);

  Variant res = f_mysql_query("select * from test");
  f_mysql_data_seek(res, 1);
  Variant row = f_mysql_fetch_assoc(res);
  VS(f_print_r(row, true),
     "Array\n"
     "(\n"
     "    [id] => 2\n"
     "    [name] => test2\n"
     ")\n");

  return Count(true);
}

bool TestExtMysql::test_mysql_fetch_row() {
  Variant conn = f_mysql_connect(TEST_HOSTNAME, TEST_USERNAME, TEST_PASSWORD);
  VERIFY(CreateTestTable());
  VS(f_mysql_query("insert into test (name) values ('test'),('test2')"), true);

  Variant res = f_mysql_query("select * from test");
  Variant row = f_mysql_fetch_row(res);
  VS(f_print_r(row, true),
     "Array\n"
     "(\n"
     "    [0] => 1\n"
     "    [1] => test\n"
     ")\n");
  return Count(true);
}

bool TestExtMysql::test_mysql_fetch_assoc() {
  Variant conn = f_mysql_connect(TEST_HOSTNAME, TEST_USERNAME, TEST_PASSWORD);
  VERIFY(CreateTestTable());
  VS(f_mysql_query("insert into test (name) values ('test'),('test2')"), true);

  Variant res = f_mysql_query("select * from test");
  Variant row = f_mysql_fetch_assoc(res);
  VS(f_print_r(row, true),
     "Array\n"
     "(\n"
     "    [id] => 1\n"
     "    [name] => test\n"
     ")\n");
  return Count(true);
}

bool TestExtMysql::test_mysql_fetch_array() {
  Variant conn = f_mysql_connect(TEST_HOSTNAME, TEST_USERNAME, TEST_PASSWORD);
  VERIFY(CreateTestTable());
  VS(f_mysql_query("insert into test (name) values ('test'),('test2')"), true);

  Variant res = f_mysql_query("select * from test");
  Variant row = f_mysql_fetch_array(res);
  VS(f_print_r(row, true),
     "Array\n"
     "(\n"
     "    [0] => 1\n"
     "    [id] => 1\n"
     "    [1] => test\n"
     "    [name] => test\n"
     ")\n");
  return Count(true);
}

bool TestExtMysql::test_mysql_fetch_lengths() {
  Variant conn = f_mysql_connect(TEST_HOSTNAME, TEST_USERNAME, TEST_PASSWORD);
  VERIFY(CreateTestTable());
  VS(f_mysql_query("insert into test (name) values ('test'),('test2')"), true);

  Variant res = f_mysql_query("select * from test");
  Variant row = f_mysql_fetch_row(res);
  Variant lengths = f_mysql_fetch_lengths(res);
  VS(f_print_r(lengths, true),
     "Array\n"
     "(\n"
     "    [0] => 1\n"
     "    [1] => 4\n"
     ")\n");

  // A much more intense test on lengths
  f_mysql_query("drop table testlen");
  VS(f_mysql_query("create table testlen (id int not null auto_increment, "
                   "d decimal(10,5), t tinyint, i int, b bigint, f float, "
                   "db double, y2 year(2), y4 year(4), primary key (id)) "
                   "engine=innodb"), true);
  VS(f_mysql_query("insert into testlen(d, t, i, b, f, db, y2, y4) values"
                   "(.343, null, 384, -1, 03.44, -03.43892874e101, 00, 0000)"),
     true);
  res = f_mysql_query("select * from testlen");
  row = f_mysql_fetch_row(res);
  lengths = f_mysql_fetch_lengths(res);
  VS(f_print_r(lengths, true),
     "Array\n"
     "(\n"
     "    [0] => 1\n"
     "    [1] => 7\n"
     "    [2] => 0\n"
     "    [3] => 3\n"
     "    [4] => 2\n"
     "    [5] => 4\n"
     "    [6] => 16\n"
     "    [7] => 2\n"
     "    [8] => 4\n"
     ")\n");

  return Count(true);
}

bool TestExtMysql::test_mysql_fetch_object() {
  Variant conn = f_mysql_connect(TEST_HOSTNAME, TEST_USERNAME, TEST_PASSWORD);
  VERIFY(CreateTestTable());
  VS(f_mysql_query("insert into test (name) values ('test'),('test2')"), true);

  Variant res = f_mysql_query("select * from test");
  Variant row = f_mysql_fetch_object(res);
  VS(row.toObject()->o_get("name"), "test");
  return Count(true);
}

bool TestExtMysql::test_mysql_result() {
  Variant conn = f_mysql_connect(TEST_HOSTNAME, TEST_USERNAME, TEST_PASSWORD);
  VERIFY(CreateTestTable());
  VS(f_mysql_query("insert into test (name) values ('test'),('test2')"), true);

  Variant res = f_mysql_query("select * from test");
  VS(f_mysql_result(res, 1, 1), "test2");
  return Count(true);
}

bool TestExtMysql::test_mysql_fetch_field() {
  Variant conn = f_mysql_connect(TEST_HOSTNAME, TEST_USERNAME, TEST_PASSWORD);
  VERIFY(CreateTestTable());
  VS(f_mysql_query("insert into test (name) values ('test'),('test2')"), true);

  Variant res = f_mysql_query("select * from test");
  VS(f_mysql_fetch_field(res, 1).toObject()->o_get("name"), "name");
  return Count(true);
}

bool TestExtMysql::test_mysql_field_seek() {
  Variant conn = f_mysql_connect(TEST_HOSTNAME, TEST_USERNAME, TEST_PASSWORD);
  VERIFY(CreateTestTable());
  VS(f_mysql_query("insert into test (name) values ('test'),('test2')"), true);

  Variant res = f_mysql_query("select * from test");
  VERIFY(f_mysql_field_seek(res, 1));
  VS(f_mysql_fetch_field(res).toObject()->o_get("name"), "name");
  return Count(true);
}

bool TestExtMysql::test_mysql_field_name() {
  Variant conn = f_mysql_connect(TEST_HOSTNAME, TEST_USERNAME, TEST_PASSWORD);
  VERIFY(CreateTestTable());
  VS(f_mysql_query("insert into test (name) values ('test'),('test2')"), true);

  Variant res = f_mysql_query("select * from test");
  VS(f_mysql_field_name(res, 1), "name");
  return Count(true);
}

bool TestExtMysql::test_mysql_field_table() {
  Variant conn = f_mysql_connect(TEST_HOSTNAME, TEST_USERNAME, TEST_PASSWORD);
  VERIFY(CreateTestTable());
  VS(f_mysql_query("insert into test (name) values ('test'),('test2')"), true);

  Variant res = f_mysql_query("select * from test");
  VS(f_mysql_field_table(res, 1), "test");
  return Count(true);
}

bool TestExtMysql::test_mysql_field_len() {
  Variant conn = f_mysql_connect(TEST_HOSTNAME, TEST_USERNAME, TEST_PASSWORD);
  VERIFY(CreateTestTable());
  VS(f_mysql_query("insert into test (name) values ('test'),('test2')"), true);

  Variant res = f_mysql_query("select * from test");
  VS(f_mysql_field_len(res, 1), 255);
  return Count(true);
}

bool TestExtMysql::test_mysql_field_type() {
  Variant conn = f_mysql_connect(TEST_HOSTNAME, TEST_USERNAME, TEST_PASSWORD);
  VERIFY(CreateTestTable());
  VS(f_mysql_query("insert into test (name) values ('test'),('test2')"), true);

  Variant res = f_mysql_query("select * from test");
  VS(f_mysql_field_type(res, 1), "string");
  return Count(true);
}

bool TestExtMysql::test_mysql_field_flags() {
  Variant conn = f_mysql_connect(TEST_HOSTNAME, TEST_USERNAME, TEST_PASSWORD);
  VERIFY(CreateTestTable());
  VS(f_mysql_query("insert into test (name) values ('test'),('test2')"), true);

  Variant res = f_mysql_query("select * from test");
  VS(f_mysql_field_flags(res, 0), "not_null primary_key auto_increment");
  return Count(true);
}
