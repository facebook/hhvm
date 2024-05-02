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
#include "hphp/util/hash-map.h"

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

String get_target_memo_key(const StringData* memo_key,
                                const Func* func,
                                const HPHP::ImplicitContext::State state) {

  assertx(state != ImplicitContext::State::Value); // Value state IC cration should never call this

  if (state == ImplicitContext::State::Inaccessible) {
    return s_ICInaccessibleMemoKey;
  }

  assertx(func);
  assertx(state == ImplicitContext::State::SoftInaccessible ||
          state == ImplicitContext::State::SoftSet);

  auto const cur_blame = [&] {
    if (memo_key) return memo_key;
    return func->fullName();
  }();

  StringBuffer sb;
  sb.append(state == ImplicitContext::State::SoftSet
              ? s_ICSoftSetMemoKey.get() : s_ICSoftInaccessibleMemoKey.get());
  if (auto const obj = *ImplicitContext::activeCtx) {
    auto const prev = Native::data<ImplicitContext>(obj);
    for (auto const& s : prev->m_blameFromSoftInaccessible) {
      serialize_memoize_string_data(sb, s);
    }
    if (state == ImplicitContext::State::SoftInaccessible) {
      serialize_memoize_string_data(sb, cur_blame);
    }
    sb.append('%'); // separator
    for (auto const& s : prev->m_blameFromSoftSet) {
      serialize_memoize_string_data(sb, s);
    }
    if (state == ImplicitContext::State::SoftSet) {
      serialize_memoize_string_data(sb, cur_blame);
    }
  } else {
    serialize_memoize_string_data(sb, cur_blame);
  }
  return sb.detach();
}

void set_implicit_context_blame(ImplicitContext* context,
                                const StringData* memo_key,
                                const Func* func) {
  auto const state = context->m_state;

  // the memo_key is only provided for one of two situations:
  // (1) always when entering the SoftSet state
  // (2) when entering the SoftInaccessible state via
  //     the function run_with_soft_inaccessible_state
  assertx(IMPLIES(state == ImplicitContext::State::SoftSet, memo_key));
  assertx(IMPLIES(state != ImplicitContext::State::SoftSet &&
                  state != ImplicitContext::State::SoftInaccessible, !memo_key));

  if (state == ImplicitContext::State::Value ||
      state == ImplicitContext::State::Inaccessible) {
    // We do not currently need blame here
    return;
  }

  assertx(func);
  assertx(state == ImplicitContext::State::SoftInaccessible ||
          state == ImplicitContext::State::SoftSet);

  req::vector<const StringData*> blameFromSoftInaccessible;
  req::vector<const StringData*> blameFromSoftSet;

  if (auto const obj = *ImplicitContext::activeCtx) {
    auto const prev = Native::data<ImplicitContext>(obj);
    blameFromSoftInaccessible = prev->m_blameFromSoftInaccessible;
    blameFromSoftSet = prev->m_blameFromSoftSet;
  }

  auto const cur_blame = [&] {
    if (memo_key) return memo_key;
    return func->fullName();
  }();

  if (state == ImplicitContext::State::SoftInaccessible) {
    blameFromSoftInaccessible.push_back(cur_blame);
  } else {
    assertx(state == ImplicitContext::State::SoftSet);
    blameFromSoftSet.push_back(cur_blame);
  }

  context->m_blameFromSoftInaccessible = std::move(blameFromSoftInaccessible);
  context->m_blameFromSoftSet = std::move(blameFromSoftSet);
}

} // namespace

String HHVM_FUNCTION(get_state_unsafe) {
  auto const obj = *ImplicitContext::activeCtx;
  if (!obj) return String{s_ICStateNull.get()};
  auto const context = Native::data<ImplicitContext>(obj);

  switch (context->m_state) {
    case ImplicitContext::State::Value:
      return String{s_ICStateValue.get()};
    case ImplicitContext::State::Inaccessible:
      return String{s_ICStateInaccessible.get()};
    case ImplicitContext::State::SoftInaccessible:
      return String{s_ICStateSoftInaccessible.get()};
    case ImplicitContext::State::SoftSet:
      return String{s_ICStateSoftSet.get()};
  }
  not_reached();
}

TypedValue HHVM_FUNCTION(get_implicit_context, StringArg key) {
  auto const obj = *ImplicitContext::activeCtx;
  if (!obj) return make_tv<KindOfNull>();
  auto const context = Native::data<ImplicitContext>(obj);

  switch (context->m_state) {
    case ImplicitContext::State::Value: {
      auto const it = context->m_map.find(key.get());
      if (it == context->m_map.end()) return make_tv<KindOfNull>();
      auto const result = it->second;
      if (isRefcountedType(result.m_type)) tvIncRefCountable(result);
      return result;
    }
    case ImplicitContext::State::Inaccessible:
      throw_implicit_context_exception("Implicit context is set to inaccessible");
    case ImplicitContext::State::SoftInaccessible:
      raise_implicit_context_warning("Implicit context is set to soft inaccessible");
      // fallthru
    case ImplicitContext::State::SoftSet:
      return make_tv<KindOfNull>();
  }
  not_reached();
}

Variant HHVM_FUNCTION(get_whole_implicit_context) {
  auto const obj = *ImplicitContext::activeCtx;
  if (!obj) return init_null();
  return Object{obj};
}

/*
 * Caution! This function relies on the assumption that
 * every IC is kept alive throughout the request lifetime
 * the IC should stay alive due to being stored in s_memokey_to_IC
*/
int64_t HHVM_FUNCTION(get_implicit_context_memo_key) {
  auto const obj = *ImplicitContext::activeCtx;
  return reinterpret_cast<int64_t>(obj);
}

/*
* Returns a human readable memo key for the current IC
* Only to be used for testing
*/
Array HHVM_FUNCTION(get_implicit_context_debug_info) {
  auto const obj = *ImplicitContext::activeCtx;
  if (!obj) return Array{};
  auto const context = Native::data<ImplicitContext>(obj);

  if (context->m_state == ImplicitContext::State::Inaccessible) {
    VecInit ret{1};
    ret.append(s_ICInaccessibleMemoKey.data());
    return ret.toArray();
  }
  if (context->m_state == ImplicitContext::State::SoftSet ||
      context->m_state == ImplicitContext::State::SoftInaccessible) {
    VecInit ret{context->m_blameFromSoftInaccessible.size() +
                context->m_blameFromSoftSet.size() + 1}; // extra 1 for the memokey name
    if (context->m_state == ImplicitContext::State::SoftInaccessible) {
      ret.append(s_ICSoftInaccessibleMemoKey.data());
    } else {
      ret.append(s_ICSoftSetMemoKey.data());
    }

    for (auto const& s : context->m_blameFromSoftInaccessible) {
      ret.append(s->data());
    }

    for (auto const& s : context->m_blameFromSoftSet) {
      ret.append(s->data());
    }
    return ret.toArray();
  }

  if (context->m_state == ImplicitContext::State::Value) {
    VecInit ret{context->m_map.size() * 2}; // key and value
    for (auto const& p : context->m_map) {
      auto const key = String(p.first->data());
      auto const value = HHVM_FN(serialize_memoize_param)(p.second);
      ret.append(key);
      ret.append(value);
    }
    return ret.toArray();
  }
  return Array{};
}


Object HHVM_FUNCTION(create_implicit_context, StringArg keyarg,
                                              TypedValue data) {
  auto const key = keyarg.get();
  // Reserve the underscore prefix for the time being in case we want to
  // emit keys from the compiler. This would allow us to avoid having
  // conflicts with other key generation mechanisms.
  if (key->size() == 0 || key->data()[0] == '_') {
    throw_implicit_context_exception(
      "Implicit context keys cannot be empty or start with _");
  }
  auto const prev = *ImplicitContext::activeCtx;

  // Compute memory key first
  using Elem = std::pair<const StringData*, TypedValue>;
  req::vector<Elem> vec;
  auto memokey = Variant::attach(HHVM_FN(serialize_memoize_param)(data));

  if (prev) {
    auto& existing_m_map = Native::data<ImplicitContext>(prev)->m_map;
    for (auto const& p : existing_m_map) {
      vec.push_back(std::make_pair(p.first, HHVM_FN(serialize_memoize_param)(p.second)));
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

  context->m_state = ImplicitContext::State::Value;
  set_implicit_context_blame(context, nullptr, nullptr);
  // IncRef data and key as we are storing in the map
  if (isRefcountedType(data.m_type)) tvIncRefCountable(data);
  key->incRefCount();
  if (prev) context->m_map = Native::data<ImplicitContext>(prev)->m_map;
  context->m_map.insert_or_assign(key, data);

  // Store new IC into the map
  auto UNUSED ret = m_to_ic.emplace(std::make_pair(target_memo_key.detach(), Object(obj).detach()));
  assertx(ret.second); // the emplace should always succeed
  return obj;
}

namespace {

Object create_special_implicit_context_impl(int64_t type_enum,
                                            const StringData* memo_key,
                                            const Func* func) {
  auto const prev_obj = *ImplicitContext::activeCtx;
  auto const prev_context =
    prev_obj ? Native::data<ImplicitContext>(prev_obj) : nullptr;

  auto const type = static_cast<ImplicitContext::State>(type_enum);
  assertx(type != ImplicitContext::State::Value);
  if (type == ImplicitContext::State::SoftSet &&
      prev_context &&
      (prev_context->m_state == ImplicitContext::State::Value ||
       prev_context->m_state == ImplicitContext::State::Inaccessible)) {
    // If we are moving from Value or Inaccessible to SoftSet, remain
    // in previous configuration
    return Object{prev_obj};
  }

  if (type == ImplicitContext::State::SoftInaccessible) {
    auto const sampleRate = [&] {
      if (memo_key) return 1u;
      assertx(func->isMemoizeWrapper() || func->isMemoizeWrapperLSB());
      assertx(func->isSoftMakeICInaccessibleMemoize());
      return func->softMakeICInaccessibleSampleRate();
    }();
    if (sampleRate > 1) {
      bool shouldSample;
      if (UNLIKELY(RO::EvalRecordReplay)) {
        auto rand{reinterpret_cast<int64_t(*)(int64_t, const Variant&)>(
          Func::lookup(StringData::Make("rand"))->nativeFuncPtr())};
        shouldSample = rand(0, sampleRate - 1) != 0;
      } else {
        shouldSample = !folly::Random::oneIn(sampleRate);
      }
      if (shouldSample) {
        // Return the previous object if we coinflipped false
        return Object{prev_obj};
      }
    }
  }

  auto target_memo_key = get_target_memo_key(memo_key, func, type);

  auto& m_to_ic = *s_memokey_to_IC;
  if (m_to_ic.find(target_memo_key.get()) != m_to_ic.end()) {
    return Object{m_to_ic.at(target_memo_key.get())};
  }

  // create new IC since we did not find an existing one that matches the description
  auto obj = create_new_IC();
  auto const context = Native::data<ImplicitContext>(obj.get());
  context->m_state = type;
  auto const key = [&] () -> const StringData* {
    if (!memo_key) return nullptr;
    // Leak the memo_key until the end of the request
    memo_key->incRefCount();
    return memo_key;
  }();
  set_implicit_context_blame(context, key, func);
  if (type == ImplicitContext::State::SoftSet &&
      prev_context &&
      prev_context->m_state == ImplicitContext::State::SoftInaccessible) {
    // If we are moving from SoftInaccessible to SoftSet, remain in
    // SoftInaccessible
    // This must happen after setting the blame as blame setting uses the state
    context->m_state = ImplicitContext::State::SoftInaccessible;
  }
  // Store new IC into the map
  auto UNUSED ret = m_to_ic.emplace(std::make_pair(target_memo_key.detach(), Object(obj).detach()));
  assertx(ret.second); // the emplace should always succeed
  return obj;
}

} // namespace

TypedValue create_special_implicit_context_explicit(int64_t type_enum,
                                                    const StringData* memo_key,
                                                    const Func* func) {
  auto ret = create_special_implicit_context_impl(type_enum, memo_key, func);
  if (ret.isNull()) return make_tv<KindOfNull>();
  return make_tv<KindOfObject>(ret.detach());
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
    HHVM_NAMED_FE(HH\\ImplicitContext\\_Private\\get_implicit_context_debug_info,
                  HHVM_FN(get_implicit_context_debug_info));
    HHVM_NAMED_FE(HH\\Coeffects\\_Private\\enter_zoned_with,
                  HHVM_FN(enter_zoned_with));

#define X(hacknm, cppnm) HHVM_RC_INT(HH\\MEMOIZE_IC_TYPE_##hacknm, \
  static_cast<int64_t>(ImplicitContext::State::cppnm))
    X(VALUE, Value)
    X(INACCESSIBLE, Inaccessible)
    X(SOFT_INACCESSIBLE, SoftInaccessible)
    X(SOFT_SET, SoftSet)
#undef X
  }
} s_hh_implicit_context;

///////////////////////////////////////////////////////////////////////////////
}
