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

#include "hphp/runtime/vm/unit.h"

#include "hphp/runtime/vm/repo.h"

namespace HPHP {

LitstrRepoProxy::LitstrRepoProxy(Repo& repo)
    : RepoProxy(repo)
    , m_insertLitstrLocal(repo, RepoIdLocal)
    , m_insertLitstrCentral(repo, RepoIdCentral)
    , m_getLitstrsLocal(repo, RepoIdLocal)
    , m_getLitstrsCentral(repo, RepoIdCentral) {
  m_insertLitstr[RepoIdLocal] = &m_insertLitstrLocal;
  m_insertLitstr[RepoIdCentral] = &m_insertLitstrCentral;
  m_getLitstrs[RepoIdLocal] = &m_getLitstrsLocal;
  m_getLitstrs[RepoIdCentral] = &m_getLitstrsCentral;
}

void LitstrRepoProxy::createSchema(int repoId, RepoTxn& txn) {
  std::stringstream ssCreate;
  ssCreate << "CREATE TABLE " << m_repo.table(repoId, "Litstr")
           << "(litstrId INTEGER, litstr TEXT,"
              " PRIMARY KEY (litstrId));";
  txn.exec(ssCreate.str());
}

void LitstrRepoProxy::load() {
  for (int repoId = RepoIdCount - 1; repoId >= 0; --repoId) {
    if (!getLitstrs(repoId).get()) {
      break;
    }
  }
}

void LitstrRepoProxy::InsertLitstrStmt::insert(RepoTxn& txn,
                                               Id litstrId,
                                               const StringData* litstr) {
  if (!prepared()) {
    std::stringstream ssInsert;
    ssInsert << "INSERT INTO " << m_repo.table(m_repoId, "Litstr")
             << " VALUES(@litstrId, @litstr);";
    txn.prepare(*this, ssInsert.str());
  }
  RepoTxnQuery query(txn, *this);
  query.bindInt64("@litstrId", litstrId);
  query.bindStaticString("@litstr", litstr);
  query.exec();
}

bool LitstrRepoProxy::GetLitstrsStmt::get() {
  RepoTxn txn(m_repo);
  try {
    if (!prepared()) {
      std::stringstream ssSelect;
      ssSelect << "SELECT litstrId,litstr FROM "
               << m_repo.table(m_repoId, "Litstr");
      txn.prepare(*this, ssSelect.str());
    }
    RepoTxnQuery query(txn, *this);
    do {
      query.step();
      if (query.row()) {
        Id litstrId;        /**/ query.getId(0, litstrId);
        StringData* litstr; /**/ query.getStaticString(1, litstr);
        Id id UNUSED = LitstrTable::get().mergeLitstr(litstr);
        assert(id == litstrId);
      }
    } while (!query.done());
    txn.commit();
  } catch (RepoExc& re) {
    return true;
  }
  return false;
}

}
