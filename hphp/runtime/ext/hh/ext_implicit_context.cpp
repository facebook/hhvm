/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include "hphp/runtime/ext/hh/ext_implicit_context.h"

#include <cstdint>
#include <folly/Likely.h>
#include <folly/Random.h>

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/backtrace.h"
#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/implicit-context.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/ext/core/ext_core_closure.h"
#include "hphp/runtime/ext/hh/ext_hh.h"
#include "hphp/runtime/vm/coeffects.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/vm-regs.h"
#include "hphp/runtime/base/init-fini-node.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

namespace {

const StaticString
  s_ImplicitContextDataClassName("HH\\ImplicitContext\\_Private\\ImplicitContextData"),
  s_ICInaccessibleMemoKey("%Inaccessible%"),
  s_ICSoftInaccessibleMemoKey("%SoftInaccessible%"),
  s_ICSoftSetMemoKey("%SoftSet%"),
  // matches HH\ImplicitContext\State
  s_ICStateNull("NULL"),
  s_ICStateValue("VALUE"),
  s_ICStateSoftSet("SOFT_SET"),
  s_ICStateInaccessible("INACCESSIBLE"),
  s_ICStateSoftInaccessible("SOFT_INACCESSIBLE");

RDS_LOCAL(int64_t, rl_nextMemoKey);
// key 0 reserved for all agnostic and empty ICs
// so that they always map to the same int
static constexpr int64_t kAgnosticMemoKey = 0;

using ICMemoKeyToIntMap = req::hash_map<StringData*, int64_t, string_data_hash, string_data_same>;

RDS_LOCAL(ICMemoKeyToIntMap, rl_memoKeyToInt);

InitFiniNode s_clear_IC_map([]{
  rl_memoKeyToInt.destroy();
}, InitFiniNode::When::RequestFini, "s_clear_IC_map");

InitFiniNode s_init_IC_map([]{
  rl_memoKeyToInt.create();
  *(rl_nextMemoKey) = kAgnosticMemoKey;
}, InitFiniNode::When::RequestStart, "s_init_IC_map");


struct ImplicitContextLoader :
  SystemLib::ClassLoader<"HH\\ImplicitContext\\_Private\\ImplicitContextData"> {};

Object create_new_IC() {
  auto obj = Object{ ImplicitContextLoader::classof() };
  return obj;
}
} // namespace

bool HHVM_FUNCTION(has_key, StringArg keyArg) {
  assertx(*ImplicitContext::activeCtx);
  auto key = keyArg.get();
  if (key->size() == 0) { return false; }
  auto const obj = *ImplicitContext::activeCtx;
  auto const context = Native::data<ImplicitContext>(obj);
  return context->m_map.find(key) != context->m_map.end();
}

bool HHVM_FUNCTION(is_inaccessible) {
  assertx(*ImplicitContext::activeCtx);
  return (*ImplicitContext::activeCtx) == (*ImplicitContext::emptyCtx);
}

Object initEmptyContext() {
  auto const obj = Object{ ImplicitContextLoader::classof() };
  auto context = Native::data<ImplicitContext>(obj.get());
  /*
   * Special case: emptyCtx uses kAgnosticMemoKey
   * because if context is attemped to be accessed 
   * before any set occurs, the behavior should be inaccessible
   */
  context->m_memoKey = kAgnosticMemoKey;
  return obj;
};

String HHVM_FUNCTION(get_state_unsafe) {
  assertx(*ImplicitContext::activeCtx);
  if ((*ImplicitContext::activeCtx) == (*ImplicitContext::emptyCtx)) {
    return String{s_ICStateInaccessible.get()};
  }
  return String{s_ICStateValue.get()};
}

TypedValue HHVM_FUNCTION(get_implicit_context, StringArg key) {
  assertx(*ImplicitContext::activeCtx);
  auto const obj = *ImplicitContext::activeCtx;
  auto const context = Native::data<ImplicitContext>(obj);

  auto const it = context->m_map.find(key.get());
  if (it == context->m_map.end()) {
    throw_implicit_context_exception("Implicit context is set to inaccessible");
  }
  auto const result = it->second.first;
  if (isRefcountedType(result.m_type)) tvIncRefCountable(result);
  return result;
}

ObjectRet HHVM_FUNCTION(get_whole_implicit_context) {
  assertx(*ImplicitContext::activeCtx);
  auto const obj = *ImplicitContext::activeCtx;
  return Object{obj};
}

int64_t HHVM_FUNCTION(get_implicit_context_memo_key) {
  assertx(*ImplicitContext::activeCtx);
  auto const obj = *ImplicitContext::activeCtx;
  auto const context = Native::data<ImplicitContext>(obj);
  return context->m_memoKey;
}

/*
* Returns a human readable memo key for the current IC
* Only to be used for testing
*/
Array HHVM_FUNCTION(get_implicit_context_debug_info) {
  assertx(*ImplicitContext::activeCtx);
  auto const obj = *ImplicitContext::activeCtx;
  auto const context = Native::data<ImplicitContext>(obj);

  if (obj == (*ImplicitContext::emptyCtx)) {
    VecInit ret{1};
    ret.append(s_ICInaccessibleMemoKey.data());
    return ret.toArray();
  }
  VecInit ret{context->m_map.size() * 2}; // key and value
  for (auto const& p : context->m_map) {
    auto const key = String(p.first->data());
    auto const value = HHVM_FN(serialize_memoize_param)(p.second.first);
    ret.append(key);
    ret.append(value);
  }
  return ret.toArray();
}


Object HHVM_FUNCTION(create_implicit_context, StringArg keyarg,
                                              TypedValue data) {
  auto const key = keyarg.get();
  /*
   * Reserve the underscore prefix for the time being in case we want to
   * emit keys from the compiler. This would allow us to avoid having
   * conflicts with other key generation mechanisms.
  */
  if (key->size() == 0 || key->data()[0] == '_') {
    throw_implicit_context_exception(
      "Implicit context keys cannot be empty or start with _");
  }
  assertx(*ImplicitContext::activeCtx);
  auto const prev = *ImplicitContext::activeCtx;

  // Compute memory key first
  using Elem = std::pair<const StringData*, TypedValue>;
  req::vector<Elem> vec;
  auto memokey = Variant::attach(HHVM_FN(serialize_memoize_param)(data));

  auto& existing_m_map = Native::data<ImplicitContext>(prev)->m_map;
  for (auto const& p : existing_m_map) {
    vec.push_back(std::make_pair(p.first, p.second.second));
  }
  vec.push_back(std::make_pair(key, *memokey.asTypedValue()));
  std::sort(vec.begin(), vec.end(), [](const Elem& e1, const Elem& e2) {
                                return e1.first->compare(e2.first) < 0;
                              });

  StringBuffer sb;
  for (auto const& e : vec) {
    serialize_memoize_string_data(sb, e.first);
    serialize_memoize_tv(sb, 0, e.second);
  }
  auto target_memo_key = sb.detach();
  auto& m_to_int = *rl_memoKeyToInt;

  auto obj = create_new_IC();
  auto const context = Native::data<ImplicitContext>(obj.get());

  auto const it = m_to_int.find(target_memo_key.get());
  if (it == m_to_int.end()) {
    auto const memoKey = ++(*rl_nextMemoKey);
    UNUSED auto ret = m_to_int.emplace(std::make_pair(target_memo_key.detach(),
                                                      memoKey));
    assertx(ret.second); // the emplace should always succeed
    context->m_memoKey = memoKey;
  } else {
    context->m_memoKey = it->second;
  }

  context->m_map = Native::data<ImplicitContext>(prev)->m_map;
  // IncRef data and key as we are storing in the map
  if (isRefcountedType(data.m_type)) tvIncRefCountable(data);
  key->incRefCount();
  context->m_map.insert_or_assign(key, std::make_pair(data, memokey.detach()));
  obj.get()->incRefCount();
  return obj;
}

namespace {

Variant coeffects_call_helper(const Variant& function, const char* name,
                              RuntimeCoeffects coeffects,
                              bool getCoeffectsFromClosure) {
  CallCtx ctx;
  vm_decode_function(function, ctx);
  if (!ctx.func) {
    raise_error("%s expects first argument to be a closure or a "
                "function pointer",
                name);
  }
  if (getCoeffectsFromClosure &&
      ctx.func->hasCoeffectRules() &&
      ctx.func->getCoeffectRules().size() == 1 &&
      ctx.func->getCoeffectRules()[0].isClosureParentScope()) {
    assertx(ctx.func->isClosureBody());
    auto const closure = reinterpret_cast<c_Closure*>(ctx.this_);
    coeffects = closure->getCoeffects();
  }
  return Variant::attach(
    g_context->invokeFunc(ctx.func, init_null_variant, ctx.this_, ctx.cls,
                          coeffects, ctx.dynamic)
  );
}

} // namespace

Variant HHVM_FUNCTION(enter_zoned_with, const Variant& function) {
  return coeffects_call_helper(function,
                               "HH\\Coeffects\\_Private\\enter_zoned_with",
                               RuntimeCoeffects::zoned_with(), false);
}

static struct HHImplicitContext final : Extension {
  HHImplicitContext(): Extension("implicit_context", NO_EXTENSION_VERSION_YET, NO_ONCALL_YET) { }
  void moduleRegisterNative() override {
    Native::registerNativeDataInfo<ImplicitContext>(
      ImplicitContextLoader::className().get());

    HHVM_NAMED_FE(HH\\ImplicitContext\\get_state_unsafe,
                  HHVM_FN(get_state_unsafe));
    HHVM_NAMED_FE(HH\\ImplicitContext\\_Private\\get_implicit_context,
                  HHVM_FN(get_implicit_context));
    HHVM_NAMED_FE(HH\\ImplicitContext\\_Private\\get_whole_implicit_context,
                  HHVM_FN(get_whole_implicit_context));
    HHVM_NAMED_FE(HH\\ImplicitContext\\_Private\\create_implicit_context,
                  HHVM_FN(create_implicit_context));
    HHVM_NAMED_FE(HH\\ImplicitContext\\_Private\\get_implicit_context_memo_key,
                  HHVM_FN(get_implicit_context_memo_key));
    HHVM_NAMED_FE(HH\\ImplicitContext\\is_inaccessible,
                  HHVM_FN(is_inaccessible));
    HHVM_NAMED_FE(HH\\ImplicitContext\\_Private\\has_key,
                  HHVM_FN(has_key));
    HHVM_NAMED_FE(HH\\ImplicitContext\\_Private\\get_implicit_context_debug_info,
                  HHVM_FN(get_implicit_context_debug_info));
    HHVM_NAMED_FE(HH\\Coeffects\\_Private\\enter_zoned_with,
                  HHVM_FN(enter_zoned_with));

  }
} s_hh_implicit_context;

///////////////////////////////////////////////////////////////////////////////
}
