/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifndef __EXT_URL_H__
#define __EXT_URL_H__

#include <runtime/base/base_includes.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Variant f_base64_decode(CStrRef data, bool strict = false);
String f_base64_encode(CStrRef data);

Variant f_get_headers(CStrRef url, int format = 0);
Array f_get_meta_tags(CStrRef filename, bool use_include_path = false);

Variant f_http_build_query(CVarRef formdata,
                           CStrRef numeric_prefix = null_string,
                           CStrRef arg_separator = null_string);
Variant f_parse_url(CStrRef url, int component = -1);

inline String f_rawurldecode(CStrRef str) {
  return StringUtil::UrlDecode(str, false);
}

inline String f_rawurlencode(CStrRef str) {
  return StringUtil::UrlEncode(str, false);
}

inline String f_urldecode(CStrRef str) {
  return StringUtil::UrlDecode(str, true);
}

inline String f_urlencode(CStrRef str) {
  return StringUtil::UrlEncode(str, true);
}

///////////////////////////////////////////////////////////////////////////////
}

#endif // __EXT_URL_H__
