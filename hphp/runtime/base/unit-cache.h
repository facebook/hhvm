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

#include "hphp/runtime/base/autoload-map.h"
#include "hphp/runtime/base/stream-wrapper.h"

#include "hphp/runtime/ext/facts/path-and-hash.h"
#include "hphp/util/sha1.h"

#include <filesystem>
#include <string>
#include <vector>

#include <folly/String.h>
#include <folly/synchronization/AtomicNotification.h>

struct stat;

namespace HPHP {

struct Unit;
struct String;
struct StringData;
struct RepoOptions;
struct RepoUnitInfo;
struct AutoloadMap;

namespace Native {
struct FuncTable;
}

//////////////////////////////////////////////////////////////////////

/*
 * Try to get a Unit* for a php file, given a path and directory.  The actual
 * path to try to find the file at is located using these arguments,
 * resolveVmInclude, and possibly realpath calls.
 *
 * In RepoAuthoritative mode, this will only find Units that were compiled into
 * the repo ahead of time.  Otherwise, this function may invoke the compiler to
 * create a new Unit for a file.  Units are cached across requests in either
 * case, so even in non-RepoAuthoritative mode this function won't need to
 * compile it over and over, but it does need to check whether the file has
 * changed.
 *
 * The `initial_opt' argument performs two functions, if non-null:
 *
 *    o It is an output param indicating whether whether this was the first
 *      time this unit was found in this request.  (For example, if you are
 *      doing an include_once you need to know whether it was already
 *      included.)
 *
 *    o It means we should count this lookup as a request-local lookup for the
 *      current request.
 *
 * May return nullptr if the Unit can't be loaded, and may throw exceptions or
 * fatal errors.
 */
Unit* lookupUnit(const StringData* path, const char* currentDir,
                 bool* initial_opt, const Extension* extension,
                 bool alreadyRealpath, bool forPrefetch = false,
                 bool forAutoload = false);

Unit* lookupUnit(const StringData* path, const RepoUnitInfo* info,
                 const char* currentDir, bool* initial_opt,
                 const Extension* extension, bool alreadyRealpath,
                 bool forPrefetch = false, bool forAutoload = false);

/*
 * As above, but for system units. Only appropriate in
 * RepoAuthoritative mode, as we do not cache system units otherwise.
 */
Unit* lookupSyslibUnit(StringData* path);

/*
 * Mangle a file's sha1sum with runtime options that affect the Unit output.
 * The parser and this module need to agree on how this is done.
 */

#define UNITCACHEFLAGS()                        \
  R(EnableClassLevelWhereClauses)               \
  R(EvalGenerateDocComments)                    \
  R(EnableXHP)                                  \
  R(EvalEnableCallBuiltin)                      \
  R(EvalHackArrCompatSerializeNotices)          \
  R(EvalJitEnableRenameFunction)                \
  R(EvalLoadFilepathFromUnitCache)              \
  R(EvalForbidDynamicCallsToFunc)               \
  R(EvalForbidDynamicCallsToClsMeth)            \
  R(EvalForbidDynamicCallsToInstMeth)           \
  R(EvalForbidDynamicConstructs)                \
  R(EvalForbidDynamicCallsWithAttr)             \
  R(EvalLogKnownMethodsAsDynamicCalls)          \
  R(EvalNoticeOnBuiltinDynamicCalls)            \
  R(EvalAssemblerFoldDefaultValues)             \
  R(RepoDebugInfo)                              \
  R(EvalEmitClsMethPointers)                    \
  R(EvalIsVecNotices)                           \
  R(EvalAllowHhas)                              \
  R(EvalEnforceGenericsUB)                      \
  R(EvalEmitMethCallerFuncPointers)             \
  R(EvalAssemblerMaxScalarSize)                 \
  R(EvalFoldLazyClassKeys)                      \
  R(EvalEmitNativeEnumClassLabels)              \
  R(EvalEnableAbstractContextConstants)         \
  R(EvalTraitConstantInterfaceBehavior)         \
  R(EvalUnitCacheBreaker)                       \
  R(EvalDiamondTraitMethods)                    \
  R(EvalClassPassesClassname)                   \
  R(PHP7_NoHexNumerics)                         \
  R(PHP7_Builtins)                              \
  R(PHP7_Substr)                                \
  R(EvalEnableDecl)                             \
  /* This is used by HackC to turn on / off     \
   * magic decl driven bytecode functions like  \
   * `HH\embed_type_decl`  */                   \
  R(EnableIntrinsicsExtension)                  \
  R(EvalModuleLevelTraits)                      \
  R(EvalTreatCaseTypesAsMixed)                  \

std::string mangleUnitSha1(const folly::StringPiece fileSha1,
                           const folly::StringPiece fileName,
                           const RepoOptionsFlags&);

Optional<SHA1> getHashForFile(const std::string& path,
                              const std::filesystem::path& root);

/*
 * Return the number of php files that are currently loaded in this process.
 * Exported for the admin request handler.
 */
size_t numLoadedUnits();

/*
 * Return a std::vector of all the units currently loaded. Must be
 * called from a single threaded context (wrt other unit-cache functions).
 *
 * Precondition: RepoAuthoritative
 */
std::vector<Unit*> loadedUnitsRepoAuth();

/*
 * Resolve an include path, for the supplied path and directory, using the same
 * rules as PHP's fopen() or include.  May return a null String if the path
 * would not be includable.  File stat information is returned in `s'.
 *
 * If `allow_dir' is true, this resolves the path even if it is naming a
 * directory.  Otherwise for directories a null String is returned.
 *
 * Note: it's unclear what's "vm" about this, and why it's not just
 * resolve_include.  (Likely naming relic from hphpc days.)
 */
String resolveVmInclude(const StringData* path,
                        const char* currentDir,
                        struct stat* s,  // out
                        bool allow_dir = false);

/*
 * Remove the specified unit from the cache, to force HHVM to
 * recompile the file.
 */
void invalidateUnit(StringData* path);

/*
 * Needed to avoid order of destruction issues. Destroying the unit
 * caches destroys the units, which destroys the classes, which tries
 * to grab global mutexes, which can fail if the mutexes have already
 * been destroyed.
 */
void clearUnitCacheForExit();

/*
 * Stop any unit prefetcher threads. This needs to be done before
 * clearUnitCacheForExit().
 */
void shutdownUnitPrefetcher();

/*
 * Shutdown the Unit reaper. This needs to be done before
 * clearUnitCacheForExit().
 */
void shutdownUnitReaper();

/*
 * Returns a unit if it's already loaded. If not then this returns nullptr.
 * Currently only works in !RepoAuthoritative mode.
 */
Unit* getLoadedUnit(StringData* path);

/*
 * Attempt to (asynchronously) prefetch a Unit given by the given
 * path. Unit prefetching must be enabled, and RepoAuthoritative mode
 * must not be active. This request is "best effort", it may do
 * nothing, the Unit load can fail, or the prefetch can be arbitrarily
 * delayed.
 *
 * `path' is a path where the Unit can be found. It does not have to
 * be canonicalized. It must be a static string.
 *
 * If `gate' is non-nullptr, it will be incremented for every enqueued
 * prefetch request, and decremented for each finished request. This
 * can be used to fire off a number of requests, then block until they
 * complete (by waiting for the gate to reach 0). If the gate is
 * decremented to zero, an atomic_notify() will be issued on it to
 * wake up any waiters.
 *
 * If `loadingUnit' is provided, then any paths which resolve to the
 * canonical path of that Unit will be skipped. This avoids attempting
 * to prefetch a Unit which you're in the process of loading already.
 */
void prefetchUnit(StringData* path,
                  std::shared_ptr<folly::atomic_uint_fast_wait_t> gate,
                  const Unit* loadingUnit);

/*
 * As an optimization requests may specify a list of changed files via
 * unitCacheSyncRepo() to be marked as dirty. When this is done these files
 * will be reloaded in requests that set unitCacheSetSync().
 *
 * Requests should use unitCacheSetSync() to indicate that files loaded via
 * the autoloader can be trusted without resolving their paths or checking for
 * modifications via stat so long as their dirty bits are not set.
 *
 * The unitCacheClearSync() must be called to clear the thread local trust bit
 * set in the unit cache.
 */
void unitCacheSetSync();
void unitCacheClearSync();
void unitCacheSyncRepo(AutoloadMap* map,
                       std::filesystem::path& root,
                       std::vector<Facts::PathAndOptionalHash>& changed,
                       std::vector<std::filesystem::path>& deleted);

/*
 * Block until all outstanding Unit prefetch attempts finish. Note
 * that if a new attempt is queued while we are blocked here, we will
 * wait for that to finish as well. This means this can block
 * indefinitely if there's a steady stream of incoming prefetch
 * requests. Unit prefetching must be enabled, and RepoAuthoritative
 * mode must not be active.
 */
void drainUnitPrefetcher();

/*
 * Compile a string into an Unit for the purposes of eval
 * execution. Units may be cached.
 */
Unit* compileEvalString(const StringData* code,
                        const char* evalFilename = nullptr);

//////////////////////////////////////////////////////////////////////

/*
 * If the file system provides a way to query the file hash without
 * actually loading it, we can (potentially) avoid loading the file
 * entirely.
 *
 * This class wraps the potential lazy loading of a file's
 * contents. It might load the contents eagerly, or it might defer the
 * load until the contents are actually requested. If we never request
 * the contents (for example, if the emitter cache hook is
 * successful), we avoid the load.
 */
struct LazyUnitContentsLoader {
  LazyUnitContentsLoader(const char* path,
                         Stream::Wrapper* wrapper,
                         const RepoOptionsFlags& options,
                         std::filesystem::path repoRoot,
                         size_t fileLength,
                         bool forceEager);

  // When we have the contents already. The loader does not manage the
  // contents' lifetime and the caller must ensure it remains alive
  // for the lifetime of the loader.
  LazyUnitContentsLoader(SHA1 sha,
                         folly::StringPiece contents,
                         const RepoOptionsFlags& options,
                         std::filesystem::path repoRoot);

  LazyUnitContentsLoader(const LazyUnitContentsLoader&) = delete;
  LazyUnitContentsLoader(LazyUnitContentsLoader&&) = delete;
  LazyUnitContentsLoader& operator=(const LazyUnitContentsLoader&) = delete;
  LazyUnitContentsLoader& operator=(LazyUnitContentsLoader&&) = delete;

  const SHA1& sha1() const { return m_hash; }
  const RepoOptionsFlags& options() const { return m_options; }
  size_t fileLength() const { return m_file_length; }

  const std::filesystem::path& repoRoot() const { return m_repo; }

  // Did we actually perform the load?
  bool didLoad() const { return m_loaded; }

  // Return the contents of the file. If the contents are already
  // loaded, this just returns them. Otherwise it performs the I/O to
  // load the file.
  folly::StringPiece contents();

  // Some error happened during file I/O
  struct LoadError : public std::exception {};
  // Since the file contents are loaded lazily with the file's hash
  // calculated first, it's possible for the file to change after we
  // obtained the hash. When we actually load the file, we detect this
  // inconsistency and throw this. Catch it and retry the entire
  // operation.
  struct Inconsistency : public std::exception {};

  // Max number of times to attempt lazy loading. To avoid live-lock,
  // you should eventually give up and force eager loading (which
  // cannot have inconsistencies).
  static constexpr size_t kMaxLazyAttempts = 3;

private:
  void load();

  const char* m_path;
  Stream::Wrapper* m_wrapper;
  const RepoOptionsFlags& m_options;

  SHA1 m_hash;
  SHA1 m_file_hash;
  size_t m_file_length;

  std::filesystem::path m_repo;

  // Points to either m_contents (if loaded from file), or some
  // external string (if provided in ctor).
  folly::StringPiece m_contents_ptr;
  String m_contents;
  // Is m_contents_ptr valid?
  bool m_loaded;
};

//////////////////////////////////////////////////////////////////////

}
