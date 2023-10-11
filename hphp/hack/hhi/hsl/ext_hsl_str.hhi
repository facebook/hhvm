<?hh

/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
 */

namespace HH\Lib\_Private\_Str {

  use type HH\Lib\_Private\_Locale\Locale;

  function strlen_l(string $str, ?Locale $loc = null)[]: int;
  function uppercase_l(string $str, ?Locale $loc = null)[]: string;
  function lowercase_l(string $str, ?Locale $loc = null)[]: string;
  function titlecase_l(string $str, ?Locale $loc = null)[]: string;

  function foldcase_l(string $str, ?Locale $loc = null)[]: string;

  function strcoll_l(string $a, string $b, ?Locale $loc = null)[]: int;
  function strcasecmp_l(string $a, string $b, ?Locale $loc = null)[]: int;
  function starts_with_l(
    string $str,
    string $prefix,
    ?Locale $loc = null,
  )[]: bool;
  function starts_with_ci_l(
    string $str,
    string $prefix,
    ?Locale $loc = null,
  )[]: bool;
  function ends_with_l(
    string $str,
    string $suffix,
    ?Locale $loc = null,
  )[]: bool;
  function ends_with_ci_l(
    string $str,
    string $suffix,
    ?Locale $loc = null,
  )[]: bool;

  function strip_prefix_l(
    string $str,
    string $prefix,
    ?Locale $loc = null,
  )[]: string;
  function strip_suffix_l(
    string $str,
    string $prefix,
    ?Locale $loc = null,
  )[]: string;

  function strpos_l(
    string $haystack,
    string $needle,
    int $offset,
    ?Locale $loc = null,
  )[]: int;
  function strrpos_l(
    string $haystack,
    string $needle,
    int $offset,
    ?Locale $loc = null,
  )[]: int;
  function stripos_l(
    string $haystack,
    string $needle,
    int $offset,
    ?Locale $loc = null,
  )[]: int;
  function strripos_l(
    string $haystack,
    string $needle,
    int $offset,
    ?Locale $loc = null,
  )[]: int;

  function chunk_l(string $str, int $size, ?Locale $loc = null)[]: vec<string>;
  function slice_l(
    string $str,
    int $offset,
    int $length,
    ?Locale $loc = null,
  )[]: string;
  function splice_l(
    string $str,
    string $replacement,
    int $offset,
    ?int $length = null,
    ?Locale $loc = null,
  )[]: string;
  function split_l(
    string $string,
    string $delimiter,
    ?int $limit = null,
    ?Locale $loc = null,
  )[]: vec<string>;

  function reverse_l(string $str, ?Locale $loc = null)[]: string;

  function vsprintf_l(?Locale $loc, string $fmt, vec<mixed> $args)[]: string;

  function pad_left_l(
    string $str,
    int $length,
    string $pad_str = ' ',
    ?Locale $loc = null,
  )[]: string;
  function pad_right_l(
    string $str,
    int $length,
    string $pad_str = ' ',
    ?Locale $loc = null,
  )[]: string;

  function trim_l(string $str, ?string $what, ?Locale $loc = null)[]: string;
  function trim_left_l(
    string $str,
    ?string $what,
    ?Locale $loc = null,
  )[]: string;
  function trim_right_l(
    string $str,
    ?string $what,
    ?Locale $loc = null,
  )[]: string;

  function replace_l(
    string $haystack,
    string $needle,
    string $replacement,
    ?Locale $loc = null,
  )[]: string;
  function replace_ci_l(
    string $haystack,
    string $needle,
    string $replacement,
    ?Locale $loc = null,
  )[]: string;
  function replace_every_l(
    string $haystack,
    dict<string, string> $replacements,
    ?Locale $loc = null,
  )[]: string;
  function replace_every_ci_l(
    string $haystack,
    dict<string, string> $replacements,
    ?Locale $loc = null,
  )[]: string;
  function replace_every_nonrecursive_l(
    string $haystack,
    dict<string, string> $replacements,
    ?Locale $loc = null,
  )[]: string;
  function replace_every_nonrecursive_ci_l(
    string $haystack,
    dict<string, string> $replacements,
    ?Locale $loc = null,
  )[]: string;

} // namespace HH\Lib\_Private\_Str
