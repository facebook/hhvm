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

use namespace HH\Lib\{_Private, C, Keyset, Locale, Vec, _Private\_Str};

/**
 * Returns the string with the first character capitalized.
 *
 * If the first character is already capitalized or isn't alphabetic, the string
 * will be unchanged.
 *
 * Locale-specific capitalization rules will be respected, e.g. `i` -> `I` vs
 * `i` -> `İ`.
 *
 * - To capitalize all characters, see `Str\uppercase_l()`.
 * - To capitalize all words, see `Str\capitalize_words_l()`.
 */
function capitalize_l(
  Locale\Locale $locale,
  string $string,
)[]: string {
  if ($string === '') {
    return '';
  }
  return uppercase_l($locale, slice_l($locale, $string, 0, 1)).slice_l($locale, $string, 1);
}

/**
 * Returns the string with all words capitalized.
 *
 * Locale-specific capitalization rules will be respected, e.g. `i` -> `I` vs
 * `i` -> `İ`.
 *
 * Delimiters are defined by the locale.
 *
 * - To capitalize all characters, see `Str\uppercase_l()`.
 * - To capitalize only the first character, see `Str\capitalize_l()`.
 */
function capitalize_words_l(
  Locale\Locale $locale,
  string $string,
)[]: string {
  return _Str\titlecase_l($string, $locale);
}

/**
 * Returns the string with all alphabetic characters converted to lowercase.
 *
 * Locale-specific capitalization rules will be respected, e.g. `I` -> `i` vs
 * `I` -> `ı`
 */
function lowercase_l(
  Locale\Locale $locale,
  string $string,
)[]: string {
  return _Str\lowercase_l($string, $locale);
}

/**
 * Returns the string padded to the total length (in characters) by appending
 * the `$pad_string` to the left.
 *
 * If the length of the input string plus the pad string exceeds the total
 * length, the pad string will be truncated. If the total length is less than or
 * equal to the length of the input string, no padding will occur.
 *
 * To pad the string on the right, see `Str\pad_right_l()`.
 * To pad the string to a fixed number of bytes, see `Str\pad_left()`.
 */
function pad_left_l(
  Locale\Locale $locale,
  string $string,
  int $total_length,
  string $pad_string = ' ',
)[]: string {
  return _Str\pad_left_l($string, $total_length, $pad_string, $locale);
}

/**
 * Returns the string padded to the total length (in characters) by appending
 * the `$pad_string` to the right.
 *
 * If the length of the input string plus the pad string exceeds the total
 * length, the pad string will be truncated. If the total length is less than or
 * equal to the length of the input string, no padding will occur.
 *
 * To pad the string on the left, see `Str\pad_left()`.
 * To pad the string to a fixed number of bytes, see `Str\pad_right()`
 */
function pad_right_l(
  Locale\Locale $locale,
  string $string,
  int $total_length,
  string $pad_string = ' ',
)[]: string {
  return _Str\pad_right_l($string, $total_length, $pad_string, $locale);
}

/**
 * Returns the "haystack" string with all occurrences of `$needle` replaced by
 * `$replacement`.
 *
 * Strings will be normalized for comparison in encodings that support multiple
 * representations, such as UTF-8.
 *
 * - For a case-insensitive search/replace, see `Str\replace_ci_l()`.
 * - For multiple case-sensitive searches/replacements, see `Str\replace_every_l()`.
 * - For multiple case-insensitive searches/replacements, see `Str\replace_every_ci_l()`.
 */
function replace_l(
  Locale\Locale $locale,
  string $haystack,
  string $needle,
  string $replacement,
)[]: string {
  return _Str\replace_l($haystack, $needle, $replacement, $locale);
}

/**
 * Returns the "haystack" string with all occurrences of `$needle` replaced by
 * `$replacement` (case-insensitive).
 *
 * Locale-specific rules for case-insensitive comparisons will be used, and
 * strings will be normalized before comparing if the locale specifies an
 * encoding that supports multiple representations of the same characters, such
 * as UTF-8.
 *
 * - For a case-sensitive search/replace, see `Str\replace_l()`.
 * - For multiple case-sensitive searches/replacements, see `Str\replace_every_l()`.
 * - For multiple case-insensitive searches/replacements, see `Str\replace_every_ci_l()`.
 */
function replace_ci_l(
  Locale\Locale $locale,
  string $haystack,
  string $needle,
  string $replacement,
)[rx]: string {
  return _Str\replace_ci_l($haystack, $needle, $replacement, $locale);
}

/**
 * Returns the "haystack" string with all occurrences of the keys of
 * `$replacements` replaced by the corresponding values.
 *
 * Strings will be normalized for comparison in encodings that support multiple
 * representations, such as UTF-8.
 *
 * Replacements are applied in the order they are specified in `$replacements`,
 * and the new values are searched again for subsequent matches. For example,
 * `dict['a' => 'b', 'b' => 'c']` is equivalent to `dict['a' => 'c']`, but
 * `dict['b' => 'c', 'a' => 'b']` is not, despite having the same elements.
 *
 * If there are multiple overlapping matches, the match occuring earlier in
 * `$replacements` (not in `$haystack`) takes precedence.
 *
 * - For a single case-sensitive search/replace, see `Str\replace_l()`.
 * - For a single case-insensitive search/replace, see `Str\replace_ci_l()`.
 * - For multiple case-insensitive searches/replacements, see `Str\replace_every_ci_l()`.
 * - For not having new values searched again, see `Str\replace_every_nonrecursive_l()`.
 */
function replace_every_l(
  Locale\Locale $locale,
  string $haystack,
  KeyedContainer<string, string> $replacements,
)[]: string {
  return _Str\replace_every_l($haystack, dict($replacements), $locale);
}

/**
 * Returns the "haystack" string with all occurrences of the keys of
 * `$replacements` replaced by the corresponding values (case-insensitive).
 *
 * Locale-specific rules for case-insensitive comparisons will be used, and
 * strings will be normalized before comparing if the locale specifies an
 * encoding that supports multiple representations of the same characters, such
 * as UTF-8.
 *
 * Replacements are applied in the order they are specified in `$replacements`,
 * and the new values are searched again for subsequent matches. For example,
 * `dict['a' => 'b', 'b' => 'c']` is equivalent to `dict['a' => 'c']`, but
 * `dict['b' => 'c', 'a' => 'b']` is not, despite having the same elements.
 *
 * If there are multiple overlapping matches, the match occuring earlier in
 * `$replacements` (not in `$haystack`) takes precedence.
 *
 * - For a single case-sensitive search/replace, see `Str\replace_l()`.
 * - For a single case-insensitive search/replace, see `Str\replace_ci_l()`.
 * - For multiple case-sensitive searches/replacements, see `Str\replace_every_l()`.
 * - For not having new values searched again, see `Str\replace_every_nonrecursive_ci_l()`.
 */
function replace_every_ci_l(
  Locale\Locale $locale,
  string $haystack,
  KeyedContainer<string, string> $replacements,
)[rx]: string {
  return _Str\replace_every_ci_l($haystack, dict($replacements), $locale);
}

/**
 * Returns the "haystack" string with all occurrences of the keys of
 * `$replacements` replaced by the corresponding values. Once a substring has
 * been replaced, its new value will not be searched again.
 *
 * Strings will be normalized for comparison in encodings that support multiple
 * representations, such as UTF-8.
 *
 * If there are multiple overlapping matches, the match occuring earlier in
 * `$haystack` takes precedence. If a replacer is a prefix of another (like
 * "car" and "carpet"), the longer one (carpet) takes precedence. The ordering
 * of `$replacements` therefore doesn't matter.
 *
 * - For having new values searched again, see `Str\replace_every_l()`.
 */
function replace_every_nonrecursive_l(
  Locale\Locale $locale,
  string $haystack,
  KeyedContainer<string, string> $replacements,
)[rx]: string {
  return _Str\replace_every_nonrecursive_l($haystack, dict($replacements), $locale);
}

/**
 * Returns the "haystack" string with all occurrences of the keys of
 * `$replacements` replaced by the corresponding values (case-insensitive).
 * Once a substring has been replaced, its new value will not be searched
 * again.
 *
 * Locale-specific rules for case-insensitive comparisons will be used, and
 * strings will be normalized before comparing if the locale specifies an
 * encoding that supports multiple representations of the same characters, such
 * as UTF-8.
 *
 * If there are multiple overlapping matches, the match occuring earlier in
 * `$haystack` takes precedence. If a replacer is a case-insensitive prefix of
 * another (like "Car" and "CARPET"), the longer one (carpet) takes precedence.
 * The ordering of `$replacements` therefore doesn't matter.
 *
 * When two replacers are passed that are identical except for case,
 * an InvalidArgumentException is thrown.
 *
 * Time complexity: O(a + length * b), where a is the sum of all key lengths and
 * b is the sum of distinct key lengths (length is the length of `$haystack`)
 *
 * - For having new values searched again, see `Str\replace_every_ci_l()`.
 */
function replace_every_nonrecursive_ci_l(
  Locale\Locale $locale,
  string $haystack,
  KeyedContainer<string, string> $replacements,
)[rx]: string {
  return _Str\replace_every_nonrecursive_ci_l($haystack, dict($replacements), $locale);
}

/** Reverse a string by characters.
 *
 * @see `Str\reverse()` to reverse by bytes instead.
 */
function reverse_l(Locale\Locale $locale, string $string)[]: string {
  return _Str\reverse_l($string, $locale);
}

/**
 * Return the string with a slice specified by the offset/length replaced by the
 * given replacement string.
 *
 * If the length is omitted or exceeds the upper bound of the string, the
 * remainder of the string will be replaced. If the length is zero, the
 * replacement will be inserted at the offset.
 *
 *
 * Offset can be positive or negative. When positive, replacement starts from the
 * beginning of the string; when negative, replacement starts from the end of the string.
 *
 * Some examples:
 * - `Str\splice_l($l, "apple", "orange", 0)` without `$length`, `$string` is replaced, resolving to `"orange"`
 * - `Str\splice_l($l, "apple", "orange", 3)` inserting at `$offset` `3` from the start of `$string` resolves to `"apporange"`
 * - `Str\splice_l($l, "apple", "orange", -2)` inserting at `$offset` `-2` from the end of `$string` resolves to `"apporange"`
 * - `Str\splice_l($l, "apple", "orange", 0, 0)` with `$length` `0`, `$replacement` is appended at `$offset` `0` and resolves to `"orangeapple"`
 * - `Str\splice_l($l, "apple", "orange", 5, 0)` with `$length` `0`, `$replacement` is appended at `$offset` `5` and resolves to `"appleorange"`
 *
 */
function splice_l(
  Locale\Locale $locale,
  string $string,
  string $replacement,
  int $offset,
  ?int $length = null,
)[]: string {
  return _Str\splice_l($string, $replacement, $offset, $length, $locale);
}

/**
 * Returns the string with all alphabetic characters converted to uppercase.
 *
 * Locale-specific capitalization rules will be respected, e.g. `i` -> `I` vs
 * `i` -> `İ`.
 */
function uppercase_l(
  Locale\Locale $locale,
  string $string,
)[]: string {
  return _Str\uppercase_l($string, $locale);
}
