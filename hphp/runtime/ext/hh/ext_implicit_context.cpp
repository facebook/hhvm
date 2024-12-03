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
  s_ICAgnostic("%MemoAgnostic%"),
  // matches HH\ImplicitContext\State
  s_ICStateValue("VALUE"),
  s_ICStateInaccessible("INACCESSIBLE");

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

// converts key + memokey into a memokey int by leveraging side map
int64_t memoKeyForInsert(StringData* key, const Variant& serializedValue) {
  StringBuffer sb;
  auto const prev = *ImplicitContext::activeCtx;
  auto prev_ctx = Native::data<ImplicitContext>(prev);
  using Elem = std::pair<const StringData*, TypedValue>;
  req::vector<Elem> vec;

  auto& existing_m_map = prev_ctx->m_map;
  for (auto const& p : existing_m_map) {
    if (p.second.second.m_type != KindOfUninit && !p.first->same(key)) {
      vec.push_back(std::make_pair(p.first, p.second.second));
    }
  }
  vec.push_back(std::make_pair(key, *serializedValue.asTypedValue()));
  std::sort(vec.begin(), vec.end(), [](const Elem& e1, const Elem& e2) {
                                return e1.first->compare(e2.first) < 0;
                              });

  for (auto const& e : vec) {
    serialize_memoize_string_data(sb, e.first);
    serialize_memoize_tv(sb, 0, e.second);
  }
  auto target_memo_key = sb.detach();
  auto& m_to_int = *rl_memoKeyToInt;

  auto const it = m_to_int.find(target_memo_key.get());
  if (it != m_to_int.end()) {
    return it->second;
  }
  auto const memoKey = ++(*rl_nextMemoKey);
  UNUSED auto ret = m_to_int.emplace(std::make_pair(target_memo_key.detach(),
                                                    memoKey));
  assertx(ret.second); // the emplace should always succeed
  return memoKey;
}

Object createICWithParams(int64_t memoKey, auto&& map, ObjectData* agnosticIC) {
  auto ic_obj = Object{ ImplicitContextLoader::classof() };
  auto ic = Native::data<ImplicitContext>(ic_obj.get());
  ic->m_memoKey = memoKey;
  ic->m_map = std::move(map);
  ic->m_memoAgnosticIC = agnosticIC ? agnosticIC : ic_obj.get();
  // NB: this leaks memory, to be removed once the refcounting is fixed and tested
  ic_obj.get()->incRefCount();
  return ic_obj;
}

/*
 * Creates a new Memo Agnostic IC
 * Inherits values from prev_agnostic_obj
*/
Object create_memo_agnostic_IC(ObjectData* prev_agnostic_obj,
                               TypedValue data, 
                               StringData* key) {
  auto prev_agnostic_ctx = Native::data<ImplicitContext>(prev_agnostic_obj);
  assertx(prev_agnostic_ctx->m_memoKey == kAgnosticMemoKey);
  auto updated_map = prev_agnostic_ctx->m_map;
  tvIncRefGen(data);
  updated_map.insert_or_assign(String{key}.detach(), 
                               std::make_pair(data, make_tv<KindOfUninit>()));
  return createICWithParams(kAgnosticMemoKey, std::move(updated_map), nullptr /* self */);
}

/*
 * create_implicit_context_impl - creates a pair of linked ICs
 * params
 * TypedValue data => ctx baggage for agnostic, ctx data for sensitive
 * StringData* key => key for memosensitive IC, null for agnostic
 * Variant memokey => memokey for memosensitive IC, empty for agnostic
 * int64_t memo_key_int => 0 for memo agnostic, >0 for memo sensitive
*/
Object create_implicit_context_impl(TypedValue data, Variant serializedValue,
                                    StringData* key, int64_t memo_key_int) {
  assertx(data.m_type != KindOfUninit);
  assertx(*ImplicitContext::activeCtx);
  auto const prev = *ImplicitContext::activeCtx;
  auto prev_ctx = Native::data<ImplicitContext>(prev);

  /*
   * Memo sensitive IC creation necessarily returns the 
   * default IC pointer, while memo agnostic IC creation can 
   * return default OR agnostic IC, depending the existing state.
   * Prev IC's memokey being kAgnosticMemoKey implies Agnostic state
   *  +----------------+----------------+----------------+
   *  |  Prev State    |  Requested IC  |  Returns       |
   *  +----------------+----------------+----------------+
   *  |  emptyCtx      |  MemoSensitive |  MemoSensitive |
   *  +----------------+----------------+----------------+
   *  |  emptyCtx      |  MemoAgnostic  |  MemoAgnostic  |
   *  +----------------+----------------+----------------+
   *  |  MemoSensitive |  MemoSensitive |  MemoSensitive |
   *  +----------------+----------------+----------------+
   *  |  MemoSensitive |  MemoAgnostic  |  MemoSensitive |
   *  +----------------+----------------+----------------+
   *  |  MemoAgnostic  |  MemoSensitive |  MemoSensitive |
   *  +----------------+----------------+----------------+
   *  |  MemoAgnostic  |  MemoAgnostic  |  MemoAgnostic  |
   *  +----------------+----------------+----------------+
  */

  bool memo_agnostic_ic_requested = !serializedValue.isInitialized();
  bool is_prev_agnostic = prev_ctx->m_memoKey == kAgnosticMemoKey;
  
  if (is_prev_agnostic && memo_agnostic_ic_requested) {
    return create_memo_agnostic_IC(prev, data, key);
  }


  // new_ic is the default IC, create the memo agnostic branch ptr for it
  auto memo_agnostic_obj = memo_agnostic_ic_requested
                           ? create_memo_agnostic_IC(prev_ctx->m_memoAgnosticIC, data, key).get()
                           : prev_ctx->m_memoAgnosticIC;
  auto updated_map = prev_ctx->m_map;
  tvIncRefGen(data);
  updated_map.insert_or_assign(String{key}.detach(), 
                                   std::make_pair(data, serializedValue.detach()));
  auto new_ic_memokey = memo_agnostic_ic_requested ? prev_ctx->m_memoKey : memo_key_int;
  return createICWithParams(new_ic_memokey, std::move(updated_map), memo_agnostic_obj);
}

} // namespace

Object initEmptyContext() {
  auto const obj = Object{ ImplicitContextLoader::classof() };
  auto context = Native::data<ImplicitContext>(obj.get());
  /*
   * Special case: emptyCtx uses kAgnosticMemoKey
   * because if context is attemped to be accessed 
   * before any set occurs, the behavior should be inaccessible
   */
  context->m_memoKey = kAgnosticMemoKey;
  context->m_memoAgnosticIC = obj.get();
  return obj;
};

bool HHVM_FUNCTION(has_key, StringArg keyArg) {
  assertx(*ImplicitContext::activeCtx);
  auto key = keyArg.get();
  if (key->size() == 0) { return false; }
  auto const obj = *ImplicitContext::activeCtx;
  auto const context = Native::data<ImplicitContext>(obj);
  return context->m_map.find(key) != context->m_map.end();
}

String HHVM_FUNCTION(get_state_unsafe) {
  assertx(*ImplicitContext::activeCtx);
  auto const obj = *ImplicitContext::activeCtx;
  auto const context = Native::data<ImplicitContext>(obj);

  if (context->m_memoKey == kAgnosticMemoKey) {
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
  tvIncRefGen(result);
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

  VecInit ret{context->m_map.size() * 2}; // key and value
  for (auto const& p : context->m_map) {
    if (p.second.second.m_type == KindOfUninit) {
      ret.append(s_ICAgnostic);
    }
    auto const key = String(p.first->data());
    auto const value = p.second.second;
    ret.append(key);
    ret.append(value);
  }
  return ret.toArray();
}

Object HHVM_FUNCTION(create_implicit_context, StringArg keyarg,
                                              TypedValue data,
                                              bool memoSensitive) {
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
  
  if (!memoSensitive) return create_implicit_context_impl(data, Variant{}, key, kAgnosticMemoKey);

  auto serializedValue = Variant::attach(HHVM_FN(serialize_memoize_param)(data));
  auto memo_key_int = memoKeyForInsert(key, serializedValue);
  return create_implicit_context_impl(data, serializedValue, key, memo_key_int);
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
    HHVM_NAMED_FE(HH\\ImplicitContext\\_Private\\has_key,
                  HHVM_FN(has_key));
    HHVM_NAMED_FE(HH\\ImplicitContext\\_Private\\get_implicit_context_debug_info,
                  HHVM_FN(get_implicit_context_debug_info));

  }
} s_hh_implicit_context;

///////////////////////////////////////////////////////////////////////////////
}
