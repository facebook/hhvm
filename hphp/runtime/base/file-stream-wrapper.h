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

#ifndef HPHP_FILE_STREAM_WRAPPER_H
#define HPHP_FILE_STREAM_WRAPPER_H

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
                     int options, CVarRef context);
  virtual int access(const String& path, int mode) {
    return ::access(File::TranslatePath(path).data(), mode);
  }
  virtual int stat(const String& path, struct stat* buf) {
    return ::stat(File::TranslatePath(path).data(), buf);
  }
  virtual int lstat(const String& path, struct stat* buf) {
    return ::lstat(File::TranslatePath(path).data(), buf);
  }
  virtual Directory* opendir(const String& path);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // HPHP_FILE_STREAM_WRAPPER_H
