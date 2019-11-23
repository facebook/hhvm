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

void LitstrRepoProxy::createSchema(int repoId, RepoTxn& txn) {
  auto insertQuery = folly::sformat(
    "CREATE TABLE {} (litstrId INTEGER PRIMARY KEY, litstr TEXT);",
    m_repo.table(repoId, "Litstr"));
  txn.exec(insertQuery);
}

void LitstrRepoProxy::load() {
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
    namedInfo.resize(maxId + 1, nullptr);
    table.setNamedEntityPairTable(std::move(namedInfo));
    s_loadedRepoId.store(repoId, std::memory_order_release);
    loadAll();
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
  try {
    auto txn = RepoTxn{m_repo.begin()};
    if (!prepared()) {
      auto selectQuery = folly::sformat(
        "SELECT litstrId,litstr FROM {} ORDER BY litstrId;",
        m_repo.table(m_repoId, "Litstr"));
      txn.prepare(*this, selectQuery);
    }
    RepoTxnQuery query(txn, *this);
    NamedEntityPairTable namedInfo;
    namedInfo.emplace_back(nullptr);
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
        namedInfo.emplace_back(litstr);
      }
    } while (!query.done());
    namedInfo.shrink_to_fit();
    LitstrTable::get().setNamedEntityPairTable(std::move(namedInfo));
    txn.commit();
  } catch (RepoExc& re) {
    return RepoStatus::error;
  }
  return RepoStatus::success;
}

}
