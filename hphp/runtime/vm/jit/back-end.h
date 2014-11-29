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
#ifndef incl_HPHP_JIT_BACK_END_H
#define incl_HPHP_JIT_BACK_END_H

#include <iosfwd>

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/runtime/vm/jit/service-requests.h"

namespace HPHP { namespace jit {

struct Abi;
struct Block;
struct CodeGenerator;
struct CodegenState;
struct Constraint;
struct IRInstruction;
struct IRUnit;
struct PhysReg;
struct RelocationInfo;
struct CodeGenFixups;
struct RegAllocInfo;
struct AsmInfo;

/*
 * This module supports both X64 and ARM behind a platform-agnostic interface.
 *
 * On X64, concurrent modification and execution of instructions is safe if all
 * of the following hold:
 *
 * 1) The modification is done with a single processor store
 *
 * 2) Only one instruction in the original stream is modified
 *
 * 3) The modified instruction does not cross a cacheline boundary
 */

struct TReqInfo {
  uintptr_t requestNum;
  uintptr_t args[5];

  // Some TC registers need to be preserved across service requests.
  uintptr_t saved_rStashedAr;

  // Stub addresses are passed back to allow us to recycle used stubs.
  TCA stubAddr;
};

enum class TestAndSmashFlags {
  kAlignJccImmediate,
  kAlignJcc,
  kAlignJccAndJmp,
};

enum class MoveToAlignFlags {
  kJmpTargetAlign,
  kNonFallthroughAlign,
  kCacheLineAlign,
};

class BackEnd;

std::unique_ptr<BackEnd> newBackEnd();

class BackEnd {
 protected:
  BackEnd();
 public:
  virtual ~BackEnd();

  virtual Abi abi() = 0;
  virtual size_t cacheLineSize() = 0;
  size_t cacheLineMask() {
    assert((cacheLineSize() & (cacheLineSize()-1)) == 0);
    return cacheLineSize() - 1;
  }

  virtual PhysReg rSp() = 0;
  virtual PhysReg rVmSp() = 0;
  virtual PhysReg rVmFp() = 0;
  virtual PhysReg rVmTl() = 0;
  virtual bool storesCell(const IRInstruction& inst, uint32_t srcIdx) = 0;
  virtual bool loadsCell(const IRInstruction& inst) = 0;

  virtual void enterTCHelper(TCA start, TReqInfo& info) = 0;
  virtual void moveToAlign(CodeBlock& cb,
                           MoveToAlignFlags alignment
                           = MoveToAlignFlags::kJmpTargetAlign) = 0;
  virtual UniqueStubs emitUniqueStubs() = 0;
  virtual TCA emitServiceReqWork(CodeBlock& cb, TCA start,
                                 SRFlags flags, ServiceRequest req,
                                 const ServiceReqArgVec& argv) = 0;
  virtual void emitInterpReq(CodeBlock& mainCode, CodeBlock& coldCode,
                             SrcKey sk) = 0;
  virtual bool funcPrologueHasGuard(TCA prologue, const Func* func) = 0;
  virtual TCA funcPrologueToGuard(TCA prologue, const Func* func) = 0;
  virtual SrcKey emitFuncPrologue(CodeBlock& mainCode, CodeBlock& coldCode,
                                  Func* func, bool funcIsMagic, int nPassed,
                                  TCA& start, TCA& aStart) = 0;
  virtual TCA emitCallArrayPrologue(Func* func, DVFuncletsVec& dvs) = 0;
  virtual void funcPrologueSmashGuard(TCA prologue, const Func* func) = 0;
  virtual void emitIncStat(CodeBlock& cb, intptr_t disp, int n) = 0;
  virtual void emitTraceCall(CodeBlock& cb, Offset pcOff) = 0;
  /*
   * Returns true if the given current frontier can have an nBytes-long
   * instruction written that will be smashable later.
   */
  virtual bool isSmashable(Address frontier, int nBytes, int offset = 0) = 0;
  /*
   * Call before emitting a test-jcc sequence. Inserts a nop gap such that after
   * writing a testBytes-long instruction, the frontier will be smashable.
   */
  virtual void prepareForSmash(CodeBlock& cb, int nBytes, int offset = 0) = 0;
  virtual void prepareForTestAndSmash(CodeBlock& cb, int testBytes,
                                      TestAndSmashFlags flags) = 0;

  virtual void smashJmp(TCA jmpAddr, TCA newDest) = 0;
  virtual void smashCall(TCA callAddr, TCA newDest) = 0;
  virtual void smashJcc(TCA jccAddr, TCA newDest) = 0;
  /*
   * Emits a jump or call that satisfies the smash* routines above.
   */
  virtual void emitSmashableJump(CodeBlock& cb, TCA dest, ConditionCode cc) = 0;
  virtual void emitSmashableCall(CodeBlock& cb, TCA dest) = 0;
  /*
   * Find the start of a smashable call from the return address
   * observed in the callee
   */
  virtual TCA smashableCallFromReturn(TCA returnAddr) = 0;

  /*
   * Decodes jump instructions and returns their target. This includes handling
   * for ARM's multi-instruction "smashable jump" sequences. If the code does
   * not encode the right kind of jump, these functions return nullptr.
   */
  virtual TCA jmpTarget(TCA jmp) = 0;
  virtual TCA jccTarget(TCA jmp) = 0;
  virtual TCA callTarget(TCA call) = 0;

  virtual void addDbgGuard(CodeBlock& codeMain, CodeBlock& codeCold,
                           SrcKey sk, size_t dbgOff) = 0;

  virtual void streamPhysReg(std::ostream& os, PhysReg reg) = 0;
  virtual void disasmRange(std::ostream& os, int indent, bool dumpIR,
                           TCA begin, TCA end) = 0;

  virtual void genCodeImpl(IRUnit& unit, AsmInfo*) = 0;

  virtual bool supportsRelocation() const { return false; }

  /*
   * Relocate code in the range start, end into dest, and record
   * information about what was done to rel.
   * On exit, internal references (references into the source range)
   * will have been adjusted (ie they are still references into the
   * relocated code). External code references continue to point to
   * the same address as before relocation.
   */
  virtual size_t relocate(RelocationInfo& rel, CodeBlock& dest,
                          TCA start, TCA end,
                          CodeGenFixups& fixups,
                          TCA* exitAddr) {
    always_assert(false);
    return 0;
  }

  /*
   * This should be called after calling relocate on all relevant ranges. It
   * will adjust all references into the original src ranges to point into the
   * corresponding relocated ranges.
   */
  virtual void adjustForRelocation(RelocationInfo& rel) {
    always_assert(false);
  }

  /*
   * This will update a single range that was not relocated, but that
   * might refer to relocated code (such as the cold code corresponding
   * to a tracelet). Unless its guaranteed to be all position independent,
   * its "fixups" should have been passed into a relocate call earlier.
   */
  virtual void adjustForRelocation(RelocationInfo& rel, TCA start, TCA end) {
    always_assert(false);
  }

  /*
   * Adjust the contents of fixups and asmInfo based on the relocation
   * already performed on rel. This will not cause any of the relocated
   * code to be "hooked up", and its not safe to do so until all of the
   * CodeGenFixups have been processed.
   */
  virtual void adjustMetaDataForRelocation(RelocationInfo& rel,
                                           AsmInfo* asmInfo,
                                           CodeGenFixups& fixups) {
    always_assert(false);
  }

  /*
   * Adjust potentially live references that point into the relocated
   * area.
   * Must not be called until its safe to run the relocated code.
   */
  virtual void adjustCodeForRelocation(RelocationInfo& rel,
                                       CodeGenFixups& fixups) {
    always_assert(false);
  }

  virtual void findFixups(TCA start, TCA end, CodeGenFixups& fixups) {
    always_assert(false);
  }
};

}}

#endif
