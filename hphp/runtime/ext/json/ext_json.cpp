/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/ext/string/ext_string.h"
#include "hphp/runtime/base/array-data-defs.h"
#include "hphp/runtime/base/utf8-decode.h"
#include "hphp/runtime/base/variable-serializer.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
// json_encode() options
const int64_t k_JSON_HEX_TAG                 = 1<<0;
const int64_t k_JSON_HEX_AMP                 = 1<<1;
const int64_t k_JSON_HEX_APOS                = 1<<2;
const int64_t k_JSON_HEX_QUOT                = 1<<3;
const int64_t k_JSON_FORCE_OBJECT            = 1<<4;
const int64_t k_JSON_NUMERIC_CHECK           = 1<<5;
const int64_t k_JSON_UNESCAPED_SLASHES       = 1<<6;
const int64_t k_JSON_PRETTY_PRINT            = 1<<7;
const int64_t k_JSON_UNESCAPED_UNICODE       = 1<<8;
const int64_t k_JSON_PARTIAL_OUTPUT_ON_ERROR = 1<<9;
const int64_t k_JSON_PRESERVE_ZERO_FRACTION  = 1<<10;

// json_decode() options
const int64_t k_JSON_OBJECT_AS_ARRAY   = 1<<0;
const int64_t k_JSON_BIGINT_AS_STRING  = 1<<1;

// FB json_decode() options
// intentionally higher so when PHP adds more options we're fine
const int64_t k_JSON_FB_LOOSE          = 1<<20;
const int64_t k_JSON_FB_UNLIMITED      = 1<<21;
const int64_t k_JSON_FB_EXTRA_ESCAPES  = 1<<22;
const int64_t k_JSON_FB_COLLECTIONS    = 1<<23;
const int64_t k_JSON_FB_STABLE_MAPS    = 1<<24;

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
int64_t HHVM_FUNCTION(json_last_error) {
  return (int) json_get_last_error_code();
}

String HHVM_FUNCTION(json_last_error_msg) {
  return json_get_last_error_msg();
}

// Handles output of `json_encode` with fallback value for
// partial output on errors, and `false` otherwise.
Variant json_guard_error_result(const String& partial_error_output,
                                int64_t options /* = 0*/) {
  int is_partial_output = options & k_JSON_PARTIAL_OUTPUT_ON_ERROR;

   // Issue a warning on unsupported type in case of HH syntax.
  if (json_get_last_error_code() ==
      json_error_codes::JSON_ERROR_UNSUPPORTED_TYPE &&
      RuntimeOption::EnableHipHopSyntax) {
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

Variant HHVM_FUNCTION(json_encode, const Variant& value,
                                   int64_t options /* = 0 */,
                                   int64_t depth /* = 512 */) {
  // Special case for resource since VariableSerializer does not take care of it
  if (value.isResource()) {
    json_set_last_error_code(json_error_codes::JSON_ERROR_UNSUPPORTED_TYPE);
    return json_guard_error_result("null", options);
  }

  json_set_last_error_code(json_error_codes::JSON_ERROR_NONE);
  VariableSerializer vs(VariableSerializer::Type::JSON, options);
  vs.setDepthLimit(depth);

  String json = vs.serializeValue(value, !(options & k_JSON_FB_UNLIMITED));

  if (json_get_last_error_code() != json_error_codes::JSON_ERROR_NONE) {
    return json_guard_error_result(json, options);
  }

  return json;
}

Variant HHVM_FUNCTION(json_decode, const String& json, bool assoc /* = false */,
                      int64_t depth /* = 512 */, int64_t options /* = 0 */) {

  json_set_last_error_code(json_error_codes::JSON_ERROR_NONE);

  if (json.empty()) {
    return init_null();
  }

  const int64_t supported_options =
    k_JSON_FB_LOOSE |
    k_JSON_FB_COLLECTIONS |
    k_JSON_FB_STABLE_MAPS |
    k_JSON_BIGINT_AS_STRING;
  int64_t parser_options = options & supported_options;
  Variant z;
  if (JSON_parser(z, json.data(), json.size(), assoc, depth, parser_options)) {
    return z;
  }

  String trimmed = HHVM_FN(trim)(json, "\t\n\r ");

  if (trimmed.size() == 4) {
    if (!strcasecmp(trimmed.data(), "null")) {
      json_set_last_error_code(json_error_codes::JSON_ERROR_NONE);
      return init_null();
    }
    if (!strcasecmp(trimmed.data(), "true")) {
      json_set_last_error_code(json_error_codes::JSON_ERROR_NONE);
      return true;
    }
  } else if (trimmed.size() == 5 && !strcasecmp(trimmed.data(), "false")) {
    json_set_last_error_code(json_error_codes::JSON_ERROR_NONE);
    return false;
  }

  int64_t p;
  double d;
  DataType type = json.get()->isNumericWithVal(p, d, 0);
  if (type == KindOfInt64) {
    json_set_last_error_code(json_error_codes::JSON_ERROR_NONE);
    return p;
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
        return trimmed;
      }
    }
    return d;
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
        return arr[0];
      }
      // The input string could be something like: "foo","bar"
      // Which will parse inside the [] wrapper, but be invalid
      json_set_last_error_code(json_error_codes::JSON_ERROR_SYNTAX);
    }
  }

  if ((options & k_JSON_FB_LOOSE) && json.size() > 1 &&
      ch0 == '\'' && json.charAt(json.size() - 1) == '\'') {
    json_set_last_error_code(json_error_codes::JSON_ERROR_NONE);
    return json.substr(1, json.size() - 2);
  }

  assert(json_get_last_error_code() != json_error_codes::JSON_ERROR_NONE);
  return init_null();
}

///////////////////////////////////////////////////////////////////////////////

class JsonExtension final : public Extension {
 public:
  JsonExtension() : Extension("json", "1.2.1") {}
  void moduleInit() override {
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
    HHVM_RC_INT(JSON_FB_LOOSE, k_JSON_FB_LOOSE);
    HHVM_RC_INT(JSON_FB_UNLIMITED, k_JSON_FB_UNLIMITED);
    HHVM_RC_INT(JSON_FB_EXTRA_ESCAPES, k_JSON_FB_EXTRA_ESCAPES);
    HHVM_RC_INT(JSON_FB_COLLECTIONS, k_JSON_FB_COLLECTIONS);
    HHVM_RC_INT(JSON_FB_STABLE_MAPS, k_JSON_FB_STABLE_MAPS);
    HHVM_RC_INT(JSON_ERROR_NONE, k_JSON_ERROR_NONE);
    HHVM_RC_INT(JSON_ERROR_DEPTH, k_JSON_ERROR_DEPTH);
    HHVM_RC_INT(JSON_ERROR_STATE_MISMATCH, k_JSON_ERROR_STATE_MISMATCH);
    HHVM_RC_INT(JSON_ERROR_CTRL_CHAR, k_JSON_ERROR_CTRL_CHAR);
    HHVM_RC_INT(JSON_ERROR_SYNTAX, k_JSON_ERROR_SYNTAX);
    HHVM_RC_INT(JSON_ERROR_UTF8, k_JSON_ERROR_UTF8);
    HHVM_RC_INT(JSON_ERROR_RECURSION, k_JSON_ERROR_RECURSION);
    HHVM_RC_INT(JSON_ERROR_INF_OR_NAN, k_JSON_ERROR_INF_OR_NAN);
    HHVM_RC_INT(JSON_ERROR_UNSUPPORTED_TYPE, k_JSON_ERROR_UNSUPPORTED_TYPE);

    HHVM_FE(json_last_error);
    HHVM_FE(json_last_error_msg);
    HHVM_FE(json_encode);
    HHVM_FE(json_decode);

    loadSystemlib();
  }

  void requestInit() override {
    json_parser_init();
  }

  void vscan(IMarker& mark) const override {
    json_parser_scan(mark);
  }

} s_json_extension;

}
