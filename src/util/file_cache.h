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

#ifndef __FILE_CACHE_H__
#define __FILE_CACHE_H__

#include "base.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * Stores file contents in memory. Used by web server for faster static
 * content serving.
 */
DECLARE_BOOST_TYPES(FileCache);
class FileCache {
public:
  static std::string SourceRoot;

public:
  FileCache() : m_fd(-1), m_size(0), m_addr(NULL) {}
  ~FileCache();

  /**
   * Archiving data.
   */
  void write(const char *name, bool addDirectories = true);
  void write(const char *name, const char *fullpath); // name + data
  void save(const char *filename);

  /**
   * Reading data.
   */
  short getVersion(const char *filename);
  void load(const char *filename, bool onDemandUncompress, short version);
  void loadMmap(const char *filename, short version);
  void adviseOutMemory();
  bool fileExists(const char *name, bool isRelative = true) const;
  bool dirExists(const char *name, bool isRelative = true) const;
  bool exists(const char *name, bool isRelative = true) const;
  char *read(const char *name, int &len, bool &compressed) const;

  void dump();

  static std::string GetRelativePath(const char *path);
private:
  struct Buffer {
    int len;     // uncompressed len     -1: PHP file, -2: directories
    char *data;  // uncompressed data
    int clen;    // compressed len
    char *cdata; // compressed data
  };
  typedef __gnu_cxx::hash_map<std::string, Buffer, string_hash> FileMap;

  FileMap m_files;
  int m_fd;
  int m_size;
  void *m_addr;

  void writeDirectories(const char *name);

};

///////////////////////////////////////////////////////////////////////////////
}

#endif  // __FILE_CACHE_H__
