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
  assertx(env.formingRegion || env.context.kind != TransKind::Profile);
  assertx(!env.irb->guardFailBlock());
  auto const& type = env.irb->typeOf(loc, DataTypeSpecific);
  if (!(type.isKnownDataType() && type <= TArrLike)) return false;

  FTRACE_MOD(Trace::hhir, 2,
             "At {}: {}: guard input {} to layout specific: {}\n",
             sk.offset(), opcodeToName(sk.op()), show(loc), type);
  auto const gc = GuardConstraint(DataTypeSpecialized).setArrayLayoutSensitive();
  env.irb->constrainLocation(loc, gc);
  if (typeFitsConstraint(type, gc)) {
    return type.arrSpec().bespokeLayout().has_value();
  }

  auto const target_type = type.unspecialize().narrowToVanilla();
  env.irb->setGuardFailBlock(makeExit(env));
  checkType(env, loc, target_type, -1);
  env.irb->resetGuardFailBlock();
  return false;
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

  auto const dropVanilla = [&](Type type) {
    return type.isKnownDataType() && type <= TArrLike ? type.unspecialize()
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
      // TODO(mcolavita): Until we can emit a vanilla|logging guard, do the
      // check manually
      auto const loggingLayout =
        BespokeLayout::FromIndex(bespoke::LoggingArray::GetLayoutIndex().raw);
      auto const target_type = type.unspecialize()
        .narrowToBespokeLayout(loggingLayout);
      checkType(env, loc, target_type, -1);
      // Note that nextBlock is null iff opcodeChangesPC (which, for
      // layout-sensitive bytecodes, implies that opcodeBreaksBB and we are at
      // the end of the tracelet). Therefore we don't need to worry about
      // control flow after the InterpOneCF.
      try {
        translateDispatchBespoke(env, ni);
      } catch (const FailedIRGen& exn) {
        FTRACE_MOD(Trace::region, 1,
          "logging ir generation for {} failed with {} while vanilla succeeded\n",
           ni.toString(), exn.what());
        throw;
      }

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

void emitDiamondOnLocation(
    IRGS& env, const NormalizedInstruction& ni, Location loc,
    std::function<void(IRGS&)> emitVanilla) {
  assertx(env.context.kind == TransKind::Profile);

  auto const DEBUG_ONLY op = curFunc(env)->getOp(bcOff(env));
  auto const DEBUG_ONLY opChangePC = opcodeChangesPC(op);
  assertx(IMPLIES(opChangePC, opcodeBreaksBB(op, false)));

  emitLoggingDiamond(env, ni, loc, emitVanilla);
}
}

///////////////////////////////////////////////////////////////////////////////

bool checkBespokeInputs(IRGS& env, SrcKey sk) {
  if (!allowBespokeArrayLikes()) return true;

  if (auto const loc = getVanillaLocation(env, sk)) {
    auto const& type = env.irb->fs().typeOf(*loc);
    if (!type.maybe(TArrLike) ||
        type.arrSpec().vanilla() ||
        type.arrSpec().bespokeLayout()) {
      return true;
    }
    FTRACE_MOD(Trace::region, 2, "At {}: {}: input {} may be bespoke: {}\n",
               sk.offset(), opcodeToName(sk.op()), show(*loc), type);
    return false;
  }
  return true;
}

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

  if (env.formingRegion || env.context.kind != TransKind::Profile) {
    // We're forming a region or not in a profiling tracelet
    auto const bespoke = guardToLayout(env, sk, *loc);
    if (bespoke) {
      // We have a bespoke type; dispatch to specialized irgen
      translateDispatchBespoke(env, ni);
    } else {
      // We have a vanilla type or unspecialized type; dispatch to normal irgen
      emitVanilla(env);
    }
    return;
  }

  // We potentially have a logging array in a profiling tracelet; emit a
  // diamond to handle it.
  emitDiamondOnLocation(env, ni, *loc, emitVanilla);
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
