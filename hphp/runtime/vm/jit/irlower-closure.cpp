/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/ext/core/ext_core_closure.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/trace.h"

namespace HPHP::jit::irlower {

TRACE_SET_MOD(irlower);

///////////////////////////////////////////////////////////////////////////////

void cgLdClosureCls(IRLS& env, const IRInstruction* inst) {
  auto const ctx = dstLoc(env, inst, 0).reg();
  auto const obj = srcLoc(env, inst, 0).reg();
  vmain(env) << load{obj[c_Closure::ctxOffset()], ctx};
}

void cgLdClosureThis(IRLS& env, const IRInstruction* inst) {
  auto const ctx = dstLoc(env, inst, 0).reg();
  auto const obj = srcLoc(env, inst, 0).reg();
  vmain(env) << load{obj[c_Closure::ctxOffset()], ctx};
}

void cgStClosureArg(IRLS& env, const IRInstruction* inst) {
  auto const obj = srcLoc(env, inst, 0).reg();
  auto const offsets = ObjectProps::offsetOf(inst->extra<StClosureArg>()->index)
    .shift(sizeof(ObjectData));
  storeTV(vmain(env),
          inst->src(1)->type(),
          srcLoc(env, inst, 1),
          obj[offsets.typeOffset()],
          obj[offsets.dataOffset()]);
}

///////////////////////////////////////////////////////////////////////////////

}
