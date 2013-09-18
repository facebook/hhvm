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
#include "hphp/runtime/vm/jit/abi-arm.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/translator-x64.h"
#include "hphp/runtime/vm/event-hook.h"
#include "hphp/runtime/base/builtin-functions.h"

#include "hphp/vixl/a64/simulator-a64.h"

namespace HPHP {
namespace Transl {

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

namespace {

inline Cell* hardwareStackPtr() {
  if (RuntimeOption::EvalSimulateARM) {
    return reinterpret_cast<Cell*>(
      g_vmContext->m_activeSims.back()->xreg(JIT::ARM::rVmSp.code())
    );
  }
  return sp;
}

inline void setHardwareStackPtr(Cell* newSp) {
  if (RuntimeOption::EvalSimulateARM) {
    g_vmContext->m_activeSims.back()->set_xreg(JIT::ARM::rVmSp.code(), newSp);
    return;
  }
  sp = newSp;
}

}

static void setupAfterPrologue(ActRec* fp) {
  g_vmContext->m_fp = fp;
  g_vmContext->m_stack.top() = hardwareStackPtr();
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
        return Translator::Get()->uniqueStubs.resumeHelperRet;
      }
      // We've been asked to skip the function body
      // (fb_intercept). frame, stack and pc have
      // already been fixed - so just ensure that
      // we setup the registers, and return as
      // if from the call to ar
      DECLARE_FRAME_POINTER(framePtr);
      framePtr->m_savedRip = rip;
      framePtr->m_savedRbp = (uint64_t)g_vmContext->m_fp;
      setHardwareStackPtr(g_vmContext->m_stack.top());
      return nullptr;
    }
    setupAfterPrologue(ar);
    assert(ar == g_vmContext->m_fp);
    return Translator::Get()->uniqueStubs.resumeHelper;
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
    tca = Translator::Get()->uniqueStubs.resumeHelper;
  }
  tl_regState = VMRegState::DIRTY;
  return tca;
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
  setHardwareStackPtr(g_vmContext->m_stack.top());
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
