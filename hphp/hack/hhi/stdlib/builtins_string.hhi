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

<<__PHPStdLib>>
function addcslashes($str, $charlist);
<<__PHPStdLib>>
function stripcslashes($str);
<<__PHPStdLib>>
function addslashes($str);
<<__PHPStdLib>>
function stripslashes($str);
<<__PHPStdLib>>
function bin2hex($str);
<<__PHPStdLib>>
function hex2bin($str);
<<__PHPStdLib>>
function nl2br($str);
<<__PHPStdLib>>
function quotemeta($str);
<<__PHPStdLib>>
function str_shuffle($str);
<<__PHPStdLib>>
function strrev($str);
<<__PHPStdLib>>
function strtolower($str);
<<__PHPStdLib>>
function strtoupper($str);
<<__PHPStdLib>>
function ucfirst($str);
<<__PHPStdLib>>
function lcfirst($str);
<<__PHPStdLib>>
function ucwords($str, $delimiters = " \t\r\n\f\v");
<<__PHPStdLib>>
function trim($str, $charlist = HPHP_TRIM_CHARLIST)/*: string*/;
<<__PHPStdLib>>
function ltrim($str, $charlist = HPHP_TRIM_CHARLIST)/*: string*/;
<<__PHPStdLib>>
function rtrim($str, $charlist = HPHP_TRIM_CHARLIST)/*: string*/;
<<__PHPStdLib>>
function chop($str, $charlist = HPHP_TRIM_CHARLIST);
<<__Deprecated('Use implode().'), __PHPStdLib>>
function join($glue, $pieces = null);
<<__PHPStdLib>>
function str_split($str, $split_length = 1);
<<__PHPStdLib>>
function chunk_split($body, $chunklen = 76, $end = "\r\n");
<<__PHPStdLib>>
function strtok($str, $token = null);
/**
 * http://php.net/manual/en/function.str-getcsv.php
 *
 * Parse a CSV string into an array
 *
 * NOTE: return type is an array of nullable strings because:
 *  = str_getcsv('')
 *  Array
 *  (
 *    [0] => null
 *  )
 */
<<__PHPStdLib>>
function str_getcsv(?string $input, string $delimiter = ",", string $enclosure = "\"", string $escape = "\\"): array<?string>;
<<__PHPStdLib>>
function str_replace($search, $replace, $subject, &$count = null);
<<__PHPStdLib>>
function str_ireplace($search, $replace, $subject, &$count = null);
<<__PHPStdLib>>
function substr_replace($str, $replacement, $start, $length = 0x7FFFFFFF);
<<__PHPStdLib>>
function substr($str, $start, $length = 0x7FFFFFFF);
<<__PHPStdLib>>
function str_pad($input, $pad_length, $pad_string = " ", $pad_type = STR_PAD_RIGHT);
<<__PHPStdLib>>
function str_repeat($input, $multiplier);
<<__PHPStdLib>>
function wordwrap($str, $width = 75, $wordbreak = "\n", $cut = false);
<<__PHPStdLib>>
function html_entity_decode($str, $quote_style = ENT_COMPAT, $charset = "ISO-8859-1");
<<__PHPStdLib>>
function htmlentities($str, $quote_style = ENT_COMPAT, $charset = "ISO-8859-1", $double_encode = true);
<<__PHPStdLib>>
function htmlspecialchars_decode($str, $quote_style = ENT_COMPAT);
<<__PHPStdLib>>
function htmlspecialchars($str, $quote_style = ENT_COMPAT, $charset = "ISO-8859-1", $double_encode = true);
<<__PHPStdLib>>
function quoted_printable_encode($str);
<<__PHPStdLib>>
function quoted_printable_decode($str);
<<__PHPStdLib>>
function convert_uudecode($data);
<<__PHPStdLib>>
function convert_uuencode($data);
<<__PHPStdLib>>
function str_rot13($str);
<<__PHPStdLib>>
function crc32($str);
<<__PHPStdLib>>
function crypt($str, $salt = "");
<<__PHPStdLib>>
function md5($str, $raw_output = false);
<<__PHPStdLib>>
function sha1($str, $raw_output = false);
<<__PHPStdLib>>
function strtr($str, $from, $to = null);
<<__PHPStdLib>>
function convert_cyr_string($str, $from, $to);
<<__PHPStdLib>>
function get_html_translation_table($table = 0, $quote_style = ENT_COMPAT);
<<__PHPStdLib>>
function hebrev($hebrew_text, $max_chars_per_line = 0);
<<__PHPStdLib>>
function hebrevc($hebrew_text, $max_chars_per_line = 0);
<<__PHPStdLib>>
function setlocale($category, $locale, ...);
<<__PHPStdLib>>
function localeconv();
<<__PHPStdLib>>
function nl_langinfo($item);
<<__PHPStdLib>>
function vprintf($format, $args);
<<__PHPStdLib>>
function vsprintf($format, $args);
function sscanf($str, $format, ...);
<<__PHPStdLib>>
function money_format($format, $number);
<<__PHPStdLib>>
function number_format($number, $decimals = 0, $dec_point = ".", $thousands_sep = ",");
<<__PHPStdLib>>
function strcmp($str1, $str2);
<<__PHPStdLib>>
function strncmp($str1, $str2, $len);
<<__PHPStdLib>>
function strnatcmp($str1, $str2);
<<__PHPStdLib>>
function strcasecmp($str1, $str2);
<<__PHPStdLib>>
function strncasecmp($str1, $str2, $len);
<<__PHPStdLib>>
function strnatcasecmp($str1, $str2);
<<__PHPStdLib>>
function strcoll($str1, $str2);
<<__PHPStdLib>>
function substr_compare($main_str, $str, $offset, $length = PHP_INT_MAX, $case_insensitivity = false);
<<__PHPStdLib>>
function strchr($haystack, $needle);
<<__PHPStdLib>>
function strrchr($haystack, $needle);
<<__PHPStdLib>>
function strstr($haystack, $needle, bool $before_needle = false);
<<__PHPStdLib>>
function stristr($haystack, $needle, bool $before_needle = false);
<<__PHPStdLib>>
function strpbrk($haystack, $char_list);
<<__PHPStdLib>>
function strpos($haystack, $needle, $offset = 0);
<<__PHPStdLib>>
function stripos($haystack, $needle, $offset = 0);
<<__PHPStdLib>>
function strrpos($haystack, $needle, $offset = 0);
<<__PHPStdLib>>
function strripos($haystack, $needle, $offset = 0);
<<__PHPStdLib>>
function substr_count($haystack, $needle, $offset = 0, $length = 0x7FFFFFFF);
<<__PHPStdLib>>
function strspn($str1, $str2, $start = 0, $length = 0x7FFFFFFF);
<<__PHPStdLib>>
function strcspn($str1, $str2, $start = 0, $length = 0x7FFFFFFF);
<<__PHPStdLib>>
function strlen($vstr): int;
<<__PHPStdLib>>
function count_chars($str, $mode = 0);
<<__PHPStdLib>>
function str_word_count($str, $format = 0, $charlist = "");
<<__PHPStdLib>>
function levenshtein($str1, $str2, $cost_ins = 1, $cost_rep = 1, $cost_del = 1);
<<__PHPStdLib>>
function similar_text($first, $second, &$percent = null);
<<__PHPStdLib>>
function soundex($str);
<<__PHPStdLib>>
function metaphone($str, $phones = 0);
function parse_str($str, &$arr = null);
