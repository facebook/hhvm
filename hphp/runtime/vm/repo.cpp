/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "folly/Format.h"

#include "hphp/util/logger.h"
#include "hphp/util/trace.h"
#include "hphp/util/repo-schema.h"
#include "hphp/util/assertions.h"
#include "hphp/runtime/vm/blob-helper.h"

namespace HPHP {

TRACE_SET_MOD(hhbc);

const char* Repo::kMagicProduct =
  "facebook.com HipHop Virtual Machine bytecode repository";
const char* Repo::kSchemaPlaceholder = "%{schema}";
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
  if (const char* schemaOverride = getenv("HHVM_RUNTIME_REPO_SCHEMA")) {
    TRACE(1, "Schema override: HHVM_RUNTIME_REPO_SCHEMA=%s\n", schemaOverride);
    kRepoSchemaId = schemaOverride;
  }
}

IMPLEMENT_THREAD_LOCAL(Repo, t_dh);

Repo& Repo::get() {
  return *t_dh.get();
}

void Repo::shutdown() {
  t_dh.destroy();
}

SimpleMutex Repo::s_lock;
unsigned Repo::s_nRepos = 0;

bool Repo::prefork() {
  if (!t_dh.isNull()) {
    t_dh.destroy();
  }
  s_lock.lock();
  if (s_nRepos > 0) {
    s_lock.unlock();
    return true;
  }
  return false;
}

void Repo::postfork(pid_t pid) {
  if (pid == 0) {
    new (&s_lock) SimpleMutex();
  } else {
    s_lock.unlock();
  }
}

Repo::Repo()
  : RepoProxy(*this),
#define RP_OP(c, o) \
    m_##o##Local(*this, RepoIdLocal), m_##o##Central(*this, RepoIdCentral),
    RP_OPS
#undef RP_OP
    m_dbc(nullptr), m_localReadable(false), m_localWritable(false),
    m_evalRepoId(-1), m_txDepth(0), m_rollback(false), m_beginStmt(*this),
    m_rollbackStmt(*this), m_commitStmt(*this), m_urp(*this), m_pcrp(*this),
    m_frp(*this), m_lsrp(*this) {
#define RP_OP(c, o) \
  m_##o[RepoIdLocal] = &m_##o##Local; \
  m_##o[RepoIdCentral] = &m_##o##Central;
  RP_OPS
#undef RP_OP
  {
    SimpleLock lock(s_lock);
    s_nRepos++;
  }
  connect();
}

Repo::~Repo() {
  disconnect();
  {
    SimpleLock lock(s_lock);
    s_nRepos--;
  }
}

std::string Repo::s_cliFile;
void Repo::setCliFile(const std::string& cliFile) {
  assert(s_cliFile.empty());
  assert(t_dh.isNull());
  s_cliFile = cliFile;
}

void Repo::loadGlobalData() {
  m_lsrp.load();

  if (!RuntimeOption::RepoAuthoritative) return;

  std::vector<std::string> failures;

  /*
   * This should probably just go to the Local repo always, except
   * that our unit test suite is currently running RepoAuthoritative
   * tests with the compiled repo as the Central repo.
   */
  for (int repoId = RepoIdCount - 1; repoId >= 0; --repoId) {
    try {
      RepoStmt stmt(*this);
      stmt.prepare(
        folly::format(
          "SELECT data from {};", table(repoId, "GlobalData")
        ).str()
      );
      RepoTxn txn(*this);
      RepoTxnQuery query(txn, stmt);
      query.step();
      if (!query.row()) continue;
      BlobDecoder decoder = query.getBlob(0);
      decoder(s_globalData);

      txn.commit();
    } catch (RepoExc& e) {
      failures.push_back(e.msg());
      continue;
    }

    return;
  }

  // We should always have a global data section in RepoAuthoritative
  // mode, or the repo is messed up.
  std::fprintf(stderr, "failed to load Repo::GlobalData:\n");
  for (auto& f : failures) {
    std::fprintf(stderr, "  %s\n", f.c_str());
  }
  std::abort();
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
  RepoTxn txn(*this);
  RepoTxnQuery query(txn, stmt);
  BlobEncoder encoder;
  encoder(s_globalData);
  query.bindBlob("@data", encoder, /* static */ true);
  query.exec();

  // TODO(#3521039): we could just put the litstr table in the same
  // blob as the above and delete LitstrRepoProxy.
  LitstrTable::get().insert(txn, UnitOrigin::File);

  txn.commit();
}

Unit* Repo::loadUnit(const std::string& name, const MD5& md5) {
  if (m_dbc == nullptr) {
    return nullptr;
  }
  return m_urp.load(name, md5);
}

std::vector<std::pair<std::string,MD5>> Repo::enumerateUnits() {
  std::vector<std::pair<std::string,MD5>> ret;

  try {
    RepoStmt stmt(*this);
    stmt.prepare(
      folly::format(
        "SELECT path, md5 FROM {};", table(RepoIdCentral, "FileMd5")
      ).str()
    );
    RepoTxn txn(*this);
    RepoTxnQuery query(txn, stmt);

    for (query.step(); query.row(); query.step()) {
      std::string path;
      MD5 md5;

      query.getStdString(0, path);
      query.getMd5(1, md5);

      ret.emplace_back(path, md5);
    }

    txn.commit();
  } catch (RepoExc& e) {
    fprintf(stderr, "failed to enumerate units: %s\n", e.what());
  }

  return ret;
}

void Repo::InsertFileHashStmt::insert(RepoTxn& txn, const StringData* path,
                                      const MD5& md5) {
  if (!prepared()) {
    std::stringstream ssInsert;
    ssInsert << "INSERT INTO " << m_repo.table(m_repoId, "FileMd5")
             << " VALUES(@path, @md5);";
    txn.prepare(*this, ssInsert.str());
  }
  RepoTxnQuery query(txn, *this);
  query.bindStaticString("@path", path);
  query.bindMd5("@md5", md5);
  query.exec();
}

bool Repo::GetFileHashStmt::get(const char *path, MD5& md5) {
  try {
    RepoTxn txn(m_repo);
    if (!prepared()) {
      std::stringstream ssSelect;
      ssSelect << "SELECT f.md5 FROM "
               << m_repo.table(m_repoId, "FileMd5")
               << " AS f, " << m_repo.table(m_repoId, "Unit")
               << " AS u WHERE path == @path AND f.md5 == u.md5"
               << " ORDER BY unitSn DESC LIMIT 1;";
      txn.prepare(*this, ssSelect.str());
    }
    RepoTxnQuery query(txn, *this);
    query.bindText("@path", path, strlen(path));
    query.step();
    if (!query.row()) {
      return false;
    }
    query.getMd5(0, md5);
    txn.commit();
    return true;
  } catch (RepoExc& re) {
    return false;
  }
  return false;
}

bool Repo::findFile(const char *path, const std::string &root, MD5& md5) {
  if (m_dbc == nullptr) {
    return false;
  }
  int repoId;
  for (repoId = RepoIdCount - 1; repoId >= 0; --repoId) {
    if (*path == '/' && !root.empty() &&
        !strncmp(root.c_str(), path, root.size()) &&
        getFileHash(repoId).get(path + root.size(), md5)) {
      TRACE(3, "Repo loaded file hash for '%s' from '%s'\n",
               path + root.size(), repoName(repoId).c_str());
      return true;
    }
    if (getFileHash(repoId).get(path, md5)) {
      TRACE(3, "Repo loaded file hash for '%s' from '%s'\n",
                path, repoName(repoId).c_str());
      return true;
    }
  }
  TRACE(3, "Repo file hash: error loading '%s'\n", path);
  return false;
}

bool Repo::insertMd5(UnitOrigin unitOrigin, UnitEmitter* ue, RepoTxn& txn) {
  const StringData* path = ue->getFilepath();
  const MD5& md5 = ue->md5();
  int repoId = repoIdForNewUnit(unitOrigin);
  if (repoId == RepoIdInvalid) {
    return true;
  }
  try {
    insertFileHash(repoId).insert(txn, path, md5);
    return false;
  } catch(RepoExc& re) {
    TRACE(3, "Failed to commit md5 for '%s' to '%s': %s\n",
              path->data(), repoName(repoId).c_str(), re.msg().c_str());
    return true;
  }
}

void Repo::commitMd5(UnitOrigin unitOrigin, UnitEmitter* ue) {
  try {
    RepoTxn txn(*this);
    bool err = insertMd5(unitOrigin, ue, txn);
    if (!err) {
      txn.commit();
    }
  } catch(RepoExc& re) {
    int repoId = repoIdForNewUnit(unitOrigin);
    if (repoId != RepoIdInvalid) {
      TRACE(3, "Failed to commit md5 for '%s' to '%s': %s\n",
               ue->getFilepath()->data(), repoName(repoId).c_str(),
               re.msg().c_str());
    }
  }
}

std::string Repo::table(int repoId, const char* tablePrefix) {
  std::stringstream ss;
  ss << dbName(repoId) << "." << tablePrefix << "_" << kRepoSchemaId;
  return ss.str();
}

void Repo::exec(const std::string& sQuery) {
  RepoStmt stmt(*this);
  stmt.prepare(sQuery);
  RepoQuery query(stmt);
  query.exec();
}

void Repo::begin() {
  if (m_txDepth > 0) {
    m_txDepth++;
    return;
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
}

void Repo::txPop() {
  assert(m_txDepth > 0);
  if (m_txDepth > 1) {
    m_txDepth--;
    return;
  }
  if (!m_rollback) {
    RepoQuery query(m_commitStmt);
    query.exec();
  } else {
    m_rollback = false;
    try {
      RepoQuery query(m_rollbackStmt);
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
  // commit.  This allows for subsequent rollback.
  m_txDepth--;
}

void Repo::rollback() {
  m_rollback = true;
  txPop();
}

void Repo::commit() {
  txPop();
}

bool Repo::insertUnit(UnitEmitter* ue, UnitOrigin unitOrigin, RepoTxn& txn) {
  try {
    if (insertMd5(unitOrigin, ue, txn)) return true;
    if (ue->insert(unitOrigin, txn)) return true;
  } catch (const std::exception& e) {
    TRACE(0, "unexpected exception in insertUnit: %s\n",
             e.what());
    assert(false);
    return true;
  }
  return false;
}

void Repo::commitUnit(UnitEmitter* ue, UnitOrigin unitOrigin) {
  if (!RuntimeOption::RepoCommit) return;

  try {
    commitMd5(unitOrigin, ue);
    ue->commit(unitOrigin);
  } catch (const std::exception& e) {
    TRACE(0, "unexpected exception in commitUnit: %s\n",
             e.what());
    assert(false);
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
    assert(!RuntimeOption::RepoEvalMode.compare("readonly"));
    m_evalRepoId = RepoIdInvalid;
  }
  TRACE(1, "Repo.Eval.Mode=%s\n",
           (m_evalRepoId == RepoIdLocal)
           ? "local"
           : (m_evalRepoId == RepoIdCentral)
             ? "central"
             : "readonly");
}

void Repo::disconnect() {
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

  assert(m_dbc == nullptr);
  auto tryPath = [this, &error](const char* path) {
    std::string subErr;
    if (!openCentral(path, subErr)) return false;

    folly::format(&error, "  {}\n", subErr.empty() ? path : subErr);
    return true;
  };

  // Try Repo.Central.Path (or HHVM_REPO_CENTRAL_PATH).
  if (!RuntimeOption::RepoCentralPath.empty()) {
    if (!tryPath(RuntimeOption::RepoCentralPath.c_str())) {
      return;
    }
  }

  const char* HHVM_REPO_CENTRAL_PATH = getenv("HHVM_REPO_CENTRAL_PATH");
  if (HHVM_REPO_CENTRAL_PATH != nullptr) {
    if (!tryPath(HHVM_REPO_CENTRAL_PATH)) {
      return;
    }
  }

  // Try "$HOME/.hhvm.hhbc".
  char* HOME = getenv("HOME");
  if (HOME != nullptr) {
    std::string centralPath = HOME;
    centralPath += "/.hhvm.hhbc";
    if (!tryPath(centralPath.c_str())) {
      return;
    }
  }

  // Try the equivalent of "$HOME/.hhvm.hhbc", but look up the home directory
  // in the password database.
  {
    struct passwd pwbuf;
    struct passwd* pwbufp;
    long bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (bufsize != -1) {
      char buf[size_t(bufsize)];
      if (!getpwuid_r(getuid(), &pwbuf, buf, size_t(bufsize), &pwbufp)
          && (HOME == nullptr || strcmp(HOME, pwbufp->pw_dir))) {
        std::string centralPath = pwbufp->pw_dir;
        centralPath += "/.hhvm.hhbc";
        if (!tryPath(centralPath.c_str())) {
          return;
        }
      }
    }
  }

  error = "Failed to initialize central HHBC repository:\n" + error;
  // Database initialization failed; this is an unrecoverable state.
  Logger::Error("%s", error.c_str());

  always_assert_log(false, [&error] { return error; });
}

static int busyHandler(void* opaque, int nCalls) {
  Repo* repo UNUSED = static_cast<Repo*>(opaque);
  // yield to allow other threads access to the machine
  // spin-wait can starve other threads.
  usleep(1000 * nCalls);
  return 1; // Tell SQLite to retry.
}

std::string Repo::insertSchema(const char* path) {
  assert(strstr(kRepoSchemaId, kSchemaPlaceholder) == nullptr);
  std::string result = path;
  size_t idx;
  if ((idx = result.find(kSchemaPlaceholder)) != std::string::npos) {
    result.replace(idx, strlen(kSchemaPlaceholder), kRepoSchemaId);
  }
  TRACE(2, "Repo::%s() transformed %s into %s\n",
        __func__, path, result.c_str());
  return result;
}

bool Repo::openCentral(const char* rawPath, std::string& errorMsg) {
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
    return true;
  }
  // Register a busy handler to avoid spurious SQLITE_BUSY errors.
  sqlite3_busy_handler(m_dbc, busyHandler, (void*)this);
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
    return true;
  }
  // sqlite3_open_v2() will silently open in read-only mode if file permissions
  // prevent writing, and there is no apparent way to detect this other than to
  // attempt writing to the database.  Therefore, tell initSchema() to verify
  // that the database is writable.
  bool centralWritable = true;
  if (initSchema(RepoIdCentral, centralWritable) || !centralWritable) {
    TRACE(1, "Repo::initSchema() failed for candidate central repo '%s'\n",
             repoPath.c_str());
    errorMsg = folly::format("Failed to initialize schema in {}",
                             repoPath).str();
    return true;
  }
  m_centralRepo = repoPath;
  TRACE(1, "Central repo: '%s'\n", m_centralRepo.c_str());
  return false;
}

void Repo::initLocal() {
  if (RuntimeOption::RepoLocalMode.compare("--")) {
    bool isWritable;
    if (!RuntimeOption::RepoLocalMode.compare("rw")) {
      isWritable = true;
    } else {
      assert(!RuntimeOption::RepoLocalMode.compare("r-"));
      isWritable = false;
    }

    if (!RuntimeOption::RepoLocalPath.empty()) {
      attachLocal(RuntimeOption::RepoLocalPath.c_str(), isWritable);
    } else {
      if (RuntimeOption::ClientExecutionMode()) {
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
    std::stringstream ssAttach;
    ssAttach << "ATTACH DATABASE '" << repoPath << "' as "
             << dbName(RepoIdLocal) << ";";
    exec(ssAttach.str());
    pragmas(RepoIdLocal);
  } catch (RepoExc& re) {
    return;
  }
  if (initSchema(RepoIdLocal, isWritable)) {
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
  // Valid journal_mode values: delete, truncate, persist, memory, wal, off.
  setTextPragma(repoId, "journal_mode", RuntimeOption::RepoJournal.c_str());
}

void Repo::getIntPragma(int repoId, const char* name, int& val) {
  RepoTxn txn(*this);
  std::stringstream ssPragma;
  ssPragma << "PRAGMA " << dbName(repoId) << "." << name << ";";
  RepoStmt stmt(*this);
  stmt.prepare(ssPragma.str());
  RepoTxnQuery query(txn, stmt);
  query.step();
  query.getInt(0, val);
  txn.commit();
}

void Repo::setIntPragma(int repoId, const char* name, int val) {
  // Pragma writes must be executed outside transactions, since they may change
  // transaction behavior.
  std::stringstream ssPragma;
  ssPragma << "PRAGMA " << dbName(repoId) << "." << name << " = " << val << ";";
  exec(ssPragma.str());
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
  RepoTxn txn(*this);
  std::stringstream ssPragma;
  ssPragma << "PRAGMA " << dbName(repoId) << "." << name << ";";
  RepoStmt stmt(*this);
  stmt.prepare(ssPragma.str());
  RepoTxnQuery query(txn, stmt);
  const char* s;
  query.step();
  query.getText(0, s);
  val = s;
  txn.commit();
}

void Repo::setTextPragma(int repoId, const char* name, const char* val) {
  // Pragma writes must be executed outside transactions, since they may change
  // transaction behavior.
  std::stringstream ssPragma;
  ssPragma << "PRAGMA " << dbName(repoId) << "." << name << " = " << val << ";";
  exec(ssPragma.str());
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

bool Repo::initSchema(int repoId, bool& isWritable) {
  if (!schemaExists(repoId)) {
    if (createSchema(repoId)) {
      // Check whether this failure is due to losing the schema
      // initialization race with another process.
      if (!schemaExists(repoId)) {
        return true;
      }
    } else {
      // createSchema() successfully wrote to the database, so no further
      // verification is necessary.
      return false;
    }
  }
  if (isWritable) {
    isWritable = writable(repoId);
  }
  return false;
}

bool Repo::schemaExists(int repoId) {
  try {
    RepoTxn txn(*this);
    std::stringstream ssSelect;
    ssSelect << "SELECT product FROM " << table(repoId, "magic") << ";";
    RepoStmt stmt(*this);
    stmt.prepare(ssSelect.str());
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

bool Repo::createSchema(int repoId) {
  try {
    RepoTxn txn(*this);
    {
      std::stringstream ssCreate;
      ssCreate << "CREATE TABLE " << table(repoId, "magic")
               << "(product TEXT);";
      txn.exec(ssCreate.str());

      std::stringstream ssInsert;
      ssInsert << "INSERT INTO " << table(repoId, "magic")
               << " VALUES('" << kMagicProduct << "');";
      txn.exec(ssInsert.str());
    }
    {
      std::stringstream ssCreate;
      ssCreate << "CREATE TABLE " << table(repoId, "writable")
               << "(canary INTEGER);";
      txn.exec(ssCreate.str());
    }
    {
      std::stringstream ssCreate;
      ssCreate << "CREATE TABLE " << table(repoId, "FileMd5")
               << "(path TEXT, md5 BLOB, UNIQUE(path, md5));";
      txn.exec(ssCreate.str());
    }
    txn.exec(folly::format("CREATE TABLE {} (data BLOB);",
                           table(repoId, "GlobalData")).str());
    m_urp.createSchema(repoId, txn);
    m_pcrp.createSchema(repoId, txn);
    m_frp.createSchema(repoId, txn);
    m_lsrp.createSchema(repoId, txn);

    txn.commit();
  } catch (RepoExc& re) {
    return true;
  }
  return false;
}

bool Repo::writable(int repoId) {
  try {
    // Check whether database is writable by adding and removing a row in the
    // 'writable' table.
    RepoTxn txn(*this);
    std::stringstream ssInsert;
    ssInsert << "INSERT INTO " << table(repoId, "writable") << " VALUES(0);";
    txn.exec(ssInsert.str());
    std::stringstream ssDelete;
    ssDelete << "DELETE FROM " << table(repoId, "writable")
             << " WHERE canary == 0;";
    txn.exec(ssDelete.str());
    txn.commit();
  } catch (RepoExc& re) {
    return false;
  }
  return true;
}

//////////////////////////////////////////////////////////////////////

void batchCommit(std::vector<std::unique_ptr<UnitEmitter>> ues) {
  auto& repo = Repo::get();

  // Attempt batch commit.  This can legitimately fail due to multiple input
  // files having identical contents.
  bool err = false;
  {
    RepoTxn txn(repo);

    for (auto& ue : ues) {
      if (repo.insertUnit(ue.get(), UnitOrigin::File, txn)) {
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
