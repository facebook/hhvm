/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_TRANSLATOR_INLINE_H_
#define incl_HPHP_TRANSLATOR_INLINE_H_

#include "hphp/runtime/vm/jit/translator.h"
#include <boost/noncopyable.hpp>
#include "hphp/runtime/base/execution-context.h"

#define TVOFF(nm) offsetof(TypedValue, nm)
#define AROFF(nm) offsetof(ActRec, nm)

/*
 * Because of a circular dependence with ExecutionContext, these
 * translation-related helpers cannot live in translator.h.
 */
namespace HPHP   {

/*
 * Accessors for the virtual machine registers, both rvalues and
 * lvalues.
 *
 * Note that these do not assert anything about tl_regState; use
 * carefully.
 */
inline Cell*&  vmsp() { return (Cell*&)g_vmContext->m_stack.top(); }
inline Cell*&  vmfp() { return (Cell*&)g_vmContext->m_fp; }
inline const uchar*& vmpc() { return g_vmContext->m_pc; }
inline ActRec*& vmFirstAR() { return g_vmContext->m_firstAR; }

inline ActRec* liveFrame()    { return (ActRec*)vmfp(); }
inline const Func* liveFunc() { return liveFrame()->m_func; }
inline const Unit* liveUnit() { return liveFunc()->unit(); }
inline Class* liveClass() { return liveFunc()->cls(); }

inline Offset liveSpOff() {
  Cell* fp = vmfp();
  if (liveFunc()->isGenerator()) {
    fp = (Cell*)Stack::generatorStackBase((ActRec*)fp);
  }
  return fp - vmsp();
}

namespace JIT {

inline uintptr_t tlsBase() {
  uintptr_t retval;
#if defined(__x86_64__)
  asm ("movq %%fs:0, %0" : "=r" (retval));
#elif defined(__AARCH64EL__)
  // mrs == "move register <-- system"
  // tpidr_el0 == "thread process id register for exception level 0"
  asm ("mrs %0, tpidr_el0" : "=r" (retval));
#else
# error How do you access thread-local storage on this machine?
#endif
  return retval;
}

inline ptrdiff_t cellsToBytes(int nCells) {
  return nCells * sizeof(Cell);
}

inline int64_t localOffset(int64_t locId) {
  return -cellsToBytes(locId + 1);
}


struct VMRegAnchor : private boost::noncopyable {
  VMRegState m_old;
  VMRegAnchor() {
    Util::assert_native_stack_aligned();
    m_old = tl_regState;
    g_translator->sync();
  }
  explicit VMRegAnchor(ActRec* ar) {
    // Some C++ entry points have an ActRec prepared from after a call
    // instruction. This syncs us to right after the call instruction.
    assert(tl_regState == VMRegState::DIRTY);
    m_old = VMRegState::DIRTY;
    tl_regState = VMRegState::CLEAN;

    auto prevAr = g_vmContext->getOuterVMFrame(ar);
    const Func* prevF = prevAr->m_func;
    vmsp() = ar->m_func->isGenerator() ?
      Stack::generatorStackBase(ar) - 1 :
      (TypedValue*)ar - ar->numArgs();
    assert(g_vmContext->m_stack.isValidAddress((uintptr_t)vmsp()));
    vmpc() = prevF->unit()->at(prevF->base() + ar->m_soff);
    vmfp() = (TypedValue*)prevAr;
  }
  ~VMRegAnchor() {
    tl_regState = m_old;
  }
};

struct EagerVMRegAnchor {
  VMRegState m_old;
  EagerVMRegAnchor() {
    if (debug) {
      DEBUG_ONLY const Cell* fp = vmfp();
      DEBUG_ONLY const Cell* sp = vmsp();
      DEBUG_ONLY const uchar* pc = vmpc();
      VMRegAnchor _;
      assert(vmfp() == fp && vmsp() == sp && vmpc() == pc);
    }
    m_old = tl_regState;
    tl_regState = VMRegState::CLEAN;
  }
  ~EagerVMRegAnchor() {
    tl_regState = m_old;
  }
};

inline ActRec* regAnchorFP(Offset* pc = nullptr) {
  // In builtins, m_fp points to the caller's frame if called
  // through FCallBuiltin, else it points to the builtin's frame,
  // in which case, getPrevVMState() gets the caller's frame.
  // In addition, we need to skip over php-defined builtin functions
  // in order to find the true context.
  VMExecutionContext* context = g_vmContext;
  ActRec* cur = context->getFP();
  if (pc) *pc = cur->m_func->unit()->offsetOf(context->getPC());
  while (cur && cur->skipFrame()) {
    cur = context->getPrevVMState(cur, pc);
  }
  return cur;
}

inline ActRec* regAnchorFPForArgs() {
  // Like regAnchorFP, but only account for FCallBuiltin
  VMExecutionContext* context = g_vmContext;
  ActRec* cur = context->getFP();
  if (cur && cur->m_func->isCPPBuiltin()) {
    cur = context->getPrevVMState(cur);
  }
  return cur;
}

struct EagerCallerFrame : public EagerVMRegAnchor {
  ActRec* operator()() {
    return regAnchorFP();
  }
  ActRec* actRecForArgs() { return regAnchorFPForArgs(); }
};


// VM helper to retrieve the frame pointer from the TC. This is
// a common need for extensions.
struct CallerFrame : public VMRegAnchor {
  template<class... Args>
  ActRec* operator()(Args&&... args) {
    return regAnchorFP(std::forward<Args>(args)...);
  }
  ActRec* actRecForArgs() { return regAnchorFPForArgs(); }
};

#define SYNC_VM_REGS_SCOPED() \
  HPHP::JIT::VMRegAnchor _anchorUnused

} } // HPHP::JIT

#endif
