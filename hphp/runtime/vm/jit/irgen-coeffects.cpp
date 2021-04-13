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
#include "hphp/runtime/vm/coeffects.h"

#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"
#include "hphp/runtime/vm/jit/irgen-interpone.h"
#include "hphp/runtime/vm/jit/irgen-state.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"

#include "hphp/runtime/ext/asio/ext_async-generator.h"
#include "hphp/runtime/ext/generator/ext_generator.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

namespace jit {
namespace irgen {
namespace {

SSATmp* emitCCParam(IRGS& env, const Func* f, uint32_t numArgsInclUnpack,
                    uint32_t paramIdx, const StringData* name) {
  if (paramIdx >= numArgsInclUnpack) return nullptr;
  auto const index =
    numArgsInclUnpack - 1 - paramIdx + (f->hasReifiedGenerics() ? 1 : 0);
  auto const tv = top(env, BCSPRelOffset {static_cast<int32_t>(index)});
  return cond(
    env,
    [&] (Block* taken) { return gen(env, CheckType, TObj, taken, tv); },
    [&] (SSATmp* obj) {
      auto const cls  = gen(env, LdObjClass, obj);
      return gen(env, LookupClsCtxCns, cls, cns(env, name));
    },
    [&] (Block* taken) { return gen(env, CheckType, TNull, taken, tv); },
    [&] (SSATmp*) { return cns(env, RuntimeCoeffects::full().value()); },
    [&] {
      hint(env, Block::Hint::Unlikely);
      auto const msg =
        folly::sformat("Coeffect rule requires parameter at position "
                       "{} to be an object or null",
                       paramIdx + 1);
      gen(env, RaiseError, cns(env, makeStaticString(msg)));
      return cns(env, 0);
    }
  );
}

SSATmp* emitCCThis(IRGS& env, const Func* f, const StringData* name,
                   SSATmp* prologueCtx) {
  assertx(!f->isClosureBody());
  assertx(f->isMethod());
  auto const cls =
    f->isStatic() ? prologueCtx : gen(env, LdObjClass, prologueCtx);
  return gen(env, LookupClsCtxCns, cls, cns(env, name));
}

SSATmp* emitFunParam(IRGS& env, const Func* f, uint32_t numArgsInclUnpack,
                     uint32_t paramIdx) {
  if (paramIdx >= numArgsInclUnpack) return nullptr;
  auto const index =
    numArgsInclUnpack - 1 - paramIdx + (f->hasReifiedGenerics() ? 1 : 0);
  auto const tv = top(env, BCSPRelOffset {static_cast<int32_t>(index)});

  auto const fail = [&] {
    hint(env, Block::Hint::Unlikely);
    auto const data = ParamData { static_cast<int32_t>(paramIdx) };
    gen(env, RaiseCoeffectsFunParamTypeViolation, data, tv);
    return cns(env, RuntimeCoeffects::full().value());
  };

  auto const objSuccess = [&](SSATmp* obj) {
    auto const cls = gen(env, LdObjClass, obj);
    return cond(
      env,
      [&] (Block* taken) {
        auto const data = AttrData { AttrIsClosureClass };
        auto const success = gen(env, ClassHasAttr, data, cls);
        gen(env, JmpZero, taken, success);
      },
      [&] {
        return cond(
          env,
          [&] (Block* taken) {
            auto const data = AttrData { AttrHasClosureCoeffectsProp };
            auto const success = gen(env, ClassHasAttr, data, cls);
            gen(env, JmpZero, taken, success);
          },
          [&] {
            // Rules
            auto const addr = gen(env, LdPropAddr, IndexData { 0 },
                                  TInt.lval(Ptr::Prop), obj);
            return gen(env, LdMem, TInt, addr);
          },
          [&] {
            // Statically known coeffects
            auto const unreachable = makeUnreachable(env, ASSERT_REASON);
            auto const invoke = gen(env, LdObjInvoke, unreachable, cls);
            return gen(env, LdFuncRequiredCoeffects, invoke);
          }
        );
      },
      fail
    );
  };

  auto const fnPtrs = [&] {
    auto const is_any_func_type = [&] (Block* toFail) {
      return cond(
        env,
        [&] (Block* taken) { return gen(env, CheckType, TFunc, taken, tv); },
        [&] (SSATmp* func) { return func; },
        [&] (Block* taken) { return gen(env, CheckType, TRFunc, taken, tv); },
        [&] (SSATmp* ptr)  { return gen(env, LdFuncFromRFunc, ptr); },
        [&] (Block* taken) { return gen(env, CheckType, TClsMeth, taken, tv); },
        [&] (SSATmp* ptr)  { return gen(env, LdFuncFromClsMeth, ptr); },
        [&] (Block* taken) { return gen(env, CheckType, TRClsMeth, taken, tv); },
        [&] (SSATmp* ptr)  { return gen(env, LdFuncFromRClsMeth, ptr); },
        [&] {
          gen(env, Jmp, toFail);
          // To keep JIT type system happy
          return cns(env, SystemLib::s_nullFunc);
        }
      );
    };
    auto const handle_func = [&](SSATmp* func) {
      return cond(
        env,
        [&] (Block* taken) {
          auto const data = AttrData { AttrHasCoeffectRules };
          auto const success = gen(env, FuncHasAttr, data, func);
          gen(env, JmpNZero, taken, success);
        },
        [&] {
          // Static coeffects
          return gen(env, LdFuncRequiredCoeffects, func);
        },
        [&] {
          // Rules
          hint(env, Block::Hint::Unlikely);
          gen(env, RaiseCoeffectsFunParamCoeffectRulesViolation, func);
          return cns(env, RuntimeCoeffects::full().value());
        }
      );
    };
    return cond(env, is_any_func_type, handle_func, fail);
  };

  return cond(
    env,
    [&] (Block* taken) { return gen(env, CheckType, TNull, taken, tv); },
    [&] (SSATmp*)      { return cns(env, RuntimeCoeffects::full().value()); },
    [&] (Block* taken) { return gen(env, CheckType, TObj, taken, tv); },
    objSuccess,
    fnPtrs
  );

}

SSATmp* emitClosureInheritFromParent(IRGS& env, const Func* f,
                                     SSATmp* prologueCtx) {
  assertx(prologueCtx);
  assertx(f->isClosureBody());
  auto const cls = f->implCls();
  assertx(cls);
  auto const slot = cls->getCoeffectsProp();
  auto const addr = gen(env, LdPropAddr,
                        IndexData { cls->propSlotToIndex(slot) },
                        TInt.lval(Ptr::Prop), prologueCtx);
  return gen(env, LdMem, TInt, addr);
}

SSATmp* emitGeneratorThis(IRGS& env, const Func* f, SSATmp* prologueCtx) {
  assertx(f->isMethod() && !f->isStatic() && f->implCls() &&
          (f->implCls() == AsyncGenerator::getClass() ||
           f->implCls() == Generator::getClass()));
  auto const isAsync = f->implCls()->classof(AsyncGenerator::getClass());
  auto const genObj = prologueCtx;
  return cond(
    env,
    [&] (Block* taken) {
      auto const valid = gen(env, ContValid, IsAsyncData(isAsync), genObj);
      gen(env, JmpZero, taken, valid);
    },
    [&] {
      auto const genFp  = gen(env, LdContActRec, IsAsyncData(isAsync), genObj);
      auto const genFunc = gen(env, LdARFunc, genFp);
      return cond(
        env,
        [&] (Block* taken) {
          auto const data = AttrData { AttrHasCoeffectRules };
          auto const success = gen(env, FuncHasAttr, data, genFunc);
          gen(env, JmpNZero, taken, success);
        },
        [&] {
          // Static coeffects
          return gen(env, LdFuncRequiredCoeffects, genFunc);
        },
        [&] {
          hint(env, Block::Hint::Unlikely);
          gen(env, DbgCheckLocalsDecRefd, genFp);
          auto const hasReified = gen(env, HasReifiedGenerics, genFunc);
          auto const numParams = gen(env, LdFuncNumParams, genFunc);
          auto const locId =
            gen(env, AddInt, numParams, gen(env, ConvBoolToInt, hasReified));
          return gen(env, LdLocForeign, TInt, genFp, locId);
        }
      );
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      return cns(env, RuntimeCoeffects::full().value());
    }
  );
}

} // namespace
} // irgen
} // jit

jit::SSATmp* CoeffectRule::emitJit(jit::irgen::IRGS& env,
                                   const Func* f,
                                   uint32_t numArgsInclUnpack,
                                   jit::SSATmp* prologueCtx) const {
  using namespace jit::irgen;
  switch (m_type) {
    case Type::CCParam:
      return emitCCParam(env, f, numArgsInclUnpack, m_index, m_name);
    case Type::CCThis:
      return emitCCThis(env, f, m_name, prologueCtx);
    case Type::FunParam:
      return emitFunParam(env, f, numArgsInclUnpack, m_index);
    case Type::ClosureInheritFromParent:
      return emitClosureInheritFromParent(env, f, prologueCtx);
    case Type::GeneratorThis:
      return emitGeneratorThis(env, f, prologueCtx);
    case Type::Invalid:
      always_assert(false);
  }
  not_reached();
}

//////////////////////////////////////////////////////////////////////

}
