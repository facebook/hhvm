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

#ifndef incl_HPHP_VM_REPO_H_
#define incl_HPHP_VM_REPO_H_

#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/preclass-emit.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/litstr-repo-proxy.h"

#include <sqlite3.h>

// For sysconf(3).
#include <unistd.h>
// For getpwuid_r(3).
#include <sys/types.h>
#include <pwd.h>

namespace HPHP {

class Repo : public RepoProxy {
 private:
  static SimpleMutex s_lock;
  static unsigned s_nRepos;
 public:
  // Do not directly instantiate this class; a thread-local creates one per
  // thread on demand when Repo::get() is called.
  static Repo& get();
  static bool prefork();
  static void postfork(pid_t pid);
  Repo();
  ~Repo();

  const char* dbName(int repoId) const {
    assert(repoId < RepoIdCount);
    return kDbs[repoId];
  }
  sqlite3* dbc() const { return m_dbc; }
  int repoIdForNewUnit(UnitOrigin unitOrigin) const {
    switch (unitOrigin) {
    case UnitOrigin::File:
      return m_localWritable ? RepoIdLocal : RepoIdCentral;
    case UnitOrigin::Eval:
      return m_evalRepoId;
    default:
      assert(false);
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
  FuncRepoProxy& frp() { return m_frp; }
  LitstrRepoProxy& lsrp() { return m_lsrp; }

  static void setCliFile(const std::string& cliFile);

  void loadLitstrs();
  Unit* loadUnit(const std::string& name, const MD5& md5);
  bool findFile(const char* path, const std::string& root, MD5& md5);
  bool insertMd5(UnitOrigin unitOrigin, UnitEmitter* ue, RepoTxn& txn);
  void commitMd5(UnitOrigin unitOrigin, UnitEmitter *ue);

#define RP_IOP(o) RP_OP(Insert##o, insert##o)
#define RP_GOP(o) RP_OP(Get##o, get##o)
#define RP_OPS \
  RP_IOP(FileHash) \
  RP_GOP(FileHash)
  class InsertFileHashStmt : public RepoProxy::Stmt {
    public:
      InsertFileHashStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
      void insert(RepoTxn& txn, const StringData* path, const MD5& md5);
  };
  class GetFileHashStmt : public RepoProxy::Stmt {
    public:
      GetFileHashStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
      bool get(const char* path, MD5& md5);
  };
#define RP_OP(c, o) \
 public: \
  c##Stmt& o(int repoId) { return *m_##o[repoId]; } \
 private: \
  c##Stmt m_##o##Local; \
  c##Stmt m_##o##Central; \
  c##Stmt* m_##o[RepoIdCount];
  RP_OPS
#undef RP_OP

 public:
  std::string table(int repoId, const char* tablePrefix);
  void exec(const std::string& sQuery);

  void begin();
 private:
  void txPop();
 public:
  void rollback(); // nothrow
  void commit();
  bool insertUnit(UnitEmitter* ue, UnitOrigin unitOrigin,
                  RepoTxn& txn); // nothrow
  void commitUnit(UnitEmitter* ue, UnitOrigin unitOrigin); // nothrow
  void insertLitstrs(RepoTxn& txn, UnitOrigin unitOrigin);

  // All database table names use the schema ID (md5 checksum based on the
  // source code) as a suffix.  For example, if the schema ID is
  // "b02c58478ce89719782fea89f3009295", the file magic is stored in the
  // magic_b02c58478ce89719782fea89f3009295 table:
  //
  //   CREATE TABLE magic_b02c58478ce89719782fea89f3009295(product[TEXT]);
  //   INSERT INTO magic_b02c58478ce89719782fea89f3009295 VALUES(
  //     'facebook.com HipHop Virtual Machine bytecode repository');
  //
  // This allows multiple schemas to coexist in the same database, which is
  // especially important if multiple versions of hhvm are in use at the same
  // time.
 private:
  // Magic product constant used to distinguish a .hhbc database.
  static const char* kMagicProduct;
  static const char* kSchemaPlaceholder;

  static const char* kDbs[RepoIdCount];

  void connect();
  void disconnect();
  void initCentral();
  std::string insertSchema(const char* path);
  bool openCentral(const char* repoPath);
  void initLocal();
  void attachLocal(const char* repoPath, bool isWritable);
  void pragmas(int repoId);
  void getIntPragma(int repoId, const char* name, int& val);
  void setIntPragma(int repoId, const char* name, int val);
  void getTextPragma(int repoId, const char* name, std::string& val);
  void setTextPragma(int repoId, const char* name, const char* val);
  bool initSchema(int repoId, bool& isWritable);
  bool schemaExists(int repoId);
  bool createSchema(int repoId);
  bool writable(int repoId);

  static std::string s_cliFile;
  std::string m_localRepo;
  std::string m_centralRepo;
  sqlite3* m_dbc; // Database connection, shared by multiple attached databases.
  bool m_localReadable;
  bool m_localWritable;
  int m_evalRepoId;
  unsigned m_txDepth; // Transaction nesting depth.
  bool m_rollback; // If true, rollback rather than commit.
  RepoStmt m_beginStmt;
  RepoStmt m_rollbackStmt;
  RepoStmt m_commitStmt;
  UnitRepoProxy m_urp;
  PreClassRepoProxy m_pcrp;
  FuncRepoProxy m_frp;
  LitstrRepoProxy m_lsrp;
};

}

#endif
