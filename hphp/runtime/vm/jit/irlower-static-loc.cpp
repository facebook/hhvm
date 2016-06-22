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

#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/header-kind.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/ref-data.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/extra-data.h"
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

void cgLdStaticLoc(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<LdStaticLoc>();
  auto const link = rds::bindStaticLocal(extra->func, extra->name);
  auto const dst = dstLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const sf = checkRDSHandleInitialized(v, link.handle());
  fwdJcc(v, env, CC_NE, sf, inst->taken());
  v << lea{rvmtl()[link.handle()], dst};
}

void cgInitStaticLoc(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<InitStaticLoc>();
  auto const link = rds::bindStaticLocal(extra->func, extra->name);
  auto const dst = dstLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  // If we're here, the target-cache-local RefData is all zeros, so we can
  // initialize it by storing the new value into it's TypedValue and
  // incrementing the RefData reference count (which will set it to 1).
  //
  // We are storing the src value into the static, but we don't need to incref
  // it because it's a bytecode invariant that it's not a reference counted
  // type.
  v << lea{rvmtl()[link.handle()], dst};
  storeTV(v, dst[RefData::tvOffset()], srcLoc(env, inst, 0), inst->src(0));
  v << storeli{1, dst[FAST_REFCOUNT_OFFSET]};
  v << storebi{0, dst[RefData::cowZOffset()]};
  v << storebi{uint8_t(HeaderKind::Ref), dst[HeaderKindOffset]};
  markRDSHandleInitialized(v, link.handle());

  static_assert(sizeof(HeaderKind) == 1, "");
}

///////////////////////////////////////////////////////////////////////////////

void cgCheckClosureStaticLocInit(IRLS& env, const IRInstruction* inst) {
  auto const src = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const sf = v.makeReg();
  emitCmpTVType(v, sf, KindOfUninit, src[RefData::tvOffset() + TVOFF(m_type)]);
  v << jcc{CC_E, sf, {label(env, inst->next()), label(env, inst->taken())}};
}

IMPL_OPCODE_CALL(LdClosureStaticLoc)

void cgInitClosureStaticLoc(IRLS& env, const IRInstruction* inst) {
  auto const ref = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  always_assert(!srcLoc(env, inst, 1).isFullSIMD());
  storeTV(v, ref[RefData::tvOffset()], srcLoc(env, inst, 1), inst->src(1));
}

///////////////////////////////////////////////////////////////////////////////

}}}
