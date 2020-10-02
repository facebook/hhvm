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

namespace HPHP { namespace jit { namespace irgen {

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
  auto vec = gen(env, AllocVArray, PackedArrayData { 2 });
  gen(env, InitVecElem, IndexData { 0 }, vec, cls);
  gen(env, InitVecElem, IndexData { 1 }, vec, func);
  return vec;
}

SSATmp* convertClassKey(IRGS& env, SSATmp* key) {
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
  return key;
}

void defineStack(IRGS& env, FPInvOffset bcSPOff) {
  // Define SP.
  if (resumeMode(env) != ResumeMode::None) {
    // rvmsp() points to the top of the stack in resumables
    auto const irSPOff = bcSPOff;
    gen(env, DefRegSP, DefStackData { irSPOff, bcSPOff });
  } else {
    // stack starts at the FP in regular functions
    auto const irSPOff = FPInvOffset { 0 };
    gen(env, DefFrameRelSP, DefStackData { irSPOff, bcSPOff }, fp(env));
  }

  // Now that the stack is initialized, update the BC marker and perform
  // initial sync of the exception stack boundary.
  updateMarker(env);
  env.irb->exceptionStackBoundary();

  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    // Assert that we're in the correct function.
    gen(env, DbgAssertFunc, fp(env));
  }
}

}}}
