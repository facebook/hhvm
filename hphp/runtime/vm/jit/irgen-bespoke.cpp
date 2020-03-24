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
#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/util/tiny-vector.h"
#include "hphp/util/trace.h"

namespace HPHP { namespace jit { namespace irgen {

///////////////////////////////////////////////////////////////////////////////

namespace {

using Locations = TinyVector<Location, 2>;

Locations getVanillaLocationsForBuiltin(
    const IRGS& env, const NormalizedInstruction& ni) {
  auto const op = ni.op();
  auto const soff = env.irb->fs().bcSPOff();

  assertx(op == Op::FCallBuiltin || op == Op::NativeImpl);
  auto const func = op == Op::FCallBuiltin
    ? Unit::lookupBuiltin(ni.m_unit->lookupLitstrId(ni.imm[3].u_SA))
    : curFunc(env);
  if (!func) return {};
  auto const param = getBuiltinVanillaParam(func->fullName()->data());
  if (param < 0) return {};

  if (op == Op::FCallBuiltin) {
    if (ni.imm[0].u_IVA != func->numParams()) return {};
    if (ni.imm[2].u_IVA != func->numInOutParams()) return {};
    return {Location::Stack{soff - func->numParams() + 1 + param}};
  } else {
    return {Location::Local{safe_cast<uint32_t>(param)}};
  }
}

Locations getVanillaLocationsForCall(
    const IRGS& env, const NormalizedInstruction& ni) {
  auto const soff = env.irb->fs().bcSPOff();
  auto const& fca = ni.imm[0].u_FCA;
  if (!fca.hasUnpack()) return {};
  auto const input = getInstrInfo(ni.op()).in;
  auto const extra = ((input & InstrFlags::Stack1) ? 1 : 0) +
                     ((input & InstrFlags::Stack2) ? 1 : 0);
  return {Location::Stack{soff - (fca.hasGenerics() ? 1 : 0) - extra}};
}

Locations getVanillaLocations(
    const IRGS& env, const NormalizedInstruction& ni) {
  auto const op = ni.op();
  auto const soff = env.irb->fs().bcSPOff();

  if (op == Op::FCallBuiltin || op == Op::NativeImpl) {
    return getVanillaLocationsForBuiltin(env, ni);
  } else if (isFCall(op)) {
    return getVanillaLocationsForCall(env, ni);
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
      auto const local = ni.imm[localImmIdx(op)].u_NLA;
      return {Location::Local{safe_cast<uint32_t>(local.id)}};
    }
    case Op::LIterInit:
    case Op::LIterNext:
    case Op::NewLikeArrayL: {
      auto const local = ni.imm[localImmIdx(op)].u_LA;
      return {Location::Local{safe_cast<uint32_t>(local)}};
    }

    case Op::SetOpL: {
      auto const local = ni.imm[localImmIdx(op)].u_LA;
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

void guardToVanilla(IRGS& env, const NormalizedInstruction& ni, Location loc) {
  auto const& type = env.irb->typeOf(loc, DataTypeSpecific);
  if (!(type.isKnownDataType() && type <= TArrLike)) return;

  FTRACE_MOD(Trace::hhir, 2, "At {}: {}: guard input {} to vanilla: {}\n",
             ni.offset(), opcodeToName(ni.op()), show(loc), type);
  auto const gc = GuardConstraint(DataTypeSpecialized).setWantVanillaArray();
  env.irb->constrainLocation(loc, gc);
  if (type.arrSpec().vanilla()) return;
  if (!env.irb->guardFailBlock()) env.irb->setGuardFailBlock(makeExit(env));
  auto const target_type = type.unspecialize().narrowToVanilla();
  checkType(env, loc, target_type, -1);
}

bool skipVanillaGuards(IRGS& env, const NormalizedInstruction& ni,
                       const Locations& locs) {
  auto const is_arrlike = [&](Location loc) {
    auto const& type = env.irb->fs().typeOf(loc);
    if (!type.isKnownDataType()) return false;
    auto const dt = type.toDataType();
    return isArrayLikeType(dt) || isClsMethType(dt);
  };

  if (ni.op() == Op::Same || ni.op() == Op::NSame) {
    assertx(locs.size() == 2);
    return !is_arrlike(locs[0]) || !is_arrlike(locs[1]);
  }
  return false;
}

}

///////////////////////////////////////////////////////////////////////////////

bool checkBespokeInputs(IRGS& env, const NormalizedInstruction& ni) {
  if (!RO::EvalAllowBespokeArrayLikes) return true;
  auto const locs = getVanillaLocations(env, ni);
  if (skipVanillaGuards(env, ni, locs)) return true;

  for (auto const loc : locs) {
    auto const& type = env.irb->fs().typeOf(loc);
    if (!type.maybe(TArrLike) || type.arrSpec().vanilla()) continue;
    FTRACE_MOD(Trace::region, 2, "At {}: {}: input {} may be bespoke: {}\n",
               ni.offset(), opcodeToName(ni.op()), show(loc), type);
    return false;
  }
  return true;
}

void handleBespokeInputs(IRGS& env, const NormalizedInstruction& ni) {
  if (!RO::EvalAllowBespokeArrayLikes) return;
  auto const locs = getVanillaLocations(env, ni);
  if (skipVanillaGuards(env, ni, locs)) return;

  assertx(!env.irb->guardFailBlock());
  for (auto const loc : locs) guardToVanilla(env, ni, loc);
  env.irb->resetGuardFailBlock();
}

///////////////////////////////////////////////////////////////////////////////

}}}
