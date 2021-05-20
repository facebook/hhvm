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
#include "hphp/runtime/vm/jit/irgen-control.h"

#include "hphp/runtime/vm/resumable.h"
#include "hphp/runtime/vm/unwind.h"

#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/switch-profile.h"
#include "hphp/runtime/vm/jit/target-profile.h"

#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"
#include "hphp/runtime/vm/jit/irgen-interpone.h"

namespace HPHP { namespace jit { namespace irgen {

void surpriseCheck(IRGS& env) {
  auto const ptr = resumeMode(env) != ResumeMode::None ? sp(env) : fp(env);
  auto const exit = makeExitSlow(env);
  gen(env, CheckSurpriseFlags, exit, ptr);
}

void surpriseCheck(IRGS& env, Offset relOffset) {
  if (relOffset <= 0 && !env.skipSurpriseCheck) {
    surpriseCheck(env);
  }
}

void surpriseCheckWithTarget(IRGS& env, Offset targetBcOff) {
  auto const ptr = resumeMode(env) != ResumeMode::None ? sp(env) : fp(env);
  auto const exit = makeExitSurprise(env, SrcKey{curSrcKey(env), targetBcOff});
  gen(env, CheckSurpriseFlags, exit, ptr);
}

/*
 * Returns an IR block corresponding to the given bytecode offset. The block
 * may be a side exit or a normal IR block, depending on whether or not the
 * offset is in the current RegionDesc.
 */
Block* getBlock(IRGS& env, Offset offset) {
  SrcKey sk(curSrcKey(env), offset);
  // If hasBlock returns true, then IRUnit already has a block for that offset
  // and makeBlock will just return it.  This will be the proper successor
  // block set by setSuccIRBlocks.  Otherwise, the given offset doesn't belong
  // to the region, so we just create an exit block.
  if (!env.irb->hasBlock(sk)) return makeExit(env, sk);

  return env.irb->makeBlock(sk, curProfCount(env));
}

//////////////////////////////////////////////////////////////////////

void jmpImpl(IRGS& env, Offset offset) {
  auto target = getBlock(env, offset);
  assertx(target != nullptr);
  gen(env, Jmp, target);
}

void implCondJmp(IRGS& env, Offset taken, bool negate, SSATmp* src) {
  auto const target = getBlock(env, taken);
  assertx(target != nullptr);
  auto const boolSrc = gen(env, ConvTVToBool, src);
  decRef(env, src);
  gen(env, negate ? JmpZero : JmpNZero, target, boolSrc);
}

//////////////////////////////////////////////////////////////////////

void emitJmp(IRGS& env, Offset relOffset) {
  surpriseCheck(env, relOffset);
  jmpImpl(env, bcOff(env) + relOffset);
}

void emitJmpNS(IRGS& env, Offset relOffset) {
  jmpImpl(env, bcOff(env) + relOffset);
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

void emitSwitch(IRGS& env, SwitchKind kind, int64_t base,
                const ImmVector& iv) {
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
  if (type <= TArrLike) {
    decRef(env, switchVal);
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

  auto const dataSize = SwitchProfile::extraSize(iv.size());
  TargetProfile<SwitchProfile> profile(
    env.context, env.irb->curMarker(), s_switchProfile.get(), dataSize
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
      SrcKey sk(curSrcKey(env), bcOff(env) + o);
      return env.irb->hasBlock(sk);
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
      SrcKey sk(curSrcKey(env), targetOff);
      if (!env.irb->hasBlock(sk)) continue;

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
  data.cases = iv.size();
  data.targets = &targets[0];
  data.spOffBCFromStackBase = spOffBCFromStackBase(env);
  data.spOffBCFromIRSP = spOffBCFromIRSP(env);

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
  data.bcSPOff    = spOffBCFromStackBase(env);

  auto const dest = gen(env,
                        fastPath ? LdSSwitchDestFast
                                 : LdSSwitchDestSlow,
                        data,
                        testVal);
  decRef(env, testVal);
  gen(
    env,
    JmpSSwitchDest,
    IRSPRelOffsetData { spOffBCFromIRSP(env) },
    dest,
    sp(env),
    fp(env)
  );
}

void emitThrowNonExhaustiveSwitch(IRGS& env) {
  interpOne(env);
}

const StaticString s_class_to_string(Strings::CLASS_TO_STRING);

void emitRaiseClassStringConversionWarning(IRGS& env) {
  if (RuntimeOption::EvalRaiseClassConversionWarning) {
      gen(env, RaiseWarning, cns(env, s_class_to_string.get()));
  }
}

//////////////////////////////////////////////////////////////////////

void emitSelect(IRGS& env) {
  auto const condSrc = popC(env);
  auto const boolSrc = gen(env, ConvTVToBool, condSrc);
  decRef(env, condSrc);

  ifThenElse(
    env,
    [&] (Block* taken) { gen(env, JmpZero, taken, boolSrc); },
    [&] { // True case
      auto const val = popC(env, DataTypeGeneric);
      popDecRef(env, DataTypeGeneric);
      push(env, val);
    },
    [&] { popDecRef(env, DataTypeGeneric); } // False case
  );
}

//////////////////////////////////////////////////////////////////////

void emitThrow(IRGS& env) {
  auto const stackEmpty = spOffBCFromStackBase(env) == spOffEmpty(env) + 1;
  auto const offset = findCatchHandler(curFunc(env), bcOff(env));
  auto const srcTy = topC(env)->type();
  auto const maybeThrowable =
    srcTy.maybe(Type::SubObj(SystemLib::s_ExceptionClass)) ||
    srcTy.maybe(Type::SubObj(SystemLib::s_ErrorClass));

  if (!stackEmpty || !maybeThrowable || !(srcTy <= TObj)) return interpOne(env);

  auto const handleThrow = [&] {
    if (offset != kInvalidOffset) return jmpImpl(env, offset);
    // There are no more catch blocks in this function, we are at the top
    // level throw
    auto const exn = popC(env);
    updateMarker(env);
    auto const etcData = EnterTCUnwindData { spOffBCFromIRSP(env), true };
    gen(env, EnterTCUnwind, etcData, sp(env), fp(env), exn);
  };

  if (srcTy <= Type::SubObj(SystemLib::s_ThrowableClass)) return handleThrow();

  ifThenElse(env,
    [&] (Block* taken) {
      assertx(srcTy <= TObj);
      auto const srcClass = gen(env, LdObjClass, topC(env));
      auto const ecdExc = ExtendsClassData { SystemLib::s_ExceptionClass };
      auto const isException = gen(env, ExtendsClass, ecdExc, srcClass);
      gen(env, JmpNZero, taken, isException);
      auto const ecdErr = ExtendsClassData { SystemLib::s_ErrorClass };
      auto const isError = gen(env, ExtendsClass, ecdErr, srcClass);
      gen(env, JmpNZero, taken, isError);
    },
    [&] { gen(env, Jmp, makeExitSlow(env)); },
    handleThrow
  );
}

//////////////////////////////////////////////////////////////////////

}}}
