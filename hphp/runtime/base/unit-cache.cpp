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

#include "hphp/runtime/base/autoload-handler.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/base/plain-file.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/stat-cache.h"
#include "hphp/runtime/base/string-util.h"
#include "hphp/runtime/base/system-profiler.h"
#include "hphp/runtime/base/vm-worker.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/server/cli-server.h"
#include "hphp/runtime/server/source-root-info.h"
#include "hphp/runtime/vm/debugger-hook.h"
#include "hphp/runtime/vm/extern-compiler.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/runtime-compiler.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/runtime/vm/unit-emitter.h"

#include "hphp/util/assertions.h"
#include "hphp/util/build-info.h"
#include "hphp/util/mutex.h"
#include "hphp/util/process.h"
#include "hphp/util/rank.h"
#include "hphp/util/struct-log.h"
#include "hphp/util/timer.h"

#include <cstdlib>
#include <memory>
#include <string>
#include <thread>

#include <folly/AtomicHashMap.h>
#include <folly/FileUtil.h>
#include <folly/Optional.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/executors/task_queue/PriorityUnboundedBlockingQueue.h>
#include <folly/portability/Fcntl.h>
#include <folly/portability/SysStat.h>
#include <folly/synchronization/AtomicNotification.h>

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
  Unit* unit{};
  size_t rdsBitId{-1uL};
};

struct CachedUnitInternal {
  CachedUnitInternal() = default;
  CachedUnitInternal(const CachedUnitInternal& src) :
      unit{src.unit.copy()},
      rdsBitId{src.rdsBitId} {}
  CachedUnitInternal& operator=(const CachedUnitInternal&) = delete;

  static Unit* const Uninit;

  CachedUnit cachedUnit() const {
    return CachedUnit { unit.get(), rdsBitId };
  }

  // nullptr if there is no Unit for this path, Uninit if the CachedUnit
  // hasn't been initialized yet.
  mutable LockFreePtrWrapper<Unit*> unit{Uninit};
  // id of the RDS bit for whether the Unit is included
  mutable size_t rdsBitId{-1u};
};

Unit* const CachedUnitInternal::Uninit = reinterpret_cast<Unit*>(-8);

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
  CachedUnitInternal,
  StringDataHashCompare,
  RankUnitCache
>;
RepoUnitCache s_repoUnitCache;

CachedUnit lookupUnitRepoAuth(const StringData* path,
                              const Native::FuncTable& nativeFuncs) {
  tracing::BlockNoTrace _{"lookup-unit-repo-auth"};

  path = makeStaticString(path);

  RepoUnitCache::const_accessor acc;
  s_repoUnitCache.insert(acc, path);
  auto const& cu = acc->second;

  if (cu.unit.copy() != CachedUnitInternal::Uninit) return cu.cachedUnit();

  cu.unit.lock_for_update();
  if (cu.unit.copy() != CachedUnitInternal::Uninit) {
    // Someone else updated the unit while we were waiting on the lock
    cu.unit.unlock();
    return cu.cachedUnit();
  }

  try {
    /*
     * We got the lock, so we're responsible for updating the entry.
     */
    SHA1 sha1;
    if (Repo::get().findFile(path->data(),
                             RuntimeOption::SourceRoot,
                             sha1) == RepoStatus::error) {
      cu.unit.update_and_unlock(nullptr);
      return cu.cachedUnit();
    }

    auto unit = Repo::get().loadUnit(
        path->data(),
        sha1,
        nativeFuncs)
      .release();
    if (unit) {
      cu.rdsBitId = rds::allocBit();
    }
    cu.unit.update_and_unlock(std::move(unit));
  } catch (...) {
    cu.unit.unlock();
    s_repoUnitCache.erase(acc);
    throw;
  }
  return cu.cachedUnit();
}

//////////////////////////////////////////////////////////////////////
// Non-repo mode unit caching

struct CachedUnitWithFree {
  CachedUnitWithFree() = delete;
  explicit CachedUnitWithFree(const CachedUnitWithFree&) = delete;
  CachedUnitWithFree& operator=(const CachedUnitWithFree&) = delete;

  explicit CachedUnitWithFree(
    const CachedUnit& src,
    const struct stat* statInfo,
    const RepoOptions& options
  ) : cu(src)
    , repoOptionsHash(options.cacheKeySha1())
  {
    if (statInfo) {
#ifdef _MSC_VER
      mtime      = statInfo->st_mtime;
#else
      mtime      = statInfo->st_mtim;
      ctime      = statInfo->st_ctim;
#endif
      ino        = statInfo->st_ino;
      devId      = statInfo->st_dev;
    }
  }
  ~CachedUnitWithFree() {
    if (!cu.unit) return;
    Treadmill::enqueue([u = cu.unit] { delete u; });
  }
  CachedUnit cu;

#ifdef _MSC_VER
  mutable time_t mtime;
#else
  mutable struct timespec mtime;
  mutable struct timespec ctime;
#endif
  mutable ino_t ino;
  mutable dev_t devId;

  SHA1 repoOptionsHash;
};

using CachedUnitWithFreePtr = copy_ptr<CachedUnitWithFree>;

struct CachedUnitNonRepo {
  CachedUnitNonRepo() = default;
  CachedUnitNonRepo(const CachedUnitNonRepo& other) :
      cachedUnit{other.cachedUnit.copy()} {}
  CachedUnitNonRepo& operator=(const CachedUnitNonRepo&) = delete;

  mutable LockFreePtrWrapper<CachedUnitWithFreePtr> cachedUnit;
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

bool isChanged(
  CachedUnitWithFreePtr cachedUnit,
  const struct stat* s,
  const RepoOptions& options
) {
  // If the cached unit is null, we always need to consider it out of date (in
  // case someone created the file).  This case should only happen if something
  // successfully stat'd the file, but then it was gone by the time we tried to
  // open() it.
  if (!s) return false;
  return !cachedUnit ||
         cachedUnit->cu.unit == nullptr ||
#ifdef _MSC_VER
         cachedUnit->mtime - s->st_mtime < 0 ||
#else
         timespecCompare(cachedUnit->mtime, s->st_mtim) < 0 ||
         timespecCompare(cachedUnit->ctime, s->st_ctim) < 0 ||
#endif
         cachedUnit->ino != s->st_ino ||
         cachedUnit->devId != s->st_dev ||
         cachedUnit->repoOptionsHash != SHA1{options.cacheKeySha1()} ||
         stressUnitCache();
}

folly::Optional<std::string> readFileAsString(const StringData* path) {
  tracing::Block _{
    "read-file", [&] { return tracing::Props{}.add("path", path); }
  };
  // If the file is too large it may OOM the request
  MemoryManager::SuppressOOM so(*tl_heap);
  std::string contents;
  if (!folly::readFile(path->data(), contents)) return folly::none;
  return contents;
}

CachedUnitWithFreePtr createUnitFromFile(const StringData* const path,
                                         Unit** releaseUnit,
                                         OptLog& ent,
                                         const Native::FuncTable& nativeFuncs,
                                         const RepoOptions& options,
                                         FileLoadFlags& flags,
                                         const struct stat* statInfo,
                                         CachedUnitWithFreePtr orig) {
  LogTimer readUnitTimer("read_unit_ms", ent);

  auto const contents = readFileAsString(path);
  if (!contents) return CachedUnitWithFreePtr{};

  readUnitTimer.stop();

  LogTimer generateSha1Timer("generate_sha1_ms", ent);
  auto const sha1 =
    SHA1{mangleUnitSha1(string_sha1(*contents), path->slice(), options)};
  generateSha1Timer.stop();

  // The stat may have indicated that the file was touched, but the
  // contents may not have actually changed. In that case, the hash we
  // just calculated may be the same as the pre-existing Unit's
  // hash. In that case, we just use the old unit.
  if (orig && orig->cu.unit && sha1 == orig->cu.unit->sha1()) return orig;

  auto const check = [&] (Unit* unit) {
    // Similar check as above, but on the hashed HHAS (as
    // opposed to the file contents). If they turn out identical
    // to the old Unit, keep the old Unit.
    if (orig && orig->cu.unit && unit &&
        unit->bcSha1() == orig->cu.unit->bcSha1()) {
      delete unit;
      return orig;
    }
    flags = FileLoadFlags::kEvicted;
    return CachedUnitWithFreePtr{
      CachedUnit { unit, unit ? rds::allocBit() : -1 },
      statInfo,
      options
    };
  };

  // Try the repo; if it's not already there, invoke the compiler.
  if (auto unit = Repo::get().loadUnit(path->slice(), sha1, nativeFuncs)) {
    flags = FileLoadFlags::kHitDisk;
    return check(unit.release());
  }

  LogTimer compileTimer("compile_ms", ent);
  rqtrace::EventGuard trace{"COMPILE_UNIT"};
  trace.annotate("file_size", folly::to<std::string>(contents->size()));
  flags = FileLoadFlags::kCompiled;
  auto const unit = compile_file(
    contents->data(),
    contents->size(),
    sha1,
    path->data(),
    nativeFuncs, options, releaseUnit
  );
  return check(unit);
}

// When running via the CLI server special access checks may need to be
// performed, and in the event that the server is unable to load the file an
// alternative per client cache may be used.
NonRepoUnitCache& getNonRepoCache(uid_t uid, const StringData* rpath) {
  auto const unit_check_quarantine = [&] () -> NonRepoUnitCache& {
    if (!RuntimeOption::EvalUnixServerQuarantineUnits) {
      return s_nonRepoUnitCache;
    }
    auto iter = s_perUserUnitCaches.find(uid);
    if (iter != s_perUserUnitCaches.end()) return *iter->second;
    auto cache = new NonRepoUnitCache;
    auto res = s_perUserUnitCaches.insert(uid, cache);
    if (!res.second) delete cache;
    return *res.first->second;
  };

  // If the server cannot access rpath attempt to open the unit on the
  // client. When UnixServerQuarantineUnits is set store units opened by
  // clients in per UID caches which are never accessible by server web
  // requests.
  if (access(rpath->data(), R_OK) == -1) return unit_check_quarantine();

  // When UnixServerVerifyExeAccess is set clients may not execute units if
  // they cannot read them, even when the server has access. To verify that
  // clients have access they are asked to open the file for read access,
  // and using fstat the server verifies that the file it sees is identical
  // to the unit opened by the client.
  if (RuntimeOption::EvalUnixServerVerifyExeAccess) {
    struct stat local, remote;
    auto const fd = open(rpath->data(), O_RDONLY);
    SCOPE_EXIT { close(fd); };
    if (fd < 0 ||
        fcntl(fd, F_GETFL) != O_RDONLY ||
        fstat(fd, &remote) != 0 ||
        stat(rpath->data(), &local) != 0 ||
        remote.st_dev != local.st_dev ||
        remote.st_ino != local.st_ino) {
      return unit_check_quarantine();
    }
  }

  return s_nonRepoUnitCache;
}

NonRepoUnitCache& getNonRepoCache(const StringData* rpath) {
  if (auto const uc = get_cli_ucred()) {
    return getNonRepoCache(uc->uid, rpath);
  }
  return s_nonRepoUnitCache;
}

std::pair<const StringData*, const StringData*>
resolveRequestedPath(const StringData* requestedPath, bool alreadyRealpath) {
  // The string we're using as a key must be static, because we're using it as
  // a key in the cache (across requests).
  auto const path = [&] {
    // XXX: it seems weird we have to do this even though we already ran
    // resolveVmInclude.
    if (FileUtil::isAbsolutePath(requestedPath->slice())) {
      return makeStaticString(requestedPath);
    }
    return makeStaticString(
      (String{SourceRootInfo::GetCurrentSourceRoot()} +
       StrNR{requestedPath}).get()
    );
  }();

  auto const rpath = [&] () -> const StringData* {
    if (RuntimeOption::CheckSymLink && !alreadyRealpath) {
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

  return std::make_pair(path, rpath);
}

//////////////////////////////////////////////////////////////////////
// Unit prefetching

struct StaticStringCompare {
  bool equal(const StringData* s1, const StringData* s2) const {
    assertx(s1);
    assertx(s2);
    assertx(s1->isStatic());
    assertx(s2->isStatic());
    return s1 == s2;
  }
  size_t hash(const StringData* s) const {
    assertx(s);
    assertx(s->isStatic());
    return hash_int64(reinterpret_cast<uintptr_t>(s));
  }
};

// To avoid enqueueing multiple identical prefetch requests, we
// maintain a global table of the timestamp of the last prefetch
// request for each path. We'll only enqueue a new prefetch request
// for that path if more than some amount has passed since the last
// attempt. Note that we store paths (without normalization), and a
// given unit might be referred to by multiple paths. This is fine
// because it just means we'll do a bit of extra work.
//
// Note that even if the request goes through, we'd still realize the
// prefetch is unnecessary and drop it eventually, but this avoids a
// lot of extra work to get to that point. The downside is we might we
// miss an opportunity to prefetch a unit that changed shortly after
// the last attempt, but this should be uncommon.

tbb::concurrent_hash_map<
  const StringData*,
  std::chrono::steady_clock::time_point,
  StaticStringCompare
> s_prefetchTimestamps;

folly::CPUThreadPoolExecutor& getPrefetchExecutor() {
  assertx(!RO::RepoAuthoritative);
  assertx(unitPrefetchingEnabled());

  // Executor compatible thread factory which sets up the HPHP thread
  // state.
  struct PrefetcherThreadFactory : folly::NamedThreadFactory {
    using folly::NamedThreadFactory::NamedThreadFactory;

    std::thread newThread(folly::Func&& func) override {
      return folly::NamedThreadFactory::newThread(
        [func = std::move(func)] () mutable {
          hphp_thread_init();
          g_context.getCheck();
          SCOPE_EXIT { hphp_thread_exit(); };
          try {
            func();
          } catch (const std::exception& exn) {
            Logger::FError("Unit prefetching thread threw: {}", exn.what());
          } catch (...) {
            Logger::Error("Unit prefetching thread threw unknown exception");
          }
        }
      );
    }
  };

  // This will maintain a thread pool containing between
  // EvalUnitPrefetcherMinThreads and EvalUnitPrefetcherMaxThreads
  // threads. New threads are spun up to the max as long as there's
  // available work. Idle threads for longer than
  // EvalUnitPrefetcherIdleThreadTimeoutSecs will be reaped. The work
  // queue is unbounded, so it will always accept new work.
  struct PrefetcherExecutor : folly::CPUThreadPoolExecutor {
    PrefetcherExecutor()
      : folly::CPUThreadPoolExecutor(
          {RO::EvalUnitPrefetcherMaxThreads,
           std::min(RO::EvalUnitPrefetcherMinThreads,
                    RO::EvalUnitPrefetcherMaxThreads)},
          std::make_unique<
            folly::PriorityUnboundedBlockingQueue<
              folly::CPUThreadPoolExecutor::CPUTask
            >
          >(3),
          std::make_shared<PrefetcherThreadFactory>("UnitPrefetchPool")
        )
    {
      setThreadDeathTimeout(
        std::chrono::seconds{RO::EvalUnitPrefetcherIdleThreadTimeoutSecs}
      );
    }
  };
  static PrefetcherExecutor e;
  return e;
}

// Given a set of symbols, attempt to prefetch any units which are
// known to define those symbols (determined by the autoloader). You
// can optionally provide any Unit which is currently being loaded,
// which will ignore any symbols defined in that unit.
void prefetchSymbolRefs(SymbolRefs symbols, const Unit* loadingUnit) {
  if (!unitPrefetchingEnabled()) return;
  assertx(!RO::RepoAuthoritative);

  // If there's no autoloader associated with this request, we can't
  // resolve the symbols, so there's nothing to do.
  if (!AutoloadHandler::s_instance->getAutoloadMap()) return;

  tracing::BlockNoTrace _{"prefetch-symbol-refs"};

  // Map all the symbols into paths from the autoloader. Note that
  // these paths may not be canonical. prefetchUnit() will deal with
  // that. The paths are static so we can use pointer equality.
  hphp_fast_set<StringData*> paths;

  auto const resolve = [&]
    (auto const& names, AutoloadMap::KindOf k) {
    for (auto const& name : names) {
      // Lookup the path in the maps that the autoloader
      // provides. Note that this won't succeed if the autoloader
      // defines the symbol via its "failure" function.
      if (auto const path =
          AutoloadHandler::s_instance->getFile(StrNR{name}, k)) {
        paths.insert(makeStaticString(*path));
      }
    }
  };

  for (auto const& sym : symbols) {
    switch (sym.first) {
      case SymbolRef::Class:
        resolve(sym.second, AutoloadMap::KindOf::Type);
        break;
      case SymbolRef::Function:
        resolve(sym.second, AutoloadMap::KindOf::Function);
        break;
      case SymbolRef::Constant:
        resolve(sym.second, AutoloadMap::KindOf::Constant);
        break;
      case SymbolRef::Include:
        break;
    }
  }

  for (auto const& p : paths) prefetchUnit(p, nullptr, loadingUnit);
}

//////////////////////////////////////////////////////////////////////

CachedUnit loadUnitNonRepoAuth(StringData* requestedPath,
                               const struct stat* statInfo,
                               OptLog& ent,
                               const Native::FuncTable& nativeFuncs,
                               const RepoOptions& options,
                               FileLoadFlags& flags,
                               bool alreadyRealpath) {
  tracing::BlockNoTrace _{"load-unit-non-repo-auth"};

  LogTimer loadTime("load_ms", ent);

  rqtrace::EventGuard trace{"WRITE_UNIT"};

  auto const [path, rpath] =
    resolveRequestedPath(requestedPath, alreadyRealpath);

  auto& cache = getNonRepoCache(rpath);

  // Freeing a unit while holding the tbb lock would cause a rank violation when
  // recycle-tc is enabled as reclaiming dead functions requires that the code
  // and metadata locks be acquired.
  Unit* releaseUnit = nullptr;
  SCOPE_EXIT { if (releaseUnit) delete releaseUnit; };

  auto const updateAndUnlock = [] (auto& cachedUnit, CachedUnitWithFreePtr p) {
    // The ptr is unchanged... nothing to do
    if (cachedUnit.copy() == p) {
      cachedUnit.unlock();
      return;
    }
    // Otherwise update the entry. Defer the destruction of the
    // old copy_ptr using the Treadmill. Other threads may be
    // reading the entry simultaneously so the ref-count cannot
    // drop to zero here.
    if (auto old = cachedUnit.update_and_unlock(std::move(p))) {
      // We don't need to do anything explicitly; the copy_ptr
      // destructor will take care of it.
      Treadmill::enqueue([o = std::move(old)] {});
    }
  };

  // The C++ standard has a defect when it comes to capturing
  // destructured names into lambdas. Work around it with a capture
  // expression.
  auto ptr = [&, rpath = rpath] {
    NonRepoUnitCache::const_accessor rpathAcc;

    cache.insert(rpathAcc, rpath);
    auto& cachedUnit = rpathAcc->second.cachedUnit;
    if (auto const tmp = cachedUnit.copy()) {
      if (!isChanged(tmp, statInfo, options)) {
        flags = FileLoadFlags::kHitMem;
        if (ent) ent->setStr("type", "cache_hit_readlock");
        return tmp;
      }
    }

    if (!cachedUnit.try_lock_for_update()) {
      tracing::BlockNoTrace _{"unit-cache-lock-acquire"};
      cachedUnit.lock_for_update();
    }

    try {
      if (auto const tmp = cachedUnit.copy()) {
        if (!isChanged(tmp, statInfo, options)) {
          cachedUnit.unlock();
          flags = FileLoadFlags::kWaited;
          if (ent) ent->setStr("type", "cache_hit_writelock");
          return tmp;
        }
        if (ent) ent->setStr("type", "cache_stale");
      } else {
        if (ent) ent->setStr("type", "cache_miss");
      }

      trace.finish();
      auto const ptr = [&] {
        auto orig = RuntimeOption::EvalCheckUnitSHA1
          ? cachedUnit.copy()
          : CachedUnitWithFreePtr{};
        return createUnitFromFile(rpath, &releaseUnit, ent,
                                  nativeFuncs, options, flags,
                                  statInfo, std::move(orig));
      }();

      // Don't cache the unit if it was created in response to an
      // internal error in ExternCompiler. Such units represent
      // transient events.
      if (UNLIKELY(!ptr || !ptr->cu.unit || ptr->cu.unit->isICE())) {
        cachedUnit.unlock();
        return ptr;
      }
      updateAndUnlock(cachedUnit, ptr);
      return ptr;
    } catch (...) {
      cachedUnit.unlock();
      throw;
    }
  }();

  // If we haven't stored the ptr in the cache (for example, if its an
  // ICE), the copy_ptr dtor will automatically treadmill the Unit
  // after the request ends.
  if (UNLIKELY(!ptr)) return CachedUnit{};
  if (path == rpath) return ptr->cu;
  if (UNLIKELY(!ptr->cu.unit || ptr->cu.unit->isICE())) return ptr->cu;

  auto const cu = ptr->cu;

  NonRepoUnitCache::const_accessor pathAcc;
  cache.insert(pathAcc, path);
  auto& cachedUnit = pathAcc->second.cachedUnit;
  if (auto const tmp = cachedUnit.copy(); tmp != ptr) {
    cachedUnit.lock_for_update();
    updateAndUnlock(cachedUnit, std::move(ptr));
  }

  return cu;
}

CachedUnit lookupUnitNonRepoAuth(StringData* requestedPath,
                                 const struct stat* statInfo,
                                 OptLog& ent,
                                 const Native::FuncTable& nativeFuncs,
                                 FileLoadFlags& flags,
                                 bool alreadyRealpath) {
  tracing::BlockNoTrace _{"lookup-unit-non-repo-auth"};

  auto const& options = RepoOptions::forFile(requestedPath->data());

  if (!g_context.isNull() && strncmp(requestedPath->data(), "/:", 2)) {
    g_context->onLoadWithOptions(requestedPath->data(), options);
  }

  auto cu = [&] {
    {
      // Steady state, its probably already in the cache. Try that first
      rqtrace::EventGuard trace{"READ_UNIT"};
      NonRepoUnitCache::const_accessor acc;
      if (s_nonRepoUnitCache.find(acc, requestedPath)) {
        auto const cachedUnit = acc->second.cachedUnit.copy();
        if (!isChanged(cachedUnit, statInfo, options)) {
          auto const cu = cachedUnit->cu;
          if (!cu.unit || !RuntimeOption::CheckSymLink || alreadyRealpath ||
              !strcmp(StatCache::realpath(requestedPath->data()).c_str(),
                      cu.unit->filepath()->data())) {
            if (ent) ent->setStr("type", "cache_hit_readlock");
            flags = FileLoadFlags::kHitMem;
            return cu;
          }
        }
      }
    }
    // Not in the cache, attempt to load it
    return loadUnitNonRepoAuth(requestedPath, statInfo, ent, nativeFuncs,
                               options, flags, alreadyRealpath);
  }();

  if (cu.unit) {
    // Check if this unit has any symbol refs. If so, atomically claim
    // them and attempt to prefetch units using the symbols. Only one
    // thread will claim the refs, so this will only be done once per
    // unit.
    if (auto symbols = cu.unit->claimSymbolRefsForPrefetch()) {
      prefetchSymbolRefs(std::move(*symbols), cu.unit);
    }
  }

  return cu;
}

//////////////////////////////////////////////////////////////////////
// resolveVmInclude callbacks

struct ResolveIncludeContext {
  struct stat* s; // stat for the file
  bool allow_dir; // return true for dirs?
  const Native::FuncTable& nativeFuncs;
  String path;    // translated path of the file
};

bool findFile(const StringData* path, struct stat* s, bool allow_dir,
              const Native::FuncTable& nativeFuncs) {
  // We rely on this side-effect in RepoAuthoritative mode right now, since the
  // stat information is an output-param of resolveVmInclude, but we aren't
  // really going to call stat.
  s->st_mode = 0;

  if (RuntimeOption::RepoAuthoritative) {
    return lookupUnitRepoAuth(path, nativeFuncs).unit != nullptr;
  }

  if (StatCache::stat(path->data(), s) == 0) {
    // The call explicitly populates the struct for dirs, but returns false for
    // them because it is geared toward file includes.
    return allow_dir || !S_ISDIR(s->st_mode);
  }

  return false;
}

bool findFileWrapper(const String& file, void* ctx) {
  auto const context = static_cast<ResolveIncludeContext*>(ctx);
  assertx(context->path.isNull());

  // TranslatePath() will canonicalize the path and also check
  // whether the file is in an allowed directory.
  String translatedPath = File::TranslatePathKeepRelative(file);
  if (!FileUtil::isAbsolutePath(file.toCppString())) {
    if (findFile(translatedPath.get(), context->s, context->allow_dir,
                 context->nativeFuncs)) {
      context->path = translatedPath;
      return true;
    }
    return false;
  }
  if (RuntimeOption::SandboxMode || !RuntimeOption::AlwaysUseRelativePath) {
    if (findFile(translatedPath.get(), context->s, context->allow_dir,
                 context->nativeFuncs)) {
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
  if (findFile(rel_path.get(), context->s, context->allow_dir,
               context->nativeFuncs)) {
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
    if (auto const info = u->getFatalInfo()) {
      auto const parse = info->m_fatalOp == FatalOp::Parse;
      ent.setStr("result", parse ? "parse_fatal" : "compile_fatal");
      ent.setStr("error", info->m_fatalMsg);
    } else {
      ent.setStr("result", "success");
    }
    ent.setStr("sha1", u->sha1().toString());
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

  switch (rl_typeProfileLocals->requestKind) {
  case RequestKind::Warmup: ent.setStr("request_kind", "warmup"); break;
  case RequestKind::Standard: ent.setStr("request_kind", "standard"); break;
  case RequestKind::NonVM: ent.setStr("request_kind", "nonVM"); break;
  }
  ent.setInt("request_count", requestCount());

  StructuredLog::log("hhvm_unit_cache", ent);
}

//////////////////////////////////////////////////////////////////////

CachedUnit checkoutFile(
  StringData* path,
  const struct stat& statInfo,
  OptLog& ent,
  const Native::FuncTable& nativeFuncs,
  FileLoadFlags& flags,
  bool alreadyRealpath
) {
  return RuntimeOption::RepoAuthoritative
    ? lookupUnitRepoAuth(path, nativeFuncs)
    : lookupUnitNonRepoAuth(path, &statInfo, ent, nativeFuncs, flags, alreadyRealpath);
}

const std::string mangleUnitPHP7Options() {
  // As the list of options increases, we may want to do something smarter here?
  std::string s;
  s += (RuntimeOption::PHP7_NoHexNumerics ? '1' : '0') +
      (RuntimeOption::PHP7_Builtins ? '1' : '0') +
      (RuntimeOption::PHP7_Substr ? '1' : '0');
  return s;
}

char mangleExtension(const folly::StringPiece fileName) {
  if (fileName.endsWith(".hack")) return '0';
  if (fileName.endsWith(".hackpartial")) return '1';
  if (fileName.endsWith(".php")) return '2';
  if (fileName.endsWith(".hhas")) return '3';
  return '4'; // other files
}

//////////////////////////////////////////////////////////////////////

} // end empty namespace

//////////////////////////////////////////////////////////////////////

std::string mangleUnitSha1(const std::string& fileSha1,
                           const folly::StringPiece fileName,
                           const RepoOptions& opts) {
  std::string t = fileSha1 + '\0'
    + repoSchemaId().toString()
    + (RuntimeOption::EnableClassLevelWhereClauses ? '1' : '0')
    + (RuntimeOption::AssertEmitted ? '1' : '0')
    + (RuntimeOption::EvalGenerateDocComments ? '1' : '0')
    + (RuntimeOption::EnableXHP ? '1' : '0')
    + (RuntimeOption::EvalEnableCallBuiltin ? '1' : '0')
    + (RuntimeOption::EvalHackArrCompatNotices ? '1' : '0')
    + (RuntimeOption::EvalHackArrCompatIsVecDictNotices ? '1' : '0')
    + (RuntimeOption::EvalHackArrCompatSerializeNotices ? '1' : '0')
    + (RuntimeOption::EvalHackCompilerUseEmbedded ? '1' : '0')
    + (RuntimeOption::EvalHackCompilerVerboseErrors ? '1' : '0')
    + (RuntimeOption::EvalJitEnableRenameFunction ? '1' : '0')
    + (RuntimeOption::EvalLoadFilepathFromUnitCache ? '1' : '0')
    + std::to_string(RuntimeOption::EvalForbidDynamicCallsToFunc)
    + std::to_string(RuntimeOption::EvalForbidDynamicCallsToClsMeth)
    + std::to_string(RuntimeOption::EvalForbidDynamicCallsToInstMeth)
    + std::to_string(RuntimeOption::EvalForbidDynamicConstructs)
    + (RuntimeOption::EvalForbidDynamicCallsWithAttr ? '1' : '0')
    + (RuntimeOption::EvalLogKnownMethodsAsDynamicCalls ? '1' : '0')
    + (RuntimeOption::EvalNoticeOnBuiltinDynamicCalls ? '1' : '0')
    + (RuntimeOption::EvalHackArrDVArrs ? '1' : '0')
    + (RuntimeOption::EvalHackArrDVArrMark ? '1' : '0')
    + (RuntimeOption::EvalAssemblerFoldDefaultValues ? '1' : '0')
    + RuntimeOption::EvalHackCompilerCommand + '\0'
    + RuntimeOption::EvalHackCompilerArgs + '\0'
    + (needs_extended_line_table() ? '1' : '0')
    + std::to_string(RuntimeOption::CheckIntOverflow)
    + (RuntimeOption::DisableNontoplevelDeclarations ? '1' : '0')
    + (RuntimeOption::DisableStaticClosures ? '1' : '0')
    + (RuntimeOption::EvalRxIsEnabled ? '1' : '0')
    + (RuntimeOption::EvalEmitClsMethPointers ? '1' : '0')
    + (RuntimeOption::EvalIsVecNotices ? '1' : '0')
    + (RuntimeOption::EvalIsCompatibleClsMethType ? '1' : '0')
    + (RuntimeOption::EvalHackRecords ? '1' : '0')
    + (RuntimeOption::EvalArrayProvenance ? '1' : '0')
    + (RuntimeOption::EnableFirstClassFunctionPointers ? '1' : '0')
    + (RuntimeOption::EvalAllowHhas ? '1' : '0')
    + std::to_string(RuntimeOption::EvalEnforceGenericsUB)
    + (RuntimeOption::EvalEmitMethCallerFuncPointers ? '1' : '0')
    + std::to_string(RuntimeOption::EvalAssemblerMaxScalarSize)
    + std::to_string(RuntimeOption::EvalEmitClassPointers)
    + opts.cacheKeyRaw()
    + mangleExtension(fileName)
    + mangleUnitPHP7Options()
    + hackc_version();
  return string_sha1(t);
}

size_t numLoadedUnits() {
  if (RuntimeOption::RepoAuthoritative) return s_repoUnitCache.size();
  auto count = s_nonRepoUnitCache.size();
  for (auto const& p : s_perUserUnitCaches) count += p.second->size();
  return count;
}

Unit* getLoadedUnit(StringData* path) {
  if (!RuntimeOption::RepoAuthoritative) {
    NonRepoUnitCache::const_accessor accessor;
    if (s_nonRepoUnitCache.find(accessor, path) ) {
      auto cachedUnit = accessor->second.cachedUnit.copy();
      return cachedUnit ? cachedUnit->cu.unit : nullptr;
    }
  }
  return nullptr;
}

std::vector<Unit*> loadedUnitsRepoAuth() {
  always_assert(RuntimeOption::RepoAuthoritative);
  std::vector<Unit*> units;
  units.reserve(s_repoUnitCache.size());
  for (auto const& elm : s_repoUnitCache) {
    if (auto const unit = elm.second.unit.copy()) {
      if (unit != CachedUnitInternal::Uninit) {
        units.push_back(unit);
      }
    }
  }
  return units;
}

void invalidateUnit(StringData* path) {
  always_assert(!RuntimeOption::RepoAuthoritative);

  path = makeStaticString(path);

  auto const erase = [&] (NonRepoUnitCache& cache) {
    NonRepoUnitCache::accessor acc;
    if (cache.find(acc, path)) {
      // We can't just erase the entry here... there's a race where
      // another thread could have copied the copy_ptr out of the map,
      // but not yet inc-ref'd it. If we just erase the entry, we
      // could dec-ref it (and free it) before the other thread has a
      // chance to inc-ref it. We need to defer the dec-ref using the
      // treadmill. Manually move the copy_ptr onto the treadmill,
      // replace it with a null copy_ptr, and then erase the entry.
      auto& cached = acc->second.cachedUnit;
      cached.lock_for_update();
      if (auto old = cached.update_and_unlock({})) {
        Treadmill::enqueue([o = std::move(old)] {});
      }
      cache.erase(acc);
    }
  };
  erase(s_nonRepoUnitCache);
  for (auto const& p : s_perUserUnitCaches) erase(*p.second);

  Repo::get().forgetUnit(path->data());
}

String resolveVmInclude(const StringData* path,
                        const char* currentDir,
                        struct stat* s,
                        const Native::FuncTable& nativeFuncs,
                        bool allow_dir /* = false */) {
  ResolveIncludeContext ctx{s, allow_dir, nativeFuncs};
  resolve_include(StrNR{path}, currentDir, findFileWrapper, &ctx);
  // If resolve_include() could not find the file, return NULL
  return ctx.path;
}

Unit* lookupUnit(StringData* path, const char* currentDir, bool* initial_opt,
                 const Native::FuncTable& nativeFuncs, bool alreadyRealpath) {
  bool init;
  bool& initial = initial_opt ? *initial_opt : init;
  initial = true;

  tracing::BlockNoTrace _{"lookup-unit"};

  OptLog ent;
  if (!RuntimeOption::RepoAuthoritative &&
      StructuredLog::coinflip(RuntimeOption::EvalLogUnitLoadRate)) {
    ent.emplace();
  }

  rqtrace::ScopeGuard trace{"LOOKUP_UNIT"};
  trace.annotate("path", path->data());
  trace.annotate("pwd", currentDir);

  LogTimer lookupTimer("lookup_ms", ent);

  /*
   * NB: the m_evaledFiles map is only for the debugger, and could be omitted
   * in RepoAuthoritative mode, but currently isn't.
   */

  struct stat s;
  auto const spath = resolveVmInclude(path, currentDir, &s, nativeFuncs);
  if (spath.isNull()) return nullptr;

  auto const eContext = g_context.getNoCheck();

  // Check if this file has already been included.
  auto it = eContext->m_evaledFiles.find(spath.get());
  if (it != eContext->m_evaledFiles.end()) {
    // In RepoAuthoritative mode we assume that the files are unchanged.
    initial = false;
    if (RuntimeOption::RepoAuthoritative ||
        (it->second.ts_sec > s.st_mtime) ||
        ((it->second.ts_sec == s.st_mtime) &&
         (it->second.ts_nsec >= s.st_mtim.tv_nsec))) {
      return it->second.unit;
    }
  }

  FileLoadFlags flags = FileLoadFlags::kHitMem;

  // This file hasn't been included yet, so we need to parse the file
  auto const cunit = checkoutFile(spath.get(), s, ent, nativeFuncs, flags, alreadyRealpath);
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
      {cunit.unit, s.st_mtime, static_cast<unsigned long>(s.st_mtim.tv_nsec),
       flags};
    spath.get()->incRefCount();
    if (!cunit.unit->filepath()->same(spath.get())) {
      eContext->m_evaledFiles[cunit.unit->filepath()] =
        {cunit.unit, s.st_mtime, static_cast<unsigned long>(s.st_mtim.tv_nsec),
         FileLoadFlags::kDup};
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

Unit* lookupSyslibUnit(StringData* path, const Native::FuncTable& nativeFuncs) {
  if (RuntimeOption::RepoAuthoritative) {
    return lookupUnitRepoAuth(path, nativeFuncs).unit;
  }
  OptLog ent;
  FileLoadFlags flags;
  return lookupUnitNonRepoAuth(path, nullptr, ent, nativeFuncs, flags, true).unit;
}

//////////////////////////////////////////////////////////////////////

void clearUnitCacheForExit() {
  s_nonRepoUnitCache.clear();
  s_repoUnitCache.clear();
  s_perUserUnitCaches.clear();
}

void shutdownUnitPrefetcher() {
  if (RO::RepoAuthoritative || !unitPrefetchingEnabled()) return;
  getPrefetchExecutor().join();
}

//////////////////////////////////////////////////////////////////////

void prefetchUnit(StringData* requestedPath,
                  std::shared_ptr<folly::atomic_uint_fast_wait_t> gate,
                  const Unit* loadingUnit) {
  assertx(!RO::RepoAuthoritative);
  assertx(unitPrefetchingEnabled());
  assertx(requestedPath->isStatic());
  assertx(!loadingUnit || loadingUnit->filepath()->isStatic());

  // If the requested path is trivially identical to a Unit being
  // loaded (without normalization), we can just skip this.
  if (loadingUnit && requestedPath == loadingUnit->filepath()) return;

  tracing::BlockNoTrace _{"prefetch-unit"};

  // Otherwise check if we've prefetched this path already
  // lately. This is just an optimization.
  auto const prefetchedAlready = [&] {
    // Assume that if we have a gate, this is an explicit request
    // for prefetching (not done from the Unit loader), so skip this
    // optimization.
    if (gate) return false;

    // Grab the timestamp for the last prefetch from the map. If none
    // exists, we'll atomically insert it. In that case, the path has
    // never been prefetched.
    auto const now = std::chrono::steady_clock::now();
    decltype(s_prefetchTimestamps)::accessor acc;
    if (s_prefetchTimestamps.insert(acc, {requestedPath, now})) {
      return false;
    }
    // The path has been prefetched before. We need to check if the
    // last prefetch was more than 15 seconds in the past. If so,
    // we'll try again (and update the timestamp). Otherwise, we'll
    // forgo this attempt. The 15 second constant was chosen somewhat
    // arbitrarily.
    if (now >= acc->second + std::chrono::seconds{15}) {
      acc->second = now;
      return false;
    }
    return true;
  }();
  if (prefetchedAlready) return;

  // Perform all the work that needs request context. The worker
  // threads aren't a request, so this must be done here before it
  // gets queued into the worker pool:

  // Normally we need to run resolveVmInclude() in a request context,
  // as it might access per-request state. However this is relatively
  // expensive. If the below criteria are true, then
  // resolveVmInclude() will not require any request state and it can
  // be deferred to the worker thread.
  auto const deferResolveVmInclude =
    FileUtil::isAbsolutePath(requestedPath->slice()) &&
    !RID().hasSafeFileAccess() &&
    (RuntimeOption::SandboxMode || !RuntimeOption::AlwaysUseRelativePath);

  folly::Optional<struct stat> fileStat;
  const StringData* path = nullptr;
  if (!deferResolveVmInclude) {
    // We can't safely defer resolveVmInclude(). Do it now.
    fileStat.emplace();
    auto const spath = resolveVmInclude(
      requestedPath,
      "",
      fileStat.get_pointer(),
      Native::s_noNativeFuncs
    );
    // File doesn't exist. Nothing to do.
    if (spath.isNull()) return;

    // Do the second round of path normalization. Resolving symlinks
    // is relatively expensive, but always can be deferred until the
    // work thread. Lie and say realpath has already been done to get
    // only the path canonicalization without symlink resolution.
    auto const [p1, p2] = resolveRequestedPath(spath.get(), true);
    assertx(p1 == p2);
    assertx(p1->isStatic());
    path = p1;
  } else {
    // Keep the path as is. We'll do all the normalization in the
    // worker thread.
    path = requestedPath;
  }

  // Now that the paths might be normalized, compare them again
  // against any loading unit, to see if we can short-circuit.
  if (loadingUnit && path == loadingUnit->filepath()) return;

  // Looking up any per-use caches requires the canonicalized path,
  // but might not have it yet. Grab any cli uid now so we can do the
  // lookup in the worker thread.
  folly::Optional<uid_t> uid;
  if (auto const uc = get_cli_ucred()) uid = uc->uid;

  // We're definitely going to enqueue this request. Bump the gate if
  // provided.
  if (gate) gate->fetch_add(1);

  tracing::BlockNoTrace _2{"prefetch-unit-enqueue"};

  // The rest of the work can be done in the worker thread. Enqueue
  // it.
  getPrefetchExecutor().addWithPriority(
    // NB: This lambda is executed at some later point in another
    // thread, so you need to be careful about the lifetime of what
    // you capture here.
    [path, fileStat, uid,
     loadingUnitPath = loadingUnit ? loadingUnit->filepath() : nullptr,
     gate = std::move(gate)] () mutable {
      SCOPE_EXIT {
        // Decrement the gate whenever we're done. If the gate hits
        // zero, do a notification to wake up any thread waiting on
        // it.
        if (!gate) return;
        auto const count = gate->fetch_sub(1) - 1;
        if (count == 0) folly::atomic_notify_one(gate.get());
      };

      // We cannot delete Units at all points in the loading path (due
      // to potential lock rank violations). If necessary, we defer
      // the deletion until when we exit this function.
      Unit* releaseUnit = nullptr;
      SCOPE_EXIT { if (releaseUnit) delete releaseUnit; };

      // Atomically update cachedUnit with the provided value, release
      // the lock on cachedUnit, and take the appropriate action to
      // delete the old Unit.
      auto const updateAndUnlock = [] (auto& cachedUnit,
                                       CachedUnitWithFreePtr p) {
        // The ptr is unchanged... nothing to do
        if (cachedUnit.copy() == p) {
          cachedUnit.unlock();
          return;
        }
        // Otherwise update the entry. Defer the destruction of the
        // old copy_ptr using the Treadmill. Other threads may be
        // reading the entry simultaneously so the ref-count cannot
        // drop to zero here.
        if (auto old = cachedUnit.update_and_unlock(std::move(p))) {
          // We don't need to do anything explicitly; the copy_ptr
          // destructor will take care of it.
          Treadmill::enqueue([o = std::move(old)] {});
        }
      };

      // If we deferred resolveVmInclude(), do it now.
      if (!fileStat) {
        fileStat.emplace();
        auto const spath = resolveVmInclude(
          path,
          "",
          fileStat.get_pointer(),
          Native::s_noNativeFuncs
        );
        // File doesn't exist. Nothing to do.
        if (spath.isNull()) return;

        // We don't need resolveRequestedPath() here as the conditions
        // for deferring resolveVmInclude() mean it would be a nop.
        assertx(FileUtil::isAbsolutePath(spath.get()->slice()));
        path = makeStaticString(spath.get());
      }

      // Now do any required symlink resolution:
      auto const [sameAsPath, rpath] = resolveRequestedPath(path, false);
      assertx(sameAsPath == path);
      assertx(rpath->isStatic());

      // Now that the paths are fully normalized, compare them against
      // any loading unit path, to see if we can return without doing
      // anything.
      if (rpath == loadingUnitPath) return;

      auto const& options = RepoOptions::forFile(path->data());

      // Lookup any per-use cache with the stored uid (if any).
      auto& cache = uid ? getNonRepoCache(*uid, rpath) : s_nonRepoUnitCache;

      auto ptr = [&, rpath = rpath] () -> CachedUnitWithFreePtr {
        NonRepoUnitCache::const_accessor rpathAcc;

        cache.insert(rpathAcc, rpath);
        auto& cachedUnit = rpathAcc->second.cachedUnit;

        // NB: It might be tempting to check if the file has changed
        // before acquiring the lock. This opens up a race where the
        // Unit can be deleted out from under us. Once we acquire the
        // lock, no other thread can delete the Unit (until we release
        // it). This isn't an issue for the request threads because
        // the Unit is freed by the treadmill (so cannot go away
        // during the request). This thread is *not* a request thread
        // and therefore the treadmill can't save us.

        // Try to acquire the update lock for this unit. Don't
        // block. If we fail to acquire the lock, its because another
        // prefetcher thread, or a request thread is currently loading
        // the unit. In either case, we don't want to do anything with
        // it, and just move onto another request.
        if (!cachedUnit.try_lock_for_update()) return {};

        // If we throw, release the lock. Successful paths will
        // manually release the lock, as they may want to update the
        // value simultaneously.
        SCOPE_FAIL { cachedUnit.unlock(); };

        // Now that we have the lock, check if the path has a Unit
        // already, and if so, has the file has changed since that
        // Unit was created. If not, there's nothing to do.
        if (auto const tmp = cachedUnit.copy()) {
          if (!isChanged(tmp, fileStat.get_pointer(), options)) {
            cachedUnit.unlock();
            return {};
          }
        }

        // The Unit doesn't already exist, or the file has
        // changed. Either way, we need to create a new Unit.
        auto ptr = [&] {
          auto orig = RuntimeOption::EvalCheckUnitSHA1
            ? cachedUnit.copy()
            : CachedUnitWithFreePtr{};
          FileLoadFlags flags;
          OptLog optLog;
          return createUnitFromFile(rpath, &releaseUnit, optLog,
                                    Native::s_noNativeFuncs,
                                    options, flags,
                                    fileStat.get_pointer(), std::move(orig));
        }();

        // We don't want to prefetch ICE units (they can be
        // transient), so if we encounter one, just drop it and leave
        // the cached entry as is.
        if (!ptr || !ptr->cu.unit || ptr->cu.unit->isICE()) {
          cachedUnit.unlock();
          return {};
        }

        // The new Unit is good. Atomically update the cache entry
        // with it while releasing the lock.
        updateAndUnlock(cachedUnit, ptr);
        return ptr;
      }();

      // If the provided path is different than the canonical path,
      // then also cache the Unit via the provided path. This is a
      // shortcut to allow us to lookup the Unit in the cache before
      // doing path canonicalization.
      if (!ptr || path == rpath) return;

      NonRepoUnitCache::const_accessor pathAcc;
      cache.insert(pathAcc, path);
      auto& cachedUnit = pathAcc->second.cachedUnit;
      if (cachedUnit.try_lock_for_update()) {
        updateAndUnlock(cachedUnit, std::move(ptr));
      }
    },
    // Use high priority for prefetch requests. Medium priority is
    // used for drain blocks. Low priority is used internally by the
    // executor to drain the queue during shutdown.
    folly::Executor::HI_PRI
  );
}

void drainUnitPrefetcher() {
  // Enqueue a medium priority task which simply posts the
  // baton. Since prefetch requests are always enqueued with high
  // priority, this task will not run until there's no queued prefetch
  // requests (the executor always processes available higher priority
  // tasks before lower priority ones).
  folly::Baton baton;
  getPrefetchExecutor().addWithPriority(
    [&baton] { baton.post(); },
    folly::Executor::MID_PRI
  );
  baton.wait();
}

//////////////////////////////////////////////////////////////////////

namespace {

// Evaled units have a footprint in the TC and translation
// metadata. The applications we care about tend to have few, short,
// stereotyped evals, where the same code keeps getting eval'ed over
// and over again; so we keep around units for each eval'ed string, so
// that the TC space isn't wasted on each eval.

using EvaledUnitsMap = RankedCHM<
  const StringData*, // Must be static
  Unit*,
  StringDataHashCompare,
  RankEvaledUnits
>;

static EvaledUnitsMap s_evaledUnits;

}

Unit* compileEvalString(const StringData* code, const char* evalFilename) {
  auto const scode = makeStaticString(code);
  EvaledUnitsMap::accessor acc;
  if (s_evaledUnits.insert(acc, scode)) {
    acc->second = compile_string(
      scode->data(),
      scode->size(),
      evalFilename,
      Native::s_noNativeFuncs,
      g_context->getRepoOptionsForCurrentFrame()
    );
  }
  return acc->second;
}

//////////////////////////////////////////////////////////////////////

namespace {

ServiceData::CounterCallback s_counters(
  [](std::map<std::string, int64_t>& counters) {
    counters["vm.path-unit-cache-size"] = numLoadedUnits();
    counters["vm.eval-unit-cache-size"] = s_evaledUnits.size();
    counters["vm.live-units"] = Unit::liveUnitCount();
  }
);

}

//////////////////////////////////////////////////////////////////////

}
