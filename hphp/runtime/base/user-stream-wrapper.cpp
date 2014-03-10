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

#include "hphp/runtime/base/user-stream-wrapper.h"
#include "hphp/system/constants.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

UserStreamWrapper::UserStreamWrapper(const String& name,
                                     const String& clsname,
                                     int flags)
    : m_name(name) {
  m_cls = Unit::loadClass(clsname.get());
  if (!m_cls) {
    throw InvalidArgumentException(0, "Undefined class '%s'", clsname.data());
  }
  m_isLocal = !(flags & k_STREAM_IS_URL);
}

File* UserStreamWrapper::open(const String& filename, const String& mode,
                              int options, const Variant& context) {
  auto file = NEWOBJ(UserFile)(m_cls, context);
  Resource wrapper(file);
  auto ret = file->openImpl(filename, mode, options);
  if (!ret) {
    return nullptr;
  }
  DEBUG_ONLY auto tmp = wrapper.detach();
  assert(tmp == file && file->hasExactlyOneRef());
  file->decRefCount();
  return file;
}

int UserStreamWrapper::access(const String& path, int mode) {
  auto file = NEWOBJ(UserFile)(m_cls);
  Resource wrapper(file);
  return file->access(path, mode);
}
int UserStreamWrapper::lstat(const String& path, struct stat* buf) {
  auto file = NEWOBJ(UserFile)(m_cls);
  Resource wrapper(file);
  return file->lstat(path, buf);
}
int UserStreamWrapper::stat(const String& path, struct stat* buf) {
  auto file = NEWOBJ(UserFile)(m_cls);
  Resource wrapper(file);
  return file->stat(path, buf);
}
int UserStreamWrapper::unlink(const String& path) {
  auto file = NEWOBJ(UserFile)(m_cls);
  Resource wrapper(file);
  return file->unlink(path) ? 0 : -1;
}
int UserStreamWrapper::rename(const String& oldname, const String& newname) {
  auto file = NEWOBJ(UserFile)(m_cls);
  Resource wrapper(file);
  return file->rename(oldname, newname) ? 0 : -1;
}
int UserStreamWrapper::mkdir(const String& path, int mode, int options) {
  auto file = NEWOBJ(UserFile)(m_cls);
  Resource wrapper(file);
  return file->mkdir(path, mode, options) ? 0 : -1;
}
int UserStreamWrapper::rmdir(const String& path, int options) {
  auto file = NEWOBJ(UserFile)(m_cls);
  Resource wrapper(file);
  return file->rmdir(path, options) ? 0 : -1;
}

Directory* UserStreamWrapper::opendir(const String& path) {
  auto dir = NEWOBJ(UserDirectory)(m_cls);
  Resource wrapper(dir);
  auto ret = dir->open(path);
  if (!ret) {
    return nullptr;
  }
  DEBUG_ONLY auto tmp = wrapper.detach();
  assert(tmp == dir);
  return dir;
}

///////////////////////////////////////////////////////////////////////////////
}
