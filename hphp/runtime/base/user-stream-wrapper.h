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
#ifndef HPHP_USER_STREAM_WRAPPER_H
#define HPHP_USER_STREAM_WRAPPER_H

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/file.h"
#include "hphp/runtime/base/stream-wrapper.h"
#include "hphp/runtime/base/user-file.h"
#include "hphp/runtime/base/user-directory.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct UserStreamWrapper final : Stream::Wrapper {
  UserStreamWrapper(const String& name, Class*, int flags);

  req::ptr<File> open(const String& filename,
                      const String& mode,
                      int options,
                      const req::ptr<StreamContext>& context) override;
  int access(const String& path, int mode) override;
  int lstat(const String& path, struct stat* buf) override;
  int stat(const String& path, struct stat* buf) override;
  int unlink(const String& path) override;
  int rename(const String& oldname, const String& newname) override;
  int mkdir(const String& path, int mode, int options) override;
  int rmdir(const String& path, int options) override;
  req::ptr<Directory> opendir(const String& path) override;
  bool touch(const String& path, int64_t mtime, int64_t atime);
  bool chmod(const String& path, int64_t mode);
  bool chown(const String& path, int64_t uid);
  bool chown(const String& path, const String& uid);
  bool chgrp(const String& path, int64_t gid);
  bool chgrp(const String& path, const String& gid);

private:
  String m_name;
  LowPtr<Class> m_cls;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // HPHP_USER_STREAM_WRAPPER_H
