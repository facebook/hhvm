<?hh     /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */
function mb_list_encodings() { }
function mb_list_encodings_alias_names($name = null) { }
function mb_list_mime_names($name = null) { }
function mb_check_encoding($var = null, $encoding = null) { }
function mb_convert_case($str, $mode, $encoding = null) { }
function mb_convert_encoding($str, $to_encoding, $from_encoding = null_variant) { }
function mb_convert_kana($str, $option = null, $encoding = null) { }
function mb_convert_variables($to_encoding, $from_encoding, &$vars, ...) { }
function mb_decode_mimeheader($str) { }
function mb_decode_numericentity($str, $convmap, $encoding = null) { }
function mb_detect_encoding($str, $encoding_list = null_variant, $strict = null_variant) { }
function mb_detect_order($encoding_list = null_variant) { }
function mb_encode_mimeheader($str, $charset = null, $transfer_encoding = null, $linefeed = "\r\n", $indent = 0) { }
function mb_encode_numericentity($str, $convmap, $encoding = null) { }
function mb_ereg_match($pattern, $str, $option = null) { }
function mb_ereg_replace($pattern, $replacement, $str, $option = null) { }
function mb_ereg_search_getpos() { }
function mb_ereg_search_getregs() { }
function mb_ereg_search_init($str, $pattern = null, $option = null) { }
function mb_ereg_search_pos($pattern = null, $option = null) { }
function mb_ereg_search_regs($pattern = null, $option = null) { }
function mb_ereg_search_setpos($position) { }
function mb_ereg_search($pattern = null, $option = null) { }
function mb_ereg($pattern, $str, &$regs = null) { }
function mb_eregi_replace($pattern, $replacement, $str, $option = null) { }
function mb_eregi($pattern, $str, &$regs = null) { }
function mb_get_info($type = null) { }
function mb_http_input($type = null) { }
function mb_http_output($encoding = null) { }
function mb_internal_encoding($encoding = null) { }
function mb_language($language = null) { }
function mb_output_handler($contents, $status) { }
function mb_parse_str($encoded_string, &$result = null) { }
function mb_preferred_mime_name($encoding) { }
function mb_regex_encoding($encoding = null) { }
function mb_regex_set_options($options = null) { }
function mb_send_mail($to, $subject, $message, $headers = null, $extra_cmd = null) { }
function mb_split($pattern, $str, $count = -1) { }
function mb_strcut($str, $start, $length = 0x7FFFFFFF, $encoding = null) { }
function mb_strimwidth($str, $start, $width, $trimmarker = null, $encoding = null) { }
function mb_stripos($haystack, $needle, $offset = 0, $encoding = null) { }
function mb_stristr($haystack, $needle, $part = false, $encoding = null) { }
function mb_strlen($str, $encoding = null) { }
function mb_strpos($haystack, $needle, $offset = 0, $encoding = null) { }
function mb_strrchr($haystack, $needle, $part = false, $encoding = null) { }
function mb_strrichr($haystack, $needle, $part = false, $encoding = null) { }
function mb_strripos($haystack, $needle, $offset = 0, $encoding = null) { }
function mb_strrpos($haystack, $needle, $offset = 0, $encoding = null) { }
function mb_strstr($haystack, $needle, $part = false, $encoding = null) { }
function mb_strtolower($str, $encoding = null) { }
function mb_strtoupper($str, $encoding = null) { }
function mb_strwidth($str, $encoding = null) { }
function mb_substitute_character($substrchar = null_variant) { }
function mb_substr_count($haystack, $needle, $encoding = null) { }
function mb_substr($str, $start, $length = 0x7FFFFFFF, $encoding = null) { }
