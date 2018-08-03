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

#include "hphp/runtime/base/ref-data.h"
#include "hphp/runtime/base/tv-mutate.h"
#include "hphp/runtime/base/tv-variant.h"

#include "hphp/runtime/vm/jit/arg-group.h"
#include "hphp/runtime/vm/jit/call-spec.h"
#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/extra-data.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/util/trace.h"

namespace HPHP { namespace jit { namespace irlower {

TRACE_SET_MOD(irlower);

///////////////////////////////////////////////////////////////////////////////

TypedValue* tvBoxHelper(TypedValue* tv) { tvBox(tv); return tv; }
tv_lval tvBoxHelper(tv_lval tv) { tvBox(tv); return tv; }

void cgBoxPtr(IRLS& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  auto const srcLoc = irlower::srcLoc(env, inst, 0);
  auto const dst = dstLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  emitTypeTest(
    v, env, TBoxedCell,
    memTVTypePtr(src, srcLoc), memTVValPtr(src, srcLoc), v.makeReg(),
    [&](ConditionCode cc, Vreg sf) {
      cond(v, cc, sf, dst, [&](Vout& /*v*/) { return srcLoc.reg(); },
           [&](Vout& v) {
             auto const args = argGroup(env, inst).ssa(0 /* addr */);
             auto const ret = v.makeReg();
             auto const target = [&] {
               if (src->isA(TPtrToGen)) {
                 TypedValue* (*f)(TypedValue*) = tvBoxHelper;
                 return CallSpec::direct(f);
               }
               tv_lval (*f)(tv_lval) = tvBoxHelper;
               return CallSpec::direct(f);
             }();
             cgCallHelper(v, env, target,
                          callDest(ret), SyncOptions::None, args);
             return ret;
           });
    });
}

void cgUnboxPtr(IRLS& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  auto const dst = inst->dst();
  auto const wide = wide_tv_val && dst->isA(TLvalToGen);
  assertx((src->isA(TPtrToGen) && dst->isA(TPtrToGen)) ||
          (src->isA(TLvalToGen) && dst->isA(TLvalToGen)));

  auto const srcLoc = irlower::srcLoc(env, inst, 0);
  auto const dstLoc = irlower::dstLoc(env, inst, 0);
  auto const valIdx = wide ? tv_lval::val_idx : 0;
  auto& v = vmain(env);

  auto const sf = v.makeReg();
  auto const type_ptr = memTVTypePtr(src, srcLoc);
  emitCmpTVType(v, sf, KindOfRef, type_ptr);

  auto const val_ptr = memTVValPtr(src, srcLoc);
  if (RefData::tvOffset() == 0) {
    v << cloadq{CC_E, sf, srcLoc.reg(valIdx), val_ptr, dstLoc.reg(valIdx)};
    if (wide) {
      static_assert(TVOFF(m_data) == 0, "");
      auto const ref_type = v.makeReg();
      v << lea{dstLoc.reg(valIdx)[TVOFF(m_type)], ref_type};
      v << cmovq{CC_E, sf, srcLoc.reg(tv_lval::type_idx),
                 ref_type, dstLoc.reg(tv_lval::type_idx)};
    }
    return;
  }

  auto const ref_ptr = v.makeReg();
  auto const cell_ptr = v.makeReg();
  v << load{val_ptr, ref_ptr};
  v << lea{ref_ptr[RefData::tvOffset()], cell_ptr};
  v << cmovq{CC_E, sf, srcLoc.reg(valIdx), cell_ptr, dstLoc.reg(valIdx)};
  if (wide) {
    static_assert(TVOFF(m_data) == 0, "");
    auto const ref_type = v.makeReg();
    v << lea{cell_ptr[TVOFF(m_type)], ref_type};
    v << cmovq{CC_E, sf, srcLoc.reg(tv_lval::type_idx),
               ref_type, dstLoc.reg(tv_lval::type_idx)};
  }
}

IMPL_OPCODE_CALL(Box)

///////////////////////////////////////////////////////////////////////////////

}}}
