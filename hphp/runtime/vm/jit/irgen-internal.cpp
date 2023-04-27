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

#include "hphp/runtime/vm/jit/irgen-internal.h"

#include "hphp/runtime/vm/jit/fixup.h"

namespace HPHP::jit::irgen {

//////////////////////////////////////////////////////////////////////

const StaticString
  s_FATAL_NULL_THIS(Strings::FATAL_NULL_THIS),
  s_clsToStringWarning(Strings::CLASS_TO_STRING);

SSATmp* checkAndLoadThis(IRGS& env) {
  if (!hasThis(env)) {
    auto const err = cns(env, s_FATAL_NULL_THIS.get());
    gen(env, RaiseError, err);
    return cns(env, TBottom);
  }
  return ldThis(env);
}

SSATmp* convertClsMethToVec(IRGS& env, SSATmp* clsMeth) {
  assertx(clsMeth->isA(TClsMeth));
  auto const cls = gen(env, LdClsName, gen(env, LdClsFromClsMeth, clsMeth));
  auto const func = gen(env, LdFuncName, gen(env, LdFuncFromClsMeth, clsMeth));
  auto vec = gen(env, AllocVec, VanillaVecData { 2 });
  gen(env, InitVecElem, IndexData { 0 }, vec, cls);
  gen(env, InitVecElem, IndexData { 1 }, vec, func);
  return vec;
}

SSATmp* convertClassKey(IRGS& env, SSATmp* key) {
  assertx (key->type().isKnownDataType());
  if (key->isA(TCls)) {
    if (RuntimeOption::EvalRaiseClassConversionWarning) {
      gen(env, RaiseWarning, cns(env, s_clsToStringWarning.get()));
    }
    return gen(env, LdClsName, key);
  }
  if (key->isA(TLazyCls)) {
    if (RuntimeOption::EvalRaiseClassConversionWarning) {
      gen(env, RaiseWarning, cns(env, s_clsToStringWarning.get()));
    }
    return gen(env, LdLazyClsName, key);
  }
  assertx(!key->type().maybe(TCls | TLazyCls));
  return key;
}

void defineFrameAndStack(IRGS& env, SBInvOffset bcSPOff) {
  // Define FP and SP.
  if (resumeMode(env) != ResumeMode::None) {
    // - resumable frames live on the heap, so they do not have a stack position
    // - fp(env) and sp(env) are backed by rvmfp() and rvmsp() registers
    // - sp(env) points to the top of the stack at translation entry
    // - stack base is `irSPOff` away from sp(env)
    gen(env, DefFP, DefFPData { std::nullopt });
    updateMarker(env);

    auto const irSPOff = bcSPOff;
    gen(env, DefRegSP, DefStackData { irSPOff, bcSPOff });
  } else if (curSrcKey(env).funcEntry()) {
    // - frames of functions that are being called do not exist yet
    // - new native frame will be initialized at rvmsp() and linked to rvmfp()
    // - fp(env) and sp(env) will be backed by the same rvmfp() register
    // - stack base is numSlotsInFrame() away from sp(env)
    gen(env, DefFuncEntryFP);
    updateMarker(env);
    env.funcEntryPrevFP = gen(env, DefFuncEntryPrevFP);

    if (!curSrcKey(env).trivialDVFuncEntry()) {
      gen(env, EnterFrame, fp(env), env.funcEntryPrevFP);
      updateMarker(env);
    }

    assertx(bcSPOff == spOffEmpty(env));
    auto const irSPOff = SBInvOffset { -curFunc(env)->numSlotsInFrame() };
    gen(env, DefFrameRelSP, DefStackData { irSPOff, bcSPOff }, fp(env));
  } else {
    // - frames of regular functions live on the stack
    // - fp(env) and sp(env) are backed by the same rvmfp() register
    // - stack base is numSlotsInFrame() away from sp(env)
    gen(env, DefFP, DefFPData { IRSPRelOffset { 0 } });
    updateMarker(env);

    auto const irSPOff = SBInvOffset { -curFunc(env)->numSlotsInFrame() };
    gen(env, DefFrameRelSP, DefStackData { irSPOff, bcSPOff }, fp(env));
  }

  // Now that the stack is initialized, update the BC marker and perform
  // initial sync of the exception stack boundary.
  updateMarker(env);
  env.irb->exceptionStackBoundary();

  gen(env, EnterTranslation);

  if (RuntimeOption::EvalHHIRGenerateAsserts &&
      !curSrcKey(env).trivialDVFuncEntry()) {
    // Assert that we're in the correct function, but we can't do so
    // for trivial DV FuncEntries because the frame isn't setup yet
    // (we skip EnterFrame for them above).
    gen(env, DbgAssertFunc, fp(env));
  }
}

void handleConvNoticeLevel(
    IRGS& env,
    const ConvNoticeData& notice_data,
    const char* const from,
  const char* const to) {
  if (LIKELY(notice_data.level == ConvNoticeLevel::None)) return;

  assertx(notice_data.reason != nullptr);
  const auto str = makeStaticString(folly::sformat(
    "Implicit {} to {} conversion for {}", from, to, notice_data.reason));
  if (notice_data.level == ConvNoticeLevel::Throw) {
    gen(env, ThrowInvalidOperation, cns(env, str));
  }
  if (notice_data.level == ConvNoticeLevel::Log) {
    gen(env, RaiseNotice, cns(env, str));
  }
}

void genStVMReturnAddr(IRGS& env) {
  auto const addr = isInlining(env)
    ? cns(env, getNextFakeReturnAddress())
    : cns(env, 0);
  gen(env, StVMReturnAddr, addr);
}

}
