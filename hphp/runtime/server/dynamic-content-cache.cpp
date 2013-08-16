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

#include "hphp/runtime/server/dynamic-content-cache.h"
#include "hphp/util/lock.h"
#include "hphp/util/compression.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DynamicContentCache DynamicContentCache::TheCache;

DynamicContentCache::DynamicContentCache() {
}

bool DynamicContentCache::find(const std::string &name, const char *&data,
                               int &len, bool &compressed) {
  assert(!name.empty());

  ReadLock lock(m_mutex);
  auto const iter = m_files.find(name);
  if (iter != m_files.end()) {
    auto& f = *iter->second;
    if (compressed && f.compressed) {
      data = f.compressed->data();
      len = f.compressed->size();
    } else {
      compressed = false;
      data = f.file->data();
      len = f.file->size();
    }
    return true;
  }
  return false;
}

void DynamicContentCache::store(const std::string &name, const char *data,
                                int size) {
  assert(!name.empty());
  assert(size > 0);

  auto const f = std::make_shared<ResourceFile>();
  auto const sb = std::make_shared<CstrBuffer>(size);
  sb->append(StringSlice{data, static_cast<uint32_t>(size)}); // makes a copy
  f->file = sb;
  int len = sb->size();
  char *compressed = gzencode(sb->data(), len, 9, CODING_GZIP);
  if (compressed) {
    if (unsigned(len) < sb->size()) {
      f->compressed = std::make_shared<CstrBuffer>(compressed, len);
    } else {
      free(compressed);
    }
  }

  WriteLock lock(m_mutex);
  if (m_files.find(name) == m_files.end()) {
    m_files[name] = f;
  }
}

///////////////////////////////////////////////////////////////////////////////
}

