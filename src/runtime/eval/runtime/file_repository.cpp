/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/eval/runtime/file_repository.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/zend/zend_string.h>
#include <util/process.h>
#include <util/atomic.h>
#include <util/trace.h>
#include <runtime/base/stat_cache.h>
#include <runtime/base/server/source_root_info.h>

#include <runtime/vm/translator/targetcache.h>
#include <runtime/vm/translator/translator-x64.h>
#include <runtime/vm/bytecode.h>
#include <runtime/vm/pendq.h>
#include <runtime/vm/repo.h>
#include <runtime/vm/runtime.h>

using std::endl;

namespace HPHP {

static const Trace::Module TRACEMOD = Trace::fr;
extern bool (*file_dump)(const char *filename);

namespace Eval {
///////////////////////////////////////////////////////////////////////////////

std::set<string> FileRepository::s_names;

static volatile bool s_interceptsEnabled = false;

PhpFile::PhpFile(const string &fileName, const string &srcRoot,
                 const string &relPath, const string &md5,
                 HPHP::VM::Unit* unit)
    : m_refCount(0), m_id(0),
      m_profName(string("run_init::") + string(fileName)),
      m_fileName(fileName), m_srcRoot(srcRoot), m_relPath(relPath), m_md5(md5),
      m_unit(unit) {
}

PhpFile::~PhpFile() {
  always_assert(m_refCount == 0);
  if (m_unit != NULL) {
    // Deleting a Unit can grab a low-ranked lock and we're probably
    // at a high rank right now
    VM::PendQ::defer(new VM::DeferredDeleter<VM::Unit>(m_unit));
    m_unit = NULL;
  }
}

void PhpFile::incRef() {
  UNUSED int ret = atomic_inc(m_refCount);
  TRACE(4, "PhpFile: %s incRef() %d -> %d %p called by %p\n",
        m_fileName.c_str(), ret - 1, ret, this, __builtin_return_address(0));
}

int PhpFile::decRef(int n) {
  int ret = atomic_add(m_refCount, -n);
  TRACE(4, "PhpFile: %s decRef() %d -> %d %p called by %p\n",
        m_fileName.c_str(), ret, ret - n, this, __builtin_return_address(0));
  ASSERT(ret >= n); // atomic_add returns the old value
  return ret - n;
}

void PhpFile::decRefAndDelete() {
  if (decRef() == 0) {
    FileRepository::onDelete(this);
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

void FileRepository::onDelete(PhpFile *f) {
  ASSERT(f->getRef() == 0);
  if (md5Enabled()) {
    WriteLock lock(s_md5Lock);
    s_md5Files.erase(f->getMd5());
  }
  delete f;
}

void FileRepository::forEachUnit(VM::UnitVisitor& uit) {
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
  PhpFile *ret = NULL;
  String name(rname);
  if (rname->data()[0] != '/') {
    name = String(SourceRootInfo::GetCurrentSourceRoot()) + name;
  }

  {
    // Get the common fast path out of the way with as little locking
    // as possible: it's in the map and has not changed on disk
    ParsedFilesMap::const_accessor acc;
    if (s_files.find(acc, name.get()) && !acc->second->isChanged(s)) {
      TRACE(1, "FR fast path hit %s\n", rname->data());
      ret = acc->second->getPhpFile();
      ret->incRef();
      return ret;
    }
  }

  TRACE(1, "FR fast path miss: %s\n", rname->data());
  bool interceptsEnabled = s_interceptsEnabled;
  const StringData *n = StringData::GetStaticString(name.get());
  ParsedFilesMap::accessor acc;
  bool isNew = s_files.insert(acc, n);
  ASSERT(isNew || acc->second); // We don't leave null entries around.
  bool isChanged = !isNew && acc->second->isChanged(s);

  if (isNew || isChanged) {
    if (!readFile(n, s, fileInfo)) {
      // Be sure to get rid of the new reference to it.
      s_files.erase(acc);
      TRACE(1, "File disappeared between stat and FR::readNewFile: %s\n",
            rname->data());
      return NULL;
    }
    ret = fileInfo.m_phpFile;
    if (isChanged && ret == acc->second->getPhpFile()) {
      // The file changed but had the same contents.
      if (debug && md5Enabled()) {
        ReadLock lock(s_md5Lock);
        ASSERT(s_md5Files.find(ret->getMd5())->second == ret);
      }
      ret->incRef();
      return ret;
    }
  } else if (!isNew) {
    // Somebody might have loaded the file between when we dropped
    // our read lock and got the write lock
    ret = acc->second->getPhpFile();
    ret->incRef();
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
    if (!ret) {
      s_files.erase(acc);
      return NULL;
    }
  }
  ASSERT(ret != NULL);

  if (isNew) {
    acc->second = new PhpFileWrapper(s, ret);
    ret->incRef();
    ret->setId(VM::Transl::TargetCache::allocBit());
  } else {
    PhpFile *f = acc->second->getPhpFile();
    if (f != ret) {
      ret->setId(f->getId());
      tx64->invalidateFile(f); // f has changed
    }
    f->decRefAndDelete();
    delete acc->second;
    acc->second = new PhpFileWrapper(s, ret);
    ret->incRef();
  }

  if (md5Enabled()) {
    WriteLock lock(s_md5Lock);
    // make sure intercepts are enabled for the functions within the
    // new units
    // Since we have the write lock on s_md5lock, s_interceptsEnabled
    // can't change, and we are serialized wrt enableIntercepts
    // (i.e., this will execute either before or after
    // enableIntercepts).
    if (interceptsEnabled != s_interceptsEnabled) {
      // intercepts were enabled since the time we created the unit
      ret->unit()->enableIntercepts();
    }
    s_md5Files[ret->getMd5()] = ret;
  }
  ret->incRef();
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
    const StringData* spath = StringData::GetStaticString(path);
    UnitMd5Map::accessor acc;
    if (s_unitMd5Map.insert(acc, spath)) {
      bool present = VM::Repo::get().findFile(
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
  char* md5str;
  int md5len;
  std::ostringstream opts;
  string t = fileMd5 + '\0'
    + (RuntimeOption::EnableHipHopSyntax ? '1' : '0')
    + (RuntimeOption::EnableEmitSwitch ? '1' : '0')
    + (RuntimeOption::EvalJitEnableRenameFunction ? '1' : '0');
  md5str = string_md5(t.c_str(), t.size(), false, md5len);
  string s = string(md5str, md5len);
  free(md5str);
  return s;
}

void FileRepository::setFileInfo(const StringData *name,
                                 const string& md5,
                                 FileInfo &fileInfo,
                                 bool fromRepo) {
  int md5len;
  char* md5str;
  // Incorporate the path into the md5 that is used as the key for file
  // repository lookups.  This assures that even if two PHP files have
  // identical content, separate units exist for them (so that
  // Unit::filepath() and Unit::dirpath() work correctly).
  string s = md5 + '\0' + name->data();
  md5str = string_md5(s.c_str(), s.size(), false, md5len);
  fileInfo.m_md5 = string(md5str, md5len);
  free(md5str);

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
      ASSERT(fileInfo.m_md5 == f->getMd5());
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
  if (!fileSize) return false;
  int fd = open(name->data(), O_RDONLY);
  if (!fd) return false; // ignore file open exception
  String str = String(fileSize, ReserveString);
  char *input = str.mutableSlice().ptr;
  if (!input) return false;
  int nbytes = read(fd, input, fileSize);
  close(fd);
  str.setSize(fileSize);
  fileInfo.m_inputString = str;
  if (nbytes != fileSize) return false;

  if (md5Enabled()) {
    string md5 = StringUtil::MD5(fileInfo.m_inputString).c_str();
    setFileInfo(name, md5, fileInfo, false);
  }
  return true;
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
    path = StringData::GetStaticString(path);
    if (s_unitMd5Map.insert(acc, path)) {
      if (!VM::Repo::get().findFile(path->data(),
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
  VM::Unit* u = VM::Repo::get().loadUnit(name, md5);
  if (u != NULL) {
    PhpFile *p = new PhpFile(name, fileInfo.m_srcRoot, fileInfo.m_relPath,
                             fileInfo.m_md5, u);
    return p;
  }

  return NULL;
}

PhpFile *FileRepository::parseFile(const std::string &name,
                                   const FileInfo &fileInfo) {
  MD5 md5 = MD5(fileInfo.m_unitMd5.c_str());
  VM::Unit* unit = VM::compile_file(fileInfo.m_inputString->data(),
                                    fileInfo.m_inputString->size(),
                                    md5, name.c_str());
  PhpFile *p = new PhpFile(name, fileInfo.m_srcRoot, fileInfo.m_relPath,
                           fileInfo.m_md5, unit);
  return p;
}

bool FileRepository::fileStat(const string &name, struct stat *s) {
  return StatCache::stat(name, s) == 0;
}

void FileRepository::enableIntercepts() {
  ReadLock lock(s_md5Lock);
  s_interceptsEnabled = true; // write protected by s_mutex in intercept

  for (hphp_hash_map<string, PhpFile*, string_hash>::const_iterator it =
       s_md5Files.begin(); it != s_md5Files.end(); it++) {
    it->second->unit()->enableIntercepts();
  }
}

struct ResolveIncludeContext {
  String path; // translated path of the file
  struct stat* s; // stat for the file
};

static bool findFileWrapper(CStrRef file, void* ctx) {
  ResolveIncludeContext* context = (ResolveIncludeContext*)ctx;
  ASSERT(context->path.isNull());
  // TranslatePath() will canonicalize the path and also check
  // whether the file is in an allowed directory.
  String translatedPath = File::TranslatePath(file, false, true);
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
