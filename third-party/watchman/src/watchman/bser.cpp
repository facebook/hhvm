/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/bser.h"
#include "watchman/Logging.h"
#include "watchman/thirdparty/jansson/jansson_private.h"

#include <math.h>

/*
 * This defines a binary serialization of the JSON data objects in this
 * library.  It is designed for use with watchman and is not intended to serve
 * as a general binary JSON interchange format.  In particular, all integers
 * are signed integers and are stored in host byte order to minimize
 * transformation overhead.
 */

namespace {

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

const char bser_true = BSER_TRUE;
const char bser_false = BSER_FALSE;
const char bser_null = BSER_NULL;
const char bser_bytestring_hdr = BSER_BYTESTRING;
const char bser_array_hdr = BSER_ARRAY;
const char bser_object_hdr = BSER_OBJECT;
const char bser_template_hdr = BSER_TEMPLATE;
const char bser_utf8string_hdr = BSER_UTF8STRING;
const char bser_skip = BSER_SKIP;

constexpr size_t kMaximumContainerSize = std::numeric_limits<uint32_t>::max();

// We could write the BSER parser to use O(1) stack depth, but in the short term
// let's limit container depth.
constexpr size_t kMaximumDepth = 500;

constexpr size_t kMaximumReservation = 10000;

template <typename T>
void limitedReservation(T& container, size_t size) {
  // When parsing BSER, we want to avoid reallocations when
  // possible. However, hostile inputs can ask for extremely large
  // arrays and maps. In those cases, simply cap the reservation
  // request to a reasonable amount before attempting to parse. If
  // reallocation is necessary, so be it.
  container.reserve(std::min(size, kMaximumReservation));
}

bool is_bser_version_supported(const bser_ctx_t* ctx) {
  return ctx->bser_version == 1 || ctx->bser_version == 2;
}

int bser_real(const bser_ctx_t* ctx, double val, void* data) {
  char sz = BSER_REAL;
  if (!is_bser_version_supported(ctx)) {
    return -1;
  }

  if (ctx->dump(&sz, sizeof(sz), data)) {
    return -1;
  }
  return ctx->dump((char*)&val, sizeof(val), data);
}

int bser_int(const bser_ctx_t* ctx, json_int_t val, void* data) {
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

int bser_generic_string(
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

int bser_bytestring(const bser_ctx_t* ctx, w_string_piece str, void* data) {
  return bser_generic_string(ctx, str, data, bser_bytestring_hdr);
}

int bser_utf8string(const bser_ctx_t* ctx, w_string_piece str, void* data) {
  if ((ctx->bser_capabilities & BSER_CAP_DISABLE_UNICODE) ||
      ctx->bser_version == 1) {
    return bser_bytestring(ctx, str, data);
  }
  return bser_generic_string(ctx, str, data, bser_utf8string_hdr);
}

int bser_mixedstring(const bser_ctx_t* ctx, w_string_piece str, void* data) {
  if (ctx->bser_version != 1 &&
      !(BSER_CAP_DISABLE_UNICODE_FOR_ERRORS & ctx->bser_capabilities) &&
      !(BSER_CAP_DISABLE_UNICODE & ctx->bser_capabilities)) {
    auto utf8_clean = str.asUTF8Clean();
    return bser_utf8string(ctx, utf8_clean, data);
  } else {
    return bser_bytestring(ctx, str, data);
  }
}

int bser_array(const bser_ctx_t* ctx, const json_ref& array, void* data);

int bser_template(
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

int bser_array(const bser_ctx_t* ctx, const json_ref& array, void* data) {
  if (!is_bser_version_supported(ctx)) {
    return -1;
  }

  auto templ = json_array_get_template(array);
  if (templ && !templ->array().empty()) {
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

int bser_object(const bser_ctx_t* ctx, const json_ref& obj, void* data) {
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

} // namespace

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

namespace {

int measure(const char*, size_t size, void* ptr) {
  auto tot = (json_int_t*)ptr;
  *tot += size;
  return 0;
}

} // namespace

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

namespace {

/**
 * Contains BserParser state. BSER is a simple format, so the only mutable state
 * is the current pointer.
 *
 * Terminology note:
 * "parse" means we know the current value type, so decode and return it.
 * "expect" means we don't know the current value type, but the document
 * requires it be a specific type.
 */
class BserParser {
 public:
  BserParser(const char* buf, const char* end)
      : buf{buf}, start{buf}, end{end} {
    if (end < buf) {
      logf(
          watchman::FATAL,
          "end {} < buf {}",
          static_cast<const void*>(end),
          static_cast<const void*>(buf));
    }
  }

  json_ref expectValue() {
    return parseValue(*ensure(1));
  }

  json_ref parseValue(char value_type) {
    switch (value_type) {
      case BSER_INT8:
      case BSER_INT16:
      case BSER_INT32:
      case BSER_INT64:
        return json_integer(parseInteger(value_type));

      case BSER_BYTESTRING:
      case BSER_UTF8STRING: {
        std::string_view str = parseString();
        return typed_string_to_json(
            str.data(),
            str.size(),
            value_type == BSER_BYTESTRING ? W_STRING_BYTE : W_STRING_UNICODE);
      }

      case BSER_REAL: {
        return json_real(parseReal());
      }

      case BSER_TRUE:
        return json_true();
      case BSER_FALSE:
        return json_false();
      case BSER_NULL:
        return json_null();
      case BSER_ARRAY:
        return json_array(parseArray());
      case BSER_TEMPLATE:
        return parseTemplate();
      case BSER_OBJECT:
        return parseObject();
      default:
        throw BserParseError("invalid bser encoding type: {:02x}", value_type);
    }
  }

 private:
  /**
   * Ensures `needed` bytes remain in the document, and advances the `buf`
   * pointer. Returns the old `buf` with the assurance that up to `needed` bytes
   * are safe to read.
   */
  const char* ensure(size_t needed) {
    assert(end >= buf);
    if (needed > static_cast<size_t>(end - buf)) {
      throw BserParseError(
          "unexpected EOF at {}: expected {} remaining but total document is {}",
          buf - start,
          needed,
          end - start);
    }
    const char* old = buf;
    buf += needed;
    return old;
  }

  char expectType(std::initializer_list<char> types) {
    char type = *ensure(1);
    for (char expected : types) {
      if (type == expected) {
        return type;
      }
    }
    std::string expected = "{";
    bool comma = false;
    for (char type_2 : types) {
      if (comma) {
        expected += ",";
      } else {
        comma = true;
      }
      expected += std::to_string(static_cast<int>(type_2));
    }
    expected += "}";
    throw BserParseError(
        "unexpected value type: expected {} but saw {}",
        expected,
        static_cast<int>(type));
  }

  template <typename T>
  json_int_t parseInteger() {
    T v;
    memcpy(&v, ensure(sizeof(v)), sizeof(v));
    return v;
  }

  json_int_t parseInteger(char type) {
    switch (type) {
      case BSER_INT8:
        return parseInteger<int8_t>();
      case BSER_INT16:
        return parseInteger<int16_t>();
      case BSER_INT32:
        return parseInteger<int32_t>();
      case BSER_INT64:
        return parseInteger<int64_t>();
    }
    assert(false && "invalid integer type");
    abort();
  }

  double parseReal() {
    double dval;
    memcpy(&dval, ensure(sizeof(double)), sizeof(dval));

    if (!isfinite(dval)) {
      throw BserParseError("reals must be finite");
    }

    return dval;
  }

  json_int_t expectInteger() {
    char type = expectType({BSER_INT8, BSER_INT16, BSER_INT32, BSER_INT64});
    return parseInteger(type);
  }

  size_t expectSize(const char* label) {
    json_int_t size = expectInteger();
    if (size < 0) {
      throw BserParseError("{} has negative size", label);
    }
    size_t rv = size;
    if (rv > kMaximumContainerSize) {
      throw BserParseError("{} size is too large: {}", label, rv);
    }
    return rv;
  }

  // References memory in the input document.
  std::string_view parseString() {
    size_t length = expectSize("string");
    return std::string_view{ensure(length), length};
  }

  std::string_view expectString() {
    expectType({BSER_BYTESTRING, BSER_UTF8STRING});
    return parseString();
  }

  std::vector<json_ref> parseArray() {
    BumpDepth scope{depth};

    size_t count = expectSize("array");

    std::vector<json_ref> rv;
    limitedReservation(rv, count);
    for (size_t i = 0; i < count; i++) {
      rv.push_back(expectValue());
    }
    return rv;
  }

  std::vector<json_ref> expectArray() {
    expectType({BSER_ARRAY});
    return parseArray();
  }

  json_ref parseTemplate() {
    BumpDepth scope{depth};

    // Load in the property names template
    auto templ = expectArray();
    if (templ.empty()) {
      // To avoid "decompression bombs" -- small documents that expand into huge
      // memory requirements -- require that templates have a non-empty key set.
      throw BserParseError("templates require a non-empty key set");
    }

    // Validate that all template keys are strings before entering the main
    // loop.
    for (const auto& template_key : templ) {
      if (!template_key.isString()) {
        throw BserParseError(
            "template value must be string, was {}", template_key.type());
      }
    }

    // And the number of objects
    auto element_count = expectSize("template");

    // Now load up the array with object values
    std::vector<json_ref> rv;
    limitedReservation(rv, element_count);
    for (size_t i = 0; i < element_count; ++i) {
      std::unordered_map<w_string, json_ref> item;
      limitedReservation(item, templ.size());
      for (const auto& template_key : templ) {
        char type = *ensure(1);
        if (type == BSER_SKIP) {
          continue;
        }

        assert(template_key.isString());
        item.insert_or_assign(
            json_string_value(template_key), parseValue(type));
      }

      rv.push_back(json_object(std::move(item)));
    }

    return json_array(std::move(rv));
  }

  json_ref parseObject() {
    BumpDepth scope{depth};

    size_t element_count = expectSize("object");

    std::unordered_map<w_string, json_ref> rv;
    limitedReservation(rv, element_count);

    for (size_t i = 0; i < element_count; i++) {
      auto key = expectString();
      auto value = expectValue();

      rv.emplace(
          w_string{
              key.data(),
              key.size(),
              // Hard-coding the string type matches BSER's previous behavior,
              // but should we respect the type encoded in the BSER document?
              W_STRING_BYTE},
          std::move(value));
    }

    return json_object(std::move(rv));
  }

  struct BumpDepth {
    explicit BumpDepth(size_t& depth) : depth{depth} {
      if (++depth == kMaximumDepth) {
        throw BserParseTooDeep{};
      }
    }
    ~BumpDepth() {
      --depth;
    }

    size_t& depth;
  };

  const char* buf;
  const char* const start;
  const char* const end;
  size_t depth = 0;
};

} // namespace

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

json_ref bunser(const char* buf, const char* end) {
  if (buf >= end) {
    throw BserParseError("document too short");
  }
  return BserParser{buf, end}.expectValue();
}
