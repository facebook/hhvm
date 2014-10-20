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

#include "hphp/runtime/vm/jit/asm-info.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/state-vector.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/util/code-cache.h"
#include "hphp/runtime/vm/jit/vasm-x64.h"

namespace HPHP { namespace jit {

struct RegAllocInfo;

enum class SyncOptions {
  kNoSyncPoint,
  kSyncPoint,
  kSyncPointAdjustOne,
  kSmashableAndSyncPoint,
};

typedef StateVector<IRInstruction, RegSet> LiveRegs;

/*
 * CatchInfo is used to pass information from call instructions to the catch
 * block they're linked with. vasm has made most of this unnecessary; afterCall
 * and savedRegs are only used in the non-vasm ARM backend.
 */
struct CatchInfo {
  /* rspOffset is the number of bytes pushed on the C++ stack for the call,
   * for functions with stack arguments. The catch trace will adjust rsp
   * by this amount before executing the catch block */
  Offset rspOffset;
};

// Stuff we need to preserve between blocks while generating code,
// and address information produced during codegen.
struct CodegenState {
  CodegenState(const IRUnit& unit, AsmInfo* asmInfo)
    : asmInfo(asmInfo)
    , catches(unit, CatchInfo())
    , labels(unit, Vlabel())
    , locs(unit, Vloc{})
  {}

  // True if this block's terminal Jmp has a desination equal to the
  // next block in the same assmbler.
  bool noTerminalJmp;

  // Output: start/end ranges of machine code addresses of each instruction.
  AsmInfo* asmInfo;

  // Used to pass information about the state of the world at native
  // calls between cgCallHelper and cgBeginCatch.
  StateVector<Block, CatchInfo> catches;

  // Have we progressed past the guards? Used to suppress TransBCMappings until
  // we're translating code that can properly be attributed to specific
  // bytecode.
  bool pastGuards{false};

  // Postponed code "points" can obtain code addresses after Vasm::finish().
  Vmeta meta;

  // vasm block labels, one for each hhir block
  StateVector<Block,Vlabel> labels;

  // vlocs for each SSATmp used or defined in reachable blocks
  StateVector<SSATmp,Vloc> locs;
};

// Allocate registers and generate machine code. Mutates the global
// singleton MCGenerator (adds code, allocates data, adds fixups).
void genCode(IRUnit&);

}}

#endif
