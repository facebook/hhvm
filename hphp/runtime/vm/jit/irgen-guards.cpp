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
#include "hphp/runtime/vm/jit/irgen-guards.h"
#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"
#include "hphp/runtime/vm/jit/target-profile.h"

namespace HPHP { namespace jit { namespace irgen {

namespace {

//////////////////////////////////////////////////////////////////////

enum class ProfGuard { CheckLoc, CheckStk };

/*
 * Emit a type guard, possibly using profiling information. Depending on the
 * current translation mode and type to be guarded, this function may emit
 * additional profiling code or modify the guarded type using previously
 * collected profiling information. Str -> StaticStr is the only supported
 * refinement for now.
 */
void profiledGuard(IRGS& env,
                   Type type,
                   ProfGuard kind,
                   int32_t id, // locId or stackOff
                   Block* checkExit) {
  auto failBlock = env.irb->guardFailBlock();
  if (failBlock == nullptr) failBlock = checkExit;
  switch (kind) {
    case ProfGuard::CheckLoc:
      gen(env, CheckLoc, type, LocalId(id), failBlock, fp(env));
      return;
    case ProfGuard::CheckStk:
      // Adjust 'id' to get an offset from the current m_irb->sp().
      auto const adjOff = offsetFromIRSP(env, BCSPOffset{id});
      gen(env, CheckStk, type,
          RelOffsetData { BCSPOffset{id}, adjOff }, failBlock, sp(env));
      return;
  }
}

uint64_t packBitVec(const std::vector<bool>& bits, unsigned i) {
  uint64_t retval = 0;
  assertx(i % 64 == 0);
  assertx(i < bits.size());
  while (i < bits.size()) {
    retval |= bits[i] << (i % 64);
    if ((++i % 64) == 0) {
      break;
    }
  }
  return retval;
}

//////////////////////////////////////////////////////////////////////

}

void assertTypeLocal(IRGS& env, uint32_t locId, Type type) {
  gen(env, AssertLoc, type, LocalId(locId), fp(env));
}

void checkTypeLocal(IRGS& env, uint32_t locId, Type type, Offset dest,
                    bool outerOnly) {
  assertx(type <= TCell || type <= TBoxedInitCell);

  auto exit = env.irb->guardFailBlock();
  if (exit == nullptr) exit = makeExit(env, dest);

  if (type <= TCell) {
    profiledGuard(env, type, ProfGuard::CheckLoc, locId, exit);
    return;
  }

  profiledGuard(env, TBoxedInitCell, ProfGuard::CheckLoc, locId, exit);
  gen(env, HintLocInner, type, LocalId { locId }, fp(env));

  if (!outerOnly && type.inner() < TInitCell) {
    auto const exit = makeExit(env);
    auto const ldPMExit = makePseudoMainExit(env);
    auto const val = ldLoc(env, locId, ldPMExit, DataTypeSpecific);
    gen(env, CheckRefInner, env.irb->predictedInnerType(locId), exit, val);
  }
}

void assertTypeStack(IRGS& env, BCSPOffset idx, Type type) {
  gen(env, AssertStk, type,
      IRSPOffsetData { offsetFromIRSP(env, idx) }, sp(env));
}

void checkTypeStack(IRGS& env,
                    BCSPOffset idx,
                    Type type,
                    Offset dest,
                    bool outerOnly) {
  assertx(type <= TCell || type <= TBoxedInitCell);

  auto exit = env.irb->guardFailBlock();
  if (exit == nullptr) exit = makeExit(env, dest);

  if (type <= TBoxedInitCell) {
    auto const soff = RelOffsetData { idx, offsetFromIRSP(env, idx) };
    profiledGuard(env, TBoxedInitCell, ProfGuard::CheckStk,
                  idx.offset, exit);
    env.irb->constrainStack(soff.irSpOffset, DataTypeSpecific);
    gen(env, HintStkInner, type, soff, sp(env));

    // Check inner type eargerly only at the beginning of a region.
    if (!outerOnly && type.inner() < TInitCell) {
      auto stk = gen(env, LdStk, TBoxedInitCell,
                     IRSPOffsetData{soff.irSpOffset}, sp(env));
      gen(env, CheckRefInner,
          env.irb->predictedStackInnerType(soff.irSpOffset),
          exit, stk);
    }
    return;
  }

  FTRACE(1, "checkTypeStack({}): no tmp: {}\n", idx.offset, type.toString());
  // Just like CheckType, CheckStk only cares about its input type if the
  // simplifier does something with it.
  profiledGuard(env, type, ProfGuard::CheckStk, idx.offset, exit);
}

void predictTypeStack(IRGS& env, BCSPOffset offset, Type type) {
  FTRACE(1, "predictTypeStack {}: {}\n", offset.offset, type);
  assert(type <= TGen);

  auto const irSPOff = offsetFromIRSP(env, offset);
  env.irb->fs().refineStackPredictedType(irSPOff, type);
}

void predictTypeLocal(IRGS& env, uint32_t locId, Type type) {
  FTRACE(1, "predictTypeLocal: {}: {}\n", locId, type);
  assert(type <= TGen);
  env.irb->fs().refineLocalPredictedType(locId, type);
}

void predictTypeLocation(
  IRGS& env,
  const RegionDesc::Location& loc,
  Type type
) {
  using T = RegionDesc::Location::Tag;
  switch (loc.tag()) {
  case T::Stack:
    predictTypeStack(env, offsetFromBCSP(env, loc.offsetFromFP()), type);
    break;
  case T::Local:
    predictTypeLocal(env, loc.localId(), type);
    break;
  }
}

//////////////////////////////////////////////////////////////////////

void assertTypeLocation(IRGS& env, const RegionDesc::Location& loc, Type type) {
  assertx(type <= TStkElem);
  using T = RegionDesc::Location::Tag;
  switch (loc.tag()) {
  case T::Stack:
    assertTypeStack(env, offsetFromBCSP(env, loc.offsetFromFP()), type);
    break;
  case T::Local:
    assertTypeLocal(env, loc.localId(), type);
    break;
  }
}

void checkTypeLocation(IRGS& env,
                       const RegionDesc::Location& loc,
                       Type type,
                       Offset dest,
                       bool outerOnly) {
  assertx(type <= TGen);
  using T = RegionDesc::Location::Tag;
  switch (loc.tag()) {
  case T::Stack:
    checkTypeStack(env, offsetFromBCSP(env, loc.offsetFromFP()), type, dest,
                   outerOnly);
    break;
  case T::Local:
    checkTypeLocal(env, loc.localId(), type, dest, outerOnly);
    break;
  }
}

void makeExitPlaceholder(IRGS& env) {
  gen(env, ExitPlaceholder, makeGuardExit(env, TransFlags{}));
}

void checkRefs(IRGS& env,
               int64_t entryArDelta,
               const std::vector<bool>& mask,
               const std::vector<bool>& vals,
               Offset dest) {
  auto const actRecOff = entryArDelta + offsetFromIRSP(env, BCSPOffset{0});
  auto const funcPtr = gen(env, LdARFuncPtr,
                           IRSPOffsetData { actRecOff }, sp(env));
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
    gen(env, CheckRefs, failBlock, funcPtr, nParams,
        cns(env, i), cns(env, mask64), cns(env, vals64));
  }
}

//////////////////////////////////////////////////////////////////////

}}}
