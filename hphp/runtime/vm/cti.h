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

#ifndef _incl_HPHP_RUNTIME_VM_CTI_H
#define _incl_HPHP_RUNTIME_VM_CTI_H

#include "hphp/util/data-block.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/rds-header.h"
#include "hphp/util/asm-x64.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/code-cache.h"
#include "hphp/runtime/vm/jit/tc-internal.h"

#if defined(__x86_64__)
#define CTI_SUPPORTED 1
#endif

namespace HPHP {
struct Func;
struct ExecutionContext;

enum class ExecMode : uint32_t { Normal=0, BB=1, Debugger=2, Coverage=4 };

// ip is always a native code address, pc can either be a virtual pc
// or a packed JitResumeAddr. Passed and stored by value.
struct PcPair { CodeAddress ip; PC pc; };

// signature for g_enterBytecode
using EntryStub = uint64_t(*)(ExecMode, PcPair);

inline ExecMode operator|(ExecMode m1, ExecMode m2) {
  return ExecMode(int(m1) | int(m2));
}
inline int operator&(ExecMode m1, ExecMode m2) {
  return int(m1) & int(m2);
}

// branch bytecodes either set pc to the next bytecode or a single
// known target
constexpr inline bool isBranch(Op opcode) {
  return isConditionalJmp(opcode) ||
         opcode == OpIterInit || opcode == OpLIterInit ||
         opcode == OpIterNext || opcode == OpLIterNext;
}

// simple bytecodes always move pc to the next bytecode.
constexpr inline bool isSimple(Op opcode) {
  return !instrIsControlFlow(opcode) && !instrCanHalt(opcode);
}

// these never return
constexpr inline bool isThrow(Op opcode) {
  return opcode == OpThrow ||
         opcode == OpExit ||
         opcode == OpThrowAsTypeStructException ||
         opcode == OpContRaise ||
         opcode == OpFatal;
}

inline CodeBlock& cti_code() {
  return jit::tc::code().bytecode();
}

void compile_cti_stubs();
Offset compile_cti(Func*, PC unitpc);
void free_cti(Offset cti_entry, uint32_t nbytes);

// slow path to look up the native pc
jit::TCA lookup_cti(const Func*, Offset cti_entry, PC pc, PC unitpc);

inline bool cti_enabled() {
  return jit::CodeCache::ABytecodeSize != 0;
}

// global data
extern EntryStub g_enterCti;
extern CodeAddress g_exitCti;
extern const CodeAddress cti_ops[];
extern const CodeAddress ctid_ops[];
extern const CodeAddress updateCoverageFunc;

constexpr auto kCtiIndirectJmpSize = 2;

}
#endif
