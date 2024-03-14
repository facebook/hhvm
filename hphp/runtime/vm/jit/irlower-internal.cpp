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
#include "hphp/runtime/vm/jit/memory-effects.h"
#include "hphp/runtime/vm/jit/native-calls.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/util/configs/jit.h"
#include "hphp/util/match.h"
#include "hphp/util/immed.h"
#include "hphp/util/trace.h"

namespace HPHP::jit::irlower {

TRACE_SET_MOD(irlower);

///////////////////////////////////////////////////////////////////////////////

namespace {

///////////////////////////////////////////////////////////////////////////////

/*
 * Prepare `arg' for a call by shifting or zero-extending as appropriate, then
 * append its Vreg to `vargs'.
 */
void prepareArg(const ArgDesc& arg,
                Vout& v,
                VregList& vargs,
                VcallArgs::Spills* spills) {
  assertx(IMPLIES(arg.aux(), arg.kind() == ArgDesc::Kind::Reg));

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
        assertx(!arg.aux());
        reg = v.makeReg();
        v << movzbq{arg.srcReg(), reg};
        vargs.push_back(reg);
      } else if (auto const aux = arg.aux()) {
        // DataType is signed. We're using movzbq here to clear out the upper 7
        // bytes of the register, not to actually extend the type value.
        auto const extended = v.makeReg();
        auto const result = v.makeReg();
        v << movzbq{arg.srcReg(), extended};
        v << orq{
          extended,
          v.cns(auxToMask(*aux)),
          result,
          v.makeReg()
        };
        vargs.push_back(result);
      } else {
        vargs.push_back(reg);
      }
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

    case ArgDesc::Kind::SpilledTV: {
      assertx(spills);
      assertx(arg.srcReg2().isValid());
      spills->emplace(vargs.size(), arg.srcReg2());
      vargs.push_back(arg.srcReg());
      break;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

Fixup makeFixup(const BCMarker& marker, SyncOptions sync) {
  assertx(marker.valid());
  // We can get here if we are memory profiling, since we override the
  // normal sync settings and sync anyway.
  always_assert(
    sync == SyncOptions::Sync ||
    Cfg::Jit::ForceVMRegSync ||
    RuntimeOption::HHProfEnabled
  );

  // Stublogue code operates on behalf of the caller, so it needs an indirect
  // fixup to obtain the real savedRip from the native frame. The stack base
  // of stublogues start at the fixup offset of their callers, so the SP offset
  // of the marker represents the additional SP offset that needs to be added.
  if (marker.stublogue()) return Fixup::indirect(0, marker.fixupBcSPOff());

  // The rest of the prologue cannot throw exceptions, but may execute C++ code
  // that may need a fixup. Let it point to the first opcode of the function.
  if (marker.prologue()) return Fixup::direct(0, marker.fixupBcSPOff());

  auto const bcOff = marker.fixupSk().funcEntry()
    ? marker.fixupSk().entryOffset() : marker.fixupSk().offset();
  return Fixup::direct(bcOff, marker.fixupBcSPOff());
}

void cgCallHelper(Vout& v, IRLS& env, CallSpec call, const CallDest& dstInfo,
                  SyncOptions sync, const ArgGroup& args) {
  assertx(call.verifySignature(dstInfo, args.argTypes()));
  auto const inst = args.inst();
  VregList vIndRetArgs, vargs, vSimdArgs, vStkArgs;
  VcallArgs::Spills vArgSpills, vStkSpills;

  for (size_t i = 0; i < args.numIndRetArgs(); ++i) {
    prepareArg(args.indRetArg(i), v, vIndRetArgs, nullptr);
  }
  for (size_t i = 0; i < args.numGpArgs(); ++i) {
    prepareArg(args.gpArg(i), v, vargs, &vArgSpills);
  }
  for (size_t i = 0; i < args.numSimdArgs(); ++i) {
    prepareArg(args.simdArg(i), v, vSimdArgs, nullptr);
  }
  for (size_t i = 0; i < args.numStackArgs(); ++i) {
    prepareArg(args.stkArg(i), v, vStkArgs, &vStkSpills);
  }

  // If it is valid to sync the VMRegs within this call, we must track the load
  // in memory-effects.
  assertx(IMPLIES(sync != SyncOptions::None, inst->maySyncVMRegsWithSources()));

  auto const syncFixup = [&] {
    if (RuntimeOption::HHProfEnabled ||
        Cfg::Jit::ForceVMRegSync ||
        sync != SyncOptions::None) {
      // If we are profiling the heap, we always need to sync because regs need
      // to be correct during allocations no matter what.
      return makeFixup(inst->marker(), sync);
    }
    return Fixup::none();
  }();

  Vlabel targets[2];
  bool nothrow = false;
  auto const taken = inst->taken();
  auto const has_catch = taken && taken->isCatch();
  auto const may_raise = inst->mayRaiseErrorWithSources();
  assertx(IMPLIES(may_raise, has_catch));
  auto const do_catch = has_catch && may_raise;

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
    // The current instruction claims to not throw. Register a null catch trace
    // to indicate this to the unwinder.
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
    std::move(vIndRetArgs),
    std::move(vArgSpills),
    std::move(vStkSpills)
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
      case DestType::Dbl:
        return callDest(env, inst);
    }
    not_reached();
  }();

  cgCallHelper(v, env, info.func.call, dest, info.sync, args);
}

Vreg emitHashInt64(IRLS& env, const IRInstruction* inst, Vreg arr) {
  auto& v = vmain(env);
  auto const hash = v.makeReg();
  if (arch() == Arch::X64) {
#if defined(USE_HWCRC) && defined(__SSE4_2__)
    v << crc32q{arr, v.cns(0), hash};
    return hash;
#endif
  }
  cgCallHelper(
    v,
    env,
    CallSpec::direct(hash_int64),
    callDest(hash),
    SyncOptions::None,
    argGroup(env, inst).reg(arr)
  );
  return hash;
}

///////////////////////////////////////////////////////////////////////////////

}
