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
#include "hphp/runtime/base/execution_context.h"

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
static inline Cell*&  vmsp() { return (Cell*&)g_vmContext->m_stack.top(); }
static inline Cell*&  vmfp() { return (Cell*&)g_vmContext->m_fp; }
static inline const uchar*& vmpc() { return g_vmContext->m_pc; }
static inline ActRec*& vmFirstAR() { return g_vmContext->m_firstAR; }

static inline ActRec* curFrame()    { return (ActRec*)vmfp(); }
static inline const Func* curFunc() { return curFrame()->m_func; }
static inline const Unit* curUnit() { return curFunc()->unit(); }
static inline Class* curClass() {
  const auto* func = curFunc();
  auto* cls = func->cls();
  if (func->isPseudoMain() || func->isTraitMethod() || cls == nullptr) {
    return nullptr;
  }
  return cls;
}

namespace Transl {

static inline uintptr_t tlsBase() {
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

static inline int cellsToBytes(int nCells) {
  return nCells * sizeof(Cell);
}

static inline size_t bytesToCells(int nBytes) {
  assert(nBytes % sizeof(Cell) == 0);
  return nBytes / sizeof(Cell);
}

struct VMRegAnchor : private boost::noncopyable {
  VMRegState m_old;
  VMRegAnchor() {
    if (debug) {
      DEBUG_ONLY DECLARE_STACK_POINTER(sp);
      // native stack pointer should be octoword-aligned.
      assert((uintptr_t(sp) & 0xf) == 0);
    }
    m_old = tl_regState;
    Translator::Get()->sync();
  }
  explicit VMRegAnchor(ActRec* ar) {
    // Some C++ entry points have an ActRec prepared from after a call
    // instruction. This syncs us to right after the call instruction.
    assert(tl_regState == REGSTATE_DIRTY);
    m_old = REGSTATE_DIRTY;
    tl_regState = REGSTATE_CLEAN;

    auto prevAr = g_vmContext->getOuterVMFrame(ar);
    const Func* prevF = prevAr->m_func;
    vmsp() = ar->m_func->isGenerator() ?
      Stack::generatorStackBase(ar) :
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
    tl_regState = REGSTATE_CLEAN;
  }
  ~EagerVMRegAnchor() {
    tl_regState = m_old;
  }
};

inline ActRec* regAnchorFP() {
  // In builtins, m_fp points to the caller's frame if called
  // through FCallBuiltin, else it points to the builtin's frame,
  // in which case, getPrevVMState() gets the caller's frame.
  // In addition, we need to skip over php-defined builtin functions
  // in order to find the true context.
  VMExecutionContext* context = g_vmContext;
  ActRec* cur = context->getFP();
  while (cur && cur->skipFrame()) {
    cur = context->getPrevVMState(cur);
  }
  return cur;
}

inline ActRec* regAnchorFPForArgs() {
  // Like regAnchorFP, but only account for FCallBuiltin
  VMExecutionContext* context = g_vmContext;
  ActRec* cur = context->getFP();
  if (cur && cur->m_func->info()) {
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
  ActRec* operator()() {
    return regAnchorFP();
  }
  ActRec* actRecForArgs() { return regAnchorFPForArgs(); }
};

#define SYNC_VM_REGS_SCOPED() \
  HPHP::Transl::VMRegAnchor _anchorUnused

} } // HPHP::Transl

#endif
