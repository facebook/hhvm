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

#include "hphp/runtime/ext/hh/ext_hh.h"

#include <sys/types.h>
#include <sys/stat.h>

#include <folly/json.h>
#include <folly/Random.h>
#include <folly/synchronization/AtomicNotification.h>

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/autoload-handler.h"
#include "hphp/runtime/base/autoload-map.h"
#include "hphp/runtime/base/backtrace.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/container-functions.h"
#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/file-stream-wrapper.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/request-tracing.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/stream-wrapper-registry.h"
#include "hphp/runtime/base/type-structure-helpers-defs.h"
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/bespoke/type-structure.h"
#include "hphp/runtime/ext/fb/ext_fb.h"
#include "hphp/runtime/ext/collections/ext_collections-pair.h"
#include "hphp/runtime/ext/std/ext_std_closure.h"
#include "hphp/runtime/vm/class-meth-data-ref.h"
#include "hphp/runtime/vm/memo-cache.h"
#include "hphp/runtime/vm/repo-global-data.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/unit-parser.h"
#include "hphp/runtime/vm/vm-regs.h"
#include "hphp/util/file.h"
#include "hphp/util/match.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

const StaticString
  s_set_frame_metadata("HH\\set_frame_metadata"),
  // The following are used in serialize_memoize_tv to serialize objects that
  // implement the IMemoizeParam interface
  s_IMemoizeParam("HH\\IMemoizeParam"),
  s_getInstanceKey("getInstanceKey"),
  // The following are used in serialize_memoize_param(); they are what
  // serialize_memoize_tv would have produced on some common inputs; having
  // these lets us avoid creating a StringBuffer
  s_nullMemoKey("\xe0"),
  s_trueMemoKey("\xe1"),
  s_falseMemoKey("\xe2");

///////////////////////////////////////////////////////////////////////////////
bool HHVM_FUNCTION(autoload_is_native) {
  auto const* autoloadMap = AutoloadHandler::s_instance->getAutoloadMap();
  return autoloadMap && autoloadMap->isNative();
}

bool HHVM_FUNCTION(autoload_set_paths,
                   const Variant& map,
                   const String& root) {
  // If we are using a native autoload map you are not allowed to override it
  // in repo mode
  if (RuntimeOption::RepoAuthoritative) {
    return false;
  }

  if (RuntimeOption::AutoloadUserlandDisabled > 1) {
    SystemLib::throwInvalidOperationExceptionObject(
      "Attempted to call HH\\autoload_set_paths() when "
      "Autoload.UserlandDisabled is greater than 1. HH\\autoload_set_paths() "
      "will soon be deleted so HHVM internals can start depending on native "
      "autoloading.");
  }

  if (RuntimeOption::AutoloadUserlandDisabled > 0) {
    auto const rate = RuntimeOption::AutoloadUserlandDisabledSampleRate;
    if (StructuredLog::coinflip(rate)) {
      StructuredLogEntry ent;
      ent.force_init = true;
      ent.setInt("autoload_is_native", HHVM_FN(autoload_is_native)());
      ent.setInt("sample_rate", rate);
      if (!root.isNull()) ent.setStr("root", root.slice());
      StructuredLog::log("hhvm_autoload_set_paths", ent);
    }
    raise_notice(
      "Attempted to call HH\\autoload_set_paths() while the native autoloader "
      "is enabled. HH\\autoload_set_paths() disables the native autoloader, "
      "putting us on a deprecated code path that will soon stop working.");
  }

  if (map.isArray()) {
    return AutoloadHandler::s_instance->setMap(map.asCArrRef(), root);
  }
  if (!(map.isObject() && map.toObject()->isCollection())) {
    return false;
  }
  // Assume we have Map<string, Map<string, string>> - convert to
  // array<string, array<string, string>>
  //
  // Exception for 'failure' which should be a callable.
  auto as_array = map.toArray();
  for (auto it = as_array.begin(); !it.end(); it.next()) {
    if (it.second().isObject() && it.second().toObject()->isCollection()) {
      as_array.set(it.first(), it.second().toArray());
    }
  }
  return AutoloadHandler::s_instance->setMap(as_array, root);
}

namespace {

Variant autoload_symbol_to_path(const String& symbol, AutoloadMap::KindOf kind) {
  if (AutoloadHandler::s_instance->getAutoloadMap() == nullptr) {
    SystemLib::throwInvalidOperationExceptionObject(
      "Only available when autoloader is active");
  }
  auto pathRes = AutoloadHandler::s_instance->getFile(symbol, kind);
  if (!pathRes) {
    return null_string;
  }
  return pathRes->path;
}

} // end anonymous namespace

Array HHVM_FUNCTION(autoload_get_paths) {
  auto const* map = AutoloadHandler::s_instance->getAutoloadMap();
  if (!map) {
    SystemLib::throwInvalidArgumentExceptionObject("Autoloader not enabled");
  }
  return map->getAllFiles();
}

Variant HHVM_FUNCTION(autoload_type_to_path, const String& type) {
  return autoload_symbol_to_path(type, AutoloadMap::KindOf::Type);
}

Variant HHVM_FUNCTION(autoload_function_to_path, const String& function) {
  return autoload_symbol_to_path(function, AutoloadMap::KindOf::Function);
}

Variant HHVM_FUNCTION(autoload_constant_to_path, const String& constant) {
  return autoload_symbol_to_path(constant, AutoloadMap::KindOf::Constant);
}

Variant HHVM_FUNCTION(autoload_module_to_path, const String& module) {
  return autoload_symbol_to_path(module, AutoloadMap::KindOf::Module);
}

Variant HHVM_FUNCTION(autoload_type_alias_to_path, const String& typeAlias) {
  return autoload_symbol_to_path(typeAlias, AutoloadMap::KindOf::TypeAlias);
}

namespace {

Array autoload_path_to_symbols(const String& path, AutoloadMap::KindOf kind) {
  if (!HHVM_FN(autoload_is_native)()) {
    SystemLib::throwInvalidOperationExceptionObject("Only available if using native autoloader");
  }
  return AutoloadHandler::s_instance->getSymbols(path, kind);
}

} // end anonymous namespace

Array HHVM_FUNCTION(autoload_path_to_types, const String& path) {
  return autoload_path_to_symbols(path, AutoloadMap::KindOf::Type);
}

Array HHVM_FUNCTION(autoload_path_to_functions, const String& path) {
  return autoload_path_to_symbols(path, AutoloadMap::KindOf::Function);
}

Array HHVM_FUNCTION(autoload_path_to_constants, const String& path) {
  return autoload_path_to_symbols(path, AutoloadMap::KindOf::Constant);
}

Array HHVM_FUNCTION(autoload_path_to_modules, const String& path) {
  return autoload_path_to_symbols(path, AutoloadMap::KindOf::Module);
}

Array HHVM_FUNCTION(autoload_path_to_type_aliases, const String& path) {
  return autoload_path_to_symbols(path, AutoloadMap::KindOf::TypeAlias);
}

bool HHVM_FUNCTION(could_include, const String& file) {
  return lookupUnit(file.get(), "", nullptr /* initial_opt */,
                    Native::s_noNativeFuncs, false) != nullptr;
}

namespace {

inline const StringData* classToMemoKeyHelper(const Class* cls) {
  if (RuntimeOption::EvalClassMemoNotices) {
   raise_class_to_memokey_conversion_warning();
 }
 return cls->name();
}

const StringData* lazyClassToMemoKeyHelper(const LazyClassData& lclass) {
  if (RuntimeOption::EvalClassMemoNotices) {
    raise_class_to_memokey_conversion_warning();
  }
  return lclass.name();
}

/**
 * Param serialization for memoization.
 *
 * This code is based on fb_compact_serialize; it was forked because
 * fb_compact_serialize would sometimes ignore insertion order in arrays;
 * e.g. [1 => 1, 2 => 2] would serialize the same as [2 => 2, 1 => 1], which
 * is not acceptable for the memoization use case. The forked code is not
 * compatible with fb_serialize or fb_compact_serialize. The values returned
 * are not intended for deserialization or for shipping across the network,
 * only for use as dict keys; this means it is possible to change it without
 * notice, and it is also why it is safe to do everything in host byte-order.
 *
 * === Format ===
 *
 * Integer values are returned as-is. Strings where the first byte is
 * < kCodePrefix are returned as-is. All other values are converted to strings
 * of the form <c> <data> where c is a byte (kCodePrefix | code), and data is
 * a sequence of 0 or more bytes. The codes are:
 *
 * 0 (NULL): KindOfUninit or KindOfNull; data has length 0
 * 1 (TRUE),
 * 2 (FALSE): KindOfBoolean; data has length 0
 * 3 (INT8),
 * 4 (INT16),
 * 5 (INT32),
 * 6 (INT64): KindOfInt64; data has length 1, 2, 4 or 8 respectively, and is
 *            a signed value in host byte order
 * 7 (DOUBLE): KindOfDouble; data has length 8, is in host byte order
 * 8 (STRING): string; data is a int serialized as above, followed by that
 *             many bytes of string data
 * 9 (OBJECT): KindOfObject that extends IMemoizeParam; data is an int
 *             serialized as above, followed by that many bytes of serialized
 *             object data (obtained by calling getInstanceKey on the object)
 * 10 (CONTAINER): any PHP array, collection, or hack array; data is the
 *                 keys and values of the container in insertion order,
 *                 serialized as above, followed by the STOP code
 * 16 (STOP): terminates a CONTAINER encoding
 */

enum SerializeMemoizeCode {
  SER_MC_NULL      = 0,
  SER_MC_TRUE      = 1,
  SER_MC_FALSE     = 2,
  SER_MC_INT8      = 3,
  SER_MC_INT16     = 4,
  SER_MC_INT32     = 5,
  SER_MC_INT64     = 6,
  SER_MC_DOUBLE    = 7,
  SER_MC_STRING    = 8,
  SER_MC_OBJECT    = 9,
  SER_MC_CONTAINER = 10,
  SER_MC_CLS       = 11,
  SER_MC_FUNC      = 12,
  SER_MC_CLSMETH   = 13,
  SER_MC_RFUNC     = 14,
  SER_MC_RCLSMETH  = 15,
  SER_MC_STOP      = 16,
};

const uint64_t kCodeMask DEBUG_ONLY = 0x1f;
const uint64_t kCodePrefix          = 0xe0;

ALWAYS_INLINE void serialize_memoize_code(StringBuffer& sb,
                                          SerializeMemoizeCode code) {
  assertx(code == (code & kCodeMask));
  uint8_t v = (kCodePrefix | code);
  sb.append(reinterpret_cast<char*>(&v), 1);
}

ALWAYS_INLINE void serialize_memoize_int64(StringBuffer& sb, int64_t val) {
  if (val == (int8_t)val) {
    serialize_memoize_code(sb, SER_MC_INT8);
    uint8_t nval = val;
    sb.append(reinterpret_cast<char*>(&nval), 1);
  } else if (val == (int16_t)val) {
    serialize_memoize_code(sb, SER_MC_INT16);
    uint16_t nval = val;
    sb.append(reinterpret_cast<char*>(&nval), 2);
  } else if (val == (int32_t)val) {
    serialize_memoize_code(sb, SER_MC_INT32);
    uint32_t nval = val;
    sb.append(reinterpret_cast<char*>(&nval), 4);
  } else {
    serialize_memoize_code(sb, SER_MC_INT64);
    uint64_t nval = val;
    sb.append(reinterpret_cast<char*>(&nval), 8);
  }
}

} // namespace

void serialize_memoize_string_data(StringBuffer& sb, const StringData* str) {
  int len = str->size();
  serialize_memoize_int64(sb, len);
  sb.append(str->data(), len);
}

void serialize_memoize_tv(StringBuffer& sb, int depth, TypedValue tv);

void serialize_memoize_tv(StringBuffer& sb, int depth, const TypedValue *tv) {
  serialize_memoize_tv(sb, depth, *tv);
}

namespace {

ALWAYS_INLINE void serialize_memoize_arraykey(StringBuffer& sb,
                                              const TypedValue& c) {
  switch (c.m_type) {
    case KindOfPersistentString:
    case KindOfString:
      serialize_memoize_code(sb, SER_MC_STRING);
      serialize_memoize_string_data(sb, c.m_data.pstr);
      break;
    case KindOfInt64:
      serialize_memoize_int64(sb, c.m_data.num);
      break;
    default:
      always_assert(false);
  }
}

void serialize_memoize_array(StringBuffer& sb, int depth, const ArrayData* ad) {
  serialize_memoize_code(sb, SER_MC_CONTAINER);
  IterateKV(ad, [&] (TypedValue k, TypedValue v) {
    serialize_memoize_arraykey(sb, k);
    serialize_memoize_tv(sb, depth, v);
  });
  serialize_memoize_code(sb, SER_MC_STOP);
}

void serialize_memoize_set(StringBuffer& sb, const ArrayData* ad) {
  serialize_memoize_code(sb, SER_MC_CONTAINER);
  IterateKV(ad, [&] (TypedValue k, TypedValue) {
    serialize_memoize_arraykey(sb, k);
    serialize_memoize_arraykey(sb, k);
  });
  serialize_memoize_code(sb, SER_MC_STOP);
}

ALWAYS_INLINE
void serialize_memoize_col(StringBuffer& sb, int depth, ObjectData* obj) {
  assertx(obj->isCollection());
  auto const ad = collections::asArray(obj);
  if (LIKELY(ad != nullptr)) {
    if (UNLIKELY(isSetCollection(obj->collectionType()))) {
      serialize_memoize_set(sb, ad);
    } else {
      serialize_memoize_array(sb, depth, ad);
    }
  } else {
    assertx(obj->collectionType() == CollectionType::Pair);
    auto const pair = reinterpret_cast<const c_Pair*>(obj);
    serialize_memoize_code(sb, SER_MC_CONTAINER);
    serialize_memoize_int64(sb, 0);
    serialize_memoize_tv(sb, depth, pair->get(0));
    serialize_memoize_int64(sb, 1);
    serialize_memoize_tv(sb, depth, pair->get(1));
    serialize_memoize_code(sb, SER_MC_STOP);
  }
}

void serialize_memoize_obj(StringBuffer& sb, int depth, ObjectData* obj) {
  if (obj->isCollection()) {
    serialize_memoize_col(sb, depth, obj);
  } else if (obj->instanceof(s_IMemoizeParam)) {
    Variant ser = obj->o_invoke_few_args(s_getInstanceKey, RuntimeCoeffects::fixme(), 0);
    serialize_memoize_code(sb, SER_MC_OBJECT);
    serialize_memoize_string_data(sb, ser.toString().get());
  } else {
    auto msg = folly::format(
      "Cannot serialize object of type {} because it does not implement "
        "HH\\IMemoizeParam",
      obj->getClassName().asString()
    ).str();
    SystemLib::throwInvalidArgumentExceptionObject(msg);
  }
}

void serialize_memoize_rfunc(StringBuffer& sb, int depth, RFuncData* rfunc) {
  serialize_memoize_code(sb, SER_MC_RFUNC);
  serialize_memoize_string_data(sb, rfunc->m_func->fullName());
  serialize_memoize_array(sb, depth, rfunc->m_arr);
}

void serialize_memoize_rcls_meth(StringBuffer& sb, int depth,
                                 RClsMethData* rclsmeth) {
  serialize_memoize_code(sb, SER_MC_RCLSMETH);
  serialize_memoize_string_data(sb, rclsmeth->m_cls->name());
  serialize_memoize_string_data(sb, rclsmeth->m_func->fullName());
  serialize_memoize_array(sb, depth, rclsmeth->m_arr);
}

} // namespace

void serialize_memoize_tv(StringBuffer& sb, int depth, TypedValue tv) {
  if (depth > 256) {
    SystemLib::throwInvalidArgumentExceptionObject("Array depth exceeded");
  }
  depth++;

  switch (tv.m_type) {
    case KindOfUninit:
    case KindOfNull:
      serialize_memoize_code(sb, SER_MC_NULL);
      break;

    case KindOfBoolean:
      serialize_memoize_code(sb, tv.m_data.num ? SER_MC_TRUE : SER_MC_FALSE);
      break;

    case KindOfInt64:
      serialize_memoize_int64(sb, tv.m_data.num);
      break;

    case KindOfDouble:
      serialize_memoize_code(sb, SER_MC_DOUBLE);
      sb.append(reinterpret_cast<const char*>(&tv.m_data.dbl), 8);
      break;

    case KindOfFunc:
      serialize_memoize_code(sb, SER_MC_FUNC);
      serialize_memoize_string_data(sb, tv.m_data.pfunc->fullName());
      break;

    case KindOfClass:
      serialize_memoize_code(sb, SER_MC_STRING);
      serialize_memoize_string_data(
        sb, classToMemoKeyHelper(tv.m_data.pclass));
      break;

    case KindOfLazyClass:
      serialize_memoize_code(sb, SER_MC_STRING);
      serialize_memoize_string_data(
        sb, lazyClassToMemoKeyHelper(tv.m_data.plazyclass));
      break;

    case KindOfPersistentString:
    case KindOfString:
      serialize_memoize_code(sb, SER_MC_STRING);
      serialize_memoize_string_data(sb, tv.m_data.pstr);
      break;

    case KindOfPersistentKeyset:
    case KindOfKeyset:
      serialize_memoize_set(sb, tv.m_data.parr);
      break;

    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
      serialize_memoize_array(sb, depth, tv.m_data.parr);
      break;

    case KindOfClsMeth:
      serialize_memoize_code(sb, SER_MC_CLSMETH);
      serialize_memoize_string_data(sb, tv.m_data.pclsmeth->getCls()->name());
      serialize_memoize_string_data(
        sb, tv.m_data.pclsmeth->getFunc()->fullName());
      break;
    case KindOfObject:
      serialize_memoize_obj(sb, depth, tv.m_data.pobj);
      break;

    case KindOfRFunc:
      serialize_memoize_rfunc(sb, depth, tv.m_data.prfunc);
      break;

    case KindOfRClsMeth:
      serialize_memoize_rcls_meth(sb, depth, tv.m_data.prclsmeth);
      break;

    case KindOfResource: {
      auto msg = folly::format(
        "Cannot Serialize unexpected type {}",
        tname(tv.m_type)
      ).str();
      SystemLib::throwInvalidArgumentExceptionObject(msg);
      break;
    }
  }
}

namespace {

ALWAYS_INLINE TypedValue serialize_memoize_string_top(StringData* str) {
  if (str->empty()) {
    return make_tv<KindOfPersistentString>(staticEmptyString());
  } else if ((unsigned char)str->data()[0] < kCodePrefix) {
    // serialize_memoize_string_data always returns a string with the first
    // character >= kCodePrefix, so anything less than that can't collide.
    // There's no worry about int-like strings because we won't perform key
    // coercion.
    str->incRefCount();
    return make_tv<KindOfString>(str);
  }

  StringBuffer sb;
  serialize_memoize_code(sb, SER_MC_STRING);
  serialize_memoize_string_data(sb, str);
  return make_tv<KindOfString>(sb.detach().detach());
}

} // end anonymous namespace

TypedValue serialize_memoize_param_arr(ArrayData* arr) {
  CoeffectsAutoGuard _;
  StringBuffer sb;
  serialize_memoize_array(sb, 0, arr);
  return tvReturn(sb.detach());
}

TypedValue serialize_memoize_param_set(ArrayData* arr) {
  CoeffectsAutoGuard _;
  StringBuffer sb;
  serialize_memoize_set(sb, arr);
  return tvReturn(sb.detach());
}

TypedValue serialize_memoize_param_obj(ObjectData* obj) {
  CoeffectsAutoGuard _;
  StringBuffer sb;
  serialize_memoize_obj(sb, 0, obj);
  return tvReturn(sb.detach());
}

TypedValue serialize_memoize_param_col(ObjectData* obj) {
  CoeffectsAutoGuard _;
  StringBuffer sb;
  serialize_memoize_col(sb, 0, obj);
  return tvReturn(sb.detach());
}

TypedValue serialize_memoize_param_str(StringData* str) {
  return serialize_memoize_string_top(str);
}

TypedValue serialize_memoize_param_lazycls(LazyClassData lcls) {
  return serialize_memoize_string_top(
    const_cast<StringData*>(lazyClassToMemoKeyHelper(lcls)));
}

TypedValue serialize_memoize_param_dbl(double val) {
  StringBuffer sb;
  serialize_memoize_code(sb, SER_MC_DOUBLE);
  sb.append(reinterpret_cast<const char*>(&val), 8);
  return tvReturn(sb.detach());
}

TypedValue HHVM_FUNCTION(serialize_memoize_param, TypedValue param) {
  // Memoize throws in the emitter if any function parameters are references, so
  // we can just assert that the param is cell here
  CoeffectsAutoGuard _;
  assertx(tvIsPlausible(param));
  auto const type = param.m_type;

  if (type == KindOfInt64) {
    return param;
  } else if (isStringType(type)) {
    return serialize_memoize_string_top(param.m_data.pstr);
  } else if (type == KindOfUninit || type == KindOfNull) {
    assertx((uint8_t)s_nullMemoKey.data()[0] == (kCodePrefix | SER_MC_NULL));
    return make_tv<KindOfPersistentString>(s_nullMemoKey.get());
  } else if (isLazyClassType(type)) {
    return serialize_memoize_string_top(
      const_cast<StringData*>(
        lazyClassToMemoKeyHelper(param.m_data.plazyclass)));
  } else if (type == KindOfBoolean) {
    assertx((uint8_t)s_trueMemoKey.data()[0] == (kCodePrefix | SER_MC_TRUE));
    assertx((uint8_t)s_falseMemoKey.data()[0] == (kCodePrefix | SER_MC_FALSE));
    return make_tv<KindOfPersistentString>(
      param.m_data.num ? s_trueMemoKey.get() : s_falseMemoKey.get()
    );
  }

  StringBuffer sb;
  serialize_memoize_tv(sb, 0, &param);
  return tvReturn(sb.detach());
}

namespace {

void clearValueLink(rds::Link<TypedValue, rds::Mode::Normal> valLink) {
  if (valLink.bound() && valLink.isInit()) {
    auto oldVal = *valLink;
    valLink.markUninit();
    tvDecRefGen(oldVal);
  }
}

void clearCacheLink(rds::Link<MemoCacheBase*, rds::Mode::Normal> cacheLink) {
    if (cacheLink.bound() && cacheLink.isInit()) {
      auto oldCache = *cacheLink;
      cacheLink.markUninit();
      if (oldCache) req::destroy_raw(oldCache);
    }
}

} // end anonymous namespace

bool HHVM_FUNCTION(clear_static_memoization,
                   TypedValue clsStr, TypedValue funcStr) {
  auto clear = [] (const Func* func) {
    if (!func->isMemoizeWrapper()) return false;
    clearValueLink(rds::attachStaticMemoValue(func));
    clearCacheLink(rds::attachStaticMemoCache(func));
    rds::clearConstMemoCache(func, nullptr);
    return true;
  };

  if (isStringType(clsStr.m_type)) {
    auto const cls = Class::load(clsStr.m_data.pstr);
    if (!cls) return false;
    if (isStringType(funcStr.m_type)) {
      auto const func = cls->lookupMethod(funcStr.m_data.pstr);
      return func && func->isStatic() && clear(func);
    }
    auto ret = false;
    for (auto i = cls->numMethods(); i--; ) {
      auto const func = cls->getMethod(i);
      if (func->isStatic()) {
        if (clear(func)) ret = true;
      }
    }
    return ret;
  }

  if (isStringType(funcStr.m_type)) {
    auto const func = Func::load(funcStr.m_data.pstr);
    return func && clear(func);
  }

  return false;
}

String HHVM_FUNCTION(ffp_parse_string_native, const String& str) {
  auto const file = fromCaller(
    [] (const BTFrame& frm) { return frm.func()->unit()->filepath()->data(); }
  );

  auto result = ffp_parse_file(
    str.get()->toCppString(),
    RepoOptions::forFile(file).flags()
  );

  FfpJSONString res;
  match<void>(
    result,
    [&](FfpJSONString& r) {
      res = std::move(r);
    },
    [&](std::string& err) {
      SystemLib::throwInvalidArgumentExceptionObject(
        "FFP failed to parse string");
    }
  );
  return res.value;
}

bool HHVM_FUNCTION(clear_lsb_memoization,
                   const String& clsStr, TypedValue funcStr) {
  auto const clear = [](const Class* cls, const Func* func) {
    if (!func->isStatic()) return false;
    if (!func->isMemoizeWrapperLSB()) return false;
    clearValueLink(rds::attachLSBMemoValue(cls, func));
    clearCacheLink(rds::attachLSBMemoCache(cls, func));
    rds::clearConstMemoCache(func, cls);
    return true;
  };

  auto const cls = Class::load(clsStr.get());
  if (!cls) return false;

  if (isStringType(funcStr.m_type)) {
    auto const func = cls->lookupMethod(funcStr.m_data.pstr);
    return func && clear(cls, func);
  }

  auto ret = false;
  for (auto i = cls->numMethods(); i--; ) {
    auto const func = cls->getMethod(i);
    if (clear(cls, func)) ret = true;
  }
  return ret;
}

bool HHVM_FUNCTION(clear_instance_memoization, const Object& obj) {
  auto const cls = obj->getVMClass();
  if (!cls->hasMemoSlots()) return false;

  if (!obj->getAttribute(ObjectData::UsedMemoCache)) return true;

  auto const nSlots = cls->numMemoSlots();
  for (Slot i = 0; i < nSlots; ++i) {
    auto slot = UNLIKELY(obj->hasNativeData())
      ? obj->memoSlotNativeData(i, cls->getNativeDataInfo()->sz)
      : obj->memoSlot(i);
    if (slot->isCache()) {
      if (auto cache = slot->getCache()) {
        slot->resetCache();
        req::destroy_raw(cache);
      }
    } else {
      auto const oldVal = *slot->getValue();
      tvWriteUninit(*slot->getValue());
      tvDecRefGen(oldVal);
    }
  }

  return true;
}

void HHVM_FUNCTION(set_frame_metadata, const Variant&) {
  SystemLib::throwInvalidArgumentExceptionObject(
    "Unsupported dynamic call of set_frame_metadata()");
}

namespace {

ArrayData* from_stats(rqtrace::EventStats stats) {
  return make_dict_array(
    "duration", stats.total_duration, "count", stats.total_count).detach();
}

template<class T>
ArrayData* from_stats_list(T stats) {
  DictInit init(stats.size());
  for (auto& pair : stats) {
    init.set(String(pair.first), Array::attach(from_stats(pair.second)));
  }
  return init.create();
}

}

bool HHVM_FUNCTION(is_enabled) {
  return g_context->getRequestTrace() != nullptr;
}

void HHVM_FUNCTION(force_enable) {
  if (g_context->getRequestTrace()) return;
  if (auto const transport = g_context->getTransport()) {
    transport->forceInitRequestTrace();
    g_context->setRequestTrace(transport->getRequestTrace());
  }
}

TypedValue HHVM_FUNCTION(all_request_stats) {
  if (auto const trace = g_context->getRequestTrace()) {
    return tvReturn(from_stats_list(trace->stats()));
  }
  return tvReturn(ArrayData::CreateDict());
}

TypedValue HHVM_FUNCTION(all_process_stats) {
  req::vector<std::pair<StringData*, rqtrace::EventStats>> stats;

  rqtrace::visit_process_stats(
    [&] (const StringData* name, rqtrace::EventStats s) {
      stats.emplace_back(const_cast<StringData*>(name), s);
    }
  );

  return tvReturn(from_stats_list(stats));
}

TypedValue HHVM_FUNCTION(request_event_stats, StringArg event) {
  if (auto const trace = g_context->getRequestTrace()) {
    auto const stats = folly::get_default(trace->stats(), event->data());
    return tvReturn(from_stats(stats));
  }
  return tvReturn(from_stats({}));
}

TypedValue HHVM_FUNCTION(process_event_stats, StringArg event) {
  return tvReturn(from_stats(rqtrace::process_stats_for(event->data())));
}

int64_t HHVM_FUNCTION(get_request_count) {
  return requestCount();
}

Array HHVM_FUNCTION(get_compiled_units, int64_t kind) {
  auto const& units = g_context.getNoCheck()->m_evaledFiles;
  KeysetInit init{units.size()};
  for (auto& u : units) {
    switch (u.second.flags) {
    case FileLoadFlags::kDup:     break;
    case FileLoadFlags::kHitMem:  break;
    case FileLoadFlags::kWaited:  if (kind < 2) break;
    case FileLoadFlags::kHitDisk: if (kind < 1) break;
    case FileLoadFlags::kCompiled:if (kind < 0) break;
    case FileLoadFlags::kEvicted:
      init.add(const_cast<StringData*>(u.first));
    }
  }
  return init.toArray();
}

void HHVM_FUNCTION(prefetch_units, const Array& paths, bool hint) {
  if (!paths.isKeyset()) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Paths must be a keyset"
    );
  }

  IterateV(
    paths.get(),
    [] (TypedValue v) {
      if (!isStringType(v.m_type)) {
        SystemLib::throwInvalidArgumentExceptionObject(
          "Paths must contain only strings"
        );
      }
    }
  );

  if (RO::RepoAuthoritative || !unitPrefetchingEnabled()) {
    // Unit prefetching isn't enabled. If the caller has specified
    // that this is a hint, just do nothing (since this is advisory).
    if (hint) return;

    // Otherwise this isn't a hint, and the caller wants us to
    // definitely load these units. Do it inline.
    IterateV(
      paths.get(),
      [&] (TypedValue v) {
        assertx(isStringType(v.m_type));
        lookupUnit(
          File::TranslatePath(String{v.m_data.pstr}).get(),
          "",
          nullptr,
          Native::s_noNativeFuncs,
          false,
          true
        );
      }
    );
    return;
  }

  // Unit prefetching is enabled, so we can use that mechanism (which
  // will be faster usually). If this is a hint, we'll just fire off
  // the requests and let them finish asynchronously. If not, we
  // cannot return until they're done, so use the prefetcher's gate
  // mechanism.
  auto gate = hint
    ? nullptr
    : std::make_shared<folly::atomic_uint_fast_wait_t>(0);

  IterateV(
    paths.get(),
    [&] (TypedValue v) {
      assertx(isStringType(v.m_type));
      prefetchUnit(makeStaticString(v.m_data.pstr), gate, nullptr);
    }
  );

  // If we passed in a gate, wait for it to signal completion of all
  // the requests.
  if (!gate) return;

  while (true) {
    auto const c = gate->load();
    if (c == 0) break;
    folly::atomic_wait(gate.get(), c);
  }
}

namespace {

enum class DynamicAttr : int8_t { Ignore, Require };

template <DynamicAttr DA = DynamicAttr::Require>
TypedValue dynamicFun(const StringData* fun) {
  auto const func = Func::load(fun);
  if (!func) {
    SystemLib::throwInvalidArgumentExceptionObject(
      folly::sformat("Unable to find function {}", fun->data())
    );
  }
  if (func->hasReifiedGenerics()) {
    if (func->getReifiedGenericsInfo().allGenericsSoft()) {
      raise_warning(
        "Function %s is reified and cannot be used with dynamic_fun",
        fun->data()
      );
    } else {
      SystemLib::throwInvalidArgumentExceptionObject(
        folly::sformat("Function {} is reified", fun->data())
      );
    }
  }
  if (!func->isDynamicallyCallable() && DA == DynamicAttr::Require) {
    auto const level = RuntimeOption::EvalDynamicFunLevel;
    if (level == 2) {
      SystemLib::throwInvalidArgumentExceptionObject(
        folly::sformat("Function {} not marked dynamic", fun->data())
      );
    }
    if (level == 1) {
      raise_warning("Function %s not marked dynamic", fun->data());
    }
  }
  return tvReturn(func);
}

template <DynamicAttr DA = DynamicAttr::Require, bool checkVis = true>
TypedValue dynamicClassMeth(const StringData* cls, const StringData* meth) {
  auto const c = Class::load(cls);
  if (!c) {
    SystemLib::throwInvalidArgumentExceptionObject(
      folly::sformat("Unable to find class {}", cls->data())
    );
  }
  auto const func = c->lookupMethod(meth);
  if (!func) {
    SystemLib::throwInvalidArgumentExceptionObject(
      folly::sformat("Unable to find method {}::{}",
                     cls->data(), meth->data())
    );
  }
  if (!func->isStaticInPrologue()) {
    SystemLib::throwInvalidArgumentExceptionObject(
      folly::sformat("Method {}::{} is not static",
                     cls->data(), meth->data())
    );
  }
  if (func->isAbstract()) {
    SystemLib::throwInvalidArgumentExceptionObject(
      folly::sformat("Method {}::{} is abstract",
                     cls->data(), meth->data())
    );
  }
  if (!func->isPublic() && checkVis) {
    auto const ctx = fromCaller(
      [] (const BTFrame& frm) { return frm.func()->cls(); }
    );
    if (func->attrs() & AttrPrivate) {
      auto const fcls = func->cls();
      if (fcls != ctx) {
        SystemLib::throwInvalidArgumentExceptionObject(
          folly::sformat(fcls == c ? "Method {}::{} is marked Private"
                                   : "Unable to find method {}::{}",
                         cls->data(), meth->data())
        );
      }
    } else if (func->attrs() & AttrProtected) {
      auto const fcls = func->cls();
      if (!ctx || (!ctx->classof(fcls) && !fcls->classof(ctx))) {
        SystemLib::throwInvalidArgumentExceptionObject(
          folly::sformat("Method {}::{} is marked Protected",
                         cls->data(), meth->data())
        );
      }
    }
  }
  if (func->hasReifiedGenerics()) {
    if (func->getReifiedGenericsInfo().allGenericsSoft()) {
      raise_warning(
        "Method %s::%s is reified and cannot be used with dynamic_class_meth",
        cls->data(),
        meth->data()
      );
    } else {
      SystemLib::throwInvalidArgumentExceptionObject(
        folly::sformat("Method {}::{} is reified", cls->data(), meth->data())
      );
    }
  }
  if (!func->isDynamicallyCallable() && DA == DynamicAttr::Require) {
    auto const level = RuntimeOption::EvalDynamicClsMethLevel;
    if (level == 2) {
      SystemLib::throwInvalidArgumentExceptionObject(
        folly::sformat("Method {}::{} not marked dynamic",
                       cls->data(), meth->data())
      );
    }
    if (level == 1) {
      raise_warning("Method %s::%s not marked dynamic",
                    cls->data(), meth->data());
    }
  }
  return tvReturn(ClsMethDataRef::create(c, func));
}

} // end anonymous namespace

TypedValue HHVM_FUNCTION(dynamic_fun, StringArg fun) {
  return dynamicFun(fun.get());
}

TypedValue HHVM_FUNCTION(dynamic_fun_force, StringArg fun) {
  if (RuntimeOption::RepoAuthoritative) {
    raise_error("You can't use %s() in RepoAuthoritative mode", __FUNCTION__+2);
  }
  return dynamicFun<DynamicAttr::Ignore>(fun.get());
}

TypedValue HHVM_FUNCTION(dynamic_class_meth, StringArg cls, StringArg meth) {
  return dynamicClassMeth(cls.get(), meth.get());
}

TypedValue HHVM_FUNCTION(dynamic_class_meth_force, StringArg cls,
                         StringArg meth) {
  if (RuntimeOption::RepoAuthoritative) {
    raise_error("You can't use %s() in RepoAuthoritative mode", __FUNCTION__+2);
  }
  return dynamicClassMeth<DynamicAttr::Ignore, false>(cls.get(), meth.get());
}

TypedValue HHVM_FUNCTION(classname_from_string_unsafe, StringArg cls) {
  return make_tv<KindOfLazyClass>(LazyClassData::create(cls.get()));
}

namespace {

const StaticString
  s_no_repo_mode("Cannot enable code coverage in Repo.Authoritative mode"),
  s_no_flag_set("Must set Eval.EnablePerFileCoverage");

void check_coverage_flags() {
  if (RO::RepoAuthoritative) {
    throw_invalid_operation_exception(s_no_repo_mode.get());
  }
  if (!RO::EvalEnablePerFileCoverage) {
    throw_invalid_operation_exception(s_no_flag_set.get());
  }
}

Unit* loadUnit(StringData* path) {
  auto const unit = lookupUnit(
    File::TranslatePath(String{path}).get(),
    "",
    nullptr,
    Native::s_noNativeFuncs,
    false
  );
  if (!unit) {
    SystemLib::throwInvalidArgumentExceptionObject(
      folly::sformat("Unable to open file: {}", path->data())
    );
  }
  return unit;
}

std::vector<Unit*> loadUnits(ArrayData* files) {
  std::vector<Unit*> units;
  IterateV(
    files,
    [&] (TypedValue v) {
      if (!isStringType(v.m_type)) {
        assertx(isIntType(v.m_type));
        SystemLib::throwInvalidArgumentExceptionObject(
          folly::sformat("Invalid filepath: {}", v.m_data.num)
        );
      }
      units.push_back(loadUnit(v.m_data.pstr));
    }
  );
  return units;
}

}

void HHVM_FUNCTION(enable_per_file_coverage, ArrayArg files) {
  check_coverage_flags();
  for (auto const u : loadUnits(files.get())) u->enableCoverage();
}

void HHVM_FUNCTION(disable_per_file_coverage, ArrayArg files) {
  check_coverage_flags();
  for (auto const u : loadUnits(files.get())) u->disableCoverage();
}

Array HHVM_FUNCTION(get_files_with_coverage) {
  check_coverage_flags();
  std::vector<const StringData*> units;
  for (auto const& p : g_context->m_evaledFiles) {
    if (p.second.unit->isCoverageEnabled()) {
      units.push_back(p.second.unit->filepath());
    }
  }
  if (units.empty()) return Array::CreateKeyset();
  KeysetInit k{units.size()};
  for (auto s : units) k.add(String{const_cast<StringData*>(s)});
  return k.toArray();
}

Array HHVM_FUNCTION(get_coverage_for_file, StringArg file) {
  check_coverage_flags();
  auto const u = loadUnit(file.get());
  if (!u->isCoverageEnabled()) {
    SystemLib::throwInvalidOperationExceptionObject(
      folly::sformat("Coverage not enabled for file: {}", file->data())
    );
  }
  return u->reportCoverage();
}

void HHVM_FUNCTION(clear_coverage_for_file, StringArg file) {
  check_coverage_flags();
  auto const u = loadUnit(file.get());
  if (!u->isCoverageEnabled()) {
    SystemLib::throwInvalidOperationExceptionObject(
      folly::sformat("Coverage not enabled for file: {}", file->data())
    );
  }
  u->clearCoverage();
}

namespace {

bespoke::TypeStructure* getBespokeTS(const Array& ts) {
  return bespoke::TypeStructure::isBespokeTypeStructure(ts.get())
    ? bespoke::TypeStructure::As(ts.get())
    : nullptr;
}
bool getBool(const Array& ts, const String& key) {
  auto const tv = ts.get()->get(key.get());
  if (isNullType(tv.m_type)) return false;
  assertx(isBoolType(tv.m_type));
  return tv.m_data.num;
}
String getString(const Array& ts, const String& key) {
  auto const tv = ts.get()->get(key.get());
  if (isNullType(tv.m_type)) return String{};
  assertx(isStringType(tv.m_type));
  return String{tv.m_data.pstr};
}
Array getArray(const Array& ts, const String& key) {
  auto const tv = ts.get()->get(key.get());
  if (isNullType(tv.m_type)) return Array{};
  assertx(isArrayLikeType(tv.m_type));
  return Array{tv.m_data.parr};
}

} // namespace

int64_t HHVM_FUNCTION(get_kind, const Array& ts) {
  if (auto const bespokeTS = getBespokeTS(ts)) {
    return bespokeTS->kind();
  }
  auto const tv = ts.get()->get(s_kind.get());
  assertx(isIntType(tv.m_type));
  return tv.m_data.num;
}

bool HHVM_FUNCTION(get_nullable, const Array& ts) {
  if (auto const bespokeTS = getBespokeTS(ts)) {
    return bespokeTS->nullable();
  }
  return getBool(ts, s_nullable);
}

bool HHVM_FUNCTION(get_soft, const Array& ts) {
  if (auto const bespokeTS = getBespokeTS(ts)) {
    return bespokeTS->soft();
  }
  return getBool(ts, s_soft);
}

bool HHVM_FUNCTION(get_like, const Array& ts) {
  if (auto const bespokeTS = getBespokeTS(ts)) {
    return bespokeTS->like();
  }
  return getBool(ts, s_like);
}

bool HHVM_FUNCTION(get_opaque, const Array& ts) {
  if (auto const bespokeTS = getBespokeTS(ts)) {
    return bespokeTS->opaque();
  }
  return getBool(ts, s_opaque);
}

bool HHVM_FUNCTION(get_optional_shape_field, const Array& ts) {
  if (auto const bespokeTS = getBespokeTS(ts)) {
    return bespokeTS->optionalShapeField();
  }
  return getBool(ts, s_optional_shape_field);
}

String HHVM_FUNCTION(get_alias, const Array& ts) {
  if (auto const bespokeTS = getBespokeTS(ts)) {
    return String{bespokeTS->alias()};
  }
  return getString(ts, s_alias);
}

String HHVM_FUNCTION(get_typevars, const Array& ts) {
  if (auto const bespokeTS = getBespokeTS(ts)) {
    return String{bespokeTS->typevars()};
  }
  return getString(ts, s_typevars);
}

Array HHVM_FUNCTION(get_typevar_types, const Array& ts) {
  if (auto const bespokeTS = getBespokeTS(ts)) {
    return Array{bespokeTS->typevarTypes()};
  }
  return getArray(ts, s_typevar_types);
}

Array HHVM_FUNCTION(get_fields, const Array& ts) {
  if (auto bespokeTS = getBespokeTS(ts)) {
    if (bespokeTS->typeKind() != TypeStructure::Kind::T_shape) {
      return Array{};
    }
    auto const s = reinterpret_cast<bespoke::TSShape*>(bespokeTS);
    return Array{s->fields()};
  }
  return getArray(ts, s_fields);
}

bool HHVM_FUNCTION(get_allows_unknown_fields, const Array& ts) {
  if (auto bespokeTS = getBespokeTS(ts)) {
    if (bespokeTS->typeKind() != TypeStructure::Kind::T_shape) {
      return false;
    }
    auto const s = reinterpret_cast<bespoke::TSShape*>(bespokeTS);
    return s->allowsUnknownFields();
  }
  return getBool(ts, s_allows_unknown_fields);
}

Array HHVM_FUNCTION(get_elem_types, const Array& ts) {
  if (auto bespokeTS = getBespokeTS(ts)) {
    if (bespokeTS->typeKind() != TypeStructure::Kind::T_tuple) {
      return Array{};
    }
    auto const s = reinterpret_cast<bespoke::TSTuple*>(bespokeTS);
    return Array{s->elemTypes()};
  }
  return getArray(ts, s_elem_types);
}

Array HHVM_FUNCTION(get_param_types, const Array& ts) {
  if (auto bespokeTS = getBespokeTS(ts)) {
    if (bespokeTS->typeKind() != TypeStructure::Kind::T_fun) {
      return Array{};
    }
    auto const s = reinterpret_cast<bespoke::TSFun*>(bespokeTS);
    return Array{s->paramTypes()};
  }
  return getArray(ts, s_param_types);
}

Array HHVM_FUNCTION(get_return_type, const Array& ts) {
  if (auto bespokeTS = getBespokeTS(ts)) {
    if (bespokeTS->typeKind() != TypeStructure::Kind::T_fun) {
      return Array{};
    }
    auto const s = reinterpret_cast<bespoke::TSFun*>(bespokeTS);
    return Array{s->returnType()};
  }
  return getArray(ts, s_return_type);
}

Array HHVM_FUNCTION(get_variadic_type, const Array& ts) {
  if (auto bespokeTS = getBespokeTS(ts)) {
    if (bespokeTS->typeKind() != TypeStructure::Kind::T_fun) {
      return Array{};
    }
    auto const s = reinterpret_cast<bespoke::TSFun*>(bespokeTS);
    return Array{s->variadicType()};
  }
  return getArray(ts, s_variadic_type);
}

String HHVM_FUNCTION(get_name, const Array& ts) {
  if (auto bespokeTS = getBespokeTS(ts)) {
    if (bespokeTS->typeKind() != TypeStructure::Kind::T_typevar) {
      return String{};
    }
    auto const s = reinterpret_cast<bespoke::TSTypevar*>(bespokeTS);
    return String{s->name()};
  }
  return getString(ts, s_name);
}

Array HHVM_FUNCTION(get_generic_types, const Array& ts) {
  if (auto bespokeTS = getBespokeTS(ts)) {
    if (bespokeTS->fieldsByte() != bespoke::TSWithGenericTypes::kFieldsByte &&
        bespokeTS->fieldsByte() != bespoke::TSWithClassishTypes::kFieldsByte) {
      return Array{};
    }
    auto const s = reinterpret_cast<bespoke::TSWithGenericTypes*>(bespokeTS);
    return Array{s->genericTypes()};
  }
  return getArray(ts, s_generic_types);
}

String HHVM_FUNCTION(get_classname, const Array& ts) {
  if (auto bespokeTS = getBespokeTS(ts)) {
    if (bespokeTS->fieldsByte() != bespoke::TSWithClassishTypes::kFieldsByte) {
      return String{};
    }
    auto const s = reinterpret_cast<bespoke::TSWithClassishTypes*>(bespokeTS);
    return String{s->classname()};
  }
  return getString(ts, s_classname);
}

bool HHVM_FUNCTION(get_exact, const Array& ts) {
  if (auto bespokeTS = getBespokeTS(ts)) {
    if (bespokeTS->fieldsByte() != bespoke::TSWithClassishTypes::kFieldsByte) {
      return false;
    }
    auto const s = reinterpret_cast<bespoke::TSWithClassishTypes*>(bespokeTS);
    return s->exact();
  }
  return getBool(ts, s_exact);
}

namespace {
bool is_dynamically_callable_inst_method_impl(const StringData* cls,
                                              const StringData* meth) {
  if (auto const c = Class::load(cls)) {
    if (auto const m = c->lookupMethod(meth)) {
      return !m->isStatic() && m->isDynamicallyCallable();
    }
  }
  return false;
}
} // namespace

bool HHVM_FUNCTION(is_dynamically_callable_inst_method, StringArg cls,
                                                        StringArg meth) {
  return is_dynamically_callable_inst_method_impl(cls.get(), meth.get());
}

void HHVM_FUNCTION(check_dynamically_callable_inst_method, StringArg cls,
                                                           StringArg meth) {
  if (is_dynamically_callable_inst_method_impl(cls.get(), meth.get())) return;
  if (RO::EvalDynamicMethCallerLevel == 0) return;
  auto const msg = folly::sformat(
    "dynamic_meth_caller(): {}::{} is not a dynamically "
    "callable instance method",
    cls.get(), meth.get());
  if (RO::EvalDynamicMethCallerLevel == 1) {
    raise_warning(msg);
    return;
  }
  SystemLib::throwInvalidArgumentExceptionObject(msg);
}

namespace {

Class* getClass(TypedValue cls) {
  switch (cls.m_type) {
    case KindOfPersistentString:
    case KindOfString:
      return Class::load(cls.m_data.pstr);
    case KindOfClass:
      return cls.m_data.pclass;
    case KindOfLazyClass:
      return Class::load(cls.m_data.plazyclass.name());
    case KindOfObject:
      return cls.m_data.pobj->getVMClass();
    case KindOfUninit:
    case KindOfNull:
    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble:
    case KindOfVec:
    case KindOfPersistentVec:
    case KindOfDict:
    case KindOfPersistentDict:
    case KindOfKeyset:
    case KindOfPersistentKeyset:
    case KindOfResource:
    case KindOfFunc:
    case KindOfRFunc:
    case KindOfClsMeth:
    case KindOfRClsMeth:
      SystemLib::throwInvalidArgumentExceptionObject(
        folly::sformat(
          "Invalid argument type passed to reflection class constructor")
      );
  }
  not_reached();
};

}

bool HHVM_FUNCTION(reflection_class_is_abstract, TypedValue cls) {
  auto const c = getClass(cls);
  if (!c) {
    SystemLib::throwInvalidArgumentExceptionObject(
      folly::sformat("Unable to find class.")
    );
  }
  return isAbstract(c);
}

String HHVM_FUNCTION(reflection_class_get_name, TypedValue classname) {
  auto const cls = getClass(classname);
  if (!cls) {
    SystemLib::throwInvalidArgumentExceptionObject(
      folly::sformat("Unable to find class.")
    );
  }
  return cls->name()->data();
}

bool HHVM_FUNCTION(reflection_class_is_final, TypedValue cls) {
  auto const c = getClass(cls);
  if (!c) {
    SystemLib::throwInvalidArgumentExceptionObject(
      folly::sformat("Unable to find class.")
    );
  }
  return c->attrs() & AttrFinal;
}

bool HHVM_FUNCTION(reflection_class_is_interface, TypedValue cls) {
  auto const c = getClass(cls);
  if (!c) {
    SystemLib::throwInvalidArgumentExceptionObject(
      folly::sformat("Unable to find class.")
    );
  }
  return isInterface(c);
}

TypedValue HHVM_FUNCTION(get_executable_lines, StringArg path) {
  auto const file = lookupUnit(path.get(), "", nullptr, Native::s_noNativeFuncs,
                               false);
  if (!file) {
    SystemLib::throwInvalidArgumentExceptionObject(
      folly::sformat("Unable to find file {}", path.get()->data())
    );
  }

  auto const lines = get_executable_lines(file);
  VecInit vinit{lines.size()};
  for (auto line : lines) vinit.append(make_tv<KindOfInt64>(line));
  return tvReturn(vinit.toVariant());
}

int64_t HHVM_FUNCTION(hphp_get_logger_request_id) {
  return Logger::GetRequestId();
}

void HHVM_FUNCTION(enable_function_coverage) {
  Func::EnableCoverage();
}

Array HHVM_FUNCTION(collect_function_coverage) {
  return Func::GetCoverage();
}

namespace {
const StaticString
  s_uses("uses"),
  s_includes("includes"),
  s_soft_includes("soft_includes"),
  s_packages("packages"),
  s_soft_packages("soft_packages"),
  s_domains("domains");
} // namespace

Array HHVM_FUNCTION(get_all_packages) {
  auto const& packageInfo = g_context->getPackageInfo();
  DictInit result(packageInfo.packages().size());
  for (auto const& [name, p] : packageInfo.packages()) {
    DictInit package(3);

    VecInit uses(p.m_uses.size());
    for (auto& s : p.m_uses) uses.append(String{makeStaticString(s)});
    package.set(s_uses.get(), uses.toVariant());

    VecInit includes(p.m_includes.size());
    for (auto& s : p.m_includes) includes.append(String{makeStaticString(s)});
    package.set(s_includes.get(), includes.toVariant());

    VecInit soft_includes(p.m_soft_includes.size());
    for (auto& s : p.m_soft_includes) soft_includes.append(String{makeStaticString(s)});
    package.set(s_soft_includes.get(), soft_includes.toVariant());
    result.set(makeStaticString(name), package.toVariant());
  }

  return result.toArray();
}

Array HHVM_FUNCTION(get_all_deployments) {
  auto const& packageInfo = g_context->getPackageInfo();
  DictInit result(packageInfo.deployments().size());
  for (auto const& [name, d] : packageInfo.deployments()) {
    DictInit deployment(3);

    VecInit packages(d.m_packages.size());
    for (auto& s : d.m_packages) packages.append(String{makeStaticString(s)});
    deployment.set(s_packages.get(), packages.toVariant());

    VecInit soft_packages(d.m_soft_packages.size());
    for (auto& s : d.m_soft_packages) soft_packages.append(String{makeStaticString(s)});
    deployment.set(s_soft_packages.get(), soft_packages.toVariant());

    VecInit domains(d.m_domains.size());
    for (auto& r : d.m_domains) {
      domains.append(String{makeStaticString(r->pattern())});
    }
    deployment.set(s_domains.get(), domains.toVariant());

    result.set(makeStaticString(name), deployment.toVariant());
  }

  return result.toArray();
}

bool HHVM_FUNCTION(package_exists, StringArg name) {
  assertx(name.get());
  if (name.get()->empty()) return false;
  auto const& packageInfo = g_context->getPackageInfo();
  return packageInfo.isPackageInActiveDeployment(name.get());
}

static struct HHExtension final : Extension {
  HHExtension(): Extension("hh", NO_EXTENSION_VERSION_YET) { }
  void moduleInit() override {
#define X(nm) HHVM_NAMED_FE(HH\\nm, HHVM_FN(nm))
    X(autoload_is_native);
    X(autoload_set_paths);
    X(autoload_get_paths);
    X(autoload_type_to_path);
    X(autoload_function_to_path);
    X(autoload_constant_to_path);
    X(autoload_module_to_path);
    X(autoload_type_alias_to_path);
    X(autoload_path_to_types);
    X(autoload_path_to_functions);
    X(autoload_path_to_constants);
    X(autoload_path_to_modules);
    X(autoload_path_to_type_aliases);
    X(could_include);
    X(serialize_memoize_param);
    X(clear_static_memoization);
    X(ffp_parse_string_native);
    X(clear_lsb_memoization);
    X(clear_instance_memoization);
    X(set_frame_metadata);
    X(get_request_count);
    X(get_compiled_units);
    X(prefetch_units);
    X(dynamic_fun);
    X(dynamic_fun_force);
    X(dynamic_class_meth);
    X(dynamic_class_meth_force);
    X(classname_from_string_unsafe);
    X(enable_per_file_coverage);
    X(disable_per_file_coverage);
    X(get_files_with_coverage);
    X(get_coverage_for_file);
    X(clear_coverage_for_file);
    X(get_executable_lines);
    X(hphp_get_logger_request_id);
    X(enable_function_coverage);
    X(collect_function_coverage);
    X(get_all_packages);
    X(get_all_deployments);
    X(package_exists);
#undef X
#define X(nm) HHVM_NAMED_FE(HH\\rqtrace\\nm, HHVM_FN(nm))
    X(is_enabled);
    X(force_enable);
    X(all_request_stats);
    X(all_process_stats);
    X(request_event_stats);
    X(process_event_stats);
#undef X
#define X(nm) HHVM_NAMED_FE(HH\\TypeStructure\\nm, HHVM_FN(nm))
    X(get_kind);
    X(get_nullable);
    X(get_soft);
    X(get_like);
    X(get_opaque);
    X(get_optional_shape_field);
    X(get_alias);
    X(get_typevars);
    X(get_typevar_types);
    X(get_fields);
    X(get_allows_unknown_fields);
    X(get_elem_types);
    X(get_param_types);
    X(get_return_type);
    X(get_variadic_type);
    X(get_name);
    X(get_generic_types);
    X(get_classname);
    X(get_exact);
#undef X

#define X(n, t) HHVM_RC_INT(HH\\n, static_cast<int64_t>(AutoloadMap::KindOf::t))
    X(AUTOLOAD_MAP_KIND_OF_TYPE, Type);
    X(AUTOLOAD_MAP_KIND_OF_FUNCTION, Function);
    X(AUTOLOAD_MAP_KIND_OF_CONSTANT, Constant);
    X(AUTOLOAD_MAP_KIND_OF_TYPE_ALIAS, TypeAlias);
#undef X

#define X(nm) HHVM_NAMED_FE(__SystemLib\\nm, HHVM_FN(nm))
    X(is_dynamically_callable_inst_method);
    X(check_dynamically_callable_inst_method);
    X(reflection_class_get_name);
    X(reflection_class_is_abstract);
    X(reflection_class_is_final);
    X(reflection_class_is_interface);
#undef X

    loadSystemlib();
  }
} s_hh_extension;

static struct XHPExtension final : Extension {
  XHPExtension(): Extension("xhp", NO_EXTENSION_VERSION_YET) { }
  bool moduleEnabled() const override { return RuntimeOption::EnableXHP; }
} s_xhp_extension;

///////////////////////////////////////////////////////////////////////////////
}
