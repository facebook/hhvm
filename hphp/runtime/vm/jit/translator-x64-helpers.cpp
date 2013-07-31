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
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/translator-x64.h"
#include "hphp/runtime/vm/jit/target-cache.h"
#include "hphp/runtime/base/file_repository.h"
#include "hphp/runtime/vm/event_hook.h"
#include "hphp/runtime/base/builtin_functions.h"
#include "hphp/runtime/base/stats.h"

#ifndef __APPLE__
#define ASM_HELPER_ALIGNMENT ".align 16\n"
#define ASM_HELPER_FUNC_NAME(name) #name
#else
#define ASM_HELPER_ALIGNMENT ".align 4\n" // on OSX this is 2^value
#define ASM_HELPER_FUNC_NAME(name) "_" #name
#endif

namespace HPHP {
namespace Transl {

inline bool
checkEval(HPHP::Eval::PhpFile* efile) {
  return !TargetCache::testAndSetBit(efile->getId());
}

/*
 * reserve the VM stack pointer within this translation unit
 *
 * Note that for rbp we use a local register asm
 * variable. But for rbx, we need to update it, and
 * local register asm vars are saved and restored just
 * like any other local. So I put this function in its own
 * file to avoid impacting the rest of translator-x64.cpp
 */
#if defined(__x86_64__) && !defined(__clang__)
register Cell* sp asm("rbx");
#else
// TODO(#2056140): we have to decide regalloc conventions for ARM
Cell* sp;
#endif

static void setupAfterPrologue(ActRec* fp) {
  g_vmContext->m_fp = fp;
  g_vmContext->m_stack.top() = sp;
  int nargs = fp->numArgs();
  int nparams = fp->m_func->numParams();
  Offset firstDVInitializer = InvalidAbsoluteOffset;
  if (nargs < nparams) {
    const Func::ParamInfoVec& paramInfo = fp->m_func->params();
    for (int i = nargs; i < nparams; ++i) {
      Offset dvInitializer = paramInfo[i].funcletOff();
      if (dvInitializer != InvalidAbsoluteOffset) {
        firstDVInitializer = dvInitializer;
        break;
      }
    }
  }
  if (firstDVInitializer != InvalidAbsoluteOffset) {
    g_vmContext->m_pc = fp->m_func->unit()->entry() + firstDVInitializer;
  } else {
    g_vmContext->m_pc = fp->m_func->getEntry();
  }
}

/*
 * fcallHelperThunk--
 *
 *   Func's prologue entries initially point here. Vector into fcallHelper,
 *   where we can translate a new prologue.
 */
static_assert(rStashedAR == reg::r15,
  "__fcallHelperThunk needs to be modified for ABI changes");
asm(
  ".byte 0\n"
  ASM_HELPER_ALIGNMENT
  ".globl __fcallHelperThunk\n"
"__fcallHelperThunk:\n"
#if defined(__x86_64__)
  // fcallHelper is used for prologues, and (in the case of
  // closures) for dispatch to the function body. In the first
  // case, there's a call, in the second, there's a jmp.
  // We can differentiate by comparing r15 and rVmFp
  "mov %r15, %rdi\n"
  "cmp %r15, %rbp\n"
  "jne 1f\n"
  "call " ASM_HELPER_FUNC_NAME(fcallHelper) "\n"
  "jmp *%rax\n"
  // fcallHelper may call doFCall. doFCall changes the return ip
  // pointed to by r15 so that it points to TranslatorX64::m_retHelper,
  // which does a REQ_POST_INTERP_RET service request. So we need to
  // to pop the return address into r15 + m_savedRip before calling
  // fcallHelper, and then push it back from r15 + m_savedRip after
  // fcallHelper returns in case it has changed it.
  "1: pop 0x8(%r15)\n"
  // There is a brief span from enterTCAtPrologue until the function
  // is entered where rbp is *below* the new actrec, and is missing
  // a number of c++ frames. The new actrec is linked onto the c++
  // frames, however, so switch it into rbp in case fcallHelper throws.
  "xchg %r15,%rbp\n"
  "call " ASM_HELPER_FUNC_NAME(fcallHelper) "\n"
  "xchg %r15,%rbp\n"
  "push 0x8(%r15)\n"
  "jmp *%rax\n"
  "ud2\n"
#elif defined(__AARCH64EL__)
  "brk 0\n"
#else
# error You sure have your work cut out for you
#endif
);

extern "C"
TCA fcallHelper(ActRec* ar) {
  try {
    TCA tca =
      Translator::Get()->funcPrologue((Func*)ar->m_func, ar->numArgs(), ar);
    if (tca) {
      return tca;
    }
    if (!ar->m_func->isClonedClosure()) {
      /*
       * If the func is a cloned closure, then the original
       * closure has already run the prologue, and the prologues
       * array is just being used as entry points for the
       * dv funclets. Dont run the prologue again.
       */
      VMRegAnchor _(ar);
      uint64_t rip = ar->m_savedRip;
      if (g_vmContext->doFCall(ar, g_vmContext->m_pc)) {
        ar->m_savedRip = rip;
        return Translator::Get()->getResumeHelperRet();
      }
      // We've been asked to skip the function body
      // (fb_intercept). frame, stack and pc have
      // already been fixed - so just ensure that
      // we setup the registers, and return as
      // if from the call to ar
      DECLARE_FRAME_POINTER(framePtr);
      framePtr->m_savedRip = rip;
      framePtr->m_savedRbp = (uint64_t)g_vmContext->m_fp;
      sp = g_vmContext->m_stack.top();
      return nullptr;
    }
    setupAfterPrologue(ar);
    assert(ar == g_vmContext->m_fp);
    return Translator::Get()->getResumeHelper();
  } catch (...) {
    /*
      The return address is set to __fcallHelperThunk,
      which has no unwind information. Its "logically"
      part of the tc, but the c++ unwinder wont know
      that. So point our return address at the called
      function's return address (which will be in the
      tc).
      Note that the registers really are clean - we
      cleaned them in the try above - so we just
      have to tell the unwinder that.
    */
    DECLARE_FRAME_POINTER(framePtr);
    tl_regState = VMRegState::CLEAN;
    framePtr->m_savedRip = ar->m_savedRip;
    throw;
  }
}

asm (
  ".byte 0\n"
  ASM_HELPER_ALIGNMENT
  ".globl __funcBodyHelperThunk\n"
"__funcBodyHelperThunk:\n"
#if defined(__x86_64__)
  /*
   * when this helper is called, its as if by a jmp
   * direct from the tc (its actually called by smashing
   * the return address of fCallArrayHelper). So we dont
   * need to worry about stack parity
   */
  "mov %rbp, %rdi\n"
  "call " ASM_HELPER_FUNC_NAME(funcBodyHelper) "\n"
  "jmp *%rax\n"
  "ud2\n"
#elif defined(__AARCH64EL__)
  "brk 0\n"
#else
# error You sure have your work cut out for you
#endif
);

/*
 * This is used to generate an entry point for the entry
 * to a function, after the prologue has run.
 */
TCA funcBodyHelper(ActRec* fp) {
  setupAfterPrologue(fp);
  tl_regState = VMRegState::CLEAN;
  Func* func = const_cast<Func*>(fp->m_func);

  TCA tca = tx64->getCallArrayPrologue(func);

  if (!tca) {
    tca = Translator::Get()->getResumeHelper();
  }
  tl_regState = VMRegState::DIRTY;
  return tca;
}

void TranslatorX64::fCallArrayHelper(const Offset pcOff, const Offset pcNext) {
  DECLARE_FRAME_POINTER(framePtr);
  ActRec* fp = (ActRec*)framePtr->m_savedRbp;

  VMExecutionContext *ec = g_vmContext;
  ec->m_fp = fp;
  ec->m_stack.top() = sp;
  ec->m_pc = fp->unit()->at(pcOff);
  PC pc = fp->unit()->at(pcNext);

  tl_regState = VMRegState::CLEAN;
  bool runFunc = ec->doFCallArray(pc);
  sp = ec->m_stack.top();
  tl_regState = VMRegState::DIRTY;
  if (!runFunc) return;

  ec->m_fp->m_savedRip = framePtr->m_savedRip;
  // smash our return and frame pointer chain
  framePtr->m_savedRip = (uint64_t)ec->m_fp->m_func->getFuncBody();
  framePtr->m_savedRbp = (uint64_t)ec->m_fp;
}

void functionEnterHelper(const ActRec* ar) {
  DECLARE_FRAME_POINTER(framePtr);

  uint64_t savedRip = ar->m_savedRip;
  uint64_t savedRbp = ar->m_savedRbp;
  if (LIKELY(EventHook::onFunctionEnter(ar, EventHook::NormalFunc))) return;
  /* We need to skip the function.
     FunctionEnter already cleaned up ar, and pushed the return value,
     so all we need to do is return to where ar would have returned to,
     with rbp set to ar's outer frame.
  */
  framePtr->m_savedRip = savedRip;
  framePtr->m_savedRbp = savedRbp;
  sp = g_vmContext->m_stack.top();
}

HOT_FUNC_VM
int64_t decodeCufIterHelper(Iter* it, TypedValue func) {
  DECLARE_FRAME_POINTER(framePtr);

  ObjectData* obj = nullptr;
  HPHP::Class* cls = nullptr;
  StringData* invName = nullptr;

  auto ar = (ActRec*)framePtr->m_savedRbp;
  if (LIKELY(ar->m_func->isBuiltin())) {
    ar = g_vmContext->getOuterVMFrame(ar);
  }
  const Func* f = vm_decode_function(tvAsVariant(&func),
                                     ar, false,
                                     obj, cls, invName,
                                     false);
  if (UNLIKELY(!f)) return false;
  CufIter &cit = it->cuf();
  cit.setFunc(f);
  if (obj) {
    cit.setCtx(obj);
    obj->incRefCount();
  } else {
    cit.setCtx(cls);
  }
  cit.setName(invName);
  return true;
}

} } // HPHP::Transl
