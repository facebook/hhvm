/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext/ext_json.h"
#include "hphp/runtime/ext/JSON_parser.h"
#include "hphp/runtime/base/utf8_decode.h"
#include "hphp/runtime/base/variable_serializer.h"

namespace HPHP {
IMPLEMENT_DEFAULT_EXTENSION(json);
///////////////////////////////////////////////////////////////////////////////
const int64_t k_JSON_HEX_TAG           = 1<<0;
const int64_t k_JSON_HEX_AMP           = 1<<1;
const int64_t k_JSON_HEX_APOS          = 1<<2;
const int64_t k_JSON_HEX_QUOT          = 1<<3;
const int64_t k_JSON_FORCE_OBJECT      = 1<<4;
const int64_t k_JSON_NUMERIC_CHECK     = 1<<5;
const int64_t k_JSON_UNESCAPED_SLASHES = 1<<6;
const int64_t k_JSON_PRETTY_PRINT      = 1<<7;
const int64_t k_JSON_UNESCAPED_UNICODE = 1<<8;
// intentionally higher so when PHP adds more options we're fine
const int64_t k_JSON_FB_LOOSE          = 1<<20;
const int64_t k_JSON_FB_UNLIMITED      = 1<<21;
const int64_t k_JSON_FB_EXTRA_ESCAPES  = 1<<22;

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
int f_json_last_error() {
  return (int) json_get_last_error_code();
}

String f_json_last_error_msg() {
  return json_get_last_error_msg();
}

String f_json_encode(CVarRef value, CVarRef options /* = 0 */) {
  int64_t json_options = options.toInt64();
  if (options.isBoolean() && options.toBooleanVal()) {
    json_options = k_JSON_FB_LOOSE;
  }

  VariableSerializer vs(VariableSerializer::Type::JSON, json_options);
  return vs.serializeValue(value, !(json_options & k_JSON_FB_UNLIMITED));
}

Variant f_json_decode(CStrRef json, bool assoc /* = false */,
                      CVarRef options /* = 0 */) {

  json_set_last_error_code(json_error_codes::JSON_ERROR_NONE);

  if (json.empty()) {
    return uninit_null();
  }

  int64_t json_options = options.toInt64();
  if (options.isBoolean() && options.toBooleanVal()) {
    json_options = k_JSON_FB_LOOSE;
  }

  Variant z;
  if (JSON_parser(z, json.data(), json.size(), assoc,
                  (json_options & k_JSON_FB_LOOSE))) {
    return z;
  }

  if (json.size() == 4) {
    if (!strcasecmp(json.data(), "null")) return uninit_null();
    if (!strcasecmp(json.data(), "true")) return true;
  } else if (json.size() == 5 && !strcasecmp(json.data(), "false")) {
    return false;
  }

  int64_t p;
  double d;
  DataType type = json->isNumericWithVal(p, d, 0);
  if (type == KindOfInt64) {
    return p;
  } else if (type == KindOfDouble) {
    return d;
  }

  char ch0 = json.charAt(0);
  if (json.size() > 1 && ch0 == '"' && json.charAt(json.size() - 1) == '"') {
    return json.substr(1, json.size() - 2);
  }

  if ((json_options & k_JSON_FB_LOOSE) && json.size() > 1 &&
      ch0 == '\'' && json.charAt(json.size() - 1) == '\'') {
    return json.substr(1, json.size() - 2);
  }

  if (ch0 == '{' || ch0 == '[') { /* invalid JSON string */
    json_set_last_error_code(json_error_codes::JSON_ERROR_SYNTAX);
  }

  assert(json_get_last_error_code() != json_error_codes::JSON_ERROR_NONE);
  return uninit_null();
}

///////////////////////////////////////////////////////////////////////////////
}
