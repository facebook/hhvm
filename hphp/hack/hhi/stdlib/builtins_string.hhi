<?hh // decl /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */
const int HTML_SPECIALCHARS = 0;
const int HTML_ENTITIES = 1;

const int ENT_COMPAT = 2;
const int ENT_QUOTES = 3;
const int ENT_NOQUOTES = 0;
const int ENT_IGNORE = 4;
const int ENT_SUBSTITUTE = 8;

const int STR_PAD_LEFT = 0;
const int STR_PAD_RIGHT = 1;
const int STR_PAD_BOTH = 2;

const int LC_ALL = 6;
const int LC_COLLATE = 3;
const int LC_CTYPE = 0;
const int LC_MESSAGES = 5;
const int LC_MONETARY = 4;
const int LC_NUMERIC = 1;
const int LC_TIME = 2;

const string HPHP_TRIM_CHARLIST = "\n\r\t\013\000 ";

function addcslashes($str, $charlist);
function stripcslashes($str);
function addslashes($str);
function stripslashes($str);
function bin2hex($str);
function hex2bin($str);
function nl2br($str);
function quotemeta($str);
function str_shuffle($str);
function strrev($str);
function strtolower($str);
function strtoupper($str);
function ucfirst($str);
function lcfirst($str);
function ucwords($str);
function trim($str, $charlist = HPHP_TRIM_CHARLIST)/*: string*/;
function ltrim($str, $charlist = HPHP_TRIM_CHARLIST)/*: string*/;
function rtrim($str, $charlist = HPHP_TRIM_CHARLIST)/*: string*/;
function chop($str, $charlist = HPHP_TRIM_CHARLIST);
<<__Deprecated('Use implode().')>>
function join($glue, $pieces = null);
function str_split($str, $split_length = 1);
function chunk_split($body, $chunklen = 76, $end = "\r\n");
function strtok($str, $token = null);
function str_replace($search, $replace, $subject, &$count = null);
function str_ireplace($search, $replace, $subject, &$count = null);
function substr_replace($str, $replacement, $start, $length = 0x7FFFFFFF);
function substr($str, $start, $length = 0x7FFFFFFF);
function str_pad($input, $pad_length, $pad_string = " ", $pad_type = STR_PAD_RIGHT);
function str_repeat($input, $multiplier);
function wordwrap($str, $width = 75, $wordbreak = "\n", $cut = false);
function html_entity_decode($str, $quote_style = ENT_COMPAT, $charset = "ISO-8859-1");
function htmlentities($str, $quote_style = ENT_COMPAT, $charset = "ISO-8859-1", $double_encode = true);
function htmlspecialchars_decode($str, $quote_style = ENT_COMPAT);
function htmlspecialchars($str, $quote_style = ENT_COMPAT, $charset = "ISO-8859-1", $double_encode = true);
function quoted_printable_encode($str);
function quoted_printable_decode($str);
function convert_uudecode($data);
function convert_uuencode($data);
function str_rot13($str);
function crc32($str);
function crypt($str, $salt = "");
function md5($str, $raw_output = false);
function sha1($str, $raw_output = false);
function strtr($str, $from, $to = null);
function convert_cyr_string($str, $from, $to);
function get_html_translation_table($table = 0, $quote_style = ENT_COMPAT);
function hebrev($hebrew_text, $max_chars_per_line = 0);
function hebrevc($hebrew_text, $max_chars_per_line = 0);
function setlocale($category, $locale, ...);
function localeconv();
function nl_langinfo($item);
function vprintf($format, $args);
function vsprintf($format, $args);
function sscanf($str, $format, ...);
function money_format($format, $number);
function number_format($number, $decimals = 0, $dec_point = ".", $thousands_sep = ",");
function strcmp($str1, $str2);
function strncmp($str1, $str2, $len);
function strnatcmp($str1, $str2);
function strcasecmp($str1, $str2);
function strncasecmp($str1, $str2, $len);
function strnatcasecmp($str1, $str2);
function strcoll($str1, $str2);
function substr_compare($main_str, $str, $offset, $length = PHP_INT_MAX, $case_insensitivity = false);
function strchr($haystack, $needle);
function strrchr($haystack, $needle);
function strstr($haystack, $needle, bool $before_needle = false);
function stristr($haystack, $needle, bool $before_needle = false);
function strpbrk($haystack, $char_list);
function strpos($haystack, $needle, $offset = 0);
function stripos($haystack, $needle, $offset = 0);
function strrpos($haystack, $needle, $offset = 0);
function strripos($haystack, $needle, $offset = 0);
function substr_count($haystack, $needle, $offset = 0, $length = 0x7FFFFFFF);
function strspn($str1, $str2, $start = 0, $length = 0x7FFFFFFF);
function strcspn($str1, $str2, $start = 0, $length = 0x7FFFFFFF);
function strlen($vstr): int;
function count_chars($str, $mode = 0);
function str_word_count($str, $format = 0, $charlist = "");
function levenshtein($str1, $str2, $cost_ins = 1, $cost_rep = 1, $cost_del = 1);
function similar_text($first, $second, &$percent = null);
function soundex($str);
function metaphone($str, $phones = 0);
function parse_str($str, &$arr = null);
