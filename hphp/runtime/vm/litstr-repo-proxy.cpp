/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/repo.h"

namespace HPHP {

std::atomic_int LitstrRepoProxy::s_loadedRepoId{RepoIdInvalid};

LitstrRepoProxy::~LitstrRepoProxy() {
  if (!m_inited) return;
  m_litstrLoader.~GetOneLitstrStmt();
}

void LitstrRepoProxy::createSchema(int repoId, RepoTxn& txn) {
  auto insertQuery = folly::sformat(
    "CREATE TABLE {} (litstrId INTEGER PRIMARY KEY, litstr TEXT);",
    m_repo.table(repoId, "Litstr"));
  txn.exec(insertQuery);
}

void LitstrRepoProxy::load(bool lazy) {
  assertx(s_loadedRepoId.load(std::memory_order_relaxed) == RepoIdInvalid);
  for (int repoId = RepoIdCount - 1; repoId >= 0; --repoId) {
    // Return success on the first loaded repo.  In the case of an error we
    // continue on to the next repo.
    GetLitstrCountStmt stmt{m_repo, repoId};
    auto const maxId = stmt.get();
    if (maxId <= 0) continue;

    auto& table = LitstrTable::get();
    assertx(table.numLitstrs() == 0);
    NamedEntityPairTable namedInfo;
    namedInfo.resize(maxId + 1, LowStringPtr{nullptr});
    table.setNamedEntityPairTable(std::move(namedInfo));
    s_loadedRepoId.store(repoId, std::memory_order_release);
    if (!lazy) loadAll();
    break;
  }
  // No repos were loadable.  This is normal for non-repo-authoritative repos.
}

void LitstrRepoProxy::loadAll() {
  auto const repoId = s_loadedRepoId.load(std::memory_order_acquire);
  assertx(repoId != RepoIdInvalid);
  GetLitstrsStmt stmt{m_repo, repoId};
  DEBUG_ONLY auto const ret = stmt.get();
  assertx(ret == RepoStatus::success);
}

StringData* LitstrRepoProxy::loadOne(Id id) {
  if (!m_inited) {
    auto const repoId = s_loadedRepoId.load(std::memory_order_acquire);
    assertx(repoId != RepoIdInvalid);
    new (&m_litstrLoader) GetOneLitstrStmt(m_repo, repoId);
    m_inited = true;
  }
  auto const ret = m_litstrLoader.getOne(id);
  LitstrTable::get().setLitstr(id, ret);
  return ret;
}

void LitstrRepoProxy::InsertLitstrStmt::insert(RepoTxn& txn,
                                               Id litstrId,
                                               const StringData* litstr) {
  if (!prepared()) {
    auto insertQuery = folly::sformat(
      "INSERT INTO {} VALUES(@litstrId, @litstr);",
      m_repo.table(m_repoId, "Litstr"));
    txn.prepare(*this, insertQuery);
  }
  RepoTxnQuery query(txn, *this);
  query.bindInt64("@litstrId", litstrId);
  query.bindStaticString("@litstr", litstr);
  query.exec();
}

int LitstrRepoProxy::GetLitstrCountStmt::get() {
  int count = -1;
  try {
    if (!prepared()) {
      auto selectQuery = folly::sformat("SELECT max(litstrId) FROM {};",
                                        m_repo.table(m_repoId, "Litstr"));
      prepare(selectQuery);
    }
    RepoQuery query(*this);
    query.step();
    if (query.row()) query.getInt(0, count);
  } catch (RepoExc& re) {
    return -1;
  }
  return count;
}

RepoStatus LitstrRepoProxy::GetLitstrsStmt::get() {
  // load all literals in the repo, after lazy loading.
  assertx(s_loadedRepoId.load(std::memory_order_acquire) == m_repoId);

  auto& table = LitstrTable::get();

  try {
    if (!prepared()) {
      auto selectQuery = folly::sformat(
        "SELECT litstrId,litstr FROM {} ORDER BY litstrId;",
        m_repo.table(m_repoId, "Litstr"));
      prepare(selectQuery);
    }
    RepoQuery query(*this);
    int index = 1;
    do {
      query.step();
      if (query.row()) {
        int litstrId;
        query.getInt(0, litstrId);
        always_assert(
          litstrId == index++ && "LitstrId needs to be from 1 to N"
        );
        StringData* litstr; /**/ query.getStaticString(1, litstr);
        table.setLitstr(index, litstr);
      }
    } while (!query.done());
  } catch (RepoExc& re) {
    return RepoStatus::error;
  }
  return RepoStatus::success;
}

StringData* LitstrRepoProxy::GetOneLitstrStmt::getOne(int litstrId) {
  try {
    if (!prepared()) {
      auto selectQuery = folly::sformat(
        "SELECT litstrid,litstr FROM {} WHERE litstrId = @litstrId;",
         m_repo.table(m_repoId, "Litstr"));
      prepare(selectQuery);
    }
    RepoQuery query(*this);
    query.bindInt("@litstrId", litstrId);
    query.step();
    StringData* litstr = nullptr;
    if (query.row()) {
      int index = 0;
      query.getInt(0, index);
      assert(litstrId == index);
      query.getStaticString(1, litstr);
      return litstr;
    }
  } catch (RepoExc& re) {
  }
  return nullptr;
}

}
