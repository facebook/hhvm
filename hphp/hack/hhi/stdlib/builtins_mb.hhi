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
  ?string $name = null,
)[]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_list_mime_names(
  ?string $name = null,
)[]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_check_encoding(
  ?string $var = null,
  ?string $encoding = null,
)[read_globals]: bool {}
<<__PHPStdLib>>
function mb_convert_case(
  string $str,
  int $mode,
  ?string $encoding = null,
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
  ?string $option = null,
  ?string $encoding = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_convert_variables(
  string $to_encoding,
  HH\FIXME\MISSING_PARAM_TYPE $from_encoding,
  inout HH\FIXME\MISSING_PARAM_TYPE $vars,
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
  ?string $encoding = null,
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
  ?string $charset = null,
  ?string $transfer_encoding = null,
  string $linefeed = "\r\n",
  int $indent = 0,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_encode_numericentity(
  string $str,
  HH\FIXME\MISSING_PARAM_TYPE $convmap,
  ?string $encoding = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_encoding_aliases(string $str)[]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_ereg_match(
  string $pattern,
  string $str,
  ?string $option = null,
)[read_globals]: bool {}
<<__PHPStdLib>>
function mb_ereg_replace(
  HH\FIXME\MISSING_PARAM_TYPE $pattern,
  string $replacement,
  string $str,
  ?string $option = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_ereg_search_getpos()[read_globals]: int {}
<<__PHPStdLib>>
function mb_ereg_search_getregs()[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_ereg_search_init(
  string $str,
  ?string $pattern = null,
  ?string $option = null,
)[read_globals]: bool {}
<<__PHPStdLib>>
function mb_ereg_search_pos(
  ?string $pattern = null,
  ?string $option = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_ereg_search_regs(
  ?string $pattern = null,
  ?string $option = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_ereg_search_setpos(
  int $position,
)[leak_safe]: bool {}
<<__PHPStdLib>>
function mb_ereg_search(
  ?string $pattern = null,
  ?string $option = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_ereg(
  HH\FIXME\MISSING_PARAM_TYPE $pattern,
  string $str,
  inout HH\FIXME\MISSING_PARAM_TYPE $regs,
)[leak_safe]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_eregi_replace(
  HH\FIXME\MISSING_PARAM_TYPE $pattern,
  string $replacement,
  string $str,
  ?string $option = null,
)[leak_safe]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_eregi(
  HH\FIXME\MISSING_PARAM_TYPE $pattern,
  string $str,
  inout HH\FIXME\MISSING_PARAM_TYPE $regs,
)[leak_safe]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_get_info(
  ?string $type = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_http_input(
  ?string $type = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_http_output(
  ?string $encoding = null,
)[leak_safe]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_internal_encoding(
  ?string $encoding = null,
)[leak_safe]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_language(
  ?string $language = null,
)[leak_safe]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_output_handler(
  string $contents,
  int $status,
)[leak_safe]: string {}
<<__PHPStdLib>>
function mb_parse_str(
  string $encoded_string,
  inout HH\FIXME\MISSING_PARAM_TYPE $result,
)[leak_safe]: bool {}
<<__PHPStdLib>>
function mb_preferred_mime_name(
  string $encoding,
)[]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_regex_encoding(
  ?string $encoding = null,
)[leak_safe]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_regex_set_options(
  ?string $options = null,
)[leak_safe]: string {}
<<__PHPStdLib>>
function mb_send_mail(
  string $to,
  string $subject,
  string $message,
  ?string $headers = null,
  ?string $extra_cmd = null,
): bool {}
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
  ?string $encoding = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_strimwidth(
  string $str,
  int $start,
  int $width,
  ?string $trimmarker = null,
  ?string $encoding = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_stripos(
  string $haystack,
  string $needle,
  int $offset = 0,
  ?string $encoding = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_stristr(
  string $haystack,
  string $needle,
  bool $part = false,
  ?string $encoding = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_strlen(
  string $str,
  ?string $encoding = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_strpos(
  string $haystack,
  string $needle,
  int $offset = 0,
  ?string $encoding = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_strrchr(
  string $haystack,
  string $needle,
  bool $part = false,
  ?string $encoding = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_strrichr(
  string $haystack,
  string $needle,
  bool $part = false,
  ?string $encoding = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_strripos(
  string $haystack,
  string $needle,
  int $offset = 0,
  ?string $encoding = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_strrpos(
  string $haystack,
  string $needle,
  HH\FIXME\MISSING_PARAM_TYPE $offset = 0,
  ?string $encoding = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_strstr(
  string $haystack,
  string $needle,
  bool $part = false,
  ?string $encoding = null,
): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_strtolower(
  string $str,
  ?string $encoding = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_strtoupper(
  string $str,
  ?string $encoding = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_strwidth(
  string $str,
  ?string $encoding = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_substitute_character(
  HH\FIXME\MISSING_PARAM_TYPE $substrchar = null,
)[leak_safe]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_substr_count(
  string $haystack,
  string $needle,
  ?string $encoding = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function mb_substr(
  string $str,
  int $start,
  HH\FIXME\MISSING_PARAM_TYPE $length = 0x7FFFFFFF,
  ?string $encoding = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
