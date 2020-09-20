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

Variant HHVM_FUNCTION(filter_list);
Variant HHVM_FUNCTION(filter_id,
                      const String& filtername);
Variant HHVM_FUNCTION(filter_var,
                      const Variant& variable,
                      int64_t filter,
                      const Variant& options);

extern const int64_t k_FILTER_FLAG_NONE;
extern const int64_t k_FILTER_REQUIRE_SCALAR;
extern const int64_t k_FILTER_REQUIRE_ARRAY;
extern const int64_t k_FILTER_FORCE_ARRAY;
extern const int64_t k_FILTER_NULL_ON_FAILURE;
extern const int64_t k_FILTER_VALIDATE_INT;
extern const int64_t k_FILTER_VALIDATE_BOOLEAN;
extern const int64_t k_FILTER_VALIDATE_FLOAT;
extern const int64_t k_FILTER_VALIDATE_REGEXP;
extern const int64_t k_FILTER_VALIDATE_URL;
extern const int64_t k_FILTER_VALIDATE_EMAIL;
extern const int64_t k_FILTER_VALIDATE_IP;
extern const int64_t k_FILTER_VALIDATE_MAC;
extern const int64_t k_FILTER_DEFAULT;
extern const int64_t k_FILTER_UNSAFE_RAW;
extern const int64_t k_FILTER_SANITIZE_STRING;
extern const int64_t k_FILTER_SANITIZE_STRIPPED;
extern const int64_t k_FILTER_SANITIZE_ENCODED;
extern const int64_t k_FILTER_SANITIZE_SPECIAL_CHARS;
extern const int64_t k_FILTER_SANITIZE_FULL_SPECIAL_CHARS;
extern const int64_t k_FILTER_SANITIZE_EMAIL;
extern const int64_t k_FILTER_SANITIZE_URL;
extern const int64_t k_FILTER_SANITIZE_NUMBER_INT;
extern const int64_t k_FILTER_SANITIZE_NUMBER_FLOAT;
extern const int64_t k_FILTER_SANITIZE_MAGIC_QUOTES;
extern const int64_t k_FILTER_CALLBACK;
extern const int64_t k_FILTER_FLAG_ALLOW_OCTAL;
extern const int64_t k_FILTER_FLAG_ALLOW_HEX;
extern const int64_t k_FILTER_FLAG_STRIP_LOW;
extern const int64_t k_FILTER_FLAG_STRIP_HIGH;
extern const int64_t k_FILTER_FLAG_ENCODE_LOW;
extern const int64_t k_FILTER_FLAG_ENCODE_HIGH;
extern const int64_t k_FILTER_FLAG_ENCODE_AMP;
extern const int64_t k_FILTER_FLAG_NO_ENCODE_QUOTES;
extern const int64_t k_FILTER_FLAG_EMPTY_STRING_NULL;
extern const int64_t k_FILTER_FLAG_STRIP_BACKTICK;
extern const int64_t k_FILTER_FLAG_ALLOW_FRACTION;
extern const int64_t k_FILTER_FLAG_ALLOW_THOUSAND;
extern const int64_t k_FILTER_FLAG_ALLOW_SCIENTIFIC;
extern const int64_t k_FILTER_FLAG_SCHEME_REQUIRED;
extern const int64_t k_FILTER_FLAG_HOST_REQUIRED;
extern const int64_t k_FILTER_FLAG_PATH_REQUIRED;
extern const int64_t k_FILTER_FLAG_QUERY_REQUIRED;
extern const int64_t k_FILTER_FLAG_IPV4;
extern const int64_t k_FILTER_FLAG_IPV6;
extern const int64_t k_FILTER_FLAG_NO_RES_RANGE;
extern const int64_t k_FILTER_FLAG_NO_PRIV_RANGE;

///////////////////////////////////////////////////////////////////////////////
}

