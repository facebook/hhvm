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

#pragma once

#include <vector>
#include <utility>
#include <string>
#include <memory>
#include <cstdlib>

#include <sqlite3.h>

// For getpwuid_r(3).
#include <sys/types.h>
#include <pwd.h>

#include "hphp/runtime/base/repo-autoload-map.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/litstr-repo-proxy.h"
#include "hphp/runtime/vm/preclass-emitter.h"
#include "hphp/runtime/vm/record-emitter.h"
#include "hphp/runtime/vm/repo-global-data.h"
#include "hphp/runtime/vm/repo-status.h"
#include "hphp/runtime/vm/unit-emitter.h"

#include <folly/portability/Unistd.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct RepoAutoloadMapBuilder;

namespace Native {
struct FuncTable;
}

struct Repo : RepoProxy {
  // Do not directly instantiate this class; a thread-local creates one per
  // thread on demand when Repo::get() is called.
  static Repo& get();
  // Prefork is called before forking. It attempts to shut down other
  // threads and returns true if forking should be prevented, false if
  // it's ok to proceed.
  static bool prefork();
  static void postfork(pid_t pid);

  /*
   * In some command line programs that use the repo, it is necessary
   * to shut it down at some point in the process.  (See hhbbc.)  This
   * function accomplishes this.
   */
  static void shutdown();

  Repo();
  ~Repo() noexcept;
  Repo(const Repo&) = delete;
  Repo& operator=(const Repo&) = delete;

  const char* dbName(int repoId) const {
    assertx(repoId < RepoIdCount);
    return kDbs[repoId];
  }

  int8_t numOpenRepos() const {
    return m_numOpenRepos;
  }

  sqlite3* dbc() const { return m_dbc; }
  int repoIdForNewUnit(UnitOrigin unitOrigin) const {
    switch (unitOrigin) {
    case UnitOrigin::File:
      if (m_localWritable) {
        return RepoIdLocal;
      }
      if (m_centralWritable) {
        return RepoIdCentral;
      }
      return RepoIdInvalid;
    case UnitOrigin::Eval:
      return RepoIdInvalid;
    default:
      assertx(false);
      return RepoIdInvalid;
    }
  }
  std::string repoName(int repoId) const {
    switch (repoId) {
    case RepoIdLocal: return m_localRepo;
    case RepoIdCentral: return m_centralRepo;
    default: return "?";
    }
  }

  UnitRepoProxy& urp() { return m_urp; }
  PreClassRepoProxy& pcrp() { return m_pcrp; }
  RecordRepoProxy& rrp() { return m_rrp; }
  FuncRepoProxy& frp() { return m_frp; }
  LitstrRepoProxy& lsrp() { return m_lsrp; }

  static void setCliFile(const std::string& cliFile);

  std::unique_ptr<Unit> loadUnit(const folly::StringPiece name,
                                 const SHA1& sha1,
                                 const Native::FuncTable&);
  void forgetUnit(const std::string& path);
  RepoStatus findFile(const char* path, const std::string& root, SHA1& sha1);
  std::optional<String> findPath(int64_t unitSn, const std::string& root);
  RepoStatus findUnit(const char* path, const std::string& root, int64_t& unitSn);
  RepoStatus insertSha1(UnitOrigin unitOrigin, UnitEmitter* ue, RepoTxn& txn);
  void commitSha1(UnitOrigin unitOrigin, UnitEmitter* ue);

  /*
   * Return the largest size for a static string that can be inserted into the
   * repo.
   */
  size_t stringLengthLimit() const;

  /*
   * Return a vector of (filepath, SHA1) for every unit in central
   * repo.
   */
  std::vector<std::pair<std::string,SHA1>> enumerateUnits(
    int repoId, bool warn);

  /*
   * Check if the repo has global data. If it does the repo was built using
   * WholeProgram mode.
   */
  bool hasGlobalData();

  /*
   * Load the repo-global metadata table, including the global litstr
   * table.  Normally called during process initialization.
   */
  void loadGlobalData(bool readGlobalTables = true);

  /*
   * Access to global data.
   *
   * Pre: loadGlobalData() already called, and
   * RuntimeOption::RepoAuthoritative.
   */
  static const RepoGlobalData& global() {
    assertx(RuntimeOption::RepoAuthoritative);
    return s_globalData;
  }

  /*
   * Used during repo creation to associate the supplied
   * RepoGlobalData with the repo that was being built.  Also saves
   * the global litstr table.
   *
   * No other threads may be reading or writing the repo
   * RepoGlobalData when this is called.
   */
  void saveGlobalData(RepoGlobalData&& newData,
                      const RepoAutoloadMapBuilder& autoloadMapBuilder);

 private:
  /*
   * RepoStmts for setting/getting file hashes.
   */
  struct InsertFileHashStmt : public RepoProxy::Stmt {
    InsertFileHashStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void insert(RepoTxn& txn, const StringData* path, const SHA1& sha1);
    // throws(RepoExc)
  };

  struct GetFileHashStmt : public RepoProxy::Stmt {
    GetFileHashStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    RepoStatus get(const char* path, SHA1& sha1);
  };

  struct RemoveFileHashStmt : public RepoProxy::Stmt {
    RemoveFileHashStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void remove(RepoTxn& txn, const std::string& path);
  };

  struct GetUnitPathStmt : public RepoProxy::Stmt {
    GetUnitPathStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    std::optional<String> get(int64_t unitSn);
  };

  struct GetUnitStmt : public RepoProxy::Stmt {
    GetUnitStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    RepoStatus get(const char* path, int64_t& unitSn);
  };

  InsertFileHashStmt m_insertFileHash[RepoIdCount];
  GetFileHashStmt m_getFileHash[RepoIdCount];
  RemoveFileHashStmt m_removeFileHash[RepoIdCount];
  GetUnitPathStmt m_getUnitPath[RepoIdCount];
  GetUnitStmt m_getUnit[RepoIdCount];

 public:
  std::string table(int repoId, const char* tablePrefix);
  void exec(const std::string& sQuery); // throws(RepoExc)

  RepoTxn begin(); // throws(RepoExc)
 private:
  friend struct RepoTxn;
  void txPop(); // throws(RepoExc)
  void rollback(); // nothrow
  void commit(); // throws(RepoExc)
 public:
  RepoStatus insertUnit(UnitEmitter* ue, UnitOrigin unitOrigin,
                        RepoTxn& txn, bool usePreAllocatedUnitSn); // nothrow
  void commitUnit(UnitEmitter* ue, UnitOrigin unitOrigin,
                  bool usePreAllocatedUnitSn); // nothrow

  static bool s_deleteLocalOnFailure;
  // All database table names use the schema ID (sha1 checksum based on the
  // source code) as a suffix.  For example, if the schema ID is
  // "b02c58478ce89719782fea89f3009295faceb00c", the file magic is stored in the
  // magic_b02c58478ce89719782fea89f3009295faceb00c table:
  //
  //   CREATE TABLE magic_b02c58478ce89719782fea89f3009295faceb00c(
  //     product[TEXT]);
  //   INSERT INTO magic_b02c58478ce89719782fea89f3009295faceb00c VALUES(
  //     'facebook.com HipHop Virtual Machine bytecode repository');
  //
  // This allows multiple schemas to coexist in the same database, which is
  // especially important if multiple versions of hhvm are in use at the same
  // time.
 private:
  // Magic product constant used to distinguish a .hhbc database.
  static const char* kMagicProduct;
  static const char* kDbs[RepoIdCount];

  void connect();
  void disconnect() noexcept;
  bool initCentral();
  RepoStatus openCentral(const char* repoPath, std::string& errorMsg);
  bool initLocal();
  bool attachLocal(const char* repoPath, bool isWritable);
  void pragmas(int repoId); // throws(RepoExc)
  void getIntPragma(int repoId, const char* name, int& val); // throws(RepoExc)
  void setIntPragma(int repoId, const char* name, int val); // throws(RepoExc)
  void getTextPragma(int repoId, const char* name, std::string& val);
  // throws(RepoExc)
  void setTextPragma(int repoId, const char* name, const char* val);
  // throws(RepoExc)
  RepoStatus initSchema(int repoId, bool& isWritable, std::string& errorMsg);
  bool schemaExists(int repoId);
  RepoStatus createSchema(int repoId, std::string& errorMsg);
  bool writable(int repoId);

private:
  static std::string s_cliFile;
  static RepoGlobalData s_globalData;

  std::string m_localRepo;
  std::string m_centralRepo;
  sqlite3* m_dbc; // Database connection, shared by multiple attached databases.
  bool m_localWritable;
  bool m_centralWritable;
  unsigned m_txDepth; // Transaction nesting depth.
  bool m_rollback; // If true, rollback rather than commit.
  RepoStmt m_beginStmt;
  RepoStmt m_rollbackStmt;
  RepoStmt m_commitStmt;
  UnitRepoProxy m_urp;
  PreClassRepoProxy m_pcrp;
  RecordRepoProxy m_rrp;
  FuncRepoProxy m_frp;
  LitstrRepoProxy m_lsrp;
  int8_t m_numOpenRepos;
};

//////////////////////////////////////////////////////////////////////

/*
 * Try to commit a vector of unit emitters to the current repo.  Note that
 * errors are ignored!
 */
void batchCommit(const std::vector<std::unique_ptr<UnitEmitter>>&,
                 bool usePreAllocatedUnitSn);

bool batchCommitWithoutRetry(const std::vector<std::unique_ptr<UnitEmitter>>&,
                             bool usePreAllocatedUnitSn);

//////////////////////////////////////////////////////////////////////

}
