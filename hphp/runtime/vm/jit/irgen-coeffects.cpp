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

#include "hphp/runtime/vm/jit/irgen-internal.h"
#include "hphp/runtime/vm/jit/irgen-interpone.h"
#include "hphp/runtime/vm/jit/irgen-state.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"

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
  auto const tv = topC(env, BCSPRelOffset {static_cast<int32_t>(index)});
  return cond(
    env,
    [&] (Block* taken) {
      auto const isObj = gen(env, IsType, TObj, tv);
      gen(env, JmpZero, taken, isObj);
    },
    [&] {
      auto const obj = gen(env, AssertType, TObj, tv);
      auto const cls  = gen(env, LdObjClass, obj);
      return gen(env, LookupClsCtxCns, cls, cns(env, name));
    },
    [&] {
      return cond(
        env,
        [&] (Block* taken) {
          auto const isObj = gen(env, IsType, TNull, tv);
          gen(env, JmpZero, taken, isObj);
        },
        [&] {
          return cns(env, RuntimeCoeffects::full().value());
        },
        [&] {
          hint(env, Block::Hint::Unlikely);
          auto const msg =
            folly::sformat("Coeffect rule requires parameter at position "
                           "{} to be an object or null",
                           paramIdx);
          gen(env, RaiseError, cns(env, makeStaticString(msg)));
          return cns(env, 0);
        }
      );
    }
  );
}

SSATmp* emitCCThis() {
  // TODO(oulgen): Implement this
  return nullptr;
}

SSATmp* emitFunParam() {
  // TODO(oulgen): Implement this
  return nullptr;
}

} // namespace
} // irgen
} // jit

jit::SSATmp* CoeffectRule::emitJit(jit::irgen::IRGS& env,
                                   const Func* f,
                                   uint32_t numArgsInclUnpack) const {
  using namespace jit::irgen;
  switch (m_type) {
    case Type::CCParam:  return emitCCParam(env, f, numArgsInclUnpack,
                                            m_index, m_name);
    case Type::CCThis:   return emitCCThis();
    case Type::FunParam: return emitFunParam();
    case Type::Invalid:
      always_assert(false);
  }
  not_reached();
}

//////////////////////////////////////////////////////////////////////

}
