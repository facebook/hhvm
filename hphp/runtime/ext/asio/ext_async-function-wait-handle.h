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

#ifndef incl_HPHP_EXT_ASIO_ASYNC_FUNCTION_WAIT_HANDLE_H_
#define incl_HPHP_EXT_ASIO_ASYNC_FUNCTION_WAIT_HANDLE_H_

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/asio/ext_resumable-wait-handle.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/resumable.h"
#include "hphp/runtime/vm/srckey.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// Tail frames embedded in AFWH

// NOTE: We use the physical values of these IDs in several places. A diagram
// will help a bit. We use 16 bits for an AsyncFrameId and store up to 4 IDs
// in a packed 64-bit quadword. Consider this field as four 16-bit words:
//
// m_packedTailFrameIds: (higher) ..w3..|..w2..|..w1..|..w0.. (lower)
//       m_tailFrameIds: (higher) .tf0..|.tf1..|.tf2..|.tf3.. (lower)
//
// Pushing tail frames into this list is the hot path; we only need to read it
// if the Awaitable results in an exception, which is rare. We can JIT faster
// code for the push operation if we push the last frame into the lowest bits,
// using bit operations - just a shift and an or. That's why the tail frame
// indices are reversed from the physical layout.
//
// As a example, if we've pushed one tail frames into this list, it'll be in
// tf3 == w0. When we push another frame, that first ID will be in tf2 == w1,
// and the second one in tf3 == w0. Any unused IDs will equal the invalid ID.
//
// We place the following on the valid and invalid ranges of these IDs:
//   1. The invalid ID is 0xffff - that is, it's all 1s in binary.
//   2. Valid IDs are in the range [1, 0x7fff] (both ends inclusive).
//
// Using these restrictions, we can initialize these IDs to all-invalid by
// setting the quadword to -1. We can test "does an AFWH have any tail frames?"
// by checking that w0 is not the invalid ID. We can also test "does an AFWH
// have room for more tail frames?" by checking that w3 is the invalid ID.
//
using AsyncFrameId = uint16_t;
constexpr AsyncFrameId kInvalidAsyncFrameId =
  std::numeric_limits<AsyncFrameId>::max();
constexpr AsyncFrameId kMaxAsyncFrameId = kInvalidAsyncFrameId >> 1;

// Will return kInvalidAsyncFrameId if we've run out of IDs.
AsyncFrameId getAsyncFrameId(SrcKey sk);
SrcKey getAsyncFrame(AsyncFrameId id);

///////////////////////////////////////////////////////////////////////////////
// class AsyncFunctionWaitHandle

/**
 * An async function wait handle represents a basic unit of asynchronous
 * execution. A dependency on another wait handle is set up by awaiting such
 * wait handle, giving control of the execution back to the asio framework.
 */
struct c_AsyncFunctionWaitHandle final :
    c_ResumableWaitHandle,
    SystemLib::ClassLoader<"HH\\AsyncFunctionWaitHandle"> {
  using SystemLib::ClassLoader<"HH\\AsyncFunctionWaitHandle">::classof;
  using SystemLib::ClassLoader<"HH\\AsyncFunctionWaitHandle">::className;

  static void instanceDtor(ObjectData* obj, const Class*) {
    auto wh = wait_handle<c_AsyncFunctionWaitHandle>(obj);
    Resumable::Destroy(wh->resumable()->size(), wh);
  }

  struct Node final {
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
    c_WaitableWaitHandle* m_child;
    AsioBlockable m_blockable;
  };

  explicit c_AsyncFunctionWaitHandle() noexcept
    : c_ResumableWaitHandle(classof(), HeaderKind::AsyncFuncWH,
                    type_scan::getIndexForMalloc<c_AsyncFunctionWaitHandle>())
  {}
  ~c_AsyncFunctionWaitHandle();

 public:
  static constexpr ptrdiff_t resumableOff() { return -sizeof(Resumable); }
  static constexpr ptrdiff_t arOff() {
    return resumableOff() + Resumable::arOff();
  }
  static constexpr ptrdiff_t resumeAddrOff() {
    return resumableOff() + Resumable::resumeAddrOff();
  }
  static constexpr ptrdiff_t childrenOff() {
    return offsetof(c_AsyncFunctionWaitHandle, m_children);
  }
  static constexpr ptrdiff_t tailFramesOff() {
    return offsetof(c_AsyncFunctionWaitHandle, m_tailFrameIds);
  }
  static c_AsyncFunctionWaitHandle* Create(
    const ActRec* origFp,
    size_t numSlots,
    jit::TCA resumeAddr,
    Offset suspendOffset,
    c_WaitableWaitHandle* child
  ); // nothrow
  static void PrepareChild(const ActRec* fp, c_WaitableWaitHandle* child);
  void onUnblocked();
  void resume();
  void await(Offset suspendOffset, req::ptr<c_WaitableWaitHandle>&& child);
  void ret(TypedValue& result);
  void fail(ObjectData* exception);
  void failCpp();
  String getName();
  c_WaitableWaitHandle* getChild();
  void exitContext(context_idx_t ctx_idx);
  bool isRunning() { return getState() == STATE_RUNNING; }
  String getFilename();
  Offset getNextExecutionOffset();

  Resumable* resumable() const {
    return reinterpret_cast<Resumable*>(
      const_cast<char*>(reinterpret_cast<const char*>(this) + resumableOff()));
  }

  ActRec* actRec() const {
    return resumable()->actRec();
  }

  bool isFastResumable() const {
    assertx(getState() == STATE_READY);
    return (resumable()->resumeAddr() &&
            m_children[0].getChild()->isSucceeded());
  }

  // Access to merged tail frames. We optimize hard for writing these tail
  // frames quickly, so reading them is a bit awkward - try to avoid doing so.
  bool hasTailFrames() const;
  size_t firstTailFrameIndex() const;
  size_t lastTailFrameIndex() const;
  AsyncFrameId tailFrame(size_t index) const;
  static constexpr size_t kNumTailFrames = 4;

 private:
  void setState(uint8_t state) { setKindState(Kind::AsyncFunction, state); }
  void initialize(c_WaitableWaitHandle* child);
  void prepareChild(c_WaitableWaitHandle* child);

  // valid if STATE_BLOCKED || STATE_READY. For now, always 1 element.
  // May become a flexible array later.
  Node m_children[1];

  union {
    AsyncFrameId m_tailFrameIds[kNumTailFrames];
    uint64_t m_packedTailFrameIds;
  };

  TYPE_SCAN_CUSTOM_FIELD(m_children) {
    auto state = getState();
    if (state == STATE_BLOCKED || state == STATE_READY) {
      scanner.scan(m_children[0]);
    }
  }
};

inline c_AsyncFunctionWaitHandle* c_Awaitable::asAsyncFunction() {
  assertx(getKind() == Kind::AsyncFunction);
  return static_cast<c_AsyncFunctionWaitHandle*>(this);
}

///////////////////////////////////////////////////////////////////////////////
}

#include "hphp/runtime/ext/asio/ext_async-function-wait-handle-inl.h"

#endif // incl_HPHP_EXT_ASIO_ASYNC_FUNCTION_WAIT_HANDLE_H_
