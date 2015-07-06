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
#include "hphp/runtime/base/unit-cache.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <memory>
#include <string>
#include <cstdlib>
#include <thread>

#include <folly/ScopeGuard.h>

#include "hphp/util/assertions.h"
#include "hphp/util/rank.h"
#include "hphp/util/mutex.h"
#include "hphp/util/process.h"
#include "hphp/runtime/base/system-profiler.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/base/string-util.h"
#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/base/plain-file.h"
#include "hphp/runtime/base/stat-cache.h"
#include "hphp/runtime/base/stream-wrapper-registry.h"
#include "hphp/runtime/base/file-stream-wrapper.h"
#include "hphp/runtime/base/profile-dump.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/server/source-root-info.h"
#include "hphp/runtime/vm/debugger-hook.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/treadmill.h"

#ifdef __APPLE__
#define st_mtim st_mtimespec
#endif

namespace HPHP {

//////////////////////////////////////////////////////////////////////

namespace {

//////////////////////////////////////////////////////////////////////

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

  /*
   * Insert path.  Find the Md5 for this path, and then the unit for
   * this Md5.  If either aren't found we return the
   * default-constructed cache entry.
   *
   * NB: we're holding the CHM lock on this bucket while we're doing
   * this.
   */
  MD5 md5;
  if (!Repo::get().findFile(path->data(),
                            RuntimeOption::SourceRoot,
                            md5)) {
    return acc->second;
  }

  acc->second.unit = Repo::get().loadUnit(path->data(), md5).release();
  if (acc->second.unit) {
    acc->second.rdsBitId = rds::allocBit();
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
  struct timespec mtime;
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

int64_t timespecCompare(const struct timespec& l,
                        const struct timespec& r) {
  if (l.tv_sec != r.tv_sec) return l.tv_sec - r.tv_sec;
  return l.tv_nsec - r.tv_nsec;
}

bool isChanged(const CachedUnitNonRepo& cu, const struct stat& s) {
  // If the cached unit is null, we always need to consider it out of date (in
  // case someone created the file).  This case should only happen if something
  // successfully stat'd the file, but then it was gone by the time we tried to
  // open() it.
  return !cu.cachedUnit ||
         cu.cachedUnit->cu.unit == nullptr ||
         timespecCompare(cu.mtime, s.st_mtim) < 0 ||
         cu.ino != s.st_ino ||
         cu.devId != s.st_dev;
}

folly::Optional<String> readFileAsString(const StringData* path) {
  auto const fd = open(path->data(), O_RDONLY);
  if (!fd) return folly::none;
  auto file = req::make<PlainFile>(fd);
  return file->read();
}

CachedUnit createUnitFromString(const char* path,
                                const String& contents) {
  auto const md5 = MD5 {
    mangleUnitMd5(string_md5(contents.data(), contents.size())).c_str()
  };
  // Try the repo; if it's not already there, invoke the compiler.
  if (auto unit = Repo::get().loadUnit(path, md5)) {
    return CachedUnit { unit.release(), rds::allocBit() };
  }
  auto const unit = compile_file(contents.data(), contents.size(), md5, path);
  return CachedUnit { unit, rds::allocBit() };
}

CachedUnit createUnitFromUrl(const StringData* const requestedPath) {
  auto const w = Stream::getWrapperFromURI(StrNR(requestedPath));
  if (!w) return CachedUnit{};
  auto const f = w->open(StrNR(requestedPath), "r", 0, nullptr);
  if (!f) return CachedUnit{};
  StringBuffer sb;
  sb.read(f.get());
  return createUnitFromString(requestedPath->data(), sb.detach());
}

CachedUnit createUnitFromFile(const StringData* const path) {
  auto const contents = readFileAsString(path);
  return contents ? createUnitFromString(path->data(), *contents)
                  : CachedUnit{};
}

CachedUnit loadUnitNonRepoAuth(StringData* requestedPath,
                               const struct stat& statInfo) {
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
      (requestedPath->data()[0] == '/'
        ? requestedPath
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

  auto const cuptr = [&] () -> std::shared_ptr<CachedUnitWithFree> {
    NonRepoUnitCache::accessor rpathAcc;

    if (!s_nonRepoUnitCache.insert(rpathAcc, rpath)) {
      if (!isChanged(rpathAcc->second, statInfo)) {
        return rpathAcc->second.cachedUnit;
      }
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

    auto const cu = createUnitFromFile(rpath);
    rpathAcc->second.cachedUnit = std::make_shared<CachedUnitWithFree>(cu);
    rpathAcc->second.mtime      = statInfo.st_mtim;
    rpathAcc->second.ino        = statInfo.st_ino;
    rpathAcc->second.devId      = statInfo.st_dev;

    return rpathAcc->second.cachedUnit;
  }();

  if (path != rpath) {
    NonRepoUnitCache::accessor pathAcc;
    s_nonRepoUnitCache.insert(pathAcc, path);
    pathAcc->second.cachedUnit = cuptr;
    pathAcc->second.mtime      = statInfo.st_mtim;
    pathAcc->second.ino        = statInfo.st_ino;
    pathAcc->second.devId      = statInfo.st_dev;
  }

  return cuptr->cu;
}

CachedUnit lookupUnitNonRepoAuth(StringData* requestedPath,
                                 const struct stat& statInfo) {
  // Steady state, its probably already in the cache. Try that first
  {
    NonRepoUnitCache::const_accessor acc;
    if (s_nonRepoUnitCache.find(acc, requestedPath)) {
      if (!isChanged(acc->second, statInfo)) {
        return acc->second.cachedUnit->cu;
      }
    }
  }
  return loadUnitNonRepoAuth(requestedPath, statInfo);
}

//////////////////////////////////////////////////////////////////////
// resolveVmInclude callbacks

struct ResolveIncludeContext {
  String path;    // translated path of the file
  struct stat* s; // stat for the file
  bool allow_dir; // return true for dirs?
};

bool findFile(const StringData* path, struct stat* s, bool allow_dir) {
  // We rely on this side-effect in RepoAuthoritative mode right now, since the
  // stat information is an output-param of resolveVmInclude, but we aren't
  // really going to call stat.
  s->st_mode = 0;

  if (RuntimeOption::RepoAuthoritative) {
    return lookupUnitRepoAuth(path).unit != nullptr;
  }

  auto const ret = StatCache::stat(path->data(), s) == 0 &&
    !S_ISDIR(s->st_mode);
  if (S_ISDIR(s->st_mode) && allow_dir) {
    // The call explicitly populates the struct for dirs, but returns false for
    // them because it is geared toward file includes.
    return true;
  }
  return ret;
}

bool findFileWrapper(const String& file, void* ctx) {
  auto const context = static_cast<ResolveIncludeContext*>(ctx);
  assert(context->path.isNull());

  Stream::Wrapper* w = Stream::getWrapperFromURI(file);
  if (w && !dynamic_cast<FileStreamWrapper*>(w)) {
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

  // TranslatePath() will canonicalize the path and also check
  // whether the file is in an allowed directory.
  String translatedPath = File::TranslatePathKeepRelative(file);
  if (file[0] != '/') {
    if (findFile(translatedPath.get(), context->s, context->allow_dir)) {
      context->path = translatedPath;
      return true;
    }
    return false;
  }
  if (RuntimeOption::SandboxMode || !RuntimeOption::AlwaysUseRelativePath) {
    if (findFile(translatedPath.get(), context->s, context->allow_dir)) {
      context->path = translatedPath;
      return true;
    }
  }
  std::string server_root(SourceRootInfo::GetCurrentSourceRoot());
  if (server_root.empty()) {
    server_root = std::string(g_context->getCwd().data());
    if (server_root.empty() || server_root[server_root.size() - 1] != '/') {
      server_root += "/";
    }
  }
  String rel_path(FileUtil::relativePath(server_root, translatedPath.data()));
  if (findFile(rel_path.get(), context->s, context->allow_dir)) {
    context->path = rel_path;
    return true;
  }
  return false;
}

//////////////////////////////////////////////////////////////////////

CachedUnit checkoutFile(StringData* path, const struct stat& statInfo) {
  return RuntimeOption::RepoAuthoritative
    ? lookupUnitRepoAuth(path)
    : lookupUnitNonRepoAuth(path, statInfo);
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

std::string mangleUnitMd5(const std::string& fileMd5) {
  std::string t = fileMd5 + '\0'
    + (RuntimeOption::EnableEmitSwitch ? '1' : '0')
    + (RuntimeOption::EnableHipHopExperimentalSyntax ? '1' : '0')
    + (RuntimeOption::EnableHipHopSyntax ? '1' : '0')
    + (RuntimeOption::EnableXHP ? '1' : '0')
    + (RuntimeOption::EvalAllowHhas ? '1' : '0')
    + (RuntimeOption::EvalJitEnableRenameFunction ? '1' : '0')
    + (RuntimeOption::IntsOverflowToInts ? '1' : '0')
    + (RuntimeOption::EvalEnableCallBuiltin ? '1' : '0');
  return string_md5(t.c_str(), t.size());
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
  resolve_include(path, currentDir, findFileWrapper, vpCtx);
  // If resolve_include() could not find the file, return NULL
  return ctx.path;
}

Unit* lookupUnit(StringData* path, const char* currentDir, bool* initial_opt) {
  bool init;
  bool& initial = initial_opt ? *initial_opt : init;
  initial = true;

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
    initial = false;
    return it->second;
  }

  // This file hasn't been included yet, so we need to parse the file
  auto const cunit = checkoutFile(spath.get(), s);
  if (cunit.unit && initial_opt) {
    // if initial_opt is not set, this shouldn't be recorded as a
    // per request fetch of the file.
    if (rds::testAndSetBit(cunit.rdsBitId)) {
      initial = false;
    }
    // if parsing was successful, update the mappings for spath and
    // rpath (if it exists).
    eContext->m_evaledFilesOrder.push_back(cunit.unit->filepath());
    eContext->m_evaledFiles[spath.get()] = cunit.unit;
    spath.get()->incRefCount();
    if (!cunit.unit->filepath()->same(spath.get())) {
      eContext->m_evaledFiles[cunit.unit->filepath()] = cunit.unit;
    }
    if (g_system_profiler) {
      g_system_profiler->fileLoadCallBack(path->toCppString());
    }
    DEBUGGER_ATTACHED_ONLY(phpDebuggerFileLoadHook(cunit.unit));
  }

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
      hphp_session_init();
      hphp_context_init();

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
