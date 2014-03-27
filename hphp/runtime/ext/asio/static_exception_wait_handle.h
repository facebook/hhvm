/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#ifndef incl_HPHP_EXT_ASIO_STATIC_EXCEPTION_WAIT_HANDLE_H_
#define incl_HPHP_EXT_ASIO_STATIC_EXCEPTION_WAIT_HANDLE_H_

#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/ext/asio/static_wait_handle.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// class StaticExceptionWaitHandle

/**
 * A wait handle that is statically failed with an exception.
 */
FORWARD_DECLARE_CLASS(StaticExceptionWaitHandle);
class c_StaticExceptionWaitHandle : public c_StaticWaitHandle {
 public:
  DECLARE_CLASS_NO_SWEEP(StaticExceptionWaitHandle)

  explicit c_StaticExceptionWaitHandle(Class* cls =
      c_StaticExceptionWaitHandle::classof())
    : c_StaticWaitHandle(cls)
  {
    setState(STATE_FAILED);
  }
  ~c_StaticExceptionWaitHandle() {
    tvRefcountedDecRefCell(&m_resultOrException);
  }

  void t___construct();

 public:
  static c_StaticExceptionWaitHandle* Create(ObjectData* exception);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_ASIO_STATIC_EXCEPTION_WAIT_HANDLE_H_
