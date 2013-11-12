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

#ifndef incl_HPHP_FILE_CACHE_H_
#define incl_HPHP_FILE_CACHE_H_

#include <memory>
#include <string>

#include "hphp/util/base.h"

namespace HPHP {

/**
 * Stores file contents in memory. Used by web server for faster static
 * content serving.
 */
DECLARE_BOOST_TYPES(FileCache);

class CacheManager;

class FileCache {
 public:
  static std::string SourceRoot;
  static bool UseNewCache;

 public:
  FileCache();
  ~FileCache();

  /**
   * Archiving data.
   */

  void write(const char *name);   // just the name
  void write(const char *name, const char *fullpath); // name + data

  void save(const char *filename);

  /**
   * Reading data.
   */
  void loadMmap(const char *filename);
  bool fileExists(const char *name, bool isRelative = true) const;
  bool dirExists(const char *name, bool isRelative = true) const;
  bool exists(const char *name, bool isRelative = true) const;
  char *read(const char *name, int &len, bool &compressed) const;
  int64_t fileSize(const char *name, bool isRelative) const;
  void dump() const;

  static std::string GetRelativePath(const char *path);

 private:
  std::unique_ptr<CacheManager> cache_manager_;
};

}   // namespace HPHP

#endif  // incl_HPHP_FILE_CACHE_H_
