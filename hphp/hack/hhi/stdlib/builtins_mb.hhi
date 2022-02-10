<?hh     /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

const MB_OVERLOAD_MAIL = 1;
const MB_OVERLOAD_STRING = 2;
const MB_OVERLOAD_REGEX = 4;
const MB_CASE_UPPER = 0;
const MB_CASE_LOWER = 1;
const MB_CASE_TITLE = 2;
<<__PHPStdLib>>
function mb_list_encodings()[] { }
<<__PHPStdLib>>
function mb_list_encodings_alias_names($name = null)[] { }
<<__PHPStdLib>>
function mb_list_mime_names($name = null)[] { }
<<__PHPStdLib>>
function mb_check_encoding($var = null, $encoding = null)[read_globals] { }
<<__PHPStdLib>>
function mb_convert_case(string $str, int $mode, $encoding = null)[read_globals] { }
<<__PHPStdLib>>
function mb_convert_encoding(string $str, string $to_encoding, $from_encoding = null)[read_globals] { }
<<__PHPStdLib>>
function mb_convert_kana(string $str, $option = null, $encoding = null)[read_globals] { }
<<__PHPStdLib>>
function mb_convert_variables(string $to_encoding, $from_encoding, inout $vars, ...$args)[read_globals] { }
<<__PHPStdLib>>
function mb_decode_mimeheader(string $str)[read_globals] { }
<<__PHPStdLib>>
function mb_decode_numericentity(string $str, $convmap, $encoding = null)[read_globals] { }
<<__PHPStdLib>>
function mb_detect_encoding(string $str, $encoding_list = null, $strict = null)[read_globals] { }
<<__PHPStdLib>>
function mb_detect_order($encoding_list = null)[read_globals] { }
<<__PHPStdLib>>
function mb_encode_mimeheader(string $str, $charset = null, $transfer_encoding = null, string $linefeed = "\r\n", int $indent = 0)[read_globals] { }
<<__PHPStdLib>>
function mb_encode_numericentity(string $str, $convmap, $encoding = null)[read_globals] { }
<<__PHPStdLib>>
function mb_encoding_aliases(string $str)[] { }
<<__PHPStdLib>>
function mb_ereg_match(string $pattern, string $str, $option = null)[read_globals] { }
<<__PHPStdLib>>
function mb_ereg_replace($pattern, string $replacement, string $str, $option = null)[read_globals] { }
<<__PHPStdLib>>
function mb_ereg_search_getpos()[read_globals] { }
<<__PHPStdLib>>
function mb_ereg_search_getregs()[read_globals] { }
<<__PHPStdLib>>
function mb_ereg_search_init(string $str, $pattern = null, $option = null)[read_globals] { }
<<__PHPStdLib>>
function mb_ereg_search_pos($pattern = null, $option = null)[read_globals] { }
<<__PHPStdLib>>
function mb_ereg_search_regs($pattern = null, $option = null)[read_globals] { }
<<__PHPStdLib>>
function mb_ereg_search_setpos(int $position)[leak_safe] { }
<<__PHPStdLib>>
function mb_ereg_search($pattern = null, $option = null)[read_globals] { }
<<__PHPStdLib>>
function mb_ereg($pattern, string $str, inout $regs)[leak_safe] { }
<<__PHPStdLib>>
function mb_eregi_replace($pattern, string $replacement, string $str, $option = null)[leak_safe] { }
<<__PHPStdLib>>
function mb_eregi($pattern, string $str, inout $regs)[leak_safe] { }
<<__PHPStdLib>>
function mb_get_info($type = null)[read_globals] { }
<<__PHPStdLib>>
function mb_http_input($type = null)[read_globals] { }
<<__PHPStdLib>>
function mb_http_output($encoding = null)[leak_safe] { }
<<__PHPStdLib>>
function mb_internal_encoding($encoding = null)[leak_safe] { }
<<__PHPStdLib>>
function mb_language($language = null)[leak_safe] { }
<<__PHPStdLib>>
function mb_output_handler(string $contents, int $status)[leak_safe] { }
<<__PHPStdLib>>
function mb_parse_str(string $encoded_string, inout $result)[leak_safe] { }
<<__PHPStdLib>>
function mb_preferred_mime_name(string $encoding)[] { }
<<__PHPStdLib>>
function mb_regex_encoding($encoding = null)[leak_safe] { }
<<__PHPStdLib>>
function mb_regex_set_options($options = null)[leak_safe] { }
<<__PHPStdLib>>
function mb_send_mail(string $to, string $subject, string $message, $headers = null, $extra_cmd = null) { }
<<__PHPStdLib>>
function mb_split(string $pattern, string $str, int $count = -1)[leak_safe] { }
<<__PHPStdLib>>
function mb_strcut(string $str, int $start, $length = 0x7FFFFFFF, $encoding = null)[read_globals] { }
<<__PHPStdLib>>
function mb_strimwidth(string $str, int $start, int $width, $trimmarker = null, $encoding = null)[read_globals] { }
<<__PHPStdLib>>
function mb_stripos(string $haystack, string $needle, int $offset = 0, $encoding = null)[read_globals] { }
<<__PHPStdLib>>
function mb_stristr(string $haystack, string $needle, bool $part = false, $encoding = null)[read_globals] { }
<<__PHPStdLib>>
function mb_strlen(string $str, $encoding = null)[read_globals] { }
<<__PHPStdLib>>
function mb_strpos(string $haystack, string $needle, int $offset = 0, $encoding = null)[read_globals] { }
<<__PHPStdLib>>
function mb_strrchr(string $haystack, string $needle, bool $part = false, $encoding = null)[read_globals] { }
<<__PHPStdLib>>
function mb_strrichr(string $haystack, string $needle, bool $part = false, $encoding = null)[read_globals] { }
<<__PHPStdLib>>
function mb_strripos(string $haystack, string $needle, int $offset = 0, $encoding = null)[read_globals] { }
<<__PHPStdLib>>
function mb_strrpos(string $haystack, string $needle, $offset = 0, $encoding = null)[read_globals] { }
<<__PHPStdLib>>
function mb_strstr(string $haystack, string $needle, bool $part = false, $encoding = null) { }
<<__PHPStdLib>>
function mb_strtolower(string $str, $encoding = null)[read_globals] { }
<<__PHPStdLib>>
function mb_strtoupper(string $str, $encoding = null)[read_globals] { }
<<__PHPStdLib>>
function mb_strwidth(string $str, $encoding = null)[read_globals] { }
<<__PHPStdLib>>
function mb_substitute_character($substrchar = null)[leak_safe] { }
<<__PHPStdLib>>
function mb_substr_count(string $haystack, string $needle, $encoding = null)[read_globals] { }
<<__PHPStdLib>>
function mb_substr(string $str, int $start, $length = 0x7FFFFFFF, $encoding = null)[read_globals] { }
