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
#include "hphp/runtime/vm/jit/abi-arm.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/event-hook.h"
#include "hphp/runtime/base/builtin-functions.h"

#include "hphp/vixl/a64/simulator-a64.h"

namespace HPHP {
namespace JIT {

static void setupAfterPrologue(ActRec* fp, void* sp) {
  g_context->m_fp = fp;
  g_context->m_stack.top() = (Cell*)sp;
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
    g_context->m_pc = fp->m_func->unit()->entry() + firstDVInitializer;
  } else {
    g_context->m_pc = fp->m_func->getEntry();
  }
}

TCA fcallHelper(ActRec* ar, void* sp) {
  try {
    assert(!ar->inGenerator());
    TCA tca =
      mcg->getFuncPrologue((Func*)ar->m_func, ar->numArgs(), ar);
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
      if (g_context->doFCall(ar, g_context->m_pc)) {
        ar->m_savedRip = rip;
        return tx->uniqueStubs.resumeHelperRet;
      }
      // We've been asked to skip the function body
      // (fb_intercept). frame, stack and pc have
      // already been fixed - flag that with a negative
      // return address.
      return (TCA)-rip;
    }
    setupAfterPrologue(ar, sp);
    assert(ar == g_context->m_fp);
    return tx->uniqueStubs.resumeHelper;
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
TCA funcBodyHelper(ActRec* fp, void* sp) {
  setupAfterPrologue(fp, sp);
  tl_regState = VMRegState::CLEAN;
  Func* func = const_cast<Func*>(fp->m_func);

  TCA tca = mcg->getCallArrayPrologue(func);

  if (!tca) {
    tca = tx->uniqueStubs.resumeHelper;
  }
  tl_regState = VMRegState::DIRTY;
  return tca;
}

int64_t decodeCufIterHelper(Iter* it, TypedValue func) {
  DECLARE_FRAME_POINTER(framePtr);

  ObjectData* obj = nullptr;
  HPHP::Class* cls = nullptr;
  StringData* invName = nullptr;

  auto ar = (ActRec*)framePtr->m_savedRbp;
  if (LIKELY(ar->m_func->isBuiltin())) {
    ar = g_context->getOuterVMFrame(ar);
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

} } // HPHP::JIT
