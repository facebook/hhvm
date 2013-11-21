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

#include "hphp/runtime/base/file-repository.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/util/process.h"
#include "hphp/util/trace.h"
#include "hphp/runtime/base/stat-cache.h"
#include "hphp/runtime/base/stream-wrapper-registry.h"
#include "hphp/runtime/base/file-stream-wrapper.h"
#include "hphp/runtime/server/source-root-info.h"

#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/pendq.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/treadmill.h"

#include "folly/ScopeGuard.h"

using std::endl;

namespace HPHP {

TRACE_SET_MOD(fr);
extern bool (*file_dump)(const char *filename);

namespace Eval {
///////////////////////////////////////////////////////////////////////////////

std::set<string> FileRepository::s_names;

PhpFile::PhpFile(const string &fileName, const string &srcRoot,
                 const string &relPath, const string &md5,
                 HPHP::Unit* unit)
    : m_refCount(0), m_id(0),
      m_profName(string("run_init::") + string(fileName)),
      m_fileName(fileName), m_srcRoot(srcRoot), m_relPath(relPath), m_md5(md5),
      m_unit(unit) {
}

PhpFile::~PhpFile() {
  always_assert(getRef() == 0);
  if (!RuntimeOption::HHProfServerEnabled && m_unit != nullptr) {
    // Deleting a Unit can grab a low-ranked lock and we're probably
    // at a high rank right now
    PendQ::defer(new DeferredDeleter<Unit>(m_unit));
    m_unit = nullptr;
  }
}

void PhpFile::incRef() {
  UNUSED int ret = m_refCount.fetch_add(1, std::memory_order_acq_rel);
  TRACE(4, "PhpFile: %s incRef() %d -> %d %p called by %p\n",
        m_fileName.c_str(), ret - 1, ret, this, __builtin_return_address(0));
}

int PhpFile::decRef(int n) {
  int ret = m_refCount.fetch_sub(n, std::memory_order_acq_rel);
  TRACE(4, "PhpFile: %s decRef() %d -> %d %p called by %p\n",
        m_fileName.c_str(), ret, ret - n, this, __builtin_return_address(0));
  assert(ret >= n); // fetch_sub returns the old value
  return ret - n;
}

void PhpFile::decRefAndDelete() {
  class FileInvalidationTrigger : public Treadmill::WorkItem {
    Eval::PhpFile* m_f;
   public:
    FileInvalidationTrigger(Eval::PhpFile* f) : m_f(f) { }
    virtual void operator()() {
      FileRepository::onDelete(m_f);
    }
  };

  if (decRef() == 0) {
    Treadmill::WorkItem::enqueue(new FileInvalidationTrigger(this));
  }
}

void PhpFile::setId(int id) {
  m_id = id;
  m_unit->setCacheId(id);
}

ReadWriteMutex FileRepository::s_md5Lock(RankFileMd5);
ParsedFilesMap FileRepository::s_files;
Md5FileMap FileRepository::s_md5Files;
UnitMd5Map FileRepository::s_unitMd5Map;

static class FileDumpInitializer {
  public: FileDumpInitializer() {
    file_dump = FileRepository::fileDump;
  }
} s_fileDumpInitializer;

bool FileRepository::fileDump(const char *filename) {
  std::ofstream out(filename);
  if (out.fail()) return false;
  out << "s_files: " << s_files.size() << endl;
  for (ParsedFilesMap::const_iterator it =
       s_files.begin(); it != s_files.end(); it++) {
    out << it->first->data() << endl;
  }
  {
    ReadLock lock(s_md5Lock);
    out << "s_md5Files: " << s_md5Files.size() << endl;
    for (Md5FileMap::const_iterator it = s_md5Files.begin();
         it != s_md5Files.end(); it++) {
      out << it->second->getMd5().c_str() << " "
          << it->second->getFileName().c_str() << endl;
    }
  }
  out.close();
  return true;
}

void FileRepository::onDelete(PhpFile* f) {
  assert(f->getRef() == 0);
  if (md5Enabled()) {
    WriteLock lock(s_md5Lock);
    s_md5Files.erase(f->getMd5());
  }
  delete f;
}

void FileRepository::forEachUnit(UnitVisitor& uit) {
  ReadLock lock(s_md5Lock);
  for (Md5FileMap::const_iterator it = s_md5Files.begin();
       it != s_md5Files.end(); ++it) {
    uit(it->second->unit());
  }
}

size_t FileRepository::getLoadedFiles() {
  ReadLock lock(s_md5Lock);
  return s_md5Files.size();
}

PhpFile *FileRepository::checkoutFile(StringData *rname,
                                      const struct stat &s) {
  FileInfo fileInfo;
  PhpFile *ret = nullptr;
  String name(rname);
  bool isPlainFile = File::IsPlainFilePath(name);

  if (isPlainFile) {
    if (rname->data()[0] != '/') {
      name = String(SourceRootInfo::GetCurrentSourceRoot()) + name;
    }
    // Get the common fast path out of the way with as little locking
    // as possible: it's in the map and has not changed on disk
    ParsedFilesMap::const_accessor acc;
    if (s_files.find(acc, name.get()) && !acc->second->isChanged(s)) {
      TRACE(1, "FR fast path hit %s\n", rname->data());
      ret = acc->second->getPhpFile();
      return ret;
    }
  } else {
    // Do the read before we get the repo lock, since it will call into the
    // stream library and will needs its own locks.
    if (isAuthoritativeRepo()) {
      throw FatalErrorException(
        "including urls doesn't work in RepoAuthoritative mode"
      );
    }
    Stream::Wrapper* w = Stream::getWrapperFromURI(name);
    File* f = w->open(name, "r", 0, null_variant);
    if (!f) return nullptr;
    StringBuffer sb;
    sb.read(f);
    fileInfo.m_inputString = sb.detach();
    computeMd5(name.get(), fileInfo);
  }

  TRACE(1, "FR fast path miss: %s\n", rname->data());
  const StringData *n = makeStaticString(name.get());

  PhpFile* toKill = nullptr;
  SCOPE_EXIT {
    // run this after acc is destroyed (and its lock released)
    if (toKill) toKill->decRefAndDelete();
  };
  ParsedFilesMap::accessor acc;
  bool isNew = s_files.insert(acc, n);
  PhpFileWrapper* old = acc->second;
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
  bool isChanged = !isNew && old->isChanged(s);

  if (isNew || isChanged) {
    if (isPlainFile && !readFile(n, s, fileInfo)) {
      TRACE(1, "File disappeared between stat and FR::readNewFile: %s\n",
            rname->data());
      return nullptr;
    }
    ret = fileInfo.m_phpFile;
    if (isChanged && ret == old->getPhpFile()) {
      // The file changed but had the same contents.
      if (debug && md5Enabled()) {
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
    ret = readHhbc(n->data(), fileInfo);
  }
  if (!ret) {
    if (isAuthoritativeRepo()) {
      raise_error("Tried to parse %s in repo authoritative mode", n->data());
    }
    ret = parseFile(n->data(), fileInfo);
    if (!ret) return nullptr;
  }
  assert(ret != nullptr);

  if (isNew) {
    acc->second = new PhpFileWrapper(s, ret);
    ret->incRef();
    ret->setId(RDS::allocBit());
  } else {
    PhpFile *f = old->getPhpFile();
    if (f != ret) {
      ret->setId(f->getId());
      ret->incRef();
    }
    acc->second = new PhpFileWrapper(s, ret);
  }

  if (md5Enabled()) {
    WriteLock lock(s_md5Lock);
    s_md5Files[ret->getMd5()] = ret;
  }
  return ret;
}

bool FileRepository::findFile(const StringData *path, struct stat *s) {
  if (isAuthoritativeRepo()) {
    {
      UnitMd5Map::const_accessor acc;
      if (s_unitMd5Map.find(acc, path)) {
        return acc->second.m_present;
      }
    }
    MD5 md5;
    const StringData* spath = makeStaticString(path);
    UnitMd5Map::accessor acc;
    if (s_unitMd5Map.insert(acc, spath)) {
      bool present = Repo::get().findFile(
        path->data(), SourceRootInfo::GetCurrentSourceRoot(), md5);
      acc->second.m_present = present;
      acc->second.m_unitMd5 = md5;
    }
    return acc->second.m_present;
  }
  return fileStat(path->data(), s) && !S_ISDIR(s->st_mode);
}

String FileRepository::translateFileName(StringData *file) {
  ParsedFilesMap::const_accessor acc;
  if (!s_files.find(acc, file)) return file;
  string srcRoot(SourceRootInfo::GetCurrentSourceRoot());
  if (srcRoot.empty()) return file;
  PhpFile *f = acc->second->getPhpFile();
  const string &parsedSrcRoot = f->getSrcRoot();
  if (srcRoot == parsedSrcRoot) return file;
  int len = parsedSrcRoot.size();
  if (len > 0 && file->size() > len &&
      strncmp(file->data(), parsedSrcRoot.c_str(), len) == 0) {
    return srcRoot + (file->data() + len);
  }
  return file;
}

string FileRepository::unitMd5(const string& fileMd5) {
  // Incorporate relevant options into the unit md5 (there will be more)
  std::ostringstream opts;
  string t = fileMd5 + '\0'
    + (RuntimeOption::EnableHipHopSyntax ? '1' : '0')
    + (RuntimeOption::EnableEmitSwitch ? '1' : '0')
    + (RuntimeOption::EvalJitEnableRenameFunction ? '1' : '0')
    + (RuntimeOption::EvalAllowHhas ? '1' : '0');
  return string_md5(t.c_str(), t.size());
}

void FileRepository::setFileInfo(const StringData *name,
                                 const string& md5,
                                 FileInfo &fileInfo,
                                 bool fromRepo) {
  // Incorporate the path into the md5 that is used as the key for file
  // repository lookups.  This assures that even if two PHP files have
  // identical content, separate units exist for them (so that
  // Unit::filepath() and Unit::dirpath() work correctly).
  string s = md5 + '\0' + name->data();
  fileInfo.m_md5 = string_md5(s.c_str(), s.size());

  if (fromRepo) {
    fileInfo.m_unitMd5 = md5;
  } else {
    fileInfo.m_unitMd5 = unitMd5(md5);
  }

  fileInfo.m_srcRoot = SourceRootInfo::GetCurrentSourceRoot();
  int srcRootLen = fileInfo.m_srcRoot.size();
  if (srcRootLen) {
    if (!strncmp(name->data(), fileInfo.m_srcRoot.c_str(), srcRootLen)) {
      fileInfo.m_relPath = string(name->data() + srcRootLen);
    }
  }

  ReadLock lock(s_md5Lock);
  Md5FileMap::iterator it = s_md5Files.find(fileInfo.m_md5);
  if (it != s_md5Files.end()) {
    PhpFile *f = it->second;
    if (!fileInfo.m_relPath.empty() &&
        fileInfo.m_relPath == f->getRelPath()) {
      assert(fileInfo.m_md5 == f->getMd5());
      fileInfo.m_phpFile = f;
    }
  }
}

bool FileRepository::readActualFile(const StringData *name,
                                    const struct stat &s,
                                    FileInfo &fileInfo) {
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

void FileRepository::computeMd5(const StringData *name, FileInfo& fileInfo) {
  if (md5Enabled()) {
    auto& input = fileInfo.m_inputString;
    string md5 = string_md5(input.data(), input.size());
    setFileInfo(name, md5, fileInfo, false);
  }
}

bool FileRepository::readRepoMd5(const StringData *path,
                                 FileInfo& fileInfo) {
  MD5 md5;
  bool found;
  {
    UnitMd5Map::const_accessor acc;
    found = s_unitMd5Map.find(acc, path);
    if (found) {
      if (!acc->second.m_present) {
        return false;
      }
      md5 = acc->second.m_unitMd5;
      path = acc->first;
    }
  }
  if (!found) {
    UnitMd5Map::accessor acc;
    path = makeStaticString(path);
    if (s_unitMd5Map.insert(acc, path)) {
      if (!Repo::get().findFile(path->data(),
                                    SourceRootInfo::GetCurrentSourceRoot(),
                                    md5)) {
        acc->second.m_present = false;
        return false;
      }
      acc->second.m_present = true;
      acc->second.m_unitMd5 = md5;
    } else {
      md5 = acc->second.m_unitMd5;
      path = acc->first;
    }
  }
  setFileInfo(path, md5.toString(), fileInfo, true);
  return true;
}

bool FileRepository::readFile(const StringData *name,
                              const struct stat &s,
                              FileInfo &fileInfo) {
  if (!isAuthoritativeRepo()) {
    TRACE(1, "read initial \"%s\"\n", name->data());
    return readActualFile(name, s, fileInfo);
  }

  if (!readRepoMd5(name, fileInfo)) {
    return false;
  }
  return true;
}

PhpFile *FileRepository::readHhbc(const std::string &name,
                                  const FileInfo &fileInfo) {
  MD5 md5 = MD5(fileInfo.m_unitMd5.c_str());
  Unit* u = Repo::get().loadUnit(name, md5);
  if (u != nullptr) {
    PhpFile *p = new PhpFile(name, fileInfo.m_srcRoot, fileInfo.m_relPath,
                             fileInfo.m_md5, u);
    return p;
  }

  return nullptr;
}

PhpFile *FileRepository::parseFile(const std::string &name,
                                   const FileInfo &fileInfo) {
  MD5 md5 = MD5(fileInfo.m_unitMd5.c_str());
  Unit* unit = compile_file(fileInfo.m_inputString->data(),
                                fileInfo.m_inputString->size(),
                                md5, name.c_str());
  PhpFile *p = new PhpFile(name, fileInfo.m_srcRoot, fileInfo.m_relPath,
                           fileInfo.m_md5, unit);
  return p;
}

bool FileRepository::fileStat(const string &name, struct stat *s) {
  return StatCache::stat(name, s) == 0;
}

struct ResolveIncludeContext {
  String path; // translated path of the file
  struct stat* s; // stat for the file
};

const StaticString s_file_url("file://");

static bool findFileWrapper(const String& file, void* ctx) {
  ResolveIncludeContext* context = (ResolveIncludeContext*)ctx;
  assert(context->path.isNull());

  Stream::Wrapper* w = Stream::getWrapperFromURI(file);
  if (!dynamic_cast<FileStreamWrapper*>(w)) {
    if (w->stat(file, context->s) == 0) {
      context->path = file;
      return true;
    }
  }

  // handle file://
  if (file.substr(0, 7) == s_file_url) {
    return findFileWrapper(file.substr(7), ctx);
  }

  // TranslatePath() will canonicalize the path and also check
  // whether the file is in an allowed directory.
  String translatedPath = File::TranslatePathKeepRelative(file);
  if (file[0] != '/') {
    if (HPHP::Eval::FileRepository::findFile(translatedPath.get(),
                                             context->s)) {
      context->path = translatedPath;
      return true;
    }
    return false;
  }
  if (RuntimeOption::SandboxMode || !RuntimeOption::AlwaysUseRelativePath) {
    if (HPHP::Eval::FileRepository::findFile(translatedPath.get(),
                                             context->s)) {
      context->path = translatedPath;
      return true;
    }
  }
  string server_root(SourceRootInfo::GetCurrentSourceRoot());
  if (server_root.empty()) {
    server_root = string(g_vmContext->getCwd()->data());
    if (server_root.empty() || server_root[server_root.size() - 1] != '/') {
      server_root += "/";
    }
  }
  String rel_path(Util::relativePath(server_root, translatedPath.data()));
  if (HPHP::Eval::FileRepository::findFile(rel_path.get(),
                                           context->s)) {
    context->path = rel_path;
    return true;
  }
  return false;
}

String resolveVmInclude(StringData* path, const char* currentDir,
                        struct stat *s) {
  ResolveIncludeContext ctx;
  ctx.s = s;
  resolve_include(path, currentDir, findFileWrapper,
                  (void*)&ctx);
  // If resolve_include() could not find the file, return NULL
  return ctx.path;
}

///////////////////////////////////////////////////////////////////////////////
}
}
