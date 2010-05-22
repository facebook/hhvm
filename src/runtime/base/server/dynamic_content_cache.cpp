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

#include <runtime/base/server/dynamic_content_cache.h>
#include <util/lock.h>
#include <util/compression.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DynamicContentCache DynamicContentCache::TheCache;

DynamicContentCache::DynamicContentCache() {
}

bool DynamicContentCache::find(const std::string &name, const char *&data,
                               int &len, bool &compressed) {
  ASSERT(!name.empty());

  ReadLock lock(m_mutex);
  StringToResourceFilePtrMap::const_iterator iter = m_files.find(name);
  if (iter != m_files.end()) {
    ResourceFile &f = *iter->second;
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
  ASSERT(!name.empty());
  ASSERT(size > 0);

  ResourceFilePtr f(new ResourceFile());
  StringBufferPtr sb(new StringBuffer(size));
  sb->append(data, size);
  f->file = sb;

  int len = sb->size();
  char *compressed = gzencode(sb->data(), len, 9, CODING_GZIP);
  if (compressed) {
    if (len < sb->size()) {
      f->compressed = StringBufferPtr(new StringBuffer(compressed, len));
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

