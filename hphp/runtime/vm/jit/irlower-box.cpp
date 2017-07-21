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

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/arg-group.h"
#include "hphp/runtime/vm/jit/call-spec.h"
#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/extra-data.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/util/trace.h"

namespace HPHP { namespace jit { namespace irlower {

TRACE_SET_MOD(irlower);

///////////////////////////////////////////////////////////////////////////////

void cgBoxPtr(IRLS& env, const IRInstruction* inst) {
  auto const base = srcLoc(env, inst, 0).reg();
  auto const dst = dstLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  irlower::emitTypeTest(
    v, env, TBoxedCell, base[TVOFF(m_type)], base[TVOFF(m_data)], v.makeReg(),
    [&](ConditionCode cc, Vreg sf) {
      cond(v, cc, sf, dst, [&](Vout& /*v*/) { return base; },
           [&](Vout& v) {
             auto const args = argGroup(env, inst).ssa(0 /* addr */);
             auto const ret = v.makeReg();
             cgCallHelper(v, env, CallSpec::direct(tvBox), callDest(ret),
                          SyncOptions::None, args);
             return ret;
           });
    });
}

void cgUnboxPtr(IRLS& env, const IRInstruction* inst) {
  auto const src = srcLoc(env, inst, 0).reg();
  auto const dst = dstLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const sf = v.makeReg();
  emitCmpTVType(v, sf, KindOfRef, src[TVOFF(m_type)]);

  if (RefData::tvOffset() == 0) {
    v << cloadq{CC_E, sf, src, src[TVOFF(m_data)], dst};
    return;
  }

  auto const ref_ptr = v.makeReg();
  auto const cell_ptr = v.makeReg();
  v << load{src[TVOFF(m_data)], ref_ptr};
  v << lea{ref_ptr[RefData::tvOffset()], cell_ptr};
  v << cmovq{CC_E, sf, src, cell_ptr, dst};
}

IMPL_OPCODE_CALL(Box)

///////////////////////////////////////////////////////////////////////////////

}}}
