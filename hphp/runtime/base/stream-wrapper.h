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

#ifndef HPHP_STREAM_WRAPPER_H
#define HPHP_STREAM_WRAPPER_H

#include <string>
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/directory.h"
#include "hphp/runtime/base/file.h"
#include "hphp/runtime/base/req-ptr.h"

#include <boost/noncopyable.hpp>

struct stat;

namespace HPHP {
class StreamContext;
namespace Stream {
///////////////////////////////////////////////////////////////////////////////

class Wrapper : boost::noncopyable {
 public:
  Wrapper() : m_isLocal(true) { }
  void registerAs(const std::string &scheme);

  virtual req::ptr<File> open(const String& filename,
                              const String& mode,
                              int options,
                              const req::ptr<StreamContext>& context) = 0;
  virtual int access(const String& path, int mode) {
    return -1;
  }
  virtual int lstat(const String& path, struct stat* buf) {
    return -1;
  }
  virtual int stat(const String& path, struct stat* buf) {
    return -1;
  }
  virtual int unlink(const String& path) {
    return -1;
  }
  virtual int rename(const String& oldname, const String& newname) {
    return -1;
  }
  virtual int mkdir(const String& path, int mode, int options) {
    return -1;
  }
  virtual int rmdir(const String& path, int options) {
    return -1;
  }
  virtual req::ptr<Directory> opendir(const String& path) {
    return nullptr;
  }

  virtual ~Wrapper() {}

  /**
   * Is there a chance that open() could return a file that is local?
   */
  bool m_isLocal;
};

///////////////////////////////////////////////////////////////////////////////
}
}

#endif // HPHP_STREAM_WRAPPER_REGISTRY_H
