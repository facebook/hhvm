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

#include "hphp/runtime/vm/class-meth-data-ref.h"
#include "hphp/runtime/vm/jit/cls-cns-profile.h"
#include "hphp/runtime/vm/jit/irgen-call.h"
#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"
#include "hphp/runtime/vm/jit/irgen-interpone.h"
#include "hphp/runtime/vm/jit/target-profile.h"

namespace HPHP::jit::irgen {

///////////////////////////////////////////////////////////////////////////////

void emitCnsE(IRGS& env, const StringData* name) {
  if (auto const tv = Constant::lookupPersistent(name)) {
    if (tv->m_type == KindOfUninit) {
      // KindOfUninit is a dynamic system constant. always a slow
      // lookup.
      push(env, gen(env, LookupCnsE, cns(env, name)));
      return;
    }

    if (auto const val = Type::tryCns(*tv)) {
      push(env, cns(env, *val));
      return;
    }
  }

  auto const v = cond(
    env,
    [&] (Block* taken) {
      return gen(env, LdCns, taken, cns(env, name));
    },
    [&] (SSATmp* v) {
      gen(env, IncRef, v);
      return v;
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      return gen(env, LookupCnsE, cns(env, name));
    }
  );
  push(env, v);
}

namespace {

//////////////////////////////////////////////////////////////////////

void exactClsCns(IRGS& env,
                const Class* cls,
                const StringData* cnsNameStr,
                const StringData* clsNameStr) {
  auto const clsCnsName = ClsCnsName { clsNameStr, cnsNameStr };

  if (cls &&
      (will_symbol_raise_module_boundary_violation(cls, curFunc(env))
      || env.unit.packageInfo().violatesDeploymentBoundary(*cls))) {
    auto const cns = gen(env, InitClsCns, TInitCell, clsCnsName);
    pushIncRef(env, cns);
    return;
  }

  // If the class is already defined in this request, the class is persistent
  // or a parent of the current context, and this constant is a scalar
  // constant, we can just compile it to a literal.
  auto cnsType = TInitCell;
  if (classIsPersistentOrCtxParent(env, cls)) {
    Slot ignore;
    if (auto const tv = cls->cnsNameToTV(cnsNameStr, ignore)) {
      if (type(tv) != KindOfUninit) {
        if (auto const val = Type::tryCns(*tv)) {
          push(env, cns(env, *val));
          return;
        }
        cnsType = typeFromTV(tv, curClass(env));
      }
    }
  }

  // Otherwise, load the constant out of RDS.
  auto const cns = cond(
    env,
    [&] (Block* taken) {
      return gen(env, LdClsCns, cnsType, clsCnsName, taken);
    },
    [&] (SSATmp* cns) { return cns; },
    [&] {
      hint(env, Block::Hint::Unlikely);
      return gen(env, InitClsCns, cnsType, clsCnsName);
    }
  );
  pushIncRef(env, cns);
}

StaticString clsCnsProfileKey { "ClsCnsProfile" };

//////////////////////////////////////////////////////////////////////

} // namespace

void emitClsCnsD(IRGS& env,
                 const StringData* cnsNameStr,
                 const StringData* clsNameStr) {
  exactClsCns(env, Class::lookup(clsNameStr), cnsNameStr, clsNameStr);
}

void emitClsCns(IRGS& env, const StringData* cnsNameStr) {
  auto const clsTmp = topC(env);
  auto const clsTy = clsTmp->type();

  if (!(clsTy <= TCls)) PUNT(ClsCns-NotClass);

  auto const loadSlot = [&] (Slot slot) {
    auto const data = ClsCnsSlotData { cnsNameStr, slot };
    auto const cns = cond(
      env,
      [&] (Block* taken) {
        auto const slotVal = gen(env, LdSubClsCns, data, clsTmp);
        return gen(env, CheckType, TUncountedInit, taken, slotVal);
      },
      [&] (SSATmp* val) { return val; },
      [&] {
        hint(env, Block::Hint::Unlikely);
        auto const cns = gen(env, InitSubClsCns, data, clsTmp);
        // The value may be ref-counted if we take the slow path
        gen(env, IncRef, cns);
        return cns;
      }
    );
    discard(env);
    push(env, cns);
  };

  if (!clsTy.clsSpec() || !isNormalClass(clsTy.clsSpec().cls())) {
    TargetProfile<ClsCnsProfile> profile(env.context,
                                         env.irb->curMarker(),
                                         clsCnsProfileKey.get());
    if (profile.profiling()) {
      auto const data = ProfileSubClsCnsData { cnsNameStr, profile.handle() };
      auto const cns = gen(
        env,
        CheckType,
        TInitCell,
        makeExitSlow(env),
        gen(env, ProfileSubClsCns, data, clsTmp)
      );
      discard(env);
      pushIncRef(env, cns);
      return;
    }

    if (profile.optimizing()) {
      auto const slot = profile.data().getSlot();
      if (slot != kInvalidSlot) {
        auto const exit = makeExitSlow(env);
        auto const len = gen(env, LdClsCnsVecLen, clsTmp);
        auto const cmp = gen(env, LteInt, len, cns(env, slot));
        gen(env, JmpNZero, exit, cmp);
        auto const data = ClsCnsSlotData { cnsNameStr, slot };
        gen(env, CheckSubClsCns, data, exit, clsTmp);
        return loadSlot(slot);
      }
    }

    return interpOne(env);
  }

  auto const cls = clsTy.clsSpec().cls();
  if (clsTy.clsSpec().exact()) {
    discard(env);
    exactClsCns(env, cls, cnsNameStr, cls->name());
  } else {
    auto const slot =
      cls->clsCnsSlot(cnsNameStr, ConstModifiers::Kind::Value, true);
    if (slot == kInvalidSlot) return interpOne(env);
    loadSlot(slot);
  }
}

void emitClsCnsL(IRGS& env, int32_t id) {
  auto const cls = topC(env);
  if (!cls->isA(TCls)) PUNT(ClsCns-NotClass);
  auto const cnsName = ldLoc(env, id, DataTypeSpecific);
  if (!cnsName->isA(TStr)) PUNT(ClsCns-NotStr);
  if (cnsName->hasConstVal(TStr)) {
    emitClsCns(env, cnsName->strVal());
  } else {
    auto const cns = gen(env, LookupClsCns, cls, cnsName);
    popDecRef(env);
    pushIncRef(env, cns);
  }
}

//////////////////////////////////////////////////////////////////////

}
