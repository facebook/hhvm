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

#ifndef incl_HPHP_RUNTIME_VM_VM_REGS_H_
#define incl_HPHP_RUNTIME_VM_VM_REGS_H_

#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/rds-header.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/vm/bytecode.h"

/*
 * This file contains accessors for the three primary VM registers:
 *
 * vmpc(): PC pointing to the currently executing bytecode instruction
 * vmfp(): ActRec* pointing to the current frame
 * vmsp(): Cell* pointing to the top of the eval stack
 *
 * The registers are physically located in the RDS header struct (defined in
 * runtime/base/rds-header.h), allowing efficient access from translated code
 * when needed. They are generally not kept up-to-date in translated code,
 * though, so there are times when it is not safe to use them. This is tracked
 * in the tl_regState variable, which is automatically checked in the accessors
 * defined here. Certain parts of the runtime do need access to the registers
 * while their state is expected to be dirty; the vmRegsUnsafe() function is
 * provided for these rare cases.
 *
 * In a C++ function potentially called from translated code, VMRegAnchor
 * should be used before accessing any of these registers. jit::FixupMap is
 * currently responsible for doing the work required to sync the VM registers,
 * though this is an implementation detail and should not matter to users of
 * VMRegAnchor.
 */

namespace HPHP {

/*
 * The current sync-state of the RDS vmRegs().
 *
 * CLEAN means that the RDS vmRegs are sync'd.  DIRTY means we need to sync
 * them (by traversing the stack and looking up fixups)---this is what the
 * value of tl_regState should be whenever we enter native code from translated
 * PHP code.
 *
 * Values above GUARDED_THRESHOLD are a special case of dirty which indicates
 * that the state will be reset to DIRTY (via a scope guard) when returning to
 * PHP code, and the actual value can be used as a start point for following the
 * c++ callchain back into the VM. This makes it suitable for guarding callbacks
 * through code compiled without frame pointers, and in places where we may
 * end up needing to clean the registers multiple times.
 */
enum VMRegState : uintptr_t {
  CLEAN,
  DIRTY,
  GUARDED_THRESHOLD
};
extern __thread VMRegState tl_regState;

inline void checkVMRegState() {
  assert(tl_regState == VMRegState::CLEAN);
}

inline void checkVMRegStateGuarded() {
  assert(tl_regState != VMRegState::DIRTY);
}

inline VMRegs& vmRegsUnsafe() {
  return rds::header()->vmRegs;
}

inline VMRegs& vmRegs() {
  checkVMRegState();
  return vmRegsUnsafe();
}

inline Stack& vmStack() {
  return vmRegs().stack;
}

inline bool isValidVMStackAddress(const void* addr) {
  return vmRegsUnsafe().stack.isValidAddress(uintptr_t(addr));
}

inline Cell*& vmsp() {
  return vmRegs().stack.top();
}

inline ActRec*& vmfp() {
  return vmRegs().fp;
}

inline const unsigned char*& vmpc() {
  return vmRegs().pc;
}

inline Offset pcOff() {
  return vmfp()->m_func->unit()->offsetOf(vmpc());
}

inline ActRec*& vmFirstAR() {
  // This is safe because firstAR is always updated directly.
  return vmRegsUnsafe().firstAR;
}

inline MInstrState& vmMInstrState() {
  // This is safe because mInstrState is always updated directly.
  return vmRegsUnsafe().mInstrState;
}

inline ActRec*& vmJitCalledFrame() {
  return vmRegsUnsafe().jitCalledFrame;
}

inline void assert_native_stack_aligned() {
#ifndef _MSC_VER
  assert(reinterpret_cast<uintptr_t>(__builtin_frame_address(0)) % 16 == 0);
#endif
}

inline void interp_set_regs(ActRec* ar, Cell* sp, Offset pcOff) {
  assert(tl_regState == VMRegState::DIRTY);
  tl_regState = VMRegState::CLEAN;
  vmfp() = ar;
  vmsp() = sp;
  vmpc() = ar->unit()->at(pcOff);
}

/*
 * Return the first VM frame that is a parent of this function's call frame.
 */
ActRec* callerFrameHelper();

///////////////////////////////////////////////////////////////////////////////

/*
 * This class is used as a scoped guard around code that is called from the JIT
 * which needs the VM to be in a consistent state. JIT helpers use it to guard
 * calls into HHVM's runtime. It is used like this:
 *
 *   void helperFunction() {
 *      VMRegAnchor _;
 *      runtimeCall();
 *   }
 *
 * VMRegAnchor should also be used before entering a C library compiled with
 * -fomit-frame-pointer which will call back into HHVM. If VMRegAnchor is not
 * used, HHVM's runtime will attempt to traverse the native stack, and will
 * assert or crash if it attempts to parse a part of the stack with no frame
 * pointers. VMRegAnchor forces the stack traversal to be done when it is
 * constructed.
 */
struct VMRegAnchor {
  VMRegAnchor();
  /*
   * Some C++ entry points have an ActRec prepared from after a call
   * instruction.  Instantiating a VMRegAnchor with an ActRec argument syncs us
   * to right after the call instruction.
   */
  explicit VMRegAnchor(ActRec* ar);

  ~VMRegAnchor() {
    if (m_old < VMRegState::GUARDED_THRESHOLD) {
      tl_regState = m_old;
    }
  }

  VMRegAnchor(const VMRegAnchor&) = delete;
  VMRegAnchor& operator=(const VMRegAnchor&) = delete;

  VMRegState m_old;
};

/*
 * This class is used as an invocation guard equivalent to VMRegAnchor, except
 * the sync is assumed to have already been done. This was part of a
 * project aimed at improving performance by doing the fixup in advance, i.e.
 * eagerly -- the benefits turned out to be marginal or negative in most cases.
 */
struct EagerVMRegAnchor {
  EagerVMRegAnchor() {
    if (debug) {
      auto& regs = vmRegsUnsafe();
      DEBUG_ONLY auto const fp = regs.fp;
      DEBUG_ONLY auto const sp = regs.stack.top();
      DEBUG_ONLY auto const pc = regs.pc;
      VMRegAnchor _;
      assert(regs.fp == fp);
      assert(regs.stack.top() == sp);
      assert(regs.pc == pc);
    }
    assert(tl_regState < VMRegState::GUARDED_THRESHOLD);
    m_old = tl_regState;
    tl_regState = VMRegState::CLEAN;
  }

  ~EagerVMRegAnchor() {
    tl_regState = m_old;
  }

  VMRegState m_old;
};

/*
 * A scoped guard used around native code that is called from the JIT and which
 * /may conditionally/ need the VM to be in a consistent state.
 *
 * Using VMRegAnchor by itself would mean that in some cases---where we perform
 * many independent operations which only conditionally require syncing---we'd
 * have to choose between always (and sometimes spuriously) syncing, or syncing
 * multiple times when we could have synced just once.
 *
 * VMRegGuard is intended to be used around these conditional syncs (i.e.,
 * conditional instantiations of VMRegAnchor).  It changes tl_regState to
 * GUARDED, which tells sub-scoped VMRegAnchors that they may keep it set to
 * CLEAN after they finish syncing.
 *
 * VMRegGuard also saves the current fp, making it suitable for guarding
 * callbacks through library code that was compiled without frame pointers.
 */
struct VMRegGuard {
  /*
   * If we know the frame pointer returned by DECLARE_FRAME_POINTER is accurate,
   * we can use ALWAYS_INLINE, and grab the frame pointer.
   * If not, we have to use NEVER_INLINE to ensure we're one level in from the
   * guard... but thats not quite enough because VMRegGuard::VMRegGuard is a
   * leaf function, and so might not have a frame
   */
#ifdef FRAME_POINTER_IS_ACCURATE
  ALWAYS_INLINE VMRegGuard() : m_old(tl_regState) {
    if (tl_regState == VMRegState::DIRTY) {
      DECLARE_FRAME_POINTER(framePtr);
      tl_regState = (VMRegState)(uintptr_t)framePtr;
    }
  }
#else
  NEVER_INLINE VMRegGuard() : m_old(tl_regState) {
    if (tl_regState == VMRegState::DIRTY) {
      DECLARE_FRAME_POINTER(framePtr);
      auto const fp = isVMFrame(framePtr->m_sfp) ? framePtr : framePtr->m_sfp;
      tl_regState = (VMRegState)(uintptr_t)fp;
    }
  }
#endif
  ~VMRegGuard() { tl_regState = m_old; }

  VMRegGuard(const VMRegGuard&) = delete;
  VMRegGuard& operator=(const VMRegGuard&) = delete;

  VMRegState m_old;
};

///////////////////////////////////////////////////////////////////////////////

namespace detail {

inline ActRec* regAnchorFP(ActRec* cur, Offset* pc = nullptr) {
  // In builtins, m_fp points to the caller's frame if called through
  // FCallBuiltin, else it points to the builtin's frame, in which case,
  // getPrevVMState() gets the caller's frame.  In addition, we need to skip
  // over php-defined builtin functions in order to find the true context.
  auto const context = g_context.getNoCheck();
  if (pc) *pc = cur->m_func->unit()->offsetOf(vmpc());
  while (cur && cur->skipFrame()) {
    cur = context->getPrevVMState(cur, pc);
  }
  return cur;
}

inline ActRec* regAnchorFPForArgs(ActRec* cur) {
  // Like regAnchorFP, but only account for FCallBuiltin
  if (cur && cur->m_func->isCPPBuiltin()) {
    auto const context = g_context.getNoCheck();
    cur = context->getPrevVMState(cur);
  }
  return cur;
}

}

/*
 * VM helper to retrieve the current vm frame pointer without ensuring
 * the vm state is clean.
 *
 * This is a common need for extensions.
 */
inline ActRec* GetCallerFrame() {
  auto fp = tl_regState == VMRegState::CLEAN ? vmfp() : callerFrameHelper();
  return detail::regAnchorFP(fp);
}

inline ActRec* GetCallerFrameForArgs() {
  auto fp = tl_regState == VMRegState::CLEAN ? vmfp() : callerFrameHelper();
  return detail::regAnchorFPForArgs(fp);
}

/*
 * VM helper to clean the vm state, and retrieve the current vm frame
 * pointer.
 *
 * This is a common need for extensions.
 */
struct CallerFrame : public VMRegAnchor {
  ActRec* operator()(Offset* pc = nullptr) {
    return detail::regAnchorFP(vmfp(), pc);
  }
  ActRec* actRecForArgs() { return detail::regAnchorFPForArgs(vmfp()); }
};

struct EagerCallerFrame : public EagerVMRegAnchor {
  ActRec* operator()() {
    return detail::regAnchorFP(vmfp());
  }
  ActRec* actRecForArgs() { return detail::regAnchorFPForArgs(vmfp()); }
};

#define SYNC_VM_REGS_SCOPED() \
  HPHP::VMRegAnchor _anchorUnused

///////////////////////////////////////////////////////////////////////////////

}

#endif
