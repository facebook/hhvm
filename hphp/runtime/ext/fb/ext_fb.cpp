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
#include "hphp/runtime/ext/fb/ext_fb.h"

#include <fstream>

#include <unicode/uchar.h>
#include <unicode/utf8.h>
#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include <folly/String.h>
#include <folly/portability/Sockets.h>

#include "hphp/util/htonll.h"
#include "hphp/util/logger.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/code-coverage.h"
#include "hphp/runtime/base/externals.h"
#include "hphp/runtime/base/file.h"
#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/base/plain-file.h"
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/base/intercept.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/stat-cache.h"
#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/base/string-util.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/ext/std/ext_std_function.h"
#include "hphp/runtime/ext/fb/FBSerialize/FBSerialize.h"
#include "hphp/runtime/ext/fb/VariantController.h"
#include "hphp/runtime/vm/unwind.h"

#include "hphp/parser/parser.h"

namespace HPHP {

// fb_serialize options
const int64_t k_FB_SERIALIZE_HACK_ARRAYS = 1<<1;

///////////////////////////////////////////////////////////////////////////////

static const UChar32 SUBSTITUTION_CHARACTER = 0xFFFD;

#define FB_UNSERIALIZE_NONSTRING_VALUE           0x0001
#define FB_UNSERIALIZE_UNEXPECTED_END            0x0002
#define FB_UNSERIALIZE_UNRECOGNIZED_OBJECT_TYPE  0x0003
#define FB_UNSERIALIZE_UNEXPECTED_ARRAY_KEY_TYPE 0x0004

#ifdef FACEBOOK
# define HHVM_FACEBOOK true
#else
# define HHVM_FACEBOOK false
#endif

///////////////////////////////////////////////////////////////////////////////

/* enum of thrift types */
enum TType {
  T_STOP    = 1,
  T_BYTE    = 2,
  T_U16     = 3,
  T_I16     = 4,
  T_U32     = 5,
  T_I32     = 6,
  T_U64     = 7,
  T_I64     = 8,
  T_STRING  = 9,
  T_STRUCT  = 10,
  T_MAP     = 11,
  T_SET     = 12,
  T_LIST    = 13,
  T_NULL    = 14,
  T_VARCHAR = 15,
  T_DOUBLE  = 16,
  T_BOOLEAN = 17,
};

/* Return the smallest size int that can store the value */
#define INT_SIZE(x) (((x) == ((int8_t)x))  ? 1 :    \
                     ((x) == ((int16_t)x)) ? 2 :    \
                     ((x) == ((int32_t)x)) ? 4 : 8)

/* Return the smallest (supported) unsigned length that can store the value */
#define LEN_SIZE(x) ((((unsigned)x) == ((uint8_t)x)) ? 1 : 4)

Variant HHVM_FUNCTION(fb_serialize, const Variant& thing, int64_t options) {
  try {
    if (options & k_FB_SERIALIZE_HACK_ARRAYS) {
      size_t len =  HPHP::serialize
        ::FBSerializer<VariantControllerUsingHackArrays>
        ::serializedSize(thing);
      String s(len, ReserveString);
      HPHP::serialize
        ::FBSerializer<VariantControllerUsingHackArrays>
        ::serialize(thing, s.mutableData());
      s.setSize(len);
      return s;
    } else {
      size_t len =
        HPHP::serialize::FBSerializer<VariantController>::serializedSize(thing);
      String s(len, ReserveString);
      HPHP::serialize::FBSerializer<VariantController>::serialize(
        thing, s.mutableData());
      s.setSize(len);
      return s;
    }
  } catch (const HPHP::serialize::KeysetSerializeError&) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Keysets cannot be serialized with fb_serialize"
    );
  } catch (const HPHP::serialize::HackArraySerializeError&) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Serializing Hack arrays requires the FB_SERIALIZE_HACK_ARRAYS "
      "option to be provided"
    );
  } catch (const HPHP::serialize::SerializeError&) {
    return init_null();
  }
}

Variant HHVM_FUNCTION(fb_unserialize,
                      const Variant& thing,
                      VRefParam success,
                      int64_t options) {
  if (thing.isString()) {
    String sthing = thing.toString();

    if (sthing.size() && (sthing.data()[0] & 0x80)) {
      return fb_compact_unserialize(sthing.data(), sthing.size(),
                                    success);
    } else {
      return fb_unserialize(sthing.data(), sthing.size(), success, options);
    }
  }

  success.assignIfRef(false);
  return false;
}

Variant fb_unserialize(const char* str,
                       int len,
                       VRefParam success,
                       int64_t options) {
  try {
    if (options & k_FB_SERIALIZE_HACK_ARRAYS) {
      auto res = HPHP::serialize
        ::FBUnserializer<VariantControllerUsingHackArrays>
        ::unserialize(folly::StringPiece(str, len));
      success.assignIfRef(true);
      return res;
    } else {
      auto res = HPHP::serialize::FBUnserializer<VariantController>
        ::unserialize(folly::StringPiece(str, len));
      success.assignIfRef(true);
      return res;
    }
  } catch (const HPHP::serialize::UnserializeError&) {
    success.assignIfRef(false);
    return false;
  }
}

///////////////////////////////////////////////////////////////////////////////

/**
 *                         FB Compact Serialize
 *                         ====================
 *
 * === Compatibility with fb_unserialize ===
 *
 * Check the high bit in the first byte of the serialized string.
 * If it's set, the string is fb_compact_serialize'd, otherwise it's
 * fb_serialize'd.
 *
 * === Format ===
 *
 * A value is serialized as a string <c> <data> where c is a byte (0xf0 | code),
 * code being one of:
 *
 *  0 (INT16): data is 2 bytes, network order signed int16
 *  1 (INT32): data is 4 bytes, network order signed int32
 *  2 (INT64): data is 8 bytes, network order signed int64
 *      All of these represent an int64 value.
 *
 *  3 (NULL): no data, null value
 *
 *  4 (TRUE),
 *  5 (FALSE): no data, boolean value
 *
 *  6 (DOUBLE): data is 8 bytes, double value
 *
 *  7 (STRING_0): no data
 *  8 (STRING_1): one char of data
 *  9 (STRING_N): followed by n as a serialized int64, followed by n characters
 *      All of these represent a string value.
 *
 *  10 (LIST_MAP): followed by serialized values until STOP is seen.
 *      Represents a map with numeric keys 0, 1, ..., n-1 (but see SKIP below).
 *
 *  11 (MAP): followed by serialized key/value pairs until STOP
 *      is seen.  Represents a map with arbitrary int64 or string keys.
 *
 *  12 (STOP): no data
 *      Marks the end of a LIST or a MAP.
 *
 *  13 (SKIP): no data
 *      If seen as an entry in a LIST_MAP, the next index in the sequence will
 *       be skipped.  E.g. array(0 => 'a', 1 => 'b', 3 => 'c) will be encoded as
 *      (LIST_MAP, 'a', 'b', SKIP, 'c') instead of
 *      (MAP, 0, 'a', 1, 'b', 3, 'c').
 *
 *  14 (VECTOR): followed by n serialized values until STOP is seen.
 *      Represents a vector of n values.
 *
 *  In addition, if <c> & 0xf0 != 0xf0, most significant bits of <c> mean:
 *
 *  - 0....... 7-bit unsigned int
 *      (NOTE: not used for the sole int value due to the compatibility
 *       requirement above)
 *  - 10...... + 6 more bytes, 54-bit unsigned int
 *  - 110..... + 1 more byte,  13-bit unsigned int
 *  - 1110.... + 2 more bytes, 20-bit unsigned int
 *
 *  All of these represent an int64 value.
 */

enum FbCompactSerializeCode {
  FB_CS_INT16      = 0,
  FB_CS_INT32      = 1,
  FB_CS_INT64      = 2,
  FB_CS_NULL       = 3,
  FB_CS_TRUE       = 4,
  FB_CS_FALSE      = 5,
  FB_CS_DOUBLE     = 6,
  FB_CS_STRING_0   = 7,
  FB_CS_STRING_1   = 8,
  FB_CS_STRING_N   = 9,
  FB_CS_LIST_MAP   = 10,
  FB_CS_MAP        = 11,
  FB_CS_STOP       = 12,
  FB_CS_SKIP       = 13,
  FB_CS_VECTOR     = 14,
  FB_CS_OBJ        = 15,
  FB_CS_MAX_CODE   = 16,
};

// 1 byte: 0<7 bits>
const uint64_t kInt7Mask            = 0x7f;
const uint64_t kInt7Prefix          = 0x00;

// 2 bytes: 110<13 bits>
const uint64_t kInt13Mask           = (1ULL << 13) - 1;
const uint64_t kInt13PrefixMsbMask  = 0xe0;
const uint64_t kInt13PrefixMsb      = 0xc0;
const uint64_t kInt13Prefix         = kInt13PrefixMsb << (1 * 8);

// 3 bytes: 1110<20 bits>
const uint64_t kInt20Mask           = (1ULL << 20) - 1;
const uint64_t kInt20PrefixMsbMask  = 0xf0;
const uint64_t kInt20PrefixMsb      = 0xe0;
const uint64_t kInt20Prefix         = kInt20PrefixMsb << (2 * 8);

// 7 bytes: 10<54 bits>
const uint64_t kInt54Mask           = (1ULL << 54) - 1;
const uint64_t kInt54PrefixMsbMask  = 0xc0;
const uint64_t kInt54PrefixMsb      = 0x80;
const uint64_t kInt54Prefix         = kInt54PrefixMsb << (6 * 8);

// 1 byte: 1111<4 bits>
const uint64_t kCodeMask            = 0x0f;
const uint64_t kCodePrefix          = 0xf0;

static void fb_compact_serialize_code(StringBuffer& sb,
                                      FbCompactSerializeCode code) {
  assert(code == (code & kCodeMask));
  uint8_t v = (kCodePrefix | code);
  sb.append(reinterpret_cast<char*>(&v), 1);
}

static void fb_compact_serialize_int64(StringBuffer& sb, int64_t val) {
  if (val >= 0 && (uint64_t)val <= kInt7Mask) {
    uint8_t nval = val;
    sb.append(reinterpret_cast<char*>(&nval), 1);

  } else if (val >= 0 && (uint64_t)val <= kInt13Mask) {
    uint16_t nval = htons(kInt13Prefix | val);
    sb.append(reinterpret_cast<char*>(&nval), 2);

  } else if (val == (int64_t)(int16_t)val) {
    fb_compact_serialize_code(sb, FB_CS_INT16);
    uint16_t nval = htons(val);
    sb.append(reinterpret_cast<char*>(&nval), 2);

  } else if (val >= 0 && (uint64_t)val <= kInt20Mask) {
    uint32_t nval = htonl(kInt20Prefix | val);
    // Skip most significant byte
    sb.append(reinterpret_cast<char*>(&nval) + 1, 3);

  } else if (val == (int64_t)(int32_t)val) {
    fb_compact_serialize_code(sb, FB_CS_INT32);
    uint32_t nval = htonl(val);
    sb.append(reinterpret_cast<char*>(&nval), 4);

  } else if (val >= 0 && (uint64_t)val <= kInt54Mask) {
    uint64_t nval = htonll(kInt54Prefix | val);
    // Skip most significant byte
    sb.append(reinterpret_cast<char*>(&nval) + 1, 7);

  } else {
    fb_compact_serialize_code(sb, FB_CS_INT64);
    uint64_t nval = htonll(val);
    sb.append(reinterpret_cast<char*>(&nval), 8);
  }
}

static void fb_compact_serialize_string(StringBuffer& sb, const String& str) {
  int len = str.size();
  if (len == 0) {
    fb_compact_serialize_code(sb, FB_CS_STRING_0);
  } else {
    if (len == 1) {
      fb_compact_serialize_code(sb, FB_CS_STRING_1);
    } else {
      fb_compact_serialize_code(sb, FB_CS_STRING_N);
      fb_compact_serialize_int64(sb, len);
    }
    sb.append(str.data(), len);
  }
}

static bool fb_compact_serialize_is_list(const Array& arr, int64_t& index_limit) {
  index_limit = arr.size();
  int64_t max_index = 0;
  for (ArrayIter it(arr); it; ++it) {
    Variant key = it.first();
    if (!key.isNumeric()) {
      return false;
    }
    int64_t index = key.toInt64();
    if (index < 0) {
      return false;
    }
    if (index > max_index) {
      max_index = index;
    }
  }

  if (max_index >= arr.size() * 2) {
    // Might as well store it as a map
    return false;
  }

  index_limit = max_index + 1;
  return true;
}

static int fb_compact_serialize_variant(
  StringBuffer& sd, const Variant& var, int depth);

static void fb_compact_serialize_array_as_list_map(
    StringBuffer& sb, const Array& arr, int64_t index_limit, int depth) {
  fb_compact_serialize_code(sb, FB_CS_LIST_MAP);
  for (int64_t i = 0; i < index_limit; ++i) {
    if (arr.exists(i)) {
      fb_compact_serialize_variant(sb, arr[i], depth + 1);
    } else {
      fb_compact_serialize_code(sb, FB_CS_SKIP);
    }
  }
  fb_compact_serialize_code(sb, FB_CS_STOP);
}

static void fb_compact_serialize_vec(
    StringBuffer& sb, const Array& arr, int depth) {
  fb_compact_serialize_code(sb, FB_CS_LIST_MAP);
  PackedArray::IterateV(
    arr.get(),
    [&](const TypedValue* v) {
      fb_compact_serialize_variant(sb, tvAsCVarRef(v), depth + 1);
    }
  );
  fb_compact_serialize_code(sb, FB_CS_STOP);
}

static void fb_compact_serialize_array_as_map(
    StringBuffer& sb, const Array& arr, int depth) {
  fb_compact_serialize_code(sb, FB_CS_MAP);
  IterateKV(
    arr.get(),
    [&](const TypedValue* k, const TypedValue* v) {
      if (tvIsString(k)) {
        fb_compact_serialize_string(sb, StrNR{k->m_data.pstr});
      } else {
        assertx(k->m_type == KindOfInt64);
        fb_compact_serialize_int64(sb, k->m_data.num);
      }
      fb_compact_serialize_variant(sb, tvAsCVarRef(v), depth + 1);
    }
  );
  fb_compact_serialize_code(sb, FB_CS_STOP);
}

static void fb_compact_serialize_keyset(
    StringBuffer& sb, const Array& arr) {
  fb_compact_serialize_code(sb, FB_CS_MAP);
  SetArray::Iterate(
    SetArray::asSet(arr.get()),
    [&](const TypedValue* v) {
      if (tvIsString(v)) {
        fb_compact_serialize_string(sb, StrNR{v->m_data.pstr});
        fb_compact_serialize_string(sb, StrNR{v->m_data.pstr});
      } else {
        assertx(v->m_type == KindOfInt64);
        fb_compact_serialize_int64(sb, v->m_data.num);
        fb_compact_serialize_int64(sb, v->m_data.num);
      }
    }
  );
  fb_compact_serialize_code(sb, FB_CS_STOP);
}

static int fb_compact_serialize_variant(
    StringBuffer& sb, const Variant& var, int depth) {
  if (depth > 256) {
    return 1;
  }

  switch (var.getType()) {
    case KindOfUninit:
    case KindOfNull:
      fb_compact_serialize_code(sb, FB_CS_NULL);
      return 0;

    case KindOfBoolean:
      if (var.toInt64()) {
        fb_compact_serialize_code(sb, FB_CS_TRUE);
      } else {
        fb_compact_serialize_code(sb, FB_CS_FALSE);
      }
      return 0;

    case KindOfInt64:
      fb_compact_serialize_int64(sb, var.toInt64());
      return 0;

    case KindOfDouble: {
      fb_compact_serialize_code(sb, FB_CS_DOUBLE);
      double d = var.toDouble();
      sb.append(reinterpret_cast<char*>(&d), 8);
      return 0;
    }

    case KindOfPersistentString:
    case KindOfString:
      fb_compact_serialize_string(sb, var.toString());
      return 0;

    case KindOfPersistentVec:
    case KindOfVec: {
      Array arr = var.toArray();
      assert(arr->isVecArray());
      fb_compact_serialize_vec(sb, std::move(arr), depth);
      return 0;
    }

    case KindOfPersistentDict:
    case KindOfDict: {
      Array arr = var.toArray();
      assert(arr->isDict());
      fb_compact_serialize_array_as_map(sb, std::move(arr), depth);
      return 0;
    }

    case KindOfPersistentKeyset:
    case KindOfKeyset: {
      Array arr = var.toArray();
      assert(arr->isKeyset());
      fb_compact_serialize_keyset(sb, std::move(arr));
      return 0;
    }

    case KindOfPersistentArray:
    case KindOfArray: {
      Array arr = var.toArray();
      assert(arr->isPHPArray());
      int64_t index_limit;
      if (fb_compact_serialize_is_list(arr, index_limit)) {
        fb_compact_serialize_array_as_list_map(
          sb, std::move(arr), index_limit, depth);
      } else {
        fb_compact_serialize_array_as_map(sb, std::move(arr), depth);
      }
      return 0;
    }

    case KindOfObject:
    case KindOfResource:
    case KindOfRef:
      fb_compact_serialize_code(sb, FB_CS_NULL);
      raise_warning(
        "fb_compact_serialize(): unable to serialize object/resource/ref"
      );
      break;
  }

  return 1;
}

String fb_compact_serialize(const Variant& thing) {
  /**
   * If thing is a single int value [0, 127] normally we would serialize
   * it as a single byte (7 bit unsigned int).
   *
   * However, we want highest bit of the first byte to always be set so
   * that we can tell if the string is fb_serialize'd or fb_compact_serialize'd.
   *
   * So we force to serialize it as 13 bit unsigned int instead.
   */
  if (thing.getType() == KindOfInt64) {
    int64_t val = thing.toInt64();
    if (val >= 0 && (uint64_t)val <= kInt7Mask) {
      String s(2, ReserveString);
      *(uint16_t*)(s.mutableData()) = (uint16_t)htons(kInt13Prefix | val);
      s.setSize(2);
      return s;
    }
  }

  StringBuffer sb;
  if (fb_compact_serialize_variant(sb, thing, 0)) {
    return String();
  }

  return sb.detach();
}

Variant HHVM_FUNCTION(fb_compact_serialize, const Variant& thing) {
  return fb_compact_serialize(thing);
}

/* Check if there are enough bytes left in the buffer */
#define CHECK_ENOUGH(bytes, pos, num) do {                      \
    if ((int)(bytes) > (int)((num) - (pos))) {                  \
      return FB_UNSERIALIZE_UNEXPECTED_END;                     \
    }                                                           \
  } while (0)


int fb_compact_unserialize_int64_from_buffer(
  int64_t& out, const char* buf, int n, int& p) {

  CHECK_ENOUGH(1, p, n);
  uint64_t first = (unsigned char)buf[p];
  if ((first & ~kInt7Mask) == kInt7Prefix) {
    p += 1;
    out = first & kInt7Mask;

  } else if ((first & kInt13PrefixMsbMask) == kInt13PrefixMsb) {
    CHECK_ENOUGH(2, p, n);
    uint16_t val = (uint16_t)ntohs(*reinterpret_cast<const uint16_t*>(buf + p));
    p += 2;
    out = val & kInt13Mask;

  } else if (first == (kCodePrefix | FB_CS_INT16)) {
    p += 1;
    CHECK_ENOUGH(2, p, n);
    int16_t val = (int16_t)ntohs(*reinterpret_cast<const int16_t*>(buf + p));
    p += 2;
    out = val;

  } else if ((first & kInt20PrefixMsbMask) == kInt20PrefixMsb) {
    CHECK_ENOUGH(3, p, n);
    uint32_t b = 0;
    memcpy(&b, buf + p, 3);
    uint32_t val = ntohl(b);
    p += 3;
    out = (val >> 8) & kInt20Mask;

  } else if (first == (kCodePrefix | FB_CS_INT32)) {
    p += 1;
    CHECK_ENOUGH(4, p, n);
    int32_t val = (int32_t)ntohl(*reinterpret_cast<const int32_t*>(buf + p));
    p += 4;
    out = val;

  } else if ((first & kInt54PrefixMsbMask) == kInt54PrefixMsb) {
    CHECK_ENOUGH(7, p, n);
    uint64_t b = 0;
    memcpy(&b, buf + p, 7);
    uint64_t val = ntohll(b);
    p += 7;
    out = (val >> 8) & kInt54Mask;

  } else if (first == (kCodePrefix | FB_CS_INT64)) {
    p += 1;
    CHECK_ENOUGH(8, p, n);
    int64_t val = (int64_t)ntohll(*reinterpret_cast<const int64_t*>(buf + p));
    p += 8;
    out = val;

  } else {
    return FB_UNSERIALIZE_UNRECOGNIZED_OBJECT_TYPE;
  }

  return 0;
}

const StaticString s_empty("");

int fb_compact_unserialize_from_buffer(
  Variant& out, const char* buf, int n, int& p) {

  CHECK_ENOUGH(1, p, n);
  int code = (unsigned char)buf[p];
  if ((code & ~kCodeMask) != kCodePrefix ||
      (code & kCodeMask) == FB_CS_INT16 ||
      (code & kCodeMask) == FB_CS_INT32 ||
      (code & kCodeMask) == FB_CS_INT64) {

    int64_t val;
    int err = fb_compact_unserialize_int64_from_buffer(val, buf, n, p);
    if (err) {
      return err;
    }
    out = (int64_t)val;
    return 0;
  }
  p += 1;
  code &= kCodeMask;
  switch (code) {
    case FB_CS_NULL:
      out = uninit_null();
      break;

    case FB_CS_TRUE:
      out = true;
      break;

    case FB_CS_FALSE:
      out = false;
      break;

    case FB_CS_DOUBLE:
    {
      CHECK_ENOUGH(8, p, n);
      double d = *reinterpret_cast<const double*>(buf + p);
      p += 8;
      out = d;
      break;
    }

    case FB_CS_STRING_0:
    {
      out = s_empty;
      break;
    }

    case FB_CS_STRING_1:
    case FB_CS_STRING_N:
    {
      int64_t len = 1;
      if (code == FB_CS_STRING_N) {
        int err = fb_compact_unserialize_int64_from_buffer(len, buf, n, p);
        if (err) {
          return err;
        }
      }

      CHECK_ENOUGH(len, p, n);
      out = Variant::attach(StringData::Make(buf + p, len, CopyString));
      p += len;
      break;
    }

    case FB_CS_LIST_MAP:
    case FB_CS_VECTOR:
    {
      Array arr = Array::Create();
      int64_t i = 0;
      while (p < n && buf[p] != (char)(kCodePrefix | FB_CS_STOP)) {
        if (buf[p] == (char)(kCodePrefix | FB_CS_SKIP)) {
          ++i;
          ++p;
        } else {
          Variant value;
          int err = fb_compact_unserialize_from_buffer(value, buf, n, p);
          if (err) {
            return err;
          }
          arr.set(i++, value);
        }
      }

      // Consume STOP
      CHECK_ENOUGH(1, p, n);
      p += 1;

      out = arr;
      break;
    }

    case FB_CS_MAP:
    {
      Array arr = Array::Create();
      while (p < n && buf[p] != (char)(kCodePrefix | FB_CS_STOP)) {
        Variant key;
        int err = fb_compact_unserialize_from_buffer(key, buf, n, p);
        if (err) {
          return err;
        }
        Variant value;
        err = fb_compact_unserialize_from_buffer(value, buf, n, p);
        if (err) {
          return err;
        }
        if (key.getType() == KindOfInt64) {
          arr.set(key.toInt64(), value);
        } else if (key.getType() == KindOfString ||
                   key.getType() == KindOfPersistentString) {
          arr.set(key, value);
        } else {
          return FB_UNSERIALIZE_UNEXPECTED_ARRAY_KEY_TYPE;
        }
      }

      // Consume STOP
      CHECK_ENOUGH(1, p, n);
      p += 1;

      out = arr;
      break;
    }

    default:
      return FB_UNSERIALIZE_UNRECOGNIZED_OBJECT_TYPE;
  }

  return 0;
}

Variant fb_compact_unserialize(const char* str, int len,
                               VRefParam success,
                               VRefParam errcode /* = uninit_variant */) {

  Variant ret;
  int p = 0;
  int err = fb_compact_unserialize_from_buffer(ret, str, len, p);
  if (err) {
    success.assignIfRef(false);
    errcode.assignIfRef(err);
    return false;
  }
  success.assignIfRef(true);
  errcode.assignIfRef(init_null());
  return ret;
}

Variant HHVM_FUNCTION(fb_compact_unserialize,
                      const Variant& thing, VRefParam success,
                      VRefParam errcode /* = uninit_variant */) {
  if (!thing.isString()) {
    success.assignIfRef(false);
    errcode.assignIfRef(FB_UNSERIALIZE_NONSTRING_VALUE);
    return false;
  }

  String s = thing.toString();
  return fb_compact_unserialize(s.data(), s.size(), ref(success),
                                ref(errcode));
}

///////////////////////////////////////////////////////////////////////////////

bool HHVM_FUNCTION(fb_utf8ize, VRefParam input) {
  String s = input.toString();
  const char* const srcBuf = s.data();
  int32_t srcLenBytes = s.size();

  if (s.size() < 0 || s.size() > INT_MAX) {
    return false; // Too long.
  }

  // Preflight to avoid allocation if the entire input is valid.
  int32_t srcPosBytes;
  for (srcPosBytes = 0; srcPosBytes < srcLenBytes; /* U8_NEXT increments */) {
    // This is lame, but gcc doesn't optimize U8_NEXT very well
    if (srcBuf[srcPosBytes] > 0 && srcBuf[srcPosBytes] <= 0x7f) {
      srcPosBytes++; // U8_NEXT would increment this
      continue;
    }
    UChar32 curCodePoint;
    // U8_NEXT() always advances srcPosBytes; save in case curCodePoint invalid
    int32_t savedSrcPosBytes = srcPosBytes;
    U8_NEXT(srcBuf, srcPosBytes, srcLenBytes, curCodePoint);
    if (curCodePoint <= 0) {
      // curCodePoint invalid; back up so we'll fix it in the loop below.
      srcPosBytes = savedSrcPosBytes;
      break;
    }
  }

  if (srcPosBytes == srcLenBytes) {
    // it's all valid
    return true;
  }

  // There are invalid bytes. Allocate memory, then copy the input, replacing
  // invalid sequences with either the substitution character or nothing,
  // depending on the value of RuntimeOption::Utf8izeReplace.
  //
  // Worst case, every remaining byte is invalid, taking a 3-byte substitution.
  int32_t bytesRemaining = srcLenBytes - srcPosBytes;
  uint64_t dstMaxLenBytes = srcPosBytes + (RuntimeOption::Utf8izeReplace ?
    bytesRemaining * U8_LENGTH(SUBSTITUTION_CHARACTER) :
    bytesRemaining);
  if (dstMaxLenBytes > INT_MAX) {
    return false; // Too long.
  }
  String dstStr(dstMaxLenBytes, ReserveString);
  char *dstBuf = dstStr.mutableData();

  // Copy valid bytes found so far as one solid block.
  memcpy(dstBuf, srcBuf, srcPosBytes);

  // Iterate through the remaining bytes.
  int32_t dstPosBytes = srcPosBytes; // already copied srcPosBytes
  for (/* already init'd */; srcPosBytes < srcLenBytes; /* see U8_NEXT */) {
    UChar32 curCodePoint;
    // This is lame, but gcc doesn't optimize U8_NEXT very well
    if (srcBuf[srcPosBytes] > 0 && srcBuf[srcPosBytes] <= 0x7f) {
      curCodePoint = srcBuf[srcPosBytes++]; // U8_NEXT would increment
    } else {
      U8_NEXT(srcBuf, srcPosBytes, srcLenBytes, curCodePoint);
    }
    if (curCodePoint <= 0) {
      // Invalid UTF-8 sequence.
      // N.B. We consider a null byte an invalid sequence.
      if (!RuntimeOption::Utf8izeReplace) {
        continue; // Omit invalid sequence
      }
      curCodePoint = SUBSTITUTION_CHARACTER; // Replace invalid sequences
    }
    // We know that resultBuffer > total possible length.
    U8_APPEND_UNSAFE(dstBuf, dstPosBytes, curCodePoint);
  }
  assert(dstPosBytes <= dstMaxLenBytes);
  input.assignIfRef(dstStr.shrink(dstPosBytes));
  return true;
}

/**
 * Private utf8_strlen implementation.
 *
 * Returns count of code points in input, substituting 1 code point per invalid
 * sequence.
 *
 * deprecated=true: instead return byte count on invalid UTF-8 sequence.
 */
static int fb_utf8_strlen_impl(const String& input, bool deprecated) {
  // Count, don't modify.
  int32_t sourceLength = input.size();
  const char* const sourceBuffer = input.data();
  int64_t num_code_points = 0;

  for (int32_t sourceOffset = 0; sourceOffset < sourceLength; ) {
    UChar32 sourceCodePoint;
    // U8_NEXT() is guaranteed to advance sourceOffset by 1-4 each time it's
    // invoked.
    U8_NEXT(sourceBuffer, sourceOffset, sourceLength, sourceCodePoint);
    if (deprecated && sourceCodePoint < 0) {
      return sourceLength; // return byte count on invalid sequence
    }
    num_code_points++;
  }
  return num_code_points;
}

int64_t HHVM_FUNCTION(fb_utf8_strlen, const String& input) {
  return fb_utf8_strlen_impl(input, /* deprecated */ false);
}

int64_t HHVM_FUNCTION(fb_utf8_strlen_deprecated, const String& input) {
  return fb_utf8_strlen_impl(input, /* deprecated */ true);
}

/**
 * Private helper; requires non-negative firstCodePoint and desiredCodePoints.
 */
static String fb_utf8_substr_simple(const String& str,
                                    int32_t firstCodePoint,
                                    int32_t numDesiredCodePoints) {
  const char* const srcBuf = str.data();
  int32_t srcLenBytes = str.size(); // May truncate; checked before use below.

  assert(firstCodePoint >= 0);  // Wrapper fixes up negative starting positions.
  assert(numDesiredCodePoints > 0); // Wrapper fixes up negative/zero length.
  if (str.size() <= 0 ||
      str.size() > INT_MAX ||
      firstCodePoint >= srcLenBytes) {
    return empty_string();
  }

  // Cannot be more code points than bytes in input.  This typically reduces
  // the INT_MAX default value to something more reasonable.
  numDesiredCodePoints = std::min(numDesiredCodePoints,
                                  srcLenBytes - firstCodePoint);

  // Pre-allocate the result.
  // the worst case can come from one of two sources:
  //  - every code point could be the substitution char (3 bytes)
  //    giving us numDesiredCodePoints * 3
  //  - every code point could be 4 bytes long, giving us
  //    numDesiredCodePoints * 4 - but capped by the length of the input
  uint64_t dstMaxLenBytes =
    std::min((uint64_t)numDesiredCodePoints * 4,
             (uint64_t)srcLenBytes - firstCodePoint);
  dstMaxLenBytes = std::max(dstMaxLenBytes,
                            (uint64_t)numDesiredCodePoints *
                            U8_LENGTH(SUBSTITUTION_CHARACTER));
  if (dstMaxLenBytes > INT_MAX) {
    return empty_string(); // Too long.
  }
  String dstStr(dstMaxLenBytes, ReserveString);
  char* dstBuf = dstStr.mutableData();
  int32_t dstPosBytes = 0;

  // Iterate through src's codepoints; srcPosBytes is incremented by U8_NEXT.
  for (int32_t srcPosBytes = 0, srcPosCodePoints = 0;
       srcPosBytes < srcLenBytes && // more available
       srcPosCodePoints < firstCodePoint + numDesiredCodePoints; // want more
       srcPosCodePoints++) {

    // U8_NEXT() advances sourceBytePos by 1-4 each time it's invoked.
    UChar32 curCodePoint;
    U8_NEXT(srcBuf, srcPosBytes, srcLenBytes, curCodePoint);

    if (srcPosCodePoints >= firstCodePoint) {
      // Copy this code point into the result.
      if (curCodePoint < 0) {
        curCodePoint = SUBSTITUTION_CHARACTER; // replace invalid sequences
      }
      // We know that resultBuffer > total possible length.
      // U8_APPEND_UNSAFE updates dstPosBytes.
      U8_APPEND_UNSAFE(dstBuf, dstPosBytes, curCodePoint);
    }
  }

  assert(dstPosBytes <= dstMaxLenBytes);
  if (dstPosBytes > 0) {
    dstStr.shrink(dstPosBytes);
    return dstStr;
  }
  return empty_string();
}

String HHVM_FUNCTION(fb_utf8_substr, const String& str, int64_t start,
                                     int64_t length /* = INT_MAX */) {
  if (length > INT_MAX) {
    length = INT_MAX;
  }
  // For negative start or length, calculate start and length values
  // based on total code points.
  if (start < 0 || length < 0) {
    // Get number of code points assuming we substitute invalid sequences.
    Variant utf8StrlenResult = HHVM_FN(fb_utf8_strlen)(str);
    int32_t sourceNumCodePoints = utf8StrlenResult.toInt32();

    if (start < 0) {
      // Negative means first character is start'th code point from end.
      // e.g., -1 means start with the last code point.
      start = sourceNumCodePoints + start; // adding negative start
    }
    if (length < 0) {
      // Negative means omit last abs(length) code points.
      length = sourceNumCodePoints - start + length; // adding negative length
    }
  }
  if (start < 0 || length <= 0) {
    return empty_string(); // Empty result
  }

  return fb_utf8_substr_simple(str, start, length);
}

///////////////////////////////////////////////////////////////////////////////

bool HHVM_FUNCTION(fb_intercept, const String& name, const Variant& handler,
                                 const Variant& data /* = uninit_variant */) {
  return register_intercept(name, handler, data);
}

bool is_dangerous_varenv_function(const StringData* name) {
  auto const f = Unit::lookupBuiltin(name);
  // Functions can which can access the caller's frame are always builtin.
  return f && f->accessesCallerFrame();
}

bool HHVM_FUNCTION(fb_rename_function, const String& orig_func_name,
                                       const String& new_func_name) {
  if (orig_func_name.empty() || new_func_name.empty() ||
      orig_func_name.get()->isame(new_func_name.get())) {
    throw_invalid_argument("unable to rename %s", orig_func_name.data());
    return false;
  }

  if (!function_exists(orig_func_name)) {
    raise_warning("fb_rename_function(%s, %s) failed: %s does not exist!",
                  orig_func_name.data(), new_func_name.data(),
                  orig_func_name.data());
    return false;
  }

  if (is_dangerous_varenv_function(orig_func_name.get())) {
    raise_warning(
      "fb_rename_function(%s, %s) failed: rename of functions that "
      "affect variable environments is not allowed",
      orig_func_name.data(), new_func_name.data());
    return false;
  }

  if (function_exists(new_func_name)) {
    if (new_func_name.data()[0] != '1') {
      raise_warning("fb_rename_function(%s, %s) failed: %s already exists!",
                    orig_func_name.data(), new_func_name.data(),
                    new_func_name.data());
      return false;
    }
  }

  rename_function(orig_func_name, new_func_name);
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// call_user_func extensions
// Linked in via fb.json.idl for now - Need OptFunc solution...

Array HHVM_FUNCTION(fb_call_user_func_safe,
                    const Variant& function,
                    const Array& argv) {
  return HHVM_FN(fb_call_user_func_array_safe)(function, argv);
}

Variant HHVM_FUNCTION(fb_call_user_func_safe_return,
                      const Variant& function,
                      const Variant& def,
                      const Array& argv) {
  if (is_callable(function)) {
    return vm_call_user_func(function, argv);
  }
  return def;
}

Array HHVM_FUNCTION(fb_call_user_func_array_safe,
                    const Variant& function,
                    const Array& params) {
  if (is_callable(function)) {
    return make_packed_array(true, vm_call_user_func(function, params));
  }
  return make_packed_array(false, uninit_variant);
}

///////////////////////////////////////////////////////////////////////////////

Variant HHVM_FUNCTION(fb_get_code_coverage, bool flush) {
  ThreadInfo *ti = ThreadInfo::s_threadInfo.getNoCheck();
  if (ti->m_reqInjectionData.getCoverage()) {
    Array ret = ti->m_coverage->Report();
    if (flush) {
      ti->m_coverage->Reset();
    }
    return ret;
  }
  return false;
}

void HHVM_FUNCTION(fb_enable_code_coverage) {
  ThreadInfo *ti = ThreadInfo::s_threadInfo.getNoCheck();
  ti->m_coverage->Reset();
  ti->m_reqInjectionData.setCoverage(true);;
  if (g_context->isNested()) {
    raise_notice("Calling fb_enable_code_coverage from a nested "
                 "VM instance may cause unpredicable results");
  }
  throw VMSwitchModeBuiltin();
}

Variant HHVM_FUNCTION(fb_disable_code_coverage) {
  ThreadInfo *ti = ThreadInfo::s_threadInfo.getNoCheck();
  ti->m_reqInjectionData.setCoverage(false);
  Array ret = ti->m_coverage->Report();
  ti->m_coverage->Reset();
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool HHVM_FUNCTION(fb_output_compression, bool new_value) {
  Transport *transport = g_context->getTransport();
  if (transport) {
    bool rv = transport->isCompressionEnabled();
    if (new_value) {
      transport->enableCompression();
    } else {
      transport->disableCompression();
    }
    return rv;
  }
  return false;
}

void HHVM_FUNCTION(fb_set_exit_callback, const Variant& function) {
  g_context->setExitCallback(function);
}

const StaticString
  s_flush_stats("flush_stats"),
  s_chunk_stats("chunk_stats"),
  s_total("total"),
  s_sent("sent"),
  s_time("time");

int64_t HHVM_FUNCTION(fb_get_last_flush_size) {
  Transport *transport = g_context->getTransport();
  return transport ? transport->getLastChunkSentSize() : 0;
}

extern Array stat_impl(struct stat*); // ext_file.cpp

template<class Function>
static Variant do_lazy_stat(Function dostat, const String& filename) {
  struct stat sb;
  if (dostat(File::TranslatePathWithFileCache(filename).c_str(), &sb)) {
    Logger::Verbose("%s/%d: %s", __FUNCTION__, __LINE__,
                    folly::errnoStr(errno).c_str());
    return false;
  }
  return stat_impl(&sb);
}

Variant HHVM_FUNCTION(fb_lazy_lstat, const String& filename) {
  if (!FileUtil::checkPathAndWarn(filename, __FUNCTION__ + 2, 1)) {
    return false;
  }
  return do_lazy_stat(StatCache::lstat, filename);
}

Variant HHVM_FUNCTION(fb_lazy_realpath, const String& filename) {
  if (!FileUtil::checkPathAndWarn(filename, __FUNCTION__ + 2, 1)) {
    return false;
  }

  return StatCache::realpath(filename.c_str());
}

///////////////////////////////////////////////////////////////////////////////

EXTERNALLY_VISIBLE
void const_load() {
  // TODO(8117903): Unused; remove after updating www side.
}

///////////////////////////////////////////////////////////////////////////////

struct FBExtension : Extension {
  FBExtension(): Extension("fb", "1.0.0") {}

  void moduleInit() override {
    HHVM_RC_BOOL_SAME(HHVM_FACEBOOK);
    HHVM_RC_INT_SAME(FB_UNSERIALIZE_NONSTRING_VALUE);
    HHVM_RC_INT_SAME(FB_UNSERIALIZE_UNEXPECTED_END);
    HHVM_RC_INT_SAME(FB_UNSERIALIZE_UNRECOGNIZED_OBJECT_TYPE);
    HHVM_RC_INT_SAME(FB_UNSERIALIZE_UNEXPECTED_ARRAY_KEY_TYPE);

    HHVM_RC_INT(FB_SERIALIZE_HACK_ARRAYS, k_FB_SERIALIZE_HACK_ARRAYS);

    HHVM_FE(fb_serialize);
    HHVM_FE(fb_unserialize);
    HHVM_FE(fb_compact_serialize);
    HHVM_FE(fb_compact_unserialize);
    HHVM_FE(fb_utf8ize);
    HHVM_FE(fb_utf8_strlen);
    HHVM_FE(fb_utf8_strlen_deprecated);
    HHVM_FE(fb_utf8_substr);
    HHVM_FE(fb_intercept);
    HHVM_FE(fb_rename_function);
    HHVM_FE(fb_get_code_coverage);
    HHVM_FE(fb_enable_code_coverage);
    HHVM_FE(fb_disable_code_coverage);
    HHVM_FE(fb_output_compression);
    HHVM_FE(fb_set_exit_callback);
    HHVM_FE(fb_get_last_flush_size);
    HHVM_FE(fb_lazy_lstat);
    HHVM_FE(fb_lazy_realpath);
    HHVM_FE(fb_call_user_func_safe);
    HHVM_FE(fb_call_user_func_safe_return);
    HHVM_FE(fb_call_user_func_array_safe);

    loadSystemlib();
  }
} s_fb_extension;

///////////////////////////////////////////////////////////////////////////////
}
