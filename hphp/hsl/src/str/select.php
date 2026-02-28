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

use namespace HH\Lib\_Private;
use namespace HH\Lib\_Private\_Str;

/**
 * Returns a substring of length `$length` of the given string starting at the
 * `$offset`.
 *
 * If no length is given, the slice will contain the rest of the
 * string. If the length is zero, the empty string will be returned. If the
 * offset is out-of-bounds, a ViolationException will be thrown.
 *
 * Previously known as `substr` in PHP.
 *
 * @guide /hack/built-in-types/string
 */
function slice(string $string, int $offset, ?int $length = null)[]: string {
  return _Str\slice_l($string, $offset, $length ?? \PHP_INT_MAX);
}

/**
 * Returns the string with one instance of the given prefix removed, or the
 * string itself if it doesn't start with the prefix.
 *
 * @guide /hack/built-in-types/string
 */
function strip_prefix(string $string, string $prefix)[]: string {
  return _Str\strip_prefix_l($string, $prefix);
}

/**
 * Returns the string with the given suffix removed, or the string itself if
 * it doesn't end with the suffix.
 *
 * @guide /hack/built-in-types/string
 */
function strip_suffix(string $string, string $suffix)[]: string {
  return _Str\strip_suffix_l($string, $suffix);
}

/**
 * Returns the given string with whitespace stripped from the beginning and end.
 *
 * If the optional character mask isn't provided, the following characters will
 * be stripped: space, tab, newline, carriage return, NUL byte, vertical tab.
 *
 * - To only strip from the left, see `Str\trim_left()`.
 * - To only strip from the right, see `Str\trim_right()`.
 *
 * @guide /hack/built-in-types/string
 */
function trim(string $string, ?string $char_mask = null)[]: string {
  return _Str\trim_l($string, $char_mask);
}

/**
 * Returns the given string with whitespace stripped from the left.
 * See `Str\trim()` for more details.
 *
 * - To strip from both ends, see `Str\trim()`.
 * - To only strip from the right, see `Str\trim_right()`.
 * - To strip a specific prefix (instead of all characters matching a mask),
 *   see `Str\strip_prefix()`.
 *
 * @guide /hack/built-in-types/string
 */
function trim_left(string $string, ?string $char_mask = null)[]: string {
  return _Str\trim_left_l($string, $char_mask);
}

/**
 * Returns the given string with whitespace stripped from the right.
 * See `Str\trim` for more details.
 *
 * - To strip from both ends, see `Str\trim()`.
 * - To only strip from the left, see `Str\trim_left()`.
 * - To strip a specific suffix (instead of all characters matching a mask),
 *   see `Str\strip_suffix()`.
 *
 * @guide /hack/built-in-types/string
 */
function trim_right(string $string, ?string $char_mask = null)[]: string {
  return _Str\trim_right_l($string, $char_mask);
}
