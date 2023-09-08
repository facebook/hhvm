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

struct ImplicitContextLoader :
  SystemLib::ClassLoader<"HH\\ImplicitContext\\_Private\\ImplicitContextData"> {};

Object create_new_IC() {
  auto obj = Object{ ImplicitContextLoader::classof() };
  // PURPOSEFULLY LEAK MEMORY: When the data is stored/restored during the
  // suspend/resume routine, we should properly refcount the data but that is
  // expensive. Leak and let the GC take care of it.
  obj.get()->incRefCount();
  return obj;
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
      auto const result = it->second.first;
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

Object HHVM_FUNCTION(get_whole_implicit_context) {
  auto const obj = *ImplicitContext::activeCtx;
  return Object{obj};
}

String HHVM_FUNCTION(get_implicit_context_memo_key) {
  auto const obj = *ImplicitContext::activeCtx;
  if (!obj) return empty_string();
  auto const context = Native::data<ImplicitContext>(obj);
  assertx(context->m_memokey);
  return String{context->m_memokey};
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

  auto obj = create_new_IC();
  auto const context = Native::data<ImplicitContext>(obj.get());

  context->m_state = ImplicitContext::State::Value;
  set_implicit_context_blame(context, nullptr, nullptr);
  if (prev) context->m_map = Native::data<ImplicitContext>(prev)->m_map;
  // Leak `data`, `key` and `memokey` to the end of the request
  if (isRefcountedType(data.m_type)) tvIncRefCountable(data);
  key->incRefCount();
  auto const memokey = HHVM_FN(serialize_memoize_param)(data);
  auto entry = std::make_pair(data, memokey);
  auto const it = context->m_map.insert({key, entry});
  // If the insertion failed, overwrite
  if (!it.second) it.first->second = entry;

  using Elem = std::pair<const StringData*, TypedValue>;
  req::vector<Elem> vec;
  for (auto const& p : context->m_map) {
    vec.push_back(std::make_pair(p.first, p.second.second));
  }
  std::sort(vec.begin(), vec.end(), [](const Elem e1, const Elem e2) {
                                      return e1.first->compare(e2.first) < 0;
                                    });
  StringBuffer sb;
  for (auto const& e : vec) {
    serialize_memoize_string_data(sb, e.first);
    serialize_memoize_tv(sb, 0, e.second);
  }
  context->m_memokey = sb.detach().detach();
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
  context->m_memokey = [&] {
    if (type == ImplicitContext::State::Inaccessible) {
      return s_ICInaccessibleMemoKey.get();
    }
    StringBuffer sb;
    sb.append(type == ImplicitContext::State::SoftSet
                ? s_ICSoftSetMemoKey.get() : s_ICSoftInaccessibleMemoKey.get());
    for (auto const& s : context->m_blameFromSoftInaccessible) {
      serialize_memoize_string_data(sb, s);
    }
    sb.append('%'); // separator
    for (auto const& s : context->m_blameFromSoftSet) {
      serialize_memoize_string_data(sb, s);
    }
    return sb.detach().detach();
  }();
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
  void moduleInit() override {
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
