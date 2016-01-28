/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/user-directory.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/ext/stream/ext_stream.h"

namespace HPHP {

const StaticString
  s_dir_opendir("dir_opendir"),
  s_dir_closedir("dir_closedir"),
  s_dir_readdir("dir_readdir"),
  s_dir_rewinddir("dir_rewinddir");

///////////////////////////////////////////////////////////////////////////////

UserDirectory::UserDirectory(Class* cls) : UserFSNode(cls) {
  m_DirOpen  = lookupMethod(s_dir_opendir.get());
  m_DirClose = lookupMethod(s_dir_closedir.get());
  m_DirRead  = lookupMethod(s_dir_readdir.get());
  m_DirRewind  = lookupMethod(s_dir_rewinddir.get());
}

void UserDirectory::sweep() {
  // Don't close like the parent, 'cause that's what zend does
}

bool UserDirectory::open(const String& path) {
  // bool dir_opendir ( string $path , int $options )
  bool invoked = false;
  Variant ret = invoke(m_DirOpen, s_dir_opendir,
                       make_packed_array(path, 0), invoked);
  if (invoked && ret.toBoolean()) {
    return true;
  }
  raise_warning("\"%s::dir_opendir\" call failed", m_cls->name()->data());
  return false;
}

void UserDirectory::close() {
  // void dir_closedir()
  invoke(m_DirClose, s_dir_closedir, Array::Create());
}

Variant UserDirectory::read() {
  // String dir_readdir()
  bool invoked = false;
  auto ret = invoke(m_DirRead, s_dir_readdir,
                    Array::Create(), invoked);
  if (!invoked) {
    raise_warning("%s::dir_readdir is not implemented",
                  m_cls->name()->data());
    return init_null();
  }
  return ret;
}

void UserDirectory::rewind() {
  // String dir_rewinddir()
  bool invoked = false;
  invoke(m_DirRewind, s_dir_rewinddir, Array::Create(), invoked);
  if (!invoked) {
    raise_warning("%s::dir_rewinddir is not implemented",
                  m_cls->name()->data());
  }
}

}
