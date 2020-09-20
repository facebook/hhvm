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

#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/vm/act-rec.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/resumable.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/arg-group.h"
#include "hphp/runtime/vm/jit/bc-marker.h"
#include "hphp/runtime/vm/jit/call-spec.h"
#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/extra-data.h"
#include "hphp/runtime/vm/jit/fixup.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/target-profile.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/util/trace.h"

#include <folly/Optional.h>

namespace HPHP { namespace jit { namespace irlower {

TRACE_SET_MOD(irlower);

///////////////////////////////////////////////////////////////////////////////

namespace {

///////////////////////////////////////////////////////////////////////////////

template<class ExtraData>
Vreg adjustSPForReturn(IRLS& env, const IRInstruction* inst) {
  auto const sp = srcLoc(env, inst, 0).reg();
  auto const adjust = inst->extra<ExtraData>()->offset.offset;
  auto& v = vmain(env);

  auto const sync_sp = v.makeReg();
  v << lea{sp[cellsToBytes(adjust)], sync_sp};
  v << syncvmsp{sync_sp};

  return sync_sp;
}

/*
 * Take the return value, given in `retVal' and `retLoc', and pack it into the
 * ABI-specified return registers.
 */
void prepare_return_regs(Vout& v, SSATmp* retVal, Vloc retLoc,
                         const AuxUnion& aux) {
  using u_data_type = std::make_unsigned<data_type_t>::type;

  auto const tp = [&] {
    auto const mask = [&] {
      if (!aux.u_raw) return uint64_t{};
      if (aux.u_raw == static_cast<uint32_t>(-1)) {
        return static_cast<uint64_t>(-1) <<
          std::numeric_limits<u_data_type>::digits;
      }
      return uint64_t{aux.u_raw} << 32;
    }();

    if (!retLoc.hasReg(1)) {
      auto const dt = static_cast<u_data_type>(retVal->type().toDataType());
      static_assert(std::numeric_limits<u_data_type>::digits <= 32, "");
      return v.cns(dt | mask);
    }

    auto const type = retLoc.reg(1);
    auto const extended = v.makeReg();
    auto const result = v.makeReg();

    // DataType is signed. We're using movzbq here to clear out the upper 7
    // bytes of the register, not to actually extend the type value.
    v << movzbq{type, extended};
    v << orq{extended, v.cns(mask), result, v.makeReg()};
    return result;
  }();
  auto const data = zeroExtendIfBool(v, retVal->type(), retLoc.reg(0));

  v << syncvmret{data, tp};
}

void asyncFuncRetImpl(IRLS& env, const IRInstruction* inst, TCA target) {
  auto const ret = inst->src(2);
  auto const retLoc = srcLoc(env, inst, 2);
  auto& v = vmain(env);

  adjustSPForReturn<IRSPRelOffsetData>(env, inst);

  // The asyncFuncRet{,Slow} stubs take the return TV as its arguments.
  copyTV(v, rarg(0), rarg(1), retLoc, ret);
  auto args = vm_regs_with_sp() | rarg(1);
  if (!ret->isA(TNull)) args |= rarg(0);

  v << jmpi{target, args};
}

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

void traceRet(ActRec* fp, TypedValue* sp, void* rip) {
  if (rip == tc::ustubs().callToExit) return;
  checkFrame(fp, sp, false /* fullCheck */);
  assertx(sp <= (TypedValue*)fp || isResumed(fp));
}

void cgRetCtrl(IRLS& env, const IRInstruction* inst) {
  auto const fp = srcLoc(env, inst, 1).reg();
  auto const sync_sp = adjustSPForReturn<RetCtrlData>(env, inst);
  auto& v = vmain(env);

  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    auto rip = v.makeReg();
    auto prev_fp = v.makeReg();
    v << load{fp[AROFF(m_savedRip)], rip};
    v << load{fp[AROFF(m_sfp)], prev_fp};

    v << vcall{
      CallSpec::direct(traceRet),
      v.makeVcallArgs({{prev_fp, sync_sp, rip}}),
      v.makeTuple({})
    };
  }

  prepare_return_regs(v, inst->src(2), srcLoc(env, inst, 2),
                      inst->extra<RetCtrl>()->aux);
  v << phpret{fp, php_return_regs()};
}

void cgAsyncFuncRet(IRLS& env, const IRInstruction* inst) {
  asyncFuncRetImpl(env, inst, tc::ustubs().asyncFuncRet);
}

void cgAsyncFuncRetSlow(IRLS& env, const IRInstruction* inst) {
  asyncFuncRetImpl(env, inst, tc::ustubs().asyncFuncRetSlow);
}

void cgAsyncSwitchFast(IRLS& env, const IRInstruction* inst) {
  auto& v = vmain(env);
  adjustSPForReturn<IRSPRelOffsetData>(env, inst);
  v << jmpi{tc::ustubs().asyncSwitchCtrl, vm_regs_with_sp()};
}

///////////////////////////////////////////////////////////////////////////////

void cgLdRetVal(IRLS& env, const IRInstruction* inst) {
  auto const fp = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);
  loadTV(v, inst->dst(), dstLoc(env, inst, 0), fp[kArRetOff]);
}

void cgDbgTrashRetVal(IRLS& env, const IRInstruction* inst) {
  auto& v = vmain(env);
  trashFullTV(v, srcLoc(env, inst, 0).reg()[kArRetOff], kTVTrashJITRetVal);
}

///////////////////////////////////////////////////////////////////////////////

void cgGenericRetDecRefs(IRLS& env, const IRInstruction* inst) {
  auto const fp = srcLoc(env, inst, 0).reg();
  auto const& marker = inst->marker();
  auto const numLocals = marker.func()->numLocals();
  auto& v = vmain(env);

  // TODO: assert that fp is the same value stored in rvmfp here.

  if (numLocals == 0) return;

  auto const target = numLocals > kNumFreeLocalsHelpers
    ? tc::ustubs().freeManyLocalsHelper
    : tc::ustubs().freeLocalsHelpers[numLocals - 1];

  auto const startType = v.makeReg();
  auto const startData = v.makeReg();
  v << lea{ptrToLocalType(fp, numLocals - 1), startType};
  v << lea{ptrToLocalData(fp, numLocals - 1), startData};

  auto const fix = Fixup{
    marker.bcOff() - marker.func()->base(),
    marker.spOff().offset
  };
  // The stub uses arg reg 0 as scratch and to pass arguments to
  // destructors, so it expects the starting pointers in arg reg 1 and
  // 2.
  auto const args =
    v.makeVcallArgs({{v.cns(Vconst::Quad), startType, startData}});
  v << vcall{CallSpec::stub(target), args, v.makeTuple({}),
             fix, DestType::None, false};
}

///////////////////////////////////////////////////////////////////////////////

}}}
