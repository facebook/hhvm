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
#include "hphp/runtime/vm/jit/irgen-interpone.h"
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

void guardToVanilla(IRGS& env, SrcKey sk, Location loc) {
  assertx(env.formingRegion || env.context.kind != TransKind::Profile);
  assertx(!env.irb->guardFailBlock());
  auto const& type = env.irb->typeOf(loc, DataTypeSpecific);
  if (!(type.isKnownDataType() && type <= TArrLike)) return;

  FTRACE_MOD(Trace::hhir, 2, "At {}: {}: guard input {} to vanilla: {}\n",
             sk.offset(), opcodeToName(sk.op()), show(loc), type);
  auto const gc = GuardConstraint(DataTypeSpecialized).setWantVanillaArray();
  env.irb->constrainLocation(loc, gc);
  if (type.arrSpec().vanilla()) return;

  auto const target_type = type.unspecialize().narrowToVanilla();
  env.irb->setGuardFailBlock(makeExit(env));
  checkType(env, loc, target_type, -1);
  env.irb->resetGuardFailBlock();
}

// In a profiling tracelet, we don't want to guard on the array being vanilla,
// so we emit code to handle both vanilla and logging arrays.
void emitLoggingDiamond(IRGS& env, SrcKey sk, Location loc, Block* nextBlock) {
  assertx(!env.formingRegion && env.context.kind == TransKind::Profile);
  assertx(!env.irb->guardFailBlock());
  auto const& type = env.irb->typeOf(loc, DataTypeSpecific);
  if (!(type.isKnownDataType() && type <= TArrLike)) return;

  FTRACE_MOD(Trace::hhir, 2, "At {}: {}: log when {} is bespoke: {}\n",
             sk.offset(), opcodeToName(sk.op()), show(loc), type);
  ifThen(
    env,
    [&](Block* taken) {
      auto const target_type = type.unspecialize().narrowToVanilla();
      env.irb->setGuardFailBlock(taken);
      checkType(env, loc, target_type, -1);
      env.irb->resetGuardFailBlock();
    },
    [&] {
      // Note that nextBlock is null iff opcodeChangesPC (which, for
      // layout-sensitive bytecodes, implies that opcodeBreaksBB and we are at
      // the end of the tracelet). Therefore we don't need to worry about
      // control flow after the InterpOneCF.
      interpOne(env);
      if (nextBlock) gen(env, Jmp, nextBlock);
    }
  );
}

Block* emitDiamondOnLocation(IRGS& env, SrcKey sk, Location loc) {
  assertx(env.context.kind == TransKind::Profile);

  auto const op = curFunc(env)->getOp(bcOff(env));
  auto const opChangePC = opcodeChangesPC(op);
  auto const nextBlock = opChangePC ? nullptr
                                    : defBlock(env, Block::Hint::Likely);
  assertx(IMPLIES(opChangePC, opcodeBreaksBB(op, false)));

  emitLoggingDiamond(env, sk, loc, nextBlock);

  auto const cur = env.irb->curBlock();
  auto const vanillaBlock = defBlock(env, Block::Hint::Likely);
  if (cur->empty() || !cur->back().isBlockEnd()) {
    gen(env, Jmp, vanillaBlock);
  } else if (!cur->back().isTerminal()) {
    cur->back().setNext(vanillaBlock);
  }
  env.irb->appendBlock(vanillaBlock);

  return nextBlock;
}

/* We have a vanilla and a logging side of the diamond. The logging side may
 * have lost type information because of using InterpOne. Emit AssertTypes with
 * the type information from the vanilla side after the merge point to regain
 * this information. This is not relevant for InterpOneCF cases, as those only
 * occur at the end of the tracelet. */
void emitDiamondTypeAssertions(IRGS& env, SrcKey sk, Block* merge) {
  assertx(merge);
  auto const dropVanilla = [&](Type type) {
    return type.isKnownDataType() && type <= TArrLike ? type.unspecialize()
                                                      : type;
  };
  auto const locals = curFunc(env)->numLocals();
  auto const pushed = getStackPushed(sk.pc());
  std::vector<Type> vanillaLocalTypes;
  vanillaLocalTypes.reserve(locals);
  std::vector<Type> vanillaStackTypes;
  vanillaStackTypes.reserve(pushed);
  for (uint32_t i = 0; i < locals; i ++) {
    auto const type = env.irb->fs().local(i).type;
    vanillaLocalTypes.push_back(dropVanilla(type));
  }
  for (int32_t i = 0; i < pushed; i ++) {
    auto const idx = BCSPRelOffset{-i};
    auto const type = env.irb->fs().stack(offsetFromIRSP(env, idx)).type;
    vanillaStackTypes.push_back(dropVanilla(type));
  }

  auto const cur = env.irb->curBlock();
  if (cur->empty() || !cur->back().isBlockEnd()) {
    gen(env, Jmp, merge);
  } else if (!cur->back().isTerminal()) {
    cur->back().setNext(merge);
  }
  env.irb->appendBlock(merge);

  for (uint32_t i = 0; i < locals; i ++) {
    gen(env, AssertLoc, vanillaLocalTypes[i], LocalId(i), fp(env));
  }
  for (int32_t i = 0; i < pushed; i ++) {
    auto const idx = BCSPRelOffset{-i};
    gen(env, AssertStk, vanillaStackTypes[i],
        IRSPRelOffsetData { offsetFromIRSP(env, idx) }, sp(env));
  }
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

void handleBespokeInputs(IRGS& env, SrcKey sk,
                         std::function<void(IRGS&)> emitVanilla) {
  if (!allowBespokeArrayLikes()) {
    // No bespokes, just irgen for vanilla
    emitVanilla(env);
    return;
  }

  auto const loc = getVanillaLocation(env, sk);
  if (!loc) {
    // No layout-sensitive inputs, just irgen for vanilla
    emitVanilla(env);
    return;
  }

  if (env.formingRegion || env.context.kind != TransKind::Profile) {
    // We're forming a region or not in a profiling tracelet, irgen as normal
    guardToVanilla(env, sk, *loc);
    emitVanilla(env);
    return;
  }

  // We potentially have a logging array in a profiling tracelet; emit a
  // diamond to handle it.
  auto const mergeBlock = emitDiamondOnLocation(env, sk, *loc);
  emitVanilla(env);
  if (mergeBlock) emitDiamondTypeAssertions(env, sk, mergeBlock);
}

void handleVanillaOutputs(IRGS& env, SrcKey sk) {
  if (!allowBespokeArrayLikes()) return;
  if (env.context.kind != TransKind::Profile) return;
  if (!isArrLikeConstructorOp(sk.op())) return;

  auto const vanilla = topC(env);
  auto const logging = gen(env, NewLoggingArray, vanilla);
  discard(env);
  push(env, logging);
}

///////////////////////////////////////////////////////////////////////////////

}}}
