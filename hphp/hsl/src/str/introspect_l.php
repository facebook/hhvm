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
 * Returns `< 0` if `$string1` is less than `$string2`, `> 0` if `$string1` is
 * greater than `$string2`, and `0` if they are equal.
 *
 * For a case-insensitive comparison, see `Str\compare_ci_l()`.
 *
 * Locale-specific collation rules will be followed, and strings will be
 * normalized in encodings that support multiple representations of the same
 * characters, such as UTF8.
 *
 * @guide /hack/built-in-types/string
 */
function compare_l(
  Locale\Locale $locale,
  string $string1,
  string $string2,
)[]: int {
  return _Str\strcoll_l($string1, $string2, $locale);
}

/**
 * Returns `< 0` if `$string1` is less than `$string2`, `> 0` if `$string1` is
 * greater than `$string2`, and `0` if they are equal (case-insensitive).
 *
 * For a case-sensitive comparison, see `Str\compare_l()`.
 *
 * Locale-specific collation and case-sensitivity rules will be used. For
 * example, case-insensitive comparisons between `i`, `I`, `ı`, and `İ` vary
 * by locale.
 *
 * @guide /hack/built-in-types/string
 */
function compare_ci_l(
  Locale\Locale $locale,
  string $string1,
  string $string2,
)[]: int {
  return _Str\strcasecmp_l($string1, $string2, $locale);
}

/**
 * Returns whether the "haystack" string contains the "needle" string.
 *
 * An optional offset determines where in the haystack the search begins. If the
 * offset is negative, the search will begin that many characters from the end
 * of the string. If the offset is out-of-bounds, a InvalidArgumentException will be
 * thrown.
 *
 * Strings will be normalized for comparison in encodings that support multiple
 * representations, such as UTF-8.
 *
 * - To get the position of the needle, see `Str\search_l()`.
 * - To search for the needle case-insensitively, see `Str\contains_ci_l()`.
 *
 * @guide /hack/built-in-types/string
 */
function contains_l(
  Locale\Locale $locale,
  string $haystack,
  string $needle,
  int $offset = 0,
)[]: bool {
  if ($needle === '') {
    if ($offset === 0) {
      return true;
    }
    $length = length_l($locale, $haystack);
    if ($offset > $length || $offset < -$length) {
      throw new \InvalidArgumentException(
        format('Offset %d out of bounds for length %d', $offset, $length)
      );
    }
    return true;
  }
  return search_l($locale, $haystack, $needle, $offset) !== null;
}

/**
 * Returns whether the "haystack" string contains the "needle" string
 * (case-insensitive).
 *
 * An optional offset determines where in the haystack the search begins. If the
 * offset is negative, the search will begin that many characters from the end
 * of the string. If the offset is out-of-bounds, a InvalidArgumentException will be
 * thrown.
 *
 * Locale-specific rules for case-insensitive comparisons will be used, and
 * strings will be normalized before comparing if the locale specifies an
 * encoding that supports multiple representations of the same characters, such
 * as UTF-8.
 *
 * - To search for the needle case-sensitively, see `Str\contains_l()`.
 * - To get the position of the needle case-insensitively, see `Str\search_ci_l()`.
 *
 * @guide /hack/built-in-types/string
 */
function contains_ci_l(
  Locale\Locale $locale,
  string $haystack,
  string $needle,
  int $offset = 0,
)[]: bool {
  if ($needle === '') {
    if ($offset === 0) {
      return true;
    }
    $length = length_l($locale, $haystack);
    if ($offset > $length || $offset < -$length) {
      throw new \InvalidArgumentException(
        format('Offset %d out of bounds for length %d', $offset, $length)
      );
    }
    return true;
  }
  return search_ci_l($locale, $haystack, $needle, $offset) !== null;
}

/**
 * Returns whether the string ends with the given suffix.
 *
 * Strings will be normalized before comparing if the locale specifies an
 * encoding that supports multiple representations of the same characters, such
 * as UTF-8.
 *
 * For a case-insensitive check, see `Str\ends_with_ci_l()`.
 *
 * @guide /hack/built-in-types/string
 */
function ends_with_l(
  Locale\Locale $locale,
  string $string,
  string $suffix,
)[]: bool {
  return _Str\ends_with_l($string, $suffix, $locale);
}

/**
 * Returns whether the string ends with the given suffix (case-insensitive).
 *
 * Locale-specific rules for case-insensitive comparisons will be used, and
 * strings will be normalized before comparing if the locale specifies an
 * encoding that supports multiple representations of the same characters, such
 * as UTF-8.
 *
 * For a case-sensitive check, see `Str\ends_with_l()`.
 *
 * @guide /hack/built-in-types/string
 */
function ends_with_ci_l(
  Locale\Locale $locale,
  string $string,
  string $suffix,
)[]: bool {
  return _Str\ends_with_ci_l($string, $suffix, $locale);
}

/**
 * Returns the length of the given string in characters.
 *
 * This function may be `O(1)` or `O(n)` depending on the encoding specified
 * by the locale (LC_CTYPE).
 *
 * See `Str\length()` (or pass `Locale\c()`) for the length in bytes.
 *
 * @guide /hack/built-in-types/string
 */
function length_l(
  Locale\Locale $locale,
  string $string,
)[]: int {
  return _Str\strlen_l($string, $locale);
}

/**
 * Returns the first position of the "needle" string in the "haystack" string,
 * or null if it isn't found.
 *
 * An optional offset determines where in the haystack the search begins. If the
 * offset is negative, the search will begin that many characters from the end
 * of the string. If the offset is out-of-bounds, a InvalidArgumentException will be
 * thrown.
 *
 * - To simply check if the haystack contains the needle, see `Str\contains_l()`.
 * - To get the case-insensitive position, see `Str\search_ci_l()`.
 * - To get the last position of the needle, see `Str\search_last_l()`.
 *
 * @guide /hack/built-in-types/string
 */
function search_l(
  Locale\Locale $locale,
  string $haystack,
  string $needle,
  int $offset = 0,
)[]: ?int {
  $position = _Str\strpos_l($haystack, $needle, $offset, $locale);
  if ($position < 0) {
    return null;
  }
  return $position;
}

/**
 * Returns the first position of the "needle" string in the "haystack" string,
 * or null if it isn't found (case-insensitive).
 *
 * Locale-specific rules for case-insensitive comparisons will be used.
 *
 * An optional offset determines where in the haystack the search begins. If the
 * offset is negative, the search will begin that many characters from the end
 * of the string. If the offset is out-of-bounds, a InvalidArgumentException will be
 * thrown.
 *
 * - To simply check if the haystack contains the needle, see `Str\contains()`.
 * - To get the case-sensitive position, see `Str\search()`.
 * - To get the last position of the needle, see `Str\search_last()`.
 *
 * @guide /hack/built-in-types/string
 */
function search_ci_l(
  Locale\Locale $locale,
  string $haystack,
  string $needle,
  int $offset = 0,
)[]: ?int {
  $position = _Str\stripos_l($haystack, $needle, $offset, $locale);
  if ($position < 0) {
    return null;
  }
  return $position;
}

/**
 * Returns the last position of the "needle" string in the "haystack" string,
 * or null if it isn't found.
 *
 * An optional offset determines where in the haystack (from the beginning) the
 * search begins. If the offset is negative, the search will begin that many
 * characters from the end of the string and go backwards. If the offset is
 * out-of-bounds, a InvalidArgumentException will be thrown.
 *
 * - To simply check if the haystack contains the needle, see `Str\contains()`.
 * - To get the first position of the needle, see `Str\search()`.
 *
 * Previously known in PHP as `strrpos`.
 *
 * @guide /hack/built-in-types/string
 */
function search_last_l(
  Locale\Locale $locale,
  string $haystack,
  string $needle,
  int $offset = 0,
)[]: ?int {
  $haystack_length = length_l($locale, $haystack);
  $position = _Str\strrpos_l($haystack, $needle, $offset, $locale);
  if ($position < 0) {
    return null;
  }
  return $position;
}

/**
 * Returns whether the string starts with the given prefix.
 *
 * Strings will be normalized for comparison in encodings that support multiple
 * representations, such as UTF-8.
 *
 * For a case-insensitive check, see `Str\starts_with_ci_l()`.
 * For a byte-wise check, see `Str\starts_with()`
 *
 * @guide /hack/built-in-types/string
 */
function starts_with_l(
  Locale\Locale $locale,
  string $string,
  string $prefix,
)[]: bool {
  return _Str\starts_with_l($string, $prefix, $locale);
}

/**
 * Returns whether the string starts with the given prefix (case-insensitive).
 *
 * Locale-specific collation rules will be followed, and strings will be
 * normalized in encodings that support multiple representations of the same
 * characters, such as UTF8.
 *
 * For a case-sensitive check, see `Str\starts_with()`.
 *
 * @guide /hack/built-in-types/string
 */
function starts_with_ci_l(
  Locale\Locale $locale,
  string $string,
  string $prefix,
)[]: bool {
  return _Str\starts_with_ci_l($string, $prefix, $locale);
}
