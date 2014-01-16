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

#ifndef incl_HPHP_EXT_URL_H_
#define incl_HPHP_EXT_URL_H_

#include "hphp/runtime/base/base-includes.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

extern const int64_t k_PHP_URL_SCHEME;
extern const int64_t k_PHP_URL_HOST;
extern const int64_t k_PHP_URL_PORT;
extern const int64_t k_PHP_URL_USER;
extern const int64_t k_PHP_URL_PASS;
extern const int64_t k_PHP_URL_PATH;
extern const int64_t k_PHP_URL_QUERY;
extern const int64_t k_PHP_URL_FRAGMENT;

Variant f_base64_decode(const String& data, bool strict = false);
String f_base64_encode(const String& data);

Variant f_get_headers(const String& url, int format = 0);
Array f_get_meta_tags(const String& filename, bool use_include_path = false);

Variant f_http_build_query(CVarRef formdata,
                           const String& numeric_prefix = null_string,
                           const String& arg_separator = null_string);
Variant f_parse_url(const String& url, int component = -1);

String f_rawurldecode(const String& str);

String f_rawurlencode(const String& str);

String f_urldecode(const String& str);

String f_urlencode(const String& str);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_URL_H_
