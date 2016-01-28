/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/arch.h"
#include "hphp/runtime/base/request-injection-data.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/vm/srckey.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/code-gen-tls.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/smashable-instr.h"
#include "hphp/runtime/vm/jit/srcdb.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

void addDbgGuardImpl(SrcKey sk, SrcRec* sr) {
  TCA realCode = sr->getTopTranslation();
  if (!realCode) return;  // No translations, nothing to do.

  auto& cb = mcg->code.main();

  auto const dbgGuard = vwrap(cb, [&] (Vout& v) {
    if (!sk.resumed()) {
      auto const off = sr->nonResumedSPOff();
      v << lea{rvmfp()[-cellsToBytes(off.offset)], rvmsp()};
    }

    auto const tinfo = v.makeReg();
    auto const attached = v.makeReg();
    auto const sf = v.makeReg();

    auto const done = v.makeBlock();

    constexpr size_t dbgOff =
      offsetof(ThreadInfo, m_reqInjectionData) +
      RequestInjectionData::debuggerReadOnlyOffset();

    v << ldimmq{reinterpret_cast<uintptr_t>(sk.pc()), rarg(0)};

    emitTLSLoad(v, tls_datum(ThreadInfo::s_threadInfo), tinfo);
    v << loadb{tinfo[dbgOff], attached};
    v << testbi{static_cast<int8_t>(0xffu), attached, sf};

    v << jcci{CC_NZ, sf, done, mcg->tx().uniqueStubs.interpHelper};

    v = done;
    v << fallthru{};
  }, CodeKind::Helper);

  // Emit a jump to the actual code.
  auto const dbgBranchGuardSrc = emitSmashableJmp(cb, realCode);

  // Add the guard to the SrcRec.
  sr->addDebuggerGuard(dbgGuard, dbgBranchGuardSrc);
}

///////////////////////////////////////////////////////////////////////////////

}}
