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

#ifndef HPHP_DATA_STREAM_WRAPPER_H
#define HPHP_DATA_STREAM_WRAPPER_H

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/file.h"
#include "hphp/runtime/base/stream-wrapper.h"
#include "hphp/runtime/base/stream-wrapper-registry.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////n

class DataStreamWrapper : public Stream::Wrapper {
 public:
  DataStreamWrapper() {
    m_isLocal = true;
  }
  virtual File* open(const String& filename, const String& mode,
                     int options, const Variant& context);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // HPHP_DATA_STREAM_WRAPPER_H
