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

#pragma GCC diagnostic ignored "-Wreturn-type"

#include "hphp/runtime/vm/jit/back-end-ppc64.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/disasm.h"
#include "hphp/util/text-color.h"

#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/check.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/service-requests-inline.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-llvm.h"

namespace HPHP { namespace jit {

namespace ppc64 {

struct BackEnd : public jit::BackEnd {
  BackEnd() {}
  ~BackEnd() {}

  Abi abi() override {}

  size_t cacheLineSize() override {}

  PhysReg rSp() override {}

  PhysReg rVmSp() override {}

  PhysReg rVmFp() override {}

  PhysReg rVmTl() override {}

  bool storesCell(const IRInstruction& inst, uint32_t srcIdx) override {}

  bool loadsCell(const IRInstruction& inst) override {}

  void enterTCHelper(TCA start, ActRec* stashedAR) override {}

  void moveToAlign(CodeBlock& cb,
                   MoveToAlignFlags alignment
                   = MoveToAlignFlags::kJmpTargetAlign) override {}

  UniqueStubs emitUniqueStubs() override {}

  TCA emitServiceReqWork(CodeBlock& cb, TCA start, SRFlags flags,
                         ServiceRequest req,
                         const ServiceReqArgVec& argv) override {}

  void emitInterpReq(CodeBlock& mainCode, CodeBlock& coldCode,
                     SrcKey sk) override {}

  bool funcPrologueHasGuard(TCA prologue, const Func* func) override {}

  TCA funcPrologueToGuard(TCA prologue, const Func* func) override {}

  SrcKey emitFuncPrologue(CodeBlock& mainCode, CodeBlock& coldCode, Func* func,
                          bool funcIsMagic, int nPassed, TCA& start,
                          TCA& aStart) override {}

  TCA emitCallArrayPrologue(Func* func, DVFuncletsVec& dvs) override {}

  void funcPrologueSmashGuard(TCA prologue, const Func* func) override {}

  void emitIncStat(CodeBlock& cb, intptr_t disp, int n) override {}

  void emitTraceCall(CodeBlock& cb, Offset pcOff) override {}

  bool isSmashable(Address frontier, int nBytes, int offset = 0) override {}

 private:

 public:
  void prepareForSmash(CodeBlock& cb, int nBytes, int offset = 0) override {}

  void prepareForTestAndSmash(CodeBlock& cb, int testBytes,
                              TestAndSmashFlags flags) override {}

  bool supportsRelocation() const override {
    return true;
  }

  void adjustForRelocation(RelocationInfo& rel) override {}

  void adjustForRelocation(RelocationInfo& rel,
                           TCA srcStart, TCA srcEnd) override {}

  void adjustMetaDataForRelocation(RelocationInfo& rel,
                                   AsmInfo* asmInfo,
                                   CodeGenFixups& fixups) override {}

  void adjustCodeForRelocation(RelocationInfo& rel,
                               CodeGenFixups& fixups) override {}

 private:

 public:
  void smashJmp(TCA jmpAddr, TCA newDest) override {}

  void smashCall(TCA callAddr, TCA newDest) override {}

  void smashJcc(TCA jccAddr, TCA newDest) override {}

  void emitSmashableJump(CodeBlock& cb, TCA dest, ConditionCode cc) override {}

  TCA smashableCallFromReturn(TCA retAddr) override {}

  void emitSmashableCall(CodeBlock& cb, TCA dest) override {}

  TCA jmpTarget(TCA jmp) override {}

  TCA jccTarget(TCA jmp) override {}

  ConditionCode jccCondCode(TCA jmp) override {}

  TCA callTarget(TCA call) override {}

  void addDbgGuard(CodeBlock& codeMain, CodeBlock& codeCold,
                   SrcKey sk, size_t dbgOff) override {}

  void streamPhysReg(std::ostream& os, PhysReg reg) override {}

  void disasmRange(std::ostream& os, int indent, bool dumpIR, TCA begin,
                   TCA end) override {}

  void genCodeImpl(IRUnit& unit, AsmInfo*) override;
};


std::unique_ptr<jit::BackEnd> newBackEnd() {
  return folly::make_unique<BackEnd>();
}

void BackEnd::genCodeImpl(IRUnit& unit, AsmInfo* asmInfo) {}

}}}

#pragma GCC diagnostic pop

