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

#ifndef incl_HPHP_EXT_ICONV_H_
#define incl_HPHP_EXT_ICONV_H_

#include "hphp/runtime/base/base-includes.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////


static Variant HHVM_FUNCTION(iconv_mime_encode,
    const String& field_name, const String& field_value,
    const Variant& preferences = null_variant);

static Variant HHVM_FUNCTION(iconv_mime_decode,
    const String& encoded_string, int64_t mode = 0,
    const Variant& charset = null_string);

static Variant HHVM_FUNCTION(iconv_mime_decode_headers,
    const String& encoded_headers,
    int64_t mode = 0,
    const Variant& charset = null_string);

static Variant HHVM_FUNCTION(iconv_get_encoding, const String& type = "all");

static bool HHVM_FUNCTION(iconv_set_encoding, const String& type,
		const String& charset);

static Variant HHVM_FUNCTION(iconv, const String& in_charset,
    const String& out_charset, const String& str);

static Variant HHVM_FUNCTION(iconv_strlen,
    const String& str, const Variant& charset = null_string);

static Variant HHVM_FUNCTION(iconv_strpos,
    const String& haystack, const String& needle, int64_t offset = 0,
    const Variant& charset = null_string);

static Variant HHVM_FUNCTION(iconv_strrpos,
    const String& haystack, const String& needle,
    const Variant& charset = null_string);

static Variant HHVM_FUNCTION(iconv_substr,
    const String& str, int64_t offset, int64_t length = INT64_MAX,
    const Variant& charset = null_string);

static String HHVM_FUNCTION(ob_iconv_handler,
    const String& contents, int64_t status);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_ICONV_H_
