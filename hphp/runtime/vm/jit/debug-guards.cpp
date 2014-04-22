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

#include "hphp/runtime/vm/jit/debug-guards.h"

#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/service-requests-inline.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/back-end-x64.h" // XXX Layering violation.

namespace HPHP { namespace JIT {

static constexpr size_t dbgOff =
  offsetof(ThreadInfo, m_reqInjectionData) +
  RequestInjectionData::debuggerReadOnlyOffset();

//////////////////////////////////////////////////////////////////////

void addDbgGuardImpl(SrcKey sk, SrcRec* srcRec) {
  TCA realCode = srcRec->getTopTranslation();
  if (!realCode) {
    // no translations, nothing to do
    return;
  }
  TCA dbgGuard = mcg->code.main().frontier();

  mcg->backEnd().addDbgGuard(mcg->code.main(), mcg->code.stubs(), sk, dbgOff);

  // Emit a jump to the actual code
  //
  // XXX kJmpLen access here is a layering violation.
  mcg->backEnd().prepareForSmash(mcg->code.main(), X64::kJmpLen);
  TCA dbgBranchGuardSrc = mcg->code.main().frontier();
  mcg->backEnd().emitSmashableJump(mcg->code.main(), realCode, CC_None);

  // Add it to srcRec
  srcRec->addDebuggerGuard(dbgGuard, dbgBranchGuardSrc);
}

}}
