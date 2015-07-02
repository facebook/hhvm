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

#ifndef incl_HPHP_EXT_ASIO_CONDITION_WAIT_HANDLE_H_
#define incl_HPHP_EXT_ASIO_CONDITION_WAIT_HANDLE_H_

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/ext/collections/ext_collections-idl.h"
#include "hphp/runtime/ext/asio/ext_waitable-wait-handle.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// class ConditionWaitHandle

/**
 * A wait handle that waits for a list of wait handles. The wait handle succeeds
 * with null once all given wait handles are finished (succeeded or failed).
 */
class c_ConditionWaitHandle final : public c_WaitableWaitHandle {
 public:
  DECLARE_CLASS_NO_SWEEP(ConditionWaitHandle)

  explicit c_ConditionWaitHandle(Class* cls = c_ConditionWaitHandle::classof())
    : c_WaitableWaitHandle(cls) {}
  ~c_ConditionWaitHandle() {}

  static void ti_setoncreatecallback(const Variant& callback);
  static Object ti_create(const Variant& child);
  void t_succeed(const Variant& result);
  void t_fail(const Variant& exception);

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

 private:
  c_WaitableWaitHandle* m_child;
  AsioBlockable m_blockable;
};

inline c_ConditionWaitHandle* c_WaitHandle::asCondition() {
  assert(getKind() == Kind::Condition);
  return static_cast<c_ConditionWaitHandle*>(this);
}

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_ASIO_CONDITION_WAIT_HANDLE_H_
