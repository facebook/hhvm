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
#include "hphp/runtime/base/utf8-decode.h"
#include "hphp/runtime/base/variable-serializer.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
// json_encode() options
const int64_t k_JSON_HEX_TAG           = 1<<0;
const int64_t k_JSON_HEX_AMP           = 1<<1;
const int64_t k_JSON_HEX_APOS          = 1<<2;
const int64_t k_JSON_HEX_QUOT          = 1<<3;
const int64_t k_JSON_FORCE_OBJECT      = 1<<4;
const int64_t k_JSON_NUMERIC_CHECK     = 1<<5;
const int64_t k_JSON_UNESCAPED_SLASHES = 1<<6;
const int64_t k_JSON_PRETTY_PRINT      = 1<<7;
const int64_t k_JSON_UNESCAPED_UNICODE = 1<<8;

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

///////////////////////////////////////////////////////////////////////////////
int64_t HHVM_FUNCTION(json_last_error) {
  return (int) json_get_last_error_code();
}

String HHVM_FUNCTION(json_last_error_msg) {
  return json_get_last_error_msg();
}

String HHVM_FUNCTION(json_encode, const Variant& value, int64_t options /* = 0 */,
                                  int64_t depth /* = 512 */) {

  json_set_last_error_code(json_error_codes::JSON_ERROR_NONE);

  VariableSerializer vs(VariableSerializer::Type::JSON, options);
  return vs.serializeValue(value, !(options & k_JSON_FB_UNLIMITED));
}

Variant HHVM_FUNCTION(json_decode, const String& json, bool assoc /* = false */,
                      int64_t depth /* = 512 */, int64_t options /* = 0 */) {

  json_set_last_error_code(json_error_codes::JSON_ERROR_NONE);

  if (json.empty()) {
    return uninit_null();
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

  if (json.size() == 4) {
    if (!strcasecmp(json.data(), "null")) {
      json_set_last_error_code(json_error_codes::JSON_ERROR_NONE);
      return uninit_null();
    }
    if (!strcasecmp(json.data(), "true")) {
      json_set_last_error_code(json_error_codes::JSON_ERROR_NONE);
      return true;
    }
  } else if (json.size() == 5 && !strcasecmp(json.data(), "false")) {
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
  return uninit_null();
}

///////////////////////////////////////////////////////////////////////////////

const StaticString s_JSON_HEX_TAG("JSON_HEX_TAG");
const StaticString s_JSON_HEX_AMP("JSON_HEX_AMP");
const StaticString s_JSON_HEX_APOS("JSON_HEX_APOS");
const StaticString s_JSON_HEX_QUOT("JSON_HEX_QUOT");
const StaticString s_JSON_FORCE_OBJECT("JSON_FORCE_OBJECT");
const StaticString s_JSON_NUMERIC_CHECK("JSON_NUMERIC_CHECK");
const StaticString s_JSON_UNESCAPED_SLASHES("JSON_UNESCAPED_SLASHES");
const StaticString s_JSON_PRETTY_PRINT("JSON_PRETTY_PRINT");
const StaticString s_JSON_UNESCAPED_UNICODE("JSON_UNESCAPED_UNICODE");
const StaticString s_JSON_BIGINT_AS_STRING("JSON_BIGINT_AS_STRING");
const StaticString s_JSON_FB_LOOSE("JSON_FB_LOOSE");
const StaticString s_JSON_FB_UNLIMITED("JSON_FB_UNLIMITED");
const StaticString s_JSON_FB_EXTRA_ESCAPES("JSON_FB_EXTRA_ESCAPES");
const StaticString s_JSON_FB_COLLECTIONS("JSON_FB_COLLECTIONS");
const StaticString s_JSON_FB_STABLE_MAPS("JSON_FB_STABLE_MAPS");
const StaticString s_JSON_ERROR_NONE("JSON_ERROR_NONE");
const StaticString s_JSON_ERROR_DEPTH("JSON_ERROR_DEPTH");
const StaticString s_JSON_ERROR_STATE_MISMATCH("JSON_ERROR_STATE_MISMATCH");
const StaticString s_JSON_ERROR_CTRL_CHAR("JSON_ERROR_CTRL_CHAR");
const StaticString s_JSON_ERROR_SYNTAX("JSON_ERROR_SYNTAX");
const StaticString s_JSON_ERROR_UTF8("JSON_ERROR_UTF8");

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

    HHVM_FE(json_last_error);
    HHVM_FE(json_last_error_msg);
    HHVM_FE(json_encode);
    HHVM_FE(json_decode);

    loadSystemlib();
  }
} s_json_extension;

}
