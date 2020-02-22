#include <string>

#include <folly/Range.h>
#include <folly/experimental/TestUtil.h>
#include <folly/portability/GTest.h>

#include "hphp/util/sqlite-wrapper.h"

using namespace HPHP;

TEST(SQLiteWrapperTest, StoreData) {
  SQLite db = SQLite::connect(":memory:");

  SQLiteTxn txn = db.begin();

  txn.exec("CREATE TABLE foo (bar)");
  txn.exec("INSERT INTO foo (bar) VALUES (1)");
  txn.exec("INSERT INTO foo (bar) VALUES (2)");

  SQLiteStmt getStmt = db.prepare("SELECT bar FROM foo");
  SQLiteQuery query = txn.query(getStmt);

  ASSERT_FALSE(query.done());
  ASSERT_FALSE(query.row());

  query.step();
  ASSERT_FALSE(query.done());
  ASSERT_TRUE(query.row());
  EXPECT_EQ(query.getInt(0), 1);

  query.step();
  ASSERT_FALSE(query.done());
  ASSERT_TRUE(query.row());
  EXPECT_EQ(query.getInt(0), 2);

  query.step();
  EXPECT_TRUE(query.done());
  EXPECT_FALSE(query.row());
}

TEST(SQLiteWrapperTest, BindInt) {
  SQLite db = SQLite::connect(":memory:");

  SQLiteTxn txn = db.begin();
  txn.exec("CREATE TABLE foo (bar)");

  SQLiteStmt insertStmt = db.prepare("INSERT INTO foo VALUES (@v)");
  for (auto i = 0; i < 10; i++) {
    SQLiteQuery query = txn.query(insertStmt);

    query.bindInt("@v", i);
    query.step();
  }

  SQLiteStmt selectStmt = db.prepare("SELECT bar FROM foo");
  SQLiteQuery query = txn.query(selectStmt);
  for (auto i = 0; i < 10; i++) {
    ASSERT_FALSE(query.done());
    query.step();
    ASSERT_TRUE(query.row());
    EXPECT_EQ(query.getInt(0), i);
  }
}

TEST(SQLiteWrapperTest, Rollback) {
  auto db = SQLite::connect(":memory:");

  {
    auto txn = db.begin();
    txn.exec("CREATE TABLE foo (bar)");
    txn.commit();
  }

  {
    auto txn = db.begin();
    txn.exec("INSERT INTO foo VALUES (1)");
    // rollback
  }

  {
    auto txn = db.begin();
    auto stmt = db.prepare("SELECT bar FROM foo");
    auto query = txn.query(stmt);
    query.step();
    EXPECT_TRUE(query.done());
  }
}

TEST(SQLiteWrapperTest, MoveConstructor) {
  SQLite db = SQLite::connect(":memory:");

  {
    SQLiteTxn txn = db.begin();
    txn.exec("CREATE TABLE foo (bar)");
    txn.commit();
    SQLiteStmt insertStmt = db.prepare("INSERT INTO foo VALUES (@v)");
    for (auto i = 0; i < 10; i++) {
      SQLiteQuery query = txn.query(insertStmt);

      query.bindInt("@v", i);
      query.step();
    }
  }

  SQLite db2 = std::move(db);

  SQLiteTxn txn = db2.begin();
  SQLiteStmt selectStmt = db2.prepare("SELECT bar FROM foo");
  SQLiteQuery query = txn.query(selectStmt);
  for (auto i = 0; i < 10; i++) {
    ASSERT_FALSE(query.done());
    query.step();
    ASSERT_TRUE(query.row());
    EXPECT_EQ(query.getInt(0), i);
  }
}

TEST(SQLiteWrapperTest, ReadOnlyDB) {
  folly::test::TemporaryDirectory m_tmpdir{"sqlite-wrapper-readonly"};
  auto dbPath = m_tmpdir.path() / "db.sql3";

  SQLite db = SQLite::connect(dbPath.native());

  {
    SQLiteTxn txn = db.begin();
    txn.exec("CREATE TABLE foo (bar)");
    SQLiteStmt insertStmt = db.prepare("INSERT INTO foo VALUES (@v)");
    for (auto i = 0; i < 10; i++) {
      SQLiteQuery query = txn.query(insertStmt);

      query.bindInt("@v", i);
      query.step();
    }
    txn.commit();
  }

  SQLite db2 = SQLite::connect(dbPath.native(), SQLite::OpenMode::ReadOnly);

  SQLiteTxn txn = db2.begin();
  SQLiteStmt selectStmt = db2.prepare("SELECT bar FROM foo");
  SQLiteQuery query = txn.query(selectStmt);
  for (auto i = 0; i < 10; i++) {
    ASSERT_FALSE(query.done());
    query.step();
    ASSERT_TRUE(query.row());
    EXPECT_EQ(query.getInt(0), i);
  }
}
