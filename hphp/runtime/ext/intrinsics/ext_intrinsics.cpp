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
#include "hphp/runtime/base/backtrace.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/request-tracing.h"
#include "hphp/runtime/base/surprise-flags.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/unit-cache.h"

#include "hphp/runtime/vm/native-data.h"
#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/runtime/ext/asio/asio-external-thread-event.h"
#include "hphp/runtime/ext/std/ext_std.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

void HHVM_FUNCTION(trigger_oom, bool oom) {
  if (oom) {
    setSurpriseFlag(MemExceededFlag);
    RID().setRequestOOMFlag();
  }
}

void HHVM_FUNCTION(trigger_break, bool brk) {
  if (brk) {
    raise(SIGTRAP);
  }
}

void HHVM_FUNCTION(trigger_crash) {
  always_assert(false);
}

TypedValue HHVM_FUNCTION(launder_value, const Variant& val) {
  return tvReturn(val);
}

void HHVM_FUNCTION(launder_value_inout, Variant& val) {
  return;
}

Array HHVM_FUNCTION(memory_manager_stats) {
  auto const& stats = tl_heap->getStats();
  return make_dict_array(
    "mm_udebt", stats.mm_udebt,
    "mm_uallocated", stats.mm_uallocated,
    "mm_freed", stats.mm_freed,
    "extUsage", stats.extUsage,
    "malloc_cap", stats.malloc_cap,
    "mmap_cap", stats.mmap_cap,
    "peakUsage", stats.peakUsage,
    "peakCap", stats.peakCap,
    "peakIntervalUsage", stats.peakIntervalUsage,
    "peakIntervalCap", stats.peakIntervalCap,
    "totalAlloc", stats.totalAlloc,
    "mmap_volume", stats.mmap_volume,
    "mmAllocated", stats.mmAllocated(),
    "mmUsage", stats.mmUsage(),
    "usage", stats.usage(),
    "capacity", stats.capacity(),
    "auxUsage", stats.auxUsage()
  );
}

Array HHVM_FUNCTION(dummy_varray_builtin, const Array& arr) {
  if (arr.isVec()) return arr;
  return Array::CreateVec();
}

Array HHVM_FUNCTION(dummy_darray_builtin, const Array& arr) {
  if (arr.isDict()) return arr;
  return Array::CreateDict();
}

TypedValue HHVM_FUNCTION(dummy_kindofdarray_builtin) {
  return make_array_like_tv(ArrayData::CreateDict());
}

TypedValue HHVM_FUNCTION(dummy_kindofvarray_builtin) {
  return make_array_like_tv(ArrayData::CreateVec());
}

TypedValue HHVM_FUNCTION(dummy_varr_or_darr_builtin, const Variant& var) {
  if (var.isArray()) {
    auto const& arr = var.asCArrRef();
    if (arr.isVec() || arr.isDict()) return tvReturn(arr);
  }
  return tvReturn(ArrayData::CreateVec());
}

TypedValue HHVM_FUNCTION(dummy_arraylike_builtin, const Variant& var) {
  if (var.isArray()) {
    auto const& arr = var.asCArrRef();
    return tvReturn(arr);
  }
  return tvReturn(ArrayData::CreateKeyset());
}

Array HHVM_FUNCTION(dummy_dict_builtin, const Array& arr) {
  if (arr.isDict()) return arr;
  return Array::CreateDict();
}

void HHVM_FUNCTION(dummy_lots_inout,
                   Variant& p1, Variant& p2, Variant& p3, Variant& p4,
                   Variant& p5, Variant& p6, Variant& p7, Variant& p8,
                   Variant& p9, Variant& p10, Variant& p11, Variant& p12) {
}

String HHVM_FUNCTION(serialize_with_format, const Variant& thing,
                     int64_t format) {
  if (format > static_cast<int64_t>(VariableSerializer::Type::Last)) {
    raise_invalid_argument_warning("invalid serializer format");
  }
  VariableSerializer vs(static_cast<VariableSerializer::Type>(format));
  return vs.serialize(thing, true);
}

Variant HHVM_FUNCTION(deserialize_with_format, const String& str,
                      int64_t format) {
  if (format > static_cast<int64_t>(VariableUnserializer::Type::Last)) {
    raise_invalid_argument_warning("invalid serializer format");
  }
  auto const type = static_cast<VariableUnserializer::Type>(format);
  VariableUnserializer vu(str.data(), str.size(), type);
  return vu.unserialize();
}

String HHVM_FUNCTION(serialize_keep_dvarrays, const Variant& value) {
  return serialize_keep_dvarrays(value);
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
      auto const elem0 = arr->get(int64_t{0});
      auto const elem1 = arr->get(int64_t{1});
      auto const elem2 = arr->get(int64_t{2});
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
 *   int64_t $i,
 *   inout object $obj
 *   object $o,
 *   mixed $m,
 *   inout mixed $mix,
 *   bool retOrig,
 *   <<__OutOnly("KindOfBoolean")>> inout mixed $out1,
 *   <<__OutOnly("varray")>> inout mixed $out2,
 *   <<__OutOnly("KindOfObject")>> inout mixed $out3,
 * ): array;
 */
Array HHVM_FUNCTION(
  builtin_io,
  StringArg s,
  String& str,
  int64_t& num,
  int64_t i,
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
    ? make_vec_array(s.get(), str, num, i, obj, o.get(), m, mix)
    : Array::CreateVec();

  str += ";; IN =\"";
  str += StrNR{s.get()};
  str += "\"";

  num = num + i * i;

  if (retOrig) mix = obj;
  else         mix = m;

  obj = Object{o.get()};

  outArr = retOrig
    ? make_vec_array(outBool, outArr, outObj)
    : Array::CreateVec();
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
    auto arr = make_dict_array("foo", "bar", "baz", "quux");
    tv = make_array_like_tv(arr.detach());
  }
};

struct DummyDArrayAwait : AsioExternalThreadEvent {
  DummyDArrayAwait() { markAsFinished(); }

  void unserialize(TypedValue& tv) override {
    auto arr = make_dict_array("foo", "bar", "baz", "quux");
    tv = make_array_like_tv(arr.detach());
  }
};

struct DummyDictAwait : AsioExternalThreadEvent {
  DummyDictAwait() { markAsFinished(); }

  void unserialize(TypedValue& tv) override {
    auto arr = make_dict_array("foo", "bar", "baz", "quux");
    tv = make_array_like_tv(arr.detach());
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

int64_t HHVM_FUNCTION(dummy_int_upper_bound) {
  return 42;
}

///////////////////////////////////////////////////////////////////////////////

Variant HHVM_FUNCTION(create_class_pointer, TypedValue name) {
  auto const cls = [&] {
    switch (name.m_type) {
      case KindOfString:
      case KindOfPersistentString:
        return Class::load(name.m_data.pstr);
      case KindOfLazyClass:
        return Class::load(name.m_data.plazyclass.name());
      case KindOfClass:
        return name.m_data.pclass;
      default:
        return static_cast<Class*>(nullptr);
    }
  }();
  return cls ? Variant{cls} : init_null();
}

Variant HHVM_FUNCTION(create_clsmeth_pointer, StringArg cls, StringArg meth) {
  if (RuntimeOption::RepoAuthoritative) {
    raise_error("You can't use %s() in RepoAuthoritative mode", __FUNCTION__+2);
  }
  auto const c = Class::load(cls.get());
  if (!c) return init_null();
  auto const m = c->lookupMethod(meth.get());
  if (!m || !m->isStaticInPrologue()) return init_null();
  return Variant{ClsMethDataRef::create(c, m)};
}

bool HHVM_FUNCTION(is_lazy_class, TypedValue val) {
  return tvIsLazyClass(val);
}

void HHVM_FUNCTION(debug_var_dump_lazy_class, TypedValue val) {
  switch (val.m_type) {
    case KindOfLazyClass:
      g_context->write(folly::sformat("lazyclass({})\n", val.m_data.plazyclass.name()));
      break;
    case KindOfClass:
      g_context->write(folly::sformat("class({})\n", val.m_data.pclass->name()));
      break;
    default:
      VariableSerializer vs(VariableSerializer::Type::VarDump, 0, 2);
      vs.serialize(Variant::attach(val), false);
  }
}

///////////////////////////////////////////////////////////////////////////////

bool HHVM_FUNCTION(is_unit_loaded, StringArg path) {
  return getLoadedUnit(path.get()) != nullptr;
}

void HHVM_FUNCTION(drain_unit_prefetcher) {
  if (RO::RepoAuthoritative || !unitPrefetchingEnabled()) return;
  drainUnitPrefetcher();
}

///////////////////////////////////////////////////////////////////////////////

String HHVM_FUNCTION(debug_get_bytecode) {
  return fromCaller([] (const BTFrame& frame) {
    std::ostringstream ss;
    frame.func()->prettyPrint(ss);
    return String(ss.str());
  });
}

Array HHVM_FUNCTION(debug_file_deps) {
  return fromCaller([] (const BTFrame& frame) {
    auto const& deps = frame.func()->unit()->deps();
    VecInit vinit{deps.size()};
    for (auto const& [name, sha] : deps) {
      vinit.append(String(name));
    }
    return vinit.toArray();
  });
}

///////////////////////////////////////////////////////////////////////////////

bool HHVM_FUNCTION(is_module_in_deployment, StringArg module,
                                            StringArg deployment) {
  auto const& packageInfo = g_context->getPackageInfo();
  auto const it =
    packageInfo.deployments().find(deployment.get()->toCppString());
  if (it == end(packageInfo.deployments())) return false;
  return packageInfo.moduleInDeployment(module.get(), it->second, DeployKind::HardOrSoft);
}

///////////////////////////////////////////////////////////////////////////////

}

struct DummyNativeData {
  int64_t dummy;
};

namespace {

const StaticString s_ExtensibleNewableClassWithNativeData(
  "__hhvm_intrinsics\\ExtensibleNewableClassWithNativeData");

void
HHVM_METHOD(ExtensibleNewableClassWithNativeData, setDummyValue, int64_t v) {
  Native::data<DummyNativeData>(this_)->dummy = v;
}

int64_t HHVM_METHOD(ExtensibleNewableClassWithNativeData, getDumyValue) {
  return Native::data<DummyNativeData>(this_)->dummy;
}

///////////////////////////////////////////////////////////////////////////////

}

static struct IntrinsicsExtension final : Extension {
  IntrinsicsExtension() : Extension("intrinsics", NO_EXTENSION_VERSION_YET, NO_ONCALL_YET) {}

  bool moduleEnabled() const override {
    return RuntimeOption::EnableIntrinsicsExtension;
  }

  void moduleRegisterNative() override {
    HHVM_FALIAS(__hhvm_intrinsics\\builtin_io, builtin_io);
    HHVM_FALIAS(__hhvm_intrinsics\\builtin_io_no_fca, builtin_io);
    HHVM_FALIAS(__hhvm_intrinsics\\builtin_io_foldable, builtin_io_foldable);

    HHVM_FALIAS(__hhvm_intrinsics\\trigger_oom, trigger_oom);
    HHVM_FALIAS(__hhvm_intrinsics\\trigger_break, trigger_break);
    HHVM_FALIAS(__hhvm_intrinsics\\trigger_crash, trigger_crash);
    HHVM_FALIAS(__hhvm_intrinsics\\launder_value, launder_value);
    HHVM_FALIAS(__hhvm_intrinsics\\launder_value_inout, launder_value_inout);

    HHVM_FALIAS(__hhvm_intrinsics\\memory_manager_stats, memory_manager_stats);

    HHVM_FALIAS(__hhvm_intrinsics\\dummy_varray_builtin, dummy_varray_builtin);
    HHVM_FALIAS(__hhvm_intrinsics\\dummy_darray_builtin, dummy_darray_builtin);
    HHVM_FALIAS(__hhvm_intrinsics\\dummy_kindofdarray_builtin, dummy_kindofdarray_builtin);
    HHVM_FALIAS(__hhvm_intrinsics\\dummy_kindofvarray_builtin, dummy_kindofvarray_builtin);
    HHVM_FALIAS(__hhvm_intrinsics\\dummy_varr_or_darr_builtin,
                dummy_varr_or_darr_builtin);
    HHVM_FALIAS(__hhvm_intrinsics\\dummy_arraylike_builtin,
                dummy_arraylike_builtin);
    HHVM_FALIAS(__hhvm_intrinsics\\dummy_dict_builtin, dummy_dict_builtin);

    HHVM_FALIAS(__hhvm_intrinsics\\dummy_array_await, dummy_array_await);
    HHVM_FALIAS(__hhvm_intrinsics\\dummy_darray_await, dummy_darray_await);
    HHVM_FALIAS(__hhvm_intrinsics\\dummy_dict_await, dummy_dict_await);

    HHVM_FALIAS(__hhvm_intrinsics\\dummy_int_upper_bound, dummy_int_upper_bound);

    HHVM_FALIAS(__hhvm_intrinsics\\dummy_lots_inout, dummy_lots_inout);

    HHVM_FALIAS(__hhvm_intrinsics\\serialize_with_format, serialize_with_format);
    HHVM_FALIAS(__hhvm_intrinsics\\deserialize_with_format,
                deserialize_with_format);
    HHVM_FALIAS(__hhvm_intrinsics\\serialize_keep_dvarrays,
                serialize_keep_dvarrays);

    HHVM_FALIAS(__hhvm_intrinsics\\rqtrace_create_event, rqtrace_create_event);
    HHVM_FALIAS(__hhvm_intrinsics\\rqtrace_create_scope, rqtrace_create_scope);
    HHVM_FALIAS(__hhvm_intrinsics\\rqtrace_create_scoped_events,
                rqtrace_create_scoped_events);

    HHVM_FALIAS(__hhvm_intrinsics\\hhbbc_fail_verification,
                hhbbc_fail_verification);

    HHVM_FALIAS(__hhvm_intrinsics\\create_class_pointer, create_class_pointer);
    HHVM_FALIAS(__hhvm_intrinsics\\create_clsmeth_pointer,
                create_clsmeth_pointer);
    HHVM_FALIAS(__hhvm_intrinsics\\is_lazy_class, is_lazy_class);
    HHVM_FALIAS(__hhvm_intrinsics\\debug_var_dump_lazy_class,
		debug_var_dump_lazy_class);

    HHVM_FALIAS(__hhvm_intrinsics\\is_unit_loaded, is_unit_loaded);
    HHVM_FALIAS(__hhvm_intrinsics\\drain_unit_prefetcher, drain_unit_prefetcher);

    HHVM_FALIAS(__hhvm_intrinsics\\debug_get_bytecode, debug_get_bytecode);
    HHVM_FALIAS(__hhvm_intrinsics\\debug_file_deps, debug_file_deps);

    HHVM_FALIAS(__hhvm_intrinsics\\is_module_in_deployment, is_module_in_deployment);

    HHVM_NAMED_ME(__hhvm_intrinsics\\ExtensibleNewableClassWithNativeData,
                  setDummyValue,
                  HHVM_MN(ExtensibleNewableClassWithNativeData, setDummyValue));
    HHVM_NAMED_ME(__hhvm_intrinsics\\ExtensibleNewableClassWithNativeData,
                  getDumyValue,
                  HHVM_MN(ExtensibleNewableClassWithNativeData,getDumyValue));

    Native::registerNativeDataInfo<DummyNativeData>(
      s_ExtensibleNewableClassWithNativeData.get());
  }
} s_intrinsics_extension;

///////////////////////////////////////////////////////////////////////////////

}
