/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include <cstdint>
#include <unwind.h>

#include "hphp/runtime/vm/jit/fixup.h"

#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/runtime/vm/jit/tc.h"

#include "hphp/util/configs/jit.h"
#include "hphp/util/dwarf-reg.h"

TRACE_SET_MOD(fixup)

namespace HPHP {

bool isVMFrame(const ActRec* ar, bool may_be_non_runtime) {
  assertx(ar);
  // Determine whether the frame pointer is outside the native stack, cleverly
  // using a single unsigned comparison to do both halves of the bounds check.
  auto const ret = uintptr_t(ar) - s_stackLimit >= s_stackSize;
  assertx(
    !ret ||
    may_be_non_runtime ||
    (ar->func()->validate(), true)
  );
  return ret;
}

//////////////////////////////////////////////////////////////////////

namespace jit {

ActRec* findVMFrameForDebug(uintptr_t start) {
  DECLARE_FRAME_POINTER(framePtr);
  auto rbp = start != 0 ? (ActRec*)start : framePtr;

  while (!isVMFrame(rbp, true)) {
    auto const nextRbp = rbp->m_sfp;
    if (nextRbp == nullptr || nextRbp == rbp) return nullptr;
    rbp = nextRbp;
  }

  return rbp;
}

std::string Fixup::show() const {
  if (!isValid()) return "invalid";
  if (isIndirect()) {
    return folly::sformat("indirect ripOff={} extraSpOff={}",
                          ripOffset(), spOffset().offset);
  } else if (isAsioStub()) {
    return folly::sformat("asio stub spOff={}", spOffset().offset);
  } else {
    return folly::sformat("direct pcOff={} spOff={}",
                          pcOffset(), spOffset().offset);
  }
}

//////////////////////////////////////////////////////////////////////

namespace FixupMap { namespace {

constexpr unsigned kInitCapac = 128;

struct VMRegs {
  PC pc;
  TypedValue* sp;
  const ActRec* fp;
  TCA retAddr;
};

struct FixupHash {
  size_t operator()(uint32_t k) const {
    return hash_int64(k);
  }
};

TreadHashMap<uint32_t,Fixup,FixupHash> s_fixups{kInitCapac};
static ServiceData::ExportedCounter* s_fixupmap_counter =
  ServiceData::createCounter("admin.fixup_map_size");

void regsFromActRec(TCA tca, const ActRec* ar, const Fixup& fixup,
                    SBInvOffset extraSpOffset, VMRegs* outRegs) {
  assertx(!fixup.isIndirect());
  const Func* f = ar->func();
  assertx(f);
  TRACE(3, "regsFromActRec: tca %p -> %s\n", tca, fixup.show().c_str());
  if (fixup.isAsioStub()) {
    auto const prevAR = g_context->getOuterVMFrame(ar);
    auto const prevFunc = prevAR->func();
    outRegs->pc = prevFunc->at(ar->callOffset());
    outRegs->fp = prevAR;
  } else {
    outRegs->pc = f->entry() + fixup.pcOffset();
    outRegs->fp = ar;
  }
  outRegs->retAddr = tca;

  auto const stackBase = Stack::anyFrameStackBase(ar);
  outRegs->sp = stackBase - fixup.spOffset().offset - extraSpOffset.offset;
}

//////////////////////////////////////////////////////////////////////

bool getFrameRegs(VMFrame frame, VMRegs* outVMRegs) {
  auto tca = frame.m_rip;
  auto fixup = s_fixups.find(tc::addrToOffset(tca));
  if (!fixup) return false;

  auto extraSpOffset = SBInvOffset{0};
  if (fixup->isIndirect()) {
    auto const ar = frame.m_prevCfa - kNativeFrameSize;
    TRACE(3, "getFrameRegs: fp %p -> %s\n", (void*) ar, fixup->show().c_str());
    auto const ripAddr = ar + fixup->ripOffset();
    tca = *reinterpret_cast<TCA*>(ripAddr);
    extraSpOffset = fixup->spOffset();
    fixup = s_fixups.find(tc::addrToOffset(tca));
    assertx(fixup && "Missing fixup for indirect fixup");
    assertx(!fixup->isIndirect() && "Invalid doubly indirect fixup");
  }

  // Non-obvious off-by-one fun: if the *return address* points into the TC,
  // then the frame we were running on in the TC is actually the previous
  // frame.
  auto const nextAR = frame.m_actRec;
  regsFromActRec(tca, nextAR, *fixup, extraSpOffset, outVMRegs);
  return true;
}

//////////////////////////////////////////////////////////////////////
}

void recordFixup(CTCA tca, const Fixup& fixup) {
  TRACE(3, "recordFixup: tca %p -> %s\n", tca, fixup.show().c_str());

  assertx(fixup.isValid());
  auto const offset = tc::addrToOffset(tca);
  if (auto pos = s_fixups.find(offset)) {
    *pos = fixup;
  } else {
    s_fixups.insert(offset, fixup);
    s_fixupmap_counter->increment();
  }
}

const Fixup* findFixup(CTCA tca) {
  auto const fixup = s_fixups.find(tc::addrToOffset(tca));
  return fixup ? &*fixup : nullptr;
}

size_t size() { return s_fixups.size(); }

bool processFixupForVMFrame(VMFrame frame) {
  assertx(isVMFrame(frame.m_actRec, false));

  VMRegs regs;
  if (!getFrameRegs(frame, &regs)) return false;

  TRACE(2, "fixup(end): func %s fp %p sp %p pc %p retAddr %p\n",
      regs.fp->func()->name()->data(),
      regs.fp, regs.sp, regs.pc, regs.retAddr);
  auto& vmRegs = vmRegsUnsafe();
  vmRegs.fp = const_cast<ActRec*>(regs.fp);
  vmRegs.pc = reinterpret_cast<PC>(regs.pc);
  vmRegs.stack.top() = regs.sp;
  vmRegs.jitReturnAddr = regs.retAddr;
  return true;
}

////////////////////////////////////////////////////////////////////////////////
}

namespace detail {
struct FixupWorkState {
  bool soft;
  bool synced;
};

/*
 * Perform a fixup of the VM registers for the current stack.
 */
_Unwind_Reason_Code fixupWork(_Unwind_Context* context, void* arg) {
  auto state = static_cast<FixupWorkState*>(arg);
  auto const fp = reinterpret_cast<ActRec*>(_Unwind_GetGR(context, dw_reg::FP));
  
  TRACE(2, "considering frame %p, %p\n", fp, (void*)fp->m_savedRip);

  if (isVMFrame(fp, state->soft)) {
      const uintptr_t cfa = _Unwind_GetCFA(context);
      const uintptr_t rip = _Unwind_GetIP(context);
      auto frame = VMFrame{fp, TCA(rip), cfa};
      state->synced = FixupMap::processFixupForVMFrame(frame);
      if (state->synced || LIKELY(state->soft)) {
        return _URC_END_OF_STACK;
      }
      always_assert(false && "Fixup expected for leafmost VM frame");
  }
  return _URC_NO_REASON;
}

void syncVMRegsWork(bool soft) {
  assertx(Cfg::Jit::Enabled);

  TRACE(1, "fixup(begin):\n");

  // Start looking for fixup entries at the current (C++) frame.  This
  // will walk the frames upward until we find a TC frame.
  FixupWorkState state{soft, false};
  _Unwind_Backtrace(fixupWork, &state);

  assertx((state.synced || state.soft) && "Missing fixup for native call");

  if (state.synced) regState() = VMRegState::CLEAN;
  Stats::inc(Stats::TC_Sync);
}
}

static std::atomic<int32_t> s_nextFakeAddress{-1};

int32_t getNextFakeReturnAddress() {
  return s_nextFakeAddress.fetch_sub(1, std::memory_order_acq_rel);
}

}}
