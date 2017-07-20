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

#include "hphp/runtime/base/autoload-handler.h"
#include "hphp/runtime/base/container-functions.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/ext/fb/ext_fb.h"
#include "hphp/runtime/ext/collections/ext_collections-pair.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/vm-regs.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

const StaticString
  s_86metadata("86metadata"),
  // The following are used in serialize_memoize_tv to serialize objects that
  // implement the IMemoizeParam interface
  s_IMemoizeParam("HH\\IMemoizeParam"),
  s_getInstanceKey("getInstanceKey"),
  // The following are used in serialize_memoize_param(); they are what
  // serialize_memoize_tv would have produced on some common inputs; having
  // these lets us avoid creating a StringBuffer
  s_nullMemoKey("\xf0"),
  s_trueMemoKey("\xf1"),
  s_falseMemoKey("\xf2");

///////////////////////////////////////////////////////////////////////////////
bool HHVM_FUNCTION(autoload_set_paths,
                   const Variant& map,
                   const String& root) {
  if (map.isArray()) {
    return AutoloadHandler::s_instance->setMap(map.toCArrRef(), root);
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

bool HHVM_FUNCTION(could_include, const String& file) {
  return lookupUnit(file.get(), "", nullptr /* initial_opt */) != nullptr;
}

namespace {

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
 * Integer values are returned as-is. Strings where the first byte is < 0xf0
 * are returned as-is. All other values are converted to strings of the form
 * <c> <data> where c is a byte (0xf0 | code), and data is a sequence of 0 or
 * more bytes. The codes are:
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
 * 11 (STOP): terminates a CONTAINER encoding
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
  SER_MC_STOP      = 11,
};

const uint64_t kCodeMask DEBUG_ONLY = 0x0f;
const uint64_t kCodePrefix          = 0xf0;

ALWAYS_INLINE void serialize_memoize_code(StringBuffer& sb,
                                          SerializeMemoizeCode code) {
  assert(code == (code & kCodeMask));
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

ALWAYS_INLINE void serialize_memoize_string_data(StringBuffer& sb,
                                                 const StringData* str) {
  int len = str->size();
  serialize_memoize_int64(sb, len);
  sb.append(str->data(), len);
}

void serialize_memoize_tv(StringBuffer& sb, int depth, TypedValue tv);

void serialize_memoize_tv(StringBuffer& sb, int depth, const TypedValue *tv) {
  serialize_memoize_tv(sb, depth, *tv);
}

void serialize_memoize_array(StringBuffer& sb, int depth, const ArrayData* ad) {
  serialize_memoize_code(sb, SER_MC_CONTAINER);
  IterateKV(ad, [&] (Cell k, TypedValue v) {
    serialize_memoize_tv(sb, depth, k);
    serialize_memoize_tv(sb, depth, v);
    return false;
  });
  serialize_memoize_code(sb, SER_MC_STOP);
}

void serialize_memoize_obj(StringBuffer& sb, int depth, ObjectData* obj) {
  if (obj->isCollection()) {
    const ArrayData* ad = collections::asArray(obj);
    if (ad) {
      serialize_memoize_array(sb, depth, ad);
    } else {
      assertx(obj->collectionType() == CollectionType::Pair);

      auto const pair = reinterpret_cast<c_Pair*>(obj);
      serialize_memoize_code(sb, SER_MC_CONTAINER);
      serialize_memoize_int64(sb, 0);
      serialize_memoize_tv(sb, depth, pair->get(0));
      serialize_memoize_int64(sb, 1);
      serialize_memoize_tv(sb, depth, pair->get(1));
      serialize_memoize_code(sb, SER_MC_STOP);
    }
  } else if (obj->instanceof(s_IMemoizeParam)) {
    Variant ser = obj->o_invoke_few_args(s_getInstanceKey, 0);
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

    case KindOfPersistentString:
    case KindOfString:
      serialize_memoize_code(sb, SER_MC_STRING);
      serialize_memoize_string_data(sb, tv.m_data.pstr);
      break;

    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentArray:
    case KindOfArray:
      serialize_memoize_array(sb, depth, tv.m_data.parr);
      break;

    case KindOfObject:
      serialize_memoize_obj(sb, depth, tv.m_data.pobj);
      break;

    case KindOfResource:
    case KindOfRef: {
      auto msg = folly::format(
        "Cannot Serialize unexpected type {}",
        tname(tv.m_type)
      ).str();
      SystemLib::throwInvalidArgumentExceptionObject(msg);
      break;
    }
  }
}

} // end anonymous namespace

TypedValue HHVM_FUNCTION(serialize_memoize_param, TypedValue param) {
  // Memoize throws in the emitter if any function parameters are references, so
  // we can just assert that the param is cell here
  assertx(param.m_type != KindOfRef);
  auto const type = param.m_type;

  if (type == KindOfInt64) {
    return param;
  } else if (isStringType(type)) {
    auto const str = param.m_data.pstr;
    if (str->empty()) {
      return make_tv<KindOfPersistentString>(staticEmptyString());
    } else if ((unsigned char)str->data()[0] < 0xf0) {
      // serialize_memoize_tv always returns a string with the first character
      // >= 0xf0, so anything less than that can't collide. There's no worry
      // about int-like strings because the key returned from this function is
      // used in dicts (which don't perform key coercion).
      str->incRefCount();
      return param;
    }
  } else if (type == KindOfUninit || type == KindOfNull) {
    return make_tv<KindOfPersistentString>(s_nullMemoKey.get());
  } else if (type == KindOfBoolean) {
    return make_tv<KindOfPersistentString>(
      param.m_data.num ? s_trueMemoKey.get() : s_falseMemoKey.get()
    );
  }

  StringBuffer sb;
  serialize_memoize_tv(sb, 0, &param);
  return tvReturn(sb.detach());
}

void HHVM_FUNCTION(set_frame_metadata, const Variant& metadata) {
  VMRegAnchor _;
  auto fp = vmfp();
  if (UNLIKELY(!fp)) return;
  if (fp->skipFrame()) fp = g_context->getPrevVMStateSkipFrame(fp);
  if (UNLIKELY(!fp)) return;

  if (LIKELY(!(fp->func()->attrs() & AttrMayUseVV)) ||
      LIKELY(!fp->hasVarEnv())) {
    auto const local = fp->func()->lookupVarId(s_86metadata.get());
    if (LIKELY(local != kInvalidId)) {
      cellSet(*metadata.asCell(), *tvAssertCell(frame_local(fp, local)));
    } else {
      SystemLib::throwInvalidArgumentExceptionObject(
        "Unsupported dynamic call of set_frame_metadata()");
    }
  } else {
    fp->getVarEnv()->set(s_86metadata.get(), metadata.asTypedValue());
  }
}

static struct HHExtension final : Extension {
  HHExtension(): Extension("hh", NO_EXTENSION_VERSION_YET) { }
  void moduleInit() override {
    HHVM_NAMED_FE(HH\\autoload_set_paths, HHVM_FN(autoload_set_paths));
    HHVM_NAMED_FE(HH\\could_include, HHVM_FN(could_include));
    HHVM_NAMED_FE(HH\\serialize_memoize_param,
                  HHVM_FN(serialize_memoize_param));
    HHVM_NAMED_FE(HH\\set_frame_metadata, HHVM_FN(set_frame_metadata));
    loadSystemlib();
  }
} s_hh_extension;

static struct XHPExtension final : Extension {
  XHPExtension(): Extension("xhp", NO_EXTENSION_VERSION_YET) { }
  bool moduleEnabled() const override { return RuntimeOption::EnableXHP; }
} s_xhp_extension;

///////////////////////////////////////////////////////////////////////////////
}
