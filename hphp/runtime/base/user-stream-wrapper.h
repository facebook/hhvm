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
#ifndef HPHP_USER_STREAM_WRAPPER_H
#define HPHP_USER_STREAM_WRAPPER_H

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/file.h"
#include "hphp/runtime/base/stream-wrapper.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class UserStreamWrapper : public Stream::Wrapper {
 public:
  UserStreamWrapper(CStrRef name, CStrRef clsname);
  virtual File* open(CStrRef filename, CStrRef mode,
                     int options, CVarRef context);
 private:
  String m_name;
  Class *m_cls;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // HPHP_USER_STREAM_WRAPPER_H
