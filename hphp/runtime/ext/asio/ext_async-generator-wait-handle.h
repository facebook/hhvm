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

#ifndef incl_HPHP_EXT_ASIO_ASYNC_GENERATOR_WAIT_HANDLE_H_
#define incl_HPHP_EXT_ASIO_ASYNC_GENERATOR_WAIT_HANDLE_H_

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/asio/ext_resumable-wait-handle.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/resumable.h"
#include "hphp/runtime/vm/jit/types.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// class AsyncGeneratorWaitHandle

/**
 * An async generator wait handle represents one step of asynchronous execution
 * between two yield statements of an async generator.
 *
 * Unfinished AsyncGeneratorWaitHandles are always referenced by m_waitHandle
 * property of AsyncGenerators. In addition, semi-implicit references may be
 * implied based on the current state:
 *
 * SUCCEEDED, FAILED: no reference
 * BLOCKED: referenced by child via its AsioBlockableChain
 * READY: referenced once per each entry in AsioContext runnable queues
 * RUNNING: referenced by being executed
 *
 * When transitioning between states, incref/decref pairs are simply avoided.
 * The initial incref happens at the construction time and corresponding decref
 * at transition to the SUCCEEDED or FAILED state.
 */
struct AsyncGenerator;

struct c_AsyncGeneratorWaitHandle final :
    c_ResumableWaitHandle,
    SystemLib::ClassLoader<"HH\\AsyncGeneratorWaitHandle"> {
  using SystemLib::ClassLoader<"HH\\AsyncGeneratorWaitHandle">::classof;
  using SystemLib::ClassLoader<"HH\\AsyncGeneratorWaitHandle">::className;
  WAITHANDLE_DTOR(AsyncGeneratorWaitHandle);

  c_AsyncGeneratorWaitHandle(AsyncGenerator* gen, c_WaitableWaitHandle* child);
  ~c_AsyncGeneratorWaitHandle();

 public:
  static constexpr ptrdiff_t generatorOff() {
    return offsetof(c_AsyncGeneratorWaitHandle, m_generator);
  }
  static constexpr ptrdiff_t blockableOff() {
    return offsetof(c_AsyncGeneratorWaitHandle, m_blockable);
  }

  static c_AsyncGeneratorWaitHandle* Create(
    const ActRec* fp,
    jit::TCA resumeAddr,
    Offset suspendOffset,
    c_WaitableWaitHandle* child
  ); // nothrow

  void resume();
  void onUnblocked();
  void await(req::ptr<c_WaitableWaitHandle>&& child);
  void ret(TypedValue& result);
  void fail(ObjectData* exception);
  void failCpp();
  String getName();
  c_WaitableWaitHandle* getChild();
  void exitContext(context_idx_t ctx_idx);
  bool isRunning() { return getState() == STATE_RUNNING; }

  Resumable* resumable() const;

 private:
  void setState(uint8_t state) { setKindState(Kind::AsyncGenerator, state); }
  void prepareChild(c_WaitableWaitHandle* child);

  Object m_generator; // has AsyncGenerator nativedata.

  // valid if STATE_READY || STATE_BLOCKED
  c_WaitableWaitHandle* m_child;
  AsioBlockable m_blockable;

  TYPE_SCAN_CUSTOM_FIELD(m_child) {
    auto state = getState();
    if (state == STATE_BLOCKED || state == STATE_READY) {
      scanner.scan(m_child);
    }
  }
};

inline c_AsyncGeneratorWaitHandle* c_Awaitable::asAsyncGenerator() {
  assertx(getKind() == Kind::AsyncGenerator);
  return static_cast<c_AsyncGeneratorWaitHandle*>(this);
}

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_ASIO_ASYNC_GENERATOR_WAIT_HANDLE_H_
