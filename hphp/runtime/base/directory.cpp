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

#include "hphp/runtime/base/types.h"

#include "hphp/runtime/ext/ext_file.h"

#include <sys/types.h>

namespace HPHP {

IMPLEMENT_OBJECT_ALLOCATION(PlainDirectory)

///////////////////////////////////////////////////////////////////////////////

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
  return Variant(f_basename(ret.toString()));
}

void ArrayDirectory::rewind() {
  m_it.setPos(0);
}

String ArrayDirectory::path() {
  if (!m_it) {
    return empty_string;
  }

  auto entry = m_it.second();
  assert(entry.isString());
  return f_dirname(entry.toString());
}

///////////////////////////////////////////////////////////////////////////////
}
