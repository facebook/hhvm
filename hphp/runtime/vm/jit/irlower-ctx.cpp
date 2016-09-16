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
#include "hphp/runtime/vm/jit/bc-marker.h"
#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/trace.h"

namespace HPHP { namespace jit { namespace irlower {

TRACE_SET_MOD(irlower);

///////////////////////////////////////////////////////////////////////////////

void cgLdClsCtx(IRLS& env, const IRInstruction* inst) {
  auto dst = dstLoc(env, inst, 0).reg();
  auto ctx = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const sf = v.makeReg();
  v << testqi{ActRec::kHasClassBit, ctx, sf};

  unlikelyCond(v, vcold(env), CC_NZ, sf, dst,
    [&] (Vout& v) { return emitLdClsCctx(v, ctx, v.makeReg()); }, // Cctx
    [&] (Vout& v) { return emitLdObjClass(v, ctx, v.makeReg()); } // This
  );
}

void cgLdClsCctx(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const cctx = srcLoc(env, inst, 0).reg();
  emitLdClsCctx(vmain(env), cctx, dst);
}

void cgConvClsToCctx(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const cls = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);
  v << orqi{ActRec::kHasClassBit, cls, dst, v.makeReg()};
}

void cgCheckCtxThis(IRLS& env, const IRInstruction* inst) {
  auto const ctx = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const func = inst->marker().func();
  if (!func->mayHaveThis()) {
    v << jmp{label(env, inst->taken())};
    return;
  }

  auto const sf = v.makeReg();
  v << testqi{ActRec::kHasClassBit, ctx, sf};
  v << jcc{CC_NZ, sf, {label(env, inst->next()), label(env, inst->taken())}};
}

///////////////////////////////////////////////////////////////////////////////

}}}
