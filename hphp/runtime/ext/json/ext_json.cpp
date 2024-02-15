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

#include "hphp/runtime/ext/json/ext_json.h"
#include "hphp/runtime/ext/json/JSON_parser.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-data-defs.h"
#include "hphp/runtime/base/struct-log-util.h"
#include "hphp/runtime/base/utf8-decode.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/util/stack-trace.h"
#include "hphp/runtime/vm/bytecode.h"

#include "hphp/runtime/ext/string/ext_string.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
// json_encode() options
const int64_t k_JSON_HEX_TAG                 = 1ll << 0;
const int64_t k_JSON_HEX_AMP                 = 1ll << 1;
const int64_t k_JSON_HEX_APOS                = 1ll << 2;
const int64_t k_JSON_HEX_QUOT                = 1ll << 3;
const int64_t k_JSON_FORCE_OBJECT            = 1ll << 4;
const int64_t k_JSON_NUMERIC_CHECK           = 1ll << 5;
const int64_t k_JSON_UNESCAPED_SLASHES       = 1ll << 6;
const int64_t k_JSON_PRETTY_PRINT            = 1ll << 7;
const int64_t k_JSON_UNESCAPED_UNICODE       = 1ll << 8;
const int64_t k_JSON_PARTIAL_OUTPUT_ON_ERROR = 1ll << 9;
const int64_t k_JSON_PRESERVE_ZERO_FRACTION  = 1ll << 10;

// json_decode() options
const int64_t k_JSON_OBJECT_AS_ARRAY   = 1ll << 0;
const int64_t k_JSON_BIGINT_AS_STRING  = 1ll << 1;

// FB json_decode() options
// intentionally higher so when PHP adds more options we're fine
const int64_t k_JSON_FB_DARRAYS        = 1ll << 19;
const int64_t k_JSON_FB_LOOSE          = 1ll << 20;
const int64_t k_JSON_FB_UNLIMITED      = 1ll << 21;
const int64_t k_JSON_FB_EXTRA_ESCAPES  = 1ll << 22;
const int64_t k_JSON_FB_COLLECTIONS    = 1ll << 23;
const int64_t k_JSON_FB_STABLE_MAPS    = 1ll << 24;
const int64_t k_JSON_FB_HACK_ARRAYS    = 1ll << 25;
const int64_t k_JSON_FB_FORCE_PHP_ARRAYS = 1ll << 26;
const int64_t k_JSON_FB_WARN_DICTS       = 1ll << 27;
const int64_t k_JSON_FB_WARN_PHP_ARRAYS  = 1ll << 28;
const int64_t k_JSON_FB_DARRAYS_AND_VARRAYS = 1ll << 29;
const int64_t k_JSON_FB_WARN_EMPTY_DARRAYS = 1ll << 30;
const int64_t k_JSON_FB_WARN_VEC_LIKE_DARRAYS = 1ll << 31;
const int64_t k_JSON_FB_WARN_DICT_LIKE_DARRAYS = 1ll << 32;
const int64_t k_JSON_FB_IGNORE_LATEINIT = 1ll << 33;
const int64_t k_JSON_FB_THRIFT_SIMPLE_JSON = 1ll << 34;
const int64_t k_JSON_FB_WARN_KEYSETS       = 1ll << 36;
const int64_t k_JSON_FB_FORCE_HACK_ARRAYS  = 1ll << 37;

const int64_t k_JSON_ERROR_NONE
  = json_error_codes::JSON_ERROR_NONE;
const int64_t k_JSON_ERROR_DEPTH
  = json_error_codes::JSON_ERROR_DEPTH;
const int64_t k_JSON_ERROR_STATE_MISMATCH
  = json_error_codes::JSON_ERROR_STATE_MISMATCH;
const int64_t k_JSON_ERROR_CTRL_CHAR
  = json_error_codes::JSON_ERROR_CTRL_CHAR;
const int64_t k_JSON_ERROR_SYNTAX
  = json_error_codes::JSON_ERROR_SYNTAX;
const int64_t k_JSON_ERROR_UTF8
  = json_error_codes::JSON_ERROR_UTF8;
const int64_t k_JSON_ERROR_RECURSION
  = json_error_codes::JSON_ERROR_RECURSION;
const int64_t k_JSON_ERROR_INF_OR_NAN
  = json_error_codes::JSON_ERROR_INF_OR_NAN;
const int64_t k_JSON_ERROR_UNSUPPORTED_TYPE
  = json_error_codes::JSON_ERROR_UNSUPPORTED_TYPE;

///////////////////////////////////////////////////////////////////////////////

// Handles output of `json_encode` with fallback value for
// partial output on errors, and `false` otherwise.
Variant json_guard_error_result(const String& partial_error_output,
                                int64_t options /* = 0*/) {
  int is_partial_output = options & k_JSON_PARTIAL_OUTPUT_ON_ERROR;

   // Issue a warning on unsupported type in case of HH syntax.
  if (json_get_last_error_code() ==
      json_error_codes::JSON_ERROR_UNSUPPORTED_TYPE) {
    // Unhandled case is always returned as `false`; for partial output
    // we render "null" value.
    raise_warning("json_encode(): type is unsupported, encoded as %s",
      is_partial_output ? "null" : "false");
  }

  if (is_partial_output) {
    return partial_error_output;
  }

  return false;
}

TypedValue json_encode_impl(const Variant& value, int64_t options,
                            int64_t depth, bool pure) {
  // Special case for resource since VariableSerializer does not take care of it
  if (value.isResource()) {
    json_set_last_error_code(json_error_codes::JSON_ERROR_UNSUPPORTED_TYPE);
    return tvReturn(json_guard_error_result("null", options));
  }

  json_set_last_error_code(json_error_codes::JSON_ERROR_NONE);
  VariableSerializer vs(VariableSerializer::Type::JSON, options);
  vs.setDepthLimit(depth);
  if (options & k_JSON_FB_FORCE_PHP_ARRAYS) vs.setForcePHPArrays();
  if (options & k_JSON_FB_FORCE_HACK_ARRAYS) vs.setForceHackArrays();
  if (options & k_JSON_FB_WARN_DICTS) vs.setDictWarn();
  if (options & k_JSON_FB_WARN_KEYSETS) vs.setKeysetWarn();
  if (options & k_JSON_FB_WARN_PHP_ARRAYS) vs.setPHPWarn();
  if (options & k_JSON_FB_WARN_EMPTY_DARRAYS) vs.setEmptyDArrayWarn();
  if (options & k_JSON_FB_WARN_VEC_LIKE_DARRAYS) vs.setVecLikeDArrayWarn();
  if (options & k_JSON_FB_WARN_DICT_LIKE_DARRAYS) vs.setDictLikeDArrayWarn();
  if (options & k_JSON_FB_IGNORE_LATEINIT) vs.setIgnoreLateInit();
  if (pure) vs.setPure();

  String json = vs.serializeValue(value, !(options & k_JSON_FB_UNLIMITED));
  assertx(json.get() != nullptr);
  if (UNLIKELY(StructuredLog::coinflip(RuntimeOption::EvalSerDesSampleRate))) {
    StructuredLog::logSerDes("json", "ser", json, value);
  }

  if (json_get_last_error_code() != json_error_codes::JSON_ERROR_NONE) {
    return tvReturn(json_guard_error_result(json, options));
  }

  return tvReturn(std::move(json));
}

TypedValue HHVM_FUNCTION(json_encode, const Variant& value,
                         int64_t options, int64_t depth) {
  return json_encode_impl(value, options, depth, false);
}

TypedValue json_encode_with_error_impl(const Variant& value, Variant& error,
                                       int64_t options, int64_t depth,
                                       bool pure) {
  // fn is pure, so prior state must be preserved and restored
  auto prior_error_code = json_get_last_error_code();

  auto result = json_encode_impl(value, options, depth, pure);

  auto error_code = json_get_last_error_code();
  if (error_code == k_JSON_ERROR_NONE) {
    error.setNull();
  } else {
    error = make_vec_array(error_code, json_get_last_error_msg());
  }

  json_set_last_error_code(prior_error_code);

  return result;
}

TypedValue HHVM_FUNCTION(json_encode_with_error, const Variant& value,
                         Variant& error, int64_t options, int64_t depth) {
  return json_encode_with_error_impl(value, error, options, depth, false);
}

TypedValue HHVM_FUNCTION(json_encode_pure, const Variant& value,
                         Variant& error, int64_t options, int64_t depth) {
  return json_encode_with_error_impl(value, error, options, depth, true);
}

TypedValue HHVM_FUNCTION(json_decode, const String& json,
                         bool assoc, int64_t depth, int64_t options) {
  json_set_last_error_code(json_error_codes::JSON_ERROR_NONE);

  if (json.empty()) {
    return make_tv<KindOfNull>();
  }
  if (depth < 0 || depth > INT_MAX) {
    json_set_last_error_code(json_error_codes::JSON_ERROR_DEPTH);
    return make_tv<KindOfNull>();
  }

  const int64_t supported_options =
    k_JSON_FB_LOOSE |
    k_JSON_FB_COLLECTIONS |
    k_JSON_FB_STABLE_MAPS |
    k_JSON_BIGINT_AS_STRING |
    k_JSON_FB_HACK_ARRAYS |
    k_JSON_FB_DARRAYS |
    k_JSON_FB_DARRAYS_AND_VARRAYS |
    k_JSON_FB_THRIFT_SIMPLE_JSON;
  int64_t parser_options = options & supported_options;
  Variant z;
  const auto ok =
    JSON_parser(z, json.data(), json.size(), assoc, depth, parser_options);
  if (UNLIKELY(StructuredLog::coinflip(RuntimeOption::EvalSerDesSampleRate))) {
    StructuredLog::logSerDes("json", "des", json, z);
  }
  if (ok) {
    return tvReturn(std::move(z));
  }

  String trimmed = HHVM_FN(trim)(json, "\t\n\r ");

  if (trimmed.size() == 4) {
    if (!strcasecmp(trimmed.data(), "null")) {
      json_set_last_error_code(json_error_codes::JSON_ERROR_NONE);
      return make_tv<KindOfNull>();
    }
    if (!strcasecmp(trimmed.data(), "true")) {
      json_set_last_error_code(json_error_codes::JSON_ERROR_NONE);
      return make_tv<KindOfBoolean>(true);
    }
  } else if (trimmed.size() == 5 && !strcasecmp(trimmed.data(), "false")) {
    json_set_last_error_code(json_error_codes::JSON_ERROR_NONE);
    return make_tv<KindOfBoolean>(false);
  }

  int64_t p;
  double d;
  DataType type = json.get()->isNumericWithVal(p, d, 0);
  if (type == KindOfInt64) {
    json_set_last_error_code(json_error_codes::JSON_ERROR_NONE);
    return make_tv<KindOfInt64>(p);
  } else if (type == KindOfDouble) {
    json_set_last_error_code(json_error_codes::JSON_ERROR_NONE);
    if ((options & k_JSON_BIGINT_AS_STRING) &&
        (json.toInt64() == LLONG_MAX || json.toInt64() == LLONG_MIN)
        && errno == ERANGE) { // Overflow
      bool is_float = false;
      for (int i = (trimmed[0] == '-' ? 1 : 0); i < trimmed.size(); ++i) {
        if (trimmed[i] < '0' || trimmed[i] > '9') {
          is_float = true;
          break;
        }
      }
      if (!is_float) {
        return tvReturn(trimmed);
      }
    }
    return make_tv<KindOfDouble>(d);
  }

  char ch0 = json.charAt(0);
  if (json.size() > 1 && ch0 == '"' && json.charAt(json.size() - 1) == '"') {
    json_set_last_error_code(json_error_codes::JSON_ERROR_NONE);

    // Wrap the string in an array to allow the JSON_parser to handle
    // things like unicode escape sequences, then unwrap to get result
    String wrapped("[");
    wrapped += json + "]";
    // Stick to a normal hhvm array for the wrapper
    const int64_t mask = ~(k_JSON_FB_COLLECTIONS | k_JSON_FB_STABLE_MAPS);
    if (JSON_parser(z, wrapped.data(), wrapped.size(), false, depth,
                    parser_options & mask) && z.isArray()) {
      Array arr = z.toArray();
      if ((arr.size() == 1) && arr.exists(0)) {
        return tvReturn(arr[0]);
      }
      // The input string could be something like: "foo","bar"
      // Which will parse inside the [] wrapper, but be invalid
      json_set_last_error_code(json_error_codes::JSON_ERROR_SYNTAX);
    }
  }

  if ((options & k_JSON_FB_LOOSE) && json.size() > 1 &&
      ch0 == '\'' && json.charAt(json.size() - 1) == '\'') {
    json_set_last_error_code(json_error_codes::JSON_ERROR_NONE);
    return tvReturn(json.substr(1, json.size() - 2));
  }

  assertx(json_get_last_error_code() != json_error_codes::JSON_ERROR_NONE);
  return make_tv<KindOfNull>();
}

TypedValue HHVM_FUNCTION(json_decode_with_error, const String& json,
                         Variant& error, bool assoc, int64_t depth,
                         int64_t options) {
  // fn is pure, so prior state must be preserved and restored
  auto prior_error_code = json_get_last_error_code();

  auto result = HHVM_FN(json_decode)(json, assoc, depth, options);

  auto error_code = json_get_last_error_code();
  if (error_code == k_JSON_ERROR_NONE) {
    error.setNull();
  } else {
    error = make_vec_array(error_code, json_get_last_error_msg());
  }

  json_set_last_error_code(prior_error_code);

  return result;
}

///////////////////////////////////////////////////////////////////////////////

struct JsonExtension final : Extension {
  JsonExtension() : Extension("json", "1.2.1", NO_ONCALL_YET) {}
  void moduleRegisterNative() override {
    HHVM_RC_INT(JSON_HEX_TAG, k_JSON_HEX_TAG);
    HHVM_RC_INT(JSON_HEX_AMP, k_JSON_HEX_AMP);
    HHVM_RC_INT(JSON_HEX_APOS, k_JSON_HEX_APOS);
    HHVM_RC_INT(JSON_HEX_QUOT, k_JSON_HEX_QUOT);
    HHVM_RC_INT(JSON_FORCE_OBJECT, k_JSON_FORCE_OBJECT);
    HHVM_RC_INT(JSON_NUMERIC_CHECK, k_JSON_NUMERIC_CHECK);
    HHVM_RC_INT(JSON_UNESCAPED_SLASHES, k_JSON_UNESCAPED_SLASHES);
    HHVM_RC_INT(JSON_PRETTY_PRINT, k_JSON_PRETTY_PRINT);
    HHVM_RC_INT(JSON_UNESCAPED_UNICODE, k_JSON_UNESCAPED_UNICODE);
    HHVM_RC_INT(JSON_PARTIAL_OUTPUT_ON_ERROR, k_JSON_PARTIAL_OUTPUT_ON_ERROR);
    HHVM_RC_INT(JSON_PRESERVE_ZERO_FRACTION, k_JSON_PRESERVE_ZERO_FRACTION);
    HHVM_RC_INT(JSON_OBJECT_AS_ARRAY, k_JSON_OBJECT_AS_ARRAY);
    HHVM_RC_INT(JSON_BIGINT_AS_STRING, k_JSON_BIGINT_AS_STRING);
    HHVM_RC_INT(JSON_FB_DARRAYS, k_JSON_FB_DARRAYS);
    HHVM_RC_INT(JSON_FB_LOOSE, k_JSON_FB_LOOSE);
    HHVM_RC_INT(JSON_FB_UNLIMITED, k_JSON_FB_UNLIMITED);
    HHVM_RC_INT(JSON_FB_EXTRA_ESCAPES, k_JSON_FB_EXTRA_ESCAPES);
    HHVM_RC_INT(JSON_FB_COLLECTIONS, k_JSON_FB_COLLECTIONS);
    HHVM_RC_INT(JSON_FB_HACK_ARRAYS, k_JSON_FB_HACK_ARRAYS);
    HHVM_RC_INT(JSON_FB_STABLE_MAPS, k_JSON_FB_STABLE_MAPS);
    HHVM_RC_INT(JSON_FB_FORCE_PHP_ARRAYS, k_JSON_FB_FORCE_PHP_ARRAYS);
    HHVM_RC_INT(JSON_FB_WARN_DICTS, k_JSON_FB_WARN_DICTS);
    HHVM_RC_INT(JSON_FB_WARN_PHP_ARRAYS, k_JSON_FB_WARN_PHP_ARRAYS);
    HHVM_RC_INT(JSON_FB_WARN_EMPTY_DARRAYS, k_JSON_FB_WARN_EMPTY_DARRAYS);
    HHVM_RC_INT(JSON_FB_WARN_VEC_LIKE_DARRAYS,
                k_JSON_FB_WARN_VEC_LIKE_DARRAYS);
    HHVM_RC_INT(JSON_FB_WARN_DICT_LIKE_DARRAYS,
                k_JSON_FB_WARN_DICT_LIKE_DARRAYS);
    HHVM_RC_INT(JSON_FB_IGNORE_LATEINIT, k_JSON_FB_IGNORE_LATEINIT);
    HHVM_RC_INT(JSON_FB_THRIFT_SIMPLE_JSON, k_JSON_FB_THRIFT_SIMPLE_JSON);
    HHVM_RC_INT(JSON_FB_WARN_KEYSETS, k_JSON_FB_WARN_KEYSETS);
    HHVM_RC_INT(JSON_FB_FORCE_HACK_ARRAYS, k_JSON_FB_FORCE_HACK_ARRAYS);

    HHVM_RC_INT(JSON_ERROR_NONE, k_JSON_ERROR_NONE);
    HHVM_RC_INT(JSON_ERROR_DEPTH, k_JSON_ERROR_DEPTH);
    HHVM_RC_INT(JSON_ERROR_STATE_MISMATCH, k_JSON_ERROR_STATE_MISMATCH);
    HHVM_RC_INT(JSON_ERROR_CTRL_CHAR, k_JSON_ERROR_CTRL_CHAR);
    HHVM_RC_INT(JSON_ERROR_SYNTAX, k_JSON_ERROR_SYNTAX);
    HHVM_RC_INT(JSON_ERROR_UTF8, k_JSON_ERROR_UTF8);
    HHVM_RC_INT(JSON_ERROR_RECURSION, k_JSON_ERROR_RECURSION);
    HHVM_RC_INT(JSON_ERROR_INF_OR_NAN, k_JSON_ERROR_INF_OR_NAN);
    HHVM_RC_INT(JSON_ERROR_UNSUPPORTED_TYPE, k_JSON_ERROR_UNSUPPORTED_TYPE);

    HHVM_FE(json_encode);
    HHVM_FE(json_encode_with_error);
    HHVM_FE(json_encode_pure);
    HHVM_FE(json_decode);
    HHVM_FE(json_decode_with_error);
  }

  void requestInit() override {
    json_parser_init();
  }

} s_json_extension;

}
