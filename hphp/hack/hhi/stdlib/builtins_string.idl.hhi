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
function addcslashes(string $str, string $charlist): string { }
function stripcslashes(string $str): string { }
function addslashes(string $str): string { }
function stripslashes(string $str): string { }
function bin2hex(string $str): string { }
function hex2bin(string $str): string { }
function nl2br(string $str): string { }
function quotemeta(string $str) { }
function str_shuffle(string $str): string { }
function strrev(string $str): string { }
function strtolower(string $str): string { }
function strtoupper(string $str): string { }
function ucfirst(string $str): string { }
function lcfirst(string $str): string { }
function ucwords(string $str): string { }
function trim(string $str, string $charlist = HPHP_TRIM_CHARLIST): string { }
function ltrim(string $str, string $charlist = HPHP_TRIM_CHARLIST): string { }
function rtrim(string $str, string $charlist = HPHP_TRIM_CHARLIST): string { }
function chop(string $str, string $charlist = HPHP_TRIM_CHARLIST): string { }
function join($glue, $pieces = null_variant) { }
function str_split(string $str, int $split_length = 1) { }
function chunk_split(string $body, int $chunklen = 76, string $end = "\r\n"): string { }
function strtok(string $str, string $token = null_variant) { }
function str_replace($search, $replace, $subject, &$count = null) { }
function str_ireplace($search, $replace, $subject, &$count = null) { }
function substr_replace($str, $replacement, $start, $length = 0x7FFFFFFF) { }
function substr(string $str, int $start, int $length = 0x7FFFFFFF) { }
function str_pad(string $input, int $pad_length, string $pad_string = " ", int $pad_type = STR_PAD_RIGHT): string { }
function str_repeat(string $input, int $multiplier): string { }
function wordwrap(string $str, int $width = 75, string $wordbreak = "\n", bool $cut = false): string { }
function html_entity_decode(string $str, int $quote_style = ENT_COMPAT, string $charset = "ISO-8859-1"): string { }
function htmlentities(string $str, int $quote_style = ENT_COMPAT, string $charset = "ISO-8859-1", bool $double_encode = true): string { }
function htmlspecialchars_decode(string $str, int $quote_style = ENT_COMPAT): string { }
function htmlspecialchars(string $str, int $quote_style = ENT_COMPAT, string $charset = "ISO-8859-1", bool $double_encode = true): string { }
function fb_htmlspecialchars(string $str, int $quote_style = ENT_COMPAT, string $charset = "ISO-8859-1", $extra = array()): string { }
function quoted_printable_encode(string $str): string { }
function quoted_printable_decode(string $str): string { }
function convert_uudecode(string $data) { }
function convert_uuencode(string $data) { }
function str_rot13(string $str): string { }
function crc32(string $str): int { }
function crypt(string $str, string $salt = ""): string { }
function md5(string $str, bool $raw_output = false): string { }
function sha1(string $str, bool $raw_output = false): string { }
function strtr(string $str, $from, string $to = null_variant) { }
function convert_cyr_string(string $str, string $from, string $to): string { }
function get_html_translation_table(int $table = 0, int $quote_style = ENT_COMPAT) { }
function hebrev(string $hebrew_text, int $max_chars_per_line = 0): string { }
function hebrevc(string $hebrew_text, int $max_chars_per_line = 0): string { }
function setlocale(int $category, $locale, ...) { }
function localeconv() { }
function nl_langinfo($item) { }
function vprintf(string $format, array $args): string { }
function vsprintf(string $format, array $args): string { }
function sscanf(string $str, string $format, ...) { }
function money_format(string $format, $number): ?string { }
function number_format($number, int $decimals = 0, string $dec_point = ".", string $thousands_sep = ","): string { }
function strcmp(string $str1, string $str2): int { }
function strncmp(string $str1, string $str2, int $len): int { }
function strnatcmp(string $str1, string $str2) { }
function strcasecmp(string $str1, string $str2) { }
function strncasecmp(string $str1, string $str2, int $len) { }
function strnatcasecmp(string $str1, string $str2) { }
function strcoll(string $str1, string $str2) { }
function substr_compare(string $main_str, string $str, int $offset, int $length = PHP_INT_MAX, bool $case_insensitivity = false) { }
function strchr(string $haystack, $needle) { }
function strrchr(string $haystack, $needle) { }
function strstr(string $haystack, $needle, bool $before_needle = false) { }
function stristr(string $haystack, $needle) { }
function strpbrk(string $haystack, string $char_list) { }
function strpos(string $haystack, $needle, int $offset = 0) { }
function stripos(string $haystack, $needle, int $offset = 0) { }
function strrpos(string $haystack, $needle, int $offset = 0) { }
function strripos(string $haystack, $needle, int $offset = 0) { }
function substr_count(string $haystack, $needle, int $offset = 0, int $length = 0x7FFFFFFF): int { }
function strspn(string $str1, string $str2, int $start = 0, int $length = 0x7FFFFFFF): int { }
function strcspn(string $str1, string $str2, int $start = 0, int $length = 0x7FFFFFFF): int { }
function strlen(string $vstr): int { }
function count_chars(string $str, int $mode = 0) { }
function str_word_count(string $str, int $format = 0, string $charlist = "") { }
function levenshtein(string $str1, string $str2, int $cost_ins = 1, int $cost_rep = 1, int $cost_del = 1): int { }
function similar_text(string $first, string $second, &$percent = null): int { }
function soundex(string $str): string { }
function metaphone(string $str, int $phones = 0) { }
function parse_str(string $str, &$arr = null) { }
