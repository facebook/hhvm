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

#include "hphp/runtime/base/directory.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/types.h"

#include "hphp/runtime/ext/std/ext_std_file.h"
#include "hphp/runtime/base/file.h"
#include "hphp/runtime/server/static-content-cache.h"

#include <sys/types.h>
#include <tuple>

namespace HPHP {

IMPLEMENT_RESOURCE_ALLOCATION(PlainDirectory)

///////////////////////////////////////////////////////////////////////////////

const StaticString
  s_wrapper_type("wrapper_type"),
  s_stream_type("stream_type"),
  s_mode("mode"),
  s_unread_bytes("unread_bytes"),
  s_seekable("seekable"),
  s_timed_out("timed_out"),
  s_blocked("blocked"),
  s_eof("eof"),
  s_plainfile("plainfile"),
  s_dir("dir"),
  s_r("r");

Array Directory::getMetaData() {
  return make_map_array(
    s_wrapper_type, s_plainfile, // PHP5 compatibility
    s_stream_type,  s_dir,
    s_mode,         s_r,
    s_unread_bytes, 0,
    s_seekable,     false,
    s_timed_out,    false,
    s_blocked,      true,
    s_eof,          isEof()
  );
}

///////////////////////////////////////////////////////////////////////////////

PlainDirectory::PlainDirectory(const String& path) {
  m_dir = ::opendir(path.data());
}

PlainDirectory::~PlainDirectory() {
  close();
}

void PlainDirectory::close() {
  if (m_dir) {
    ::closedir(m_dir);
    m_dir = nullptr;
  }
}

Variant PlainDirectory::read() {
  struct dirent entry;
  struct dirent *result;
  int ret = readdir_r(m_dir, &entry, &result);
  if (ret != 0 || !result) {
    return false;
  }
  return String(entry.d_name, CopyString);
}

void PlainDirectory::rewind() {
  ::rewinddir(m_dir);
}

bool PlainDirectory::isValid() const {
  return m_dir;
}

Variant ArrayDirectory::read() {
  if (!m_it) {
    return false;
  }

  auto ret = m_it.second();
  assert(ret.isString());
  ++m_it;
  return Variant(HHVM_FN(basename)(ret.toString()));
}

void ArrayDirectory::rewind() {
  m_it.setPos(0);
}

bool ArrayDirectory::isEof() const {
  return m_it.end();
}

String ArrayDirectory::path() {
  if (!m_it) {
    return empty_string();
  }

  auto entry = m_it.second();
  assert(entry.isString());
  return HHVM_FN(dirname)(entry.toString());
}

CachedDirectory::CachedDirectory(const String& path) {
  assert(File::IsVirtualDirectory(path));
  m_files = StaticContentCache::TheFileCache->readDirectory(path.c_str());
}

///////////////////////////////////////////////////////////////////////////////
}
