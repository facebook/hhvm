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

#include "hphp/runtime/vm/jit/normalized-instruction.h"

#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"
#include "hphp/runtime/vm/jit/irgen-interpone.h"

namespace HPHP { namespace jit { namespace irgen {

namespace {

//////////////////////////////////////////////////////////////////////

// Return a constant SSATmp representing a static value held in a TypedValue.
// The TypedValue may be a non-scalar, but it must have a static value.
SSATmp* staticTVCns(IRGS& env, const TypedValue* tv) {
  switch (tv->m_type) {
    case KindOfNull:          return cns(env, TInitNull);
    case KindOfBoolean:       return cns(env, !!tv->m_data.num);
    case KindOfInt64:         return cns(env, tv->m_data.num);
    case KindOfDouble:        return cns(env, tv->m_data.dbl);
    case KindOfPersistentString:
    case KindOfString:        return cns(env, tv->m_data.pstr);
    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentArray:
    case KindOfArray:         return cns(env, tv->m_data.parr);

    case KindOfUninit:
    case KindOfObject:
    case KindOfResource:
    case KindOfRef:
      break;
  }
  always_assert(false);
}

void implCns(IRGS& env,
             const StringData* name,
             const StringData* fallbackName,
             bool error) {
  assertx(fallbackName == nullptr || !error);
  auto const cnsNameTmp = cns(env, name);
  auto const tv = Unit::lookupPersistentCns(name);
  SSATmp* result = nullptr;

  SSATmp* fallbackNameTmp = nullptr;
  if (fallbackName != nullptr) {
    fallbackNameTmp = cns(env, fallbackName);
  }
  if (tv) {
    if (tv->m_type == KindOfUninit) {
      // KindOfUninit is a dynamic system constant. always a slow
      // lookup.
      assertx(!fallbackNameTmp);
      if (error) {
        result = gen(env, LookupCnsE, cnsNameTmp);
      } else {
        result = gen(env, LookupCns, cnsNameTmp);
      }
    } else {
      result = staticTVCns(env, tv);
    }
  } else {
    result = cond(
      env,
      [&] (Block* taken) { // branch
        return gen(env, LdCns, taken, cnsNameTmp);
      },
      [&] (SSATmp* cns) { // Next: LdCns hit in TC
        return cns;
      },
      [&] { // Taken: miss in TC, do lookup & init
        hint(env, Block::Hint::Unlikely);

        if (fallbackNameTmp) {
          return gen(env,
                     LookupCnsU,
                     cnsNameTmp,
                     fallbackNameTmp);
        }
        if (error) {
          return gen(env, LookupCnsE, cnsNameTmp);
        }
        return gen(env, LookupCns, cnsNameTmp);
      }
    );
  }
  push(env, result);
}

//////////////////////////////////////////////////////////////////////

}

void emitCns(IRGS& env, const StringData* name) {
  implCns(env, name, nullptr, false);
}

void emitCnsE(IRGS& env, const StringData* name) {
  implCns(env, name, nullptr, true);
}

void emitCnsU(IRGS& env,
              const StringData* name,
              const StringData* fallback) {
  implCns(env, name, fallback, false);
}

void implClsCns(IRGS& env,
                const Class* cls,
                const StringData* cnsNameStr,
                const StringData* clsNameStr) {
  auto const clsCnsName = ClsCnsName { clsNameStr, cnsNameStr };

  // If the class is already defined in this request, the class is persistent
  // or a parent of the current context, and this constant is a scalar
  // constant, we can just compile it to a literal.
  if (cls) {
    Slot ignore;
    auto const tv = cls->cnsNameToTV(cnsNameStr, ignore);
    if (tv && tv->m_type != KindOfUninit &&
        classIsPersistentOrCtxParent(env, cls)) {
      push(env, staticTVCns(env, tv));
      return;
    }
  }

  // Otherwise, load the constant out of RDS.  Right now we always guard that
  // it is at least uncounted (this means a constant set to STDIN or something
  // will always side exit here).
  cond(
    env,
    [&] (Block* taken) {
      auto const prds = gen(env, LdClsCns, clsCnsName, taken);
      gen(env, CheckTypeMem, TUncountedInit, taken, prds);
      return prds;
    },
    [&] (SSATmp* prds) {
      auto const val = gen(env, LdMem, TUncountedInit, prds);
      push(env, val);
      return nullptr;
    },
    [&] () -> SSATmp* {
      // Make progress through this instruction before side-exiting to the next
      // instruction, by doing a slower lookup.
      hint(env, Block::Hint::Unlikely);
      auto const val = gen(env, InitClsCns, clsCnsName);
      push(env, val);
      gen(env, Jmp, makeExit(env, nextBcOff(env)));
      return nullptr;
    }
  );
}

void emitClsCnsD(IRGS& env,
                 const StringData* cnsNameStr,
                 const StringData* clsNameStr) {
  implClsCns(env, Unit::lookupClass(clsNameStr), cnsNameStr, clsNameStr);
}

void emitClsCns(IRGS& env, const StringData* cnsNameStr, uint32_t slot) {
  auto const clsTmp = peekClsRef(env, slot);
  auto const clsTy = clsTmp->type();
  if (!clsTy.clsSpec()) {
    interpOne(env, *env.currentNormalizedInstruction);
    return;
  }
  auto const cls = clsTy.clsSpec().cls();

  ifThenElse(
    env,
    [&] (Block* taken) {
      gen(env, CheckType, taken, Type::ExactCls(cls), clsTmp);
    },
    [&] {
      killClsRef(env, slot);
      implClsCns(env, cls, cnsNameStr, cls->name());
    },
    [&] {
      gen(env, Jmp, makeExitSlow(env));
    }
  );
}

//////////////////////////////////////////////////////////////////////

}}}
