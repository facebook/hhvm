/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/ext/ext_fb.h>
#include <runtime/ext/ext_function.h>
#include <runtime/ext/ext_mysql.h>
#include <util/db_conn.h>
#include <util/logger.h>
#include <runtime/base/stat_cache.h>
#include <netinet/in.h>
#include <runtime/base/externals.h>
#include <runtime/base/string_util.h>
#include <runtime/base/util/string_buffer.h>
#include <runtime/base/code_coverage.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/array/zend_array.h>
#include <runtime/base/intercept.h>
#include <runtime/base/taint/taint_data.h>
#include <runtime/base/taint/taint_trace.h>
#include <runtime/base/taint/taint_warning.h>
#include <runtime/vm/backup_gc.h>
#include <unicode/uchar.h>
#include <unicode/utf8.h>
#include <runtime/eval/runtime/file_repository.h>

#include <util/parser/parser.h>

namespace HPHP {
IMPLEMENT_DEFAULT_EXTENSION(fb);
///////////////////////////////////////////////////////////////////////////////

static const UChar32 SUBSTITUTION_CHARACTER = 0xFFFD;

#define FB_UNSERIALIZE_NONSTRING_VALUE           0x0001
#define FB_UNSERIALIZE_UNEXPECTED_END            0x0002
#define FB_UNSERIALIZE_UNRECOGNIZED_OBJECT_TYPE  0x0003
#define FB_UNSERIALIZE_UNEXPECTED_ARRAY_KEY_TYPE 0x0004

const int64 k_FB_UNSERIALIZE_NONSTRING_VALUE = FB_UNSERIALIZE_NONSTRING_VALUE;
const int64 k_FB_UNSERIALIZE_UNEXPECTED_END = FB_UNSERIALIZE_UNEXPECTED_END;
const int64 k_FB_UNSERIALIZE_UNRECOGNIZED_OBJECT_TYPE =
  FB_UNSERIALIZE_UNRECOGNIZED_OBJECT_TYPE;
const int64 k_FB_UNSERIALIZE_UNEXPECTED_ARRAY_KEY_TYPE =
  FB_UNSERIALIZE_UNEXPECTED_ARRAY_KEY_TYPE;

const int64 k_TAINT_NONE = TAINT_BIT_NONE;
const int64 k_TAINT_HTML = TAINT_BIT_HTML;
const int64 k_TAINT_MUTATED = TAINT_BIT_MUTATED;
const int64 k_TAINT_SQL = TAINT_BIT_SQL;
const int64 k_TAINT_SHELL = TAINT_BIT_SHELL;
const int64 k_TAINT_TRACE_HTML = TAINT_BIT_TRACE_HTML;
const int64 k_TAINT_ALL = TAINT_BIT_ALL;
const int64 k_TAINT_TRACE_SELF = TAINT_BIT_TRACE_SELF;

///////////////////////////////////////////////////////////////////////////////

/* Linux and other systems don't currently support a ntohx or htonx
   set of functions for 64-bit values.  We've implemented our own here
   which is based off of GNU Net's implementation with some slight
   modifications (changed to macro's rather than functions). */
#if __BYTE_ORDER == __BIG_ENDIAN
#define ntohll(n) (n)
#define htonll(n) (n)
#else
#define ntohll(n) ( (((uint64_t)ntohl(n)) << 32) | ((uint64_t)ntohl((n) >> 32) & 0x00000000ffffffff) )
#define htonll(n) ( (((uint64_t)htonl(n)) << 32) | ((uint64_t)htonl((n) >> 32) & 0x00000000ffffffff) )
#endif

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

static int fb_serialized_size(CVarRef thing, int depth, int *bytes) {
  if (depth > 256) {
    return 1;
  }

  /* Get the size for an object, including one byte for the type */
  switch (thing.getType()) {
  case KindOfUninit:
  case KindOfNull:      *bytes = 1; break;     /* type */
  case KindOfBoolean:   *bytes = 2; break;    /* type + sizeof(char) */
  case KindOfInt64:     *bytes = 1 + INT_SIZE(thing.toInt64()); break;
  case KindOfDouble:    *bytes = 9; break;     /* type + sizeof(double) */
  case KindOfStaticString:
  case KindOfString:
    {
      int len = thing.toString().size();
      *bytes = 1 + LEN_SIZE(len) + len;
      break;
    }
  case KindOfArray:
    {
      int size = 2;
      Array arr = thing.toArray();
      for (ArrayIter iter(arr); iter; ++iter) {
        Variant key = iter.first();
        if (key.isNumeric()) {
          int64 index = key.toInt64();
          size += 1 + INT_SIZE(index);
        } else {
          int len = key.toString().size();
          size += 1 + LEN_SIZE(len) + len;
        }
        int additional_bytes = 0;
        if (fb_serialized_size(iter.second(), depth + 1,
                               &additional_bytes)) {
          return 1;
        }
        size += additional_bytes;
      }
      *bytes = size;
      break;
    }
  default:
    return 1;
  }
  return 0;
}

static void fb_serialize_long_into_buffer(int64 val, char *buff, int *pos) {
  switch (INT_SIZE(val)) {
  case 1:
    buff[(*pos)++] = T_BYTE;
    buff[(*pos)++] = (int8_t)val;
    break;
  case 2:
    buff[(*pos)++] = T_I16;
    *(int16_t *)(buff + (*pos)) = htons(val);
    (*pos) += 2;
    break;
  case 4:
    buff[(*pos)++] = T_I32;
    *(int32_t *)(buff + (*pos)) = htonl(val);
    (*pos) += 4;
    break;
  case 8:
    buff[(*pos)++] = T_I64;
    *(int64_t *)(buff + (*pos)) = htonll(val);
    (*pos) += 8;
    break;
  }
}

static void fb_serialize_string_into_buffer(CStrRef str, char *buf, int *pos) {
  int len = str.size();
  switch (LEN_SIZE(len)) {
  case 1:
    buf[(*pos)++] = T_VARCHAR;
    buf[(*pos)++] = (uint8_t)len;
    break;
  case 4:
    buf[(*pos)++] = T_STRING;
    *(uint32_t *)(buf + (*pos)) = htonl(len);
    (*pos) += 4;
    break;
  }

  /* memcpy the string into the buffer */
  memcpy(buf + (*pos), str.data(), len);
  (*pos) += len;
}

static bool fb_serialize_into_buffer(CVarRef thing, char *buff, int *pos) {
  switch (thing.getType()) {
  case KindOfNull:
    buff[(*pos)++] = T_NULL;
    break;
  case KindOfBoolean:
    buff[(*pos)++] = T_BOOLEAN;
    buff[(*pos)++] = (int8_t)thing.toInt64();
    break;
  case KindOfInt64:
    fb_serialize_long_into_buffer(thing.toInt64(), buff, pos);
    break;
  case KindOfDouble:
    buff[(*pos)++] = T_DOUBLE;
    *(double *)(buff + (*pos)) = thing.toDouble();
    (*pos) += 8;
    break;
  case KindOfStaticString:
  case KindOfString:
    fb_serialize_string_into_buffer(thing.toString(), buff, pos);
    break;
  case KindOfArray:
    {
      buff[(*pos)++] = T_STRUCT;
      Array arr = thing.toArray();
      for (ArrayIter iter(arr); iter; ++iter) {
        Variant key = iter.first();
        if (key.isNumeric()) {
          int64 index = key.toInt64();
          fb_serialize_long_into_buffer(index, buff, pos);
        } else {
          fb_serialize_string_into_buffer(key.toString(), buff, pos);
        }

        if (!fb_serialize_into_buffer(iter.second(), buff, pos)) {
          return false;
        }
      }

      /* Write the final stop marker */
      buff[(*pos)++] = T_STOP;
    }
    break;
  default:
    raise_warning("unserializable object unexpectedly passed through "
                  "fb_serialized_size");
    ASSERT(false);
  }
  return true;
}

/* Check if there are enough bytes left in the buffer */
#define CHECK_ENOUGH(bytes, pos, num) do {                  \
    if ((int)(bytes) > (int)((num) - (pos))) {              \
      return FB_UNSERIALIZE_UNEXPECTED_END;                 \
    }                                                       \
  } while (0)

int fb_unserialize_from_buffer(Variant &res, const char *buff,
                               int buff_len, int *pos) {

  /* Check we have at least 1 byte for the type */
  CHECK_ENOUGH(1, *pos, buff_len);

  int type;
  switch (type = buff[(*pos)++]) {
  case T_NULL:
    res = null;
    break;
  case T_BOOLEAN:
    CHECK_ENOUGH(sizeof(int8_t), *pos, buff_len);
    res = (bool)(int8_t)buff[(*pos)++];
    break;
  case T_BYTE:
    CHECK_ENOUGH(sizeof(int8_t), *pos, buff_len);
    res = (int8_t)buff[(*pos)++];
    break;
  case T_I16:
    {
      CHECK_ENOUGH(sizeof(int16_t), *pos, buff_len);
      int16_t ret = (int16_t)ntohs(*(int16_t *)(buff + (*pos)));
      (*pos) += 2;
      res = ret;
      break;
    }
  case T_I32:
    {
      CHECK_ENOUGH(sizeof(int32_t), *pos, buff_len);
      int32_t ret = (int32_t)ntohl(*(int32_t *)(buff + (*pos)));
      (*pos) += 4;
      res = ret;
      break;
    }
  case T_I64:
    {
      CHECK_ENOUGH(sizeof(int64_t), *pos, buff_len);
      int64_t ret = (int64_t)ntohll(*(int64_t *)(buff + (*pos)));
      (*pos) += 8;
      res = (int64)ret;
      break;
    }
  case T_DOUBLE:
    {
      CHECK_ENOUGH(sizeof(double), *pos, buff_len);
      double ret = *(double *)(buff + (*pos));
      (*pos) += 8;
      res = ret;
      break;
    }
  case T_VARCHAR:
    {
      CHECK_ENOUGH(sizeof(uint8_t), *pos, buff_len);
      int len = (uint8_t)buff[(*pos)++];

      CHECK_ENOUGH(len, *pos, buff_len);
      StringData* ret = NEW(StringData)(buff + (*pos), len, CopyString);
      (*pos) += len;
      res = ret;
      break;
    }
  case T_STRING:
    {
      CHECK_ENOUGH(sizeof(uint32_t), *pos, buff_len);
      int len = (uint32_t)ntohl(*(uint32_t *)(buff + (*pos)));
      (*pos) += 4;

      CHECK_ENOUGH(len, *pos, buff_len);
      StringData* ret = NEW(StringData)(buff + (*pos), len, CopyString);
      (*pos) += len;
      res = ret;
      break;
    }
  case T_STRUCT:
    {
      Array ret = Array::Create();
      /* Need at least 1 byte for type/stop */
      CHECK_ENOUGH(1, *pos, buff_len);
      while ((type = buff[(*pos)++]) != T_STOP) {
        String key;
        int64 index = 0;
        switch(type) {
        case T_BYTE:
          CHECK_ENOUGH(sizeof(int8_t), *pos, buff_len);
          index = (int8_t)buff[(*pos)++];
          break;
        case T_I16:
          {
            CHECK_ENOUGH(sizeof(int16_t), *pos, buff_len);
            index = (int16_t)ntohs(*(int16_t *)(buff + (*pos)));
            (*pos) += 2;
            break;
          }
        case T_I32:
          {
            CHECK_ENOUGH(sizeof(int32_t), *pos, buff_len);
            index = (int32_t)ntohl(*(int32_t *)(buff + (*pos)));
            (*pos) += 4;
            break;
          }
        case T_I64:
          {
            CHECK_ENOUGH(sizeof(int64_t), *pos, buff_len);
            index = (int64_t)ntohll(*(int64_t *)(buff + (*pos)));
            (*pos) += 8;
            break;
          }
        case T_VARCHAR:
          {
            CHECK_ENOUGH(sizeof(uint8_t), *pos, buff_len);
            int len = (uint8_t)buff[(*pos)++];

            CHECK_ENOUGH(len, *pos, buff_len);
            key = NEW(StringData)(buff + (*pos), len, CopyString);
            (*pos) += len;
            break;
          }
        case T_STRING:
          {
            CHECK_ENOUGH(sizeof(uint32_t), *pos, buff_len);
            int len = (uint32_t)ntohl(*(uint32_t *)(buff + (*pos)));
            (*pos) += 4;

            CHECK_ENOUGH(len, *pos, buff_len);
            key = NEW(StringData)(buff + (*pos), len, CopyString);
            (*pos) += len;
            break;
          }
        default:
          return FB_UNSERIALIZE_UNEXPECTED_ARRAY_KEY_TYPE;
        }

        Variant value;
        int retval;
        if ((retval = fb_unserialize_from_buffer(value, buff, buff_len, pos))) {
          return retval;
        }
        if (!key.isNull()) {
          ret.set(key, value);
        } else {
          ret.set(index, value);
        }
        /* Need at least 1 byte for type/stop (see start of loop) */
        CHECK_ENOUGH(1, *pos, buff_len);
      }
      res = ret;
    }
    break;
  default:
    return FB_UNSERIALIZE_UNRECOGNIZED_OBJECT_TYPE;
  }

  return 0;
}

Variant f_fb_thrift_serialize(CVarRef thing) {
  int len;
  if (fb_serialized_size(thing, 0, &len)) {
    return null;
  }
  String s(len, ReserveString);
  int pos = 0;
  fb_serialize_into_buffer(thing, s.mutableSlice().ptr, &pos);
  ASSERT(pos == len);
  return s.setSize(len);
}

int fb_compact_unserialize_from_buffer(
  Variant& out, const char* buf, int n, int& p);

Variant f_fb_thrift_unserialize(CVarRef thing, VRefParam success,
                                VRefParam errcode /* = null_variant */) {
  int pos = 0;
  errcode = null;
  int errcd;
  Variant ret;
  success = false;
  if (thing.isString()) {
    String sthing = thing.toString();
    // high bit set: it's a fb_compact_serialize'd string
    if (!sthing.empty() && (sthing[0] & 0x80)) {
      errcd = fb_compact_unserialize_from_buffer(
        ret, sthing.data(), sthing.size(), pos);
    } else {
      errcd = fb_unserialize_from_buffer(
        ret, sthing.data(), sthing.size(), &pos);
    }
    if (errcd) {
      errcode = errcd;
    } else {
      success = true;
      return ret;
    }
  } else {
    errcode = FB_UNSERIALIZE_NONSTRING_VALUE;
  }
  return false;
}

Variant f_fb_serialize(CVarRef thing) {
  return f_fb_thrift_serialize(thing);
}

Variant f_fb_unserialize(CVarRef thing, VRefParam success,
                         VRefParam errcode /* = null_variant */) {
  return f_fb_thrift_unserialize(thing, ref(success), ref(errcode));
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
  FB_CS_MAX_CODE   = 15,
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


static void fb_compact_serialize_code(
  StringData* sd, FbCompactSerializeCode code) {

  ASSERT(code == (code & kCodeMask));
  uint8_t v = (kCodePrefix | code);
  sd->append(reinterpret_cast<char*>(&v), 1);
}

static void fb_compact_serialize_int64(StringData* sd, int64_t val) {
  if (val >= 0 && (uint64_t)val <= kInt7Mask) {
    uint8_t nval = val;
    sd->append(reinterpret_cast<char*>(&nval), 1);

  } else if (val >= 0 && (uint64_t)val <= kInt13Mask) {
    uint16_t nval = htons(kInt13Prefix | val);
    sd->append(reinterpret_cast<char*>(&nval), 2);

  } else if (val == (int64_t)(int16_t)val) {
    fb_compact_serialize_code(sd, FB_CS_INT16);
    uint16_t nval = htons(val);
    sd->append(reinterpret_cast<char*>(&nval), 2);

  } else if (val >= 0 && (uint64_t)val <= kInt20Mask) {
    uint32_t nval = htonl(kInt20Prefix | val);
    // Skip most significant byte
    sd->append(reinterpret_cast<char*>(&nval) + 1, 3);

  } else if (val == (int64_t)(int32_t)val) {
    fb_compact_serialize_code(sd, FB_CS_INT32);
    uint32_t nval = htonl(val);
    sd->append(reinterpret_cast<char*>(&nval), 4);

  } else if (val >= 0 && (uint64_t)val <= kInt54Mask) {
    uint64_t nval = htonll(kInt54Prefix | val);
    // Skip most significant byte
    sd->append(reinterpret_cast<char*>(&nval) + 1, 7);

  } else {
    fb_compact_serialize_code(sd, FB_CS_INT64);
    uint64_t nval = htonll(val);
    sd->append(reinterpret_cast<char*>(&nval), 8);
  }
}

static void fb_compact_serialize_string(StringData* sd, CStrRef str) {
  int len = str.size();
  if (len == 0) {
    fb_compact_serialize_code(sd, FB_CS_STRING_0);
  } else {
    if (len == 1) {
      fb_compact_serialize_code(sd, FB_CS_STRING_1);
    } else {
      fb_compact_serialize_code(sd, FB_CS_STRING_N);
      fb_compact_serialize_int64(sd, len);
    }
    sd->append(str.data(), len);
  }
}

static bool fb_compact_serialize_is_list(CArrRef arr, int64_t& index_limit) {
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

static int fb_compact_serialize_variant(StringData* sd, CVarRef var, int depth);

static void fb_compact_serialize_array_as_list_map(
  StringData* sd, CArrRef arr, int64_t index_limit, int depth) {

  fb_compact_serialize_code(sd, FB_CS_LIST_MAP);
  for (int64 i = 0; i < index_limit; ++i) {
    if (arr.exists(i)) {
      fb_compact_serialize_variant(sd, arr[i], depth + 1);
    } else {
      fb_compact_serialize_code(sd, FB_CS_SKIP);
    }
  }
  fb_compact_serialize_code(sd, FB_CS_STOP);
}

static void fb_compact_serialize_array_as_map(
  StringData* sd, CArrRef arr, int depth) {

  fb_compact_serialize_code(sd, FB_CS_MAP);
  for (ArrayIter it(arr); it; ++it) {
    Variant key = it.first();
    if (key.isNumeric()) {
      fb_compact_serialize_int64(sd, key.toInt64());
    } else {
      fb_compact_serialize_string(sd, key.toString());
    }
    fb_compact_serialize_variant(sd, it.second(), depth + 1);
  }
  fb_compact_serialize_code(sd, FB_CS_STOP);
}


static int fb_compact_serialize_variant(
  StringData* sd, CVarRef var, int depth) {

  if (depth > 256) {
    return 1;
  }

  switch (var.getType()) {
    case KindOfUninit:
    case KindOfNull:
      fb_compact_serialize_code(sd, FB_CS_NULL);
      break;

    case KindOfBoolean:
      if (var.toInt64()) {
        fb_compact_serialize_code(sd, FB_CS_TRUE);
      } else {
        fb_compact_serialize_code(sd, FB_CS_FALSE);
      }
      break;

    case KindOfInt64:
      fb_compact_serialize_int64(sd, var.toInt64());
      break;

    case KindOfDouble:
    {
      fb_compact_serialize_code(sd, FB_CS_DOUBLE);
      double d = var.toDouble();
      sd->append(reinterpret_cast<char*>(&d), 8);
      break;
    }

    case KindOfStaticString:
    case KindOfString:
      fb_compact_serialize_string(sd, var.toString());
      break;

    case KindOfArray:
    {
      Array arr = var.toArray();
      int64_t index_limit;
      if (fb_compact_serialize_is_list(arr, index_limit)) {
        fb_compact_serialize_array_as_list_map(sd, arr, index_limit, depth);
      } else {
        fb_compact_serialize_array_as_map(sd, arr, depth);
      }
      break;
    }

    default:
      return 1;
  }

  return 0;
}

Variant f_fb_compact_serialize(CVarRef thing) {
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
      *(uint16_t*)(s.mutableSlice().ptr) = (uint16_t)htons(kInt13Prefix | val);
      return s.setSize(2);
    }
  }

  StringData* sd = NEW(StringData);
  // StringData will throw a FatalErrorException if we try to grow it too large,
  // so no need to check for length.
  if (fb_compact_serialize_variant(sd, thing, 0)) {
    DELETE(StringData)(sd);
    return null;
  }

  return Variant(sd);
}

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
    char b[4];
    memcpy(b, buf + p, 3);
    uint32_t val = (uint32_t)ntohl(*reinterpret_cast<const uint32_t*>(b));
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
    char b[8];
    memcpy(b, buf + p, 7);
    uint64_t val = (uint64_t)ntohll(*reinterpret_cast<const uint64_t*>(b));
    p += 7;
    out = (val >> 8) & kInt54Mask;

  } else if (first == (kCodePrefix | FB_CS_INT64)) {
    p += 1;
    CHECK_ENOUGH(8, p, n);
    int64 val = (int64_t)ntohll(*reinterpret_cast<const int64_t*>(buf + p));
    p += 8;
    out = val;

  } else {
    return FB_UNSERIALIZE_UNRECOGNIZED_OBJECT_TYPE;
  }

  return 0;
}

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
    out = (int64)val;
    return 0;
  }
  p += 1;
  code &= kCodeMask;
  switch (code) {
    case FB_CS_NULL:
      out = null;
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
      StringData* sd = NEW(StringData);
      out = sd;
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
      StringData* sd = NEW(StringData)(buf + p, len, CopyString);
      p += len;
      out = sd;
      break;
    }

    case FB_CS_LIST_MAP:
    case FB_CS_VECTOR:
    {
      // There's no concept of vector in PHP (yet),
      // so return an array in both cases
      Array arr = Array::Create();
      int64 i = 0;
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
                   key.getType() == KindOfStaticString) {
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

Variant f_fb_compact_unserialize(CVarRef thing, VRefParam success, VRefParam errcode /* = null_variant */) {

  if (!thing.isString()) {
    success = false;
    errcode = FB_UNSERIALIZE_NONSTRING_VALUE;
    return false;
  }
  Variant ret;
  String s = thing.toString();
  int p = 0;
  int err = fb_compact_unserialize_from_buffer(ret, s.data(), s.size(), p);
  if (err) {
    success = false;
    errcode = err;
    return false;
  }
  success = true;
  errcode = null;
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

static void output_dataset(Array &ret, int affected, DBDataSet &ds,
                           const DBConn::ErrorInfoMap &errors) {
  ret.set("affected", affected);

  Array rows;
  MYSQL_FIELD *fields = ds.getFields();
  for (ds.moveFirst(); ds.getRow(); ds.moveNext()) {
    Array row;
    for (int i = 0; i < ds.getColCount(); i++) {
      const char *field = ds.getField(i);
      int len = ds.getFieldLength(i);
      row.set(String(fields[i].name, CopyString),
              mysql_makevalue(String(field, len, CopyString), fields + i));
    }
    rows.append(row);
  }
  ret.set("result", rows);

  if (!errors.empty()) {
    Array error, codes;
    for (DBConn::ErrorInfoMap::const_iterator iter = errors.begin();
         iter != errors.end(); ++iter) {
      error.set(iter->first, String(iter->second.msg));
      codes.set(iter->first, iter->second.code);
    }
    ret.set("error", error);
    ret.set("errno", codes);
  }
}

void f_fb_load_local_databases(CArrRef servers) {
  DBConn::ClearLocalDatabases();
  for (ArrayIter iter(servers); iter; ++iter) {
    int dbId = iter.first().toInt32();
    Array data = iter.second().toArray();
    if (!data.empty()) {
      std::vector< std::pair<string, string> > sessionVariables;
      if (data.exists("session_variable")) {
        Array sv = data["session_variable"].toArray();
        for (ArrayIter svIter(sv); svIter; ++svIter) {
          sessionVariables.push_back(std::pair<string, string>(
            svIter.first().toString().data(),
            svIter.second().toString().data()));
        }
      }
      DBConn::AddLocalDB(dbId, data["ip"].toString().data(),
                         data["db"].toString().data(),
                         data["port"].toInt32(),
                         data["username"].toString().data(),
                         data["password"].toString().data(),
                         sessionVariables);
    }
  }
}

Array f_fb_parallel_query(CArrRef sql_map, int max_thread /* = 50 */,
                          bool combine_result /* = true */,
                          bool retry_query_on_fail /* = true */,
                          int connect_timeout /* = -1 */,
                          int read_timeout /* = -1 */,
                          bool timeout_in_ms /* = false */) {
  if (!timeout_in_ms) {
    if (connect_timeout > 0) connect_timeout *= 1000;
    if (read_timeout > 0) read_timeout *= 1000;
  }

  ServerQueryVec queries;
  for (ArrayIter iter(sql_map); iter; ++iter) {
    Array data = iter.second().toArray();
    if (!data.empty()) {
      std::vector< std::pair<string, string> > sessionVariables;
      if (data.exists("session_variable")) {
        Array sv = data["session_variable"].toArray();
        for (ArrayIter svIter(sv); svIter; ++svIter) {
          sessionVariables.push_back(std::pair<string, string>(
            svIter.first().toString().data(),
            svIter.second().toString().data()));
        }
      }
      ServerDataPtr server
        (new ServerData(data["ip"].toString().data(),
                        data["db"].toString().data(),
                        data["port"].toInt32(),
                        data["username"].toString().data(),
                        data["password"].toString().data(),
                        sessionVariables));
      queries.push_back(ServerQuery(server, data["sql"].toString().data()));
    } else {
      // so we can report errors according to array index
      queries.push_back(ServerQuery(ServerDataPtr(), ""));
    }
  }

  Array ret;
  if (combine_result) {
    DBDataSet ds;
    DBConn::ErrorInfoMap errors;
    int affected = DBConn::parallelExecute(queries, ds, errors, max_thread,
                     retry_query_on_fail,
                     connect_timeout, read_timeout,
                     RuntimeOption::MySQLMaxRetryOpenOnFail,
                     RuntimeOption::MySQLMaxRetryQueryOnFail);
    output_dataset(ret, affected, ds, errors);
  } else {
    DBDataSetPtrVec dss(queries.size());
    for (unsigned int i = 0; i < dss.size(); i++) {
      dss[i] = DBDataSetPtr(new DBDataSet());
    }

    DBConn::ErrorInfoMap errors;
    int affected = DBConn::parallelExecute(queries, dss, errors, max_thread,
                     retry_query_on_fail,
                     connect_timeout, read_timeout,
                     RuntimeOption::MySQLMaxRetryOpenOnFail,
                     RuntimeOption::MySQLMaxRetryQueryOnFail);
    for (unsigned int i = 0; i < dss.size(); i++) {
      Array dsRet;
      output_dataset(dsRet, affected, *dss[i], errors);
      ret.append(dsRet);
    }
  }
  return ret;
}

Array f_fb_crossall_query(CStrRef sql, int max_thread /* = 50 */,
                          bool retry_query_on_fail /* = true */,
                          int connect_timeout /* = -1 */,
                          int read_timeout /* = -1 */,
                          bool timeout_in_ms /* = false */) {
  if (!timeout_in_ms) {
    if (connect_timeout > 0) connect_timeout *= 1000;
    if (read_timeout > 0) read_timeout *= 1000;
  }

  Array ret;
  // parameter checking
  if (!sql || !*sql) {
    ret.set("error", "empty SQL");
    return ret;
  }

  // security checking
  String ssql = StringUtil::ToLower(sql);
  if (ssql.find("where") < 0) {
    ret.set("error", "missing where clause");
    return ret;
  }
  if (ssql.find("select") < 0) {
    ret.set("error", "non-SELECT not supported");
    return ret;
  }

  // do it
  DBDataSet ds;
  DBConn::ErrorInfoMap errors;
  int affected = DBConn::parallelExecute(ssql.c_str(), ds, errors, max_thread,
                     retry_query_on_fail,
                     connect_timeout, read_timeout,
                     RuntimeOption::MySQLMaxRetryOpenOnFail,
                     RuntimeOption::MySQLMaxRetryQueryOnFail);
  output_dataset(ret, affected, ds, errors);
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool f_fb_utf8ize(VRefParam input) {
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
  char *dstBuf = dstStr.mutableSlice().ptr;

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
  input = dstStr.setSize(dstPosBytes);
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
static int f_fb_utf8_strlen_impl(CStrRef input, bool deprecated) {
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

int64 f_fb_utf8_strlen(CStrRef input) {
  return f_fb_utf8_strlen_impl(input, /* deprecated */ false);
}

int64 f_fb_utf8_strlen_deprecated(CStrRef input) {
  return f_fb_utf8_strlen_impl(input, /* deprecated */ true);
}

/**
 * Private helper; requires non-negative firstCodePoint and desiredCodePoints.
 */
static Variant f_fb_utf8_substr_simple(CStrRef str, int32_t firstCodePoint,
                                       int32_t numDesiredCodePoints) {
  const char* const srcBuf = str.data();
  int32_t srcLenBytes = str.size(); // May truncate; checked before use below.

  ASSERT(firstCodePoint >= 0);  // Wrapper fixes up negative starting positions.
  ASSERT(numDesiredCodePoints > 0); // Wrapper fixes up negative/zero length.
  if (str.size() <= 0 ||
      str.size() > INT_MAX ||
      firstCodePoint >= srcLenBytes) {
    return false;
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
    return false; // Too long.
  }
  String dstStr(dstMaxLenBytes, ReserveString);
  char* dstBuf = dstStr.mutableSlice().ptr;
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

  if (dstPosBytes > 0) {
    return dstStr.setSize(dstPosBytes);
  }
  return false;
}

Variant f_fb_utf8_substr(CStrRef str, int start, int length /* = INT_MAX */) {
  // For negative start or length, calculate start and length values
  // based on total code points.
  if (start < 0 || length < 0) {
    // Get number of code points assuming we substitute invalid sequences.
    Variant utf8StrlenResult = f_fb_utf8_strlen(str);
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
    return false; // Empty result
  }

  return f_fb_utf8_substr_simple(str, start, length);
}

///////////////////////////////////////////////////////////////////////////////

bool f_fb_could_include(CStrRef file) {
  if (hhvm) {
    struct stat s;
    return !Eval::resolveVmInclude(file.get(), "", &s).isNull();
  }
  return !resolve_include(file, "", hphp_could_invoke_file, NULL).isNull();
}

bool f_fb_intercept(CStrRef name, CVarRef handler,
                    CVarRef data /* = null_variant */) {
  return register_intercept(name, handler, data);
}

Variant f_fb_stubout_intercept_handler(CStrRef name, CVarRef obj,
                                       CArrRef params, CVarRef data,
                                       VRefParam done) {
  if (obj.isNull()) {
    return f_call_user_func_array(data, params);
  }
  return f_call_user_func_array(CREATE_VECTOR2(obj, data), params);
}

Variant f_fb_rpc_intercept_handler(CStrRef name, CVarRef obj, CArrRef params,
                                   CVarRef data, VRefParam done) {
  String host = data["host"].toString();
  int port = data["port"].toInt32();
  String auth = data["auth"].toString();
  int timeout = data["timeout"].toInt32();

  if (obj.isNull()) {
    return f_call_user_func_array_rpc(host, port, auth, timeout, name, params);
  }
  return f_call_user_func_array_rpc(host, port, auth, timeout,
                                    CREATE_VECTOR2(obj, name), params);
}

void f_fb_renamed_functions(CArrRef names) {
  check_renamed_functions(names);
}

bool f_fb_rename_function(CStrRef orig_func_name, CStrRef new_func_name) {
  if (orig_func_name.empty() || new_func_name.empty() ||
      orig_func_name->isame(new_func_name.get())) {
    throw_invalid_argument("unable to rename %s", orig_func_name.data());
    return false;
  }

  if (!function_exists(orig_func_name)) {
    raise_warning("fb_rename_function(%s, %s) failed: %s does not exists!",
                  orig_func_name.data(), new_func_name.data(),
                  orig_func_name.data());
    return false;
  }

  if (function_exists(new_func_name)) {
    if (new_func_name.data()[0] !=
        ParserBase::CharCreateFunction) { // create_function
      raise_warning("fb_rename_function(%s, %s) failed: %s already exists!",
                    orig_func_name.data(), new_func_name.data(),
                    new_func_name.data());
      return false;
    }
  }

  if (!check_renamed_function(orig_func_name) &&
      !check_renamed_function(new_func_name)) {
    raise_error("fb_rename_function(%s, %s) failed: %s is not allowed to "
                "rename. Please add it to the list provided to "
                "fb_renamed_functions().",
                orig_func_name.data(), new_func_name.data(),
                orig_func_name.data());
    return false;
  }

  rename_function(orig_func_name, new_func_name);
  return true;
}

/*
  fb_autoload_map($map, $root) specifies a mapping
  from classes, functions and constants to the files
  that define them. The map has the form:

    array('class'    => array('cls' => 'cls_file.php', ...),
          'function' => array('fun' => 'fun_file.php', ...),
          'constant' => array('con' => 'con_file.php', ...),
          'failure' => callable);

    If the 'failure' element exists, it will be called if the
    lookup in the map fails, or the file cant be included. It
    takes a kind ('class', 'function' or 'constant') and the
    name of the entity we're trying to autoload.

  If $root is non empty, it is prepended to every filename
  (so will typically need to end with '/').
*/

bool f_fb_autoload_map(CVarRef map, CStrRef root) {
  if (map.isArray()) {
    return AutoloadHandler::s_instance->setMap(map.toCArrRef(), root);
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// call_user_func extensions

Array f_fb_call_user_func_safe(int _argc, CVarRef function,
                               CArrRef _argv /* = null_array */) {
  return f_fb_call_user_func_array_safe(function, _argv);
}

static Variant fb_call_user_func_safe(CVarRef function, CArrRef params,
                                      bool &ok) {
  MethodCallPackage mcp;
  String classname, methodname;
  bool doBind;
  if (get_callable_user_func_handler(function,
                                     mcp, classname, methodname, doBind)) {
    ok = true;
    if (doBind) {
      FrameInjection::StaticClassNameHelper scn(
        ThreadInfo::s_threadInfo.getNoCheck(), classname);
      ASSERT(!mcp.m_isFunc);
      return mcp.ci->getMeth()(mcp, params);
    } else {
      if (mcp.m_isFunc) {
        return mcp.ci->getFunc()(mcp.extra, params);
      } else {
        return mcp.ci->getMeth()(mcp, params);
      }
    }
  }
  ok = false;
  return null;
}

Variant f_fb_call_user_func_safe_return(int _argc, CVarRef function,
                                        CVarRef def,
                                        CArrRef _argv /* = null_array */) {
  if (hhvm) {
    if (f_is_callable(function)) {
      return f_call_user_func_array(function, _argv);
    }
    return def;
  } else {
    bool ok;
    Variant ret = fb_call_user_func_safe(function, _argv, ok);
    return ok ? ret : def;
  }
}

Array f_fb_call_user_func_array_safe(CVarRef function, CArrRef params) {
  if (hhvm) {
    if (f_is_callable(function)) {
      return CREATE_VECTOR2(true, f_call_user_func_array(function, params));
    }
    return CREATE_VECTOR2(false, null);
  } else {
    bool ok;
    Variant ret = fb_call_user_func_safe(function, params, ok);
    return CREATE_VECTOR2(ok, ret);
  }
}

///////////////////////////////////////////////////////////////////////////////

Variant f_fb_get_code_coverage(bool flush) {
  ThreadInfo *ti = ThreadInfo::s_threadInfo.getNoCheck();
  if (ti->m_reqInjectionData.coverage) {
    Array ret = ti->m_coverage->Report();
    if (flush) {
      ti->m_coverage->Reset();
    }
    return ret;
  }
  return false;
}

void f_fb_enable_code_coverage() {
  ThreadInfo *ti = ThreadInfo::s_threadInfo.getNoCheck();
  ti->m_coverage->Reset();
  ti->m_reqInjectionData.coverage = true;
  if (hhvm) {
    if (g_vmContext->isNested()) {
      raise_notice("Calling fb_enable_code_coverage from a nested "
                   "VM instance may cause unpredicable results");
    }
    throw VMSwitchModeException(true);
  }
}

Variant f_fb_disable_code_coverage() {
  ThreadInfo *ti = ThreadInfo::s_threadInfo.getNoCheck();
  ti->m_reqInjectionData.coverage = false;
  Array ret = ti->m_coverage->Report();
  ti->m_coverage->Reset();
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

void f_fb_set_taint(VRefParam str, int taint) {
#ifdef TAINTED
  if (!str.isString()) {
    return;
  }

  StringData *sd = str.getStringData();
  ASSERT(sd);
  if (sd->getCount() > 1) {
    // Pass taint to our copy.
    TAINT_OBSERVER(TAINT_BIT_NONE, TAINT_BIT_NONE);
    str = NEW(StringData)(sd->data(), sd->size(), CopyString);
  }

  str.getStringData()->getTaintDataRef().setTaint(taint);
#endif
}

void f_fb_unset_taint(VRefParam str, int taint) {
#ifdef TAINTED
  if (!str.isString()) {
    return;
  }

  StringData *sd = str.getStringData();
  ASSERT(sd);
  if (sd->getCount() > 1) {
    // Pass taint to our copy.
    TAINT_OBSERVER(TAINT_BIT_NONE, TAINT_BIT_NONE);
    str = NEW(StringData)(sd->data(), sd->size(), CopyString);
  }

  str.getStringData()->getTaintDataRef().unsetTaint(taint);
#endif
}

bool f_fb_get_taint(CStrRef str, int taint) {
#ifdef TAINTED
  StringData *string_data = str.get();
  ASSERT(string_data);
  return string_data->getTaintDataRefConst().getTaint() & taint;
#else
  return false;
#endif
}

Array f_fb_get_taint_warning_counts() {
#ifdef TAINTED
  return TaintWarning::GetCounts();
#else
  Array counts;
  counts.set(TAINT_BIT_HTML, 0);
  counts.set(TAINT_BIT_MUTATED, 0);
  counts.set(TAINT_BIT_SQL, 0);
  counts.set(TAINT_BIT_SHELL, 0);
  counts.set(TAINT_BIT_ALL, 0);
  return counts;
#endif
}

void f_fb_enable_html_taint_trace() {
#ifdef TAINTED
  TaintTracer::SwitchTrace(TAINT_BIT_TRACE_HTML, true);
#endif
}

bool f_fb_output_compression(bool new_value) {
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

void f_fb_set_exit_callback(CVarRef function) {
  g_context->setExitCallback(function);
}

Array f_fb_get_flush_stat() {
  Transport *transport = g_context->getTransport();
  if (transport) {
    Array chunkStats(ArrayData::Create());
    transport->getChunkSentSizes(chunkStats);

    int total = transport->getResponseTotalSize();
    int sent = transport->getResponseSentSize();
    int64 time = transport->getFlushTime();
    return CREATE_MAP2(
        "flush_stats", CREATE_MAP3("total", total, "sent", sent, "time", time),
        "chunk_stats", chunkStats);
  }
  return NULL;
}

int64 f_fb_get_last_flush_size() {
  Transport *transport = g_context->getTransport();
  return transport ? transport->getLastChunkSentSize() : 0;
}

extern Array stat_impl(struct stat*); // ext_file.cpp

template<class Function>
static Variant do_lazy_stat(Function dostat, CStrRef filename) {
  struct stat sb;
  if (dostat(File::TranslatePath(filename, true).c_str(), &sb)) {
    Logger::Verbose("%s/%d: %s", __FUNCTION__, __LINE__,
                    Util::safe_strerror(errno).c_str());
    return false;
  }
  return stat_impl(&sb);
}

Variant f_fb_lazy_stat(CStrRef filename) {
  return do_lazy_stat(StatCache::stat, filename);
}

Variant f_fb_lazy_lstat(CStrRef filename) {
  return do_lazy_stat(StatCache::lstat, filename);
}

String f_fb_lazy_realpath(CStrRef filename) {
  return StatCache::realpath(filename.c_str());
}

String f_fb_gc_collect_cycles() {
  std::string s = VM::gc_collect_cycles();
  return String(s);
}

void f_fb_gc_detect_cycles(CStrRef filename) {
  VM::gc_detect_cycles(std::string(filename.c_str()));
}

///////////////////////////////////////////////////////////////////////////////
// const index functions

static Array const_data;

Variant f_fb_const_fetch(CVarRef key) {
  String k = key.toString();
  Variant *ret = const_data.lvalPtr(k, false, false);
  if (ret) return *ret;
  return false;
}

void const_load_set(CStrRef key, CVarRef value) {
  const_data.set(key, value, true);
}

KEEP_SECTION
void const_load() {
  // after all loading
  const_load_set("zend_array_size", const_data.size());
  const_data.setEvalScalar();
}

bool const_dump(const char *filename) {
  std::ofstream out(filename);
  if (out.fail()) {
    return false;
  }
  const_data->dump(out);
  out.close();
  return true;
}

///////////////////////////////////////////////////////////////////////////////
}
