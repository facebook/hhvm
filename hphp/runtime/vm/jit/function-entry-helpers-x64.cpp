/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/event-hook.h"
#include "hphp/runtime/base/builtin-functions.h"

#include "hphp/vixl/a64/simulator-a64.h"

namespace HPHP { namespace jit {

static void setupAfterPrologue(ActRec* fp, void* sp) {
  auto& regs = vmRegsUnsafe();
  regs.fp = fp;
  regs.stack.top() = (Cell*)sp;
  int nargs = fp->numArgs();
  int nparams = fp->m_func->numNonVariadicParams();
  Offset firstDVInitializer = InvalidAbsoluteOffset;
  if (nargs < nparams) {
    const Func::ParamInfoVec& paramInfo = fp->m_func->params();
    for (int i = nargs; i < nparams; ++i) {
      Offset dvInitializer = paramInfo[i].funcletOff;
      if (dvInitializer != InvalidAbsoluteOffset) {
        firstDVInitializer = dvInitializer;
        break;
      }
    }
  }
  if (firstDVInitializer != InvalidAbsoluteOffset) {
    regs.pc = fp->m_func->unit()->entry() + firstDVInitializer;
  } else {
    regs.pc = fp->m_func->getEntry();
  }
}

TCA fcallHelper(ActRec* ar) {
  assert_native_stack_aligned();
  try {
    assertx(!ar->resumed());
    auto const tca = mcg->getFuncPrologue(
      const_cast<Func*>(ar->m_func),
      ar->numArgs(),
      ar
    );
    if (tca) return tca;

    VMRegAnchor _(ar);
    if (doFCall(ar, vmpc())) {
      return mcg->tx().uniqueStubs.resumeHelperRet;
    }
    // We've been asked to skip the function body (fb_intercept).  The vmregs
    // have already been fixed; indicate this with a nullptr return.
    return nullptr;
  } catch (...) {
    // We cleaned the VM registers above, but we need to tell the unwinder.
    tl_regState = VMRegState::CLEAN;
    if (ar < vmfp()) {
      // We're in a really weird state; fcallHelper's frame points to
      // ar, but ar isn't active yet (it probably failed the stack
      // check).
      // Make it look like we were called directly from ar's caller
      DECLARE_FRAME_POINTER(framePtr);
      framePtr->m_sfp = ar->m_sfp;
      framePtr->m_savedRip = ar->m_savedRip;

      while (vmsp() < static_cast<void*>(ar)) {
        vmStack().popTV();
      }
      vmStack().popAR();
    }
    throw;
  }
}

/*
 * This is used to generate an entry point for the entry to a function, after
 * the prologue has run.
 */
TCA funcBodyHelper(ActRec* fp) {
  assert_native_stack_aligned();
  void* const sp = reinterpret_cast<Cell*>(fp) - fp->func()->numSlotsInFrame();
  setupAfterPrologue(fp, sp);
  tl_regState = VMRegState::CLEAN;

  auto const func = const_cast<Func*>(fp->m_func);
  auto tca = mcg->getFuncBody(func);
  if (!tca) {
    tca = mcg->tx().uniqueStubs.resumeHelper;
  }

  tl_regState = VMRegState::DIRTY;
  return tca;
}

int64_t decodeCufIterHelper(Iter* it, TypedValue func) {
  DECLARE_FRAME_POINTER(framePtr);

  ObjectData* obj = nullptr;
  HPHP::Class* cls = nullptr;
  StringData* invName = nullptr;

  auto ar = framePtr->m_sfp;
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

}}
