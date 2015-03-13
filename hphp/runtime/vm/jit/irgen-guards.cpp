/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/target-profile.h"

#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"

namespace HPHP { namespace jit { namespace irgen {

namespace {

//////////////////////////////////////////////////////////////////////

enum class ProfGuard { GuardLoc, CheckLoc, GuardStk, CheckStk };

/*
 * Emit a type guard, possibly using profiling information. Depending on the
 * current translation mode and type to be guarded, this function may emit
 * additional profiling code or modify the guarded type using previously
 * collected profiling information. Str -> StaticStr is the only supported
 * refinement for now.
 */
void profiledGuard(HTS& env,
                   Type type,
                   ProfGuard kind,
                   int32_t id, // locId or stackOff
                   Block* checkExit) {
  switch (kind) {
  case ProfGuard::CheckLoc:
  case ProfGuard::GuardLoc:
    if (auto failBlock = env.irb->guardFailBlock()) {
      gen(env, CheckLoc, type, LocalId(id), failBlock, fp(env));
    } else if (kind == ProfGuard::CheckLoc) {
      gen(env, CheckLoc, type, LocalId(id), checkExit, fp(env));
    } else {
      gen(env, GuardLoc, type, LocalId(id), fp(env), sp(env));
    }
    return;
  case ProfGuard::CheckStk:
  case ProfGuard::GuardStk:
    {
      // Adjust 'id' to get an offset from the current m_irb->sp().
      auto const adjOff = offsetFromIRSP(env, BCSPOffset{id});
      if (auto failBlock = env.irb->guardFailBlock()) {
        gen(env, CheckStk,
          type, IRSPOffsetData { adjOff }, failBlock, sp(env));
      } else if (kind == ProfGuard::CheckStk) {
        gen(env, CheckStk,
          type, IRSPOffsetData { adjOff }, checkExit, sp(env));
      } else {
        gen(env, GuardStk, type,
          RelOffsetData { BCSPOffset{id}, adjOff }, sp(env), fp(env));
      }
      return;
    }
  }
}

void guardTypeStack(
  HTS& env,
  BCSPOffset stackIndex,
  Type type,
  bool outerOnly
) {
  assert(type <= Type::Cell || type <= Type::BoxedCell);

  // This should only be called at the beginning of a trace, with a clean
  // stack.
  assert(env.irb->evalStack().size() == 0);
  assert(env.irb->stackDeficit() == 0);
  auto const stackOff = RelOffsetData {
    stackIndex,
    offsetFromIRSP(env, stackIndex)
  };

  if (type <= Type::Cell) {
    profiledGuard(env, type, ProfGuard::GuardStk, stackIndex.offset, nullptr);
    return;
  }

  profiledGuard(env, Type::BoxedInitCell, ProfGuard::GuardStk,
    stackIndex.offset, nullptr);
  env.irb->constrainStack(stackOff.irSpOffset, DataTypeSpecific);
  gen(env, HintStkInner, type & Type::BoxedInitCell, stackOff, sp(env));

  if (!outerOnly && type <= Type::BoxedCell && type.inner() < Type::Cell) {
    auto stk = gen(env, LdStk, Type::BoxedInitCell,
      IRSPOffsetData{stackOff.irSpOffset}, sp(env));
    gen(env,
        CheckRefInner,
        env.irb->stackInnerTypePrediction(stackOff.irSpOffset),
        makeExit(env),
        stk);
  }
}

void guardTypeLocal(HTS& env, uint32_t locId, Type type, bool outerOnly) {
  assert(type <= Type::Cell || type <= Type::BoxedCell);

  if (type <= Type::Cell) {
    profiledGuard(env, type, ProfGuard::GuardLoc, locId, nullptr);
    return;
  }

  profiledGuard(env, Type::BoxedInitCell, ProfGuard::GuardLoc, locId, nullptr);
  gen(env,
      HintLocInner,
      type & Type::BoxedInitCell,
      LocalId { locId },
      fp(env));

  if (!outerOnly && type <= Type::BoxedCell && type.inner() < Type::Cell) {
    auto const ldrefExit = makeExit(env);
    auto const ldPMExit = makePseudoMainExit(env);
    auto const val = ldLoc(env, locId, ldPMExit, DataTypeSpecific);
    gen(env,
        CheckRefInner,
        env.irb->predictedInnerType(locId),
        ldrefExit,
        val);
  }
}

void checkTypeLocal(HTS& env, uint32_t locId, Type type, Offset dest) {
  assert(type <= Type::Cell || type <= Type::BoxedCell);

  if (type <= Type::Cell) {
    profiledGuard(env, type, ProfGuard::CheckLoc, locId, makeExit(env, dest));
    return;
  }

  profiledGuard(env, Type::BoxedInitCell, ProfGuard::CheckLoc, locId,
    makeExit(env, dest));
  gen(env,
      HintLocInner,
      type & Type::BoxedInitCell,
      LocalId { locId },
      fp(env));
}

uint64_t packBitVec(const std::vector<bool>& bits, unsigned i) {
  uint64_t retval = 0;
  assert(i % 64 == 0);
  assert(i < bits.size());
  while (i < bits.size()) {
    retval |= bits[i] << (i % 64);
    if ((++i % 64) == 0) {
      break;
    }
  }
  return retval;
}

void refCheckHelper(HTS& env,
                    int64_t entryArDelta,
                    const std::vector<bool>& mask,
                    const std::vector<bool>& vals,
                    Offset dest) {
  auto const actRecOff = entryArDelta + offsetFromIRSP(env, BCSPOffset{0});
  auto const funcPtr = gen(
    env,
    LdARFuncPtr,
    StackOffset { actRecOff.offset },
    sp(env)
  );
  SSATmp* nParams = nullptr;

  for (unsigned i = 0; i < mask.size(); i += 64) {
    assert(i < vals.size());

    uint64_t mask64 = packBitVec(mask, i);
    if (mask64 == 0) {
      continue;
    }

    if (i == 0) {
      nParams = cns(env, 64);
    } else if (!nParams || nParams->isConst()) {
      nParams = gen(env, LdFuncNumParams, funcPtr);
    }

    auto const vals64 = packBitVec(vals, i);
    if (auto failBlock = env.irb->guardFailBlock()) {
      gen(env,
          CheckRefs,
          failBlock,
          funcPtr,
          nParams,
          cns(env, i),
          cns(env, mask64),
          cns(env, vals64));
    } else if (dest == -1) {
      assert(offsetFromIRSP(env, BCSPOffset{0}) == 0);
      gen(env,
          GuardRefs,
          funcPtr,
          nParams,
          cns(env, i),
          cns(env, mask64),
          cns(env, vals64),
          fp(env),
          sp(env));
    } else {
      gen(env,
          CheckRefs,
          makeExit(env, dest),
          funcPtr,
          nParams,
          cns(env, i),
          cns(env, mask64),
          cns(env, vals64));
    }
  }
}

//////////////////////////////////////////////////////////////////////

}

void assertTypeLocal(HTS& env, uint32_t locId, Type type) {
  gen(env, AssertLoc, type, LocalId(locId), fp(env));
}

void assertTypeStack(HTS& env, BCSPOffset idx, Type type) {
  if (idx.offset < env.irb->evalStack().size()) {
    // We're asserting a new type so we don't care about the previous type.
    auto const tmp = top(env, Type::StkElem, idx, DataTypeGeneric);
    assert(tmp);
    env.irb->evalStack().replace(idx.offset, gen(env, AssertType, type, tmp));
  } else {
    gen(env,
        AssertStk,
        type,
        IRSPOffsetData { offsetFromIRSP(env, idx) },
        sp(env));
  }
}

void checkTypeStack(HTS& env, BCSPOffset idx, Type type, Offset dest) {
  assert(type <= Type::Gen);

  if (type <= Type::BoxedCell) {
    spillStack(env); // don't bother with the case that it's not spilled.
    auto const exit = makeExit(env, dest);
    auto const soff = RelOffsetData { idx, offsetFromIRSP(env, idx) };
    profiledGuard(env, Type::BoxedInitCell,
      ProfGuard::CheckStk, idx.offset, exit);
    env.irb->constrainStack(soff.irSpOffset, DataTypeSpecific);
    gen(env, HintStkInner, type & Type::BoxedInitCell, soff, sp(env));
    return;
  }

  auto exit = env.irb->guardFailBlock();
  if (exit == nullptr) exit = makeExit(env, dest);

  if (idx.offset < env.irb->evalStack().size()) {
    FTRACE(1, "checkTypeStack({}): generating CheckType for {}\n",
           idx.offset, type.toString());
    // CheckType only cares about its input type if the simplifier does
    // something with it and that's handled if and when it happens.
    auto const tmp = top(env, Type::StkElem, idx, DataTypeGeneric);
    assert(tmp);
    env.irb->evalStack().replace(idx.offset,
      gen(env, CheckType, type, exit, tmp));
    return;
  }
  FTRACE(1, "checkTypeStack({}): no tmp: {}\n", idx.offset, type.toString());
  // Just like CheckType, CheckStk only cares about its input type if the
  // simplifier does something with it.
  profiledGuard(env, type, ProfGuard::CheckStk, idx.offset, exit);
}

//////////////////////////////////////////////////////////////////////

void assertTypeLocation(HTS& env, const RegionDesc::Location& loc, Type type) {
  assert(type <= Type::StkElem);
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

void checkTypeLocation(HTS& env,
                       const RegionDesc::Location& loc,
                       Type type,
                       Offset dest) {
  assert(type <= Type::Gen);
  using T = RegionDesc::Location::Tag;
  switch (loc.tag()) {
  case T::Stack:
    checkTypeStack(env, offsetFromBCSP(env, loc.offsetFromFP()), type, dest);
    break;
  case T::Local:
    checkTypeLocal(env, loc.localId(), type, dest);
    break;
  }
}

void guardTypeLocation(HTS& env,
                       const RegionDesc::Location& l,
                       Type type,
                       bool outerOnly) {
  assert(type <= Type::Gen);
  using T = RegionDesc::Location::Tag;
  switch (l.tag()) {
  case T::Stack:
    guardTypeStack(env, offsetFromBCSP(env, l.offsetFromFP()), type, outerOnly);
    break;
  case T::Local:
    guardTypeLocal(env, l.localId(), type, outerOnly);
    break;
  }
}

void guardRefs(HTS& env,
               int64_t entryArDelta,
               const std::vector<bool>& mask,
               const std::vector<bool>& vals) {
  refCheckHelper(env, entryArDelta, mask, vals, -1);
}

void checkRefs(HTS& env,
               int64_t entryArDelta,
               const std::vector<bool>& mask,
               const std::vector<bool>& vals,
               Offset dest) {
  refCheckHelper(env, entryArDelta, mask, vals, dest);
}

//////////////////////////////////////////////////////////////////////

}}}
