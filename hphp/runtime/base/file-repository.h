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
#include <cstdlib>

#include "folly/Optional.h"

#include "hphp/util/md5.h"

#ifdef __APPLE__
#define st_mtim st_mtimespec
#endif

namespace HPHP {

struct Unit;
struct String;
struct StringData;

//////////////////////////////////////////////////////////////////////

struct PhpFile {
  PhpFile(const std::string& fileName,
          const std::string& srcRoot,
          const std::string& relPath,
          const std::string& md5,
          Unit* unit);
  ~PhpFile();

  PhpFile(const PhpFile&) = delete;
  PhpFile& operator=(const PhpFile&) = delete;

  const std::string& getFileName() const { return m_fileName; }
  const std::string& getSrcRoot() const { return m_srcRoot; }
  const std::string& getRelPath() const { return m_relPath; }
  const std::string& getMd5() const { return m_md5; }
  int getRef() const { return m_refCount.load(std::memory_order_acquire); }
  Unit* unit() const { return m_unit; }
  int getId() const { return m_id; }

private:
  friend struct FileRepository;
  void incRef();
  int decRef();
  void decRefAndDelete();
  void setId(int id);

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

//////////////////////////////////////////////////////////////////////

/*
 * FileRepository tracks all the Units that are currently live in this
 * hhvm process.
 *
 * Currently live means that its been loaded at least once, and we haven't
 * noticed a change to the underlying file yet.
 */
struct FileRepository {
  static PhpFile* checkoutFile(StringData* rname, const struct stat& s);
  static folly::Optional<MD5> readRepoMd5(const StringData* path);
  static std::string unitMd5(const std::string& fileMd5);
  static size_t getLoadedFiles();
};

/*
 * Resolve an include path, for the supplied path and directory, using
 * the same rules as PHP's fopen() or include.  May return a null
 * String if the path would not be includable.  File stat information
 * is returned in `s'.
 *
 * If `allow_dir' is true, this resolves the path even if it is naming
 * a directory.  Otherwise for directories a null String is returned.
 *
 * Note: it's unclear what's "vm" about this, and why it's not just
 * resolve_include.  (Likely naming relic from hphpc days.)
 */
String resolveVmInclude(StringData* path,
                        const char* currentDir,
                        struct stat* s,  // out
                        bool allow_dir = false);

//////////////////////////////////////////////////////////////////////

}

#endif
