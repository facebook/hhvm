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

#include "hphp/runtime/vm/jit/coeffect-fun-param-profile.h"
#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"
#include "hphp/runtime/vm/jit/irgen-interpone.h"
#include "hphp/runtime/vm/jit/irgen-minstr.h"
#include "hphp/runtime/vm/jit/irgen-state.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"

#include "hphp/runtime/ext/asio/ext_async-generator.h"
#include "hphp/runtime/ext/generator/ext_generator.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

namespace jit {
namespace irgen {
namespace {

SSATmp* resolveTypeConstantChain(IRGS& env, const Func* f, SSATmp* cls,
                                 const std::vector<LowStringPtr>& types) {
  auto result = cls;
  auto const ctx = f->isMethod() ? cns(env, f->implCls()) : cns(env, nullptr);
  for (auto const type : types) {
    auto const name =
      gen(env, LdTypeCnsClsName, result, cns(env, type.get()));
    result = gen(env, LdCls, name, ctx);
  }
  return result;
}

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
    [&] (SSATmp*) { return cns(env, RuntimeCoeffects::none().value()); },
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

SSATmp* emitCCThis(IRGS& env, const Func* f,
                   const std::vector<LowStringPtr>& types,
                   const StringData* name,
                   SSATmp* prologueCtx) {
  assertx(!f->isClosureBody());
  assertx(f->isMethod());
  auto const ctxCls =
    f->isStatic() ? prologueCtx : gen(env, LdObjClass, prologueCtx);
  auto const cls = resolveTypeConstantChain(env, f, ctxCls, types);
  return gen(env, LookupClsCtxCns, cls, cns(env, name));
}

const StaticString s_classname("classname");

SSATmp* emitCCReified(IRGS& env, const Func* f,
                      const std::vector<LowStringPtr>& types,
                      const StringData* name,
                      uint32_t idx,
                      SSATmp* prologueCtx,
                      bool isClass) {
  assertx(!f->isClosureBody());
  auto const generics = [&] {
    if (isClass) {
      assertx(f->isMethod() &&
              !f->isStatic() &&
              prologueCtx &&
              f->cls()->hasReifiedGenerics());
      auto const slot = f->cls()->lookupReifiedInitProp();
      assertx(slot != kInvalidSlot);
      auto const addr = ldPropAddr(
          env, prologueCtx, nullptr, f->cls(), slot, TVec);
      return gen(env, LdMem, TVec, addr);
    }
    assertx(f->hasReifiedGenerics());
    // The existence of the generic is checked by emitCalleeGenericsChecks
    return topC(env);
  }();
  auto const data = BespokeGetData { BespokeGetData::KeyState::Present };
  auto const generic = gen(env, BespokeGet, data, generics, cns(env, idx));
  auto const classname_maybe = gen(
    env,
    BespokeGet,
    BespokeGetData { BespokeGetData::KeyState::Unknown },
    gen(env, AssertType, TDict, generic),
    cns(env, s_classname.get())
  );
  auto const classname = cond(
    env,
    [&] (Block* taken) {
      return gen(env, CheckType, TStr, taken, classname_maybe);
    },
    [&] (SSATmp* s) {
      return s;
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      auto const msg = Strings::INVALID_REIFIED_COEFFECT_CLASSNAME;
      gen(env, RaiseError, cns(env, makeStaticString(msg)));
      return cns(env, staticEmptyString());
    }
  );
  auto const ctx = isClass ? cns(env, f->cls()) : cns(env, nullptr);
  auto const ctxCls = gen(env, LdCls, classname, ctx);
  auto const cls = resolveTypeConstantChain(env, f, ctxCls, types);
  return gen(env, LookupClsCtxCns, cls, cns(env, name));
}

void annotCoeffectFunParamProfile(
    IRGS& env,
    SSATmp* tv,
    const CoeffectFunParamProfile& profile,
    const std::vector<CoeffectFunParamProfile::OptType>& order) {
  if (!RuntimeOption::EvalDumpCoeffectFunParamProf) return;

  auto const fnName = curFunc(env)->fullName()->data();

  auto const orderToName = [](CoeffectFunParamProfile::OptType o) {
    switch (o) {
      case CoeffectFunParamProfile::OptType::Null:
        return "Null";
      case CoeffectFunParamProfile::OptType::Closure:
        return "Closure";
      case CoeffectFunParamProfile::OptType::Func:
        return "Func";
      case CoeffectFunParamProfile::OptType::ClsMeth:
        return "ClsMeth";
      default:
        not_reached();
    }
  };

  std::string orderStr;
  for (int i = 0; i < order.size(); ++i) {
    orderStr += folly::sformat("{}:{},", i+1, orderToName(order[i]));
  }

  env.unit.annotationData->add(
    "CoeffectFunParamProf",
    folly::sformat("BC={} FN={}: {}: {}: {}\n",
      bcOff(env), fnName, *tv, profile, orderStr)
    );
}

SSATmp* emitFunParam(IRGS& env, const Func* f, uint32_t numArgsInclUnpack,
                     uint32_t paramIdx) {
  if (paramIdx >= numArgsInclUnpack) return nullptr;
  auto const index =
    numArgsInclUnpack - 1 - paramIdx + (f->hasReifiedGenerics() ? 1 : 0);
  auto const tv = top(env, BCSPRelOffset {static_cast<int32_t>(index)});

  static const StaticString s_CoeffectFunParam{"CoeffectFunParam"};
  auto const profile = TargetProfile<CoeffectFunParamProfile> {
    env.context,
    env.irb->curMarker(),
    s_CoeffectFunParam.get()
  };

  auto const naive = [&] (bool unlikely = false) {
    if (unlikely) hint(env, Block::Hint::Unlikely);
    auto const data = ParamData { static_cast<int32_t>(paramIdx) };
    return gen(env, LdCoeffectFunParamNaive, data, tv);
  };

  auto const handleFunc = [&](SSATmp* func) {
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
        return cns(env, RuntimeCoeffects::none().value());
      }
    );
  };

  auto const fnFromName = [&](SSATmp* clsName, SSATmp* methodName) {
    auto const cls = gen(env, LdCls, clsName, cns(env, nullptr));
    auto const data = OptClassAndFuncData { nullptr, f };
    return gen(env, LdObjMethodD, data, cls, methodName);
  };

  if (profile.profiling()) {
    gen(env, ProfileCoeffectFunParam, RDSHandleData { profile.handle() }, tv);
    return naive();
  }

  auto const order = [&]() -> std::vector<CoeffectFunParamProfile::OptType> {
    if (!profile.optimizing()) {
      // If we are not profiling or optimizing, we are most likely doing a
      // a live translation. Lets give a decent chance order.
      return {
        CoeffectFunParamProfile::OptType::ClsMeth,
        CoeffectFunParamProfile::OptType::Closure
      };
    }
    auto const data = profile.data();
    auto const order = data.order();
    FTRACE_MOD(Trace::coeffects, 1, "{}\nCoeffectFunParamProfile: {}\n",
               env.irb->curMarker().show(), data.toString());
    annotCoeffectFunParamProfile(env, tv, data, order);
    return order;
  }();

  if (order.empty()) return naive();

  MultiCond mc{env};
  for (auto optType : order) {
    switch (optType) {
      case CoeffectFunParamProfile::OptType::Null:
        mc.ifTypeThen(
          tv,
          TNull,
          [&] (SSATmp*) {
            return cns(env, RuntimeCoeffects::none().value());
          }
        );
        break;
      case CoeffectFunParamProfile::OptType::Closure:
        mc.ifThen(
          [&] (Block* taken) {
            auto const obj = gen(env, CheckType, TObj, taken, tv);
            auto const cls = gen(env, LdObjClass, obj);
            auto const data = AttrData { AttrIsClosureClass };
            auto const success = gen(env, ClassHasAttr, data, cls);
            gen(env, JmpZero, taken, success);
            return cls;
          },
          [&] (SSATmp* cls) {
            return cond(
              env,
              [&] (Block* taken) {
                auto const data = AttrData { AttrHasClosureCoeffectsProp };
                auto const success = gen(env, ClassHasAttr, data, cls);
                gen(env, JmpZero, taken, success);
              },
              [&] {
                // Rules
                auto const obj = gen(env, AssertType, TObj, tv);
                auto const addr = gen(env, LdPropAddr, IndexData { 0 }, TInt, obj);
                return gen(env, LdMem, TInt, addr);
              },
              [&] {
                // Statically known coeffects
                auto const invokeOpt = gen(env, LdObjInvoke, cls);
                auto const invoke = gen(env, AssertNonNull, invokeOpt);
                return gen(env, LdFuncRequiredCoeffects, invoke);
              }
            );
          }
        );
        break;
      case CoeffectFunParamProfile::OptType::MethCaller:
        if (RO::EvalEmitMethCallerFuncPointers) {
          mc.ifThen(
            [&] (Block* taken) {
              auto const func = gen(env, CheckType, TFunc, taken, tv);
              auto const success =
                gen(env, FuncHasAttr, AttrData { AttrIsMethCaller }, func);
              gen(env, JmpZero, taken, success);
              return func;
            },
            [&] (SSATmp* func) {
              return handleFunc(fnFromName(
                gen(env, LdMethCallerName, MethCallerData{true}, func),
                gen(env, LdMethCallerName, MethCallerData{false}, func)
              ));
            }
          );
        } else {
          mc.ifThen(
            [&] (Block* taken) {
              auto const obj = gen(env, CheckType, TObj, taken, tv);
              auto const cls = gen(env, LdObjClass, obj);
              auto const success =
                gen(env, EqCls, cls, cns(env, SystemLib::s_MethCallerHelperClass));
              gen(env, JmpZero, taken, success);
              return cls;
            },
            [&] (SSATmp* cls) {
              auto const obj = gen(env, AssertType, TObj, tv);
              auto const getClsOrMethod = [&](bool is_cls) {
                auto const cls = SystemLib::s_MethCallerHelperClass;
                auto const slot = is_cls ? Slot{0} : Slot{1};
                auto const prop = ldPropAddr(env, obj, nullptr, cls, slot, TStr);
                auto const ret = gen(env, LdMem, TStr, prop);
                gen(env, IncRef, ret);
                return ret;
              };
              return handleFunc(fnFromName(getClsOrMethod(true),
                                           getClsOrMethod(false)));
            }
          );
        }
        break;
      case CoeffectFunParamProfile::OptType::Func:
        mc.ifThen(
          [&] (Block* taken) {
            auto const func = gen(env, CheckType, TFunc, taken, tv);
            if (RO::EvalEmitMethCallerFuncPointers) {
              auto const success =
                gen(env, FuncHasAttr, AttrData { AttrIsMethCaller }, func);
              gen(env, JmpNZero, taken, success);
            }
            return func;
          },
          handleFunc
        );
        break;
      case CoeffectFunParamProfile::OptType::ClsMeth:
        mc.ifTypeThen(
          tv,
          TClsMeth,
          [&] (SSATmp* ptr) {
            auto const func = gen(env, LdFuncFromClsMeth, ptr);
            return handleFunc(func);
          }
        );
        break;
      default:
        not_reached();
    }
  }
  return mc.elseDo([&] { return naive(true); });
}

SSATmp* emitClosureParentScope(IRGS& env, const Func* f, SSATmp* prologueCtx) {
  assertx(prologueCtx);
  assertx(f->isClosureBody());
  auto const cls = f->implCls();
  assertx(cls);
  auto const slot = cls->getCoeffectsProp();
  auto const addr = ldPropAddr(env, prologueCtx, nullptr, cls, slot, TInt);
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
          auto const hasReified = gen(env, FuncHasReifiedGenerics, genFunc);
          auto const numParams = gen(env, LdFuncNumParams, genFunc);
          auto const locId =
            gen(env, AddInt, numParams, gen(env, ConvBoolToInt, hasReified));
          return gen(env, LdLocForeign, TInt, genFp, locId);
        }
      );
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      return cns(env, RuntimeCoeffects::none().value());
    }
  );
}

SSATmp* emitCaller(SSATmp* provided) {
  return provided;
}

} // namespace
} // irgen
} // jit

jit::SSATmp* CoeffectRule::emitJit(jit::irgen::IRGS& env,
                                   const Func* f,
                                   uint32_t numArgsInclUnpack,
                                   jit::SSATmp* prologueCtx,
                                   jit::SSATmp* providedCoeffects) const {
  using namespace jit::irgen;
  switch (m_type) {
    case Type::CCParam:
      return emitCCParam(env, f, numArgsInclUnpack, m_index, m_name);
    case Type::CCThis:
      return emitCCThis(env, f, m_types, m_name, prologueCtx);
    case Type::CCReified:
      return emitCCReified(env, f, m_types, m_name, m_index, prologueCtx,
                           m_isClass);
    case Type::FunParam:
      return emitFunParam(env, f, numArgsInclUnpack, m_index);
    case Type::ClosureParentScope:
      return emitClosureParentScope(env, f, prologueCtx);
    case Type::GeneratorThis:
      return emitGeneratorThis(env, f, prologueCtx);
    case Type::Caller:
      return emitCaller(providedCoeffects);
    case Type::Invalid:
      always_assert(false);
  }
  not_reached();
}

//////////////////////////////////////////////////////////////////////

}
