<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HH\Lib\Str;

use namespace HH\Lib\{_Private, C, Keyset, Vec};

/**
 * Returns the string with the first character capitalized.
 *
 * If the first character is already capitalized or isn't alphabetic, the string
 * will be unchanged.
 *
 * - To capitalize all characters, see `Str\uppercase()`.
 * - To capitalize all words, see `Str\capitalize_words()`.
 */
function capitalize(
  string $string,
)[]: string {
  /* HH_FIXME[4390] \ucfirst is missing [] */
  return \ucfirst($string);
}

/**
 * Returns the string with all words capitalized.
 *
 * Words are delimited by space, tab, newline, carriage return, form-feed, and
 * vertical tab by default, but you can specify custom delimiters.
 *
 * - To capitalize all characters, see `Str\uppercase()`.
 * - To capitalize only the first character, see `Str\capitalize()`.
 */
function capitalize_words(
  string $string,
  string $delimiters = " \t\r\n\f\v",
)[]: string {
  /* HH_FIXME[4390] \ucwords is missing [] */
  return \ucwords($string, $delimiters);
}

/**
 * Returns a string representation of the given number with grouped thousands.
 *
 * If `$decimals` is provided, the string will contain that many decimal places.
 * The optional `$decimal_point` and `$thousands_separator` arguments define the
 * strings used for decimals and commas, respectively.
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
 */
function lowercase(
  string $string,
)[]: string {
  return \strtolower($string);
}

/**
 * Returns the string padded to the total length by appending the `$pad_string`
 * to the left.
 *
 * If the length of the input string plus the pad string exceeds the total
 * length, the pad string will be truncated. If the total length is less than or
 * equal to the length of the input string, no padding will occur.
 *
 * To pad the string on the right, see `Str\pad_right()`.
 */
function pad_left(
  string $string,
  int $total_length,
  string $pad_string = ' ',
)[]: string {
  invariant($pad_string !== '', 'Expected non-empty pad string.');
  invariant($total_length >= 0, 'Expected non-negative total length.');
  return \str_pad($string, $total_length, $pad_string, \STR_PAD_LEFT);
}

/**
 * Returns the string padded to the total length by appending the `$pad_string`
 * to the right.
 *
 * If the length of the input string plus the pad string exceeds the total
 * length, the pad string will be truncated. If the total length is less than or
 * equal to the length of the input string, no padding will occur.
 *
 * To pad the string on the left, see `Str\pad_left()`.
 */
function pad_right(
  string $string,
  int $total_length,
  string $pad_string = ' ',
)[]: string {
  invariant($pad_string !== '', 'Expected non-empty pad string.');
  invariant($total_length >= 0, 'Expected non-negative total length.');
  return \str_pad($string, $total_length, $pad_string, \STR_PAD_RIGHT);
}

/**
 * Returns the input string repeated `$multiplier` times.
 *
 * If the multiplier is 0, the empty string will be returned.
 */
function repeat(
  string $string,
  int $multiplier,
)[]: string {
  invariant($multiplier >= 0, 'Expected non-negative multiplier');
  return \str_repeat($string, $multiplier);
}

/**
 * Returns the "haystack" string with all occurrences of `$needle` replaced by
 * `$replacement`.
 *
 * - For a case-insensitive search/replace, see `Str\replace_ci()`.
 * - For multiple case-sensitive searches/replacements, see `Str\replace_every()`.
 * - For multiple case-insensitive searches/replacements, see `Str\replace_every_ci()`.
 */
function replace(
  string $haystack,
  string $needle,
  string $replacement,
)[]: string {
  return \str_replace($needle, $replacement, $haystack);
}

/**
 * Returns the "haystack" string with all occurrences of `$needle` replaced by
 * `$replacement` (case-insensitive).
 *
 * - For a case-sensitive search/replace, see `Str\replace()`.
 * - For multiple case-sensitive searches/replacements, see `Str\replace_every()`.
 * - For multiple case-insensitive searches/replacements, see `Str\replace_every_ci()`.
 */
// not pure: str_ireplace uses global locale for capitalization
function replace_ci(
  string $haystack,
  string $needle,
  string $replacement,
)[rx]: string {
  return \str_ireplace($needle, $replacement, $haystack);
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
 */
function replace_every(
  string $haystack,
  KeyedContainer<string, string> $replacements,
)[]: string {
  return \str_replace(
    \array_keys($replacements),
    \array_values($replacements),
    $haystack,
  );
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
 */
function replace_every_ci(
  string $haystack,
  KeyedContainer<string, string> $replacements,
)[rx]: string {
  return \str_ireplace(
    \array_keys($replacements),
    \array_values($replacements),
    $haystack,
  );
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
 */
function replace_every_nonrecursive(
  string $haystack,
  KeyedContainer<string, string> $replacements,
)[rx]: string {
  invariant(
    !C\contains_key($replacements, ''),
    'Expected non-empty keys only.',
  );

  return \strtr($haystack, $replacements);
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
 * an invariant exception is thrown.
 *
 * Time complexity: O(a + length * b), where a is the sum of all key lengths and
 * b is the sum of distinct key lengths (length is the length of `$haystack`)
 *
 * - For having new values searched again, see `Str\replace_every_ci()`.
 */
function replace_every_nonrecursive_ci(
  string $haystack,
  KeyedContainer<string, string> $replacements,
)[rx]: string {
  invariant(
    !C\contains_key($replacements, ''),
    'Expected non-empty keys only.',
  );

  $haystack_lc = lowercase($haystack);
  $key_lengths = keyset[];
  $replacements_lc = dict[];
  foreach ($replacements as $key => $value) {
    $key_lc = lowercase($key);
    invariant(
      !C\contains_key($replacements_lc, $key_lc),
      'Duplicate case-insensitive search string "%s".',
      $key_lc,
    );
    $key_lengths[] = length($key_lc);
    $replacements_lc[$key_lc] = $value;
  }

  $key_lengths = Vec\sort($key_lengths) |> Vec\reverse($$);

  $output = '';
  for ($pos = 0; $pos < length($haystack); ) {
    $found_match_at_pos = false;
    foreach ($key_lengths as $key_length) {
      $possible_match = slice($haystack_lc, $pos, $key_length);
      if (C\contains_key($replacements_lc, $possible_match)) {
        $found_match_at_pos = true;
        $output .= $replacements_lc[$possible_match];
        $pos += $key_length;
        break;
      }
    }

    if (!$found_match_at_pos) {
      $output .= $haystack[$pos];
      $pos++;
    }
  }

  return $output;
}

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
 * Previously known in PHP as `substr_replace`.
 */
function splice(
  string $string,
  string $replacement,
  int $offset,
  ?int $length = null,
)[]: string {
  invariant($length === null || $length >= 0, 'Expected non-negative length.');
  $offset = _Private\validate_offset($offset, length($string));
  return $length === null
    ? \substr_replace($string, $replacement, $offset)
    : \substr_replace($string, $replacement, $offset, $length);
}

/**
 * Returns the given string as an integer, or null if the string isn't numeric.
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
 */
function uppercase(
  string $string,
)[]: string {
  return \strtoupper($string);
}
