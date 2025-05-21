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

#pragma once

#include "hphp/runtime/ext/asio/ext_waitable-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_resumable-wait-handle.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// class PriorityBridgeWaitHandle

/**
 * A PriorityBridgeWaitHandle is a wait handle that runs in the current context
 * with a child in a lower priority context. The child is allowed to execute
 * in a lowpri state, and then will unblock this WH when finished. Awaiting
 * a PBWH does not lift the context of the child into the current context.
 */
struct c_PriorityBridgeWaitHandle final
    : c_WaitableWaitHandle,
      SystemLib::ClassLoader<"HH\\PriorityBridgeWaitHandle"> {
  using SystemLib::ClassLoader<"HH\\PriorityBridgeWaitHandle">::classof;
  using SystemLib::ClassLoader<"HH\\PriorityBridgeWaitHandle">::className;
  WAITHANDLE_DTOR(PriorityBridgeWaitHandle)

  explicit c_PriorityBridgeWaitHandle()
      : c_WaitableWaitHandle(
            classof(), HeaderKind::WaitHandle,
            type_scan::getIndexForMalloc<c_PriorityBridgeWaitHandle>()) {}
  ~c_PriorityBridgeWaitHandle() {}

public:
  static constexpr ptrdiff_t blockableOff() {
    return offsetof(c_PriorityBridgeWaitHandle, m_blockable);
  }

  String getName();
  void onUnblocked();
  c_WaitableWaitHandle* getChild();

  static const int8_t STATE_BLOCKED = 2;

private:
  void setState(uint8_t state) { setKindState(Kind::PriorityBridge, state); }
  void initialize(c_WaitableWaitHandle *child);

  friend Object HHVM_STATIC_METHOD(PriorityBridgeWaitHandle, create,
                                   const Object& child);

private:
  c_WaitableWaitHandle* m_child{};
  AsioBlockable m_blockable{};

  TYPE_SCAN_CUSTOM_FIELD(m_child) {
    if (!isFinished())
      scanner.scan(m_child);
  }
};

inline c_PriorityBridgeWaitHandle *c_Awaitable::asPriorityBridge() {
  assertx(getKind() == Kind::PriorityBridge);
  return static_cast<c_PriorityBridgeWaitHandle *>(this);
}

///////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
