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

#include "hphp/runtime/vm/jit/irgen-builtin.h"
#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"
#include "hphp/runtime/vm/jit/irgen-state.h"
#include "hphp/runtime/vm/srckey.h"
#include "hphp/util/tiny-vector.h"
#include "hphp/util/trace.h"

namespace HPHP { namespace jit { namespace irgen {

///////////////////////////////////////////////////////////////////////////////

namespace {

using Locations = TinyVector<Location, 2>;

Locations getVanillaLocationsForBuiltin(const IRGS& env, SrcKey sk) {
  auto const op = sk.op();
  auto const soff = env.irb->fs().bcSPOff();

  assertx(op == Op::FCallBuiltin || op == Op::NativeImpl);
  auto const func = op == Op::FCallBuiltin
    ? Unit::lookupBuiltin(sk.unit()->lookupLitstrId(getImm(sk.pc(), 3).u_SA))
    : curFunc(env);
  if (!func) return {};
  auto const param = getBuiltinVanillaParam(func->fullName()->data());
  if (param < 0) return {};

  if (op == Op::FCallBuiltin) {
    if (getImm(sk.pc(), 0).u_IVA != func->numParams()) return {};
    if (getImm(sk.pc(), 2).u_IVA != func->numInOutParams()) return {};
    return {Location::Stack{soff - func->numParams() + 1 + param}};
  } else {
    return {Location::Local{safe_cast<uint32_t>(param)}};
  }
}

Locations getVanillaLocationsForCall(const IRGS& env, SrcKey sk) {
  auto const soff = env.irb->fs().bcSPOff();
  auto const fca = getImm(sk.pc(), 0).u_FCA;
  if (!fca.hasUnpack()) return {};
  auto const input = getInstrInfo(sk.op()).in;
  auto const extra = ((input & InstrFlags::Stack1) ? 1 : 0) +
                     ((input & InstrFlags::Stack2) ? 1 : 0);
  return {Location::Stack{soff - (fca.hasGenerics() ? 1 : 0) - extra}};
}

Locations getVanillaLocations(const IRGS& env, SrcKey sk) {
  auto const op = sk.op();
  auto const soff = env.irb->fs().bcSPOff();

  if (op == Op::FCallBuiltin || op == Op::NativeImpl) {
    return getVanillaLocationsForBuiltin(env, sk);
  } else if (isFCall(op)) {
    return getVanillaLocationsForCall(env, sk);
  } else if (isBinaryOp(op)) {
    return {Location::Stack{soff}, Location::Stack{soff - 1}};
  } else if (isCast(op)) {
    return {Location::Stack{soff}};
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
    case Op::ColFromArray:
      return {Location::Stack{soff}};

    // Miscellaneous ops that constrain one local.
    case Op::IncDecL: {
      auto const local = getImm(sk.pc(), localImmIdx(op)).u_NLA;
      return {Location::Local{safe_cast<uint32_t>(local.id)}};
    }
    case Op::LIterInit:
    case Op::LIterNext: {
      auto const local = getImm(sk.pc(), localImmIdx(op)).u_LA;
      return {Location::Local{safe_cast<uint32_t>(local)}};
    }

    case Op::SetOpL: {
      auto const local = getImm(sk.pc(), localImmIdx(op)).u_LA;
      return {Location{Location::Local{safe_cast<uint32_t>(local)}},
              Location{Location::Stack{soff}}};
    }

    // Miscellaneous ops that constrain one stack value.
    case Op::BitNot:
    case Op::ClassGetTS:
    case Op::IterInit:
    case Op::JmpNZ:
    case Op::JmpZ:
    case Op::Not:
      return {Location::Stack{soff}};

    default:
      return {};
  }
  always_assert(false);
}

void guardToVanilla(IRGS& env, SrcKey sk, Location loc) {
  auto const& type = env.irb->typeOf(loc, DataTypeSpecific);
  if (!(type.isKnownDataType() && type <= TArrLike)) return;

  FTRACE_MOD(Trace::hhir, 2, "At {}: {}: guard input {} to vanilla: {}\n",
             sk.offset(), opcodeToName(sk.op()), show(loc), type);
  auto const gc = GuardConstraint(DataTypeSpecialized).setWantVanillaArray();
  env.irb->constrainLocation(loc, gc);
  if (type.arrSpec().vanilla()) return;
  if (!env.irb->guardFailBlock()) env.irb->setGuardFailBlock(makeExit(env));
  auto const target_type = type.unspecialize().narrowToVanilla();
  checkType(env, loc, target_type, -1);
}

bool skipVanillaGuards(IRGS& env, SrcKey sk, const Locations& locs) {
  auto const is_arrlike = [&](Location loc) {
    auto const& type = env.irb->fs().typeOf(loc);
    if (!type.isKnownDataType()) return false;
    auto const dt = type.toDataType();
    return isArrayLikeType(dt) || isClsMethType(dt);
  };

  if (sk.op() == Op::Same || sk.op() == Op::NSame) {
    assertx(locs.size() == 2);
    return !is_arrlike(locs[0]) || !is_arrlike(locs[1]);
  }
  return false;
}

}

///////////////////////////////////////////////////////////////////////////////

bool checkBespokeInputs(IRGS& env, SrcKey sk) {
  if (!RO::EvalAllowBespokeArrayLikes) return true;
  auto const locs = getVanillaLocations(env, sk);
  if (skipVanillaGuards(env, sk, locs)) return true;

  for (auto const loc : locs) {
    auto const& type = env.irb->fs().typeOf(loc);
    if (!type.maybe(TArrLike) || type.arrSpec().vanilla()) continue;
    FTRACE_MOD(Trace::region, 2, "At {}: {}: input {} may be bespoke: {}\n",
               sk.offset(), opcodeToName(sk.op()), show(loc), type);
    return false;
  }
  return true;
}

void handleBespokeInputs(IRGS& env, SrcKey sk) {
  if (!RO::EvalAllowBespokeArrayLikes) return;
  auto const locs = getVanillaLocations(env, sk);
  if (skipVanillaGuards(env, sk, locs)) return;

  assertx(!env.irb->guardFailBlock());
  for (auto const loc : locs) guardToVanilla(env, sk, loc);
  env.irb->resetGuardFailBlock();
}

void handleVanillaOutputs(IRGS& env, SrcKey sk) {
  if (!RO::EvalEmitBespokeArrayLikes) return;
  if (env.context.kind != TransKind::Profile) return;
  if (!isArrLikeConstructorOp(sk.op())) return;

  auto const vanilla = topC(env);
  auto const logging = gen(env, NewLoggingArray, vanilla);
  ifThen(env,
    [&](Block* taken) {
      auto const eq = gen(env, EqArrayDataPtr, vanilla, logging);
      gen(env, JmpZero, taken, eq);
    },
    [&]{
      discard(env);
      push(env, logging);
      gen(env, Jmp, makeExit(env, nextBcOff(env)));
    }
  );
}

///////////////////////////////////////////////////////////////////////////////

}}}
