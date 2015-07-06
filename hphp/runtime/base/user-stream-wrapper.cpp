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
#include "hphp/runtime/ext/stream/ext_stream.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

UserStreamWrapper::UserStreamWrapper(const String& name,
                                     Class* cls,
                                     int flags)
  : m_name(name)
  , m_cls(cls)
{
  assert(m_cls != nullptr);
  m_isLocal = !(flags & k_STREAM_IS_URL);
}

req::ptr<File>
UserStreamWrapper::open(const String& filename, const String& mode,
                        int options, const req::ptr<StreamContext>& context) {
  auto file = req::make<UserFile>(m_cls, context);
  auto ret = file->openImpl(filename, mode, options);
  if (!ret) {
    return nullptr;
  }
  return file;
}

int UserStreamWrapper::access(const String& path, int mode) {
  auto file = req::make<UserFile>(m_cls);
  return file->access(path, mode);
}
int UserStreamWrapper::lstat(const String& path, struct stat* buf) {
  auto file = req::make<UserFile>(m_cls);
  return file->lstat(path, buf);
}
int UserStreamWrapper::stat(const String& path, struct stat* buf) {
  auto file = req::make<UserFile>(m_cls);
  return file->stat(path, buf);
}
int UserStreamWrapper::unlink(const String& path) {
  auto file = req::make<UserFile>(m_cls);
  return file->unlink(path) ? 0 : -1;
}
int UserStreamWrapper::rename(const String& oldname, const String& newname) {
  auto file = req::make<UserFile>(m_cls);
  return file->rename(oldname, newname) ? 0 : -1;
}
int UserStreamWrapper::mkdir(const String& path, int mode, int options) {
  auto file = req::make<UserFile>(m_cls);
  return file->mkdir(path, mode, options) ? 0 : -1;
}
int UserStreamWrapper::rmdir(const String& path, int options) {
  auto file = req::make<UserFile>(m_cls);
  return file->rmdir(path, options) ? 0 : -1;
}

req::ptr<Directory> UserStreamWrapper::opendir(const String& path) {
  auto dir = req::make<UserDirectory>(m_cls);
  auto ret = dir->open(path);
  if (!ret) {
    return nullptr;
  }
  return dir;
}

bool UserStreamWrapper::touch(const String& path,
                              int64_t mtime, int64_t atime) {
  auto file = req::make<UserFile>(m_cls);
  return file->touch(path, mtime, atime);
}

bool UserStreamWrapper::chmod(const String& path, int64_t mode) {
  auto file = req::make<UserFile>(m_cls);
  return file->chmod(path, mode);
}

bool UserStreamWrapper::chown(const String& path, int64_t uid) {
  auto file = req::make<UserFile>(m_cls);
  return file->chown(path, uid);
}

bool UserStreamWrapper::chown(const String& path, const String& uid) {
  auto file = req::make<UserFile>(m_cls);
  return file->chown(path, uid);
}

bool UserStreamWrapper::chgrp(const String& path, int64_t gid) {
  auto file = req::make<UserFile>(m_cls);
  return file->chgrp(path, gid);
}

bool UserStreamWrapper::chgrp(const String& path, const String& gid) {
  auto file = req::make<UserFile>(m_cls);
  return file->chgrp(path, gid);
}

///////////////////////////////////////////////////////////////////////////////
}
