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

#ifndef incl_HPHP_JIT_CODE_GEN_H_
#define incl_HPHP_JIT_CODE_GEN_H_

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/state-vector.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/util/code-cache.h"

namespace HPHP { namespace JIT {

struct RegAllocInfo;

enum class SyncOptions {
  kNoSyncPoint,
  kSyncPoint,
  kSyncPointAdjustOne,
  kSmashableAndSyncPoint,
};

// Information about where code was generated, for pretty-printing.
struct AsmInfo {
  explicit AsmInfo(const IRUnit& unit)
    : instRanges(unit, TcaRange(nullptr, nullptr))
    , asmRanges(unit, TcaRange(nullptr, nullptr))
    , acoldRanges(unit, TcaRange(nullptr, nullptr))
    , afrozenRanges(unit, TcaRange(nullptr, nullptr))
  {}

  // Asm address info for each instruction and block
  StateVector<IRInstruction,TcaRange> instRanges;
  StateVector<Block,TcaRange> asmRanges;
  StateVector<Block,TcaRange> acoldRanges;
  StateVector<Block,TcaRange> afrozenRanges;

  void updateForInstruction(IRInstruction* inst, TCA start, TCA end);
};

typedef StateVector<IRInstruction, RegSet> LiveRegs;

struct CatchInfo {
  /* afterCall is the address after the call instruction that this catch trace
   * belongs to. It's the key used to look up catch traces by the
   * unwinder, since it's the value of %rip during unwinding. */
  TCA afterCall;

  /* savedRegs contains the caller-saved registers that were pushed onto the
   * C++ stack at the time of the call. The catch trace will pop these
   * registers (in the same order as PhysRegSaver's destructor) before doing
   * any real work to restore the register state from before the call. */
  RegSet savedRegs;

  /* rspOffset is the number of bytes pushed on the C++ stack after the
   * registers in savedRegs were saved, typically from function calls with >6
   * arguments. The catch trace will adjust rsp by this amount before popping
   * anything in savedRegs. */
  Offset rspOffset;
};

// Stuff we need to preserve between blocks while generating code,
// and address information produced during codegen.
struct CodegenState {
  CodegenState(const IRUnit& unit, const RegAllocInfo& regs,
               const LiveRegs& liveRegs, AsmInfo* asmInfo)
    : patches(unit, nullptr)
    , addresses(unit, nullptr)
    , regs(regs)
    , liveRegs(liveRegs)
    , asmInfo(asmInfo)
    , catches(unit, CatchInfo())
    , pastGuards(false)
  {}

  // Each block has a list of addresses to patch, and an address if
  // it's already been emitted.
  StateVector<Block,void*> patches;
  StateVector<Block,TCA> addresses;

  // True if this block's terminal Jmp has a desination equal to the
  // next block in the same assmbler.
  bool noTerminalJmp;

  // output from register allocator
  const RegAllocInfo& regs;

  // for each instruction, holds the RegSet of registers that must be
  // preserved across that instruction.  This is for push/pop of caller-saved
  // registers.
  const LiveRegs& liveRegs;

  // Output: start/end ranges of machine code addresses of each instruction.
  AsmInfo* asmInfo;

  // Used to pass information about the state of the world at native
  // calls between cgCallHelper and cgBeginCatch.
  StateVector<Block, CatchInfo> catches;

  // Have we progressed past the guards? Used to suppress TransBCMappings until
  // we're translating code that can properly be attributed to specific
  // bytecode.
  bool pastGuards;
};

LiveRegs computeLiveRegs(const IRUnit& unit, const RegAllocInfo& regs);

// Allocate registers and generate machine code. Mutates the global
// singleton MCGenerator (adds code, allocates data, adds fixups).
void genCode(IRUnit&);

struct CodeGenerator {
  virtual ~CodeGenerator() {}
  virtual void cgInst(IRInstruction* inst) = 0;
};

}}

#endif
