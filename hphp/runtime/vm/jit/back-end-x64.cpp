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

#include "hphp/runtime/vm/jit/back-end-x64.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/disasm.h"
#include "hphp/util/text-color.h"

#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/jit/abi-x64.h"
#include "hphp/runtime/vm/jit/align-x64.h"
#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/check.h"
#include "hphp/runtime/vm/jit/code-gen-x64.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/func-guard-x64.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/relocation.h"
#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/jit/smashable-instr.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"
#include "hphp/runtime/vm/jit/unwind-x64.h"
#include "hphp/runtime/vm/jit/vasm-emit.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-llvm.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-text.h"

namespace HPHP { namespace jit {

using namespace reg;

extern "C" void enterTCHelper(Cell* vm_sp,
                              ActRec* vm_fp,
                              TCA start,
                              ActRec* firstAR,
                              void* targetCacheBase,
                              ActRec* stashedAR);

namespace x64 {

TRACE_SET_MOD(hhir);

namespace {

//////////////////////////////////////////////////////////////////////

struct BackEnd final : jit::BackEnd {
  /*
   * enterTCHelper does not save callee-saved registers except %rbp. This means
   * when we call it from C++, we have to tell gcc to clobber all the other
   * callee-saved registers.
   */
#if defined(__CYGWIN__) || defined(__MINGW__)
  #define CALLEE_SAVED_BARRIER()                                    \
      asm volatile("" : : : "rbx", "rsi", "rdi", "r12", "r13", "r14", "r15");
#elif defined(_MSC_VER)
  // Unfortunately, we have no way to tell MSVC to do this, so we'll
  // probably have to use a pair of assembly stubs to manage this.
  #define CALLEE_SAVED_BARRIER() always_assert(false);
#else
  #define CALLEE_SAVED_BARRIER()                                    \
      asm volatile("" : : : "rbx", "r12", "r13", "r14", "r15");
#endif

  /*
   * enterTCHelper is a handwritten assembly function that transfers control in
   * and out of the TC.
   */
  static_assert(x64::rvmsp() == rbx &&
                x64::rvmfp() == rbp &&
                x64::rvmtl() == r12,
                "__enterTCHelper needs to be modified to use the correct ABI");

  void enterTCHelper(TCA start, ActRec* stashedAR) override {
    // We have to force C++ to spill anything that might be in a callee-saved
    // register (aside from rbp). enterTCHelper does not save them.
    CALLEE_SAVED_BARRIER();
    auto& regs = vmRegsUnsafe();
    jit::enterTCHelper(regs.stack.top(), regs.fp, start,
                       vmFirstAR(), rds::tl_base, stashedAR);
    CALLEE_SAVED_BARRIER();
  }

  void streamPhysReg(std::ostream& os, PhysReg reg) override {
    auto name = (reg.type() == PhysReg::GP) ? reg::regname(Reg64(reg)) :
      (reg.type() == PhysReg::SIMD) ? reg::regname(RegXMM(reg)) :
      /* (reg.type() == PhysReg::SF) ? */ reg::regname(RegSF(reg));
    os << name;
  }

  void disasmRange(std::ostream& os, int indent, bool dumpIR, TCA begin,
                   TCA end) override {
    Disasm disasm(Disasm::Options().indent(indent + 4)
                  .printEncoding(dumpIR)
                  .color(color(ANSI_COLOR_BROWN)));
    disasm.disasm(os, begin, end);
  }
};

//////////////////////////////////////////////////////////////////////

}

std::unique_ptr<jit::BackEnd> newBackEnd() {
  return folly::make_unique<BackEnd>();
}

//////////////////////////////////////////////////////////////////////

}}}
