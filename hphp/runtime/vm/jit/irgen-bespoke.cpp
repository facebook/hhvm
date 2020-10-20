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
#include "hphp/runtime/vm/jit/irgen-bespoke.h"

#include "hphp/runtime/base/bespoke-array.h"
#include "hphp/runtime/base/bespoke/logging-array.h"

#include "hphp/runtime/vm/jit/irgen-builtin.h"
#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"
#include "hphp/runtime/vm/jit/irgen-interpone.h"
#include "hphp/runtime/vm/jit/irgen-state.h"
#include "hphp/runtime/vm/srckey.h"
#include "hphp/util/tiny-vector.h"
#include "hphp/util/trace.h"

#include <folly/Optional.h>

namespace HPHP { namespace jit { namespace irgen {

///////////////////////////////////////////////////////////////////////////////

namespace {

void translateDispatchBespoke(IRGS& env,
                              const NormalizedInstruction& ni) {
  auto const DEBUG_ONLY sk = ni.source;
  FTRACE_MOD(Trace::hhir, 2, "At {}: {}: perform bespoke translation\n",
             sk.offset(), opcodeToName(sk.op()));
  switch (ni.op()) {
    case Op::SetM:
    case Op::QueryM:
    case Op::Dim:
    case Op::SetRangeM:
    case Op::IncDecM:
    case Op::SetOpM:
    case Op::UnsetM:
    case Op::FCallBuiltin:
    case Op::NativeImpl:
    case Op::Idx:
    case Op::ArrayIdx:
    case Op::AddElemC:
    case Op::AddNewElemC:
    case Op::AKExists:
    case Op::ClassGetTS:
    case Op::ColFromArray:
    case Op::IterInit:
    case Op::LIterInit:
    case Op::LIterNext:
      interpOne(env);
      return;
    default:
      not_reached();
  }
}

folly::Optional<Location> getVanillaLocationForBuiltin(const IRGS& env,
                                                        SrcKey sk) {
  auto const op = sk.op();
  auto const soff = env.irb->fs().bcSPOff();

  assertx(op == Op::FCallBuiltin || op == Op::NativeImpl);
  auto const func = op == Op::FCallBuiltin
    ? Func::lookupBuiltin(sk.unit()->lookupLitstrId(getImm(sk.pc(), 3).u_SA))
    : curFunc(env);
  if (!func) return folly::none;
  auto const param = getBuiltinVanillaParam(func->fullName()->data());
  if (param < 0) return folly::none;

  if (op == Op::FCallBuiltin) {
    if (getImm(sk.pc(), 0).u_IVA != func->numParams()) return folly::none;
    if (getImm(sk.pc(), 2).u_IVA != func->numInOutParams()) return folly::none;
    return {Location::Stack{soff - func->numParams() + 1 + param}};
  } else {
    return {Location::Local{safe_cast<uint32_t>(param)}};
  }
}

folly::Optional<Location> getVanillaLocation(const IRGS& env, SrcKey sk) {
  auto const op = sk.op();
  auto const soff = env.irb->fs().bcSPOff();

  if (op == Op::FCallBuiltin || op == Op::NativeImpl) {
    return getVanillaLocationForBuiltin(env, sk);
  } else if (isMemberDimOp(op) || isMemberFinalOp(op)) {
    return {Location::MBase{}};
  }

  switch (op) {
    // Array accesses constrain the base.
    case Op::Idx:
    case Op::ArrayIdx:
    case Op::AddElemC:
      return {Location::Stack{soff - 2}};
    case Op::AddNewElemC:
      return {Location::Stack{soff - 1}};
    case Op::AKExists:
    case Op::ClassGetTS:
    case Op::ColFromArray:
    case Op::IterInit:
      return {Location::Stack{soff}};

    // Local iterators constrain the local base.
    case Op::LIterInit:
    case Op::LIterNext: {
      auto const local = getImm(sk.pc(), localImmIdx(op)).u_LA;
      return {Location::Local{safe_cast<uint32_t>(local)}};
    }

    default:
      return folly::none;
  }
  always_assert(false);
}

// Strengthen the guards for a layout-sensitive location prior to irgen for the
// bytecode, returning false if standard vanilla translation should be used and
// true if bespoke translation should be used.
//
// If the location does not have a known data type, it returns false.
// Otherwise, it constrains the location to DataTypeSpecialized. If the
// location has a bespoke layout, we return true. If it has a vanilla layout,
// we return false. Otherwise, we emit a vanilla guard and return false.
bool guardToLayout(IRGS& env, SrcKey sk, Location loc) {
  assertx(env.context.kind != TransKind::Profile);
  assertx(!env.irb->guardFailBlock());
  auto const& type = env.irb->typeOf(loc, DataTypeSpecific);
  if (!(type.isKnownDataType() && type <= TArrLike)) return false;

  auto const base = type.unspecialize();
  auto const layout = type.arrSpec().bespokeLayout();
  auto const target_type = layout ? base.narrowToBespokeLayout(*layout)
                                  : base.narrowToVanilla();

  FTRACE_MOD(Trace::hhir, 2,
             "At {}: {}: guard input {} to layout specific: {}\n",
             sk.offset(), opcodeToName(sk.op()), show(loc), target_type);
  auto const gc = GuardConstraint(DataTypeSpecialized).setArrayLayoutSensitive();
  env.irb->constrainLocation(loc, gc);
  checkType(env, loc, target_type, -1);
  return !target_type.arrSpec().vanilla();
}

// In a profiling tracelet, we don't want to guard on the array being vanilla,
// so we emit code to handle both vanilla and logging arrays.
void emitLoggingDiamond(
    IRGS& env, const NormalizedInstruction& ni, Location loc,
    std::function<void(IRGS&)> emitVanilla) {
  assertx(env.context.kind == TransKind::Profile);
  assertx(!env.irb->guardFailBlock());
  auto const& type = env.irb->typeOf(loc, DataTypeSpecific);
  FTRACE_MOD(Trace::hhir, 2, "At {}: {}: consider {}: {}\n",
             ni.source.offset(), opcodeToName(ni.op()), show(loc), type);
  if (!(type.isKnownDataType() && type <= TArrLike)) {
    emitVanilla(env);
    return;
  }

  FTRACE_MOD(Trace::hhir, 2, "At {}: {}: log when {} is bespoke: {}\n",
             ni.source.offset(), opcodeToName(ni.op()), show(loc), type);

  if (!env.formingRegion) {
    // If we're not forming a region (and therefore have a ProfTransID), log
    // that the array reached this bytecode. If the array is a LoggingArray,
    // this will record a reach event. Once the ArraySampled bit is
    // implemented, we will also use this helper to count how many sampled vs.
    // unsampled arrays reach the usage site. This information can inform if a
    // significant number of vanilla arrays origininating from unlogged sources
    // (such as builtins) are reaching the site so we don't inappropriately
    // specialize.
    genLogArrayReach(env, loc, ni.source);
  }

  auto const dropVanilla = [&](Type type) {
    return type.isKnownDataType() && type <= TArrLike
      ? type.unspecialize()
      : type;
  };
  std::vector<Type> vanillaLocalTypes;
  std::vector<Type> vanillaStackTypes;
  ifThen(
    env,
    [&](Block* taken) {
      auto const target_type = type.unspecialize().narrowToVanilla();
      env.irb->setGuardFailBlock(taken);
      checkType(env, loc, target_type, -1);
      env.irb->resetGuardFailBlock();

      emitVanilla(env);

      // We have a vanilla and a logging side of the diamond. The logging side
      // may have lost type information because of using InterpOne.  We will
      // emit AssertTypes with the type information from the vanilla side after
      // the generated code on the logging side to regain this information.
      auto const locals = curFunc(env)->numLocals();
      auto const pushed = getStackPushed(ni.source.pc());
      vanillaLocalTypes.reserve(locals);
      vanillaStackTypes.reserve(pushed);
      for (uint32_t i = 0; i < locals; i ++) {
        auto const lType = env.irb->fs().local(i).type;
        vanillaLocalTypes.push_back(dropVanilla(lType));
      }
      for (int32_t i = 0; i < pushed; i ++) {
        auto const idx = BCSPRelOffset{-i};
        auto const sType = env.irb->fs().stack(offsetFromIRSP(env, idx)).type;
        vanillaStackTypes.push_back(dropVanilla(sType));
      }
    },
    [&] {
      hint(env, Block::Hint::Unlikely);

      auto const topLayout = BespokeLayout::TopLayout();
      auto const topType = type.unspecialize().narrowToBespokeLayout(topLayout);
      assertTypeLocation(env, loc, topType);

      try {
        translateDispatchBespoke(env, ni);
      } catch (const FailedIRGen& exn) {
        FTRACE_MOD(Trace::region, 1,
          "logging ir generation for {} failed with {} while vanilla succeeded\n",
           ni.toString(), exn.what());
        throw;
      }

      // For layout-sensitive bytecodes, opcodeChangesPC implies that
      // opcodeBreaksBB and we are at the end of the tracelet. Therefore we
      // don't need to worry about control flow after the InterpOneCF.
      auto const DEBUG_ONLY op = curFunc(env)->getOp(bcOff(env));
      auto const DEBUG_ONLY opChangePC = opcodeChangesPC(op);
      assertx(IMPLIES(opChangePC, opcodeBreaksBB(op, false)));

      for (uint32_t i = 0; i < vanillaLocalTypes.size(); i ++) {
        gen(env, AssertLoc, vanillaLocalTypes[i], LocalId(i), fp(env));
      }
      for (int32_t i = 0; i < vanillaStackTypes.size(); i ++) {
        auto const idx = BCSPRelOffset{-i};
        gen(env, AssertStk, vanillaStackTypes[i],
            IRSPRelOffsetData { offsetFromIRSP(env, idx) }, sp(env));
      }
    }
  );
}

}

///////////////////////////////////////////////////////////////////////////////

void handleBespokeInputs(IRGS& env, const NormalizedInstruction& ni,
                         std::function<void(IRGS&)> emitVanilla) {
  if (!allowBespokeArrayLikes()) {
    // No bespokes, just irgen for vanilla
    emitVanilla(env);
    return;
  }

  auto const sk = ni.source;
  auto const loc = getVanillaLocation(env, sk);
  if (!loc) {
    // No layout-sensitive inputs, just irgen for vanilla
    emitVanilla(env);
    return;
  }

  if (env.context.kind == TransKind::Profile) {
    // We are in a profiling tracelet. We most likely have a logging array in a
    // profiling tracelet; emit a diamond to handle it. If it's another bespoke,
    // side-exit.
    emitLoggingDiamond(env, ni, *loc, emitVanilla);
    return;
  }

  // We're not in a profiling tracelet; specialize for the observed layout.
  // In the future, we will supply the layout we believe we should specialize
  // the bytecode for.
  auto const bespoke = guardToLayout(env, sk, *loc);
  if (bespoke) {
    // We have a bespoke type; dispatch to specialized irgen
    translateDispatchBespoke(env, ni);
  } else {
    // We have a vanilla type or unspecialized type; dispatch to normal irgen
    emitVanilla(env);
  }
}

void handleVanillaOutputs(IRGS& env, SrcKey sk) {
  if (!allowBespokeArrayLikes()) return;
  if (env.context.kind != TransKind::Profile
      && !shouldTestBespokeArrayLikes()) {
    return;
  }
  auto const op = sk.op();
  if (!isArrLikeConstructorOp(op) && !isArrLikeCastOp(op)) return;

  // These SrcKeys are always skipped in maybeMakeLoggingArray. If we make
  // this logic more complicated, we should expose a helper to share the code.
  if ((op == Op::Array || op == Op::Dict) &&
      sk.advanced().op() == Op::IsTypeStructC) {
    return;
  }

  auto const result = topC(env);
  auto const vanilla = result->type().arrSpec().vanilla();
  assertx(IMPLIES(isArrLikeConstructorOp(op), vanilla));
  if (!vanilla) return;

  auto const logging = gen(env, NewLoggingArray, result);
  discard(env);
  push(env, logging);
}

///////////////////////////////////////////////////////////////////////////////

}}}
