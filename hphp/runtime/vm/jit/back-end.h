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
#include "hphp/runtime/vm/jit/service-requests.h"

namespace HPHP { namespace JIT {

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
  kAlignJccAndJmp
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
  virtual Constraint srcConstraint(const IRInstruction& inst, unsigned i) = 0;
  virtual Constraint dstConstraint(const IRInstruction& inst, unsigned i) = 0;
  virtual RegPair precolorSrc(const IRInstruction& inst, unsigned i) = 0;
  virtual RegPair precolorDst(const IRInstruction& inst, unsigned i) = 0;

  virtual void enterTCHelper(TCA start, TReqInfo& info) = 0;
  virtual CodeGenerator* newCodeGenerator(const IRUnit& unit,
                                          CodeBlock& mainCode,
                                          CodeBlock& coldCode,
                                          CodeBlock& frozenCode,
                                          MCGenerator* mcg,
                                          CodegenState& state) = 0;
  virtual void moveToAlign(CodeBlock& cb,
                           MoveToAlignFlags alignment
                           = MoveToAlignFlags::kJmpTargetAlign) = 0;
  virtual UniqueStubs emitUniqueStubs() = 0;
  virtual TCA emitServiceReqWork(CodeBlock& cb, TCA start,
                                 SRFlags flags, ServiceRequest req,
                                 const ServiceReqArgVec& argv) = 0;
  virtual void emitInterpReq(CodeBlock& mainCode, CodeBlock& coldCode,
                             const SrcKey& sk) = 0;
  virtual bool funcPrologueHasGuard(TCA prologue, const Func* func) = 0;
  virtual TCA funcPrologueToGuard(TCA prologue, const Func* func) = 0;
  virtual SrcKey emitFuncPrologue(CodeBlock& mainCode, CodeBlock& coldCode,
                                  Func* func, bool funcIsMagic, int nPassed,
                                  TCA& start, TCA& aStart) = 0;
  virtual TCA emitCallArrayPrologue(Func* func, DVFuncletsVec& dvs) = 0;
  virtual void funcPrologueSmashGuard(TCA prologue, const Func* func) = 0;
  virtual void emitIncStat(CodeBlock& cb, intptr_t disp, int n) = 0;
  virtual void emitTraceCall(CodeBlock& cb, int64_t pcOff) = 0;
  virtual void emitFwdJmp(CodeBlock& cb, Block* target,
                          CodegenState& state) = 0;
  virtual void patchJumps(CodeBlock& cb, CodegenState& state, Block* block) = 0;
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
   * Decodes jump instructions and returns their target. This includes handling
   * for ARM's multi-instruction "smashable jump" sequences. If the code does
   * not encode the right kind of jump, these functions return nullptr.
   */
  virtual TCA jmpTarget(TCA jmp) = 0;
  virtual TCA jccTarget(TCA jmp) = 0;
  virtual TCA callTarget(TCA call) = 0;

  virtual void addDbgGuard(CodeBlock& codeMain, CodeBlock& codeCold,
                           SrcKey sk, size_t dbgOff) = 0;

  virtual void streamPhysReg(std::ostream& os, PhysReg& reg) = 0;
  virtual void disasmRange(std::ostream& os, int indent, bool dumpIR,
                           TCA begin, TCA end) = 0;

  virtual bool supportsRelocation() const { return false; }

  /*
   * Relocate the code block described by rel to its ultimate destination,
   * and return the size of the relocated code (which may be different
   * due to alignment padding, or shrinking branches etc
   */
  virtual size_t relocate(RelocationInfo& rel, CodeGenFixups& fixups) {
    always_assert(false);
  }
  /*
   * Adjust the offsets/immediates for any instructions in the range start, end
   * based on the relocation already performed on rel.
   * Explicit pc-relative offsets, and immediates identified by
   * fixups.m_addressImmediates will be adjusted.
   */
  virtual void adjustForRelocation(TCA start, TCA end,
                                   RelocationInfo& rel, CodeGenFixups& fixups) {
    always_assert(false);
  }
  /*
   * Adjust the contents of fixups, sr, and asmInfo based on the relocation
   * already performed on rel.
   */
  virtual void adjustForRelocation(SrcRec* sr, AsmInfo* asmInfo,
                                   RelocationInfo& rel, CodeGenFixups& fixups) {
    always_assert(false);
  }
};

}}

#endif
