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

#pragma once

#include <vector>
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/vm/vm-regs.h"
#include "hphp/runtime/vm/jit/stack-offsets.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/util/atomic.h"
#include "hphp/util/data-block.h"

namespace HPHP {
struct ExecutionContext;
}

namespace HPHP::jit {

//////////////////////////////////////////////////////////////////////

ActRec* findVMFrameForDebug();

/*
 * The Fixup map allows us to reconstruct the state of the VM registers (fp,
 * sp, and pc) from an up-stack invocation record.  Each range of bytes in the
 * translation cache is associated with a "distance" in both stack cells and
 * opcode bytes from the beginning of the function.  These are known at
 * translation time.
 *
 * The way this works is by chasing the native rbp chain to find a rbp that we
 * know is a VM frame (i.e. is actually a full ActRec).  Once we find that,
 * regsFromActRec is called, which looks to see if the return ip for the frame
 * before the VM frame has an entry in the fixup map (i.e. if it points into
 * the translation cache)---if so, it finds the fixup information in one of two
 * ways:
 *
 *   - a direct Fixup: the normal case.
 *
 *     The Fixup record just stores an offset relative to the ActRec* for vmsp,
 *     and an offset from the start of the func for pc.  In the case of
 *     resumable frames the sp offset is relative to Stack::resumableStackBase.
 *
 *   - an indirect Fixup:
 *
 *     This is used when invoking C++ methods in stublogue mode, i.e. a TC code
 *     that was called, but did not set up a full frame, as it is operating on
 *     behalf of the caller. This mode is used in prologues and some shared
 *     stubs on architectures, where the callee's frame is stored immediately
 *     under the caller's sp (currently true on x64, but not arm or ppc).
 *
 *     In this case, some JIT'd code associated with the ActRec* we found made
 *     a call to a shared stub or prologue, and then that code called C++. The
 *     indirect Fixup record stores an offset to the saved frame pointer *two*
 *     levels deeper in C++, that says where the return IP for the call to the
 *     shared stub can be found.  I.e., we're trying to chase back two return
 *     ips into the TC.
 *
 *     Note that this means indirect Fixups will not work for C++ code
 *     paths that need to do a fixup without making at least one other
 *     C++ call (because of -momit-leaf-frame-pointers), but for the
 *     current use case this is fine.
 *
 *     Here's a picture of the native stack in the indirect fixup situation:
 *
 *        |..............................|
 *        |..............................|
 *        +------------------------------+  __enterTCHelper
 *        |       RetIP to enterTC()     |
 *        |--                          --|
 *        |          savedRbp            |
 *        |--                          --|
 *        |        <other junk>          |
 *        +------------------------------+  TC code
 *        |    RetIP to enterTCHelper    |
 *        |--                          --|
 *        |         saved %rdi           |  <from callUnaryStub>
 *        +------------------------------+  STUB (e.g. decRefGeneric)
 *        | RetIP to caller of dtor stub |
 *        |--                          --|
 *        |     <pushes in dtor stub>    |
 *        +------------------------------+  <call to C++>
 *        |    RetIP to the dtor stub    |
 *        |--                          --|
 *        |         saved rvmfp()        |  push %rbp; mov %rsp, %rbp
 *    +-->|--                          --|
 *    |   |    < C++ local variables>    |
 *    |   +------------------------------+
 *    |   |   RetIP to first C++ callee  |  C++ calls another function
 *    |   |--                          --|
 *    +---|     saved native %rbp (*)    |  points as shown, from mov above
 *        |--                          --|
 *        |..............................|
 *        |..............................|
 *
 *     The offset in indirect Fixup is how to get to the "RetIP to caller of
 *     dtor stub", relative to the value in the starred stack slot shown.  We
 *     then look that IP up in the fixup map again to find a normal
 *     (non-indirect) Fixup record.
 *
 */

//////////////////////////////////////////////////////////////////////

struct Fixup {
  static Fixup direct(int32_t pcOffset, SBInvOffset spOffset) {
    assertx(pcOffset >= 0);
    assertx(spOffset.offset >= 0);
    return Fixup{pcOffset, spOffset};
  }

  static Fixup indirect(uint32_t qwordsPushed, SBInvOffset extraSpOffset) {
    auto const ripOffset =
      kNativeFrameSize + AROFF(m_savedRip) + qwordsPushed * sizeof(uintptr_t);
    assertx(ripOffset > 0);
    assertx(extraSpOffset.offset >= 0);
    return Fixup{-safe_cast<int32_t>(ripOffset), extraSpOffset};
  }

  static Fixup none() {
    return Fixup{0, SBInvOffset::invalid()};
  }

  bool isValid() const { return m_spOffset.isValid(); }
  bool isIndirect() const { assertx(isValid()); return m_pcOrRipOffset < 0; }
  uint32_t pcOffset() const { assertx(!isIndirect()); return m_pcOrRipOffset; }
  uint32_t ripOffset() const { assertx(isIndirect()); return -m_pcOrRipOffset; }
  SBInvOffset spOffset() const { assertx(isValid()); return m_spOffset; }
  std::string show() const;

  void adjustRipOffset(int off) {
    assertx(isValid() && isIndirect());
    m_pcOrRipOffset -= off;
  }

  bool operator==(const Fixup& o) const {
    return m_pcOrRipOffset == o.m_pcOrRipOffset && m_spOffset == o.m_spOffset;
  }
  bool operator!=(const Fixup& o) const {
    return m_pcOrRipOffset != o.m_pcOrRipOffset || m_spOffset != o.m_spOffset;
  }

private:
  Fixup(int32_t pcOrRipOffset, SBInvOffset spOffset)
    : m_pcOrRipOffset{pcOrRipOffset}
    , m_spOffset{spOffset}
  {}

  int32_t m_pcOrRipOffset;
  SBInvOffset m_spOffset;
};

/*
 * A record of a VMFrame used in unwinding. It captures a conceptual suspended
 * frame, including the ActRec/rbp, the rip of the next instruction, and the
 * CFA of the child frame. Note that m_actRec and m_rip are stored separately
 * (instead of simply storing the child ActRec), as C++ frames need not lay out
 * their frames with these fields adjacent.
 */
struct VMFrame {
  ActRec* m_actRec;
  TCA m_rip;
  uintptr_t m_prevCfa;
};

namespace FixupMap {
/*
 * Record a new fixup (or overwrite an existing fixup) at tca.
 */
void recordFixup(CTCA tca, const Fixup& fixup);

/*
 * Find the fixup for tca if it exists (or return nullptr).
 */
const Fixup* findFixup(CTCA tca);

/*
 * Number of entries in the fixup map.
 */
size_t size();

/*
 * Perform a fixup of the VM registers for a leaf VM frame `frame`.
 *
 * Returns whether we successfully performed the fixup.
 */
bool processFixupForVMFrame(VMFrame frame);

/*
 * Perform a fixup of the VM registers for a stack whose first frame is `rbp`.
 *
 * Returns whether we successfully performed the fixup.  (We assert on failure
 * if `soft` is not set).
 */
bool fixupWork(ActRec* rbp, bool soft = false);
}

namespace detail {
void syncVMRegsWork(bool soft); // internal sync work for a dirty vm state
}

/*
 * Sync VM registers for the first TC frame in the callstack.
 */
inline void syncVMRegs(bool soft = false) {
  if (regState() == VMRegState::CLEAN) return;
#ifndef NDEBUG
  if (regState() == VMRegState::CLEAN_VERIFY) {
    auto& regs = vmRegsUnsafe();
    DEBUG_ONLY auto const fp = regs.fp;
    DEBUG_ONLY auto const sp = regs.stack.top();
    DEBUG_ONLY auto const pc = regs.pc;
    detail::syncVMRegsWork(soft);
    assertx(regs.fp == fp);
    assertx(regs.stack.top() == sp);
    assertx(regs.pc == pc);
    regState() = VMRegState::CLEAN;
    return;
  }
#endif
  detail::syncVMRegsWork(soft);
}

/*
 * Gets the next fake JIT return address for eager syncs.
 */
int32_t getNextFakeReturnAddress();

//////////////////////////////////////////////////////////////////////

}
