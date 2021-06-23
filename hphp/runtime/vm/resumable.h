/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/vm/act-rec.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/native-data.h"
#include "hphp/runtime/vm/vm-regs.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/util/alloc.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/**
 * Indicates how a function got resumed.
 */
enum class ResumeMode : uint8_t {
  // The function was regularly called and its frame is located on the stack.
  // This is the only valid mode for regular (non-async, non-generator)
  // functions. Async functions are executed in this mode (also called eager
  // execution) until reaching the first blocking await statement. Generators
  // and async generators run in this mode only during argument type enforcement
  // and suspend their execution immediately afterwards using the CreateCont
  // opcode.
  None = 0,

  // Execution of the function was resumed by the asio scheduler upon completion
  // of the awaited WaitHandle. Frame of the function is stored on the heap
  // colocated with the Resumable structure. Valid only for async functions and
  // async generators. An AsyncGeneratorWaitHandle is associated with async
  // generators.
  Async = 1,

  // Execution of the function was resumed by generator iteration (e.g. by
  // calling next() or send()). Frame of the function is stored on the heap
  // colocated with the Resumable structure. Valid only for generators and
  // async generators. Async generators are considered to be eagerly executed
  // in this mode and don't have any associated AsyncGeneratorWaitHandle.
  GenIter = 2,
};

char* resumeModeShortName(ResumeMode resumeMode);
Optional<ResumeMode> nameToResumeMode(const std::string& name);

ALWAYS_INLINE bool isResumed(const ActRec* ar) {
  assertx(ar && ar->func()->validate());
  // VM stack does not have resumed ActRecs.
  if (LIKELY(isValidVMStackAddress(ar))) return false;
  // Native stack may have fake ActRecs, which are not resumed.
  if (UNLIKELY(((uintptr_t)ar - s_stackLimit) < s_stackSize)) return false;
  // All ActRecs on the heap must be resumed.
  return true;
}

ResumeMode resumeModeFromActRecImpl(ActRec* ar);
ALWAYS_INLINE ResumeMode resumeModeFromActRec(ActRec* ar) {
  if (LIKELY(!isResumed(ar))) return ResumeMode::None;
  return resumeModeFromActRecImpl(ar);
}

/**
 * Header of the resumable frame used by async functions:
 *
 *     NativeNode* -> +--------------------------------+ low address
 *                    | kind=AsyncFuncFrame            |
 *                    +--------------------------------+
 *                    | Function locals and iterators  |
 *     Resumable*  -> +--------------------------------+
 *                    | ActRec in Resumable            |
 *                    +--------------------------------+
 *                    | Rest of Resumable              |
 *     ObjectData* -> +--------------------------------+
 *                    | c_AsyncFuncWaitHandle          |
 *                    +--------------------------------+ high address
 *
 * Header of the native frame used by generators:
 *
 *     NativeNode* -> +--------------------------------+ low address
 *                    | kind=NativeData                |
 *                    +--------------------------------+
 *                    | Function locals and iterators  |
 * BaseGenerator*  -> +--------------------------------+
 * < NativeData >     | ActRec in Resumable            |
 *                    +--------------------------------+
 *                    | Rest of Resumable              |
 *                    +--------------------------------+
 *                    | Rest of [Async]Generator       |
 *     ObjectData* -> +--------------------------------+
 *                    | Parent object                  |
 *                    +--------------------------------+ high address
 */
struct alignas(16) Resumable {
  // This function is used only by AFWH, temporary till AFWH is converted to HNI
  static Resumable* FromObj(ObjectData* obj) {
    return reinterpret_cast<Resumable*>(obj) - 1;
  }
  static const Resumable* FromObj(const ObjectData* obj) {
    return reinterpret_cast<const Resumable*>(obj) - 1;
  }
  static constexpr ptrdiff_t arOff() {
    return offsetof(Resumable, m_actRec);
  }
  static constexpr ptrdiff_t resumeAddrOff() {
    return offsetof(Resumable, m_resumeAddr);
  }
  static constexpr ptrdiff_t suspendOffsetOff() {
    return offsetof(Resumable, m_suspendOffset);
  }
  static constexpr ptrdiff_t dataOff() {
    return sizeof(Resumable);
  }
  static constexpr size_t getFrameSize(size_t numSlots) {
    return numSlots * sizeof(TypedValue);
  }

  // This function is temporary till we move AFWH to HNI
  static Resumable* Create(size_t frameSize, size_t totalSize) {
    // Allocate memory.
    (void)type_scan::getIndexForMalloc<ActRec>();
    auto node = new (tl_heap->objMalloc(totalSize))
                NativeNode(HeaderKind::AsyncFuncFrame,
                           sizeof(NativeNode) + frameSize + sizeof(Resumable));
    auto frame = reinterpret_cast<char*>(node + 1);
    return reinterpret_cast<Resumable*>(frame + frameSize);
  }

  template<bool clone>
  void initialize(const ActRec* fp, jit::TCA resumeAddr,
                  Offset suspendOffset, size_t frameSize, size_t totalSize) {
    assertx(fp);
    assertx(isResumed(fp) == clone);
    DEBUG_ONLY auto const func = fp->func();
    assertx(func);
    assertx(func->isResumable());
    assertx(func->contains(suspendOffset));
    // Check memory alignment
    assertx((((uintptr_t) actRec()) & (sizeof(TypedValue) - 1)) == 0);

    if (!clone) {
      // Copy ActRec, locals and iterators
      auto src = reinterpret_cast<const char*>(fp) - frameSize;
      auto dst = reinterpret_cast<char*>(actRec()) - frameSize;
      wordcpy(dst, src, frameSize + sizeof(ActRec));
    } else {
      // If we are cloning a Resumable, only copy the ActRec. The
      // caller will take care of copying locals, setting the VarEnv, etc.
      // When called from AFWH::Create or Generator::Create we know we are
      // going to overwrite m_sfp and m_savedRip, so don't copy them here.
      auto src = reinterpret_cast<const char*>(fp);
      auto dst = reinterpret_cast<char*>(actRec());
      wordcpy(dst + kNativeFrameSize,
              src + kNativeFrameSize,
              sizeof(ActRec) - kNativeFrameSize);
    }

    // Populate Resumable.
    m_resumeAddr = resumeAddr;
    m_offsetAndSize = (totalSize << 32 | suspendOffset);
  }

  template<class T> static void Destroy(size_t size, T* obj) {
    auto const base = reinterpret_cast<char*>(obj + 1) - size;
    obj->~T();
    tl_heap->objFree(base, size);
  }

  ActRec* actRec() { return &m_actRec; }
  const ActRec* actRec() const { return &m_actRec; }
  jit::TCA resumeAddr() const { return m_resumeAddr; }
  Offset suspendOffset() const {
    assertx(m_actRec.func()->contains(m_suspendOffset));
    return m_suspendOffset;
  }
  Offset resumeFromAwaitOffset() const {
    assertx(m_actRec.func()->contains(m_suspendOffset));
    auto const suspendPC = m_actRec.func()->at(m_suspendOffset);
    assertx(peek_op(suspendPC) == OpAwait || peek_op(suspendPC) == OpAwaitAll);
    auto const resumeOffset = m_suspendOffset + instrLen(suspendPC);
    assertx(m_actRec.func()->contains(resumeOffset));
    return resumeOffset;
  }
  Offset resumeFromYieldOffset() const {
    assertx(m_actRec.func()->contains(m_suspendOffset));
    // TODO(alexeyt) remove `yield from` and the need for this complexity
    auto const pc = m_actRec.func()->at(m_suspendOffset);
    DEBUG_ONLY auto const suspendedOp = peek_op(pc);
    assertx(suspendedOp == OpCreateCont ||
            suspendedOp == OpYield ||
            suspendedOp == OpYieldK);
    auto const resumeOffset = m_suspendOffset + instrLen(pc);
    assertx(m_actRec.func()->contains(resumeOffset));
    return resumeOffset;
  }
  size_t size() const { return m_size; }

  void setResumeAddr(jit::TCA resumeAddr, Offset suspendOffset) {
    assertx(m_actRec.func()->contains(suspendOffset));
    m_resumeAddr = resumeAddr;
    m_suspendOffset = suspendOffset;
  }

private:
  // ActRec of the resumed frame.
  ActRec m_actRec;

  // Resume address.
  jit::TCA m_resumeAddr;

  // Resume offset: bytecode offset from start of Unit's bytecode.
  union {
    struct {
      Offset m_suspendOffset;

      // Size of the memory block that includes this resumable.
      int32_t m_size;
    };
    uint64_t m_offsetAndSize;
  };
};

static_assert(Resumable::arOff() == 0,
              "ActRec must be in the beginning of Resumable");

//////////////////////////////////////////////////////////////////////

}
