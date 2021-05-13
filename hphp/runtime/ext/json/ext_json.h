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

#pragma once

#include "hphp/runtime/ext/extension.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

TypedValue HHVM_FUNCTION(json_encode, const Variant& value,
                         int64_t options = 0, int64_t depth = 512);
TypedValue HHVM_FUNCTION(json_encode_with_error, const Variant& value,
                         Variant& error,  int64_t options = 0,
                         int64_t depth = 512);
TypedValue HHVM_FUNCTION(json_decode, const String& json, bool assoc = false,
                         int64_t depth = 512, int64_t options = 0);
TypedValue HHVM_FUNCTION(json_decode_with_error, const String& json,
                         Variant& error, bool assoc = false,
                         int64_t depth = 512, int64_t options = 0);

extern const int64_t k_JSON_HEX_TAG;
extern const int64_t k_JSON_HEX_AMP;
extern const int64_t k_JSON_HEX_APOS;
extern const int64_t k_JSON_HEX_QUOT;
extern const int64_t k_JSON_FORCE_OBJECT;
extern const int64_t k_JSON_NUMERIC_CHECK;
extern const int64_t k_JSON_UNESCAPED_SLASHES;
extern const int64_t k_JSON_PRETTY_PRINT;
extern const int64_t k_JSON_UNESCAPED_UNICODE;
extern const int64_t k_JSON_PARTIAL_OUTPUT_ON_ERROR;
extern const int64_t k_JSON_PRESERVE_ZERO_FRACTION;
extern const int64_t k_JSON_FB_DARRAYS;
extern const int64_t k_JSON_FB_LOOSE;
extern const int64_t k_JSON_FB_EXTRA_ESCAPES;
extern const int64_t k_JSON_FB_UNLIMITED;
extern const int64_t k_JSON_FB_COLLECTIONS;
extern const int64_t k_JSON_FB_HACK_ARRAYS;
extern const int64_t k_JSON_FB_STABLE_MAPS;
extern const int64_t k_JSON_BIGINT_AS_STRING;
extern const int64_t k_JSON_FB_DARRAYS_AND_VARRAYS;
extern const int64_t k_JSON_FB_THRIFT_SIMPLE_JSON;
extern const int64_t k_JSON_FB_FORCE_HACK_ARRAYS;

extern const int64_t k_JSON_ERROR_NONE;
extern const int64_t k_JSON_ERROR_DEPTH;
extern const int64_t k_JSON_ERROR_STATE_MISMATCH;
extern const int64_t k_JSON_ERROR_CTRL_CHAR;
extern const int64_t k_JSON_ERROR_SYNTAX;
extern const int64_t k_JSON_ERROR_UTF8;
///////////////////////////////////////////////////////////////////////////////
}
