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

/*
 * This pragma was set to do not show warnings of no return value for the
 * "implemented" functions for this class.
 * This file was created just to handle PPC64 architecture and initially
 * to support PPC64 with EvalJit=false.
 * This is a work in progress to port HHVM Jit to PPC64 architecture.
 * */

// Ignoring warning of no return statement - PPC64 port under development
#pragma GCC diagnostic ignored "-Wreturn-type"

#include "hphp/runtime/vm/jit/back-end-ppc64.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/disasm.h"
#include "hphp/util/text-color.h"

#include "hphp/runtime/vm/func.h"
//#include "hphp/runtime/vm/jit/abi-ppc64.h"
#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/check.h"
//#include "hphp/runtime/vm/jit/code-gen-helpers-ppc64.h"
//#include "hphp/runtime/vm/jit/code-gen-ppc64.h"
//#include "hphp/runtime/vm/jit/func-guard-ppc64.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/service-requests.h"
//#include "hphp/runtime/vm/jit/service-requests-x64.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/unique-stubs-x64.h"
//#include "hphp/runtime/vm/jit/unwind-x64.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-llvm.h"
#include "hphp/runtime/vm/jit/relocation.h"

namespace HPHP { namespace jit {

namespace ppc64 {

struct BackEnd final : public jit::BackEnd {

	  Abi abi() override {}

	  size_t cacheLineSize() override {
		  not_implemented();
	  }

	  PhysReg rSp() override {
		  not_implemented();
	  }

	  /*
	   * enterTCHelper does not save callee-saved registers except %rbp. This means
	   * when we call it from C++, we have to tell gcc to clobber all the other
	   * callee-saved registers.
	   */
	#if defined(__CYGWIN__) || defined(__MINGW__)
	  #define CALLEE_SAVED_BARRIER()                                    \
	      asm volatile("" : : : "rbx", "rsi", "rdi", "r12", "r13", "r14", "r15");
	#elif defined (__powerpc64__)
	  #define CALLEE_SAVED_BARRIER()                                    \
	      not_implemented();
	#else
	  #define CALLEE_SAVED_BARRIER()                                    \
	      asm volatile("" : : : "rbx", "r12", "r13", "r14", "r15");
	#endif

	  void enterTCHelper(TCA start, ActRec* stashedAR) override {
		  not_implemented();
	  }

	  UniqueStubs emitUniqueStubs() override {
		  not_implemented();
	  }

	  void emitInterpReq(CodeBlock& mainCode,
	                     SrcKey sk,
	                     FPInvOffset spOff) override {
		  not_implemented();
	  }

	  bool funcPrologueHasGuard(TCA prologue, const Func* func) override {
		  not_implemented();
	  }

	  TCA funcPrologueToGuard(TCA prologue, const Func* func) override {
		  not_implemented();
	  }

	  void funcPrologueSmashGuard(TCA prologue, const Func* func) override {
		  not_implemented();
	  }

	  void emitIncStat(CodeBlock& cb, intptr_t disp, int n) override {
		  not_implemented();
	  }

	  void prepareForTestAndSmash(CodeBlock& cb, int testBytes,
	                              TestAndSmashFlags flags) override {
		  not_implemented();
	  }

	  void smashJmp(TCA jmpAddr, TCA newDest) override {
		  not_implemented();
	  }

	  void smashCall(TCA callAddr, TCA newDest) override {
		  not_implemented();
	  }

	  void smashJcc(TCA jccAddr, TCA newDest) override {
		  not_implemented();
	  }

	  void emitSmashableJump(CodeBlock& cb, TCA dest, ConditionCode cc) override {
		  not_implemented();
	  }

	  TCA smashableCallFromReturn(TCA retAddr) override {
		  not_implemented();
	  }

	  void emitSmashableCall(CodeBlock& cb, TCA dest) override {
		  not_implemented();
	  }

	  TCA jmpTarget(TCA jmp) override {
		  not_implemented();
	  }

	  TCA jccTarget(TCA jmp) override {
		  not_implemented();
	  }

	  ConditionCode jccCondCode(TCA jmp) override {
		  not_implemented();
	  }

	  TCA callTarget(TCA call) override {
		  not_implemented();
	  }

	  void addDbgGuard(CodeBlock& codeMain,
	                   CodeBlock& codeCold,
	                   SrcKey sk,
	                   size_t dbgOff) override {
		  not_implemented();
	  }

	  void streamPhysReg(std::ostream& os, PhysReg reg) override {
		  not_implemented();
	  }

	  void disasmRange(std::ostream& os, int indent, bool dumpIR, TCA begin,
	                   TCA end) override {
		  not_implemented();
	  }

	  void genCodeImpl(IRUnit& unit, CodeKind, AsmInfo*) override;

	private:
	  void do_moveToAlign(CodeBlock& cb, MoveToAlignFlags alignment) override {
		  not_implemented();
	  }

	  bool do_isSmashable(Address frontier, int nBytes, int offset) override {
		  not_implemented();
	  }

	  void do_prepareForSmash(CodeBlock& cb, int nBytes, int offset) override {
		  not_implemented();
	  }

};


std::unique_ptr<jit::BackEnd> newBackEnd() {
  return folly::make_unique<BackEnd>();
}

static size_t genBlock(CodegenState& state, Vout& v, Vout& vc, Block* block) {
	not_implemented();
}

/*
 * Print side-by-side code dumps comparing vasm output with LLVM.
 */
static void printLLVMComparison(const IRUnit& ir_unit,
                                const Vunit& vasm_unit,
                                const jit::vector<Varea>& areas,
                                const CompareLLVMCodeGen* compare) {
	not_implemented();
}

void BackEnd::genCodeImpl(IRUnit& unit, CodeKind kind, AsmInfo* asmInfo) {
	not_implemented();
}

//////////////////////////////////////////////////////////////////////

bool isSmashable(Address frontier, int nBytes, int offset /* = 0 */) {
	not_implemented();
}

void prepareForSmashImpl(CodeBlock& cb, int nBytes, int offset) {
	not_implemented();
}

void smashJmp(TCA jmpAddr, TCA newDest) {
	not_implemented();
}

void smashCall(TCA callAddr, TCA newDest) {
	not_implemented();
}

}}}

#pragma GCC diagnostic pop

