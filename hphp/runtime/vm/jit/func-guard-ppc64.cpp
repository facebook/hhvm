/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | (c) Copyright IBM Corporation 2015-2016                              |
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

#include "hphp/runtime/vm/jit/func-guard-ppc64.h"

#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/abi-ppc64.h"
#include "hphp/runtime/vm/jit/smashable-instr-ppc64.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"

#include "hphp/ppc64-asm/asm-ppc64.h"
#include "hphp/util/data-block.h"
#include "hphp/util/immed.h"

namespace HPHP { namespace jit { namespace ppc64 {

///////////////////////////////////////////////////////////////////////////////

namespace {

///////////////////////////////////////////////////////////////////////////////

// funcGuard: Immediate, ld, cmpd, kJccLen
// it needs to rewind all of those instructions but pointing to the first.
constexpr auto kFuncGuardLen = smashableMovqLen()
  + 2 * ppc64_asm::instr_size_in_bytes + ppc64_asm::Assembler::kJccLen;

ALWAYS_INLINE bool isPrologueStub(TCA addr) {
  return addr == tc::ustubs().fcallHelperThunk;
}

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

void emitFuncGuard(const Func* func, CodeBlock& cb, CGMeta& fixups) {
  ppc64_asm::Assembler a { cb };

  const auto tmp1 = ppc64_asm::reg::r3;
  const auto tmp2 = ppc64_asm::reg::r4;

  assertx(ppc64::abi(CodeKind::CrossTrace).gpUnreserved.contains(tmp1));
  assertx(ppc64::abi(CodeKind::CrossTrace).gpUnreserved.contains(tmp2));

  emitSmashableMovq(a.code(), fixups, uint64_t(func), tmp1);
  a.  ld     (tmp2, rvmfp()[AROFF(m_func)]);
  a.  cmpd   (tmp1, tmp2);

  a.  branchFar(tc::ustubs().funcPrologueRedispatch,
                  ppc64_asm::BranchConditions::NotEqual,
                  ppc64_asm::ImmType::TocOnly);

  DEBUG_ONLY auto guard = funcGuardFromPrologue(a.frontier(), func);
  assertx(funcGuardMatches(guard, func));
}

TCA funcGuardFromPrologue(TCA prologue, const Func* /*func*/) {
  if (isPrologueStub(prologue)) return prologue;
  return (prologue - kFuncGuardLen);
}

bool funcGuardMatches(TCA guard, const Func* func) {
  if (isPrologueStub(guard)) return false;

  const ppc64_asm::DecodedInstruction di(guard);
  auto const ifunc = reinterpret_cast<uintptr_t>(func);
  return static_cast<uintptr_t>(di.immediate()) == ifunc;
}

void clobberFuncGuard(TCA guard, const Func* /*func*/) {
  smashMovq(guard, 0);
}

///////////////////////////////////////////////////////////////////////////////

}}}
