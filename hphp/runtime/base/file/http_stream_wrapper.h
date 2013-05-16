/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifndef HPHP_HTTP_STREAM_WRAPPER_H
#define HPHP_HTTP_STREAM_WRAPPER_H

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/file/file.h"
#include "hphp/runtime/base/file/stream_wrapper.h"
#include "hphp/runtime/base/file/stream_wrapper_registry.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class HttpStreamWrapper : public Stream::Wrapper {
 public:
  virtual File* open(CStrRef filename, CStrRef mode,
                     int options, CVarRef context);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // HPHP_HTTP_STREAM_WRAPPER_H
