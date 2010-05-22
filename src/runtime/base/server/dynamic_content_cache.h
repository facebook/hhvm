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

#ifndef __DYNAMIC_CONTENT_CACHE_H__
#define __DYNAMIC_CONTENT_CACHE_H__

#include <runtime/base/util/string_buffer.h>
#include <util/mutex.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class DynamicContentCache {
public:
  static DynamicContentCache TheCache;

public:
  DynamicContentCache();

  /**
   * Find a file from cache.
   */
  bool find(const std::string &name, const char *&data, int &len,
            bool &compressed);

  /**
   * Store a file to cache.
   */
  void store(const std::string &name, const char *data, int size);

private:
  ReadWriteMutex m_mutex;

  struct ResourceFile {
    StringBufferPtr file;
    StringBufferPtr compressed;
  };
  DECLARE_BOOST_TYPES(ResourceFile);

  StringToResourceFilePtrMap m_files;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __DYNAMIC_CONTENT_CACHE_H__
