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

#ifndef incl_HPHP_DYNAMIC_CONTENT_CACHE_H_
#define incl_HPHP_DYNAMIC_CONTENT_CACHE_H_

#include <memory>

#include "hphp/runtime/base/string-buffer.h"
#include "hphp/util/mutex.h"

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
  struct ResourceFile {
    std::shared_ptr<CstrBuffer> file;
    std::shared_ptr<CstrBuffer> compressed;
  };

  ReadWriteMutex m_mutex;
  hphp_hash_map<std::string,std::shared_ptr<ResourceFile>,string_hash>
    m_files;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_DYNAMIC_CONTENT_CACHE_H_
