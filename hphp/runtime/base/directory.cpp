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

#include "hphp/runtime/base/directory.h"
#include "hphp/runtime/base/types.h"
#include <sys/types.h>

namespace HPHP {

IMPLEMENT_OBJECT_ALLOCATION(PlainDirectory)
IMPLEMENT_OBJECT_ALLOCATION(ArrayDirectory)

///////////////////////////////////////////////////////////////////////////////

StaticString Directory::s_class_name("Directory");

///////////////////////////////////////////////////////////////////////////////

PlainDirectory::PlainDirectory(CStrRef path) {
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

String PlainDirectory::read() {
  struct dirent entry;
  struct dirent *result;
  int ret = readdir_r(m_dir, &entry, &result);
  if (ret != 0 || !result) {
    return null_string;
  }
  return String(entry.d_name, CopyString);
}

void PlainDirectory::rewind() {
  ::rewinddir(m_dir);
}

bool PlainDirectory::isValid() const {
  return m_dir;
}

String ArrayDirectory::read() {
  if (!m_it) {
    return null_string;
  }

  String ret = m_it.second();
  ++m_it;
  return ret;
}

void ArrayDirectory::rewind() {
  m_it.setPos(0);
}

///////////////////////////////////////////////////////////////////////////////
}
