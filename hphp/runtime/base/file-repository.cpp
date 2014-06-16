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
#include "hphp/runtime/base/file-repository.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include <atomic>
#include <ctime>
#include <cstdlib>

#include "folly/ScopeGuard.h"

#include "hphp/util/lock.h"
#include "hphp/util/assertions.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/util/process.h"
#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/base/stat-cache.h"
#include "hphp/runtime/base/stream-wrapper-registry.h"
#include "hphp/runtime/base/file-stream-wrapper.h"
#include "hphp/runtime/base/profile-dump.h"
#include "hphp/runtime/server/source-root-info.h"

#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/bytecode.h"
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

/*
 * In RepoAuthoritative mode, loaded units are never unloaded, we
 * don't support symlink chasing, you can't include urls, and files
 * are never changed, which makes the code here significantly simpler.
 * Because of this it pays to keep it separate from the other cases so
 * they don't need to be littered with RepoAuthoritative checks.
 */

using RepoUnitCache = tbb::concurrent_hash_map<
  const StringData*,     // must be static
  CachedUnit,
  StringDataHashCompare
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

struct PhpFile {
  PhpFile(const std::string& fileName,
          const std::string& relPath,
          const std::string& md5,
          Unit* unit);
  ~PhpFile();

  PhpFile(const PhpFile&) = delete;
  PhpFile& operator=(const PhpFile&) = delete;

  const std::string& getFileName() const { return m_fileName; }
  const std::string& getRelPath() const { return m_relPath; }
  const std::string& getMd5() const { return m_md5; }
  int getRef() const { return m_refCount.load(std::memory_order_acquire); }
  Unit* unit() const { return m_unit; }
  int getId() const { return m_id; }
  void incRef();
  int decRef();
  void decRefAndDelete();
  void setId(int id);

private:
  std::atomic<int> m_refCount;
  unsigned m_id;
  std::string m_fileName;
  std::string m_relPath;
  std::string m_md5;
  Unit* m_unit;
};

//////////////////////////////////////////////////////////////////////

int64_t timespecCompare(const struct timespec& l,
                        const struct timespec& r) {
  if (l.tv_sec != r.tv_sec) return l.tv_sec - r.tv_sec;
  return l.tv_nsec - r.tv_nsec;
}

struct PhpFileWrapper {
  PhpFileWrapper(const struct stat& s, PhpFile* phpFile)
    : m_mtime(s.st_mtim)
    , m_ino(s.st_ino)
    , m_devId(s.st_dev)
    , m_phpFile(phpFile)
  {}

  bool isChanged(const struct stat& s) const {
    return timespecCompare(m_mtime, s.st_mtim) < 0 ||
           m_ino != s.st_ino ||
           m_devId != s.st_dev;
  }

  PhpFile* getPhpFile() const { return m_phpFile; }

private:
  struct timespec m_mtime;
  ino_t m_ino;
  dev_t m_devId;
  PhpFile* m_phpFile;
};

struct FileInfo {
  PhpFile* m_phpFile{nullptr};
  String m_inputString;
  std::string m_md5;
  std::string m_unitMd5;
  std::string m_relPath;
};

//////////////////////////////////////////////////////////////////////

/*
 * This module maintains a concurrent hash map from file-paths to
 * PhpFileWrappers.  The PhpFileWrapper is owned by the chm entry, and
 * refers to a refCounted PhpFile (the refCount is the number of times
 * it appears in the FileRepository in total - symlinks can cause a
 * PhpFile to appear more than once). The PhpFile refers to, and owns
 * a Unit.
 *
 * When the last reference to a PhpFile goes away, the Unit and its contents
 * can no longer be reached by new requests, but existing requests could still
 * be referring to it, so the Unit is freed via TreadMill.
 */

using ParsedFilesMap = RankedCHM<
  const StringData*,
  PhpFileWrapper*,
  StringDataHashCompare,
  RankFileRepo
>;
ParsedFilesMap s_files;

ReadWriteMutex s_md5Lock;
hphp_hash_map<std::string,PhpFile*,string_hash> s_md5Files;

//////////////////////////////////////////////////////////////////////

PhpFile::PhpFile(const std::string& fileName,
                 const std::string& relPath,
                 const std::string& md5,
                 Unit* unit)
  : m_refCount(0)
  , m_id(0)
  , m_fileName(fileName)
  , m_relPath(relPath)
  , m_md5(md5)
  , m_unit(unit)
{}

PhpFile::~PhpFile() {
  always_assert(getRef() == 0);
  if (do_assert) {
    ReadLock lock(s_md5Lock);
    UNUSED auto const it = s_md5Files.find(getMd5());
    assert(it == end(s_md5Files) || it->second != this);
  }
  if (m_unit != nullptr) {
    // If we have or in the process of a collecting an hhprof dump than we need
    // to keep these units around as they might be needed for symbol resolution
    // when that dump is collected by pprof. We will treadmill these later as
    // part of post-collection cleanup.
    if (memory_profiling && RuntimeOption::HHProfServerEnabled &&
        ProfileController::isTracking()) {
      ProfileController::enqueueOrphanedUnit(m_unit);
    } else {
      delete m_unit;
    }
    m_unit = nullptr;
  }
}

void PhpFile::incRef() {
  m_refCount.fetch_add(1, std::memory_order_acq_rel);
}

int PhpFile::decRef() {
  int ret = m_refCount.fetch_sub(1, std::memory_order_acq_rel);
  assert(ret >= 1); // fetch_sub returns the old value
  return ret - 1;
}

void PhpFile::decRefAndDelete() {
  if (decRef() == 0) {
    {
      WriteLock lock(s_md5Lock);
      auto const it = s_md5Files.find(getMd5());
      if (it != end(s_md5Files) && it->second == this) {
        s_md5Files.erase(it);
      }
    }
    Treadmill::enqueue([this] { delete this; });
  }
}

void PhpFile::setId(int id) {
  m_id = id;
  m_unit->setCacheId(id);
}

//////////////////////////////////////////////////////////////////////

void setFileInfo(const StringData* name,
                 const std::string& md5,
                 FileInfo& fileInfo,
                 bool fromRepo) {
  // Incorporate the path into the md5 that is used as the key for file
  // repository lookups.  This assures that even if two PHP files have
  // identical content, separate units exist for them (so that
  // Unit::filepath() and Unit::dirpath() work correctly).
  std::string s = md5 + '\0' + name->data();
  fileInfo.m_md5 = string_md5(s.c_str(), s.size());

  if (fromRepo) {
    fileInfo.m_unitMd5 = md5;
  } else {
    fileInfo.m_unitMd5 = mangleUnitMd5(md5);
  }

  auto const srcRoot = SourceRootInfo::GetCurrentSourceRoot();
  int srcRootLen = srcRoot.size();
  if (srcRootLen) {
    if (!strncmp(name->data(), srcRoot.c_str(), srcRootLen)) {
      fileInfo.m_relPath = std::string(name->data() + srcRootLen);
    }
  }

  ReadLock lock(s_md5Lock);
  auto it = s_md5Files.find(fileInfo.m_md5);
  if (it != s_md5Files.end()) {
    PhpFile* f = it->second;
    if (f->getRef() != 0 &&
        !fileInfo.m_relPath.empty() &&
        fileInfo.m_relPath == f->getRelPath()) {
      assert(fileInfo.m_md5 == f->getMd5());
      fileInfo.m_phpFile = f;
    }
  }
}

PhpFile* parseFile(const std::string& name,
                   const FileInfo& fileInfo) {
  MD5 md5 = MD5(fileInfo.m_unitMd5.c_str());
  Unit* unit = compile_file(fileInfo.m_inputString.data(),
                            fileInfo.m_inputString.size(),
                            md5, name.c_str());
  always_assert(unit != nullptr &&
                "failed to produce a unit; possibly due to corrupt hhbc repo");

  return new PhpFile(name, fileInfo.m_relPath, fileInfo.m_md5, unit);
}

PhpFile* readHhbc(const std::string& name, const FileInfo& fileInfo) {
  auto const md5 = MD5(fileInfo.m_unitMd5.c_str());
  auto const u = Repo::get().loadUnit(name, md5);
  if (u != nullptr) {
    return new PhpFile(name, fileInfo.m_relPath, fileInfo.m_md5, u);
  }

  return nullptr;
}

void computeMd5(const StringData *name, FileInfo& fileInfo) {
  auto& input = fileInfo.m_inputString;
  std::string md5 = string_md5(input.data(), input.size());
  setFileInfo(name, md5, fileInfo, false);
}

bool readFile(const StringData* name,
              const struct stat& s,
              FileInfo& fileInfo) {
  if (s.st_size > StringData::MaxSize) {
    throw FatalErrorException(0, "file %s is too big", name->data());
  }
  int fileSize = s.st_size;
  int fd = open(name->data(), O_RDONLY);
  if (!fd) return false; // ignore file open exception
  String str = String(fileSize, ReserveString);
  char *input = str.bufferSlice().ptr;
  if (!input) return false;
  int nbytes = read(fd, input, fileSize);
  close(fd);
  str.setSize(fileSize);
  fileInfo.m_inputString = str;
  if (nbytes != fileSize) return false;

  computeMd5(name, fileInfo);
  return true;
}

PhpFile* checkoutFileNonRepo(StringData* rname, const struct stat& statInfo) {
  FileInfo fileInfo;
  PhpFile* ret = nullptr;
  String name(rname);
  bool isPlainFile = File::IsPlainFilePath(name);

  if (isPlainFile) {
    if (rname->data()[0] != '/') {
      name = String(SourceRootInfo::GetCurrentSourceRoot()) + name;
    }
    // Get the common fast path out of the way with as little locking
    // as possible: it's in the map and has not changed on disk
    ParsedFilesMap::const_accessor acc;
    if (s_files.find(acc, name.get()) && !acc->second->isChanged(statInfo)) {
      ret = acc->second->getPhpFile();
      return ret;
    }
  } else {
    auto const w = Stream::getWrapperFromURI(name);
    if (!w) return nullptr;
    File* f = w->open(name, "r", 0, null_variant);
    if (!f) return nullptr;
    StringBuffer sb;
    sb.read(f);
    fileInfo.m_inputString = sb.detach();
    computeMd5(name.get(), fileInfo);
  }

  auto const staticName = makeStaticString(name.get());

  PhpFile* toKill = nullptr;
  SCOPE_EXIT {
    // run this after acc is destroyed (and its lock released)
    if (toKill) toKill->decRefAndDelete();
  };
  ParsedFilesMap::accessor acc;
  bool isNew = s_files.insert(acc, staticName);
  auto const old = acc->second;
  SCOPE_EXIT {
    // run this just before acc is released
    if (old && old != acc->second) {
      if (old->getPhpFile() != acc->second->getPhpFile()) {
        toKill = old->getPhpFile();
      }
      delete old;
    }
    if (!acc->second) s_files.erase(acc);
  };

  assert(isNew || old); // We don't leave null entries around.
  bool isChanged = !isNew && old->isChanged(statInfo);

  if (isNew || isChanged) {
    if (isPlainFile && !readFile(staticName, statInfo, fileInfo)) {
      // File disappeared between stat and readFile.
      return nullptr;
    }
    ret = fileInfo.m_phpFile;
    if (isChanged && ret == old->getPhpFile()) {
      // The file changed but had the same contents.
      if (do_assert) {
        ReadLock lock(s_md5Lock);
        assert(s_md5Files.find(ret->getMd5())->second == ret);
      }
      return ret;
    }
  } else if (!isNew) {
    // Somebody might have loaded the file between when we dropped
    // our read lock and got the write lock
    ret = old->getPhpFile();
    return ret;
  }

  // If we get here the file was not in s_files or has changed on disk
  if (!ret) {
    // Try to read Unit from .hhbc repo.
    ret = readHhbc(staticName->data(), fileInfo);
  }
  if (!ret) {
    ret = parseFile(staticName->data(), fileInfo);
    if (!ret) return nullptr;
  }
  assert(ret != nullptr);

  if (isNew) {
    acc->second = new PhpFileWrapper(statInfo, ret);
    ret->incRef();
    ret->setId(RDS::allocBit());
  } else {
    PhpFile* f = old->getPhpFile();
    if (f != ret) {
      ret->setId(f->getId());
      ret->incRef();
    }
    acc->second = new PhpFileWrapper(statInfo, ret);
  }

  {
    WriteLock lock(s_md5Lock);
    assert(ret->getRef() != 0);
    s_md5Files[ret->getMd5()] = ret;
  }
  return ret;
}

//////////////////////////////////////////////////////////////////////

CachedUnit checkoutFile(StringData* path, const struct stat& statInfo) {
  if (RuntimeOption::RepoAuthoritative) return lookupUnitRepoAuth(path);

  if (auto const phpFile = checkoutFileNonRepo(path, statInfo)) {
    return CachedUnit { phpFile->unit(),
      static_cast<size_t>(phpFile->getId()) };
  }
  return CachedUnit{};
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

  ReadLock lock(s_md5Lock);
  return s_md5Files.size();
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
   * NB: the m_evaledFiles map is only for the debugger, and could be omitted in
   * RepoAuthoritative mode, but currently isn't.
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
    // Don't incRef efile; checkoutFile() already counted it.
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
