/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/vm/jit/debug-guards.h"

#include "hphp/util/asm-x64.h"
#include "hphp/vixl/a64/macro-assembler-a64.h"
#include "hphp/runtime/vm/jit/abi-arm.h"
#include "hphp/runtime/vm/jit/code-gen-helpers-arm.h"
#include "hphp/runtime/vm/jit/code-gen-helpers-x64.h"
#include "hphp/runtime/vm/jit/jump-smash.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/translator-x64.h"
#include "hphp/runtime/vm/jit/types.h"

namespace HPHP { namespace JIT {


static constexpr size_t dbgOff =
  offsetof(ThreadInfo, m_reqInjectionData) +
  RequestInjectionData::debuggerReadOnlyOffset();

//////////////////////////////////////////////////////////////////////

namespace X64 {

void addDbgGuardImpl(SrcKey sk) {
  Asm a { tx64->mainCode };

  // Emit the checks for debugger attach
  emitTLSLoad<ThreadInfo>(a, ThreadInfo::s_threadInfo, reg::rAsm);
  a.   load_reg64_disp_reg32(reg::rAsm, dbgOff, reg::rAsm);
  a.   testb((int8_t)0xff, rbyte(reg::rAsm));

  // Branch to a special REQ_INTERPRET if attached
  TCA fallback =
    emitServiceReq(tx64->stubsCode, REQ_INTERPRET, sk.offset(), 0);
  a. jnz(fallback);
}

}

//////////////////////////////////////////////////////////////////////

namespace ARM {

void addDbgGuardImpl(SrcKey sk) {
  vixl::MacroAssembler a { tx64->mainCode };

  vixl::Label after;
  vixl::Label interpReqAddr;

  // Get the debugger-attached flag from thread-local storage. Don't bother
  // saving caller-saved regs around the host call; this is between blocks.
  emitTLSLoad<ThreadInfo>(a, ThreadInfo::s_threadInfo, rAsm);

  // Is the debugger attached?
  a.   Ldr  (rAsm.W(), rAsm[dbgOff]);
  a.   Tst  (rAsm, 0xff);
  // skip jump to stubs if no debugger attached
  a.   B    (&after, vixl::eq);
  a.   Ldr  (rAsm, &interpReqAddr);
  a.   Br   (rAsm);
  if (!a.isFrontierAligned(8)) {
    a. Nop  ();
    assert(a.isFrontierAligned(8));
  }
  a.   bind (&interpReqAddr);
  TCA interpReq =
    emitServiceReq(tx64->stubsCode, REQ_INTERPRET, sk.offset(), 0);
  a.   dc64 (interpReq);
  a.   bind (&after);
}

}

//////////////////////////////////////////////////////////////////////

void addDbgGuardImpl(SrcKey sk, SrcRec* srcRec) {
  TCA dbgGuard = tx64->mainCode.frontier();

  switch (arch()) {
    case Arch::X64:
      X64::addDbgGuardImpl(sk);
      prepareForSmash(tx64->mainCode, X64::kJmpLen);
      break;
    case Arch::ARM:
      ARM::addDbgGuardImpl(sk);
      break;
    default:
      not_implemented();
  }

  // Emit a jump to the actual code
  TCA realCode = srcRec->getTopTranslation();
  TCA dbgBranchGuardSrc = tx64->mainCode.frontier();
  emitSmashableJump(tx64->mainCode, realCode, CC_None);

  // Add it to srcRec
  srcRec->addDebuggerGuard(dbgGuard, dbgBranchGuardSrc);
}

}}
