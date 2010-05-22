/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __STATIC_CONTENT_CACHE_H__
#define __STATIC_CONTENT_CACHE_H__

#include <runtime/base/util/string_buffer.h>
#include <util/file_cache.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class StaticContentCache {
public:
  static StaticContentCache TheCache;
  static FileCachePtr TheFileCache;

public:
  StaticContentCache();

  /**
   * Load all registered static files from RuntimeOption::DocumentRoot.
   */
  void load();

  /**
   * Find a file from cache.
   */
  bool find(const std::string &name, const char *&data, int &len,
            bool &compressed) const;

private:
  int m_totalSize;

  struct ResourceFile {
    StringBufferPtr file;
    StringBufferPtr compressed;
  };
  DECLARE_BOOST_TYPES(ResourceFile);

  StringToResourceFilePtrMap m_files;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __STATIC_CONTENT_CACHE_H__
