/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/ext/ext_string.h"
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

// json_decode() options
const int64_t k_JSON_BIGINT_AS_STRING  = 1<<0;

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

Variant HHVM_FUNCTION(json_encode, const Variant& value,
                                   int64_t options /* = 0 */,
                                   int64_t depth /* = 512 */) {
  // Special case for resource since VariableSerializer does not take care of it
  if (value.isResource()) {
    json_set_last_error_code(json_error_codes::JSON_ERROR_UNSUPPORTED_TYPE);

    if (options & k_JSON_PARTIAL_OUTPUT_ON_ERROR) {
      return "null";
    }

    return false;
  }

  json_set_last_error_code(json_error_codes::JSON_ERROR_NONE);
  VariableSerializer vs(VariableSerializer::Type::JSON, options);
  vs.setDepthLimit(depth);

  String json = vs.serializeValue(value, !(options & k_JSON_FB_UNLIMITED));

  if ((json_get_last_error_code() != json_error_codes::JSON_ERROR_NONE &&
      (json_get_last_error_code() != json_error_codes::JSON_ERROR_UTF8)) &&
      !(options & k_JSON_PARTIAL_OUTPUT_ON_ERROR)) {
    return false;
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

  String trimmed = f_trim(json, "\t\n\r ");

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

const StaticString
  s_JSON_HEX_TAG("JSON_HEX_TAG"),
  s_JSON_HEX_AMP("JSON_HEX_AMP"),
  s_JSON_HEX_APOS("JSON_HEX_APOS"),
  s_JSON_HEX_QUOT("JSON_HEX_QUOT"),
  s_JSON_FORCE_OBJECT("JSON_FORCE_OBJECT"),
  s_JSON_NUMERIC_CHECK("JSON_NUMERIC_CHECK"),
  s_JSON_UNESCAPED_SLASHES("JSON_UNESCAPED_SLASHES"),
  s_JSON_PRETTY_PRINT("JSON_PRETTY_PRINT"),
  s_JSON_UNESCAPED_UNICODE("JSON_UNESCAPED_UNICODE"),
  s_JSON_PARTIAL_OUTPUT_ON_ERROR("JSON_PARTIAL_OUTPUT_ON_ERROR"),
  s_JSON_BIGINT_AS_STRING("JSON_BIGINT_AS_STRING"),
  s_JSON_FB_LOOSE("JSON_FB_LOOSE"),
  s_JSON_FB_UNLIMITED("JSON_FB_UNLIMITED"),
  s_JSON_FB_EXTRA_ESCAPES("JSON_FB_EXTRA_ESCAPES"),
  s_JSON_FB_COLLECTIONS("JSON_FB_COLLECTIONS"),
  s_JSON_FB_STABLE_MAPS("JSON_FB_STABLE_MAPS"),
  s_JSON_ERROR_NONE("JSON_ERROR_NONE"),
  s_JSON_ERROR_DEPTH("JSON_ERROR_DEPTH"),
  s_JSON_ERROR_STATE_MISMATCH("JSON_ERROR_STATE_MISMATCH"),
  s_JSON_ERROR_CTRL_CHAR("JSON_ERROR_CTRL_CHAR"),
  s_JSON_ERROR_SYNTAX("JSON_ERROR_SYNTAX"),
  s_JSON_ERROR_UTF8("JSON_ERROR_UTF8"),
  s_JSON_ERROR_RECURSION("JSON_ERROR_RECURSION"),
  s_JSON_ERROR_INF_OR_NAN("JSON_ERROR_INF_OR_NAN"),
  s_JSON_ERROR_UNSUPPORTED_TYPE("JSON_ERROR_UNSUPPORTED_TYPE");

class JsonExtension : public Extension {
 public:
  JsonExtension() : Extension("json", "1.2.1") {}
  virtual void moduleInit() {
    Native::registerConstant<KindOfInt64>(
      s_JSON_HEX_TAG.get(), k_JSON_HEX_TAG
    );
    Native::registerConstant<KindOfInt64>(
      s_JSON_HEX_AMP.get(), k_JSON_HEX_AMP
    );
    Native::registerConstant<KindOfInt64>(
      s_JSON_HEX_APOS.get(), k_JSON_HEX_APOS
    );
    Native::registerConstant<KindOfInt64>(
      s_JSON_HEX_QUOT.get(), k_JSON_HEX_QUOT
    );
    Native::registerConstant<KindOfInt64>(
      s_JSON_FORCE_OBJECT.get(), k_JSON_FORCE_OBJECT
    );
    Native::registerConstant<KindOfInt64>(
      s_JSON_NUMERIC_CHECK.get(), k_JSON_NUMERIC_CHECK
    );
    Native::registerConstant<KindOfInt64>(
      s_JSON_UNESCAPED_SLASHES.get(), k_JSON_UNESCAPED_SLASHES
    );
    Native::registerConstant<KindOfInt64>(
      s_JSON_PRETTY_PRINT.get(), k_JSON_PRETTY_PRINT
    );
    Native::registerConstant<KindOfInt64>(
      s_JSON_UNESCAPED_UNICODE.get(), k_JSON_UNESCAPED_UNICODE
    );
    Native::registerConstant<KindOfInt64>(
      s_JSON_PARTIAL_OUTPUT_ON_ERROR.get(), k_JSON_PARTIAL_OUTPUT_ON_ERROR
    );
    Native::registerConstant<KindOfInt64>(
      s_JSON_BIGINT_AS_STRING.get(), k_JSON_BIGINT_AS_STRING
    );
    Native::registerConstant<KindOfInt64>(
      s_JSON_FB_LOOSE.get(), k_JSON_FB_LOOSE
    );
    Native::registerConstant<KindOfInt64>(
      s_JSON_FB_UNLIMITED.get(), k_JSON_FB_UNLIMITED
    );
    Native::registerConstant<KindOfInt64>(
      s_JSON_FB_EXTRA_ESCAPES.get(), k_JSON_FB_EXTRA_ESCAPES
    );
    Native::registerConstant<KindOfInt64>(
      s_JSON_FB_COLLECTIONS.get(), k_JSON_FB_COLLECTIONS
    );
    Native::registerConstant<KindOfInt64>(
      s_JSON_FB_STABLE_MAPS.get(), k_JSON_FB_STABLE_MAPS
    );
    Native::registerConstant<KindOfInt64>(
      s_JSON_ERROR_NONE.get(), k_JSON_ERROR_NONE
    );
    Native::registerConstant<KindOfInt64>(
      s_JSON_ERROR_DEPTH.get(), k_JSON_ERROR_DEPTH
    );
    Native::registerConstant<KindOfInt64>(
      s_JSON_ERROR_STATE_MISMATCH.get(), k_JSON_ERROR_STATE_MISMATCH
    );
    Native::registerConstant<KindOfInt64>(
      s_JSON_ERROR_CTRL_CHAR.get(), k_JSON_ERROR_CTRL_CHAR
    );
    Native::registerConstant<KindOfInt64>(
      s_JSON_ERROR_SYNTAX.get(), k_JSON_ERROR_SYNTAX
    );
    Native::registerConstant<KindOfInt64>(
      s_JSON_ERROR_UTF8.get(), k_JSON_ERROR_UTF8
    );
  Native::registerConstant<KindOfInt64>(
    s_JSON_ERROR_RECURSION.get(), k_JSON_ERROR_RECURSION
  );
  Native::registerConstant<KindOfInt64>(
    s_JSON_ERROR_INF_OR_NAN.get(), k_JSON_ERROR_INF_OR_NAN
  );
  Native::registerConstant<KindOfInt64>(
    s_JSON_ERROR_UNSUPPORTED_TYPE.get(), k_JSON_ERROR_UNSUPPORTED_TYPE
  );

    HHVM_FE(json_last_error);
    HHVM_FE(json_last_error_msg);
    HHVM_FE(json_encode);
    HHVM_FE(json_decode);

    loadSystemlib();
  }
} s_json_extension;

}
