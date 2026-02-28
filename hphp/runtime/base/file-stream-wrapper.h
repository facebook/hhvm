/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/base/file.h"
#include "hphp/runtime/base/mem-file.h"
#include "hphp/runtime/base/stat-cache.h"
#include "hphp/runtime/base/stream-wrapper.h"
#include <folly/String.h>
#include <folly/portability/SysStat.h>
#include <folly/portability/Unistd.h>

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

struct FileStreamWrapper final : Stream::Wrapper {
  static req::ptr<MemFile> openFromCache(const String& filename,
                                         const String& mode);
  req::ptr<File> open(const String& filename, const String& mode, int options,
                      const req::ptr<StreamContext>& context) override;
  int access(const String& path, int mode) override {
    return ::access(File::TranslatePath(path).data(), mode);
  }
  int stat(const String& path, struct stat* buf) override {
    return ::stat(File::TranslatePath(path).data(), buf);
  }
  int lstat(const String& path, struct stat* buf) override {
    return ::lstat(File::TranslatePath(path).data(), buf);
  }
  int unlink(const String& path) override;
  int rename(const String& oldname, const String& newname) override;
  int mkdir(const String& path, int mode, int options) override;
  int rmdir(const String& path, int /*options*/) override {
    ERROR_RAISE_WARNING(::rmdir(File::TranslatePath(path).data()));
    return ret;
  }

  Optional<std::string> getxattr(const char* path, const char* xattr) override;

  bool isNormalFileStream() const override { return true; }

  req::ptr<Directory> opendir(const String& path) override;

  String realpath(const String& path) override {
    return realpathLibc(File::TranslatePath(path).data());
  }

 private:
  int mkdir_recursive(const String& path, int mode);
};

extern FileStreamWrapper s_file_stream_wrapper;

///////////////////////////////////////////////////////////////////////////////
}

#endif // HPHP_FILE_STREAM_WRAPPER_H
