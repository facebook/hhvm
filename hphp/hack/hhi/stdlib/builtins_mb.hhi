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
function mb_list_encodings() { }
<<__PHPStdLib>>
function mb_list_encodings_alias_names($name = null) { }
<<__PHPStdLib>>
function mb_list_mime_names($name = null) { }
<<__PHPStdLib>>
function mb_check_encoding($var = null, $encoding = null) { }
<<__PHPStdLib>>
function mb_convert_case($str, $mode, $encoding = null) { }
<<__PHPStdLib>>
function mb_convert_encoding($str, $to_encoding, $from_encoding = null) { }
<<__PHPStdLib>>
function mb_convert_kana($str, $option = null, $encoding = null) { }
<<__PHPStdLib>>
function mb_convert_variables($to_encoding, $from_encoding, &$vars, ...) { }
<<__PHPStdLib>>
function mb_decode_mimeheader($str) { }
<<__PHPStdLib>>
function mb_decode_numericentity($str, $convmap, $encoding = null) { }
<<__PHPStdLib>>
function mb_detect_encoding($str, $encoding_list = null, $strict = null) { }
<<__PHPStdLib>>
function mb_detect_order($encoding_list = null) { }
<<__PHPStdLib>>
function mb_encode_mimeheader($str, $charset = null, $transfer_encoding = null, $linefeed = "\r\n", $indent = 0) { }
<<__PHPStdLib>>
function mb_encode_numericentity($str, $convmap, $encoding = null) { }
<<__PHPStdLib>>
function mb_ereg_match($pattern, $str, $option = null) { }
<<__PHPStdLib>>
function mb_ereg_replace($pattern, $replacement, $str, $option = null) { }
<<__PHPStdLib>>
function mb_ereg_search_getpos() { }
<<__PHPStdLib>>
function mb_ereg_search_getregs() { }
<<__PHPStdLib>>
function mb_ereg_search_init($str, $pattern = null, $option = null) { }
<<__PHPStdLib>>
function mb_ereg_search_pos($pattern = null, $option = null) { }
<<__PHPStdLib>>
function mb_ereg_search_regs($pattern = null, $option = null) { }
<<__PHPStdLib>>
function mb_ereg_search_setpos($position) { }
<<__PHPStdLib>>
function mb_ereg_search($pattern = null, $option = null) { }
<<__PHPStdLib>>
function mb_ereg($pattern, $str, &$regs = null) { }
<<__PHPStdLib>>
function mb_eregi_replace($pattern, $replacement, $str, $option = null) { }
<<__PHPStdLib>>
function mb_eregi($pattern, $str, &$regs = null) { }
<<__PHPStdLib>>
function mb_get_info($type = null) { }
<<__PHPStdLib>>
function mb_http_input($type = null) { }
<<__PHPStdLib>>
function mb_http_output($encoding = null) { }
<<__PHPStdLib>>
function mb_internal_encoding($encoding = null) { }
<<__PHPStdLib>>
function mb_language($language = null) { }
<<__PHPStdLib>>
function mb_output_handler($contents, $status) { }
<<__PHPStdLib>>
function mb_parse_str($encoded_string, &$result = null) { }
<<__PHPStdLib>>
function mb_preferred_mime_name($encoding) { }
<<__PHPStdLib>>
function mb_regex_encoding($encoding = null) { }
<<__PHPStdLib>>
function mb_regex_set_options($options = null) { }
<<__PHPStdLib>>
function mb_send_mail($to, $subject, $message, $headers = null, $extra_cmd = null) { }
<<__PHPStdLib>>
function mb_split($pattern, $str, $count = -1) { }
<<__PHPStdLib>>
function mb_strcut($str, $start, $length = 0x7FFFFFFF, $encoding = null) { }
<<__PHPStdLib>>
function mb_strimwidth($str, $start, $width, $trimmarker = null, $encoding = null) { }
<<__PHPStdLib>>
function mb_stripos($haystack, $needle, $offset = 0, $encoding = null) { }
<<__PHPStdLib>>
function mb_stristr($haystack, $needle, $part = false, $encoding = null) { }
<<__PHPStdLib>>
function mb_strlen($str, $encoding = null) { }
<<__PHPStdLib>>
function mb_strpos($haystack, $needle, $offset = 0, $encoding = null) { }
<<__PHPStdLib>>
function mb_strrchr($haystack, $needle, $part = false, $encoding = null) { }
<<__PHPStdLib>>
function mb_strrichr($haystack, $needle, $part = false, $encoding = null) { }
<<__PHPStdLib>>
function mb_strripos($haystack, $needle, $offset = 0, $encoding = null) { }
<<__PHPStdLib>>
function mb_strrpos($haystack, $needle, $offset = 0, $encoding = null) { }
<<__PHPStdLib>>
function mb_strstr($haystack, $needle, $part = false, $encoding = null) { }
<<__PHPStdLib>>
function mb_strtolower($str, $encoding = null) { }
<<__PHPStdLib>>
function mb_strtoupper($str, $encoding = null) { }
<<__PHPStdLib>>
function mb_strwidth($str, $encoding = null) { }
<<__PHPStdLib>>
function mb_substitute_character($substrchar = null) { }
<<__PHPStdLib>>
function mb_substr_count($haystack, $needle, $encoding = null) { }
<<__PHPStdLib>>
function mb_substr($str, $start, $length = 0x7FFFFFFF, $encoding = null) { }
