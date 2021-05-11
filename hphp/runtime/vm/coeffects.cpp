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

#include "hphp/runtime/base/coeffects-config.h"
#include "hphp/runtime/vm/blob-helper.h"
#include "hphp/runtime/vm/runtime.h"

#include "hphp/runtime/vm/jit/translator-runtime.h"

#include "hphp/runtime/ext/std/ext_std_closure.h"

#include "hphp/util/trace.h"

TRACE_SET_MOD(coeffects);

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

StaticCoeffects StaticCoeffects::defaults() {
  static StaticCoeffects defaults = CoeffectsConfig::fromName("defaults");
  return defaults;
}

RuntimeCoeffects RuntimeCoeffects::defaults() {
  return StaticCoeffects::defaults().toAmbient();
}

RuntimeCoeffects RuntimeCoeffects::pure() {
  static RuntimeCoeffects pure = CoeffectsConfig::fromName("pure").toAmbient();
  return pure;
}

RuntimeCoeffects RuntimeCoeffects::policied_of() {
  static RuntimeCoeffects c =
    CoeffectsConfig::fromName("policied_of").toAmbient();
  return c;
}

const std::string RuntimeCoeffects::toString() const {
  // Pretend to be StaticCoeffects, this is safe since RuntimeCoeffects is a
  // subset of StaticCoeffects
  auto const data = StaticCoeffects::fromValue(m_data);
  auto const list = CoeffectsConfig::toStringList(data);
  if (list.empty()) return "defaults";
  if (list.size() == 1 && list[0] == "pure") return "";
  return folly::join(", ", list);
}

bool RuntimeCoeffects::canCallWithWarning(const RuntimeCoeffects o) const {
  auto const promoted =
    RuntimeCoeffects::fromValue(o.m_data | CoeffectsConfig::warningMask());
  return canCall(promoted);
}

const folly::Optional<std::string> StaticCoeffects::toString() const {
  auto const list = CoeffectsConfig::toStringList(*this);
  if (list.empty()) return folly::none;
  return folly::join(" ", list);
}

RuntimeCoeffects StaticCoeffects::toAmbient() const {
  auto const val = m_data - locals();
  return RuntimeCoeffects::fromValue(val);
}

RuntimeCoeffects StaticCoeffects::toRequired() const {
  // This converts the 01 (local) pattern to 10 (shallow) pattern
  // (m_data | (locals << 1)) & (~locals)
  // => m_data - locals + 2 * locals
  // => m_data + locals
  auto const val = m_data + locals();
  return RuntimeCoeffects::fromValue(val);
}

RuntimeCoeffects StaticCoeffects::toShallowWithLocals() const {
  return RuntimeCoeffects::fromValue(locals() << 1);
}

RuntimeCoeffects& RuntimeCoeffects::operator&=(const RuntimeCoeffects o) {
  m_data &= o.m_data;
  return *this;
}

StaticCoeffects& StaticCoeffects::operator|=(const StaticCoeffects o) {
  return (*this = CoeffectsConfig::combine(*this, o));
}

StaticCoeffects::storage_t StaticCoeffects::locals() const {
  return (((~m_data) >> 1) & m_data) & CoeffectsConfig::escapeMask();
}

folly::Optional<std::string> CoeffectRule::toString(const Func* f) const {
  switch (m_type) {
    case Type::FunParam:
      return folly::to<std::string>("ctx $",
                                    f->localVarName(m_index)->toCppString());
    case Type::CCParam:
      return folly::to<std::string>("$",
                                    f->localVarName(m_index)->toCppString(),
                                    "::",
                                    m_name->toCppString());
    case Type::CCThis: {
      std::vector<std::string> types;
      for (auto type : m_types) {
        types.push_back(folly::to<std::string>("::", type->toCppString()));
      }
      return folly::sformat("this{}::{}", folly::join("", types), m_name);
    }
    case Type::ClosureParentScope:
    case Type::GeneratorThis:
    case Type::Caller:
      return folly::none;
    case Type::Invalid:
      always_assert(false);
  }
  not_reached();
}

namespace {

const Class* resolveTypeConstantChain(const Class* cls,
                                      const std::vector<LowStringPtr>& types) {
  auto result = cls;
  for (auto const type : types) {
    auto const name = jit::loadClsTypeCnsClsNameHelper(result, type);
    result = Class::load(name);
    if (!result) raise_error(Strings::UNKNOWN_CLASS, name->data());
  }
  return result;
}

RuntimeCoeffects emitCCParam(const Func* f,
                             uint32_t numArgsInclUnpack,
                             uint32_t paramIdx,
                             const StringData* name) {
  if (paramIdx >= numArgsInclUnpack) return RuntimeCoeffects::full();
  auto const index =
    numArgsInclUnpack - 1 - paramIdx + (f->hasReifiedGenerics() ? 1 : 0);
  auto const tv = vmStack().indC(index);
  if (tvIsNull(tv)) return RuntimeCoeffects::full();
  if (!tvIsObject(tv)) {
    raise_error(folly::sformat("Coeffect rule requires parameter at "
                               "position {} to be an object or null",
                               paramIdx + 1));
  }
  auto const cls = tv->m_data.pobj->getVMClass();
  return *cls->clsCtxCnsGet(name, true);
}

RuntimeCoeffects emitCCThis(const Func* f,
                            const std::vector<LowStringPtr>& types,
                            const StringData* name,
                            void* prologueCtx) {
  assertx(!f->isClosureBody());
  assertx(f->isMethod());
  auto const ctxCls = f->isStatic()
    ? reinterpret_cast<Class*>(prologueCtx)
    : reinterpret_cast<ObjectData*>(prologueCtx)->getVMClass();
  auto const cls = resolveTypeConstantChain(ctxCls, types);
  return *cls->clsCtxCnsGet(name, true);
}

RuntimeCoeffects emitFunParam(const Func* f, uint32_t numArgsInclUnpack,
                              uint32_t paramIdx) {
  if (paramIdx >= numArgsInclUnpack) return RuntimeCoeffects::full();
  auto const index =
    numArgsInclUnpack - 1 - paramIdx + (f->hasReifiedGenerics() ? 1 : 0);
  auto const tv = vmStack().indC(index);
  auto const handleFunc = [&](const Func* func) {
    if (func->hasCoeffectRules()) {
      raiseCoeffectsFunParamCoeffectRulesViolation(func);
      return RuntimeCoeffects::full();
    }
    return func->requiredCoeffects();
  };
  auto const error = [&]{
    raiseCoeffectsFunParamTypeViolation(*tv, paramIdx);
    return RuntimeCoeffects::full();
  };
  if (tvIsNull(tv))     return RuntimeCoeffects::full();
  if (tvIsFunc(tv))     return handleFunc(tv->m_data.pfunc);
  if (tvIsRFunc(tv))    return handleFunc(tv->m_data.prfunc->m_func);
  if (tvIsClsMeth(tv))  return handleFunc(tv->m_data.pclsmeth->getFunc());
  if (tvIsRClsMeth(tv)) return handleFunc(tv->m_data.prclsmeth->m_func);
  if (tvIsObject(tv)) {
    auto const obj = tv->m_data.pobj;
    auto const closureCls = obj->getVMClass();
    if (!closureCls->isClosureClass()) return error();
    if (!closureCls->hasClosureCoeffectsProp()) {
      assertx(!closureCls->getCachedInvoke()->hasCoeffectRules());
      return closureCls->getCachedInvoke()->requiredCoeffects();
    }
    return c_Closure::fromObject(obj)->getCoeffects();
  }
  return error();
}

RuntimeCoeffects emitClosureParentScope(const Func* f,
                                        void* prologueCtx) {
  assertx(prologueCtx);
  assertx(f->isClosureBody());
  auto const closure = reinterpret_cast<c_Closure*>(prologueCtx);
  return closure->getCoeffects();
}

RuntimeCoeffects emitGeneratorThis(const Func* f, void* prologueCtx) {
  assertx(prologueCtx);
  assertx(f->isMethod() && !f->isStatic() && f->implCls() &&
          (f->implCls() == AsyncGenerator::getClass() ||
           f->implCls() == Generator::getClass()));
  auto const obj = reinterpret_cast<ObjectData*>(prologueCtx);
  auto const gen = f->implCls() == AsyncGenerator::getClass()
    ? static_cast<BaseGenerator*>(AsyncGenerator::fromObject(obj))
    : static_cast<BaseGenerator*>(Generator::fromObject(obj));
  if (gen->getState() == BaseGenerator::State::Done) {
    // We need to make sure coeffects check passes
    return RuntimeCoeffects::full();
  }
  return gen->actRec()->requiredCoeffects();
}

RuntimeCoeffects emitCaller(RuntimeCoeffects provided) {
  return provided;
}

} // namespace

RuntimeCoeffects CoeffectRule::emit(const Func* f,
                                    uint32_t numArgsInclUnpack,
                                    void* prologueCtx,
                                    RuntimeCoeffects providedCoeffects) const {
  switch (m_type) {
    case Type::CCParam:
      return emitCCParam(f, numArgsInclUnpack, m_index, m_name);
    case Type::CCThis:
      return emitCCThis(f, m_types, m_name, prologueCtx);
    case Type::FunParam:
      return emitFunParam(f, numArgsInclUnpack, m_index);
    case Type::ClosureParentScope:
      return emitClosureParentScope(f, prologueCtx);
    case Type::GeneratorThis:
      return emitGeneratorThis(f, prologueCtx);
    case Type::Caller:
      return emitCaller(providedCoeffects);
    case Type::Invalid:
      always_assert(false);
  }
  not_reached();
}

bool CoeffectRule::isClosureParentScope() const {
  return m_type == Type::ClosureParentScope;
}

bool CoeffectRule::isGeneratorThis() const {
  return m_type == Type::GeneratorThis;
}

std::string CoeffectRule::getDirectiveString() const {
  switch (m_type) {
    case Type::FunParam:
      return folly::sformat(".coeffects_fun_param {};", m_index);
    case Type::CCParam:
      return folly::sformat(".coeffects_cc_param {} {};", m_index,
                            folly::cEscape<std::string>(
                              m_name->toCppString()));
    case Type::CCThis: {
      std::vector<std::string> names;
      for (auto type : m_types) {
        names.push_back(folly::cEscape<std::string>(type->toCppString()));
      }
      names.push_back(folly::cEscape<std::string>(m_name->toCppString()));
      return folly::sformat(".coeffects_cc_this {};",
                            folly::join(" ", names));
    }
    case Type::ClosureParentScope:
      return ".coeffects_closure_parent_scope;";
    case Type::GeneratorThis:
      return ".coeffects_generator_this;";
    case Type::Caller:
      return ".coeffects_caller;";
    case Type::Invalid:
      always_assert(false);
  }
  not_reached();
}

template<class SerDe>
void CoeffectRule::serde(SerDe& sd) {
  sd(m_type)
    (m_index)
    (m_types)
    (m_name)
  ;
}

template void CoeffectRule::serde<>(BlobDecoder&);
template void CoeffectRule::serde<>(BlobEncoder&);

///////////////////////////////////////////////////////////////////////////////
}
