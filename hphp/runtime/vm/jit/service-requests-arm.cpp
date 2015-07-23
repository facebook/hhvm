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

#include "hphp/vixl/a64/macro-assembler-a64.h"

#include <folly/Optional.h>

#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/vm/jit/abi-arm.h"
#include "hphp/runtime/vm/jit/code-gen-helpers-arm.h"
#include "hphp/runtime/vm/jit/back-end.h"
#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/vasm-print.h"

namespace HPHP { namespace jit { namespace arm {

using namespace vixl;

//////////////////////////////////////////////////////////////////////

void emitBindJ(CodeBlock& cb, CodeBlock& frozen, ConditionCode cc,
               SrcKey dest) {
  TCA toSmash = cb.frontier();
  if (cb.base() == frozen.base()) {
    // This is just to reserve space. We'll overwrite with the real dest later.
    mcg->backEnd().emitSmashableJump(cb, toSmash, cc);
  }

  mcg->setJmpTransID(toSmash);

  TCA sr = svcreq::emit_ephemeral(
    frozen,
    mcg->getFreeStub(frozen, &mcg->cgFixups()),
    folly::none,
    REQ_BIND_JMP,
    toSmash,
    dest.toAtomicInt(),
    TransFlags{}.packed
  );

  MacroAssembler a { cb };
  if (cb.base() == frozen.base()) {
    UndoMarker um {cb};
    cb.setFrontier(toSmash);
    mcg->backEnd().emitSmashableJump(cb, sr, cc);
    um.undo();
  } else {
    mcg->backEnd().emitSmashableJump(cb, sr, cc);
  }
}

}}}
