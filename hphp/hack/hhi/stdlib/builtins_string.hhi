<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
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

<<__PHPStdLib, __Rx>>
function addcslashes(string $str, string $charlist);
<<__PHPStdLib, __Rx>>
function stripcslashes(string $str);
<<__PHPStdLib, __Rx>>
function addslashes(string $str);
<<__PHPStdLib, __Rx>>
function stripslashes(string $str);
<<__PHPStdLib, __Rx>>
function bin2hex(string $str);
<<__PHPStdLib>>
function hex2bin(string $str);
<<__PHPStdLib, __Rx>>
function nl2br(string $str);
<<__PHPStdLib, __Rx>>
function quotemeta(string $str);
<<__PHPStdLib>>
function str_shuffle(string $str);
<<__PHPStdLib, __Rx>>
function strrev(string $str);
<<__PHPStdLib, __Rx>>
function strtolower(string $str);
<<__PHPStdLib, __Rx>>
function strtoupper(string $str);
<<__PHPStdLib>>
function ucfirst(string $str);
<<__PHPStdLib>>
function lcfirst(string $str);
<<__PHPStdLib>>
function ucwords(string $str, string $delimiters = " \t\r\n\f\v");
<<__PHPStdLib, __Rx>>
function trim(string $str, string $charlist = HPHP_TRIM_CHARLIST)/*: string*/;
<<__PHPStdLib, __Rx>>
function ltrim(string $str, string $charlist = HPHP_TRIM_CHARLIST)/*: string*/;
<<__PHPStdLib, __Rx>>
function rtrim(string $str, string $charlist = HPHP_TRIM_CHARLIST)/*: string*/;
<<__PHPStdLib, __Rx>>
function chop(string $str, string $charlist = HPHP_TRIM_CHARLIST);
<<__Deprecated('Use implode().'), __PHPStdLib, __Rx>>
function join($glue, $pieces = null);
<<__PHPStdLib, __Rx>>
function str_split(string $str, int $split_length = 1);
<<__PHPStdLib, __Rx>>
function chunk_split(string $body, int $chunklen = 76, string $end = "\r\n");
<<__PHPStdLib>>
function strtok(string $str, $token = null);
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
function str_getcsv(string $input, string $delimiter = ",", string $enclosure = "\"", string $escape = "\\"): array<?string>;
<<__PHPStdLib>>
function str_replace($search, $replace, $subject);
<<__PHPStdLib>>
function str_replace_with_count($search, $replace, $subject, inout $count);
<<__PHPStdLib>>
function str_ireplace($search, $replace, $subject);
<<__PHPStdLib>>
function str_ireplace_with_count($search, $replace, $subject, inout $count);
<<__PHPStdLib, __Rx>>
function substr_replace($str, $replacement, $start, $length = 0x7FFFFFFF);
<<__PHPStdLib, __Rx>>
function substr(string $str, int $start, int $length = 0x7FFFFFFF);
<<__PHPStdLib, __Rx>>
function str_pad(string $input, int $pad_length, string $pad_string = " ", int $pad_type = STR_PAD_RIGHT);
<<__PHPStdLib, __Rx>>
function str_repeat(string $input, int $multiplier);
<<__PHPStdLib>>
function wordwrap(string $str, int $width = 75, string $wordbreak = "\n", bool $cut = false);
<<__PHPStdLib>>
function html_entity_decode(string $str, int $quote_style = ENT_COMPAT, string $charset = "ISO-8859-1"): string;
<<__PHPStdLib>>
function htmlentities(string $str, int $quote_style = ENT_COMPAT, string $charset = "ISO-8859-1", bool $double_encode = true): string;
<<__PHPStdLib>>
function htmlspecialchars_decode(string $str, int $quote_style = ENT_COMPAT): string;
<<__PHPStdLib>>
function htmlspecialchars(string $str, int $quote_style = ENT_COMPAT, string $charset = "ISO-8859-1", bool $double_encode = true): string;
<<__PHPStdLib, __Rx>>
function quoted_printable_encode(string $str);
<<__PHPStdLib, __Rx>>
function quoted_printable_decode(string $str);
<<__PHPStdLib, __Rx>>
function convert_uudecode(string $data);
<<__PHPStdLib, __Rx>>
function convert_uuencode(string $data);
<<__PHPStdLib, __Rx>>
function str_rot13(string $str);
<<__PHPStdLib, __Rx>>
function crc32(string $str);
<<__PHPStdLib>>
function crypt(string $str, string $salt = "");
<<__PHPStdLib, __Rx>>
function md5(string $str, bool $raw_output = false);
<<__PHPStdLib, __Rx>>
function sha1(string $str, bool $raw_output = false);
<<__PHPStdLib, __Rx>>
function strtr(string $str, $from, $to = null);
<<__PHPStdLib, __Rx>>
function convert_cyr_string(string $str, string $from, string $to);
<<__PHPStdLib>>
function get_html_translation_table(int $table = 0, int $quote_style = ENT_COMPAT);
<<__PHPStdLib>>
function hebrev(string $hebrew_text, int $max_chars_per_line = 0);
<<__PHPStdLib>>
function hebrevc(string $hebrew_text, int $max_chars_per_line = 0);
<<__PHPStdLib>>
function setlocale(int $category, $locale, ...$args);
<<__PHPStdLib>>
function localeconv();
<<__PHPStdLib>>
function nl_langinfo(int $item);
<<__PHPStdLib>>
function vprintf($format, $args);
<<__PHPStdLib, __Rx>>
function vsprintf($format, $args);
<<__Rx>>
function sscanf(string $str, string $format);
<<__PHPStdLib>>
function money_format(string $format, float $number);
<<__PHPStdLib, __Rx>>
function number_format(float $number, int $decimals = 0, $dec_point = ".", $thousands_sep = ",");
<<__PHPStdLib, __Rx>>
function strcmp(string $str1, string $str2);
<<__PHPStdLib, __Rx>>
function strncmp(string $str1, string $str2, int $len);
<<__PHPStdLib, __Rx>>
function strnatcmp(string $str1, string $str2);
<<__PHPStdLib, __Rx>>
function strcasecmp(string $str1, string $str2);
<<__PHPStdLib, __Rx>>
function strncasecmp(string $str1, string $str2, int $len);
<<__PHPStdLib, __Rx>>
function strnatcasecmp(string $str1, string $str2);
<<__PHPStdLib, __Rx>>
function strcoll(string $str1, string $str2);
<<__PHPStdLib, __Rx>>
function substr_compare(string $main_str, string $str, int $offset, int $length = PHP_INT_MAX, bool $case_insensitivity = false);
<<__PHPStdLib, __Rx>>
function strchr(string $haystack, $needle);
<<__PHPStdLib, __Rx>>
function strrchr(string $haystack, $needle);
<<__PHPStdLib, __Rx>>
function strstr(string $haystack, $needle, bool $before_needle = false);
<<__PHPStdLib, __Rx>>
function stristr(string $haystack, $needle, bool $before_needle = false);
<<__PHPStdLib, __Rx>>
function strpbrk(string $haystack, string $char_list);
<<__PHPStdLib, __Rx>>
function strpos(string $haystack, $needle, int $offset = 0);
<<__PHPStdLib, __Rx>>
function stripos(string $haystack, $needle, int $offset = 0);
<<__PHPStdLib, __Rx>>
function strrpos(string $haystack, $needle, int $offset = 0);
<<__PHPStdLib, __Rx>>
function strripos(string $haystack, $needle, int $offset = 0);
<<__PHPStdLib, __Rx>>
function substr_count(string $haystack, string $needle, int $offset = 0, int $length = 0x7FFFFFFF);
<<__PHPStdLib, __Rx>>
function strspn(string $str1, string $str2, int $start = 0, int $length = 0x7FFFFFFF);
<<__PHPStdLib, __Rx>>
function strcspn(string $str1, string $str2, int $start = 0, int $length = 0x7FFFFFFF);
<<__PHPStdLib, __Rx>>
function strlen(string $vstr): int;
<<__PHPStdLib, __Rx>>
function count_chars(string $str, int $mode = 0);
<<__PHPStdLib, __Rx>>
function str_word_count(string $str, int $format = 0, string $charlist = "");
<<__PHPStdLib, __Rx>>
function levenshtein(string $str1, string $str2, int $cost_ins = 1, int $cost_rep = 1, int $cost_del = 1);
<<__PHPStdLib>>
function similar_text(string $first, string $second, inout $percent);
<<__PHPStdLib, __Rx>>
function soundex(string $str);
<<__PHPStdLib, __Rx>>
function metaphone(string $str, int $phones = 0);
function parse_str(string $str, inout $arr);
