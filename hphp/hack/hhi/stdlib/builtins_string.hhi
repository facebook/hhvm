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

<<__PHPStdLib>>
function addcslashes(string $str, string $charlist)[];
<<__PHPStdLib>>
function stripcslashes(string $str)[];
<<__PHPStdLib>>
function addslashes(string $str)[];
<<__PHPStdLib>>
function stripslashes(string $str)[];
<<__PHPStdLib>>
function bin2hex(string $str)[];
<<__PHPStdLib>>
function hex2bin(string $str)[];
<<__PHPStdLib>>
function nl2br(string $str)[];
<<__PHPStdLib>>
function quotemeta(string $str)[];
<<__PHPStdLib>>
function str_shuffle(string $str);
<<__PHPStdLib>>
function strrev(string $str)[];
<<__PHPStdLib>>
function strtolower(string $str)[];
<<__PHPStdLib>>
function strtoupper(string $str)[];
<<__PHPStdLib>>
function ucfirst(string $str);
<<__PHPStdLib>>
function lcfirst(string $str);
<<__PHPStdLib>>
function ucwords(string $str, string $delimiters = " \t\r\n\f\v");
<<__PHPStdLib>>
function trim(string $str, string $charlist = HPHP_TRIM_CHARLIST)[]/*: string*/;
<<__PHPStdLib>>
function ltrim(string $str, string $charlist = HPHP_TRIM_CHARLIST)[]/*: string*/;
<<__PHPStdLib>>
function rtrim(string $str, string $charlist = HPHP_TRIM_CHARLIST)[]/*: string*/;
<<__PHPStdLib>>
function chop(string $str, string $charlist = HPHP_TRIM_CHARLIST)[];
<<__Deprecated('Use implode().'), __PHPStdLib>>
function join($glue, $pieces = null)[];
<<__PHPStdLib>>
function str_split(string $str, int $split_length = 1)[];
<<__PHPStdLib>>
function chunk_split(string $body, int $chunklen = 76, string $end = "\r\n")[];
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
function str_getcsv(string $input, string $delimiter = ",", string $enclosure = "\"", string $escape = "\\"): varray<?string>;
<<__PHPStdLib>>
function str_replace($search, $replace, $subject)[];
<<__PHPStdLib>>
function str_replace_with_count($search, $replace, $subject, inout $count);
<<__PHPStdLib>> // not pure: uses global locale for capitalization
function str_ireplace($search, $replace, $subject);
<<__PHPStdLib>>
function str_ireplace_with_count($search, $replace, $subject, inout $count);
<<__PHPStdLib>>
function substr_replace($str, $replacement, $start, $length = 0x7FFFFFFF)[];
<<__PHPStdLib>>
function substr(string $str, int $start, int $length = 0x7FFFFFFF)[];
<<__PHPStdLib>>
function str_pad(string $input, int $pad_length, string $pad_string = " ", int $pad_type = STR_PAD_RIGHT)[];
<<__PHPStdLib>>
function str_repeat(string $input, int $multiplier)[];
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
<<__PHPStdLib>>
function quoted_printable_encode(string $str)[];
<<__PHPStdLib>>
function quoted_printable_decode(string $str)[];
<<__PHPStdLib>>
function convert_uudecode(string $data)[];
<<__PHPStdLib>>
function convert_uuencode(string $data)[];
<<__PHPStdLib>>
function str_rot13(string $str)[];
<<__PHPStdLib>>
function crc32(string $str)[];
<<__PHPStdLib>>
function crypt(string $str, string $salt = "");
<<__PHPStdLib>>
function md5(string $str, bool $raw_output = false)[];
<<__PHPStdLib>>
function sha1(string $str, bool $raw_output = false)[];
<<__PHPStdLib>>
function strtr(string $str, $from, $to = null)[];
<<__PHPStdLib>>
function convert_cyr_string(string $str, string $from, string $to)[];
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
<<__PHPStdLib>>
function vsprintf($format, $args)[];
function sscanf(string $str, string $format)[];
<<__PHPStdLib>>
function money_format(string $format, float $number);
<<__PHPStdLib>>
function number_format(float $number, int $decimals = 0, $dec_point = ".", $thousands_sep = ",")[];
<<__PHPStdLib>>
function strcmp(string $str1, string $str2)[];
<<__PHPStdLib>>
function strncmp(string $str1, string $str2, int $len)[];
<<__PHPStdLib>>
function strnatcmp(string $str1, string $str2)[];
<<__PHPStdLib>>
function strcasecmp(string $str1, string $str2)[];
<<__PHPStdLib>>
function strncasecmp(string $str1, string $str2, int $len)[];
<<__PHPStdLib>>
function strnatcasecmp(string $str1, string $str2)[];
<<__PHPStdLib>>
function strcoll(string $str1, string $str2)[];
<<__PHPStdLib>>
function substr_compare(string $main_str, string $str, int $offset, int $length = PHP_INT_MAX, bool $case_insensitivity = false)[];
<<__PHPStdLib>>
function strchr(string $haystack, $needle)[];
<<__PHPStdLib>>
function strrchr(string $haystack, $needle)[];
<<__PHPStdLib>>
function strstr(string $haystack, $needle, bool $before_needle = false)[];
<<__PHPStdLib>>
function stristr(string $haystack, $needle, bool $before_needle = false)[];
<<__PHPStdLib>>
function strpbrk(string $haystack, string $char_list)[];
<<__PHPStdLib>>
function strpos(string $haystack, $needle, int $offset = 0)[];
<<__PHPStdLib>>
function stripos(string $haystack, $needle, int $offset = 0)[];
<<__PHPStdLib>>
function strrpos(string $haystack, $needle, int $offset = 0)[];
<<__PHPStdLib>>
function strripos(string $haystack, $needle, int $offset = 0)[];
<<__PHPStdLib>>
function substr_count(string $haystack, string $needle, int $offset = 0, int $length = 0x7FFFFFFF)[];
<<__PHPStdLib>>
function strspn(string $str1, string $str2, int $start = 0, int $length = 0x7FFFFFFF)[];
<<__PHPStdLib>>
function strcspn(string $str1, string $str2, int $start = 0, int $length = 0x7FFFFFFF)[];
<<__PHPStdLib>>
function strlen(string $vstr)[]: int;
<<__PHPStdLib>>
function count_chars(string $str, int $mode = 0)[];
<<__PHPStdLib>>
function str_word_count(string $str, int $format = 0, string $charlist = "")[];
<<__PHPStdLib>>
function levenshtein(string $str1, string $str2, int $cost_ins = 1, int $cost_rep = 1, int $cost_del = 1)[];
<<__PHPStdLib>>
function similar_text(string $first, string $second, inout $percent);
<<__PHPStdLib>>
function soundex(string $str)[];
<<__PHPStdLib>>
function metaphone(string $str, int $phones = 0)[];
function parse_str(string $str, inout $arr);
