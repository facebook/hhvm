/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1998-2010 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
*/

#ifndef HPHP_FILE_STREAM_WRAPPER_H
#define HPHP_FILE_STREAM_WRAPPER_H

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/file.h"
#include "hphp/runtime/base/mem_file.h"
#include "hphp/runtime/base/stream_wrapper.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class FileStreamWrapper : public Stream::Wrapper {
 public:
  static MemFile* openFromCache(CStrRef filename, CStrRef mode);
  virtual File* open(CStrRef filename, CStrRef mode,
                     int options, CVarRef context);
  virtual int access(CStrRef path, int mode) {
    return ::access(File::TranslatePath(path).data(), mode);
  }
  virtual int stat(CStrRef path, struct stat* buf) {
    return ::stat(File::TranslatePath(path).data(), buf);
  }
  virtual int lstat(CStrRef path, struct stat* buf) {
    return ::lstat(File::TranslatePath(path).data(), buf);
  }
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // HPHP_FILE_STREAM_WRAPPER_H
