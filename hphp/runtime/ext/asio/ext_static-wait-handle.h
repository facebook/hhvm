/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#ifndef incl_HPHP_EXT_ASIO_STATIC_WAIT_HANDLE_H_
#define incl_HPHP_EXT_ASIO_STATIC_WAIT_HANDLE_H_

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/asio/ext_wait-handle.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// class StaticWaitHandle

/**
 * A static wait handle is a wait handle that is statically finished. The result
 * of the operation is always available and waiting for the wait handle finishes
 * immediately.
 */
struct c_StaticWaitHandle final : c_WaitHandle {
  WAITHANDLE_CLASSOF(StaticWaitHandle);
  WAITHANDLE_DTOR(StaticWaitHandle);

  explicit c_StaticWaitHandle()
    : c_WaitHandle(c_StaticWaitHandle::classof(),
                   HeaderKind::WaitHandle,
                   type_scan::getIndexForMalloc<c_StaticWaitHandle>())
  {}
  ~c_StaticWaitHandle() {
    assert(isFinished());
    tvRefcountedDecRef(&m_resultOrException);
  }

 public:
  static c_StaticWaitHandle* CreateSucceeded(Cell result); // nothrow
  static c_StaticWaitHandle* CreateFailed(ObjectData* exception);

 private:
  void setState(uint8_t state) { setKindState(Kind::Static, state); }
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_ASIO_STATIC_WAIT_HANDLE_H_
