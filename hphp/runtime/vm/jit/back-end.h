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
#ifndef incl_HPHP_JIT_BACK_END_H
#define incl_HPHP_JIT_BACK_END_H

#include <iosfwd>

#include <folly/Optional.h>

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/jit/stack-offsets.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"

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
 */

enum class MoveToAlignFlags {
  kJmpTargetAlign,
  kNonFallthroughAlign,
  kCacheLineAlign,
};

struct BackEnd;

std::unique_ptr<BackEnd> newBackEnd();

struct BackEnd {
  virtual ~BackEnd();

  virtual Abi abi() = 0;
  virtual size_t cacheLineSize() = 0;

  size_t cacheLineMask() {
    assertx((cacheLineSize() & (cacheLineSize()-1)) == 0);
    return cacheLineSize() - 1;
  }

  virtual PhysReg rSp() = 0;

  virtual void enterTCHelper(TCA start, ActRec* stashedAR) = 0;

  void moveToAlign(CodeBlock& cb,
                   MoveToAlignFlags alignment =
                     MoveToAlignFlags::kJmpTargetAlign) {
    do_moveToAlign(cb, alignment);
  }

  virtual UniqueStubs emitUniqueStubs() = 0;

  /*
   * Emit a small piece of code to `code' that jumps to
   * uniqueStubs.interpHelper.
   */
  virtual void emitInterpReq(CodeBlock& code,
                             SrcKey sk,
                             FPInvOffset spOff) = 0;

  virtual bool funcPrologueHasGuard(TCA prologue, const Func* func) = 0;
  virtual TCA funcPrologueToGuard(TCA prologue, const Func* func) = 0;
  virtual void funcPrologueSmashGuard(TCA prologue, const Func* func) = 0;
  virtual void emitIncStat(CodeBlock& cb, intptr_t disp, int n) = 0;

  /*
   * Find the start of a smashable call from the return address
   * observed in the callee
   */
  virtual TCA smashableCallFromReturn(TCA returnAddr) = 0;

  virtual void addDbgGuard(CodeBlock& codeMain, CodeBlock& codeCold,
                           SrcKey sk, size_t dbgOff) = 0;

  virtual void streamPhysReg(std::ostream& os, PhysReg reg) = 0;
  virtual void disasmRange(std::ostream& os, int indent, bool dumpIR,
                           TCA begin, TCA end) = 0;

  virtual void genCodeImpl(IRUnit& unit, CodeKind, AsmInfo*) = 0;

protected:
  BackEnd() {}

private:
  virtual void do_moveToAlign(CodeBlock&, MoveToAlignFlags) = 0;
};

}}

#endif
