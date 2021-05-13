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

#include "hphp/runtime/vm/jit/fixup.h"

#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/vm/resumable.h"
#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/runtime/vm/jit/abi-arm.h"
#include "hphp/runtime/vm/jit/code-cache.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/translator-inline.h"

#include "hphp/util/data-block.h"

TRACE_SET_MOD(fixup);

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

ActRec* findVMFrameForDebug() {
  DECLARE_FRAME_POINTER(framePtr);
  auto rbp = (ActRec*) framePtr;

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

void regsFromActRec(TCA tca, const ActRec* ar, const Fixup& fixup,
                    SBInvOffset extraSpOffset, VMRegs* outRegs) {
  assertx(!fixup.isIndirect());
  const Func* f = ar->func();
  assertx(f);
  TRACE(3, "regsFromActRec: tca %p -> %s\n", tca, fixup.show().c_str());
  outRegs->pc = f->entry() + fixup.pcOffset();
  outRegs->fp = ar;
  outRegs->retAddr = tca;

  auto const stackBase = Stack::anyFrameStackBase(ar);
  outRegs->sp = stackBase - fixup.spOffset().offset - extraSpOffset.offset;
}

//////////////////////////////////////////////////////////////////////

bool getFrameRegs(const ActRec* ar, VMRegs* outVMRegs) {
  auto tca = (TCA)ar->m_savedRip;
  auto fixup = s_fixups.find(tc::addrToOffset(tca));
  if (!fixup) return false;

  auto extraSpOffset = SBInvOffset{0};
  if (fixup->isIndirect()) {
    TRACE(3, "getFrameRegs: fp %p -> %s\n", ar, fixup->show().c_str());
    auto const ripAddr = reinterpret_cast<uintptr_t>(ar) + fixup->ripOffset();
    tca = *reinterpret_cast<TCA*>(ripAddr);
    extraSpOffset = fixup->spOffset();
    fixup = s_fixups.find(tc::addrToOffset(tca));
    assertx(fixup && "Missing fixup for indirect fixup");
    assertx(!fixup->isIndirect() && "Invalid doubly indirect fixup");
  }

  // Non-obvious off-by-one fun: if the *return address* points into the TC,
  // then the frame we were running on in the TC is actually the previous
  // frame.
  ar = ar->m_sfp;

  regsFromActRec(tca, ar, *fixup, extraSpOffset, outVMRegs);
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
  }
}

const Fixup* findFixup(CTCA tca) {
  auto const fixup = s_fixups.find(tc::addrToOffset(tca));
  return fixup ? &*fixup : nullptr;
}

size_t size() { return s_fixups.size(); }

bool fixupWork(ActRec* nextRbp, bool soft) {
  assertx(RuntimeOption::EvalJit);

  TRACE(1, "fixup(begin):\n");

  while (true) {
    auto const rbp = nextRbp;
    nextRbp = rbp->m_sfp;

    if (UNLIKELY(soft) && (!nextRbp || nextRbp == rbp)) return false;
    assertx(nextRbp && nextRbp != rbp && "Missing fixup for native call");

    TRACE(2, "considering frame %p, %p\n", rbp, (void*)rbp->m_savedRip);

    if (isVMFrame(nextRbp, soft)) {
      TRACE(2, "fixup checking vm frame %s\n",
            nextRbp->func()->name()->data());
      VMRegs regs;
      if (getFrameRegs(rbp, &regs)) {
        TRACE(2, "fixup(end): func %s fp %p sp %p pc %p\n",
              regs.fp->func()->name()->data(),
              regs.fp, regs.sp, regs.pc);
        auto& vmRegs = vmRegsUnsafe();
        vmRegs.fp = const_cast<ActRec*>(regs.fp);
        vmRegs.pc = reinterpret_cast<PC>(regs.pc);
        vmRegs.stack.top() = regs.sp;
        vmRegs.jitReturnAddr = regs.retAddr;
        return true;
      } else {
        if (LIKELY(soft)) return false;
        always_assert(false && "Fixup expected for leafmost VM frame");
      }
    }
  }
  return false;
}

/* This is somewhat hacky. It decides which helpers/builtins should
 * use eager vmreganchor based on profile information. Using eager
 * vmreganchor for all helper calls is a perf regression. */
bool eagerRecord(const Func* func) {
  const char* list[] = {
    "array_filter",
    "array_map",
    "thrift_protocol_read_binary",
    "thrift_protocol_read_binary_struct",
    "thrift_protocol_read_compact",
    "thrift_protocol_read_compact_struct",
    "HH\\Asio\\join",
  };

  for (auto str : list) {
    if (!strcmp(func->name()->data(), str)) return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
}

namespace detail {
void syncVMRegsWork(bool soft) {
  assertx(tl_regState != VMRegState::CLEAN);

  // Start looking for fixup entries at the current (C++) frame.  This
  // will walk the frames upward until we find a TC frame.
  DECLARE_FRAME_POINTER(framePtr);
  auto fp = tl_regState >= VMRegState::GUARDED_THRESHOLD ?
    (ActRec*)tl_regState : framePtr;

  auto const synced = FixupMap::fixupWork(fp, soft);

  if (synced) tl_regState = VMRegState::CLEAN;
  Stats::inc(Stats::TC_Sync);
}
}

}}
