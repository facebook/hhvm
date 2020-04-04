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

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/request-tracing.h"
#include "hphp/runtime/base/surprise-flags.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/runtime/vm/jit/inlining-decider.h"

#include "hphp/runtime/ext/asio/asio-external-thread-event.h"
#include "hphp/runtime/ext/std/ext_std.h"

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

TypedValue HHVM_FUNCTION(dummy_kindofdarray_builtin) {
  TypedValue tv;
  type(tv) = KindOfDArray;
  val(tv).parr = ArrayData::CreateDArray();
  return tv;
}

TypedValue HHVM_FUNCTION(dummy_kindofvarray_builtin) {
  TypedValue tv;
  type(tv) = KindOfVArray;
  val(tv).parr = ArrayData::CreateVArray();
  return tv;
}

TypedValue HHVM_FUNCTION(dummy_varr_or_darr_builtin, const Variant& var) {
  if (var.isArray()) {
    auto const& arr = var.asCArrRef();
    if (arr.isVecOrVArray() ||
        arr.isDictOrDArray()) return tvReturn(arr);
  }
  return tvReturn(ArrayData::CreateVArray());
}

TypedValue HHVM_FUNCTION(dummy_arraylike_builtin, const Variant& var) {
  if (var.isArray()) {
    auto const& arr = var.asCArrRef();
    return tvReturn(arr);
  }
  return tvReturn(ArrayData::CreateKeyset());
}

TypedValue HHVM_FUNCTION(dummy_cast_to_kindofarray, const Variant& var) {
  if (!var.isArray()) {
    SystemLib::throwInvalidOperationExceptionObject("must pass arraylike");
  }
  auto const& arr = var.asCArrRef();
  if (arr->isPHPArrayType() && arr->isNotDVArray()) {
    return tvReturn(arr.get());
  }
  TypedValue tv;
  val(tv).parr = arr.toPHPArray().detach();
  type(tv) = KindOfArray;
  return tv;
}

TypedValue HHVM_FUNCTION(dummy_cast_to_kindofdarray, const Array& arr) {
  if (arr->isDArray()) return tvReturn(arr.get());
  TypedValue tv;
  val(tv).parr = arr.toDArray().detach();
  type(tv) = KindOfDArray;
  return tv;
}

TypedValue HHVM_FUNCTION(dummy_cast_to_kindofvarray, const Array& arr) {
  if (arr->isVArray()) return tvReturn(arr.get());
  TypedValue tv;
  val(tv).parr = arr.toVArray().detach();
  type(tv) = KindOfVArray;
  return tv;
}

Array HHVM_FUNCTION(dummy_array_builtin, const Array& arr) {
  if (!arr.isVecOrVArray() && !arr.isDictOrDArray()) return arr;
  return Array::Create();
}

Array HHVM_FUNCTION(dummy_dict_builtin, const Array& arr) {
  if (arr.isDict()) return arr;
  return Array::CreateDict();
}

String HHVM_FUNCTION(serialize_with_format, const Variant& thing,
                     int64_t format) {
  if (format > static_cast<int64_t>(VariableSerializer::Type::Last)) {
    raise_invalid_argument_warning("invalid serializer format");
  }
  VariableSerializer vs(static_cast<VariableSerializer::Type>(format));
  return vs.serialize(thing, true);
}

namespace {

///////////////////////////////////////////////////////////////////////////////

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
    [&] (TypedValue k, TypedValue v) {
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
    [&] (TypedValue k, TypedValue v) {
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

/*
 * function builtin_io(
 *   string $s,
 *   inout string $str,
 *   inout int $num,
 *   int $i,
 *   inout object $obj
 *   object $o,
 *   mixed $m,
 *   inout mixed $mix,
 *   bool retOrig,
 *   <<__OutOnly("KindOfBoolean")>> inout mixed $out1,
 *   <<__OutOnly("KindOfArray")>> inout mixed $out2,
 *   <<__OutOnly("KindOfObject")>> inout mixed $out3,
 * ): array;
 */
Array HHVM_FUNCTION(
  builtin_io,
  StringArg s,
  String& str,
  int64_t& num,
  int i,
  Object& obj,
  ObjectArg o,
  const Variant& m,
  Variant& mix,
  bool retOrig,
  bool& outBool,
  Array& outArr,
  Variant& outObj
) {
  auto const orig = retOrig
    ? make_packed_array(s.get(), str, num, i, obj, o.get(), m, mix)
    : Array::Create();

  str += ";; IN =\"";
  str += StrNR{s.get()};
  str += "\"";

  num = num + i * i;

  if (retOrig) mix = obj;
  else         mix = m;

  obj = Object{o.get()};

  outArr = retOrig
    ? make_packed_array(outBool, outArr, outObj)
    : Array::Create();
  outBool = true;
  outObj = SystemLib::AllocStdClassObject();

  return orig;
}

int64_t HHVM_FUNCTION(
  builtin_io_foldable,
  int64_t a,
  int64_t& b,
  int64_t& c,
  int64_t& d
) {
  auto const t1 = b * c * d;
  auto const t2 = a * c * d;
  auto const t3 = a * b * d;
  auto const t4 = a * b * c;
  b = t2;
  c = t3;
  d = t4;
  return t1;
}

///////////////////////////////////////////////////////////////////////////////

struct DummyArrayAwait : AsioExternalThreadEvent {
  DummyArrayAwait() { markAsFinished(); }

  void unserialize(TypedValue& tv) override {
    auto arr = make_map_array("foo", "bar", "baz", "quux");
    type(tv) = KindOfArray;
    val(tv).parr = arr.detach();
  }
};

struct DummyDArrayAwait : AsioExternalThreadEvent {
  DummyDArrayAwait() { markAsFinished(); }

  void unserialize(TypedValue& tv) override {
    auto arr = make_darray("foo", "bar", "baz", "quux");
    type(tv) = arr->toDataType();
    val(tv).parr = arr.detach();
  }
};

struct DummyDictAwait : AsioExternalThreadEvent {
  DummyDictAwait() { markAsFinished(); }

  void unserialize(TypedValue& tv) override {
    auto arr = make_dict_array("foo", "bar", "baz", "quux");
    type(tv) = KindOfDict;
    val(tv).parr = arr.detach();
  }
};

Object HHVM_FUNCTION(dummy_array_await) {
  auto ev = new DummyArrayAwait();
  return Object{ev->getWaitHandle()};
}

Object HHVM_FUNCTION(dummy_darray_await) {
  auto ev = new DummyDArrayAwait();
  return Object{ev->getWaitHandle()};
}

Object HHVM_FUNCTION(dummy_dict_await) {
  auto ev = new DummyDictAwait();
  return Object{ev->getWaitHandle()};
}

///////////////////////////////////////////////////////////////////////////////

Variant HHVM_FUNCTION(create_class_pointer, StringArg name) {
  auto const cls = Unit::loadClass(name.get());
  return cls ? Variant{cls} : init_null();
}

Variant HHVM_FUNCTION(create_clsmeth_pointer, StringArg cls, StringArg meth) {
  if (RuntimeOption::RepoAuthoritative) {
    raise_error("You can't use %s() in RepoAuthoritative mode", __FUNCTION__+2);
  }
  auto const c = Unit::loadClass(cls.get());
  if (!c) return init_null();
  auto const m = c->lookupMethod(meth.get());
  if (!m || !m->isStaticInPrologue()) return init_null();
  return Variant{ClsMethDataRef::create(c, m)};
}

///////////////////////////////////////////////////////////////////////////////

}

void StandardExtension::initIntrinsics() {
  if (!RuntimeOption::EnableIntrinsicsExtension) return;

  HHVM_FALIAS(__hhvm_intrinsics\\builtin_io, builtin_io);
  HHVM_FALIAS(__hhvm_intrinsics\\builtin_io_no_fca, builtin_io);
  HHVM_FALIAS(__hhvm_intrinsics\\builtin_io_foldable, builtin_io_foldable);

  HHVM_FALIAS(__hhvm_intrinsics\\trigger_oom, trigger_oom);
  HHVM_FALIAS(__hhvm_intrinsics\\launder_value, launder_value);

  HHVM_FALIAS(__hhvm_intrinsics\\dummy_varray_builtin, dummy_varray_builtin);
  HHVM_FALIAS(__hhvm_intrinsics\\dummy_darray_builtin, dummy_darray_builtin);
  HHVM_FALIAS(__hhvm_intrinsics\\dummy_kindofdarray_builtin, dummy_kindofdarray_builtin);
  HHVM_FALIAS(__hhvm_intrinsics\\dummy_kindofvarray_builtin, dummy_kindofvarray_builtin);
  HHVM_FALIAS(__hhvm_intrinsics\\dummy_varr_or_darr_builtin,
              dummy_varr_or_darr_builtin);
  HHVM_FALIAS(__hhvm_intrinsics\\dummy_arraylike_builtin,
              dummy_arraylike_builtin);
  HHVM_FALIAS(__hhvm_intrinsics\\dummy_array_builtin, dummy_array_builtin);
  HHVM_FALIAS(__hhvm_intrinsics\\dummy_dict_builtin, dummy_dict_builtin);

  HHVM_FALIAS(__hhvm_intrinsics\\dummy_cast_to_kindofarray,
              dummy_cast_to_kindofarray);
  HHVM_FALIAS(__hhvm_intrinsics\\dummy_cast_to_kindofdarray,
              dummy_cast_to_kindofdarray);
  HHVM_FALIAS(__hhvm_intrinsics\\dummy_cast_to_kindofvarray,
              dummy_cast_to_kindofvarray);

  HHVM_FALIAS(__hhvm_intrinsics\\dummy_array_await, dummy_array_await);
  HHVM_FALIAS(__hhvm_intrinsics\\dummy_darray_await, dummy_darray_await);
  HHVM_FALIAS(__hhvm_intrinsics\\dummy_dict_await, dummy_dict_await);

  HHVM_FALIAS(__hhvm_intrinsics\\serialize_with_format, serialize_with_format);

  HHVM_FALIAS(__hhvm_intrinsics\\rqtrace_create_event, rqtrace_create_event);
  HHVM_FALIAS(__hhvm_intrinsics\\rqtrace_create_scope, rqtrace_create_scope);
  HHVM_FALIAS(__hhvm_intrinsics\\rqtrace_create_scoped_events,
              rqtrace_create_scoped_events);

  HHVM_FALIAS(__hhvm_intrinsics\\hhbbc_fail_verification,
              hhbbc_fail_verification);

  HHVM_FALIAS(__hhvm_intrinsics\\create_class_pointer, create_class_pointer);
  HHVM_FALIAS(__hhvm_intrinsics\\create_clsmeth_pointer,
              create_clsmeth_pointer);

  loadSystemlib("std_intrinsics");
}

///////////////////////////////////////////////////////////////////////////////

}
