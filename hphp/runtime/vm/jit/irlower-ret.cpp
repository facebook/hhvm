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
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/vm/act-rec.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/func.h"

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

Vreg adjustSPForReturn(IRLS& env, const IRInstruction* inst) {
  auto const sp = srcLoc(env, inst, 0).reg();
  auto const adjust = inst->extra<RetCtrlData>()->spOffset.offset;
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
                         folly::Optional<AuxUnion> aux) {
  auto const type = [&] {
    auto const mask = [&] { return uint64_t{(*aux).u_raw} << 32; };

    if (!retLoc.hasReg(1)) {
      auto const dt = retVal->type().toDataType();
      return aux ? v.cns(dt | mask()) : v.cns(dt);
    }
    auto const type = retLoc.reg(1);

    if (!aux) {
      auto const ret = v.makeReg();
      v << copy{type, ret};
      return ret;
    }

    auto const extended = v.makeReg();
    auto const result = v.makeReg();

    v << movzbq{type, extended};
    v << orq{extended, v.cns(mask()), result, v.makeReg()};
    return result;
  }();
  auto const data = zeroExtendIfBool(v, retVal, retLoc.reg(0));

  v << syncvmret{data, type};
}

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

void traceRet(ActRec* fp, Cell* sp, void* rip) {
  if (rip == tc::ustubs().callToExit) return;
  checkFrame(fp, sp, false /* fullCheck */, 0);
  assertx(sp <= (Cell*)fp || fp->resumed());
}

void cgRetCtrl(IRLS& env, const IRInstruction* inst) {
  auto const fp = srcLoc(env, inst, 1).reg();
  auto const sync_sp = adjustSPForReturn(env, inst);
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
  v << phpret{fp, rvmfp(), php_return_regs()};
}

void cgAsyncRetCtrl(IRLS& env, const IRInstruction* inst) {
  auto& v = vmain(env);
  adjustSPForReturn(env, inst);
  prepare_return_regs(v, inst->src(2), srcLoc(env, inst, 2),
                      inst->extra<AsyncRetCtrl>()->aux);
  v << leavetc{php_return_regs()};
}

void cgAsyncRetFast(IRLS& env, const IRInstruction* inst) {
  auto const ret = inst->src(2);
  auto const retLoc = srcLoc(env, inst, 2);
  auto& v = vmain(env);

  adjustSPForReturn(env, inst);

  // The asyncRetCtrl stub takes the return TV as its arguments.
  copyTV(v, rarg(0), rarg(1), retLoc, ret);
  auto args = vm_regs_with_sp() | rarg(1);
  if (!ret->isA(TNull)) args |= rarg(0);

  v << jmpi{tc::ustubs().asyncRetCtrl, args};
}

void cgAsyncSwitchFast(IRLS& env, const IRInstruction* inst) {
  auto& v = vmain(env);
  adjustSPForReturn(env, inst);
  prepare_return_regs(v, inst->src(2), srcLoc(env, inst, 2),
                      inst->extra<AsyncSwitchFast>()->aux);
  v << jmpi{tc::ustubs().asyncSwitchCtrl, php_return_regs()};
}

///////////////////////////////////////////////////////////////////////////////

void cgLdRetVal(IRLS& env, const IRInstruction* inst) {
  auto const fp = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);
  loadTV(v, inst->dst(), dstLoc(env, inst, 0), fp[kArRetOff], true);
}

void cgDbgTrashRetVal(IRLS& env, const IRInstruction* inst) {
  auto& v = vmain(env);
  trashTV(v, srcLoc(env, inst, 0).reg(), kArRetOff, kTVTrashJITRetVal);
}

void cgFreeActRec(IRLS& env, const IRInstruction* inst) {
  auto fp = srcLoc(env, inst, 0).reg();
  auto dst = dstLoc(env, inst, 0).reg();
  vmain(env) << load{fp[AROFF(m_sfp)], dst};
}

///////////////////////////////////////////////////////////////////////////////

void cgGenericRetDecRefs(IRLS& env, const IRInstruction* inst) {
  auto const fp = srcLoc(env, inst, 0).reg();
  auto const& marker = inst->marker();
  auto const numLocals = marker.func()->numLocals();
  auto& v = vmain(env);

  assertx(fp == rvmfp() &&
          "freeLocalsHelper assumes the frame pointer is rvmfp()");

  if (numLocals == 0) return;

  auto const target = numLocals > kNumFreeLocalsHelpers
    ? tc::ustubs().freeManyLocalsHelper
    : tc::ustubs().freeLocalsHelpers[numLocals - 1];

  auto const iterReg = v.makeReg();
  v << lea{fp[localOffset(numLocals - 1)], iterReg};

  auto const fix = Fixup{
    marker.bcOff() - marker.func()->base(),
    marker.spOff().offset
  };
  // The stub uses arg reg 0 as scratch and to pass arguments to destructors,
  // so it expects the iter argument in arg reg 1.
  auto const args = v.makeVcallArgs({{v.cns(Vconst::Quad), iterReg}});
  v << vcall{CallSpec::stub(target), args, v.makeTuple({}),
             fix, DestType::None, false};
}

///////////////////////////////////////////////////////////////////////////////

const StaticString s_ReleaseVV("ReleaseVV");

struct ReleaseVVProfile {
  std::string toString() const {
    return folly::sformat("{}/{} released", released, executed);
  }

  int percentReleased() const {
    return executed ? (100 * released / executed) : 0;
  };

  static void reduce(ReleaseVVProfile& a, const ReleaseVVProfile& b) {
    // Racy but OK---just used for profiling to trigger optimization.
    a.executed += b.executed;
    a.released += b.released;
  }

  uint16_t executed;
  uint16_t released;
};

void cgReleaseVVAndSkip(IRLS& env, const IRInstruction* inst) {
  auto const fp = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);
  auto& vc = vcold(env);

  auto const profile = TargetProfile<ReleaseVVProfile> {
    env.unit.context(), inst->marker(), s_ReleaseVV.get()
  };

  if (profile.profiling()) {
    auto const executedOff = offsetof(ReleaseVVProfile, executed);
    v << incwm{rvmtl()[profile.handle() + executedOff], v.makeReg()};
  }

  auto const releaseUnlikely = [&] {
    if (!profile.optimizing()) return true;

    auto const data = profile.data(ReleaseVVProfile::reduce);
    FTRACE(3, "cgReleaseVVAndSkip({}): percentReleased = {}\n",
           inst->toString(), data.percentReleased());

    return data.percentReleased() <
           RuntimeOption::EvalJitPGOReleaseVVMinPercent;
  }();

  auto const sf = v.makeReg();
  v << cmpqim{0, fp[AROFF(m_varEnv)], sf};

  ifThen(v, vc, CC_NZ, sf, [&] (Vout& v) {
    if (profile.profiling()) {
      auto const releasedOff = offsetof(ReleaseVVProfile, released);
      v << incwm{rvmtl()[profile.handle() + releasedOff], v.makeReg()};
    }

    auto const sf = v.makeReg();
    v << testqim{ActRec::kExtraArgsBit, fp[AROFF(m_varEnv)], sf};

    unlikelyIfThenElse(v, vc, CC_NZ, sf,
      [&] (Vout& v) {
        cgCallHelper(
          v, env,
          CallSpec::direct(static_cast<void (*)(ActRec*)>(
                           ExtraArgs::deallocate)),
          kVoidDest,
          SyncOptions::Sync,
          argGroup(env, inst).reg(fp)
        );
      },
      [&] (Vout& v) {
        cgCallHelper(
          v, env,
          CallSpec::direct(static_cast<void (*)(ActRec*)>(
                           VarEnv::deallocate)),
          kVoidDest,
          SyncOptions::Sync,
          argGroup(env, inst).reg(fp)
        );
        v << jmp{label(env, inst->taken())};
      });
  }, releaseUnlikely);
}

///////////////////////////////////////////////////////////////////////////////

}}}
