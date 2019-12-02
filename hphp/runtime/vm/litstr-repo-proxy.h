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

#ifndef incl_HPHP_VM_LITSTR_REPO_PROXY_H_
#define incl_HPHP_VM_LITSTR_REPO_PROXY_H_

#include "hphp/runtime/vm/repo-helpers.h"
#include "hphp/runtime/vm/repo-status.h"

#include <atomic>

namespace HPHP {

// Only one of the local/central repos can contain a global litstr table, and
// the repo id is recorded as s_loadedRepoId.
struct LitstrRepoProxy : RepoProxy {
  explicit LitstrRepoProxy(Repo& repo) : RepoProxy(repo) {}
  ~LitstrRepoProxy();

  void createSchema(int repoId, RepoTxn& txn); // throws(RepoExc)

  void load(bool lazy);                // only called during initialization
  void loadAll();

  // Return the literal with a given id from the loaded repo. This works when
  // called concurrently from multiple threads.
  StringData* loadOne(Id id);

  struct InsertLitstrStmt : RepoProxy::Stmt {
    InsertLitstrStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void insert(RepoTxn& txn, Id litstrId, const StringData* litstr);
    // throws(RepoExc)
  };

  struct GetLitstrCountStmt : RepoProxy::Stmt {
    GetLitstrCountStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    // Return the max Id of all string literals, or -1 on error.
    int get();
  };

  struct GetOneLitstrStmt : RepoProxy::Stmt {
    GetOneLitstrStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    // Return either a static string, or nullptr on error. The static string
    // will be registered in the global LitstrTable
    StringData* getOne(int litstrId);
  };

  struct GetLitstrsStmt : RepoProxy::Stmt {
    GetLitstrsStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    // Load all Litstrs to the global LitstrTable
    RepoStatus get();
  };

private:
  bool m_inited{false};
  union {
    GetOneLitstrStmt m_litstrLoader;    // valid when m_init is true
  };
  static std::atomic_int s_loadedRepoId;
};

}

#endif
