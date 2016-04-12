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

#ifndef HPHP_FILE_STREAM_WRAPPER_H
#define HPHP_FILE_STREAM_WRAPPER_H

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "hphp/runtime/base/file.h"
#include "hphp/runtime/base/mem-file.h"
#include "hphp/runtime/base/stream-wrapper.h"
#include <folly/String.h>

#define ERROR_RAISE_WARNING(exp)        \
  int ret = (exp);                      \
  if (ret != 0) {                       \
    raise_warning(                      \
      "%s(): %s",                       \
      __FUNCTION__,                     \
      folly::errnoStr(errno).c_str()    \
    );                                  \
  }                                     \

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct Directory;

struct FileStreamWrapper : Stream::Wrapper {
  static req::ptr<MemFile> openFromCache(
    const String& filename, const String& mode);
  virtual req::ptr<File> open(const String& filename,
                              const String& mode,
                              int options,
                              const req::ptr<StreamContext>& context);
  virtual int access(const String& path, int mode) {
    return ::access(File::TranslatePath(path).data(), mode);
  }
  virtual int stat(const String& path, struct stat* buf) {
    return ::stat(File::TranslatePath(path).data(), buf);
  }
  virtual int lstat(const String& path, struct stat* buf) {
    return ::lstat(File::TranslatePath(path).data(), buf);
  }
  virtual int unlink(const String& path) {
    int ret = ::unlink(File::TranslatePath(path).data());
    if (ret != 0) {
      raise_warning(
        "%s(%s): %s",
        __FUNCTION__,
        path.c_str(),
        folly::errnoStr(errno).c_str()
      );
    }
    return ret;
  }
  virtual int rename(const String& oldname, const String& newname);
  virtual int mkdir(const String& path, int mode, int options);
  virtual int rmdir(const String& path, int options) {
    ERROR_RAISE_WARNING(::rmdir(File::TranslatePath(path).data()));
    return ret;
  }

  virtual req::ptr<Directory> opendir(const String& path);

 private:
  int mkdir_recursive(const String& path, int mode);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // HPHP_FILE_STREAM_WRAPPER_H
