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

#include "hphp/runtime/vm/jit/func-guard-arm.h"
#include "hphp/runtime/vm/jit/func-guard-x64.h"

#include "hphp/runtime/vm/jit/abi-x64.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/data-block.h"
#include "hphp/util/immed.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

namespace x64 {

///////////////////////////////////////////////////////////////////////////////

void emitFuncGuard(const Func* func, CodeBlock& cb) {
  using namespace reg;
  X64Assembler a { cb };

  assertx(cross_trace_abi.gpUnreserved.contains(rax));

  auto const funcImm = Immed64(func);
  int nbytes, offset;

  if (funcImm.fits(sz::dword)) {
    nbytes = kFuncGuardShortSmash;
    offset = kFuncGuardShortImm;
  } else {
    nbytes = kFuncGuardSmash;
    offset = kFuncGuardImm;
  }
  mcg->backEnd().prepareForSmash(a.code(), nbytes, offset);

  TCA start DEBUG_ONLY = a.frontier();

  if (funcImm.fits(sz::dword)) {
    a.  cmpq   (funcImm.l(), rVmFp[AROFF(m_func)]);
  } else {
    // Although func doesn't fit in a signed 32-bit immediate, it may still fit
    // in an unsigned one.  Rather than deal with yet another case (which only
    // happens when we disable jemalloc), just force it to be an 8-byte
    // immediate, and patch it up afterwards.
    a.  movq   (0xdeadbeeffeedface, rax);

    auto immptr = reinterpret_cast<uintptr_t*>(a.frontier()) - 1;
    assertx(*immptr == 0xdeadbeeffeedface);
    *immptr = uintptr_t(func);

    a.  cmpq   (rax, rVmFp[AROFF(m_func)]);
  }
  a.    jnz    (mcg->tx().uniqueStubs.funcPrologueRedispatch);

  assertx(funcPrologueToGuard(a.frontier(), func) == start);
  assertx(funcPrologueHasGuard(a.frontier(), func));
}

///////////////////////////////////////////////////////////////////////////////

} // x64

namespace arm {

///////////////////////////////////////////////////////////////////////////////

void emitFuncGuard(const Func* func, CodeBlock& cb) {
  not_implemented();
}

///////////////////////////////////////////////////////////////////////////////

} // arm

///////////////////////////////////////////////////////////////////////////////

}}
