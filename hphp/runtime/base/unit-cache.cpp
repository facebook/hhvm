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
#include "hphp/runtime/base/unit-cache.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <memory>
#include <string>
#include <cstdlib>
#include <thread>

#include <folly/ScopeGuard.h>
#include <folly/portability/Fcntl.h>

#include "hphp/util/assertions.h"
#include "hphp/util/rank.h"
#include "hphp/util/mutex.h"
#include "hphp/util/process.h"
#include "hphp/util/struct-log.h"
#include "hphp/util/timer.h"
#include "hphp/runtime/base/system-profiler.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/base/string-util.h"
#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/base/plain-file.h"
#include "hphp/runtime/base/stat-cache.h"
#include "hphp/runtime/base/stream-wrapper-registry.h"
#include "hphp/runtime/base/file-stream-wrapper.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/server/cli-server.h"
#include "hphp/runtime/server/source-root-info.h"
#include "hphp/runtime/vm/debugger-hook.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/treadmill.h"

#ifdef __APPLE__
#define st_mtim st_mtimespec
#define st_ctim st_ctimespec
#endif

namespace HPHP {

//////////////////////////////////////////////////////////////////////

namespace {

//////////////////////////////////////////////////////////////////////

using OptLog = folly::Optional<StructuredLogEntry>;

struct LogTimer {
  LogTimer(const char* name, OptLog& ent)
    : m_name(name)
    , m_ent(ent)
    , m_start(m_ent ? Timer::GetThreadCPUTimeNanos() : -1)
  {}
  ~LogTimer() { stop(); }

  void stop() {
    if (m_start == -1) return;

    auto const elapsed = Timer::GetThreadCPUTimeNanos() - m_start;
    m_ent->setInt(m_name, elapsed / 1000);
    m_start = -1;
  }

private:
  const char* m_name;
  OptLog& m_ent;
  int64_t m_start;
};

struct CachedUnit {
  CachedUnit() = default;
  explicit CachedUnit(Unit* unit, size_t rdsBitId)
    : unit(unit)
    , rdsBitId(rdsBitId)
  {}

  Unit* unit{nullptr};  // null if there is no Unit for this path
  size_t rdsBitId{-1u}; // id of the RDS bit for whether the Unit is included
};

//////////////////////////////////////////////////////////////////////
// RepoAuthoritative mode unit caching

/*
 * In RepoAuthoritative mode, loaded units are never unloaded, we
 * don't support symlink chasing, you can't include urls, and files
 * are never changed, which makes the code here significantly simpler.
 * Because of this it pays to keep it separate from the other cases so
 * they don't need to be littered with RepoAuthoritative checks.
 */

using RepoUnitCache = RankedCHM<
  const StringData*,     // must be static
  CachedUnit,
  StringDataHashCompare,
  RankUnitCache
>;
RepoUnitCache s_repoUnitCache;

CachedUnit lookupUnitRepoAuth(const StringData* path) {
  path = makeStaticString(path);

  RepoUnitCache::accessor acc;
  if (!s_repoUnitCache.insert(acc, path)) {
    return acc->second;
  }

  try {
    /*
     * Insert path.  Find the Md5 for this path, and then the unit for
     * this Md5.  If either aren't found we return the
     * default-constructed cache entry.
     *
     * NB: we're holding the CHM lock on this bucket while we're doing
     * this.
     */
    MD5 md5;
    if (Repo::get().findFile(path->data(),
                             RuntimeOption::SourceRoot,
                             md5) == RepoStatus::error) {
      return acc->second;
    }

    acc->second.unit = Repo::get().loadUnit(path->data(), md5).release();
    if (acc->second.unit) {
      acc->second.rdsBitId = rds::allocBit();
    }
  } catch (...) {
    s_repoUnitCache.erase(acc);
    throw;
  }
  return acc->second;
}

//////////////////////////////////////////////////////////////////////
// Non-repo mode unit caching

struct CachedUnitWithFree {
  CachedUnitWithFree() = delete;
  explicit CachedUnitWithFree(const CachedUnitWithFree&) = delete;
  CachedUnitWithFree& operator=(const CachedUnitWithFree&) = delete;

  explicit CachedUnitWithFree(const CachedUnit& src) : cu(src) {}
  ~CachedUnitWithFree() {
    if (auto oldUnit = cu.unit) {
      Treadmill::enqueue([oldUnit] { delete oldUnit; });
    }
  }
  CachedUnit cu;
};

struct CachedUnitNonRepo {
  std::shared_ptr<CachedUnitWithFree> cachedUnit;
#ifdef _MSC_VER
  time_t mtime;
#else
  struct timespec mtime;
  struct timespec ctime;
#endif
  ino_t ino;
  dev_t devId;
};

using NonRepoUnitCache = RankedCHM<
  const StringData*,     // must be static
  CachedUnitNonRepo,
  StringDataHashCompare,
  RankUnitCache
>;
NonRepoUnitCache s_nonRepoUnitCache;

// When running in remote unix server mode with UnixServerQuarantineUnits set,
// we need to cache any unit generated from a file descriptor passed via the
// client process in a special quarantined cached. Units in this cache may only
// be loaded in unix server requests from the same user and are never used when
// handling web requests.
using PerUserCache = folly::AtomicHashMap<uid_t, NonRepoUnitCache*>;
PerUserCache s_perUserUnitCaches(10);

#ifndef _MSC_VER
int64_t timespecCompare(const struct timespec& l,
                        const struct timespec& r) {
  if (l.tv_sec != r.tv_sec) return l.tv_sec - r.tv_sec;
  return l.tv_nsec - r.tv_nsec;
}
#endif

uint64_t g_units_seen_count = 0;

bool stressUnitCache() {
  if (RuntimeOption::EvalStressUnitCacheFreq <= 0) return false;
  if (RuntimeOption::EvalStressUnitCacheFreq == 1) return true;
  return ++g_units_seen_count % RuntimeOption::EvalStressUnitCacheFreq == 0;
}

bool isChanged(const CachedUnitNonRepo& cu, const struct stat& s) {
  // If the cached unit is null, we always need to consider it out of date (in
  // case someone created the file).  This case should only happen if something
  // successfully stat'd the file, but then it was gone by the time we tried to
  // open() it.
  return !cu.cachedUnit ||
         cu.cachedUnit->cu.unit == nullptr ||
#ifdef _MSC_VER
         cu.mtime - s.st_mtime < 0 ||
#else
         timespecCompare(cu.mtime, s.st_mtim) < 0 ||
         timespecCompare(cu.ctime, s.st_ctim) < 0 ||
#endif
         cu.ino != s.st_ino ||
         cu.devId != s.st_dev ||
         stressUnitCache();
}

folly::Optional<String> readFileAsString(Stream::Wrapper* w,
                                         const StringData* path) {
  if (w) {
    if (const auto f = w->open(StrNR(path), "r", 0, nullptr)) {
      return f->read();
    }
    return folly::none;
  }
  auto const fd = open(path->data(), O_RDONLY);
  if (fd < 0) return folly::none;
  auto file = req::make<PlainFile>(fd);
  return file->read();
}

CachedUnit createUnitFromString(const char* path,
                                const String& contents,
                                Unit** releaseUnit,
                                OptLog& ent) {
  auto const md5 = MD5{mangleUnitMd5(string_md5(contents.slice()))};
  // Try the repo; if it's not already there, invoke the compiler.
  if (auto unit = Repo::get().loadUnit(path, md5)) {
    return CachedUnit { unit.release(), rds::allocBit() };
  }
  LogTimer compileTimer("compile_ms", ent);
  auto const unit = compile_file(contents.data(), contents.size(), md5, path,
                                 releaseUnit);
  return CachedUnit { unit, rds::allocBit() };
}

CachedUnit createUnitFromUrl(const StringData* const requestedPath) {
  auto const w = Stream::getWrapperFromURI(StrNR(requestedPath));
  if (!w) return CachedUnit{};
  auto const f = w->open(StrNR(requestedPath), "r", 0, nullptr);
  if (!f) return CachedUnit{};
  StringBuffer sb;
  sb.read(f.get());
  OptLog ent;
  return createUnitFromString(requestedPath->data(), sb.detach(), nullptr, ent);
}

CachedUnit createUnitFromFile(const StringData* const path,
                              Unit** releaseUnit, Stream::Wrapper* w,
                              OptLog& ent) {
  auto const contents = readFileAsString(w, path);
  return contents
    ? createUnitFromString(path->data(), *contents, releaseUnit, ent)
    : CachedUnit{};
}

// When running via the CLI server special access checks may need to be
// performed, and in the event that the server is unable to load the file an
// alternative per client cache may be used.
NonRepoUnitCache& getNonRepoCache(const StringData* rpath,
                                  Stream::Wrapper*& w) {
  if (auto uc = get_cli_ucred()) {
    if (!(w = Stream::getWrapperFromURI(StrNR(rpath)))) {
      return s_nonRepoUnitCache;
    }

    auto unit_check_quarantine = [&] () -> NonRepoUnitCache& {
      if (!RuntimeOption::EvalUnixServerQuarantineUnits) {
        return s_nonRepoUnitCache;
      }
      auto iter = s_perUserUnitCaches.find(uc->uid);
      if (iter != s_perUserUnitCaches.end()) return *iter->second;
      auto cache = new NonRepoUnitCache;
      auto res = s_perUserUnitCaches.insert(uc->uid, cache);
      if (!res.second) delete cache;
      return *res.first->second;
    };

    // If the server cannot access rpath attempt to open the unit on the
    // client. When UnixServerQuarantineUnits is set store units opened by
    // clients in per UID caches which are never accessible by server web
    // requests.
    if (access(rpath->data(), R_OK) == -1) {
      return unit_check_quarantine();
    }

    // When UnixServerVerifyExeAccess is set clients may not execute units if
    // they cannot read them, even when the server has access. To verify that
    // clients have access they are asked to open the file for read access,
    // and using fstat the server verifies that the file it sees is identical
    // to the unit opened by the client.
    if (RuntimeOption::EvalUnixServerVerifyExeAccess) {
      struct stat local, remote;
      auto remoteFile = w->open(StrNR(rpath), "r", 0, nullptr);
      if (!remoteFile ||
          fcntl(remoteFile->fd(), F_GETFL) != O_RDONLY ||
          fstat(remoteFile->fd(), &remote) != 0 ||
          stat(rpath->data(), &local) != 0 ||
          remote.st_dev != local.st_dev ||
          remote.st_ino != local.st_ino) {
        return unit_check_quarantine();
      }
    }

    // When the server is able to read the file prefer to access it that way,
    // in all modes units loaded by the server are cached for all clients.
    w = nullptr;
  }
  return s_nonRepoUnitCache;
}

CachedUnit loadUnitNonRepoAuth(StringData* requestedPath,
                               const struct stat& statInfo,
                               OptLog& ent) {
  LogTimer loadTime("load_ms", ent);
  if (strstr(requestedPath->data(), "://") != nullptr) {
    // URL-based units are not currently cached in memory, but the Repo still
    // caches them on disk.
    return createUnitFromUrl(requestedPath);
  }

  // The string we're using as a key must be static, because we're using it as
  // a key in the cache (across requests).
  auto const path =
    makeStaticString(
      // XXX: it seems weird we have to do this even though we already ran
      // resolveVmInclude.
      (FileUtil::isAbsolutePath(requestedPath->toCppString())
       ?  String{requestedPath}
        : String(SourceRootInfo::GetCurrentSourceRoot()) + StrNR(requestedPath)
      ).get()
    );

  auto const rpath = [&] () -> const StringData* {
    if (RuntimeOption::CheckSymLink) {
      std::string rp = StatCache::realpath(path->data());
      if (rp.size() != 0) {
        if (rp.size() != path->size() ||
            memcmp(rp.data(), path->data(), rp.size())) {
          return makeStaticString(rp);
        }
      }
    }
    return path;
  }();

  Stream::Wrapper* w = nullptr;
  auto& cache = getNonRepoCache(rpath, w);

  assert(
    !w || &cache != &s_nonRepoUnitCache ||
    !RuntimeOption::EvalUnixServerQuarantineUnits
  );

  // Freeing a unit while holding the tbb lock would cause a rank violation when
  // recycle-tc is enabled as reclaiming dead functions requires that the code
  // and metadata locks be acquired.
  Unit* releaseUnit = nullptr;
  SCOPE_EXIT { if (releaseUnit) delete releaseUnit; };

  auto const cuptr = [&] () -> std::shared_ptr<CachedUnitWithFree> {
    NonRepoUnitCache::accessor rpathAcc;

    if (!cache.insert(rpathAcc, rpath)) {
      if (!isChanged(rpathAcc->second, statInfo)) {
        if (ent) ent->setStr("type", "cache_hit_writelock");
        return rpathAcc->second.cachedUnit;
      }
      if (ent) ent->setStr("type", "cache_stale");
    } else {
      if (ent) ent->setStr("type", "cache_miss");
    }

    /*
     * NB: the new-unit creation path is here, and is done while holding the tbb
     * lock on s_nonRepoUnitCache.  This was originally done deliberately to
     * avoid wasting time in the compiler (during server startup, many requests
     * hit the same code initial paths that are shared, and would all be
     * compiling the same files).  It's not 100% clear if this is the best way
     * to handle that idea, though (tbb locks spin aggressively and are
     * expected to be low contention).
     */

    auto const cu = createUnitFromFile(rpath, &releaseUnit, w, ent);
    rpathAcc->second.cachedUnit = std::make_shared<CachedUnitWithFree>(cu);
#ifdef _MSC_VER
    rpathAcc->second.mtime      = statInfo.st_mtime;
#else
    rpathAcc->second.mtime      = statInfo.st_mtim;
    rpathAcc->second.ctime      = statInfo.st_ctim;
#endif
    rpathAcc->second.ino        = statInfo.st_ino;
    rpathAcc->second.devId      = statInfo.st_dev;

    return rpathAcc->second.cachedUnit;
  }();

  if (path != rpath) {
    NonRepoUnitCache::accessor pathAcc;
    cache.insert(pathAcc, path);
    pathAcc->second.cachedUnit = cuptr;
#ifdef _MSC_VER
    pathAcc->second.mtime      = statInfo.st_mtime;
#else
    pathAcc->second.mtime      = statInfo.st_mtim;
    pathAcc->second.ctime      = statInfo.st_ctim;
#endif
    pathAcc->second.ino        = statInfo.st_ino;
    pathAcc->second.devId      = statInfo.st_dev;
  }

  return cuptr->cu;
}

CachedUnit lookupUnitNonRepoAuth(StringData* requestedPath,
                                 const struct stat& statInfo,
                                 OptLog& ent) {
  // Steady state, its probably already in the cache. Try that first
  {
    NonRepoUnitCache::const_accessor acc;
    if (s_nonRepoUnitCache.find(acc, requestedPath)) {
      if (!isChanged(acc->second, statInfo)) {
        if (ent) ent->setStr("type", "cache_hit_readlock");
        return acc->second.cachedUnit->cu;
      }
    }
  }
  return loadUnitNonRepoAuth(requestedPath, statInfo, ent);
}

//////////////////////////////////////////////////////////////////////
// resolveVmInclude callbacks

struct ResolveIncludeContext {
  String path;    // translated path of the file
  struct stat* s; // stat for the file
  bool allow_dir; // return true for dirs?
};

bool findFile(const StringData* path, struct stat* s, bool allow_dir,
              Stream::Wrapper* w) {
  // We rely on this side-effect in RepoAuthoritative mode right now, since the
  // stat information is an output-param of resolveVmInclude, but we aren't
  // really going to call stat.
  s->st_mode = 0;

  if (RuntimeOption::RepoAuthoritative) {
    return lookupUnitRepoAuth(path).unit != nullptr;
  }

  if (StatCache::stat(path->data(), s) == 0) {
    // The call explicitly populates the struct for dirs, but returns false for
    // them because it is geared toward file includes.
    return allow_dir || !S_ISDIR(s->st_mode);
  }

  if (w && w->stat(StrNR(path), s) == 0) {
    return allow_dir || !S_ISDIR(s->st_mode);
  }
  return false;
}

bool findFileWrapper(const String& file, void* ctx) {
  auto const context = static_cast<ResolveIncludeContext*>(ctx);
  assert(context->path.isNull());

  Stream::Wrapper* w = Stream::getWrapperFromURI(file);
  if (w && !w->isNormalFileStream()) {
    if (w->stat(file, context->s) == 0) {
      context->path = file;
      return true;
    }
  }

  // handle file://
  if (StringUtil::IsFileUrl(file)) {
    return findFileWrapper(file.substr(7), ctx);
  }

  if (!w) return false;

  auto passW =
    RuntimeOption::RepoAuthoritative ||
    dynamic_cast<FileStreamWrapper*>(w) ||
    !w->isNormalFileStream()
      ? nullptr
      : w;

  // TranslatePath() will canonicalize the path and also check
  // whether the file is in an allowed directory.
  String translatedPath = File::TranslatePathKeepRelative(file);
  if (!FileUtil::isAbsolutePath(file.toCppString())) {
    if (findFile(translatedPath.get(), context->s, context->allow_dir,
                 passW)) {
      context->path = translatedPath;
      return true;
    }
    return false;
  }
  if (RuntimeOption::SandboxMode || !RuntimeOption::AlwaysUseRelativePath) {
    if (findFile(translatedPath.get(), context->s, context->allow_dir, passW)) {
      context->path = translatedPath;
      return true;
    }
  }
  std::string server_root(SourceRootInfo::GetCurrentSourceRoot());
  if (server_root.empty()) {
    server_root = std::string(g_context->getCwd().data());
    if (server_root.empty() ||
        FileUtil::isDirSeparator(server_root[server_root.size() - 1])) {
      server_root += FileUtil::getDirSeparator();
    }
  }
  String rel_path(FileUtil::relativePath(server_root, translatedPath.data()));
  if (findFile(rel_path.get(), context->s, context->allow_dir, passW)) {
    context->path = rel_path;
    return true;
  }
  return false;
}

void logLoad(
  StructuredLogEntry& ent,
  StringData* path,
  const char* cwd,
  String rpath,
  const CachedUnit& cu
) {
  ent.setStr("include_path", path->data());
  ent.setStr("current_dir", cwd);
  ent.setStr("resolved_path", rpath.data());
  if (auto const u = cu.unit) {
    const StringData* err;
    int line;
    if (u->compileTimeFatal(err, line)) {
      ent.setStr("result", "compile_fatal");
      ent.setStr("error", err->data());
    } else if (u->parseFatal(err, line)) {
      ent.setStr("result", "parse_fatal");
      ent.setStr("error", err->data());
    } else {
      ent.setStr("result", "success");
    }

    ent.setStr("md5", u->md5().toString());
    ent.setStr("repo_sn", folly::to<std::string>(u->sn()));
    ent.setStr("repo_id", folly::to<std::string>(u->repoID()));

    ent.setInt("bc_len", u->bclen());
    ent.setInt("num_litstrs", u->numLitstrs());
    ent.setInt("num_funcs", u->funcs().size());
    ent.setInt("num_classes", u->preclasses().size());
    ent.setInt("num_type_aliases", u->typeAliases().size());
  } else {
    ent.setStr("result", "file_not_found");
  }

  switch (requestKind) {
  case RequestKind::Warmup: ent.setStr("request_kind", "warmup"); break;
  case RequestKind::Profile: ent.setStr("request_kind", "profile"); break;
  case RequestKind::Standard: ent.setStr("request_kind", "standard"); break;
  }
  ent.setInt("request_count", requestCount());

  StructuredLog::log("hhvm_unit_cache", ent);
}

//////////////////////////////////////////////////////////////////////

CachedUnit checkoutFile(
  StringData* path,
  const struct stat& statInfo,
  OptLog& ent
) {
  return RuntimeOption::RepoAuthoritative
    ? lookupUnitRepoAuth(path)
    : lookupUnitNonRepoAuth(path, statInfo, ent);
}

//////////////////////////////////////////////////////////////////////

} // end empty namespace

//////////////////////////////////////////////////////////////////////

const std::string mangleUnitPHP7Options() {
  // As the list of options increases, we may want to do something smarter here?
  std::string s;
  s += (RuntimeOption::PHP7_IntSemantics ? '1' : '0') +
      (RuntimeOption::PHP7_LTR_assign ? '1' : '0') +
      (RuntimeOption::PHP7_NoHexNumerics ? '1' : '0') +
      (RuntimeOption::PHP7_Builtins ? '1' : '0') +
      (RuntimeOption::PHP7_ScalarTypes ? '1' : '0') +
      (RuntimeOption::PHP7_Substr ? '1' : '0') +
      (RuntimeOption::PHP7_UVS ? '1' : '0');
  return s;
}

const std::string mangleAliasedNamespaces() {
  std::string s;
  s += folly::to<std::string>(RuntimeOption::AliasedNamespaces.size());
  s += '\0';
  for (auto& par : RuntimeOption::AliasedNamespaces) {
    s += par.first + '\0' + par.second + '\0';
  }
  return s;
}

std::string mangleUnitMd5(const std::string& fileMd5) {
  std::string t = fileMd5 + '\0'
    + (RuntimeOption::EvalEmitSwitch ? '1' : '0')
    + (RuntimeOption::EnableHipHopExperimentalSyntax ? '1' : '0')
    + (RuntimeOption::EnableHipHopSyntax ? '1' : '0')
    + (RuntimeOption::EnableXHP ? '1' : '0')
    + (RuntimeOption::EvalAllowHhas ? '1' : '0')
    + (RuntimeOption::EvalJitEnableRenameFunction ? '1' : '0')
    + (RuntimeOption::IntsOverflowToInts ? '1' : '0')
    + (RuntimeOption::EvalEnableCallBuiltin ? '1' : '0')
    + (RuntimeOption::AssertEmitted ? '1' : '0')
    + RuntimeOption::EvalHackCompilerCommand + '\0'
    + (RuntimeOption::EvalHackCompilerFallback ? '1' : '0')
    + (RuntimeOption::EvalHackCompilerVerify ? '1' : '0')
    + (RuntimeOption::AutoprimeGenerators ? '1' : '0')
    + (RuntimeOption::EvalHackArrCompatNotices ? '1' : '0')
    + (RuntimeOption::EvalLoadFilepathFromUnitCache ? '1' : '0')
    + (RuntimeOption::EvalAllowHhas ? '1' : '0')
    + mangleUnitPHP7Options()
    + mangleAliasedNamespaces();
  return string_md5(t);
}

size_t numLoadedUnits() {
  if (RuntimeOption::RepoAuthoritative) {
    return s_repoUnitCache.size();
  }
  return s_nonRepoUnitCache.size();
}

String resolveVmInclude(StringData* path,
                        const char* currentDir,
                        struct stat* s,
                        bool allow_dir /* = false */) {
  ResolveIncludeContext ctx;
  ctx.s = s;
  ctx.allow_dir = allow_dir;
  void* vpCtx = &ctx;
  resolve_include(String{path}, currentDir, findFileWrapper, vpCtx);
  // If resolve_include() could not find the file, return NULL
  return ctx.path;
}

Unit* lookupUnit(StringData* path, const char* currentDir, bool* initial_opt) {
  bool init;
  bool& initial = initial_opt ? *initial_opt : init;
  initial = true;

  OptLog ent;
  if (!RuntimeOption::RepoAuthoritative &&
      StructuredLog::coinflip(RuntimeOption::EvalLogUnitLoadRate)) {
    ent.emplace();
  }

  LogTimer lookupTimer("lookup_ms", ent);

  /*
   * NB: the m_evaledFiles map is only for the debugger, and could be omitted
   * in RepoAuthoritative mode, but currently isn't.
   */

  struct stat s;
  auto const spath = resolveVmInclude(path, currentDir, &s);
  if (spath.isNull()) return nullptr;

  auto const eContext = g_context.getNoCheck();

  // Check if this file has already been included.
  auto it = eContext->m_evaledFiles.find(spath.get());
  if (it != end(eContext->m_evaledFiles)) {
    // In RepoAuthoritative mode we assume that the files are unchanged.
    initial = false;
    if (RuntimeOption::RepoAuthoritative ||
        (it->second.ts_sec > s.st_mtime) ||
        ((it->second.ts_sec == s.st_mtime) &&
         (it->second.ts_nsec >= s.st_mtim.tv_nsec))) {
      return it->second.unit;
    }
  }

  // This file hasn't been included yet, so we need to parse the file
  auto const cunit = checkoutFile(spath.get(), s, ent);
  if (cunit.unit && initial_opt) {
    // if initial_opt is not set, this shouldn't be recorded as a
    // per request fetch of the file.
    if (rds::testAndSetBit(cunit.rdsBitId)) {
      initial = false;
    }
    // if parsing was successful, update the mappings for spath and
    // rpath (if it exists).
    eContext->m_evaledFilesOrder.push_back(cunit.unit->filepath());
    eContext->m_evaledFiles[spath.get()] =
      {cunit.unit, s.st_mtime, static_cast<unsigned long>(s.st_mtim.tv_nsec)};
    spath.get()->incRefCount();
    if (!cunit.unit->filepath()->same(spath.get())) {
      eContext->m_evaledFiles[cunit.unit->filepath()] =
        {cunit.unit, s.st_mtime, static_cast<unsigned long>(s.st_mtim.tv_nsec)};
    }
    if (g_system_profiler) {
      g_system_profiler->fileLoadCallBack(path->toCppString());
    }
    DEBUGGER_ATTACHED_ONLY(phpDebuggerFileLoadHook(cunit.unit));
  }

  lookupTimer.stop();
  if (ent) logLoad(*ent, path, currentDir, spath, cunit);
  return cunit.unit;
}

//////////////////////////////////////////////////////////////////////

void preloadRepo() {
  auto& repo = Repo::get();
  auto units = repo.enumerateUnits(RepoIdLocal, true, false);
  if (units.size() == 0) {
    units = repo.enumerateUnits(RepoIdCentral, true, false);
  }
  if (!units.size()) return;

  std::vector<std::thread> workers;
  auto numWorkers = Process::GetCPUCount();
  // Compute a batch size that causes each thread to process approximately 16
  // batches.  Even if the batches are somewhat imbalanced in what they contain,
  // the straggler workers are very unlikey to take more than 10% longer than
  // the first worker to finish.
  size_t batchSize{std::max(units.size() / numWorkers / 16, size_t(1))};
  std::atomic<size_t> index{0};
  for (auto worker = 0; worker < numWorkers; ++worker) {
    workers.push_back(std::thread([&] {
      hphp_thread_init();
      hphp_session_init();

      while (true) {
        auto begin = index.fetch_add(batchSize);
        auto end = std::min(begin + batchSize, units.size());
        if (begin >= end) break;
        auto unitCount = end - begin;
        for (auto i = size_t{0}; i < unitCount; ++i) {
          auto& kv = units[begin + i];
          try {
            lookupUnit(String(RuntimeOption::SourceRoot + kv.first).get(),
                       "", nullptr);
          } catch (...) {
            // swallow errors silently
          }
        }
      }

      hphp_context_exit();
      hphp_session_exit();
      hphp_thread_exit();
    }));
  }
  for (auto& worker : workers) {
    worker.join();
  }
}

//////////////////////////////////////////////////////////////////////

}
