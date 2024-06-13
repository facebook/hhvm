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

using ImplicitContextMap = req::hash_map<StringData*, ObjectData*, string_data_hash, string_data_same>;

/*
 * Caution: This map holds the addresses of memokeys to ICs
 * We are storing the IC ObjectData* and Memokey StringData*,
 * Hence they are kept alive to the end of the request
 * If we start to properly refcount and free the IC objects,
 * we should refactor this to accommodate for that
 * (e.g. use a monotonically increasing int instead of the address of IC obj)
*/
RDS_LOCAL(ImplicitContextMap, s_memokey_to_IC);

InitFiniNode s_clear_IC_map([]{
  s_memokey_to_IC.destroy();
}, InitFiniNode::When::RequestFini, "s_clear_IC_map");

InitFiniNode s_init_IC_map([]{
  s_memokey_to_IC.create();
}, InitFiniNode::When::RequestStart, "s_init_IC_map");


struct ImplicitContextLoader :
  SystemLib::ClassLoader<"HH\\ImplicitContext\\_Private\\ImplicitContextData"> {};

Object create_new_IC() {
  auto obj = Object{ ImplicitContextLoader::classof() };
  return obj;
}
} // namespace

bool HHVM_FUNCTION(is_inaccessible) {
  assertx(*ImplicitContext::activeCtx);
  return (*ImplicitContext::activeCtx) == (*ImplicitContext::emptyCtx);
}

Object initEmptyContext() {
  auto const obj = Object{ ImplicitContextLoader::classof() };
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

/*
 * Caution! This function relies on the assumption that
 * every IC is kept alive throughout the request lifetime
 * the IC should stay alive due to being stored in s_memokey_to_IC
*/
int64_t HHVM_FUNCTION(get_implicit_context_memo_key) {
  assertx(*ImplicitContext::activeCtx);
  auto const obj = *ImplicitContext::activeCtx;
  return reinterpret_cast<int64_t>(obj);
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

  if (prev) {
    auto& existing_m_map = Native::data<ImplicitContext>(prev)->m_map;
    for (auto const& p : existing_m_map) {
      vec.push_back(std::make_pair(p.first, p.second.second));
    }
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
  auto& m_to_ic = *s_memokey_to_IC;
  if (m_to_ic.find(target_memo_key.get()) != m_to_ic.end()) {
    return Object{m_to_ic.at(target_memo_key.get())};
  }

  // create a new IC since we did not find an existing one that matches the description
  auto obj = create_new_IC();
  auto const context = Native::data<ImplicitContext>(obj.get());

  // IncRef data and key as we are storing in the map
  if (isRefcountedType(data.m_type)) tvIncRefCountable(data);
  key->incRefCount();
  if (prev) context->m_map = Native::data<ImplicitContext>(prev)->m_map;
  context->m_map.insert_or_assign(key, std::make_pair(data, memokey.detach()));

  // Store new IC into the map
  auto UNUSED ret = m_to_ic.emplace(std::make_pair(target_memo_key.detach(), Object(obj).detach()));
  assertx(ret.second); // the emplace should always succeed
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
    HHVM_NAMED_FE(HH\\ImplicitContext\\_Private\\get_implicit_context_debug_info,
                  HHVM_FN(get_implicit_context_debug_info));
    HHVM_NAMED_FE(HH\\Coeffects\\_Private\\enter_zoned_with,
                  HHVM_FN(enter_zoned_with));

  }
} s_hh_implicit_context;

///////////////////////////////////////////////////////////////////////////////
}
