/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <test/test_ext_pdo.h>
#include <runtime/ext/ext_mysql.h>
#include <runtime/ext/ext_pdo.h>
#include <runtime/ext/ext_sqlite3.h>
#include <runtime/ext/ext_file.h>
#include <test/test_mysql_info.inc>

using namespace std;

IMPLEMENT_SEP_EXTENSION_TEST(Pdo);
///////////////////////////////////////////////////////////////////////////////

bool TestExtPdo::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_pdo_drivers);
  RUN_TEST(test_pdo_mysql);
  RUN_TEST(test_pdo_sqlite);

  return ret;
}

static void CreateMySqlTestTable() {
  Variant conn = f_mysql_connect(TEST_HOSTNAME, TEST_USERNAME, TEST_PASSWORD);
  f_mysql_select_db(TEST_DATABASE);
  f_mysql_query("drop table test");
  f_mysql_query("create table test (id int not null auto_increment,"
                " name varchar(255) not null, primary key (id)) "
                "engine=innodb");
  f_mysql_query("insert into test (name) values ('test'),('test2')");
}

static void CreateSqliteTestTable() {
  f_unlink("/tmp/foo.db");
  sp_sqlite3 db(NEW(c_sqlite3)());
  db->t_open("/tmp/foo.db");
  db->t_exec("CREATE TABLE foo (bar STRING)");
  db->t_exec("INSERT INTO foo VALUES ('ABC')");
  db->t_exec("INSERT INTO foo VALUES ('DEF')");
}

static void CleanupSqliteTestTable() {
  f_unlink("/tmp/foo.db");
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtPdo::test_pdo_drivers() {
  VERIFY(!f_pdo_drivers().empty());
  return Count(true);
}

bool TestExtPdo::test_pdo_mysql() {
  CreateMySqlTestTable();

  try {
    string source = "mysql:host=";
    source += TEST_HOSTNAME;
    source += ";dbname=";
    source += TEST_DATABASE;

    sp_pdo dbh((NEW(c_pdo)())->

              create(source.c_str(), TEST_USERNAME, TEST_PASSWORD,
                     CREATE_MAP1(q_pdo_ATTR_PERSISTENT, false)));
    Variant vstmt = dbh->t_prepare("select * from test");
    c_pdostatement *stmt = vstmt.toObject().getTyped<c_pdostatement>();
    VERIFY(stmt->t_execute());

    Variant rs = stmt->t_fetch(q_pdo_FETCH_ASSOC);
    VS(rs, CREATE_MAP2("id", "1", "name", "test"));
    rs = stmt->t_fetch(q_pdo_FETCH_ASSOC);
    VS(rs, CREATE_MAP2("id", "2", "name", "test2"));

  } catch (Object &e) {
    VS(e, null);
  }
  return Count(true);
}

bool TestExtPdo::test_pdo_sqlite() {
  CreateSqliteTestTable();

  try {
    string source = "sqlite:/tmp/foo.db";

    sp_pdo dbh((NEW(c_pdo)())->
              create(source.c_str(), TEST_USERNAME, TEST_PASSWORD,
                     CREATE_MAP1(q_pdo_ATTR_PERSISTENT, false)));
    Variant vstmt = dbh->t_prepare("select * from foo");
    c_pdostatement *stmt = vstmt.toObject().getTyped<c_pdostatement>();
    VERIFY(stmt->t_execute());

    Variant rs = stmt->t_fetch(q_pdo_FETCH_ASSOC);
    VS(rs, CREATE_MAP1("bar", "ABC"));
    rs = stmt->t_fetch(q_pdo_FETCH_ASSOC);
    VS(rs, CREATE_MAP1("bar", "DEF"));

  } catch (Object &e) {
    VS(e, null);
  }

  CleanupSqliteTestTable();
  return Count(true);
}
