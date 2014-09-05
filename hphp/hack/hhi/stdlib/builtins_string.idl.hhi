<?hh // decl    /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */
function addcslashes($str, $charlist): string { }
function stripcslashes($str): string { }
function addslashes($str): string { }
function stripslashes($str): string { }
function bin2hex($str): string { }
function hex2bin($str): string { }
function nl2br($str): string { }
function quotemeta($str): string { }
function str_shuffle($str): string { }
function strrev($str): string { }
function strtolower($str): string { }
function strtoupper($str): string { }
function ucfirst($str): string { }
function lcfirst($str): string { }
function ucwords($str): string { }
function trim($str, $charlist = HPHP_TRIM_CHARLIST): string { }
function ltrim($str, $charlist = HPHP_TRIM_CHARLIST): string { }
function rtrim($str, $charlist = HPHP_TRIM_CHARLIST): string { }
function chop($str, $charlist = HPHP_TRIM_CHARLIST) { }
function join($glue, $pieces = null_variant) { }
function str_split($str, $split_length = 1) { }
function chunk_split($body, $chunklen = 76, $end = "\r\n") { }
function strtok($str, $token = null_variant) { }
function str_replace($search, $replace, $subject, &$count = null): string { }
function str_ireplace($search, $replace, $subject, &$count = null): string { }
function substr_replace($str, $replacement, $start, $length = 0x7FFFFFFF): string { }
function substr($str, $start, $length = 0x7FFFFFFF): string { }
function str_pad($input, $pad_length, $pad_string = " ", $pad_type = STR_PAD_RIGHT): string { }
function str_repeat($input, $multiplier): string { }
function wordwrap($str, $width = 75, $wordbreak = "\n", $cut = false): string { }
function html_entity_decode($str, $quote_style = ENT_COMPAT, $charset = "ISO-8859-1"): string { }
function htmlentities($str, $quote_style = ENT_COMPAT, $charset = "ISO-8859-1", $double_encode = true): string { }
function htmlspecialchars_decode($str, $quote_style = ENT_COMPAT): string { }
function htmlspecialchars($str, $quote_style = ENT_COMPAT, $charset = "ISO-8859-1", $double_encode = true): string { }
function fb_htmlspecialchars($str, $quote_style = ENT_COMPAT, $charset = "ISO-8859-1", $extra = array()): string { }
function quoted_printable_encode($str): string { }
function quoted_printable_decode($str): string { }
function convert_uudecode($data) { }
function convert_uuencode($data) { }
function str_rot13($str): string { }
function crc32($str) { }
function crypt($str, $salt = "") { }
function md5($str, $raw_output = false): string { }
function sha1($str, $raw_output = false): string { }
function strtr($str, $from, $to = null_variant): string { }
function convert_cyr_string($str, $from, $to) { }
function get_html_translation_table($table = 0, $quote_style = ENT_COMPAT) { }
function hebrev($hebrew_text, $max_chars_per_line = 0): string { }
function hebrevc($hebrew_text, $max_chars_per_line = 0): string { }
function setlocale($category, $locale, ...) { }
function localeconv() { }
function nl_langinfo($item) { }
function vprintf($format, $args): string { }
function vsprintf($format, $args): string { }
function sscanf($str, $format, ...) { }
function money_format($format, $number): ?string { }
function number_format($number, $decimals = 0, $dec_point = ".", $thousands_sep = ","): string { }
function strcmp($str1, $str2) { }
function strncmp($str1, $str2, $len) { }
function strnatcmp($str1, $str2) { }
function strcasecmp($str1, $str2) { }
function strncasecmp($str1, $str2, $len) { }
function strnatcasecmp($str1, $str2) { }
function strcoll($str1, $str2) { }
function substr_compare($main_str, $str, $offset, $length = PHP_INT_MAX, $case_insensitivity = false) { }
function strchr($haystack, $needle) { }
function strrchr($haystack, $needle) { }
function strstr($haystack, $needle, $before_needle = false) { }
function stristr($haystack, $needle) { }
function strpbrk($haystack, $char_list) { }
function strpos($haystack, $needle, $offset = 0) { }
function stripos($haystack, $needle, $offset = 0) { }
function strrpos($haystack, $needle, $offset = 0) { }
function strripos($haystack, $needle, $offset = 0) { }
function substr_count($haystack, $needle, $offset = 0, $length = 0x7FFFFFFF) { }
function strspn($str1, $str2, $start = 0, $length = 0x7FFFFFFF) { }
function strcspn($str1, $str2, $start = 0, $length = 0x7FFFFFFF) { }
function strlen($vstr) { }
function count_chars($str, $mode = 0) { }
function str_word_count($str, $format = 0, $charlist = "") { }
function levenshtein($str1, $str2, $cost_ins = 1, $cost_rep = 1, $cost_del = 1) { }
function similar_text($first, $second, &$percent = null) { }
function soundex($str) { }
function metaphone($str, $phones = 0) { }
function parse_str($str, &$arr = null) { }
