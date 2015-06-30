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
 */
class AsyncGenerator;

class c_AsyncGeneratorWaitHandle final : public c_ResumableWaitHandle {
 public:
  DECLARE_CLASS_NO_SWEEP(AsyncGeneratorWaitHandle)

  explicit c_AsyncGeneratorWaitHandle(Class* cls =
      c_AsyncGeneratorWaitHandle::classof())
    : c_ResumableWaitHandle(cls) {}
  ~c_AsyncGeneratorWaitHandle();

  void t___construct();

 public:
  static constexpr ptrdiff_t blockableOff() {
    return offsetof(c_AsyncGeneratorWaitHandle, m_blockable);
  }
  static c_AsyncGeneratorWaitHandle* Create(AsyncGenerator* gen,
                                            c_WaitableWaitHandle* child);
  void resume();
  void onUnblocked();
  void await(c_WaitableWaitHandle* child);
  void ret(Cell& result);
  void fail(ObjectData* exception);
  void failCpp();
  String getName();
  c_WaitableWaitHandle* getChild();
  void exitContext(context_idx_t ctx_idx);
  bool isRunning() { return getState() == STATE_RUNNING; }

 private:
  void setState(uint8_t state) { setKindState(Kind::AsyncGenerator, state); }
  void initialize(AsyncGenerator* gen, c_WaitableWaitHandle* child);
  void prepareChild(c_WaitableWaitHandle* child);

  AsyncGenerator* m_generator;

  // valid if STATE_SCHEDULED || STATE_BLOCKED
  c_WaitableWaitHandle* m_child;
  AsioBlockable m_blockable;
};

inline c_AsyncGeneratorWaitHandle* c_WaitHandle::asAsyncGenerator() {
  assert(getKind() == Kind::AsyncGenerator);
  return static_cast<c_AsyncGeneratorWaitHandle*>(this);
}

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_ASIO_ASYNC_GENERATOR_WAIT_HANDLE_H_
