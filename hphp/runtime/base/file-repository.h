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

#ifndef incl_HPHP_EVAL_FILE_REPOSITORY_H_
#define incl_HPHP_EVAL_FILE_REPOSITORY_H_

#include <time.h>
#include <sys/stat.h>
#include "hphp/util/lock.h"

#include "hphp/runtime/base/runtime-option.h"
#include "hphp/util/md5.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/string-util.h"

#ifdef __APPLE__
#define st_mtim st_mtimespec
#endif

namespace HPHP {
class Unit;

/* UnitVisitor: abstract interface for running code on each Unit. */
class UnitVisitor {
public:
  virtual ~UnitVisitor() { }
  virtual void operator()(Unit *u) = 0;
};
}

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

static inline bool md5Enabled() {
  return true;
}

static inline bool isAuthoritativeRepo() {
  return RuntimeOption::RepoAuthoritative;
}

class PhpFile {
public:
  PhpFile(const std::string &fileName, const std::string &srcRoot,
          const std::string &relPath, const std::string &md5,
          HPHP::Unit* unit);
  ~PhpFile();
  void incRef();
  int decRef(int num = 1);
  void decRefAndDelete();
  int getRef() { return m_refCount.load(std::memory_order_acquire); }
  // time_t readTime() const { return m_timestamp; }
  const std::string &getFileName() const { return m_fileName; }
  const std::string &getSrcRoot() const { return m_srcRoot; }
  const std::string &getRelPath() const { return m_relPath; }
  const std::string &getMd5() const { return m_md5; }
  HPHP::Unit* unit() const { return m_unit; }
  int getId() const { return m_id; }
  void setId(int id);

private:
  std::atomic<int> m_refCount;
  unsigned m_id;
  std::string m_profName;
  std::string m_fileName;
  std::string m_srcRoot;
  std::string m_relPath;
  std::string m_md5;
  HPHP::Unit* m_unit;
};

class PhpFileWrapper {
  static int64_t timespecCompare(const struct timespec& l,
                               const struct timespec& r) {
    if (l.tv_sec != r.tv_sec) return l.tv_sec - r.tv_sec;
    int64_t ret = l.tv_nsec - r.tv_nsec;
    return ret;
  }
public:
  PhpFileWrapper(const struct stat &s, PhpFile *phpFile) :
    m_mtime(s.st_mtim), m_ino(s.st_ino), m_devId(s.st_dev),
    m_phpFile(phpFile) {
  }
  ~PhpFileWrapper() {}
  bool isChanged(const struct stat &s) {
    if (isAuthoritativeRepo()) {
      return false;
    }
    return timespecCompare(m_mtime, s.st_mtim) < 0 ||
           m_ino != s.st_ino ||
           m_devId != s.st_dev;
  }
  PhpFile *getPhpFile() { return m_phpFile; }

private:
  struct timespec m_mtime;
  ino_t m_ino;
  dev_t m_devId;
  PhpFile *m_phpFile;
};

struct UnitMd5Val {
  MD5 m_unitMd5;
  bool m_present;
};

typedef tbb::concurrent_hash_map<const StringData *, struct UnitMd5Val,
                                 StringDataHashCompare> UnitMd5Map;
typedef RankedCHM<const StringData*, HPHP::Eval::PhpFileWrapper*,
                  StringDataHashCompare, RankFileRepo> ParsedFilesMap;
typedef hphp_hash_map<std::string, PhpFile*, string_hash> Md5FileMap;

/**
 * FileRepository tracks all the Units that are currently live in this session.
 *
 * Currently live means that its been loaded at least once, and we haven't
 * noticed a change to the underlying file yet.
 *
 * FileRepository contains a chm from file-paths to PhpFileWrappers.
 * The PhpFileWrapper is owned by the chm entry, and refers to a refCounted
 * PhpFile (the refCount is the number of times it appears in the
 * FileRepository in total - symlinks can cause a PhpFile to appear more than
 * once). The PhpFile refers to, and owns a Unit.
 *
 * When the last reference to a PhpFile goes away, the Unit and its contents
 * can no longer be reached by new requests, but existing requests could still
 * be referring to it, so the Unit is freed via TreadMill.
 */
class FileRepository {
public:
  class FileInfo {
  public:
    FileInfo() : m_phpFile(nullptr) {}
    PhpFile *m_phpFile;
    String m_inputString;
    std::string m_md5;
    std::string m_unitMd5;
    std::string m_srcRoot;
    std::string m_relPath;
  };

  /**
   * The first time you attempt to invoke a file in a request, this is called.
   * From then on, invoke_file will store the PhpFile and use that.
   */
  static PhpFile *checkoutFile(StringData *rname, const struct stat &s);
  static bool findFile(const StringData *path, struct stat *s);
  static bool fileDump(const char *filename);
  static std::string unitMd5(const std::string& fileMd5);
  static void setFileInfo(const StringData *name, const std::string& md5,
                          FileInfo &fileInfo, bool fromRepo = false);
  static bool readActualFile(const StringData *name, const struct stat &s,
                             FileInfo &fileInfo);
  static void computeMd5(const StringData *name, FileInfo& fileInfo);
  static bool readRepoMd5(const StringData *path, FileInfo& fileInfo);
  static bool readFile(const StringData *name,
                       const struct stat &s, FileInfo &fileInfo);
  static PhpFile *readHhbc(const std::string &name, const FileInfo &fileInfo);
  static PhpFile *parseFile(const std::string &name, const FileInfo &fileInfo);
  static String translateFileName(StringData *file);
  static void onDelete(PhpFile *f);
  static void forEachUnit(UnitVisitor& uit);
  static size_t getLoadedFiles();
private:
  static ParsedFilesMap s_files;
  static UnitMd5Map s_unitMd5Map;
  static ReadWriteMutex s_md5Lock;
  static Md5FileMap s_md5Files;

  static bool fileStat(const std::string &name, struct stat *s);
  static std::set<std::string> s_names;
};

String resolveVmInclude(StringData* path, const char* currentDir,
                        struct stat *s);

///////////////////////////////////////////////////////////////////////////////
}
}

#endif /* incl_HPHP_EVAL_FILE_REPOSITORY_H_ */
