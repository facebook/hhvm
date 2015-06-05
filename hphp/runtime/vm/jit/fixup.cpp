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
#include "hphp/runtime/vm/jit/fixup.h"

#include "hphp/vixl/a64/simulator-a64.h"

#include "hphp/runtime/vm/vm-regs.h"
#include "hphp/runtime/vm/jit/abi-arm.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/util/data-block.h"

namespace HPHP { namespace jit {

//////////////////////////////////////////////////////////////////////

bool FixupMap::getFrameRegs(const ActRec* ar,
                            const ActRec* prevAr,
                            VMRegs* outVMRegs) const {
  CTCA tca = (CTCA)ar->m_savedRip;
  // Non-obvious off-by-one fun: if the *return address* points into the TC,
  // then the frame we were running on in the TC is actually the previous
  // frame.
  ar = ar->m_sfp;
  auto* ent = m_fixups.find(tca);
  if (!ent) return false;
  if (ent->isIndirect()) {
    // Note: if indirect fixups happen frequently enough, we could
    // just compare savedRip to be less than some threshold where
    // stubs in a.code stop.
    assertx(prevAr);
    auto pRealRip = ent->indirect.returnIpDisp +
      uintptr_t(prevAr->m_sfp);
    ent = m_fixups.find(*reinterpret_cast<CTCA*>(pRealRip));
    assertx(ent && !ent->isIndirect());
  }
  regsFromActRec(tca, ar, ent->fixup, outVMRegs);
  return true;
}

void FixupMap::fixupWork(ExecutionContext* ec, ActRec* rbp) const {
  assertx(RuntimeOption::EvalJit);

  TRACE(1, "fixup(begin):\n");

  auto* nextRbp = rbp;
  rbp = 0;
  do {
    auto* prevRbp = rbp;
    rbp = nextRbp;
    assertx(rbp && "Missing fixup for native call");
    nextRbp = rbp->m_sfp;
    TRACE(2, "considering frame %p, %p\n", rbp, (void*)rbp->m_savedRip);

    if (isVMFrame(nextRbp)) {
      TRACE(2, "fixup checking vm frame %s\n",
               nextRbp->m_func->name()->data());
      VMRegs regs;
      if (getFrameRegs(rbp, prevRbp, &regs)) {
        TRACE(2, "fixup(end): func %s fp %p sp %p pc %p\n",
              regs.fp->m_func->name()->data(),
              regs.fp, regs.sp, regs.pc);
        auto& vmRegs = vmRegsUnsafe();
        vmRegs.fp = const_cast<ActRec*>(regs.fp);
        vmRegs.pc = reinterpret_cast<PC>(regs.pc);
        vmRegs.stack.top() = regs.sp;
        return;
      }
    }
  } while (rbp && rbp != nextRbp);

  // OK, we've exhausted the entire actRec chain.  We are only
  // invoking ::fixup() from contexts that were known to be called out
  // of the TC, so this cannot happen.
  always_assert(false);
}

void FixupMap::fixupWorkSimulated(ExecutionContext* ec) const {
  TRACE(1, "fixup(begin):\n");

  auto isVMFrame = [] (ActRec* ar, const vixl::Simulator* sim) {
    // If this assert is failing, you may have forgotten a sync point somewhere
    assertx(ar);
    bool ret =
      uintptr_t(ar) - s_stackLimit >= s_stackSize &&
      !sim->is_on_stack(ar);
    assertx(!ret || isValidVMStackAddress(ar) || ar->resumed());
    return ret;
  };

  // For each nested simulator (corresponding to nested VM invocations), look at
  // its PC to find a potential fixup key.
  //
  // Callstack walking is necessary, because we may get called from a
  // uniqueStub.
  for (int i = ec->m_activeSims.size() - 1; i >= 0; --i) {
    auto const* sim = ec->m_activeSims[i];
    auto* rbp = reinterpret_cast<ActRec*>(sim->xreg(jit::arm::rVmFp.code()));
    auto tca = reinterpret_cast<TCA>(sim->pc());
    TRACE(2, "considering frame %p, %p\n", rbp, tca);

    while (rbp && !isVMFrame(rbp, sim)) {
      tca = reinterpret_cast<TCA>(rbp->m_savedRip);
      rbp = rbp->m_sfp;
    }

    if (!rbp) continue;

    auto* ent = m_fixups.find(tca);
    if (!ent) {
      continue;
    }

    if (ent->isIndirect()) {
      not_implemented();
    }

    VMRegs regs;
    regsFromActRec(tca, rbp, ent->fixup, &regs);
    TRACE(2, "fixup(end): func %s fp %p sp %p pc %p\b",
          regs.fp->m_func->name()->data(),
          regs.fp, regs.sp, regs.pc);
    auto& vmRegs = vmRegsUnsafe();
    vmRegs.fp = const_cast<ActRec*>(regs.fp);
    vmRegs.pc = reinterpret_cast<PC>(regs.pc);
    vmRegs.stack.top() = regs.sp;
    return;
  }

  // This shouldn't be reached.
  always_assert(false);
}

void FixupMap::fixup(ExecutionContext* ec) const {
  if (RuntimeOption::EvalSimulateARM) {
    // Walking the C++ stack doesn't work in simulation mode. Fortunately, the
    // execution context has a stack of simulators, which we consult instead.
    fixupWorkSimulated(ec);
  } else {
    // Start looking for fixup entries at the current (C++) frame.  This
    // will walk the frames upward until we find a TC frame.
    DECLARE_FRAME_POINTER(framePtr);
    fixupWork(ec, framePtr);
  }
}

/* This is somewhat hacky. It decides which helpers/builtins should
 * use eager vmreganchor based on profile information. Using eager
 * vmreganchor for all helper calls is a perf regression. */
bool FixupMap::eagerRecord(const Func* func) {
  const char* list[] = {
    "func_get_args",
    "__SystemLib\\func_get_args_sl",
    "get_called_class",
    "func_num_args",
    "__SystemLib\\func_num_arg_",
    "array_filter",
    "array_map",
    "__SystemLib\\func_slice_args",
  };

  for (auto str : list) {
    if (!strcmp(func->name()->data(), str)) return true;
  }

  return func->cls() &&
    !strcmp(func->cls()->name()->data(), "HH\\WaitHandle") &&
    !strcmp(func->name()->data(), "join");
}

//////////////////////////////////////////////////////////////////////

}}
