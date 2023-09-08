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

#include "hphp/runtime/base/tv-refcount.h"

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
struct c_StaticWaitHandle final :
    c_Awaitable,
    SystemLib::ClassLoader<"HH\\StaticWaitHandle"> {
  using SystemLib::ClassLoader<"HH\\StaticWaitHandle">::classof;
  using SystemLib::ClassLoader<"HH\\StaticWaitHandle">::className;
  WAITHANDLE_DTOR(StaticWaitHandle);

  explicit c_StaticWaitHandle()
    : c_Awaitable(c_StaticWaitHandle::classof(),
                  HeaderKind::WaitHandle,
                  type_scan::getIndexForMalloc<c_StaticWaitHandle>())
  {}
  ~c_StaticWaitHandle() {
    assertx(isFinished());
    tvDecRefGen(&m_resultOrException);
  }

 public:
  static c_StaticWaitHandle* CreateSucceeded(TypedValue result); // nothrow
  static c_StaticWaitHandle* CreateFailed(ObjectData* exception);

  static rds::Link<Object, rds::Mode::Normal> NullHandle;
  static rds::Link<Object, rds::Mode::Normal> TrueHandle;
  static rds::Link<Object, rds::Mode::Normal> FalseHandle;

 private:
  static c_StaticWaitHandle* CreateSucceededImpl(TypedValue result); // nothrow
  void setState(uint8_t state) { setKindState(Kind::Static, state); }

  friend struct AsioExtension;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_ASIO_STATIC_WAIT_HANDLE_H_
