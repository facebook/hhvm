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

#include <test/test_ext_sqlite3.h>
#include <runtime/ext/ext_sqlite3.h>
#include <runtime/ext/ext_file.h>

///////////////////////////////////////////////////////////////////////////////

bool TestExtSqlite3::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_sqlite3);
  RUN_TEST(test_sqlite3stmt);
  RUN_TEST(test_sqlite3result);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtSqlite3::test_sqlite3() {
  sp_sqlite3 db(NEW(c_sqlite3)());
  db->t_open(":memory:test");
  db->t_exec("DROP TABLE foo");
  db->t_exec("CREATE TABLE foo (bar STRING)");

  db->t_exec("INSERT INTO foo VALUES ('ABC')");
  db->t_exec("INSERT INTO foo VALUES ('DEF')");
  VS(db->t_lastinsertrowid(), 2);
  VS(db->t_changes(), 1);
  VS(db->t_lasterrorcode(), 0);
  VS(db->t_lasterrormsg(), "not an error");

  VS(db->t_escapestring("'\""), "''\"");
  VS(db->t_querysingle("SELECT * FROM foo"), "ABC");
  VS(db->t_querysingle("SELECT * FROM foo", true), CREATE_MAP1("bar", "ABC"));

  // testing query() and sqlite3result
  {
    Object objResult = db->t_query("SELECT * FROM foo").toObject();
    c_sqlite3result *res = objResult.getTyped<c_sqlite3result>();

    VS(res->t_fetcharray(), CREATE_MAP2(0, "ABC", "bar", "ABC"));
    VS(res->t_numcolumns(), 1);
    VS(res->t_columnname(0), "bar");
    VS(res->t_columntype(0), k_SQLITE3_TEXT);

    VS(res->t_fetcharray(k_SQLITE3_NUM), CREATE_VECTOR1("DEF"));
  }

  // testing prepare() and sqlite3stmt
  {
    Object objStmt = db->t_prepare("SELECT * FROM foo WHERE bar = :id");
    c_sqlite3stmt *stmt = objStmt.getTyped<c_sqlite3stmt>();
    VS(stmt->t_paramcount(), 1);

    Variant id = "DEF";
    VERIFY(stmt->t_bindvalue(":id", ref(id), SQLITE3_TEXT));
    id = "ABC";
    {
      Object objResult = stmt->t_execute();
      c_sqlite3result *res = objResult.getTyped<c_sqlite3result>();
      VS(res->t_fetcharray(k_SQLITE3_NUM), CREATE_VECTOR1("DEF"));
    }

    VERIFY(stmt->t_clear());
    VERIFY(stmt->t_reset());
    id = "DEF";
    VERIFY(stmt->t_bindparam(":id", ref(id), SQLITE3_TEXT));
    id = "ABC";
    {
      Object objResult = stmt->t_execute();
      c_sqlite3result *res = objResult.getTyped<c_sqlite3result>();
      VS(res->t_fetcharray(k_SQLITE3_NUM), CREATE_VECTOR1("ABC"));
    }
  }

  // testing UDF
  {
    VERIFY(db->t_createfunction("tolower", "lower", 1));
    Object objResult = db->t_query("SELECT tolower(bar) FROM foo").toObject();
    c_sqlite3result *res = objResult.getTyped<c_sqlite3result>();
    VS(res->t_fetcharray(k_SQLITE3_NUM), CREATE_VECTOR1("abc"));
  }
  {
    VERIFY(db->t_createaggregate("sumlen", "sumlen_step", "sumlen_fini", 1));
    Object objResult = db->t_query("SELECT sumlen(bar) FROM foo").toObject();
    c_sqlite3result *res = objResult.getTyped<c_sqlite3result>();
    VS(res->t_fetcharray(k_SQLITE3_NUM), CREATE_VECTOR1(6));
  }

  db->t_close();

  VS(db->t_version(),
     CREATE_MAP2("versionString", "3.6.23.1", "versionNumber", 3006023));
  f_unlink(":memory:test");
  return Count(true);
}

bool TestExtSqlite3::test_sqlite3stmt() {
  // tested in test_sqlite3()
  return Count(true);
}

bool TestExtSqlite3::test_sqlite3result() {
  // tested in test_sqlite3()
  return Count(true);
}
