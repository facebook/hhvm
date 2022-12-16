<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

const int MB_OVERLOAD_MAIL;
const int MB_OVERLOAD_STRING;
const int MB_OVERLOAD_REGEX;
const int MB_CASE_UPPER;
const int MB_CASE_LOWER;
const int MB_CASE_TITLE;
<<__PHPStdLib>>
function mb_list_encodings()[]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_list_encodings_alias_names(
  HH\FIXME\MISSING_PARAM_TYPE $name = null,
)[]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_list_mime_names(
  HH\FIXME\MISSING_PARAM_TYPE $name = null,
)[]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_check_encoding(
  HH\FIXME\MISSING_PARAM_TYPE $var = null,
  HH\FIXME\MISSING_PARAM_TYPE $encoding = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_convert_case(
  string $str,
  int $mode,
  HH\FIXME\MISSING_PARAM_TYPE $encoding = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_convert_encoding(
  string $str,
  string $to_encoding,
  HH\FIXME\MISSING_PARAM_TYPE $from_encoding = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_convert_kana(
  string $str,
  HH\FIXME\MISSING_PARAM_TYPE $option = null,
  HH\FIXME\MISSING_PARAM_TYPE $encoding = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_convert_variables(
  string $to_encoding,
  HH\FIXME\MISSING_PARAM_TYPE $from_encoding,
  inout $vars,
  HH\FIXME\MISSING_PARAM_TYPE ...$args
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_decode_mimeheader(
  string $str,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_decode_numericentity(
  string $str,
  HH\FIXME\MISSING_PARAM_TYPE $convmap,
  HH\FIXME\MISSING_PARAM_TYPE $encoding = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_detect_encoding(
  string $str,
  HH\FIXME\MISSING_PARAM_TYPE $encoding_list = null,
  HH\FIXME\MISSING_PARAM_TYPE $strict = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_detect_order(
  HH\FIXME\MISSING_PARAM_TYPE $encoding_list = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_encode_mimeheader(
  string $str,
  HH\FIXME\MISSING_PARAM_TYPE $charset = null,
  HH\FIXME\MISSING_PARAM_TYPE $transfer_encoding = null,
  string $linefeed = "\r\n",
  int $indent = 0,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_encode_numericentity(
  string $str,
  HH\FIXME\MISSING_PARAM_TYPE $convmap,
  HH\FIXME\MISSING_PARAM_TYPE $encoding = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_encoding_aliases(string $str)[]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_ereg_match(
  string $pattern,
  string $str,
  HH\FIXME\MISSING_PARAM_TYPE $option = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_ereg_replace(
  HH\FIXME\MISSING_PARAM_TYPE $pattern,
  string $replacement,
  string $str,
  HH\FIXME\MISSING_PARAM_TYPE $option = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_ereg_search_getpos()[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_ereg_search_getregs()[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_ereg_search_init(
  string $str,
  HH\FIXME\MISSING_PARAM_TYPE $pattern = null,
  HH\FIXME\MISSING_PARAM_TYPE $option = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_ereg_search_pos(
  HH\FIXME\MISSING_PARAM_TYPE $pattern = null,
  HH\FIXME\MISSING_PARAM_TYPE $option = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_ereg_search_regs(
  HH\FIXME\MISSING_PARAM_TYPE $pattern = null,
  HH\FIXME\MISSING_PARAM_TYPE $option = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_ereg_search_setpos(
  int $position,
)[leak_safe]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_ereg_search(
  HH\FIXME\MISSING_PARAM_TYPE $pattern = null,
  HH\FIXME\MISSING_PARAM_TYPE $option = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_ereg(
  HH\FIXME\MISSING_PARAM_TYPE $pattern,
  string $str,
  inout $regs,
)[leak_safe]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_eregi_replace(
  HH\FIXME\MISSING_PARAM_TYPE $pattern,
  string $replacement,
  string $str,
  HH\FIXME\MISSING_PARAM_TYPE $option = null,
)[leak_safe]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_eregi(
  HH\FIXME\MISSING_PARAM_TYPE $pattern,
  string $str,
  inout $regs,
)[leak_safe]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_get_info(
  HH\FIXME\MISSING_PARAM_TYPE $type = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_http_input(
  HH\FIXME\MISSING_PARAM_TYPE $type = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_http_output(
  HH\FIXME\MISSING_PARAM_TYPE $encoding = null,
)[leak_safe]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_internal_encoding(
  HH\FIXME\MISSING_PARAM_TYPE $encoding = null,
)[leak_safe]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_language(
  HH\FIXME\MISSING_PARAM_TYPE $language = null,
)[leak_safe]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_output_handler(
  string $contents,
  int $status,
)[leak_safe]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_parse_str(
  string $encoded_string,
  inout $result,
)[leak_safe]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_preferred_mime_name(
  string $encoding,
)[]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_regex_encoding(
  HH\FIXME\MISSING_PARAM_TYPE $encoding = null,
)[leak_safe]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_regex_set_options(
  HH\FIXME\MISSING_PARAM_TYPE $options = null,
)[leak_safe]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_send_mail(
  string $to,
  string $subject,
  string $message,
  HH\FIXME\MISSING_PARAM_TYPE $headers = null,
  HH\FIXME\MISSING_PARAM_TYPE $extra_cmd = null,
): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_split(
  string $pattern,
  string $str,
  int $count = -1,
)[leak_safe]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_strcut(
  string $str,
  int $start,
  HH\FIXME\MISSING_PARAM_TYPE $length = 0x7FFFFFFF,
  HH\FIXME\MISSING_PARAM_TYPE $encoding = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_strimwidth(
  string $str,
  int $start,
  int $width,
  HH\FIXME\MISSING_PARAM_TYPE $trimmarker = null,
  HH\FIXME\MISSING_PARAM_TYPE $encoding = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_stripos(
  string $haystack,
  string $needle,
  int $offset = 0,
  HH\FIXME\MISSING_PARAM_TYPE $encoding = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_stristr(
  string $haystack,
  string $needle,
  bool $part = false,
  HH\FIXME\MISSING_PARAM_TYPE $encoding = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_strlen(
  string $str,
  HH\FIXME\MISSING_PARAM_TYPE $encoding = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_strpos(
  string $haystack,
  string $needle,
  int $offset = 0,
  HH\FIXME\MISSING_PARAM_TYPE $encoding = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_strrchr(
  string $haystack,
  string $needle,
  bool $part = false,
  HH\FIXME\MISSING_PARAM_TYPE $encoding = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_strrichr(
  string $haystack,
  string $needle,
  bool $part = false,
  HH\FIXME\MISSING_PARAM_TYPE $encoding = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_strripos(
  string $haystack,
  string $needle,
  int $offset = 0,
  HH\FIXME\MISSING_PARAM_TYPE $encoding = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_strrpos(
  string $haystack,
  string $needle,
  HH\FIXME\MISSING_PARAM_TYPE $offset = 0,
  HH\FIXME\MISSING_PARAM_TYPE $encoding = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_strstr(
  string $haystack,
  string $needle,
  bool $part = false,
  HH\FIXME\MISSING_PARAM_TYPE $encoding = null,
): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_strtolower(
  string $str,
  HH\FIXME\MISSING_PARAM_TYPE $encoding = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_strtoupper(
  string $str,
  HH\FIXME\MISSING_PARAM_TYPE $encoding = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_strwidth(
  string $str,
  HH\FIXME\MISSING_PARAM_TYPE $encoding = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_substitute_character(
  HH\FIXME\MISSING_PARAM_TYPE $substrchar = null,
)[leak_safe]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_substr_count(
  string $haystack,
  string $needle,
  HH\FIXME\MISSING_PARAM_TYPE $encoding = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_substr(
  string $str,
  int $start,
  HH\FIXME\MISSING_PARAM_TYPE $length = 0x7FFFFFFF,
  HH\FIXME\MISSING_PARAM_TYPE $encoding = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
