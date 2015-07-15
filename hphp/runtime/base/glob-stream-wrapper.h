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

#ifndef HPHP_GLOB_STREAM_WRAPPER_H
#define HPHP_GLOB_STREAM_WRAPPER_H

#include "hphp/runtime/base/stream-wrapper.h"
#include "hphp/runtime/base/runtime-error.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class GlobStreamWrapper : public Stream::Wrapper {
 public:
  virtual req::ptr<File> open(const String& filename,
                              const String& mode,
                              int options,
                              const req::ptr<StreamContext>& context);
  virtual req::ptr<Directory> opendir(const String& path);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // HPHP_GLOB_STREAM_WRAPPER_H
