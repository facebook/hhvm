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
#include "hphp/runtime/vm/repo.h"

#include <sstream>

#include <folly/Format.h>
#include <folly/Singleton.h>

#include "hphp/runtime/vm/blob-helper.h"
#include "hphp/runtime/vm/repo-global-data.h"
#include "hphp/runtime/server/xbox-server.h"

#include "hphp/util/assertions.h"
#include "hphp/util/async-func.h"
#include "hphp/util/build-info.h"
#include "hphp/util/hugetlb.h"
#include "hphp/util/logger.h"
#include "hphp/util/process.h"
#include "hphp/util/trace.h"

#include <grp.h>
#include <pwd.h>
#include <sys/stat.h>

namespace HPHP {

TRACE_SET_MOD(hhbc);

const char* Repo::kMagicProduct =
  "facebook.com HipHop Virtual Machine bytecode repository";
const char* Repo::kDbs[RepoIdCount] = { "main",   // Central.
                                        "local"}; // Local.
Repo::GlobalData Repo::s_globalData;

void initialize_repo() {
  if (!sqlite3_threadsafe()) {
    TRACE(0, "SQLite was compiled without thread support; aborting\n");
    abort();
  }
  if (sqlite3_config(SQLITE_CONFIG_MULTITHREAD) != SQLITE_OK) {
    TRACE(1, "Failed to set default SQLite multi-threading mode\n");
  }
  if (sqlite3_config(SQLITE_CONFIG_MEMSTATUS, 0) != SQLITE_OK) {
    TRACE(1, "Failed to disable SQLite memory statistics\n");
  }
}

THREAD_LOCAL(Repo, t_dh);

Repo& Repo::get() {
  return *t_dh.get();
}

void Repo::shutdown() {
  t_dh.destroy();
}

static SimpleMutex s_lock;
static std::atomic<unsigned> s_nRepos;

bool Repo::prefork() {
  if (num_1g_pages() > 0) {
    // We put data on the 1G huge pages, and we don't want to do COW upon
    // fork().  If you need to fork(), configure HHVM not to use 1G pages.
    return true;
  }
  if (!t_dh.isNull()) {
    t_dh.destroy();
  }
  s_lock.lock();
  XboxServer::Stop();
  if (s_nRepos > 0 || AsyncFuncImpl::count()) {
    XboxServer::Restart();
    s_lock.unlock();
    return true;
  }
  folly::SingletonVault::singleton()->destroyInstances();
  return false;
}

void Repo::postfork(pid_t pid) {
  folly::SingletonVault::singleton()->reenableInstances();
  XboxServer::Restart();
  if (pid == 0) {
    Logger::ResetPid();
    new (&s_lock) SimpleMutex();
  } else {
    s_lock.unlock();
  }
}

Repo::Repo()
  : RepoProxy(*this),
    m_insertFileHash{InsertFileHashStmt(*this, 0),
                     InsertFileHashStmt(*this, 1)},
    m_getFileHash{GetFileHashStmt(*this, 0), GetFileHashStmt(*this, 1)},
    m_removeFileHash{RemoveFileHashStmt(*this, 0),
                     RemoveFileHashStmt(*this, 1)},
    m_dbc(nullptr), m_localReadable(false), m_localWritable(false),
    m_evalRepoId(-1), m_txDepth(0), m_rollback(false), m_beginStmt(*this),
    m_rollbackStmt(*this), m_commitStmt(*this), m_urp(*this), m_pcrp(*this),
    m_rrp(*this), m_frp(*this), m_lsrp(*this) {

  ++s_nRepos;
  connect();
}

Repo::~Repo() noexcept {
  disconnect();
  --s_nRepos;
}

std::string Repo::s_cliFile;
void Repo::setCliFile(const std::string& cliFile) {
  assertx(s_cliFile.empty());
  assertx(t_dh.isNull());
  s_cliFile = cliFile;
}

size_t Repo::stringLengthLimit() const {
  static const size_t limit = sqlite3_limit(m_dbc, SQLITE_LIMIT_LENGTH, -1);
  return limit;
}

bool Repo::hasGlobalData() {
  for (int repoId = RepoIdCount - 1; repoId >= 0; --repoId) {
    if (repoName(repoId).empty()) {
      // The repo wasn't loadable
      continue;
    }

    RepoStmt stmt(*this);
    const auto& tbl = table(repoId, "GlobalData");
    stmt.prepare(
      folly::sformat(
        "SELECT count(*) FROM {};", tbl
      )
    );
    auto txn = RepoTxn{begin()};
    RepoTxnQuery query(txn, stmt);
    query.step();

    if (!query.row()) {
      return false;
    }

    int val;
    query.getInt(0, val);
    return val != 0;
  }

  return false;
}

void Repo::loadGlobalData(bool readArrayTable /* = true */) {
  if (readArrayTable) m_lsrp.load();

  if (!RuntimeOption::RepoAuthoritative) return;

  std::vector<std::string> failures;

  /*
   * This should probably just go to the Local repo always, except
   * that our unit test suite is currently running RepoAuthoritative
   * tests with the compiled repo as the Central repo.
   */
  for (int repoId = RepoIdCount - 1; repoId >= 0; --repoId) {
    if (repoName(repoId).empty()) {
      // The repo wasn't loadable
      continue;
    }
    try {
      RepoStmt stmt(*this);
      const auto& tbl = table(repoId, "GlobalData");
      stmt.prepare(
        folly::sformat(
          "SELECT count(*), data FROM {};", tbl
        )
      );
      auto txn = RepoTxn{begin()};
      RepoTxnQuery query(txn, stmt);
      query.step();
      if (!query.row()) {
        throw RepoExc("Can't find table %s", tbl.c_str());
      };
      int val;
      query.getInt(0, val);
      if (val == 0) {
        throw RepoExc("No rows in %s. Did you forget to compile that file with "
                      "this HHVM version?", tbl.c_str());
      }
      BlobDecoder decoder = query.getBlob(1, true);
      decoder(s_globalData);
      FTRACE(1, "GlobalData loaded from '{}':\n", repoName(repoId));
      FTRACE(1, "{}", show(s_globalData));
      if (readArrayTable) {
        auto& arrayTypeTable = globalArrayTypeTable();
        decoder(arrayTypeTable);
        decoder(s_globalData.APCProfile);
        decoder(s_globalData.ConstantFunctions);
        decoder.assertDone();
      }
      txn.commit();
    } catch (RepoExc& e) {
      failures.push_back(repoName(repoId) + ": "  + e.msg());
      continue;
    }

    // TODO: this should probably read out the other elements of the global data
    // which control Option or RuntimeOption values -- the others are read out
    // in an inconsistent and ad-hoc manner. But I don't understand their uses
    // and interactions well enough to feel comfortable fixing now.
    RuntimeOption::EvalPromoteEmptyObject    = s_globalData.PromoteEmptyObject;
    RuntimeOption::EnableIntrinsicsExtension =
      s_globalData.EnableIntrinsicsExtension;
    RuntimeOption::PHP7_Builtins            = s_globalData.PHP7_Builtins;
    RuntimeOption::PHP7_IntSemantics        = s_globalData.PHP7_IntSemantics;
    RuntimeOption::PHP7_NoHexNumerics       = s_globalData.PHP7_NoHexNumerics;
    RuntimeOption::PHP7_Substr              = s_globalData.PHP7_Substr;
    RuntimeOption::EvalReffinessInvariance  = s_globalData.ReffinessInvariance;
    RuntimeOption::EvalCheckPropTypeHints   = s_globalData.CheckPropTypeHints;
    RuntimeOption::EvalHackArrDVArrs        = s_globalData.HackArrDVArrs;
    /*
     * We only should enable array provenance at runtime if it was enabled in
     * the repo AND we have logging enabled--otherwise it's pointless to do the
     * bookkeeping
     *
     * Also--just because array provenance wasn't enabled in the repo doesn't
     * mean it can't be explicitly enabled at runtime
     */
    RuntimeOption::EvalArrayProvenance = RuntimeOption::EvalArrayProvenance ||
      (s_globalData.ArrayProvenance && RuntimeOption::EvalLogArrayProvenance);
    RuntimeOption::EnableArgsInBacktraces = s_globalData.EnableArgsInBacktraces;
    RuntimeOption::EvalAbortBuildOnVerifyError =
      s_globalData.AbortBuildOnVerifyError;
    RuntimeOption::StrictArrayFillKeys      = s_globalData.StrictArrayFillKeys;
    if (s_globalData.HardReturnTypeHints) {
      RuntimeOption::EvalCheckReturnTypeHints = 3;
    }
    if (s_globalData.ThisTypeHintLevel == 3) {
      RuntimeOption::EvalThisTypeHintLevel = s_globalData.ThisTypeHintLevel;
    }

    RuntimeOption::EvalIsCompatibleClsMethType =
      s_globalData.IsCompatibleClsMethType;
    RuntimeOption::EvalEmitClsMethPointers = s_globalData.EmitClsMethPointers;

    RuntimeOption::ConstantFunctions.clear();
    for (auto const& elm : s_globalData.ConstantFunctions) {
      RuntimeOption::ConstantFunctions.insert(elm);
    }

    return;
  }

  if (failures.empty()) {
    std::fprintf(stderr, "No repo was loadable. Check all the possible repo "
                 "locations (Repo.Central.Path, HHVM_REPO_CENTRAL_PATH, and "
                 "$HOME/.hhvm.hhbc) to make sure one of them is a valid "
                 "sqlite3 HHVM repo built with this exact HHVM version.\n");
  } else {
    // We should always have a global data section in RepoAuthoritative
    // mode, or the repo is messed up.
    std::fprintf(stderr, "Failed to load Repo::GlobalData:\n");
    for (auto& f : failures) {
      std::fprintf(stderr, "  %s\n", f.c_str());
    }
  }

  assertx(Process::IsInMainThread());
  exit(1);
}

void Repo::saveGlobalData(GlobalData newData) {
  s_globalData = newData;

  auto const repoId = repoIdForNewUnit(UnitOrigin::File);
  RepoStmt stmt(*this);
  stmt.prepare(
    folly::format(
      "INSERT INTO {} VALUES(@data);", table(repoId, "GlobalData")
    ).str()
  );
  auto txn = RepoTxn{begin()};
  RepoTxnQuery query(txn, stmt);
  BlobEncoder encoder{true};
  encoder(s_globalData);
  encoder(globalArrayTypeTable());
  encoder(s_globalData.APCProfile);
  encoder(s_globalData.ConstantFunctions);
  query.bindBlob("@data", encoder, /* static */ true);
  query.exec();

  // TODO(#3521039): we could just put the litstr table in the same
  // blob as the above and delete LitstrRepoProxy.
  LitstrTable::get().forEachLitstr(
    [this, &txn, repoId](int i, const StringData* name) {
      lsrp().insertLitstr(repoId).insert(txn, i, name);
    });

  txn.commit();
}

std::unique_ptr<Unit> Repo::loadUnit(const folly::StringPiece name,
                                     const SHA1& sha1,
                                     const Native::FuncTable& nativeFuncs) {
  if (m_dbc == nullptr) return nullptr;
  return m_urp.load(name, sha1, nativeFuncs);
}

void Repo::forgetUnit(const std::string& name) {
  if (m_dbc == nullptr) {
    return;
  }

  auto const repoId = repoIdForNewUnit(UnitOrigin::File);
  auto txn = RepoTxn{begin()};
  m_removeFileHash[repoId].remove(txn, name);
  txn.commit();
}

std::vector<std::pair<std::string,SHA1>>
Repo::enumerateUnits(int repoId, bool warn) {
  std::vector<std::pair<std::string,SHA1>> ret;

  try {
    RepoStmt stmt(*this);
    stmt.prepare(folly::sformat(
                   "SELECT path, sha1 FROM {};",
                   table(repoId, "FileSha1"))
                );
    auto txn = RepoTxn{begin()};
    RepoTxnQuery query(txn, stmt);

    for (query.step(); query.row(); query.step()) {
      std::string path;
      SHA1 sha1;

      query.getStdString(0, path);
      query.getSha1(1, sha1);

      ret.emplace_back(path, sha1);
    }

    txn.commit();
  } catch (RepoExc& e) {
    if (warn) {
      fprintf(stderr, "failed to enumerate units: %s\n", e.what());
    }
    // Ugh - the error is dropped.  At least we might have printed an error to
    // stderr.
  }

  return ret;
}

void Repo::InsertFileHashStmt::insert(RepoTxn& txn, const StringData* path,
                                      const SHA1& sha1) {
  if (!prepared()) {
    auto insertQuery = folly::sformat(
      "INSERT INTO {} VALUES(@path, @sha1);",
      m_repo.table(m_repoId, "FileSha1"));
    txn.prepare(*this, insertQuery);
  }
  RepoTxnQuery query(txn, *this);
  query.bindStaticString("@path", path);
  query.bindSha1("@sha1", sha1);
  query.exec();
}

RepoStatus Repo::GetFileHashStmt::get(const char *path, SHA1& sha1) {
  try {
    auto txn = RepoTxn{m_repo.begin()};
    if (!prepared()) {
      auto selectQuery = folly::sformat(
        "SELECT f.sha1 "
        "FROM {} AS f, {} AS u "
        "WHERE path == @path AND f.sha1 == u.sha1 "
        "ORDER BY unitSn DESC LIMIT 1;",
        m_repo.table(m_repoId, "FileSha1"),
        m_repo.table(m_repoId, "Unit"));
      txn.prepare(*this, selectQuery);
    }
    RepoTxnQuery query(txn, *this);
    query.bindText("@path", path, strlen(path));
    query.step();
    if (!query.row()) {
      return RepoStatus::error;
    }
    query.getSha1(0, sha1);
    txn.commit();
    return RepoStatus::success;
  } catch (RepoExc& re) {
    return RepoStatus::error;
  }
}

void Repo::RemoveFileHashStmt::remove(RepoTxn& txn, const std::string& path) {
  if (!prepared()) {
    auto insertQuery = folly::sformat(
      "DELETE FROM {} WHERE path == @path;",
      m_repo.table(m_repoId, "FileSha1"));
    txn.prepare(*this, insertQuery);
  }
  RepoTxnQuery query(txn, *this);
  query.bindStdString("@path", path);
  query.exec();
}

RepoStatus Repo::findFile(const char *path, const std::string &root,
                          SHA1& sha1) {
  if (m_dbc == nullptr) {
    return RepoStatus::error;
  }
  int repoId;
  for (repoId = RepoIdCount - 1; repoId >= 0; --repoId) {
    if (*path == '/' && !root.empty() &&
        !strncmp(root.c_str(), path, root.size()) &&
        (m_getFileHash[repoId].get(path + root.size(), sha1) ==
         RepoStatus::success)) {
      TRACE(3, "Repo loaded file hash for '%s' from '%s'\n",
               path + root.size(), repoName(repoId).c_str());
      return RepoStatus::success;
    }
    if (m_getFileHash[repoId].get(path, sha1) == RepoStatus::success) {
      TRACE(3, "Repo loaded file hash for '%s' from '%s'\n",
                path, repoName(repoId).c_str());
      return RepoStatus::success;
    }
  }
  TRACE(3, "Repo file hash: error loading '%s'\n", path);
  return RepoStatus::error;
}

RepoStatus Repo::insertSha1(UnitOrigin unitOrigin, UnitEmitter* ue,
                           RepoTxn& txn) {
  const StringData* path = ue->m_filepath;
  const SHA1& sha1 = ue->sha1();
  int repoId = repoIdForNewUnit(unitOrigin);
  if (repoId == RepoIdInvalid) {
    return RepoStatus::error;
  }
  try {
    m_insertFileHash[repoId].insert(txn, path, sha1);
    return RepoStatus::success;
  } catch (RepoExc& re) {
    TRACE(3, "Failed to commit sha1 for '%s' to '%s': %s\n",
              path->data(), repoName(repoId).c_str(), re.msg().c_str());
    return RepoStatus::error;
  }
}

void Repo::commitSha1(UnitOrigin unitOrigin, UnitEmitter* ue) {
  try {
    auto txn = RepoTxn{begin()};
    RepoStatus err = insertSha1(unitOrigin, ue, txn);
    if (err == RepoStatus::success) {
      txn.commit();
    }
  } catch (RepoExc& re) {
    int repoId = repoIdForNewUnit(unitOrigin);
    if (repoId != RepoIdInvalid) {
      TRACE(3, "Failed to commit sha1 for '%s' to '%s': %s\n",
               ue->m_filepath->data(), repoName(repoId).c_str(),
               re.msg().c_str());
    }
  }
}

std::string Repo::table(int repoId, const char* tablePrefix) {
  return folly::sformat(
    "{}.{}_{}", dbName(repoId), tablePrefix, repoSchemaId());
}

void Repo::exec(const std::string& sQuery) {
  RepoStmt stmt(*this);
  stmt.prepare(sQuery);
  RepoQuery query(stmt);
  query.exec();
}

RepoTxn Repo::begin() {
  if (m_txDepth > 0) {
    m_txDepth++;
    return RepoTxn{*this};
  }
  if (debug) {
    // Verify start state.
    always_assert(m_txDepth == 0);
    always_assert(!m_rollback);
    if (true) {
      // Bypass RepoQuery, in order to avoid triggering exceptions.
      int rc = sqlite3_step(m_rollbackStmt.get());
      switch (rc) {
      case SQLITE_DONE:
      case SQLITE_ROW:
        not_reached();
      default:
        break;
      }
    } else {
      bool rollbackFailed = false;
      try {
        RepoQuery query(m_rollbackStmt);
        query.exec();
      } catch (RepoExc& re) {
        rollbackFailed = true;
      }
      always_assert(rollbackFailed);
    }
  }
  RepoQuery query(m_beginStmt);
  query.exec();
  m_txDepth++;

  return RepoTxn(*this);
}

void Repo::txPop() {
  // We mix the concept of rollback with a normal commit so that if we try to
  // rollback an inner transaction we eventually end up rolling back the outer
  // transaction instead (Sqlite doesn't support rolling back partial
  // transactions).
  assertx(m_txDepth > 0);
  if (m_txDepth > 1) {
    m_txDepth--;
    return;
  }
  if (!m_rollback) {
    RepoQuery query(m_commitStmt);
    query.exec();
  } else {
    // We're in the outermost transaction - so clear the rollback flag.
    m_rollback = false;
    RepoQuery query(m_rollbackStmt);
    try {
      query.exec();
    } catch (RepoExc& ex) {
      /*
       * Having a rollback fail is actually a normal, expected case,
       * so just swallow this.
       *
       * In particular, according to the docs, if we got an I/O error
       * while doing a commit, the rollback will often fail with "no
       * transaction in progress", because the commit will have
       * automatically been rolled back.  Recommended practice is
       * still to execute a rollback statement and ignore the error.
       */
      TRACE(4, "repo rollback failure: %s\n", ex.what());
    }
  }
  // Decrement depth after query execution, in case an exception occurs during
  // commit.  This allows for subsequent rollback of the failed commit.
  m_txDepth--;
}

void Repo::rollback() {
  m_rollback = true;
  // NOTE: A try/catch isn't necessary - txPop() handles rollback as a nothrow.
  txPop();
}

void Repo::commit() {
  txPop();
}

RepoStatus Repo::insertUnit(UnitEmitter* ue, UnitOrigin unitOrigin,
                            RepoTxn& txn) {
  if (insertSha1(unitOrigin, ue, txn) == RepoStatus::error ||
      ue->insert(unitOrigin, txn) == RepoStatus::error) {
    return RepoStatus::error;
  }
  return RepoStatus::success;
}

void Repo::commitUnit(UnitEmitter* ue, UnitOrigin unitOrigin) {
  if (!RuntimeOption::RepoCommit || ue->m_ICE) return;

  try {
    commitSha1(unitOrigin, ue);
    ue->commit(unitOrigin);
  } catch (const std::exception& e) {
    TRACE(0, "unexpected exception in commitUnit: %s\n",
          e.what());
    assertx(false);
  }
}

void Repo::connect() {
  initCentral();
  initLocal();
  if (!RuntimeOption::RepoEvalMode.compare("local")) {
    m_evalRepoId = (m_localWritable) ? RepoIdLocal : RepoIdCentral;
  } else if (!RuntimeOption::RepoEvalMode.compare("central")) {
    m_evalRepoId = RepoIdCentral;
  } else {
    assertx(!RuntimeOption::RepoEvalMode.compare("readonly"));
    m_evalRepoId = RepoIdInvalid;
  }
  TRACE(1, "Repo.Eval.Mode=%s\n",
           (m_evalRepoId == RepoIdLocal)
           ? "local"
           : (m_evalRepoId == RepoIdCentral)
             ? "central"
             : "readonly");
}

void Repo::disconnect() noexcept {
  if (m_dbc != nullptr) {
    sqlite3_close(m_dbc);
    m_dbc = nullptr;
    m_localReadable = false;
    m_localWritable = false;
    m_evalRepoId = RepoIdInvalid;
  }
}

void Repo::initCentral() {
  std::string error;

  assertx(m_dbc == nullptr);
  auto tryPath = [this, &error](const char* path) {
    std::string subErr;
    if (openCentral(path, subErr) == RepoStatus::error) {
      folly::format(&error, "  {}\n", subErr.empty() ? path : subErr);
      return false;
    }
    return true;
  };

  auto fail_no_repo = [&error] {
    error = "Failed to initialize central HHBC repository:\n" + error;
    // Database initialization failed; this is an unrecoverable state.
    Logger::Error(error);

    if (Process::IsInMainThread()) {
      exit(1);
    }
    always_assert_flog(false, "{}", error);
  };

  // Try Repo.Central.Path
  if (!RuntimeOption::RepoCentralPath.empty() &&
      tryPath(RuntimeOption::RepoCentralPath.c_str())) {
    return;
  }

  // Try HHVM_REPO_CENTRAL_PATH
  const char* HHVM_REPO_CENTRAL_PATH = getenv("HHVM_REPO_CENTRAL_PATH");
  if (HHVM_REPO_CENTRAL_PATH != nullptr &&
      tryPath(HHVM_REPO_CENTRAL_PATH)) {
    return;
  }

  if (!RuntimeOption::RepoAllowFallbackPath) fail_no_repo();

  // Try "$HOME/.hhvm.hhbc".
  char* HOME = getenv("HOME");
  if (HOME != nullptr) {
    std::string centralPath = HOME;
    centralPath += "/.hhvm.hhbc";
    if (tryPath(centralPath.c_str())) {
      return;
    }
  }

#ifndef _WIN32
  // Try the equivalent of "$HOME/.hhvm.hhbc", but look up the home directory
  // in the password database.
  {
    passwd pwbuf;
    passwd* pwbufp;
    long bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (bufsize != -1) {
      auto buf = new char[bufsize];
      SCOPE_EXIT { delete[] buf; };
      if (!getpwuid_r(getuid(), &pwbuf, buf, size_t(bufsize), &pwbufp)
          && (HOME == nullptr || strcmp(HOME, pwbufp->pw_dir))) {
        std::string centralPath = pwbufp->pw_dir;
        centralPath += "/.hhvm.hhbc";
        if (tryPath(centralPath.c_str())) {
          return;
        }
      }
    }
  }
#else // _WIN32
  // Try "$HOMEDRIVE$HOMEPATH/.hhvm.hhbc"
  char* HOMEDRIVE = getenv("HOMEDRIVE");
  if (HOMEDRIVE != nullptr) {
    char* HOMEPATH = getenv("HOMEPATH");
    if (HOMEPATH != nullptr) {
      std::string centralPath = HOMEDRIVE;
      centralPath += HOMEPATH;
      centralPath += "\\.hhvm.hhbc";
      if (tryPath(centralPath.c_str()))
        return;
    }
  }
#endif

  fail_no_repo();
}

namespace {
/*
 * Convert the permission bits from the given stat struct to an ls-style
 * rwxrwxrwx format.
 */
std::string showPermissions(const struct stat& s) {
  static const std::pair<int, char> bits[] = {
    {S_IRUSR, 'r'}, {S_IWUSR, 'w'}, {S_IXUSR, 'x'},
    {S_IRGRP, 'r'}, {S_IWGRP, 'w'}, {S_IXGRP, 'x'},
    {S_IROTH, 'r'}, {S_IWOTH, 'w'}, {S_IXOTH, 'x'},
  };
  std::string ret;
  ret.reserve(sizeof(bits) / sizeof(bits[0]));

  for (auto pair : bits) {
    ret += (s.st_mode & pair.first) ? pair.second : '-';
  }
  return ret;
}

struct PasswdBuffer {
  explicit PasswdBuffer(int name)
    : size{sysconf(name)}
  {
    if (size == -1) size = 1024;
    data = std::make_unique<char[]>(size);
  }

  long size;
  std::unique_ptr<char[]> data;
};

/*
 * Return the name of the user with the given id.
 */
std::string uidToName(uid_t uid) {
#ifndef _WIN32
  auto buffer = PasswdBuffer{_SC_GETPW_R_SIZE_MAX};
  passwd pw;
  passwd* result;

  auto err = getpwuid_r(uid, &pw, buffer.data.get(), buffer.size, &result);
  if (err != 0) return folly::errnoStr(errno).toStdString();
  if (result == nullptr) return "user does not exist";
  return pw.pw_name;
#else
  return "<unsupported>";
#endif
}

/*
 * Return the uid of the user with the given name.
 */
uid_t nameToUid(const std::string& name) {
#ifndef _WIN32
  auto buffer = PasswdBuffer{_SC_GETPW_R_SIZE_MAX};
  passwd pw;
  passwd* result;

  auto err = getpwnam_r(
    name.c_str(), &pw, buffer.data.get(), buffer.size, &result
  );
  if (err != 0 || result == nullptr) return -1;
  return pw.pw_uid;
#else
  return -1;
#endif
}

/*
 * Return the name of the group with the given id.
 */
std::string gidToName(gid_t gid) {
#ifndef _WIN32
  auto buffer = PasswdBuffer{_SC_GETGR_R_SIZE_MAX};
  group grp;
  group* result;

  auto err = getgrgid_r(gid, &grp, buffer.data.get(), buffer.size, &result);
  if (err != 0) return folly::errnoStr(errno).toStdString();
  if (result == nullptr) return "group does not exist";
  return grp.gr_name;
#else
  return "<unsupported>";
#endif
}

/*
 * Return the gid of the group with the given name.
 */
gid_t nameToGid(const std::string& name) {
#ifndef _WIN32
  auto buffer = PasswdBuffer{_SC_GETGR_R_SIZE_MAX};
  group grp;
  group* result;

  auto err = getgrnam_r(
    name.c_str(), &grp, buffer.data.get(), buffer.size, &result
  );
  if (err != 0 || result == nullptr) return -1;
  return grp.gr_gid;
#else
  return -1;
#endif
}

void setCentralRepoFileMode(const std::string& path) {
  // These runtime options are all best-effort, so we don't care if any of the
  // operations fail.

  if (auto const mode = RuntimeOption::RepoCentralFileMode) {
    chmod(path.c_str(), mode);
  }

  if (!RuntimeOption::RepoCentralFileUser.empty()) {
    auto const uid = nameToUid(RuntimeOption::RepoCentralFileUser);
    chown(path.c_str(), uid, -1);
  }

  if (!RuntimeOption::RepoCentralFileGroup.empty()) {
    auto const gid = nameToGid(RuntimeOption::RepoCentralFileGroup);
    chown(path.c_str(), -1, gid);
  }
}
}

RepoStatus Repo::openCentral(const char* rawPath, std::string& errorMsg) {
  std::string repoPath = insertSchema(rawPath);
  // SQLITE_OPEN_NOMUTEX specifies that the connection be opened such
  // that no mutexes are used to protect the database connection from other
  // threads.  However, multiple connections can still be used concurrently,
  // because SQLite as a whole is thread-safe.
  if (int err = sqlite3_open_v2(repoPath.c_str(), &m_dbc,
                                SQLITE_OPEN_NOMUTEX |
                                SQLITE_OPEN_READWRITE |
                                SQLITE_OPEN_CREATE, nullptr)) {
    TRACE(1, "Repo::%s() failed to open candidate central repo '%s'\n",
             __func__, repoPath.c_str());
    errorMsg = folly::format("Failed to open {}: {} - {}",
                             repoPath, err, sqlite3_errmsg(m_dbc)).str();
    return RepoStatus::error;
  }

  if (RuntimeOption::RepoBusyTimeoutMS) {
    sqlite3_busy_timeout(m_dbc, RuntimeOption::RepoBusyTimeoutMS);
  }
  try {
    m_beginStmt.prepare("BEGIN TRANSACTION;");
    m_rollbackStmt.prepare("ROLLBACK;");
    m_commitStmt.prepare("COMMIT;");
    pragmas(RepoIdCentral);
  } catch (RepoExc& re) {
    TRACE(1, "Repo::%s() failed to initialize connection to canditate repo"
             " '%s': %s\n", __func__, repoPath.c_str(), re.what());
    errorMsg = folly::format("Failed to initialize connection to {}: {}",
                             repoPath, re.what()).str();
    return RepoStatus::error;
  }

  // sqlite3_open_v2() will silently open in read-only mode if file permissions
  // prevent writing.  Therefore, tell initSchema() to verify that the database
  // is writable.
  bool centralWritable = true;
  if (initSchema(RepoIdCentral, centralWritable, errorMsg) == RepoStatus::error
      || !centralWritable) {
    TRACE(1, "Repo::initSchema() failed for candidate central repo '%s'\n",
             repoPath.c_str());
    struct stat repoStat;
    std::string statStr;
    if (stat(repoPath.c_str(), &repoStat) == 0) {
      statStr = folly::sformat("{} {}:{}",
                               showPermissions(repoStat),
                               uidToName(repoStat.st_uid),
                               gidToName(repoStat.st_gid));
    } else {
      statStr = folly::errnoStr(errno).toStdString();
    }
    errorMsg = folly::format("Failed to initialize schema in {}({}): {}",
                             repoPath, statStr, errorMsg).str();
    return RepoStatus::error;
  }
  m_centralRepo = repoPath;
  setCentralRepoFileMode(repoPath);
  TRACE(1, "Central repo: '%s'\n", m_centralRepo.c_str());
  return RepoStatus::success;
}

void Repo::initLocal() {
  if (RuntimeOption::RepoLocalMode.compare("--")) {
    bool isWritable;
    if (!RuntimeOption::RepoLocalMode.compare("rw")) {
      isWritable = true;
    } else {
      assertx(!RuntimeOption::RepoLocalMode.compare("r-"));
      isWritable = false;
    }

    if (!RuntimeOption::RepoLocalPath.empty()) {
      attachLocal(RuntimeOption::RepoLocalPath.c_str(), isWritable);
    } else if (RuntimeOption::RepoAllowFallbackPath) {
      if (!RuntimeOption::ServerExecutionMode()) {
        std::string cliRepo = s_cliFile;
        if (!cliRepo.empty()) {
          cliRepo += ".hhbc";
        }
        attachLocal(cliRepo.c_str(), isWritable);
      } else {
        attachLocal("hhvm.hhbc", isWritable);
      }
    }
  }
}

void Repo::attachLocal(const char* path, bool isWritable) {
  std::string repoPath = insertSchema(path);
  if (!isWritable) {
    // Make sure the repo exists before attaching it, in order to avoid
    // creating a read-only repo.
    struct stat buf;
    if (!strchr(repoPath.c_str(), ':') &&
        stat(repoPath.c_str(), &buf) != 0) {
      return;
    }
  }
  try {
    auto attachQuery = folly::sformat(
      "ATTACH DATABASE '{}' as {};", repoPath, dbName(RepoIdLocal));
    exec(attachQuery);
    pragmas(RepoIdLocal);
  } catch (RepoExc& re) {
    // Failed to run pragmas on local DB - ignored
    return;
  }

  std::string error;
  if (initSchema(RepoIdLocal, isWritable, error) == RepoStatus::error) {
    FTRACE(1, "Local repo {} failed to init schema: {}\n", repoPath, error);
    return;
  }
  m_localRepo = repoPath;
  m_localReadable = true;
  m_localWritable = isWritable;
  TRACE(1, "Local repo: '%s' (read%s)\n",
           m_localRepo.c_str(), m_localWritable ? "-write" : "-only");
}

void Repo::pragmas(int repoId) {
  // Valid synchronous values: 0 (OFF), 1 (NORMAL), 2 (FULL).
  static const int synchronous = 0;
  setIntPragma(repoId, "synchronous", synchronous);
  setIntPragma(repoId, "cache_size", 20);
  // Valid journal_mode values: delete, truncate, persist, memory, wal, off.
  setTextPragma(repoId, "journal_mode", RuntimeOption::RepoJournal.c_str());
}

void Repo::getIntPragma(int repoId, const char* name, int& val) {
  auto pragmaQuery = folly::sformat("PRAGMA {}.{};", dbName(repoId), name);
  RepoStmt stmt(*this);
  stmt.prepare(pragmaQuery);
  RepoQuery query(stmt);
  query.step();
  query.getInt(0, val);
}

void Repo::setIntPragma(int repoId, const char* name, int val) {
  // Read first to see if a write can be avoided
  int oldval = -1;
  getIntPragma(repoId, name, oldval);
  if (val == oldval) return;

  // Pragma writes must be executed outside transactions, since they may change
  // transaction behavior.
  auto pragmaQuery = folly::sformat(
    "PRAGMA {}.{} = {};", dbName(repoId), name, val);
  exec(pragmaQuery);
  if (debug) {
    // Verify that the pragma had the desired effect.
    int newval = -1;
    getIntPragma(repoId, name, newval);
    if (newval != val) {
      throw RepoExc("Unexpected PRAGMA %s.%s value: %d\n",
                    dbName(repoId), name, newval);
    }
  }
}

void Repo::getTextPragma(int repoId, const char* name, std::string& val) {
  auto pragmaQuery = folly::sformat("PRAGMA {}.{};", dbName(repoId), name);
  RepoStmt stmt(*this);
  stmt.prepare(pragmaQuery);
  RepoQuery query(stmt);
  const char* s;
  query.step();
  query.getText(0, s);
  val = s;
}

void Repo::setTextPragma(int repoId, const char* name, const char* val) {
  // Read first to see if a write can be avoided
  std::string oldval = "?";
  getTextPragma(repoId, name, oldval);
  if (!strcmp(oldval.c_str(), val)) return;

  // Pragma writes must be executed outside transactions, since they may change
  // transaction behavior.
  auto pragmaQuery = folly::sformat(
    "PRAGMA {}.{} = {};", dbName(repoId), name, val);
  exec(pragmaQuery);
  if (debug) {
    // Verify that the pragma had the desired effect.
    std::string newval = "?";
    getTextPragma(repoId, name, newval);
    if (strcmp(newval.c_str(), val)) {
      throw RepoExc("Unexpected PRAGMA %s.%s value: %s\n",
                    dbName(repoId), name, newval.c_str());
    }
  }
}

RepoStatus Repo::initSchema(int repoId, bool& isWritable,
                            std::string& errorMsg) {
  if (!schemaExists(repoId)) {
    if (createSchema(repoId, errorMsg) == RepoStatus::error) {
      // Check whether this failure is due to losing the schema
      // initialization race with another process.
      if (!schemaExists(repoId)) {
        return RepoStatus::error;
      }
    } else {
      // createSchema() successfully wrote to the database, so no further
      // verification is necessary.
      return RepoStatus::success;
    }
  }
  if (isWritable) {
    isWritable = writable(repoId);
  }
  return RepoStatus::success;
}

bool Repo::schemaExists(int repoId) {
  try {
    auto txn = RepoTxn{begin()};
    auto selectQuery = folly::sformat(
      "SELECT product FROM {};", table(repoId, "magic"));
    RepoStmt stmt(*this);
    // If the DB is 'new' and hasn't been initialized yet then we expect this
    // prepare() to fail.
    stmt.prepare(selectQuery);
    // This SHOULDN'T fail - we create the table under a transaction - so if it
    // exists then it should have our magic value.
    RepoTxnQuery query(txn, stmt);
    query.step();
    const char* text; /**/ query.getText(0, text);
    if (strcmp(kMagicProduct, text)) {
      return false;
    }
    txn.commit();
  } catch (RepoExc& re) {
    return false;
  }
  return true;
}

RepoStatus Repo::createSchema(int repoId, std::string& errorMsg) {
  try {
    auto txn = RepoTxn{begin()};
    {
      auto createQuery = folly::sformat(
        "CREATE TABLE {} (product TEXT);", table(repoId, "magic"));
      txn.exec(createQuery);

      auto insertQuery = folly::sformat(
        "INSERT INTO {} VALUES('{}');", table(repoId, "magic"), kMagicProduct);
      txn.exec(insertQuery);
    }
    {
      auto createQuery = folly::sformat(
        "CREATE TABLE {} (path TEXT, sha1 BLOB, UNIQUE(path, sha1));",
        table(repoId, "FileSha1"));
      txn.exec(createQuery);
    }
    txn.exec(folly::sformat("CREATE TABLE {} (data BLOB);",
                           table(repoId, "GlobalData")));
    m_urp.createSchema(repoId, txn);
    m_pcrp.createSchema(repoId, txn);
    m_rrp.createSchema(repoId, txn);
    m_frp.createSchema(repoId, txn);
    m_lsrp.createSchema(repoId, txn);

    txn.commit();
  } catch (RepoExc& re) {
    errorMsg = re.what();
    return RepoStatus::error;
  }
  return RepoStatus::success;
}

bool Repo::writable(int repoId) {
  switch (sqlite3_db_readonly(m_dbc, dbName(repoId))) {
    case 0:  return true;
    case 1:  return false;
    case -1: return false;
    default: break;
  }
  always_assert(false);
}

//////////////////////////////////////////////////////////////////////

void batchCommit(const std::vector<std::unique_ptr<UnitEmitter>>& ues) {
  auto& repo = Repo::get();

  // Attempt batch commit.  This can legitimately fail due to multiple input
  // files having identical contents.
  bool err = false;
  {
    auto txn = RepoTxn{repo.begin()};

    for (auto& ue : ues) {
      if (repo.insertUnit(ue.get(), UnitOrigin::File, txn) ==
          RepoStatus::error) {
        err = true;
        break;
      }
    }
    if (!err) {
      txn.commit();
    }
  }

  // Commit units individually if an error occurred during batch commit.
  if (err) {
    for (auto& ue : ues) {
      repo.commitUnit(ue.get(), UnitOrigin::File);
    }
  }
}

//////////////////////////////////////////////////////////////////////

}
