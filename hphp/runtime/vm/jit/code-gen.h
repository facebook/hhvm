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
#include "hphp/runtime/vm/jit/vasm.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

namespace HPHP { namespace jit {
///////////////////////////////////////////////////////////////////////////////

struct TransLoc;

enum class SyncOptions {
  kNoSyncPoint,
  kSyncPoint,
  kSyncPointAdjustOne,
  kSmashableAndSyncPoint,
};

enum class CatchCall {
  Uninit,
  PHP,
  CPP,
};

// Stuff we need to preserve between blocks while generating code,
// and address information produced during codegen.
struct CodegenState {
  CodegenState(const IRUnit& unit, AsmInfo* asmInfo, CodeBlock& frozen)
    : unit(unit)
    , asmInfo(asmInfo)
    , frozen(frozen)
    , catch_offsets(unit, 0)
    , catch_calls(unit, CatchCall::Uninit)
    , labels(unit, Vlabel())
    , locs(unit, Vloc{})
  {}

  const IRUnit& unit;

  // Output: start/end ranges of machine code addresses of each instruction.
  AsmInfo* asmInfo;

  // Frozen code section, when we need to eagerly generate stubs
  CodeBlock& frozen;

  // Each catch block needs to know the number of bytes pushed at the
  // callsite so it can fix rsp before executing the catch block.
  StateVector<Block,Offset> catch_offsets;

  // Catch blocks that are targets of php calls (bindcall, contenter, callstub)
  // are handled specially. This StateVector is used to propagate information
  // from the cg* function that detects this situation to cgBeginCatch, which
  // encodes the information in the landingpad{} instruction.
  StateVector<Block,CatchCall> catch_calls;

  // Have we progressed past the guards? Used to suppress TransBCMappings until
  // we're translating code that can properly be attributed to specific
  // bytecode.
  bool pastGuards{false};

  // vasm block labels, one for each hhir block
  StateVector<Block,Vlabel> labels;

  // vlocs for each SSATmp used or defined in reachable blocks
  StateVector<SSATmp,Vloc> locs;
};

// Generate machine code; converts to vasm, optionally converts to llvm,
// further optimizes, emits code into main/cold/frozen sections, allocates rds
// and global data, and adds fixup metadata.
void genCode(IRUnit& unit, CodeKind kind = CodeKind::Trace);

///////////////////////////////////////////////////////////////////////////////
}}

#endif
