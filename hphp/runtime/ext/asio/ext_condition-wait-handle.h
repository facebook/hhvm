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

#ifndef incl_HPHP_EXT_ASIO_CONDITION_WAIT_HANDLE_H_
#define incl_HPHP_EXT_ASIO_CONDITION_WAIT_HANDLE_H_

#include "hphp/runtime/base/vanilla-dict.h"
#include "hphp/runtime/ext/asio/ext_waitable-wait-handle.h"
#include "hphp/runtime/ext/extension.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// class ConditionWaitHandle

/**
 * A wait handle that waits for a list of wait handles. The wait handle succeeds
 * with null once all given wait handles are finished (succeeded or failed).
 */
struct c_ConditionWaitHandle final :
    c_WaitableWaitHandle,
    SystemLib::ClassLoader<"HH\\ConditionWaitHandle"> {
  using SystemLib::ClassLoader<"HH\\ConditionWaitHandle">::classof;
  using SystemLib::ClassLoader<"HH\\ConditionWaitHandle">::className;
  WAITHANDLE_DTOR(ConditionWaitHandle);

  explicit c_ConditionWaitHandle()
    : c_WaitableWaitHandle(classof(), HeaderKind::WaitHandle,
                       type_scan::getIndexForMalloc<c_ConditionWaitHandle>()) {}
  ~c_ConditionWaitHandle() {}

 public:
  static constexpr ptrdiff_t blockableOff() {
    return offsetof(c_ConditionWaitHandle, m_blockable);
  }

  String getName();
  void onUnblocked();
  c_WaitableWaitHandle* getChild();

  static const int8_t STATE_BLOCKED = 2;

 private:
  void setState(uint8_t state) { setKindState(Kind::Condition, state); }
  void initialize(c_WaitableWaitHandle* child);

  friend Object HHVM_STATIC_METHOD(ConditionWaitHandle, create,
                                   const Variant& child);
  friend void HHVM_METHOD(ConditionWaitHandle, succeed,
                          const Variant& result);
  friend void HHVM_METHOD(ConditionWaitHandle, fail,
                          const Object& ex);
 private:
  c_WaitableWaitHandle* m_child;
  AsioBlockable m_blockable;

  TYPE_SCAN_CUSTOM_FIELD(m_child) {
    if (!isFinished()) scanner.scan(m_child);
  }
};

inline c_ConditionWaitHandle* c_Awaitable::asCondition() {
  assertx(getKind() == Kind::Condition);
  return static_cast<c_ConditionWaitHandle*>(this);
}

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_ASIO_CONDITION_WAIT_HANDLE_H_
