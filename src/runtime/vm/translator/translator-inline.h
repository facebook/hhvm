/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifndef incl_TRANSLATOR_INLINE_H_
#define incl_TRANSLATOR_INLINE_H_

#include "translator.h"
#include <boost/noncopyable.hpp>
#include <runtime/base/execution_context.h>

#define TVOFF(nm) offsetof(TypedValue, nm)
#define AROFF(nm) offsetof(ActRec, nm)

/*
 * Because of a circular dependence with ExecutionContext, these
 * translation-related helpers cannot live in translator.h.
 */
namespace HPHP   {
namespace VM     {
namespace Transl {

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
  const Func* func = curFunc();
  Class* clss = func->cls();
  if (func->isPseudoMain() || func->isTraitMethod() || clss == NULL) {
    return NULL;
  }
  return clss;
}

static inline uintptr_t tlsBase() {
  uintptr_t retval;
  asm ("movq %%fs:0, %0" : "=r" (retval));
  return retval;
}

static inline int cellsToBytes(int nCells) {
  return nCells * sizeof(Cell);
}

static inline size_t bytesToCells(int nBytes) {
  ASSERT(nBytes % sizeof(Cell) == 0);
  return nBytes / sizeof(Cell);
}

struct VMRegAnchor : private boost::noncopyable {
  VMRegState m_old;
  VMRegAnchor() {
    if (debug) {
      uint64_t sp;
      asm volatile("movq %%rsp, %0" : "=r"(sp) ::);
      // rsp should be octoword-aligned.
      ASSERT((sp & 0xf) == 0);
    }
    m_old = tl_regState;
    Translator::Get()->sync();
  }
  VMRegAnchor(ActRec* ar, bool atFCall=false) {
    // Some C++ entry points have an ActRec prepared from after a call
    // instruction. This syncs us to right after the call instruction.
    ASSERT(tl_regState == REGSTATE_DIRTY);
    m_old = REGSTATE_DIRTY;
    tl_regState = REGSTATE_CLEAN;
    int numArgs = ar->numArgs();

    const Func* prevF = ((ActRec*)(ar->m_savedRbp))->m_func;
    vmsp() = (TypedValue*)ar - numArgs;
    ASSERT(g_vmContext->m_stack.isValidAddress((uintptr_t)vmsp()));
    vmpc() = prevF->unit()->at(prevF->base() + ar->m_soff);
    if (atFCall) {
      // VMExecutionContext::doFCall expects vmfp to be the caller's
      // ActRec, but if we call VMRegAnchor while executing the FCall
      // sequence (in TargetCache::callAndResume), we actually have the
      // callee's ActRec.
      vmfp() = (TypedValue*)ar->m_savedRbp;
    } else {
      vmfp() = (TypedValue*)ar;
    }
  }
  ~VMRegAnchor() {
    tl_regState = m_old;
  }
};

// VM helper to retrieve the frame pointer from the TC. This is
// a common need for extensions.
struct CallerFrame : public VMRegAnchor {
  ActRec* operator()() {
    // In builtins, m_fp points to the caller's frame if called
    // through FCallBuiltin, else it points to the builtin's frame,
    // in which case, getPrevVMState() gets the caller's frame.
    VMExecutionContext* context = g_vmContext;
    ActRec* cur = context->getFP();
    if (!cur) return NULL;
    if (cur->skipFrame()) {
      ActRec* prev = context->getPrevVMState(cur);
      if (prev == cur) return NULL;
      return prev;
    } else {
      return cur;
    }
  }
};

#ifdef HHVM
#define SYNC_VM_REGS_SCOPED() \
  HPHP::VM::Transl::VMRegAnchor _anchorUnused
#else
#define SYNC_VM_REGS_SCOPED() \
  do {} while(0)
#endif

} } } // HPHP::VM::Transl

#endif
