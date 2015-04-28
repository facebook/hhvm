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

#include "hphp/runtime/vm/jit/normalized-instruction.h"

#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"

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
    case KindOfStaticString:
    case KindOfString:        return cns(env, tv->m_data.pstr);
    case KindOfArray:         return cns(env, tv->m_data.parr);

    case KindOfUninit:
    case KindOfObject:
    case KindOfResource:
    case KindOfRef:
    case KindOfClass:
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
    auto const c1 = gen(env, LdCns, cnsNameTmp);
    result = cond(
      env,
      [&] (Block* taken) { // branch
        gen(env, CheckInit, taken, c1);
      },
      [&] { // Next: LdCns hit in TC
        return c1;
      },
      [&] { // Taken: miss in TC, do lookup & init
        hint(env, Block::Hint::Unlikely);
        // We know that c1 is Uninit in this branch but we have to encode this
        // in the IR.
        gen(env, AssertType, TUninit, c1);

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

void emitClsCnsD(IRGS& env,
                 const StringData* cnsNameStr,
                 const StringData* clsNameStr) {
  auto const clsCnsName = ClsCnsName { clsNameStr, cnsNameStr };

  /*
   * If the class is already defined in this request, the class is persistent
   * or a parent of the current context, and this constant is a scalar
   * constant, we can just compile it to a literal.
   */
  if (auto const cls = Unit::lookupClass(clsNameStr)) {
    Slot ignore;
    auto const tv = cls->cnsNameToTV(cnsNameStr, ignore);
    if (tv && tv->m_type != KindOfUninit &&
        classIsPersistentOrCtxParent(env, cls)) {
      push(env, staticTVCns(env, tv));
      return;
    }
  }

  /*
   * Otherwise, load the constant out of RDS.  Right now we always guard that
   * it is at least uncounted (this means a constant set to STDIN or something
   * will always side exit here).
   */
  auto const link = rds::bindClassConstant(clsNameStr, cnsNameStr);
  auto const prds = gen(
    env,
    LdRDSAddr,
    RDSHandleData { link.handle() },
    TCell.ptr(Ptr::ClsCns)
  );
  auto const guardType = TUncountedInit;

  ifThen(
    env,
    [&] (Block* taken) {
      gen(env, CheckTypeMem, guardType, taken, prds);
    },
    [&] {
      // Make progress through this instruction before side-exiting to the next
      // instruction, by doing a slower lookup.
      hint(env, Block::Hint::Unlikely);
      auto const val = gen(env, LookupClsCns, clsCnsName);
      push(env, val);
      gen(env, Jmp, makeExit(env, nextBcOff(env)));
    }
  );

  auto const val = gen(env, LdMem, guardType, prds);
  push(env, val);
}

//////////////////////////////////////////////////////////////////////

}}}

