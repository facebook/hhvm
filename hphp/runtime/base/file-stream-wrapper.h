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

#ifndef HPHP_FILE_STREAM_WRAPPER_H
#define HPHP_FILE_STREAM_WRAPPER_H

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/file.h"
#include "hphp/runtime/base/mem-file.h"
#include "hphp/runtime/base/stream-wrapper.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class Directory;

class FileStreamWrapper : public Stream::Wrapper {
 public:
  static MemFile* openFromCache(const String& filename, const String& mode);
  virtual File* open(const String& filename, const String& mode,
                     int options, const Variant& context);
  virtual int access(const String& path, int mode) {
    return valid(path) ? ::access(TranslatePath(path).data(), mode) : -1;
  }
  virtual int stat(const String& path, struct stat* buf) {
    return valid(path) ? ::stat(TranslatePath(path).data(), buf) : -1;
  }
  virtual int lstat(const String& path, struct stat* buf) {
    return valid(path) ? ::lstat(TranslatePath(path).data(), buf) : -1;
  }
  virtual int unlink(const String& path) {
    return valid(path) ? ::unlink(TranslatePath(path).data()) : -1;
  }
  virtual int rename(const String& oldname, const String& newname);
  virtual int mkdir(const String& path, int mode, int options);
  virtual int rmdir(const String& path, int options) {
    return valid(path) ? ::rmdir(TranslatePath(path).data()) : -1;
  }

  virtual Directory* opendir(const String& path);

 private:
  int mkdir_recursive(const String& path, int mode);
  virtual bool valid(const String& path) {
    assert(strncmp(path.c_str(), "file://", sizeof("file://") - 1) == 0);
    if (path.data()[7] == '/') { // not just file://, but file:///
      return true;
    }
    raise_warning("Only hostless file::// URLs are supported: %s", path.data());
    errno = ENOENT;
    return false;
  }
  virtual String TranslatePath(const String& filename) {
    assert(valid(filename));
    return File::TranslatePath(filename.substr(sizeof("file://") - 1));
  }
};

class PlainStreamWrapper : public FileStreamWrapper {
 private:
  virtual bool valid(const String& path) { return true; }
  virtual  String TranslatePath(const String& filename) {
    return File::TranslatePath(filename);
  }
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // HPHP_FILE_STREAM_WRAPPER_H
