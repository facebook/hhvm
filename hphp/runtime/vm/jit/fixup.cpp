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
#include "hphp/runtime/vm/jit/fixup.h"

#include "hphp/vixl/a64/simulator-a64.h"

#include "hphp/runtime/vm/jit/abi-arm.h"
#include "hphp/runtime/vm/jit/translator-x64.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/util/data-block.h"

namespace HPHP {
namespace JIT {

bool
FixupMap::getFrameRegs(const ActRec* ar, const ActRec* prevAr,
                       VMRegs* outVMRegs) const {
  CTCA tca = (CTCA)ar->m_savedRip;
  // Non-obvious off-by-one fun: if the *return address* points into the TC,
  // then the frame we were running on in the TC is actually the previous
  // frame.
  ar = (const ActRec*)ar->m_savedRbp;
  auto* ent = m_fixups.find(tca);
  if (!ent) return false;
  if (ent->isIndirect()) {
    // Note: if indirect fixups happen frequently enough, we could
    // just compare savedRip to be less than some threshold where
    // stubs in a.code stop.
    assert(prevAr);
    auto pRealRip = ent->indirect.returnIpDisp +
      uintptr_t(prevAr->m_savedRbp);
    ent = m_fixups.find(*reinterpret_cast<CTCA*>(pRealRip));
    assert(ent && !ent->isIndirect());
  }
  regsFromActRec(tca, ar, ent->fixup, outVMRegs);
  return true;
}

void
FixupMap::recordSyncPoint(CodeAddress frontier, Offset pcOff, Offset spOff) {
  m_pendingFixups.push_back(PendingFixup(frontier, Fixup(pcOff, spOff)));
}

void
FixupMap::recordIndirectFixup(CodeAddress frontier, int dwordsPushed) {
  recordIndirectFixup(frontier, IndirectFixup((2 + dwordsPushed) * 8));
}

void
FixupMap::fixupWork(VMExecutionContext* ec, ActRec* rbp) const {
  assert(RuntimeOption::EvalJit);

  TRACE(1, "fixup(begin):\n");

  auto isVMFrame = [] (ActRec* ar) {
    // If this assert is failing, you may have forgotten a sync point somewhere
    assert(ar);
    bool ret = uintptr_t(ar) - Util::s_stackLimit >= Util::s_stackSize;
    assert(!ret ||
           (ar >= g_vmContext->m_stack.getStackLowAddress() &&
            ar < g_vmContext->m_stack.getStackHighAddress()) ||
           ar->m_func->isGenerator());
    return ret;
  };

  auto* nextRbp = rbp;
  rbp = 0;
  do {
    auto* prevRbp = rbp;
    rbp = nextRbp;
    assert(rbp && "Missing fixup for native call");
    nextRbp = reinterpret_cast<ActRec*>(rbp->m_savedRbp);
    TRACE(2, "considering frame %p, %p\n", rbp, (void*)rbp->m_savedRip);

    if (isVMFrame(nextRbp)) {
      TRACE(2, "fixup checking vm frame %s\n",
               nextRbp->m_func->name()->data());
      VMRegs regs;
      if (getFrameRegs(rbp, prevRbp, &regs)) {
        TRACE(2, "fixup(end): func %s fp %p sp %p pc %p\n",
              regs.m_fp->m_func->name()->data(),
              regs.m_fp, regs.m_sp, regs.m_pc);
        ec->m_fp = const_cast<ActRec*>(regs.m_fp);
        ec->m_pc = regs.m_pc;
        vmsp() = regs.m_sp;
        return;
      }
    }
  } while (rbp && rbp != nextRbp);

  // OK, we've exhausted the entire actRec chain.  We are only
  // invoking ::fixup() from contexts that were known to be called out
  // of the TC, so this cannot happen.
  always_assert(false);
}

void
FixupMap::fixupWorkSimulated(VMExecutionContext* ec) const {
  TRACE(1, "fixup(begin):\n");

  auto isVMFrame = [] (ActRec* ar, const vixl::Simulator* sim) {
    // If this assert is failing, you may have forgotten a sync point somewhere
    assert(ar);
    bool ret =
      uintptr_t(ar) - Util::s_stackLimit >= Util::s_stackSize &&
      !sim->is_on_stack(ar);
    assert(!ret ||
           (ar >= g_vmContext->m_stack.getStackLowAddress() &&
            ar < g_vmContext->m_stack.getStackHighAddress()) ||
           ar->m_func->isGenerator());
    return ret;
  };

  // For each nested simulator (corresponding to nested VM invocations), look at
  // its PC to find a potential fixup key.
  //
  // Callstack walking is necessary, because we may get called from a
  // uniqueStub.
  for (int i = ec->m_activeSims.size() - 1; i >= 0; --i) {
    auto const* sim = ec->m_activeSims[i];
    auto* rbp = reinterpret_cast<ActRec*>(sim->xreg(JIT::ARM::rVmFp.code()));
    auto tca = reinterpret_cast<TCA>(sim->pc());
    TRACE(2, "considering frame %p, %p\n", rbp, tca);

    while (rbp && !isVMFrame(rbp, sim)) {
      tca = reinterpret_cast<TCA>(rbp->m_savedRip);
      rbp = reinterpret_cast<ActRec*>(rbp->m_savedRbp);
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
          regs.m_fp->m_func->name()->data(),
          regs.m_fp, regs.m_sp, regs.m_pc);
    ec->m_fp = const_cast<ActRec*>(regs.m_fp);
    ec->m_pc = regs.m_pc;
    vmsp() = regs.m_sp;
    return;
  }

  // This shouldn't be reached.
  always_assert(false);
}

void
FixupMap::fixup(VMExecutionContext* ec) const {
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

void
FixupMap::processPendingFixups() {
  for (uint i = 0; i < m_pendingFixups.size(); i++) {
    TCA tca = m_pendingFixups[i].m_tca;
    assert(tx64->isValidCodeAddress(tca));
    recordFixup(tca, m_pendingFixups[i].m_fixup);
  }
  m_pendingFixups.clear();
}

/* This is somewhat hacky. It decides which helpers/builtins should
 * use eager vmreganchor based on profile information. Using eager
 * vmreganchor for all helper calls is a perf regression. */
bool
FixupMap::eagerRecord(const Func* func) {
  const char* list[] = {
    "func_get_args",
    "get_called_class",
    "func_num_args",
    "array_filter",
    "array_map",
    "hphp_func_slice_args",
  };

  for (int i = 0; i < sizeof(list)/sizeof(list[0]); i++) {
    if (!strcmp(func->name()->data(), list[i])) {
      return true;
    }
  }
  if (func->cls() && !strcmp(func->cls()->name()->data(), "WaitHandle")
      && !strcmp(func->name()->data(), "join")) {
    return true;
  }
  return false;
}

} // HPHP::JIT

} // HPHP
