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
 * should be used before accessing any of these registers. jit::MCGenerator is
 * currently responsible for doing the work required to sync the VM registers,
 * though this is an implementation detail and should not matter to users of
 * VMRegAnchor.
 */

namespace HPHP {

/*
 * DIRTY when the live register state is spread across the stack and Fixups.
 * CLEAN when it has been sync'ed into RDS.
 */
enum class VMRegState : uint8_t {
  CLEAN,
  DIRTY
};
extern __thread VMRegState tl_regState;

inline void checkVMRegState() {
  assert(tl_regState == VMRegState::CLEAN);
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
    tl_regState = m_old;
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
    m_old = tl_regState;
    tl_regState = VMRegState::CLEAN;
  }

  ~EagerVMRegAnchor() {
    tl_regState = m_old;
  }

  VMRegState m_old;
};

///////////////////////////////////////////////////////////////////////////////

namespace detail {

inline ActRec* regAnchorFP(Offset* pc = nullptr) {
  // In builtins, m_fp points to the caller's frame if called through
  // FCallBuiltin, else it points to the builtin's frame, in which case,
  // getPrevVMState() gets the caller's frame.  In addition, we need to skip
  // over php-defined builtin functions in order to find the true context.
  auto const context = g_context.getNoCheck();
  auto cur = vmfp();
  if (pc) *pc = cur->m_func->unit()->offsetOf(vmpc());
  while (cur && cur->skipFrame()) {
    cur = context->getPrevVMState(cur, pc);
  }
  return cur;
}

inline ActRec* regAnchorFPForArgs() {
  // Like regAnchorFP, but only account for FCallBuiltin
  auto const context = g_context.getNoCheck();
  ActRec* cur = vmfp();
  if (cur && cur->m_func->isCPPBuiltin()) {
    cur = context->getPrevVMState(cur);
  }
  return cur;
}

}

/*
 * VM helper to retrieve the frame pointer from the TC.  This is a common need
 * for extensions.
 */
struct CallerFrame : public VMRegAnchor {
  template<class... Args>
  ActRec* operator()(Args&&... args) {
    return detail::regAnchorFP(std::forward<Args>(args)...);
  }
  ActRec* actRecForArgs() { return detail::regAnchorFPForArgs(); }
};

struct EagerCallerFrame : public EagerVMRegAnchor {
  ActRec* operator()() {
    return detail::regAnchorFP();
  }
  ActRec* actRecForArgs() { return detail::regAnchorFPForArgs(); }
};

#define SYNC_VM_REGS_SCOPED() \
  HPHP::VMRegAnchor _anchorUnused

///////////////////////////////////////////////////////////////////////////////

}

#endif
