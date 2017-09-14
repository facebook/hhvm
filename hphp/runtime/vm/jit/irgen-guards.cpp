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

#include "hphp/runtime/vm/jit/irgen.h"
#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"
#include "hphp/runtime/vm/jit/location.h"
#include "hphp/runtime/vm/jit/target-profile.h"

namespace HPHP { namespace jit { namespace irgen {

namespace {

//////////////////////////////////////////////////////////////////////

uint64_t packBitVec(const std::vector<bool>& bits, unsigned i) {
  uint64_t retval = 0;
  assertx(i % 64 == 0);
  assertx(i < bits.size());
  while (i < bits.size()) {
    retval |= ((uint64_t)bits[i]) << (i % 64);
    if ((++i % 64) == 0) {
      break;
    }
  }
  return retval;
}

// If its known that the location doesn't contain a boxed value, then everything
// after the check should be unreachable. Bail out now to avoid asserting on
// incompatible types. This can happen if we're inlining and one of the
// arguments has a type which doesn't match what we previously profiled (the
// guard will always fail).
bool haltIfNotBoxed(IRGS& env, const Location& loc) {
  auto const knownType = env.irb->fs().typeOf(loc);
  if (!knownType.maybe(TBoxedInitCell)) {
    gen(env, Unreachable);
    return true;
  }
  return false;
}

void checkTypeLocal(IRGS& env, uint32_t locId, Type type,
                    Offset dest, bool outerOnly) {
  auto exit = env.irb->guardFailBlock();
  if (exit == nullptr) exit = makeExit(env, dest);

  if (type <= TCell) {
    gen(env, CheckLoc, type, LocalId(locId), exit, fp(env));
    return;
  }
  assertx(type <= TBoxedInitCell);

  gen(env, CheckLoc, TBoxedInitCell, LocalId(locId), exit, fp(env));

  if (haltIfNotBoxed(env, Location::Local{locId})) return;

  gen(env, HintLocInner, type, LocalId { locId }, fp(env));

  auto const innerType = env.irb->predictedLocalInnerType(locId);
  if (!outerOnly && innerType < TInitCell) {
    env.irb->constrainLocal(locId, DataTypeSpecific, "HintLocInner");
    auto const ldPMExit = makePseudoMainExit(env);
    auto const val = ldLoc(env, locId, ldPMExit, DataTypeSpecific);
    gen(env, CheckRefInner, innerType, exit, val);
  }
}

void checkTypeStack(IRGS& env, BCSPRelOffset idx, Type type,
                    Offset dest, bool outerOnly) {
  auto exit = env.irb->guardFailBlock();
  if (exit == nullptr) exit = makeExit(env, dest);

  auto const soff = IRSPRelOffsetData { offsetFromIRSP(env, idx) };

  if (type <= TCell) {
    gen(env, CheckStk, type, soff, exit, sp(env));
    return;
  }
  assertx(type <= TBoxedInitCell);

  gen(env, CheckStk, TBoxedInitCell, soff, exit, sp(env));

  if (haltIfNotBoxed(env, Location::Stack{offsetFromFP(env, soff.offset)})) {
    return;
  }

  gen(env, HintStkInner, type, soff, sp(env));

  auto const innerType = env.irb->predictedStackInnerType(soff.offset);
  if (!outerOnly && innerType < TInitCell) {
    env.irb->constrainStack(soff.offset, DataTypeSpecific);
    auto const stk = gen(env, LdStk, TBoxedInitCell, soff, sp(env));
    gen(env, CheckRefInner, innerType, exit, stk);
  }
}

void checkTypeMBase(IRGS& env, Type type, Offset dest, bool outerOnly) {
  auto exit = env.irb->guardFailBlock();
  if (exit == nullptr) exit = makeExit(env, dest);

  auto const mbr = gen(env, LdMBase, TPtrToGen);

  if (type <= TCell) {
    gen(env, CheckMBase, type, exit, mbr);
    return;
  }
  assertx(type <= TBoxedInitCell);

  gen(env, CheckMBase, TBoxedInitCell, exit, mbr);

  if (haltIfNotBoxed(env, Location::MBase{})) return;

  gen(env, HintMBaseInner, type);

  auto const innerType = env.irb->predictedMBaseInnerType();
  if (!outerOnly && innerType < TInitCell) {
    env.irb->constrainLocation(Location::MBase{}, DataTypeSpecific);
    auto const basePtr = gen(env, LdMBase, TPtrToGen);
    auto const base = gen(env, LdMem, TBoxedInitCell, basePtr);
    gen(env, CheckRefInner, innerType, exit, base);
  }
}

//////////////////////////////////////////////////////////////////////

}

void assertTypeLocal(IRGS& env, uint32_t locId, Type type) {
  gen(env, AssertLoc, type, LocalId(locId), fp(env));
}

void assertTypeStack(IRGS& env, BCSPRelOffset idx, Type type) {
  gen(env, AssertStk, type,
      IRSPRelOffsetData { offsetFromIRSP(env, idx) }, sp(env));
}

static void assertTypeMBase(IRGS& env, Type type) {
  gen(env, AssertMBase, type);
}

void assertTypeLocation(IRGS& env, const Location& loc, Type type) {
  assertx(type <= TGen);

  switch (loc.tag()) {
    case LTag::Stack:
      assertTypeStack(env, offsetFromBCSP(env, loc.stackIdx()), type);
      break;
    case LTag::Local:
      assertTypeLocal(env, loc.localId(), type);
      break;
    case LTag::MBase:
      assertTypeMBase(env, type);
      break;
    case LTag::CSlot:
      assertx("Attempting to emit assert-type for class-ref slot" && false);
      break;
  }
}

void checkType(IRGS& env, const Location& loc,
               Type type, Offset dest, bool outerOnly) {
  assertx(type <= TGen);

  switch (loc.tag()) {
    case LTag::Stack:
      checkTypeStack(env, offsetFromBCSP(env, loc.stackIdx()),
                     type, dest, outerOnly);
      break;
    case LTag::Local:
      checkTypeLocal(env, loc.localId(), type, dest, outerOnly);
      break;
    case LTag::MBase:
      checkTypeMBase(env, type, dest, outerOnly);
      break;
    case LTag::CSlot:
      assertx("Attempting to emit check-type for class-ref slot" && false);
      break;
  }
}

void predictType(IRGS& env, const Location& loc, Type type) {
  FTRACE(1, "predictType {}: {}\n", show(loc), type);
  assertx(type <= TGen);
  env.irb->fs().refinePredictedType(loc, type);
}

//////////////////////////////////////////////////////////////////////

void makeExitPlaceholder(IRGS& env) {
  gen(env, ExitPlaceholder, makeGuardExit(env, TransFlags{}));
}

void checkRefs(IRGS& env,
               int64_t entryArDelta,
               const std::vector<bool>& mask,
               const std::vector<bool>& vals,
               Offset dest) {
  auto const actRecOff = entryArDelta + spOffBCFromIRSP(env);
  auto const funcPtr = gen(env, LdARFuncPtr, TFunc,
                           IRSPRelOffsetData { actRecOff }, sp(env));
  SSATmp* nParams = nullptr;

  for (unsigned i = 0; i < mask.size(); i += 64) {
    assertx(i < vals.size());

    uint64_t mask64 = packBitVec(mask, i);
    if (mask64 == 0) {
      continue;
    }

    if (i == 0) {
      nParams = cns(env, 64);
    } else if (!nParams || nParams->hasConstVal()) {
      nParams = gen(env, LdFuncNumParams, funcPtr);
    }

    auto const vals64 = packBitVec(vals, i);
    auto failBlock = env.irb->guardFailBlock();
    if (failBlock == nullptr) failBlock = makeExit(env, dest);
    gen(env, CheckRefs, failBlock,
        CheckRefsData { i, mask64, vals64 },
        funcPtr, nParams);
  }
}

//////////////////////////////////////////////////////////////////////

}}}
