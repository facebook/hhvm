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

#ifndef incl_HPHP_VM_LITSTR_REPO_PROXY_H_
#define incl_HPHP_VM_LITSTR_REPO_PROXY_H_

#include "hphp/runtime/vm/repo-helpers.h"

namespace HPHP {

class LitstrRepoProxy : public RepoProxy {
public:
  explicit LitstrRepoProxy(Repo& repo);
  ~LitstrRepoProxy() {}
  void createSchema(int repoId, RepoTxn& txn);
  void load();

  class InsertLitstrStmt : public RepoProxy::Stmt {
  public:
    InsertLitstrStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void insert(RepoTxn& txn, Id litstrId, const StringData* litstr);
  };

  class GetLitstrsStmt : public RepoProxy::Stmt {
  public:
    GetLitstrsStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    bool get();
  };

public:
  InsertLitstrStmt& insertLitstr(int repoId) { return *m_insertLitstr[repoId]; }
  GetLitstrsStmt& getLitstrs(int repoId) { return *m_getLitstrs[repoId]; }

private:
  InsertLitstrStmt m_insertLitstrLocal;
  InsertLitstrStmt m_insertLitstrCentral;
  InsertLitstrStmt* m_insertLitstr[RepoIdCount];

  GetLitstrsStmt m_getLitstrsLocal;
  GetLitstrsStmt m_getLitstrsCentral;
  GetLitstrsStmt* m_getLitstrs[RepoIdCount];
};

}

#endif
