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

#include "hphp/runtime/vm/jit/irlower-internal.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/runtime/ext/std/ext_std_closure.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/trace.h"

namespace HPHP { namespace jit { namespace irlower {

TRACE_SET_MOD(irlower);

///////////////////////////////////////////////////////////////////////////////

void cgLdClosure(IRLS& env, const IRInstruction* inst) {
  assertx(!inst->func() || inst->func()->isClosureBody());
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const fp = srcLoc(env, inst, 0).reg();
  vmain(env) << load{fp[AROFF(m_thisUnsafe)], dst};
}

void cgLdClosureCtx(IRLS& env, const IRInstruction* inst) {
  auto const ctx = dstLoc(env, inst, 0).reg();
  auto const obj = srcLoc(env, inst, 0).reg();
  vmain(env) << load{obj[c_Closure::ctxOffset()], ctx};
}

void cgStClosureCtx(IRLS& env, const IRInstruction* inst) {
  auto const obj = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  if (inst->src(1)->isA(TNullptr)) {
    v << storeqi{0, obj[c_Closure::ctxOffset()]};
  } else {
    auto const ctx = srcLoc(env, inst, 1).reg();
    v << store{ctx, obj[c_Closure::ctxOffset()]};
  }
}

void cgStClosureArg(IRLS& env, const IRInstruction* inst) {
  auto const obj = srcLoc(env, inst, 0).reg();
  auto const off = inst->extra<StClosureArg>()->offsetBytes;
  storeTV(vmain(env), obj[off], srcLoc(env, inst, 1), inst->src(1));
}

///////////////////////////////////////////////////////////////////////////////

}}}
