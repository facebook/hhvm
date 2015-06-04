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
#include "hphp/runtime/vm/jit/irgen-control.h"

#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/target-profile.h"

#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"

namespace HPHP { namespace jit { namespace irgen {

void surpriseCheck(IRGS& env, Offset relOffset) {
  if (relOffset <= 0) {
    auto const ptr = resumed(env) ? sp(env) : fp(env);
    auto const exit = makeExitSlow(env);
    gen(env, CheckSurpriseFlags, exit, ptr);
  }
}

/*
 * Returns an IR block corresponding to the given bytecode offset. The block
 * may be a side exit or a normal IR block, depending on whether or not the
 * offset is in the current RegionDesc.
 */
Block* getBlock(IRGS& env, Offset offset) {
  // If hasBlock returns true, then IRUnit already has a block for that offset
  // and makeBlock will just return it.  This will be the proper successor
  // block set by setSuccIRBlocks.  Otherwise, the given offset doesn't belong
  // to the region, so we just create an exit block.
  if (!env.irb->hasBlock(offset)) return makeExit(env, offset);

  return env.irb->makeBlock(offset);
}

//////////////////////////////////////////////////////////////////////

void jmpImpl(IRGS& env, Offset offset, JmpFlags flags) {
  if (flags & JmpFlagNextIsMerge) {
    prepareForHHBCMergePoint(env);
  }
  auto target = getBlock(env, offset);
  assertx(target != nullptr);
  gen(env, Jmp, target);
}

void implCondJmp(IRGS& env, Offset taken, bool negate, SSATmp* src) {
  auto const flags = instrJmpFlags(*env.currentNormalizedInstruction);
  if (flags & JmpFlagEndsRegion) {
    spillStack(env);
  }
  if ((flags & JmpFlagNextIsMerge) != 0) {
    prepareForHHBCMergePoint(env);
  }
  auto const target = getBlock(env, taken);
  assertx(target != nullptr);
  auto const boolSrc = gen(env, ConvCellToBool, src);
  gen(env, DecRef, src);
  gen(env, negate ? JmpZero : JmpNZero, target, boolSrc);
}

//////////////////////////////////////////////////////////////////////

void emitJmp(IRGS& env, Offset relOffset) {
  surpriseCheck(env, relOffset);
  auto const offset = bcOff(env) + relOffset;
  jmpImpl(env, offset, instrJmpFlags(*env.currentNormalizedInstruction));
}

void emitJmpNS(IRGS& env, Offset relOffset) {
  jmpImpl(env, bcOff(env) + relOffset,
    instrJmpFlags(*env.currentNormalizedInstruction));
}

void emitJmpZ(IRGS& env, Offset relOffset) {
  surpriseCheck(env, relOffset);
  auto const takenOff = bcOff(env) + relOffset;
  implCondJmp(env, takenOff, true, popC(env));
}

void emitJmpNZ(IRGS& env, Offset relOffset) {
  surpriseCheck(env, relOffset);
  auto const takenOff = bcOff(env) + relOffset;
  implCondJmp(env, takenOff, false, popC(env));
}

//////////////////////////////////////////////////////////////////////

static const StaticString s_switchProfile("SwitchProfile");

void emitSwitch(IRGS& env,
                const ImmVector& iv,
                int64_t base,
                SwitchKind kind) {
  auto bounded = kind == SwitchKind::Bounded;
  int nTargets = bounded ? iv.size() - 2 : iv.size();

  SSATmp* const switchVal = popC(env);
  Type type = switchVal->type();
  assertx(IMPLIES(!(type <= TInt), bounded));
  assertx(IMPLIES(bounded, iv.size() > 2));
  SSATmp* index;
  SSATmp* ssabase = cns(env, base);
  SSATmp* ssatargets = cns(env, nTargets);

  Offset defaultOff = bcOff(env) + iv.vec32()[iv.size() - 1];
  Offset zeroOff = 0;
  if (base <= 0 && (base + nTargets) > 0) {
    zeroOff = bcOff(env) + iv.vec32()[0 - base];
  } else {
    zeroOff = defaultOff;
  }

  if (instrJmpFlags(*env.currentNormalizedInstruction) & JmpFlagNextIsMerge) {
    prepareForHHBCMergePoint(env);
  }

  if (type <= TNull) {
    gen(env, Jmp, getBlock(env, zeroOff));
    return;
  }
  if (type <= TBool) {
    Offset nonZeroOff = bcOff(env) + iv.vec32()[iv.size() - 2];
    gen(env, JmpNZero, getBlock(env, nonZeroOff), switchVal);
    gen(env, Jmp, getBlock(env, zeroOff));
    return;
  }
  if (type <= TArr) {
    gen(env, DecRef, switchVal);
    gen(env, Jmp, getBlock(env, defaultOff));
    return;
  }

  if (type <= TInt) {
    // No special treatment needed
    index = switchVal;
  } else if (type <= TDbl) {
    // switch(Double|String|Obj)Helper do bounds-checking for us, so we need to
    // make sure the default case is in the jump table, and don't emit our own
    // bounds-checking code.
    bounded = false;
    index = gen(env, LdSwitchDblIndex, switchVal, ssabase, ssatargets);
  } else if (type <= TStr) {
    bounded = false;
    index = gen(env, LdSwitchStrIndex, switchVal, ssabase, ssatargets);
  } else if (type <= TObj) {
    // switchObjHelper can throw exceptions and reenter the VM so we use the
    // catch block here.
    bounded = false;
    index = gen(env, LdSwitchObjIndex, switchVal, ssabase, ssatargets);
  } else {
    PUNT(Switch-UnknownType);
  }

  auto const dataSize = iv.size() * sizeof(SwitchProfile::cases[0]);
  TargetProfile<SwitchProfile> profile(
    env.unit.context(), env.irb->curMarker(), s_switchProfile.get(),
    dataSize
  );

  auto checkBounds = [&] {
    if (!bounded) return;
    index = gen(env, SubInt, index, cns(env, base));
    auto const ok = gen(env, CheckRange, index, cns(env, nTargets));
    gen(env, JmpZero, getBlock(env, defaultOff), ok);
    bounded = false;
  };
  auto const offsets = iv.range32();

  // We lower Switch to a series of comparisons if any of the successors are in
  // included in the region.
  auto const shouldLower =
    std::any_of(offsets.begin(), offsets.end(), [&](Offset o) {
      return env.irb->hasBlock(bcOff(env) + o);
    });
  if (shouldLower && profile.optimizing()) {
    auto const values = sortedSwitchProfile(profile, iv.size());
    FTRACE(2, "Switch profile data for Switch @ {}\n", bcOff(env));
    for (UNUSED auto const& val : values) {
      FTRACE(2, "  case {} hit {} times\n", val.caseIdx, val.count);
    }

    // Emit conditional checks for all successors in this region, in descending
    // order of hotness. We rely on the region selector to decide which arcs
    // are appropriate to include in the region. Fall through to the
    // fully-generic JmpSwitchDest at the end if nothing matches.
    for (auto const& val : values) {
      auto targetOff = bcOff(env) + offsets[val.caseIdx];
      if (!env.irb->hasBlock(targetOff)) continue;

      if (bounded && val.caseIdx == iv.size() - 2) {
        // If we haven't checked bounds yet and this is the "first non-zero"
        // case, we have to skip it. This case is only hit for non-Int input
        // types anyway.
        continue;
      }

      if (val.caseIdx == iv.size() - 1) {
        // Default case.
        checkBounds();
      } else {
        auto ok = gen(env, EqInt, cns(env, val.caseIdx + (bounded ? base : 0)),
                      index);
        gen(env, JmpNZero, getBlock(env, targetOff), ok);
      }
    }
  } else if (profile.profiling()) {
    gen(env, ProfileSwitchDest,
        ProfileSwitchData{profile.handle(), iv.size(), bounded ? base : 0},
        index);
  }

  // Make sure to check bounds, if we haven't yet.
  checkBounds();

  std::vector<SrcKey> targets;
  targets.reserve(iv.size());
  for (auto const offset : offsets) {
    targets.emplace_back(SrcKey{curSrcKey(env), bcOff(env) + offset});
  }

  auto data = JmpSwitchData{};
  data.cases       = iv.size();
  data.targets     = &targets[0];
  data.invSPOff    = invSPOff(env);
  data.irSPOff     = offsetFromIRSP(env, BCSPOffset{0});

  spillStack(env);
  gen(env, JmpSwitchDest, data, index, sp(env), fp(env));
}

void emitSSwitch(IRGS& env, const ImmVector& iv) {
  const int numCases = iv.size() - 1;

  /*
   * We use a fast path translation with a hashtable if none of the
   * cases are numeric strings and if the input is actually a string.
   *
   * Otherwise we do a linear search through the cases calling string
   * conversion routines.
   */
  const bool fastPath =
    topC(env)->isA(TStr) &&
    std::none_of(iv.strvec(), iv.strvec() + numCases,
      [&](const StrVecItem& item) {
        return curUnit(env)->lookupLitstrId(item.str)->isNumeric();
      }
    );

  auto const testVal = popC(env);

  std::vector<LdSSwitchData::Elm> cases(numCases);
  for (int i = 0; i < numCases; ++i) {
    auto const& kv = iv.strvec()[i];
    cases[i].str  = curUnit(env)->lookupLitstrId(kv.str);
    cases[i].dest = SrcKey{curSrcKey(env), bcOff(env) + kv.dest};
  }

  LdSSwitchData data;
  data.numCases   = numCases;
  data.cases      = &cases[0];
  data.defaultSk  = SrcKey{curSrcKey(env),
                           bcOff(env) + iv.strvec()[iv.size() - 1].dest};
  data.spOff      = invSPOff(env);

  auto const dest = gen(env,
                        fastPath ? LdSSwitchDestFast
                                 : LdSSwitchDestSlow,
                        data,
                        testVal);
  gen(env, DecRef, testVal);
  gen(
    env,
    JmpSSwitchDest,
    IRSPOffsetData { offsetFromIRSP(env, BCSPOffset{0}) },
    dest,
    sp(env),
    fp(env)
  );
}

//////////////////////////////////////////////////////////////////////

}}}
