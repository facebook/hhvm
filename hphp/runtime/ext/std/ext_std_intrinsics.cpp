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
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/request-tracing.h"
#include "hphp/runtime/base/surprise-flags.h"
#include "hphp/runtime/ext/std/ext_std.h"
#include "hphp/runtime/vm/jit/inlining-decider.h"
#include "hphp/runtime/vm/vm-regs.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

void HHVM_FUNCTION(trigger_oom, bool oom) {
  if (oom) setSurpriseFlag(MemExceededFlag);
}

TypedValue HHVM_FUNCTION(launder_value, const Variant& val) {
  return tvReturn(val);
}

Array HHVM_FUNCTION(dummy_varray_builtin, const Array& arr) {
  if (arr.isVecOrVArray()) return arr;
  return Array::CreateVArray();
}

Array HHVM_FUNCTION(dummy_darray_builtin, const Array& arr) {
  if (arr.isDictOrDArray()) return arr;
  return Array::CreateDArray();
}

TypedValue HHVM_FUNCTION(dummy_varr_or_darr_builtin, const Variant& var) {
  if (var.isArray()) {
    auto const& arr = var.asCArrRef();
    if (arr.isVecOrVArray() ||
        arr.isDictOrDArray()) return tvReturn(arr);
  }
  return tvReturn(staticEmptyVArray());
}

TypedValue HHVM_FUNCTION(dummy_arraylike_builtin, const Variant& var) {
  if (var.isArray()) {
    auto const& arr = var.asCArrRef();
    return tvReturn(arr);
  }
  return tvReturn(staticEmptyKeysetArray());
}

Array HHVM_FUNCTION(dummy_array_builtin, const Array& arr) {
  if (!arr.isVecOrVArray() && !arr.isDictOrDArray()) return arr;
  return Array::Create();
}

String HHVM_FUNCTION(serialize_with_format, const Variant& thing,
                     int64_t format) {
  if (format > static_cast<int64_t>(VariableSerializer::Type::Last)) {
    throw_invalid_argument("invalid serializer format");
  }
  VariableSerializer vs(static_cast<VariableSerializer::Type>(format));
  return vs.serialize(thing, true);
}

namespace {

timespec from_micros(int64_t us) {
  struct timespec ts;
  ts.tv_sec = us / 1000000;
  ts.tv_nsec = (us % 1000000) * 1000;
  return ts;
}

template<class T>
void annotate_items(T& what, const ArrayData* annot) {
  IterateKV(
    annot,
    [&] (Cell k, TypedValue v) {
      if (!tvIsString(k) || !tvIsString(v)) return;
      what.annotate(k.m_data.pstr->data(), v.m_data.pstr->data());
    }
  );
}

void HHVM_FUNCTION(
  rqtrace_create_event,
  StringArg name,
  int64_t start_us,
  int64_t end_us,
  ArrayArg annot
) {
  rqtrace::ScopeGuard dummy("DUMMY_SCOPE", from_micros(start_us));
  rqtrace::EventGuard g(name->data(), from_micros(start_us));
  annotate_items(g, annot.get());
  g.finish(from_micros(end_us));
  dummy.finish(from_micros(end_us));
}

void HHVM_FUNCTION(
  rqtrace_create_scope,
  StringArg name,
  int64_t start_us,
  int64_t end_us,
  ArrayArg annot
) {
  rqtrace::ScopeGuard g(name->data(), from_micros(start_us));
  annotate_items(g, annot.get());
  g.finish(from_micros(end_us));
}

void HHVM_FUNCTION(
  rqtrace_create_scoped_events,
  StringArg name,
  int64_t start_us,
  int64_t end_us,
  StringArg prefix,
  StringArg suffix,
  ArrayArg annot,
  ArrayArg events
) {
  rqtrace::ScopeGuard g(name->data(), from_micros(start_us));
  annotate_items(g, annot.get());
  g.setEventPrefix(prefix->data());
  g.setEventSuffix(suffix->data());
  IterateKV(
    events.get(),
    [&] (Cell k, TypedValue v) {
      auto const arr = v.m_data.parr;
      if (!tvIsString(k) || !tvIsArrayLike(v) || arr->size() != 3) return;
      auto const elem0 = arr->rval(int64_t{0});
      auto const elem1 = arr->rval(int64_t{1});
      auto const elem2 = arr->rval(int64_t{2});
      if (!elem0 || !elem1 || !elem2) return;
      if (!tvIsInt(elem0) || !tvIsInt(elem1) || !tvIsDict(elem2)) return;
      rqtrace::EventGuard g_event(
        k.m_data.pstr->data(), from_micros(elem0.val().num));
      annotate_items(g_event, elem2.val().parr);
      g_event.finish(from_micros(elem1.val().num));
    }
  );
  g.finish(from_micros(end_us));
}

void HHVM_FUNCTION(hhbbc_fail_verification) {
  g_context->write("PASS\n", 5);
}

}

void StandardExtension::initIntrinsics() {
  if (!RuntimeOption::EnableIntrinsicsExtension) return;

  HHVM_FALIAS(__hhvm_intrinsics\\trigger_oom, trigger_oom);
  HHVM_FALIAS(__hhvm_intrinsics\\launder_value, launder_value);

  HHVM_FALIAS(__hhvm_intrinsics\\dummy_varray_builtin, dummy_varray_builtin);
  HHVM_FALIAS(__hhvm_intrinsics\\dummy_darray_builtin, dummy_darray_builtin);
  HHVM_FALIAS(__hhvm_intrinsics\\dummy_varr_or_darr_builtin,
              dummy_varr_or_darr_builtin);
  HHVM_FALIAS(__hhvm_intrinsics\\dummy_arraylike_builtin,
              dummy_arraylike_builtin);
  HHVM_FALIAS(__hhvm_intrinsics\\dummy_array_builtin, dummy_array_builtin);

  HHVM_FALIAS(__hhvm_intrinsics\\serialize_with_format, serialize_with_format);

  HHVM_FALIAS(__hhvm_intrinsics\\rqtrace_create_event, rqtrace_create_event);
  HHVM_FALIAS(__hhvm_intrinsics\\rqtrace_create_scope, rqtrace_create_scope);
  HHVM_FALIAS(__hhvm_intrinsics\\rqtrace_create_scoped_events,
              rqtrace_create_scoped_events);

  HHVM_FALIAS(__hhvm_intrinsics\\hhbbc_fail_verification,
              hhbbc_fail_verification);

  loadSystemlib("std_intrinsics");
}

///////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
