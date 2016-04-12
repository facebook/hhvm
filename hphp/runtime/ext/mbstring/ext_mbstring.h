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

#ifndef incl_HPHP_EXT_MB_H_
#define incl_HPHP_EXT_MB_H_

#include "hphp/runtime/ext/extension.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Array HHVM_FUNCTION(mb_list_encodings);
Variant HHVM_FUNCTION(mb_list_encodings_alias_names,
                      const Variant& opt_name = null_variant);
Variant HHVM_FUNCTION(mb_list_mime_names,
                      const Variant& opt_name = null_variant);
bool HHVM_FUNCTION(mb_check_encoding,
                   const Variant& opt_var = null_variant,
                   const Variant& opt_encoding = null_variant);
Variant HHVM_FUNCTION(mb_convert_case,
                      const String& str,
                      int mode,
                      const Variant& opt_encoding = null_variant);
Variant HHVM_FUNCTION(mb_convert_encoding,
                      const String& str,
                      const String& to_encoding,
                      const Variant& from_encoding = null_variant);
Variant HHVM_FUNCTION(mb_convert_kana,
                      const String& str,
                      const Variant& opt_option = null_variant,
                      const Variant& opt_encoding = null_variant);
Variant HHVM_FUNCTION(mb_convert_variables,
                      const String& to_encoding,
                      const Variant& from_encoding,
                      VRefParam vars,
                      const Array& args = null_array);
Variant HHVM_FUNCTION(mb_decode_mimeheader,
                      const String& str);
Variant HHVM_FUNCTION(mb_decode_numericentity,
                      const String& str,
                      const Variant& convmap,
                      const Variant& opt_encoding = null_variant);
Variant HHVM_FUNCTION(mb_detect_encoding,
                      const String& str,
                      const Variant& encoding_list = null_variant,
                      const Variant& strict = null_variant);
Variant HHVM_FUNCTION(mb_detect_order,
                      const Variant& encoding_list = null_variant);
Variant HHVM_FUNCTION(mb_encode_mimeheader,
                      const String& str,
                      const Variant& opt_charset = null_variant,
                      const Variant& opt_transfer_encoding = null_variant,
                      const String& linefeed = "\r\n",
                      int indent = 0);
Variant HHVM_FUNCTION(mb_encode_numericentity,
                      const String& str,
                      const Variant& convmap,
                      const Variant& opt_encoding = null_variant,
                      bool is_hex = false);
Variant HHVM_FUNCTION(mb_encoding_aliases,
                      const String& name);
bool HHVM_FUNCTION(mb_ereg_match,
                   const String& pattern,
                   const String& str,
                   const Variant& opt_option = null_variant);
Variant HHVM_FUNCTION(mb_ereg_replace,
                      const Variant& pattern,
                      const String& replacement,
                      const String& str,
                      const Variant& opt_option = null_variant);
int64_t HHVM_FUNCTION(mb_ereg_search_getpos);
Variant HHVM_FUNCTION(mb_ereg_search_getregs);
bool HHVM_FUNCTION(mb_ereg_search_init,
                   const String& str,
                   const Variant& opt_pattern = null_variant,
                   const Variant& opt_option = null_variant);
Variant HHVM_FUNCTION(mb_ereg_search_pos,
                      const Variant& opt_pattern = null_variant,
                      const Variant& opt_option = null_variant);
Variant HHVM_FUNCTION(mb_ereg_search_regs,
                      const Variant& opt_pattern = null_variant,
                      const Variant& opt_option = null_variant);
bool HHVM_FUNCTION(mb_ereg_search_setpos,
                   int position);
Variant HHVM_FUNCTION(mb_ereg_search,
                      const Variant& opt_pattern = null_variant,
                      const Variant& opt_option = null_variant);
Variant HHVM_FUNCTION(mb_ereg,
                      const Variant& pattern,
                      const String& str,
                      VRefParam regs = uninit_null());
Variant HHVM_FUNCTION(mb_eregi_replace,
                      const Variant& pattern,
                      const String& replacement,
                      const String& str,
                      const Variant& opt_option = null_variant);
Variant HHVM_FUNCTION(mb_eregi,
                      const Variant& pattern,
                      const String& str,
                      VRefParam regs = uninit_null());
Variant HHVM_FUNCTION(mb_get_info,
                      const Variant& opt_type = null_variant);
Variant HHVM_FUNCTION(mb_http_input,
                      const Variant& opt_type = null_variant);
Variant HHVM_FUNCTION(mb_http_output,
                      const Variant& opt_encoding = null_variant);
Variant HHVM_FUNCTION(mb_internal_encoding,
                      const Variant& opt_encoding = null_variant);
Variant HHVM_FUNCTION(mb_language,
                      const Variant& opt_language = null_variant);
String HHVM_FUNCTION(mb_output_handler,
                     const String& contents,
                     int status);
bool HHVM_FUNCTION(mb_parse_str,
                   const String& encoded_string,
                   VRefParam result = uninit_null());
Variant HHVM_FUNCTION(mb_preferred_mime_name,
                      const String& encoding);
Variant HHVM_FUNCTION(mb_regex_encoding,
                      const Variant& opt_encoding = null_variant);
String HHVM_FUNCTION(mb_regex_set_options,
                     const Variant& opt_options = null_variant);
bool HHVM_FUNCTION(mb_send_mail,
                   const String& to,
                   const String& subject,
                   const String& message,
                   const Variant& opt_headers = null_variant,
                   const Variant& opt_extra_cmd = null_variant);
Variant HHVM_FUNCTION(mb_split,
                      const String& pattern,
                      const String& str,
                      int count = -1);
Variant HHVM_FUNCTION(mb_strcut,
                      const String& str,
                      int start,
                      const Variant& length = uninit_null(),
                      const Variant& opt_encoding = null_variant);
Variant HHVM_FUNCTION(mb_strimwidth,
                      const String& str,
                      int start,
                      int width,
                      const Variant& opt_trimmarker = null_variant,
                      const Variant& opt_encoding = null_variant);
Variant HHVM_FUNCTION(mb_stripos,
                      const String& haystack,
                      const String& needle,
                      int offset = 0,
                      const Variant& opt_encoding = null_variant);
Variant HHVM_FUNCTION(mb_stristr,
                      const String& haystack,
                      const String& needle,
                      bool part = false,
                      const Variant& opt_encoding = null_variant);
Variant HHVM_FUNCTION(mb_strlen,
                      const String& str,
                      const Variant& opt_encoding = null_variant);
Variant HHVM_FUNCTION(mb_strpos,
                      const String& haystack,
                      const String& needle,
                      int offset = 0,
                      const Variant& opt_encoding = null_variant);
Variant HHVM_FUNCTION(mb_strrchr,
                      const String& haystack,
                      const String& needle,
                      bool part = false,
                      const Variant& opt_encoding = null_variant);
Variant HHVM_FUNCTION(mb_strrichr,
                      const String& haystack,
                      const String& needle,
                      bool part = false,
                      const Variant& opt_encoding = null_variant);
Variant HHVM_FUNCTION(mb_strripos,
                      const String& haystack,
                      const String& needle,
                      int offset = 0,
                      const Variant& opt_encoding = null_variant);
Variant HHVM_FUNCTION(mb_strrpos,
                      const String& haystack,
                      const String& needle,
                      const Variant& offset = 0LL,
                      const Variant& opt_encoding = null_variant);
Variant HHVM_FUNCTION(mb_strstr,
                      const String& haystack,
                      const String& needle,
                      bool part = false,
                      const Variant& opt_encoding = null_variant);
Variant HHVM_FUNCTION(mb_strtolower,
                      const String& str,
                      const Variant& opt_encoding = null_variant);
Variant HHVM_FUNCTION(mb_strtoupper,
                      const String& str,
                      const Variant& opt_encoding = null_variant);
Variant HHVM_FUNCTION(mb_strwidth,
                      const String& str,
                      const Variant& opt_encoding = null_variant);
Variant HHVM_FUNCTION(mb_substitute_character,
                      const Variant& substrchar = null_variant);
Variant HHVM_FUNCTION(mb_substr_count,
                      const String& haystack,
                      const String& needle,
                      const Variant& opt_encoding = null_variant);
Variant HHVM_FUNCTION(mb_substr,
                      const String& str,
                      int start,
                      const Variant& length = uninit_null(),
                      const Variant& opt_encoding = null_variant);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_MB_H_
