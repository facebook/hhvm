<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

namespace HH\Lib\Str;

use namespace HH\Lib\{Locale, _Private, _Private\_Str};

/**
 * Returns a substring of length `$length` of the given string starting at the
 * `$offset`.
 *
 * `$offset` and `$length` are specified as a number of characters.
 *
 * If no length is given, the slice will contain the rest of the
 * string. If the length is zero, the empty string will be returned. If the
 * offset is out-of-bounds, an InvalidArgumentException will be thrown.
 *
 * See `slice()` for a byte-based operation.
 *
 * @guide /hack/built-in-types/string
 */
function slice_l(
  Locale\Locale $locale,
  string $string,
  int $offset,
  ?int $length = null,
)[]: string {
  return _Str\slice_l($string, $offset, $length ?? \PHP_INT_MAX, $locale);
}

/**
 * Returns the string with one instance of the given prefix removed, or the
 * string itself if it doesn't start with the prefix.
 *
 * Strings will be normalized for comparison in encodings that support multiple
 * representations, such as UTF-8.
 *
 * @guide /hack/built-in-types/string
 */
function strip_prefix_l(
  Locale\Locale $locale,
  string $string,
  string $prefix,
)[]: string {
  return _Str\strip_prefix_l($string, $prefix, $locale);
}

/**
 * Returns the string with the given suffix removed, or the string itself if
 * it doesn't end with the suffix.
 *
 * Strings will be normalized for comparison in encodings that support multiple
 * representations, such as UTF-8.
 *
 * @guide /hack/built-in-types/string
 */
function strip_suffix_l(
  Locale\Locale $locale,
  string $string,
  string $suffix,
)[]: string {
  return _Str\strip_suffix_l($string, $suffix, $locale);
}

/**
 * Returns the given string with whitespace stripped from the beginning and end.
 *
 * If the optional character mask isn't provided, the characters removed are
 * defined by the locale/encoding.
 *
 * - To only strip from the left, see `Str\trim_left_l()`.
 * - To only strip from the right, see `Str\trim_right_l()`.
 *
 * @guide /hack/built-in-types/string
 */
function trim_l(
  Locale\Locale $locale,
  string $string,
  ?string $char_mask = null,
)[]: string {
  return _Str\trim_l($string, $char_mask, $locale);
}

/**
 * Returns the given string with whitespace stripped from the left.
 * See `Str\trim_l()` for more details.
 *
 * - To strip from both ends, see `Str\trim_l()`.
 * - To only strip from the right, see `Str\trim_right_l()`.
 * - To strip a specific prefix (instead of all characters matching a mask),
 *   see `Str\strip_prefix_l()`.
 *
 * @guide /hack/built-in-types/string
 */
function trim_left_l(
  Locale\Locale $locale,
  string $string,
  ?string $char_mask = null,
)[]: string {
  return _Str\trim_left_l($string, $char_mask, $locale);
}

/**
 * Returns the given string with whitespace stripped from the right.
 * See `Str\trim_l` for more details.
 *
 * - To strip from both ends, see `Str\trim_l()`.
 * - To only strip from the left, see `Str\trim_left_l()`.
 * - To strip a specific suffix (instead of all characters matching a mask),
 *   see `Str\strip_suffix_l()`.
 *
 * @guide /hack/built-in-types/string
 */
function trim_right_l(
  Locale\Locale $locale,
  string $string,
  ?string $char_mask = null,
)[]: string {
  return _Str\trim_right_l($string, $char_mask, $locale);
}
