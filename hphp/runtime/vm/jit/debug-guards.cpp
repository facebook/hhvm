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

#include "hphp/runtime/vm/jit/debug-guards.h"

#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/jit/smashable-instr.h"
#include "hphp/runtime/vm/jit/types.h"

namespace HPHP { namespace jit {

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

  mcg->backEnd().addDbgGuard(mcg->code.main(), mcg->code.cold(), sk, dbgOff);

  // Emit a jump to the actual code.
  auto const dbgBranchGuardSrc =
    emitSmashableJmp(mcg->code.main(), realCode);

  // Add it to srcRec.
  srcRec->addDebuggerGuard(dbgGuard, dbgBranchGuardSrc);
}

}}
