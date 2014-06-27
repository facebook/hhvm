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
#include <ctime>
#include <cstdlib>

#include "folly/ScopeGuard.h"

#include "hphp/util/rank.h"
#include "hphp/util/mutex.h"
#include "hphp/util/assertions.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/base/stat-cache.h"
#include "hphp/runtime/base/stream-wrapper-registry.h"
#include "hphp/runtime/base/file-stream-wrapper.h"
#include "hphp/runtime/base/profile-dump.h"
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
                            SourceRootInfo::GetCurrentSourceRoot(),
                            md5)) {
    return acc->second;
  }

  acc->second.unit = Repo::get().loadUnit(path->data(), md5);
  if (acc->second.unit) {
    acc->second.rdsBitId = RDS::allocBit();
  }
  return acc->second;
}

//////////////////////////////////////////////////////////////////////
// Non-repo mode unit caching

struct CachedUnitNonRepo {
  CachedUnit cachedUnit;
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

/*
 * When a unit is removed from the unit cache, we wait for a Treadmill round
 * before reclaiming it using this routine.
 *
 * If we have or are in the process of a collecting an hhprof dump then we need
 * to keep these units around even longer, as they might be needed for symbol
 * resolution when that dump is collected by pprof.  In this case, we just pass
 * ownership to the ProfileControler module.
 */
void reclaimUnit(const Unit* unit) {
  if (memory_profiling && RuntimeOption::HHProfServerEnabled &&
      ProfileController::isTracking()) {
    ProfileController::enqueueOrphanedUnit(unit);
    return;
  }
  delete unit;
}

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
  return cu.cachedUnit.unit == nullptr ||
         timespecCompare(cu.mtime, s.st_mtim) < 0 ||
         cu.ino != s.st_ino ||
         cu.devId != s.st_dev;
}

folly::Optional<String> readFileAsString(const StringData* path,
                                         off_t fileSize) {
  if (fileSize > StringData::MaxSize) {
    throw FatalErrorException(0, "file %s is too big", path->data());
  }

  auto const fd = open(path->data(), O_RDONLY);
  if (!fd) return folly::none;
  SCOPE_EXIT { close(fd); };

  String str(fileSize, ReserveString);
  auto const input = str.bufferSlice().ptr;
  auto const nbytes = read(fd, input, fileSize);
  str.setSize(fileSize);
  if (nbytes != fileSize) return folly::none;
  return str;
}

CachedUnit createUnitFromString(const char* path,
                                const String& contents) {
  auto const md5 = MD5 {
    mangleUnitMd5(string_md5(contents.data(), contents.size())).c_str()
  };
  // Try the repo; if it's not already there, invoke the compiler.
  if (auto const unit = Repo::get().loadUnit(path, md5)) {
    return CachedUnit { unit, RDS::allocBit() };
  }
  auto const unit = compile_file(contents.data(), contents.size(), md5, path);
  return CachedUnit { unit, RDS::allocBit() };
}

CachedUnit createUnitFromUrl(const StringData* const requestedPath) {
  auto const w = Stream::getWrapperFromURI(StrNR(requestedPath));
  if (!w) return CachedUnit{};
  auto const f = w->open(StrNR(requestedPath), "r", 0, null_variant);
  if (!f) return CachedUnit{};
  StringBuffer sb;
  sb.read(f);
  return createUnitFromString(requestedPath->data(), sb.detach());
}

CachedUnit createUnitFromFile(StringData* const path, off_t fileSize) {
  auto const contents = readFileAsString(path, fileSize);
  return contents ? createUnitFromString(path->data(), *contents)
                  : CachedUnit{};
}

CachedUnit lookupUnitNonRepoAuth(StringData* requestedPath,
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

  NonRepoUnitCache::accessor acc;
  if (!s_nonRepoUnitCache.insert(acc, path)) {
    if (!isChanged(acc->second, statInfo)) {
      return acc->second.cachedUnit;
    }
  }

  /*
   * NB: the new-unit creation path is here, and is done while holding the tbb
   * lock on s_nonRepoUnitCache.  This was originally done deliberately to
   * avoid wasting time in the compiler (during server startup, many requests
   * hit the same code initial paths that are shared, and would all be
   * compiling the same files).  It's not 100% clear if this is the best way to
   * handle that idea, though (tbb locks spin aggressively and are expected to
   * be low contention).
   */

  /*
   * Don't cache if createNewUnit returns an empty CachedUnit---we'll need to
   * try again anyway if someone tries to load this path, since it might exist
   * later.
   *
   * If there was a unit for this path already, we need to put it on the
   * Treadmill for eventual reclaimation.  We can't delete it immediately
   * because other requests may still be using it.
   */
  auto const cu = createUnitFromFile(path, statInfo.st_size);
  if (auto const oldUnit = acc->second.cachedUnit.unit) {
    Treadmill::enqueue([oldUnit] { reclaimUnit(oldUnit); });
  }
  acc->second.cachedUnit = cu;
  acc->second.mtime      = statInfo.st_mtim;
  acc->second.ino        = statInfo.st_ino;
  acc->second.devId      = statInfo.st_dev;
  return cu;
}

//////////////////////////////////////////////////////////////////////
// resolveVmInclude callbacks

const StaticString s_file_url("file://");

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
  if (file.substr(0, 7) == s_file_url) {
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
    + (RuntimeOption::IntsOverflowToInts ? '1' : '0');
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

  // We didn't find it, so try the realpath.
  auto const alreadyResolved =
    RuntimeOption::RepoAuthoritative ||
    (!RuntimeOption::CheckSymLink && (spath[0] == '/'));
  bool hasRealpath = false;
  String rpath;
  if (!alreadyResolved) {
    std::string rp = StatCache::realpath(spath.data());
    if (rp.size() != 0) {
      rpath = StringData::Make(rp.data(), rp.size(), CopyString);
      if (!rpath.same(spath)) {
        hasRealpath = true;
        it = eContext->m_evaledFiles.find(rpath.get());
        if (it != eContext->m_evaledFiles.end()) {
          // We found it! Update the mapping for spath and return the
          // unit.
          auto const unit = it->second;
          spath.get()->incRefCount();
          eContext->m_evaledFiles[spath.get()] = unit;
          initial = false;
          return unit;
        }
      }
    }
  }

  // This file hasn't been included yet, so we need to parse the file
  auto const cunit = checkoutFile(hasRealpath ? rpath.get() : spath.get(), s);
  if (cunit.unit && initial_opt) {
    // if initial_opt is not set, this shouldn't be recorded as a
    // per request fetch of the file.
    if (RDS::testAndSetBit(cunit.rdsBitId)) {
      initial = false;
    }
    // if parsing was successful, update the mappings for spath and
    // rpath (if it exists).
    eContext->m_evaledFilesOrder.push_back(cunit.unit->filepath());
    eContext->m_evaledFiles[spath.get()] = cunit.unit;
    spath.get()->incRefCount();
    if (hasRealpath) {
      eContext->m_evaledFiles[rpath.get()] = cunit.unit;
      rpath.get()->incRefCount();
    }
    DEBUGGER_ATTACHED_ONLY(phpDebuggerFileLoadHook(cunit.unit));
  }

  return cunit.unit;
}

//////////////////////////////////////////////////////////////////////

}
