/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_RUNTIME_VM_RESUMABLE_H_
#define incl_HPHP_RUNTIME_VM_RESUMABLE_H_

#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/jit/types.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/**
 * Header of the resumable frame used by async functions and generators.
 *
 * The memory is lay out as follows:
 *
 *                +-------------------------+ high address
 *                |      Parent object      |
 *                +-------------------------+
 *                |    Rest of Resumable    |
 *                +-------------------------+
 *                |   ActRec in Resumable   |
 *                +-------------------------+
 *                |   Function locals and   |
 *                |   iterators             |
 * malloc ptr ->  +-------------------------+ low address
 */
struct Resumable {
  static constexpr ptrdiff_t arOff() {
    return offsetof(Resumable, m_actRec);
  }
  static constexpr ptrdiff_t stashedSpOff() {
    return offsetof(Resumable, m_stashedSp);
  }
  static constexpr ptrdiff_t resumeAddrOff() {
    return offsetof(Resumable, m_resumeAddr);
  }
  static constexpr ptrdiff_t resumeOffsetOff() {
    return offsetof(Resumable, m_resumeOffset);
  }

  static void* Create(const ActRec* fp, size_t numSlots, JIT::TCA resumeAddr,
                      Offset resumeOffset, size_t objSize) {
    assert(fp);
    auto const func = fp->func();
    assert(func);
    assert(func->isResumable());
    assert(func->contains(resumeOffset));

    // Allocate memory.
    size_t frameSize = numSlots * sizeof(TypedValue);
    size_t totalSize = frameSize + sizeof(Resumable) + objSize;
    void* mem = MM().objMallocLogged(totalSize);
    auto resumable = (Resumable*)((char*)mem + frameSize);

    // Populate ActRec.
    auto& actRec = resumable->m_actRec;
    actRec.m_func = func;
    actRec.initNumArgsFromResumable(fp->numArgs());
    actRec.setVarEnv(nullptr);
    actRec.setThisOrClassAllowNull(fp->getThisOrClass());

    // Populate Resumable.
    resumable->m_resumeAddr = resumeAddr;
    resumable->m_resumeOffset = resumeOffset;
    resumable->m_size = totalSize;

    // Return pointer to the parent object.
    return resumable + 1;
  }

  ActRec* actRec() { return &m_actRec; }
  JIT::TCA resumeAddr() const { return m_resumeAddr; }
  Offset resumeOffset() const {
    assert(m_actRec.func()->contains(m_resumeOffset));
    return m_resumeOffset;
  }
  size_t size() const { return m_size; }

  void setResumeAddr(JIT::TCA resumeAddr, Offset resumeOffset) {
    assert(m_actRec.func()->contains(resumeOffset));
    m_resumeAddr = resumeAddr;
    m_resumeOffset = resumeOffset;
  }

private:
  static ptrdiff_t sizeForFunc(const Func* func) {
    assert(func->isResumable());
    return sizeof(Iter) * func->numIterators() +
           sizeof(TypedValue) * func->numLocals() +
           sizeof(Resumable);
  }

  // ActRec of the resumed frame.
  ActRec m_actRec;

  // Temporary storage used to save the SP when inlining into a resumable. This
  // is used in an offsetof expression above, but clang doesn't recognize that
  // as a "use", hence the UNUSED.
  UNUSED void* m_stashedSp;

  // Resume address.
  JIT::TCA m_resumeAddr;

  // Resume offset.
  Offset m_resumeOffset;

  // Size of the smart allocated memory that includes this resumable.
  int32_t m_size;
} __attribute__((aligned(16)));

static_assert(Resumable::arOff() == 0,
              "ActRec must be in the beginning of Resumable");

//////////////////////////////////////////////////////////////////////

}

#endif
