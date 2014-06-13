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

#ifndef incl_HPHP_FILE_REPOSITORY_H_
#define incl_HPHP_FILE_REPOSITORY_H_

#include <sys/stat.h>
#include <atomic>
#include <ctime>
#include <set>
#include <vector>

#include "hphp/util/lock.h"
#include "hphp/util/md5.h"

#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/type-string.h"

#ifdef __APPLE__
#define st_mtim st_mtimespec
#endif

namespace HPHP {

struct Unit;

//////////////////////////////////////////////////////////////////////

struct PhpFile {
  PhpFile(const std::string &fileName, const std::string &srcRoot,
          const std::string &relPath, const std::string &md5,
          Unit* unit);
  ~PhpFile();

  PhpFile(const PhpFile&) = delete;
  PhpFile& operator=(const PhpFile&) = delete;

  const std::string &getFileName() const { return m_fileName; }
  const std::string &getSrcRoot() const { return m_srcRoot; }
  const std::string &getRelPath() const { return m_relPath; }
  const std::string &getMd5() const { return m_md5; }
  Unit* unit() const { return m_unit; }
  int getId() const { return m_id; }
  void setId(int id);

private:
  friend struct FileRepository;
  int getRef() const { return m_refCount.load(std::memory_order_acquire); }
  void incRef();
  int decRef();
  void decRefAndDelete();

private:
  std::atomic<int> m_refCount;
  unsigned m_id;
  std::string m_profName;
  std::string m_fileName;
  std::string m_srcRoot;
  std::string m_relPath;
  std::string m_md5;
  Unit* m_unit;
};

struct PhpFileWrapper {
  PhpFileWrapper(const struct stat& s, PhpFile* phpFile) :
    m_mtime(s.st_mtim), m_ino(s.st_ino), m_devId(s.st_dev),
    m_phpFile(phpFile) {
  }
  ~PhpFileWrapper() {}

  bool isChanged(const struct stat &s) {
    if (RuntimeOption::RepoAuthoritative) {
      return false;
    }
    return timespecCompare(m_mtime, s.st_mtim) < 0 ||
           m_ino != s.st_ino ||
           m_devId != s.st_dev;
  }
  PhpFile* getPhpFile() { return m_phpFile; }

private:
  static int64_t timespecCompare(const struct timespec& l,
                               const struct timespec& r) {
    if (l.tv_sec != r.tv_sec) return l.tv_sec - r.tv_sec;
    int64_t ret = l.tv_nsec - r.tv_nsec;
    return ret;
  }

private:
  struct timespec m_mtime;
  ino_t m_ino;
  dev_t m_devId;
  PhpFile* m_phpFile;
};

struct UnitMd5Val {
  MD5 m_unitMd5;
  bool m_present;
};

typedef tbb::concurrent_hash_map<const StringData*, struct UnitMd5Val,
                                 StringDataHashCompare> UnitMd5Map;
typedef RankedCHM<const StringData*, PhpFileWrapper*,
                  StringDataHashCompare, RankFileRepo> ParsedFilesMap;
typedef hphp_hash_map<std::string, PhpFile*, string_hash> Md5FileMap;

/*
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
struct FileRepository {
  struct FileInfo {
    PhpFile* m_phpFile{nullptr};
    String m_inputString;
    std::string m_md5;
    std::string m_unitMd5;
    std::string m_srcRoot;
    std::string m_relPath;
  };

  static PhpFile* checkoutFile(StringData* rname, const struct stat& s);
  static void deleteOrphanedUnits();
  static bool readRepoMd5(const StringData *path, FileInfo& fileInfo);
  static std::string unitMd5(const std::string& fileMd5);
  static size_t getLoadedFiles();

public: // XXX: logically private
  static bool findFile(const StringData* path, struct stat* s);

private:
  friend struct PhpFile;

private:
  static void setFileInfo(const StringData *name, const std::string& md5,
                          FileInfo &fileInfo, bool fromRepo = false);
  static bool readActualFile(const StringData *name, const struct stat &s,
                             FileInfo &fileInfo);
  static void computeMd5(const StringData *name, FileInfo& fileInfo);
  static bool readFile(const StringData *name,
                       const struct stat &s, FileInfo &fileInfo);
  static PhpFile* readHhbc(const std::string &name, const FileInfo &fileInfo);
  static PhpFile* parseFile(const std::string &name, const FileInfo &fileInfo);
  static void enqueueOrphanedUnitForDeletion(Unit* u);

private:
  static ParsedFilesMap s_files;
  static UnitMd5Map s_unitMd5Map;
  static ReadWriteMutex s_md5Lock;
  static Md5FileMap s_md5Files;
  static std::vector<Unit*> s_orphanedUnitsToDelete;

  static bool fileStat(const std::string &name, struct stat *s);
  static std::set<std::string> s_names;
};

String resolveVmInclude(StringData* path, const char* currentDir,
                        struct stat *s, bool allow_dir = false);

//////////////////////////////////////////////////////////////////////

}

#endif
