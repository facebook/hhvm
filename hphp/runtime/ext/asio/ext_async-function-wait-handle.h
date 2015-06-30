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

#ifndef incl_HPHP_EXT_ASIO_ASYNC_FUNCTION_WAIT_HANDLE_H_
#define incl_HPHP_EXT_ASIO_ASYNC_FUNCTION_WAIT_HANDLE_H_

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/asio/ext_resumable-wait-handle.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/resumable.h"
#include "hphp/runtime/vm/jit/types.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// class AsyncFunctionWaitHandle

/**
 * An async function wait handle represents a basic unit of asynchronous
 * execution. A dependency on another wait handle is set up by awaiting such
 * wait handle, giving control of the execution back to the asio framework.
 */
class c_AsyncFunctionWaitHandle final : public c_ResumableWaitHandle {
 public:
  DECLARE_CLASS_NO_SWEEP(AsyncFunctionWaitHandle)

  class Node final {
   public:
    static constexpr ptrdiff_t childOff() {
      return offsetof(Node, m_child);
    }
    static constexpr ptrdiff_t blockableOff() {
      return offsetof(Node, m_blockable);
    }

    void setChild(c_WaitableWaitHandle* child);
    c_WaitableWaitHandle* getChild() const;
    bool isFirstUnfinishedChild() const;
    c_AsyncFunctionWaitHandle* getWaitHandle() const;
    void onUnblocked();

   private:
    template <typename F> friend void scan(const Node&, F&);
    c_WaitableWaitHandle* m_child;
    AsioBlockable m_blockable;
  };

  explicit c_AsyncFunctionWaitHandle(Class* cls =
      c_AsyncFunctionWaitHandle::classof()) noexcept
    : c_ResumableWaitHandle(cls, HeaderKind::ResumableObj) {}
  ~c_AsyncFunctionWaitHandle();
  void t___construct();

 public:
  static constexpr ptrdiff_t resumableOff() { return -sizeof(Resumable); }
  static constexpr ptrdiff_t arOff() {
    return resumableOff() + Resumable::arOff();
  }
  static constexpr ptrdiff_t resumeAddrOff() {
    return resumableOff() + Resumable::resumeAddrOff();
  }
  static constexpr ptrdiff_t resumeOffsetOff() {
    return resumableOff() + Resumable::resumeOffsetOff();
  }
  static constexpr ptrdiff_t childrenOff() {
    return offsetof(c_AsyncFunctionWaitHandle, m_children);
  }
  template <bool mayUseVV>
  static c_AsyncFunctionWaitHandle* Create(
    const ActRec* origFp,
    size_t numSlots,
    jit::TCA resumeAddr,
    Offset resumeOffset,
    c_WaitableWaitHandle* child
  ); // nothrow
  static void PrepareChild(const ActRec* fp, c_WaitableWaitHandle* child);
  void resume();
  void onUnblocked();
  void await(Offset resumeOffset, c_WaitableWaitHandle* child);
  void ret(Cell& result);
  void fail(ObjectData* exception);
  void failCpp();
  String getName();
  c_WaitableWaitHandle* getChild();
  void exitContext(context_idx_t ctx_idx);
  bool isRunning() { return getState() == STATE_RUNNING; }
  String getFileName();
  Offset getNextExecutionOffset();
  int getLineNumber();

  Resumable* resumable() const {
    return reinterpret_cast<Resumable*>(
      const_cast<char*>(reinterpret_cast<const char*>(this) + resumableOff()));
  }

  ActRec* actRec() const {
    return resumable()->actRec();
  }

 private:
  void setState(uint8_t state) { setKindState(Kind::AsyncFunction, state); }
  void initialize(c_WaitableWaitHandle* child);
  void prepareChild(c_WaitableWaitHandle* child);

  // valid if STATE_SCHEDULED || STATE_BLOCKED
  Node m_children[1];
};

inline c_AsyncFunctionWaitHandle* c_WaitHandle::asAsyncFunction() {
  assert(getKind() == Kind::AsyncFunction);
  return static_cast<c_AsyncFunctionWaitHandle*>(this);
}

///////////////////////////////////////////////////////////////////////////////
}

#include "hphp/runtime/ext/asio/ext_async-function-wait-handle-inl.h"

#endif // incl_HPHP_EXT_ASIO_ASYNC_FUNCTION_WAIT_HANDLE_H_
