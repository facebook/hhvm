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

#ifndef incl_HPHP_EXT_ICONV_H_
#define incl_HPHP_EXT_ICONV_H_

#include "hphp/runtime/base/base-includes.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Variant f_iconv_mime_encode(
  const String& field_name, const String& field_value,
  CVarRef preferences = null_variant);
Variant f_iconv_mime_decode(
  const String& encoded_string, int mode = 0,
  const String& charset = null_string);
Variant f_iconv_mime_decode_headers(
  const String& encoded_headers, int mode = 0,
  const String& charset = null_string);
Variant f_iconv_get_encoding(const String& type = "all");
bool f_iconv_set_encoding(const String& type, const String& charset);
Variant f_iconv(
  const String& in_charset, const String& out_charset, const String& str);
Variant f_iconv_strlen(const String& str, const String& charset = null_string);
Variant f_iconv_strpos(
  const String& haystack, const String& needle, int offset = 0,
  const String& charset = null_string);
Variant f_iconv_strrpos(
  const String& haystack, const String& needle,
  const String& charset = null_string);
Variant f_iconv_substr(
  const String& str, int offset, int length = INT_MAX,
  const String& charset = null_string);
String f_ob_iconv_handler(const String& contents, int status);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_ICONV_H_
