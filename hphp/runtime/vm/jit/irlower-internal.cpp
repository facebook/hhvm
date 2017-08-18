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

#include "hphp/runtime/base/runtime-option.h"

#include "hphp/runtime/vm/jit/arg-group.h"
#include "hphp/runtime/vm/jit/bc-marker.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/native-calls.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/util/immed.h"
#include "hphp/util/trace.h"

namespace HPHP { namespace jit { namespace irlower {

TRACE_SET_MOD(irlower);

///////////////////////////////////////////////////////////////////////////////

namespace {

///////////////////////////////////////////////////////////////////////////////

/*
 * Prepare `arg' for a call by shifting or zero-extending as appropriate, then
 * append its Vreg to `vargs'.
 */
void prepareArg(const ArgDesc& arg, Vout& v, VregList& vargs) {
  switch (arg.kind()) {
    case ArgDesc::Kind::IndRet: {
      auto const tmp = v.makeReg();
      v << lea{arg.srcReg()[arg.disp().l()], tmp};
      vargs.push_back(tmp);
      break;
    }

    case ArgDesc::Kind::Reg: {
      auto reg = arg.srcReg();
      if (arg.isZeroExtend()) {
        reg = v.makeReg();
        v << movzbq{arg.srcReg(), reg};
      }
      vargs.push_back(reg);
      break;
    }

    case ArgDesc::Kind::Imm:
      vargs.push_back(v.cns(arg.imm().q()));
      break;

    case ArgDesc::Kind::TypeImm:
      vargs.push_back(v.cns(arg.typeImm()));
      break;

    case ArgDesc::Kind::Addr: {
      auto tmp = v.makeReg();
      v << lea{arg.srcReg()[arg.disp().l()], tmp};
      vargs.push_back(tmp);
      break;
    }

    case ArgDesc::Kind::DataPtr: {
      auto tmp = v.makeReg();
      v << lead{reinterpret_cast<void*>(arg.imm().q()), tmp};
      vargs.push_back(tmp);
      break;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

Fixup makeFixup(const BCMarker& marker, SyncOptions sync) {
  assertx(marker.valid());
  auto const stackOff = [&] {
    switch (sync) {
      case SyncOptions::SyncAdjustOne:
        return marker.spOff() -= 1;

      case SyncOptions::None:
        // We can get here if we are memory profiling, since we override the
        // normal sync settings and sync anyway.
        always_assert(RuntimeOption::HHProfEnabled);
        // fallthru
      case SyncOptions::Sync:
        return marker.spOff();
    }
    not_reached();
  }();

  auto const bcOff = marker.fixupBcOff() - marker.fixupFunc()->base();
  return Fixup{bcOff, stackOff.offset};
}

void cgCallHelper(Vout& v, IRLS& env, CallSpec call, const CallDest& dstInfo,
                  SyncOptions sync, const ArgGroup& args) {
  auto const inst = args.inst();
  jit::vector<Vreg> vIndRetArgs, vargs, vSimdArgs, vStkArgs;

  for (size_t i = 0; i < args.numIndRetArgs(); ++i) {
    prepareArg(args.indRetArg(i), v, vIndRetArgs);
  }
  for (size_t i = 0; i < args.numGpArgs(); ++i) {
    prepareArg(args.gpArg(i), v, vargs);
  }
  for (size_t i = 0; i < args.numSimdArgs(); ++i) {
    prepareArg(args.simdArg(i), v, vSimdArgs);
  }
  for (size_t i = 0; i < args.numStackArgs(); ++i) {
    prepareArg(args.stkArg(i), v, vStkArgs);
  }

  auto const syncFixup = [&] {
    if (RuntimeOption::HHProfEnabled || sync != SyncOptions::None) {
      // If we are profiling the heap, we always need to sync because regs need
      // to be correct during allocations no matter what.
      return makeFixup(inst->marker(), sync);
    }
    return Fixup{};
  }();

  Vlabel targets[2];
  bool nothrow = false;
  auto const taken = inst->taken();
  auto const do_catch = taken && taken->isCatch();

  if (do_catch) {
    always_assert_flog(
      inst->is(InterpOne) || sync != SyncOptions::None,
      "cgCallHelper called with None but inst has a catch block: {}\n",
      *inst
    );
    always_assert_flog(
      taken->catchMarker() == inst->marker(),
      "Catch trace doesn't match fixup:\n"
      "Instruction: {}\n"
      "Catch trace: {}\n"
      "Fixup      : {}\n",
      inst->toString(),
      taken->catchMarker().show(),
      inst->marker().show()
    );

    targets[0] = v.makeBlock();
    targets[1] = env.labels[taken];
  } else {
    // The current instruction doesn't have a catch block so it'd better not
    // throw.  Register a null catch trace to indicate this to the unwinder.
    nothrow = true;
  }

  VregList dstRegs;
  if (dstInfo.reg0.isValid()) {
    dstRegs.push_back(dstInfo.reg0);
    if (dstInfo.reg1.isValid()) {
      dstRegs.push_back(dstInfo.reg1);
    }
  }

  auto const argsId = v.makeVcallArgs({
    std::move(vargs),
    std::move(vSimdArgs),
    std::move(vStkArgs),
    std::move(vIndRetArgs)
  });
  auto const dstId = v.makeTuple(std::move(dstRegs));

  if (do_catch) {
    v << vinvoke{call, argsId, dstId, {targets[0], targets[1]},
                 syncFixup, dstInfo.type};
    v = targets[0];
  } else {
    v << vcall{call, argsId, dstId, syncFixup, dstInfo.type, nothrow};
  }
}

void cgCallNative(Vout& v, IRLS& env, const IRInstruction* inst) {
  using namespace NativeCalls;
  always_assert(CallMap::hasInfo(inst->op()));
  auto const& info = CallMap::info(inst->op());

  ArgGroup args = toArgGroup(info, env.locs, inst);

  auto const dest = [&]() -> CallDest {
    switch (info.dest) {
      case DestType::None:
        return kVoidDest;
      case DestType::Indirect:
        return kIndirectDest;
      case DestType::TV:
      case DestType::SIMD:
        return callDestTV(env, inst);
      case DestType::SSA:
      case DestType::Byte:
        return callDest(env, inst);
      case DestType::Dbl:
        return callDestDbl(env, inst);
      case DestType::SSAPair:
        always_assert(false && "SSAPair not implemented for cgCallNative");
    }
    not_reached();
  }();

  cgCallHelper(v, env, info.func.call, dest, info.sync, args);
}

///////////////////////////////////////////////////////////////////////////////

}}}
