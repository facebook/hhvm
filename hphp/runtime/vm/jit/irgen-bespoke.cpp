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

#include "hphp/runtime/vm/jit/irgen-builtin.h"
#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"
#include "hphp/runtime/vm/jit/irgen-state.h"
#include "hphp/runtime/vm/srckey.h"
#include "hphp/util/tiny-vector.h"
#include "hphp/util/trace.h"

#include <folly/Optional.h>

namespace HPHP { namespace jit { namespace irgen {

///////////////////////////////////////////////////////////////////////////////

namespace {

folly::Optional<Location> getVanillaLocationForBuiltin(const IRGS& env,
                                                        SrcKey sk) {
  auto const op = sk.op();
  auto const soff = env.irb->fs().bcSPOff();

  assertx(op == Op::FCallBuiltin || op == Op::NativeImpl);
  auto const func = op == Op::FCallBuiltin
    ? Unit::lookupBuiltin(sk.unit()->lookupLitstrId(getImm(sk.pc(), 3).u_SA))
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

void guardToVanilla(IRGS& env, SrcKey sk, Location loc) {
  assertx(!env.irb->guardFailBlock());
  auto const& type = env.irb->typeOf(loc, DataTypeSpecific);
  if (!(type.isKnownDataType() && type <= TArrLike)) return;

  FTRACE_MOD(Trace::hhir, 2, "At {}: {}: guard input {} to vanilla: {}\n",
             sk.offset(), opcodeToName(sk.op()), show(loc), type);
  auto const gc = GuardConstraint(DataTypeSpecialized).setWantVanillaArray();
  env.irb->constrainLocation(loc, gc);
  if (type.arrSpec().vanilla()) return;

  env.irb->setGuardFailBlock(makeExit(env));
  auto const target_type = type.unspecialize().narrowToVanilla();
  checkType(env, loc, target_type, -1);
}
}

///////////////////////////////////////////////////////////////////////////////

bool checkBespokeInputs(IRGS& env, SrcKey sk) {
  if (!allowBespokeArrayLikes()) return true;

  if (auto const loc = getVanillaLocation(env, sk)) {
    auto const& type = env.irb->fs().typeOf(*loc);
    if (!type.maybe(TArrLike) || type.arrSpec().vanilla()) return true;
    FTRACE_MOD(Trace::region, 2, "At {}: {}: input {} may be bespoke: {}\n",
               sk.offset(), opcodeToName(sk.op()), show(*loc), type);
    return false;
  }
  return true;
}

void handleBespokeInputs(IRGS& env, SrcKey sk) {
  if (!allowBespokeArrayLikes()) return;

  if (auto const loc = getVanillaLocation(env, sk)) {
    assertx(!env.irb->guardFailBlock());
    guardToVanilla(env, sk, *loc);
    env.irb->resetGuardFailBlock();
  }
}

void handleVanillaOutputs(IRGS& env, SrcKey sk) {
  if (!allowBespokeArrayLikes()) return;
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
