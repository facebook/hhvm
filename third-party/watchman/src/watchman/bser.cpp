/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/bser.h"
#include "watchman/Logging.h"
#include "watchman/thirdparty/jansson/jansson_private.h"

/*
 * This defines a binary serialization of the JSON data objects in this
 * library.  It is designed for use with watchman and is not intended to serve
 * as a general binary JSON interchange format.  In particular, all integers
 * are signed integers and are stored in host byte order to minimize
 * transformation overhead.
 */

/* Return the smallest size int that can store the value */
#define INT_SIZE(x)                \
  (((x) == ((int8_t)x))        ? 1 \
       : ((x) == ((int16_t)x)) ? 2 \
       : ((x) == ((int32_t)x)) ? 4 \
                               : 8)

#define BSER_ARRAY 0x00
#define BSER_OBJECT 0x01
#define BSER_BYTESTRING 0x02
#define BSER_INT8 0x03
#define BSER_INT16 0x04
#define BSER_INT32 0x05
#define BSER_INT64 0x06
#define BSER_REAL 0x07
#define BSER_TRUE 0x08
#define BSER_FALSE 0x09
#define BSER_NULL 0x0a
#define BSER_TEMPLATE 0x0b
#define BSER_SKIP 0x0c
#define BSER_UTF8STRING 0x0d

static const char bser_true = BSER_TRUE;
static const char bser_false = BSER_FALSE;
static const char bser_null = BSER_NULL;
static const char bser_bytestring_hdr = BSER_BYTESTRING;
static const char bser_array_hdr = BSER_ARRAY;
static const char bser_object_hdr = BSER_OBJECT;
static const char bser_template_hdr = BSER_TEMPLATE;
static const char bser_utf8string_hdr = BSER_UTF8STRING;
static const char bser_skip = BSER_SKIP;

static constexpr size_t kMaximumContainerSize =
    std::numeric_limits<uint32_t>::max();

static bool is_bser_version_supported(const bser_ctx_t* ctx) {
  return ctx->bser_version == 1 || ctx->bser_version == 2;
}

static int bser_real(const bser_ctx_t* ctx, double val, void* data) {
  char sz = BSER_REAL;
  if (!is_bser_version_supported(ctx)) {
    return -1;
  }

  if (ctx->dump(&sz, sizeof(sz), data)) {
    return -1;
  }
  return ctx->dump((char*)&val, sizeof(val), data);
}

bool bunser_generic_string(
    const char* buf,
    json_int_t avail,
    json_int_t* needed,
    const char** start,
    json_int_t* len) {
  if (avail == 0) {
    return false;
  }

  assert(*buf == BSER_BYTESTRING || *buf == BSER_UTF8STRING);

  ++buf;
  --avail;

  size_t ineed;
  std::optional<json_int_t> string_length = bunser_int(buf, avail, &ineed);
  if (!string_length) {
    *needed = ineed;
    return false;
  }
  *len = string_length.value();

  buf += ineed;
  avail -= ineed;
  if (*len > avail) {
    return false;
  }

  *needed = ineed + 1 + *len;
  *start = buf;
  return true;
}

std::optional<json_int_t>
bunser_int(const char* buf, size_t avail, size_t* needed) {
  if (avail == 0) {
    *needed = 1;
    return std::nullopt;
  }

  switch (buf[0]) {
    case BSER_INT8:
      *needed = 2;
      if (avail < 2) {
        return std::nullopt;
      } else {
        int8_t i8;
        memcpy(&i8, buf + 1, sizeof(i8));
        return i8;
      }
    case BSER_INT16:
      *needed = 3;
      if (avail < 3) {
        return std::nullopt;
      } else {
        int16_t i16;
        memcpy(&i16, buf + 1, sizeof(i16));
        return i16;
      }
    case BSER_INT32:
      *needed = 5;
      if (avail < 5) {
        return std::nullopt;
      } else {
        int32_t i32;
        memcpy(&i32, buf + 1, sizeof(i32));
        return i32;
      }
    case BSER_INT64:
      *needed = 9;
      if (avail < 9) {
        return std::nullopt;
      } else {
        int64_t i64;
        memcpy(&i64, buf + 1, sizeof(i64));
        return i64;
      }
    default:
      *needed = kDecodeIntFailed;
      return std::nullopt;
  }
}

static int bser_int(const bser_ctx_t* ctx, json_int_t val, void* data) {
  int8_t i8;
  int16_t i16;
  int32_t i32;
  int64_t i64;
  char sz;
  int size = INT_SIZE(val);
  char* iptr;

  if (!is_bser_version_supported(ctx)) {
    return -1;
  }

  switch (size) {
    case 1:
      sz = BSER_INT8;
      i8 = (int8_t)val;
      iptr = (char*)&i8;
      break;
    case 2:
      sz = BSER_INT16;
      i16 = (int16_t)val;
      iptr = (char*)&i16;
      break;
    case 4:
      sz = BSER_INT32;
      i32 = (int32_t)val;
      iptr = (char*)&i32;
      break;
    case 8:
      sz = BSER_INT64;
      i64 = (int64_t)val;
      iptr = (char*)&i64;
      break;
    default:
      return -1;
  }

  if (ctx->dump(&sz, sizeof(sz), data)) {
    return -1;
  }

  return ctx->dump(iptr, size, data);
}

static int bser_generic_string(
    const bser_ctx_t* ctx,
    w_string_piece str,
    void* data,
    const char hdr) {
  if (!is_bser_version_supported(ctx)) {
    return -1;
  }

  if (ctx->dump(&hdr, sizeof(hdr), data)) {
    return -1;
  }

  if (bser_int(ctx, str.size(), data)) {
    return -1;
  }

  if (ctx->dump(str.data(), str.size(), data)) {
    return -1;
  }

  return 0;
}

static int
bser_bytestring(const bser_ctx_t* ctx, w_string_piece str, void* data) {
  return bser_generic_string(ctx, str, data, bser_bytestring_hdr);
}

static int
bser_utf8string(const bser_ctx_t* ctx, w_string_piece str, void* data) {
  if ((ctx->bser_capabilities & BSER_CAP_DISABLE_UNICODE) ||
      ctx->bser_version == 1) {
    return bser_bytestring(ctx, str, data);
  }
  return bser_generic_string(ctx, str, data, bser_utf8string_hdr);
}

static int
bser_mixedstring(const bser_ctx_t* ctx, w_string_piece str, void* data) {
  if (ctx->bser_version != 1 &&
      !(BSER_CAP_DISABLE_UNICODE_FOR_ERRORS & ctx->bser_capabilities) &&
      !(BSER_CAP_DISABLE_UNICODE & ctx->bser_capabilities)) {
    auto utf8_clean = str.asUTF8Clean();
    return bser_utf8string(ctx, utf8_clean, data);
  } else {
    return bser_bytestring(ctx, str, data);
  }
}

static int bser_array(const bser_ctx_t* ctx, const json_ref& array, void* data);

static int bser_template(
    const bser_ctx_t* ctx,
    const json_ref& array,
    const json_ref& templ,
    void* data) {
  size_t n = json_array_size(array);

  if (!is_bser_version_supported(ctx)) {
    return -1;
  }

  if (ctx->dump(&bser_template_hdr, sizeof(bser_template_hdr), data)) {
    return -1;
  }

  // The template goes next
  if (bser_array(ctx, templ, data)) {
    return -1;
  }

  // Now the array of arrays of object values.
  // How many objects
  if (bser_int(ctx, n, data)) {
    return -1;
  }

  auto& array_arr = array.array();
  auto& templ_arr = templ.array();
  size_t pn = templ_arr.size();

  // For each object
  for (size_t i = 0; i < n; i++) {
    auto& obj = array_arr[i];

    // For each factored key
    for (size_t pi = 0; pi < pn; pi++) {
      const char* key = json_string_value(templ_arr[pi]);

      // Look up the object property
      auto val = json_object_get(obj, key);
      if (!val) {
        // property not set on this one; emit a skip
        if (ctx->dump(&bser_skip, sizeof(bser_skip), data)) {
          return -1;
        }
        continue;
      }

      // Emit value
      if (w_bser_dump(ctx, *val, data)) {
        return -1;
      }
    }
  }

  return 0;
}

static int
bser_array(const bser_ctx_t* ctx, const json_ref& array, void* data) {
  if (!is_bser_version_supported(ctx)) {
    return -1;
  }

  auto templ = json_array_get_template(array);
  if (templ) {
    return bser_template(ctx, array, *templ, data);
  }

  if (ctx->dump(&bser_array_hdr, sizeof(bser_array_hdr), data)) {
    return -1;
  }

  auto& arr = array.array();
  if (bser_int(ctx, arr.size(), data)) {
    return -1;
  }

  for (auto& val : arr) {
    if (w_bser_dump(ctx, val, data)) {
      return -1;
    }
  }

  return 0;
}

static int bser_object(const bser_ctx_t* ctx, const json_ref& obj, void* data) {
  size_t n;

  if (!is_bser_version_supported(ctx)) {
    return -1;
  }

  if (ctx->dump(&bser_object_hdr, sizeof(bser_object_hdr), data)) {
    return -1;
  }

  n = json_object_size(obj);
  if (bser_int(ctx, n, data)) {
    return -1;
  }

  for (auto& it : obj.object()) {
    auto& key = it.first;
    auto& val = it.second;

    if (bser_bytestring(ctx, key.c_str(), data)) {
      return -1;
    }
    if (w_bser_dump(ctx, val, data)) {
      return -1;
    }
  }

  return 0;
}

int w_bser_dump(const bser_ctx_t* ctx, const json_ref& json, void* data) {
  if (!is_bser_version_supported(ctx)) {
    return -1;
  }

  switch (json.type()) {
    case JSON_NULL:
      return ctx->dump(&bser_null, sizeof(bser_null), data);
    case JSON_TRUE:
      return ctx->dump(&bser_true, sizeof(bser_true), data);
    case JSON_FALSE:
      return ctx->dump(&bser_false, sizeof(bser_false), data);
    case JSON_REAL:
      return bser_real(ctx, json_real_value(json), data);
    case JSON_INTEGER:
      return bser_int(ctx, json.asInt(), data);
    case JSON_STRING: {
      auto& wstr = json_to_w_string(json);
      switch (wstr.type()) {
        case W_STRING_BYTE:
          return bser_bytestring(ctx, wstr, data);
        case W_STRING_UNICODE:
          return bser_utf8string(ctx, wstr, data);
        case W_STRING_MIXED:
          return bser_mixedstring(ctx, wstr, data);
        default:
          w_assert(false, "unknown string type 0x%02x", wstr.type());
          return -1;
      }
    }
    case JSON_ARRAY:
      return bser_array(ctx, json, data);
    case JSON_OBJECT:
      return bser_object(ctx, json, data);
    default:
      return -1;
  }
}

static int measure(const char*, size_t size, void* ptr) {
  auto tot = (json_int_t*)ptr;
  *tot += size;
  return 0;
}

int w_bser_write_pdu(
    const uint32_t bser_version,
    const uint32_t bser_capabilities,
    json_dump_callback_t dump,
    const json_ref& json,
    void* data) {
  json_int_t m_size = 0;
  bser_ctx_t ctx{bser_version, bser_capabilities, measure};

  if (!is_bser_version_supported(&ctx)) {
    return -1;
  }

  if (w_bser_dump(&ctx, json, &m_size)) {
    return -1;
  }

  // To actually write the contents
  ctx.dump = dump;

  if (bser_version == 2) {
    if (dump(BSER_V2_MAGIC, 2, data)) {
      return -1;
    }
  } else {
    if (dump(BSER_MAGIC, 2, data)) {
      return -1;
    }
  }

  if (bser_version == 2) {
    if (dump(
            (const char*)&bser_capabilities, sizeof(bser_capabilities), data)) {
      return -1;
    }
  }

  if (bser_int(&ctx, m_size, data)) {
    return -1;
  }

  if (w_bser_dump(&ctx, json, data)) {
    return -1;
  }

  return 0;
}

static json_ref
bunser_array(const char* buf, const char* end, json_int_t* used) {
  json_int_t total = 0;

  buf++;
  total++;

  if (buf >= end) {
    throw BserParseError("document too short");
  }

  size_t needed;
  std::optional<json_int_t> element_count = bunser_int(buf, end - buf, &needed);
  if (!element_count) {
    if (needed == kDecodeIntFailed) {
      throw BserParseError(
          "invalid integer encoding {:02x}, buf={}",
          (int)buf[0],
          static_cast<const void*>(buf));
    }
    *used = needed + total;
    throw BserParseError(
        "invalid array length encoding {:02x} (needed {} but have {})",
        (int)buf[0],
        (int)needed,
        (int)(end - buf));
  }

  size_t count = element_count.value();
  if (count > kMaximumContainerSize) {
    throw BserParseError("array has too many elements");
  }

  total += needed;
  buf += needed;

  std::vector<json_ref> arrval;
  arrval.reserve(count);
  for (size_t i = 0; i < count; i++) {
    json_int_t element_needed = 0;
    auto item = bunser(buf, end, &element_needed);

    total += element_needed;
    buf += element_needed;

    arrval.push_back(std::move(item));
  }

  *used = total;
  return json_array(std::move(arrval));
}

static json_ref
bunser_template(const char* buf, const char* end, json_int_t* used) {
  json_int_t total = 0;

  buf++;
  total++;

  if (buf >= end) {
    throw BserParseError("document too short");
  }

  if (*buf != BSER_ARRAY) {
    throw BserParseError("expected array encoding, but found {:02x}", *buf);
  }

  // Load in the property names template
  json_int_t array_needed;
  auto templ = bunser_array(buf, end, &array_needed);
  total += array_needed;
  buf += array_needed;

  // And the number of objects
  size_t needed = 0;
  auto element_count = bunser_int(buf, end - buf, &needed);
  if (!element_count) {
    throw BserParseError(
        "invalid object number encoding (needed {} but have {})",
        (int)needed,
        (int)(end - buf));
  }
  size_t nelems = element_count.value();
  if (nelems > kMaximumContainerSize) {
    throw BserParseError("template has too many elements");
  }

  total += needed;
  buf += needed;

  auto& templ_arr = templ.array();
  size_t np = templ_arr.size();

  // Now load up the array with object values
  std::vector<json_ref> arrval;
  arrval.reserve(nelems);
  for (size_t i = 0; i < nelems; i++) {
    std::unordered_map<w_string, json_ref> item;
    item.reserve(np);
    for (size_t ip = 0; ip < np; ip++) {
      if (buf >= end) {
        throw BserParseError("document too short");
      }
      if (*buf == BSER_SKIP) {
        buf++;
        total++;
        continue;
      }

      json_int_t element_needed = 0;
      auto val = bunser(buf, end, &element_needed);
      buf += element_needed;
      total += element_needed;

      if (!templ_arr[ip].isString()) {
        throw BserParseError(
            "template value must be string, was {}", (int)templ_arr[ip].type());
      }

      item.insert_or_assign(json_string_value(templ_arr[ip]), std::move(val));
    }

    arrval.push_back(json_object(std::move(item)));
  }

  *used = total;
  return json_array(std::move(arrval));
}

static json_ref
bunser_object(const char* buf, const char* end, json_int_t* used) {
  char keybuf[128];

  json_int_t total = 1;
  buf++;

  size_t needed;
  auto element_count = bunser_int(buf, end - buf, &needed);
  if (!element_count) {
    throw BserParseError("invalid object property count encoding");
  }
  size_t nelems = element_count.value();

  total += needed;
  buf += needed;

  auto objval = json_object();
  for (size_t i = 0; i < nelems; i++) {
    const char* start;
    json_int_t slen;

    // Read key
    json_int_t key_needed;
    if (!bunser_generic_string(buf, end - buf, &key_needed, &start, &slen)) {
      throw BserParseError("invalid bytestring for object key: {}", end - buf);
    }
    total += key_needed;
    buf += key_needed;

    if (slen < 0) {
      throw BserParseError("negative slen");
    }

    // Saves us allocating a string when the library is going to
    // do that anyway
    if ((uint16_t)slen > sizeof(keybuf) - 1) {
      throw BserParseError("object key is too long");
    }
    memcpy(keybuf, start, (size_t)slen);
    keybuf[slen] = '\0';

    // Read value
    json_int_t value_needed;
    auto item = bunser(buf, end, &value_needed);
    total += value_needed;
    buf += value_needed;

    if (json_object_set_new_nocheck(objval, keybuf, std::move(item))) {
      throw BserParseError("failed to add object property");
    }
  }

  *used = total;
  return objval;
}

json_ref bunser(const char* buf, const char* end, json_int_t* needed) {
  if (buf >= end) {
    throw BserParseError("document too short");
  }

  switch (buf[0]) {
    case BSER_INT8:
    case BSER_INT16:
    case BSER_INT32:
    case BSER_INT64: {
      size_t int_needed;
      auto ival = bunser_int(buf, end - buf, &int_needed);
      if (!ival) {
        throw BserParseError("invalid integer encoding");
      }
      *needed = int_needed;
      return json_integer(ival.value());
    }

    case BSER_BYTESTRING:
    case BSER_UTF8STRING: {
      const char* start;
      json_int_t len;

      if (!bunser_generic_string(buf, end - buf, needed, &start, &len)) {
        throw BserParseError("invalid bytestring encoding");
      }

      if (static_cast<size_t>(len) > kMaximumContainerSize) {
        throw BserParseError("bser string too large");
      }

      return typed_string_to_json(
          start,
          len,
          buf[0] == BSER_BYTESTRING ? W_STRING_BYTE : W_STRING_UNICODE);
    }

    case BSER_REAL: {
      *needed = sizeof(double) + 1;
      ++buf;
      if (static_cast<size_t>(end - buf) < sizeof(double)) {
        throw BserParseError("document too short");
      }

      double dval;
      memcpy(&dval, buf, sizeof(dval));
      return json_real(dval);
    }

    case BSER_TRUE:
      *needed = 1;
      return json_true();
    case BSER_FALSE:
      *needed = 1;
      return json_false();
    case BSER_NULL:
      *needed = 1;
      return json_null();
    case BSER_ARRAY:
      return bunser_array(buf, end, needed);
    case BSER_TEMPLATE:
      return bunser_template(buf, end, needed);
    case BSER_OBJECT:
      return bunser_object(buf, end, needed);
  }

  throw BserParseError("invalid bser encoding type {}", (int)buf[0]);
}

/* vim:ts=2:sw=2:et:
 */
