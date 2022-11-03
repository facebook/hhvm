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
const int ENT_HTML5 = 0;

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
function addcslashes(
  string $str,
  string $charlist,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function stripcslashes(string $str)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function addslashes(string $str)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function stripslashes(string $str)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function bin2hex(string $str)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function hex2bin(string $str)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function nl2br(string $str)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function quotemeta(string $str)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function str_shuffle(string $str): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function strrev(string $str)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function strtolower(string $str)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function strtoupper(string $str)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ucfirst(string $str): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function lcfirst(string $str): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ucwords(
  string $str,
  string $delimiters = " \t\r\n\f\v",
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function trim(
  string $str,
  string $charlist = HPHP_TRIM_CHARLIST,
)[]/*: string*/: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ltrim(
  string $str,
  string $charlist = HPHP_TRIM_CHARLIST,
)[]/*: string*/: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function rtrim(
  string $str,
  string $charlist = HPHP_TRIM_CHARLIST,
)[]/*: string*/: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function chop(
  string $str,
  string $charlist = HPHP_TRIM_CHARLIST,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__Deprecated('Use implode().'), __PHPStdLib>>
function join($glue, $pieces = null)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function str_split(
  string $str,
  int $split_length = 1,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function chunk_split(
  string $body,
  int $chunklen = 76,
  string $end = "\r\n",
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function strtok(string $str, $token = null): HH\FIXME\MISSING_RETURN_TYPE;
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
function str_replace(
  $search,
  $replace,
  $subject,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function str_replace_with_count(
  $search,
  $replace,
  $subject,
  inout $count,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>> // not pure: uses global locale for capitalization
function str_ireplace(
  $search,
  $replace,
  $subject,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function str_ireplace_with_count(
  $search,
  $replace,
  $subject,
  inout $count,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function substr_replace(
  $str,
  $replacement,
  $start,
  $length = 0x7FFFFFFF,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function substr(
  string $str,
  int $start,
  int $length = 0x7FFFFFFF,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function str_pad(
  string $input,
  int $pad_length,
  string $pad_string = " ",
  int $pad_type = STR_PAD_RIGHT,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function str_repeat(
  string $input,
  int $multiplier,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function wordwrap(
  string $str,
  int $width = 75,
  string $wordbreak = "\n",
  bool $cut = false,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function html_entity_decode(string $str, int $quote_style = ENT_COMPAT, string $charset = "ISO-8859-1"): string;
<<__PHPStdLib>>
function htmlentities(string $str, int $quote_style = ENT_COMPAT, string $charset = "ISO-8859-1", bool $double_encode = true): string;
<<__PHPStdLib>>
function htmlspecialchars_decode(string $str, int $quote_style = ENT_COMPAT): string;
<<__PHPStdLib>>
function htmlspecialchars(string $str, int $quote_style = ENT_COMPAT, string $charset = "ISO-8859-1", bool $double_encode = true): string;
<<__PHPStdLib>>
function quoted_printable_encode(string $str)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function quoted_printable_decode(string $str)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function convert_uudecode(string $data)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function convert_uuencode(string $data)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function str_rot13(string $str)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function crc32(string $str)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function crypt(string $str, string $salt = ""): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function md5(
  string $str,
  bool $raw_output = false,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function sha1(
  string $str,
  bool $raw_output = false,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function strtr(string $str, $from, $to = null)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function convert_cyr_string(
  string $str,
  string $from,
  string $to,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function get_html_translation_table(
  int $table = 0,
  int $quote_style = ENT_COMPAT,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function hebrev(
  string $hebrew_text,
  int $max_chars_per_line = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function hebrevc(
  string $hebrew_text,
  int $max_chars_per_line = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function setlocale(
  int $category,
  $locale,
  ...$args
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function localeconv(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function nl_langinfo(int $item): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function vprintf($format, $args): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function vsprintf($format, $args)[]: HH\FIXME\MISSING_RETURN_TYPE;
function sscanf(string $str, string $format)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function money_format(
  string $format,
  float $number,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function number_format(
  float $number,
  int $decimals = 0,
  $dec_point = ".",
  $thousands_sep = ",",
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function strcmp(string $str1, string $str2)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function strncmp(
  string $str1,
  string $str2,
  int $len,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function strnatcmp(string $str1, string $str2)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function strcasecmp(
  string $str1,
  string $str2,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function strncasecmp(
  string $str1,
  string $str2,
  int $len,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function strnatcasecmp(
  string $str1,
  string $str2,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function strcoll(string $str1, string $str2)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function substr_compare(
  string $main_str,
  string $str,
  int $offset,
  int $length = PHP_INT_MAX,
  bool $case_insensitivity = false,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function strchr(string $haystack, $needle)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function strrchr(string $haystack, $needle)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function strstr(
  string $haystack,
  $needle,
  bool $before_needle = false,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function stristr(
  string $haystack,
  $needle,
  bool $before_needle = false,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function strpbrk(
  string $haystack,
  string $char_list,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function strpos(
  string $haystack,
  $needle,
  int $offset = 0,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function stripos(
  string $haystack,
  $needle,
  int $offset = 0,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function strrpos(
  string $haystack,
  $needle,
  int $offset = 0,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function strripos(
  string $haystack,
  $needle,
  int $offset = 0,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function substr_count(
  string $haystack,
  string $needle,
  int $offset = 0,
  int $length = 0x7FFFFFFF,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function strspn(
  string $str1,
  string $str2,
  int $start = 0,
  int $length = 0x7FFFFFFF,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function strcspn(
  string $str1,
  string $str2,
  int $start = 0,
  int $length = 0x7FFFFFFF,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function strlen(string $vstr)[]: int;
<<__PHPStdLib>>
function count_chars(
  string $str,
  int $mode = 0,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function str_word_count(
  string $str,
  int $format = 0,
  string $charlist = "",
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function levenshtein(
  string $str1,
  string $str2,
  int $cost_ins = 1,
  int $cost_rep = 1,
  int $cost_del = 1,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function similar_text(
  string $first,
  string $second,
  inout $percent,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function soundex(string $str)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function metaphone(
  string $str,
  int $phones = 0,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
function parse_str(string $str, inout $arr): HH\FIXME\MISSING_RETURN_TYPE;
