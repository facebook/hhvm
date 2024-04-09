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

use namespace HH\Lib\{_Private, C, Keyset, Vec, _Private\_Str};

/**
 * Returns the string with the first character capitalized.
 *
 * If the first character is already capitalized or isn't alphabetic, the string
 * will be unchanged.
 *
 * - To capitalize all characters, see `Str\uppercase()`.
 * - To capitalize all words, see `Str\capitalize_words()`.
 *
 * @guide /hack/built-in-types/string
 */
function capitalize(
  string $string,
)[]: string {
  if ($string === '') {
    return '';
  }
  return _Str\uppercase_l(slice($string, 0, 1)) . slice($string, 1);
}

/**
 * Returns the string with all words capitalized.
 *
 * Words are delimited by space, tab, newline, carriage return, form-feed, and
 * vertical tab by default, but you can specify custom delimiters.
 *
 * - To capitalize all characters, see `Str\uppercase()`.
 * - To capitalize only the first character, see `Str\capitalize()`.
 *
 * @guide /hack/built-in-types/string
 */
function capitalize_words(
  string $string,
  ?string $delimiters = null,
)[]: string {
  if ($string === '') {
    return $string;
  }
  if ($delimiters === null) {
    // Delimiters are defined by the locale
    return _Str\titlecase_l($string, /* locale = */ null);
  }

  $words = vec[];
  $offset = 0;
  $length = \strlen($string);
  while ($offset < $length) {
    $substr_len = \strcspn($string, $delimiters, $offset);
    $words[] = tuple(
      \substr($string, $offset, $substr_len),
      $offset + $substr_len < $length ? $string[$offset + $substr_len] : ''
    );
    $offset += $substr_len + 1;
  }

  $string = '';
  foreach ($words as list($word, $delimiter)) {
    $string .= namespace\capitalize($word).$delimiter;
  }
  return $string;
}

/**
 * Returns a string representation of the given number with grouped thousands.
 *
 * If `$decimals` is provided, the string will contain that many decimal places.
 * The optional `$decimal_point` and `$thousands_separator` arguments define the
 * strings used for decimals and commas, respectively.
 *
 * @guide /hack/built-in-types/string
 */
function format_number(
  num $number,
  int $decimals = 0,
  string $decimal_point = '.',
  string $thousands_separator = ',',
)[]: string {
  return \number_format(
    (float) $number,
    $decimals,
    $decimal_point,
    $thousands_separator,
  );
}

/**
 * Returns the string with all alphabetic characters converted to lowercase.
 *
 * @guide /hack/built-in-types/string
 */
function lowercase(
  string $string,
)[]: string {
  return _Str\lowercase_l($string);
}

/**
 * Returns the string padded to the total length (in bytes) by appending the
 * `$pad_string` to the left.
 *
 * If the length of the input string plus the pad string exceeds the total
 * length, the pad string will be truncated. If the total length is less than or
 * equal to the length of the input string, no padding will occur.
 *
 * To pad the string on the right, see `Str\pad_right()`.
 * To pad the string to a fixed number of characters, see `Str\pad_left_l()`.
 *
 * @guide /hack/built-in-types/string
 */
function pad_left(
  string $string,
  int $total_length,
  string $pad_string = ' ',
)[]: string {
  return _Str\pad_left_l($string, $total_length, $pad_string);
}

/**
 * Returns the string padded to the total length (in bytes) by appending the
 * `$pad_string` to the right.
 *
 * If the length of the input string plus the pad string exceeds the total
 * length, the pad string will be truncated. If the total length is less than or
 * equal to the length of the input string, no padding will occur.
 *
 * To pad the string on the left, see `Str\pad_left()`.
 * To pad the string to a fixed number of characters, see `Str\pad_right_l()`.
 *
 * @guide /hack/built-in-types/string
 */
function pad_right(
  string $string,
  int $total_length,
  string $pad_string = ' ',
)[]: string {
  return _Str\pad_right_l($string, $total_length, $pad_string);
}

/**
 * Returns the input string repeated `$multiplier` times.
 *
 * If the multiplier is 0, the empty string will be returned.
 *
 * @guide /hack/built-in-types/string
 */
function repeat(
  string $string,
  int $multiplier,
)[]: string {
  if ($multiplier < 0) {
    throw new \InvalidArgumentException('Expected non-negative multiplier');
  }
  return \str_repeat($string, $multiplier);
}

/**
 * Returns the "haystack" string with all occurrences of `$needle` replaced by
 * `$replacement`.
 *
 * - For a case-insensitive search/replace, see `Str\replace_ci()`.
 * - For multiple case-sensitive searches/replacements, see `Str\replace_every()`.
 * - For multiple case-insensitive searches/replacements, see `Str\replace_every_ci()`.
 *
 * @guide /hack/built-in-types/string
 */
function replace(
  string $haystack,
  string $needle,
  string $replacement,
)[]: string {
  return _Str\replace_l($haystack, $needle, $replacement);
}

/**
 * Returns the "haystack" string with all occurrences of `$needle` replaced by
 * `$replacement` (case-insensitive).
 *
 * - For a case-sensitive search/replace, see `Str\replace()`.
 * - For multiple case-sensitive searches/replacements, see `Str\replace_every()`.
 * - For multiple case-insensitive searches/replacements, see `Str\replace_every_ci()`.
 *
 * @guide /hack/built-in-types/string
 */
// not pure: str_ireplace uses global locale for capitalization
function replace_ci(
  string $haystack,
  string $needle,
  string $replacement,
): string {
  return _Str\replace_ci_l($haystack, $needle, $replacement);
}

/**
 * Returns the "haystack" string with all occurrences of the keys of
 * `$replacements` replaced by the corresponding values.
 *
 * Replacements are applied in the order they are specified in `$replacements`,
 * and the new values are searched again for subsequent matches. For example,
 * `dict['a' => 'b', 'b' => 'c']` is equivalent to `dict['a' => 'c']`, but
 * `dict['b' => 'c', 'a' => 'b']` is not, despite having the same elements.
 *
 * If there are multiple overlapping matches, the match occuring earlier in
 * `$replacements` (not in `$haystack`) takes precedence.
 *
 * - For a single case-sensitive search/replace, see `Str\replace()`.
 * - For a single case-insensitive search/replace, see `Str\replace_ci()`.
 * - For multiple case-insensitive searches/replacements, see `Str\replace_every_ci()`.
 * - For not having new values searched again, see `Str\replace_every_nonrecursive()`.
 *
 * @guide /hack/built-in-types/string
 */
function replace_every(
  string $haystack,
  KeyedContainer<string, string> $replacements,
)[]: string {
  return _Str\replace_every_l($haystack, dict($replacements));
}

/**
 * Returns the "haystack" string with all occurrences of the keys of
 * `$replacements` replaced by the corresponding values (case-insensitive).
 *
 * Replacements are applied in the order they are specified in `$replacements`,
 * and the new values are searched again for subsequent matches. For example,
 * `dict['a' => 'b', 'b' => 'c']` is equivalent to `dict['a' => 'c']`, but
 * `dict['b' => 'c', 'a' => 'b']` is not, despite having the same elements.
 *
 * If there are multiple overlapping matches, the match occuring earlier in
 * `$replacements` (not in `$haystack`) takes precedence.
 *
 * - For a single case-sensitive search/replace, see `Str\replace()`.
 * - For a single case-insensitive search/replace, see `Str\replace_ci()`.
 * - For multiple case-sensitive searches/replacements, see `Str\replace_every()`.
 * - For not having new values searched again, see `Str\replace_every_nonrecursive_ci()`.
 *
 * @guide /hack/built-in-types/string
 */
function replace_every_ci(
  string $haystack,
  KeyedContainer<string, string> $replacements,
): string {
  return _Str\replace_every_ci_l($haystack, dict($replacements));
}

/**
 * Returns the "haystack" string with all occurrences of the keys of
 * `$replacements` replaced by the corresponding values. Once a substring has
 * been replaced, its new value will not be searched again.
 *
 * If there are multiple overlapping matches, the match occuring earlier in
 * `$haystack` takes precedence. If a replacer is a prefix of another (like
 * "car" and "carpet"), the longer one (carpet) takes precedence. The ordering
 * of `$replacements` therefore doesn't matter.
 *
 * - For having new values searched again, see `Str\replace_every()`.
 *
 * @guide /hack/built-in-types/string
 */
function replace_every_nonrecursive(
  string $haystack,
  KeyedContainer<string, string> $replacements,
)[]: string {
  return _Str\replace_every_nonrecursive_l($haystack, dict($replacements));
}

/**
 * Returns the "haystack" string with all occurrences of the keys of
 * `$replacements` replaced by the corresponding values (case-insensitive).
 * Once a substring has been replaced, its new value will not be searched
 * again.
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
 * - For having new values searched again, see `Str\replace_every_ci()`.
 *
 * @guide /hack/built-in-types/string
 */
function replace_every_nonrecursive_ci(
  string $haystack,
  KeyedContainer<string, string> $replacements,
): string {
  return _Str\replace_every_nonrecursive_ci_l($haystack, dict($replacements));
}

/** Reverse a string by bytes.
 *
 * @see `Str\reverse_l()` to reverse by characters instead.
 *
 * @guide /hack/built-in-types/string
 */
function reverse(string $string)[]: string {
  for ($lo = 0, $hi = namespace\length($string) - 1; $lo < $hi; $lo++, $hi--) {
    $temp = $string[$lo];
    $string[$lo] = $string[$hi];
    $string[$hi] = $temp;
  }
  return $string;
}

/**
 * Return the string with a slice specified by the offset/length replaced by the
 * given replacement string.
 *
 * If the length is omitted or exceeds the upper bound of the string, the
 * remainder of the string will be replaced. If the length is zero, the
 * replacement will be inserted at the offset.
 *
 * Offset can be positive or negative. When positive, replacement starts from the
 * beginning of the string; when negative, replacement starts from the end of the string.
 *
 * Some examples:
 * - `Str\splice("apple", "orange", 0)` without `$length`, `$string` is replaced, resolving to `"orange"`
 * - `Str\splice("apple", "orange", 3)` inserting at `$offset` `3` from the start of `$string` resolves to `"apporange"`
 * - `Str\splice("apple", "orange", -2)` inserting at `$offset` `-2` from the end of `$string` resolves to `"apporange"`
 * - `Str\splice("apple", "orange", 0, 0)` with `$length` `0`, `$replacement` is appended at `$offset` `0` and resolves to `"orangeapple"`
 * - `Str\splice("apple", "orange", 5, 0)` with `$length` `0`, `$replacement` is appended at `$offset` `5` and resolves to `"appleorange"`
 *
 * Previously known in PHP as `substr_replace`.
 *
 * @guide /hack/built-in-types/string
 */
function splice(
  string $string,
  string $replacement,
  int $offset,
  ?int $length = null,
)[]: string {
  return _Str\splice_l($string, $replacement, $offset, $length);
}

/**
 * Returns the given string as an integer, or null if the string isn't numeric.
 *
 * @guide /hack/built-in-types/string
 */
function to_int(
  string $string,
)[]: ?int {
  if ((string)(int)$string === $string) {
    return (int)$string;
  }
  return null;
}

/**
 * Returns the string with all alphabetic characters converted to uppercase.
 *
 * @guide /hack/built-in-types/string
 */
function uppercase(
  string $string,
)[]: string {
  return _Str\uppercase_l($string);
}
