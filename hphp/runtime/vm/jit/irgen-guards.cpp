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

namespace HPHP::jit::irgen {

namespace {

//////////////////////////////////////////////////////////////////////

template<typename Check, typename Ld>
void checkTypeImpl(IRGS& env, Type type, Block* exit, Check check, Ld ld) {
  if (env.formingRegion || !type.isSpecialized()) {
    check(type, exit);
  } else {
    auto const test = type.unspecialize();
    check(test, exit);
    gen(env, CheckType, type, exit, ld(test));
  }
}

void checkTypeLocal(IRGS& env, uint32_t locId, Type type, Block* exit) {
  checkTypeImpl(env, type, exit,
    [&](Type test, Block* exit) {
      gen(env, CheckLoc, test, LocalId(locId), exit, fp(env));
    },
    [&](Type test) {
      test &= env.irb->fs().local(locId).type;
      return gen(env, LdLoc, test, LocalId(locId), fp(env));
    }
  );
}

void checkTypeStack(IRGS& env, BCSPRelOffset idx, Type type, Block* exit) {
  auto const soff = IRSPRelOffsetData { offsetFromIRSP(env, idx) };
  checkTypeImpl(env, type, exit,
    [&](Type test, Block* exit) {
      gen(env, CheckStk, test, soff, exit, sp(env));
    },
    [&](Type test) {
      test &= env.irb->fs().stack(soff.offset).type;
      return gen(env, LdStk, test, soff, sp(env));
    }
  );
}

void checkTypeMBase(IRGS& env, Type type, Block* exit) {
  auto const mbr = ldMBase(env);
  checkTypeImpl(env, type, exit,
    [&](Type test, Block* exit) {
      gen(env, CheckMBase, test, exit, mbr);
    },
    [&](Type test) {
      test &= env.irb->fs().mbase().type;
      return gen(env, LdMem, test, mbr);
    }
  );
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

void checkType(IRGS& env, const Location& loc, Type type, Block* exit) {
  assertx(type <= TCell);

  switch (loc.tag()) {
    case LTag::Stack:
      checkTypeStack(env, offsetFromBCSP(env, loc.stackIdx()), type, exit);
      break;
    case LTag::Local:
      checkTypeLocal(env, loc.localId(), type, exit);
      break;
    case LTag::MBase:
      checkTypeMBase(env, type, exit);
      break;
  }
}

SSATmp* loadLocation(IRGS& env, const Location& loc) {
  switch (loc.tag()) {
    case LTag::Local:
      return gen(env, LdLoc, TCell, LocalId(loc.localId()), fp(env));
    case LTag::Stack: {
      auto const soff = IRSPRelOffsetData {
        offsetFromIRSP(env, loc.stackIdx()) };
      return gen(env, LdStk, TCell, soff, sp(env));
    }
    case LTag::MBase: {
      auto const mbr = ldMBase(env);
      return gen(env, LdMem, env.irb->fs().mbase().type, mbr);
    }
  }
  not_reached();
}

//////////////////////////////////////////////////////////////////////

}
