/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/runtime/vm/jit/abi-arm.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/translator-inline.h"

#include "hphp/util/data-block.h"

namespace HPHP {

bool isVMFrame(const ActRec* ar) {
  assert(ar);
  // Determine whether the frame pointer is outside the native stack, cleverly
  // using a single unsigned comparison to do both halves of the bounds check.
  bool ret = uintptr_t(ar) - s_stackLimit >= s_stackSize;
  assert(!ret || isValidVMStackAddress(ar) ||
         (ar->m_func->validate(), ar->resumed()));
  return ret;
}

ActRec* callerFrameHelper() {
  DECLARE_FRAME_POINTER(frame);

  auto rbp = frame->m_sfp;
  while (true) {
    assertx(rbp && rbp != rbp->m_sfp && "Missing fixup for native call");
    if (isVMFrame(rbp)) {
      return rbp;
    }
    rbp = rbp->m_sfp;
  }
}

namespace jit {

//////////////////////////////////////////////////////////////////////

bool FixupMap::getFrameRegs(const ActRec* ar, VMRegs* outVMRegs) const {
  CTCA tca = (CTCA)ar->m_savedRip;

  auto ent = m_fixups.find(mcg->code().toOffset(tca));
  if (!ent) return false;

  // Note: If indirect fixups happen frequently enough, we could just compare
  // savedRip to be less than some threshold where stubs in a.code stop.
  if (ent->isIndirect()) {
    auto savedRIPAddr = reinterpret_cast<uintptr_t>(ar) +
                        ent->indirect.returnIpDisp;
    ent = m_fixups.find(
      mcg->code().toOffset(*reinterpret_cast<CTCA*>(savedRIPAddr))
    );
    assertx(ent && !ent->isIndirect());
  }

  // Non-obvious off-by-one fun: if the *return address* points into the TC,
  // then the frame we were running on in the TC is actually the previous
  // frame.
  ar = ar->m_sfp;

  regsFromActRec(tca, ar, ent->fixup, outVMRegs);
  return true;
}

void FixupMap::recordFixup(CTCA tca, const Fixup& fixup) {
  TRACE(3, "FixupMapImpl::recordFixup: tca %p -> (pcOff %d, spOff %d)\n",
        tca, fixup.pcOffset, fixup.spOffset);

  auto const offset = mcg->code().toOffset(tca);
  if (auto pos = m_fixups.find(offset)) {
    *pos = FixupEntry(fixup);
  } else {
    m_fixups.insert(offset, FixupEntry(fixup));
  }
}

const Fixup* FixupMap::findFixup(CTCA tca) const {
  auto ent = m_fixups.find(mcg->code().toOffset(tca));
  if (!ent) return nullptr;
  return &ent->fixup;
}

void FixupMap::fixupWork(ExecutionContext* ec, ActRec* nextRbp) const {
  assertx(RuntimeOption::EvalJit);

  TRACE(1, "fixup(begin):\n");

  while (true) {
    auto const rbp = nextRbp;
    nextRbp = rbp->m_sfp;
    assertx(nextRbp && nextRbp != rbp && "Missing fixup for native call");
    TRACE(2, "considering frame %p, %p\n", rbp, (void*)rbp->m_savedRip);

    if (isVMFrame(nextRbp)) {
      TRACE(2, "fixup checking vm frame %s\n",
               nextRbp->m_func->name()->data());
      VMRegs regs;
      if (getFrameRegs(rbp, &regs)) {
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
  }
}

void FixupMap::fixup(ExecutionContext* ec) const {
  // Start looking for fixup entries at the current (C++) frame.  This
  // will walk the frames upward until we find a TC frame.

  // In order to avoid tail call elimination optimization issues, grab the
  // parent frame pointer in order make sure this pointer is valid. The
  // fixupWork() looks for a TC frame, and we never call fixup() directly
  // from the TC, so skipping this frame isn't a problem.
  DECLARE_FRAME_POINTER(framePtr);
  auto fp = tl_regState >= VMRegState::GUARDED_THRESHOLD ?
    (ActRec*)tl_regState : framePtr->m_sfp;

  fixupWork(ec, fp);
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
    "thrift_protocol_read_binary",
    "thrift_protocol_read_binary_struct",
    "thrift_protocol_read_compact",
    "thrift_protocol_read_compact_struct",
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
