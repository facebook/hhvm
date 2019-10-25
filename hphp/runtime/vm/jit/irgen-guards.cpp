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

void checkTypeLocal(IRGS& env, uint32_t locId, Type type,
                    Offset dest, bool outerOnly) {
  auto exit = env.irb->guardFailBlock();
  if (exit == nullptr) exit = makeExit(env, dest);

  gen(env, CheckLoc, type, LocalId(locId), exit, fp(env));
}

void checkTypeStack(IRGS& env, BCSPRelOffset idx, Type type,
                    Offset dest, bool outerOnly) {
  auto exit = env.irb->guardFailBlock();
  if (exit == nullptr) exit = makeExit(env, dest);

  auto const soff = IRSPRelOffsetData { offsetFromIRSP(env, idx) };
  gen(env, CheckStk, type, soff, exit, sp(env));
}

void checkTypeMBase(IRGS& env, Type type, Offset dest, bool outerOnly) {
  auto exit = env.irb->guardFailBlock();
  if (exit == nullptr) exit = makeExit(env, dest);

  auto const mbr = gen(env, LdMBase, TLvalToCell);
  gen(env, CheckMBase, type, exit, mbr);
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
  assertx(type <= TCell);

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
   }
}

void checkType(IRGS& env, const Location& loc,
               Type type, Offset dest, bool outerOnly) {
  assertx(type <= TCell);

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
  }
}

void predictType(IRGS& env, const Location& loc, Type type) {
  FTRACE(1, "predictType {}: {}\n", show(loc), type);
  assertx(type <= TCell);
  env.irb->fs().refinePredictedType(loc, type);
}

//////////////////////////////////////////////////////////////////////

}}}
