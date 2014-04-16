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
  static constexpr ptrdiff_t offsetOff() {
    return offsetof(Resumable, m_offset);
  }

  static void* Create(const ActRec* fp, Offset offset, size_t objSize) {
    assert(fp);
    auto const func = fp->func();
    assert(func);
    assert(func->isAsync() || func->isGenerator());
    assert(func->contains(offset));

    // Allocate memory.
    size_t frameSize = func->numSlotsInFrame() * sizeof(TypedValue);
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
    resumable->m_offset = offset;
    resumable->m_size = totalSize;

    // Return pointer to the parent object.
    return resumable + 1;
  }

  ActRec* actRec() { return &m_actRec; }
  Offset offset() const {
    assert(m_actRec.func()->contains(m_offset));
    return m_offset;
  }
  size_t size() const { return m_size; }

  void setOffset(Offset offset) {
    assert(m_actRec.func()->contains(offset));
    m_offset = offset;
  }

private:
  static ptrdiff_t sizeForFunc(const Func* func) {
    assert(func->isAsync() || func->isGenerator());
    return sizeof(Iter) * func->numIterators() +
           sizeof(TypedValue) * func->numLocals() +
           sizeof(Resumable);
  }

  // ActRec of the resumed frame.
  ActRec m_actRec;

  // Temporary storage used to save the SP when inlining into a resumable.
  void* m_stashedSp;

  // Resume offset.
  Offset m_offset;

  // Size of the smart allocated memory that includes this resumable.
  int32_t m_size;
};

static_assert(Resumable::arOff() == 0,
              "ActRec must be in the beginning of Resumable");

//////////////////////////////////////////////////////////////////////

}

#endif
