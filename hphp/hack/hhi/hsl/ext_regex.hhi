<?hh
/**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

namespace HH\Lib\Regex {
  /**
   * Example usage of a Match:
   *
   * $match = Regex\first_match(
   *   $some_string,
   *   re"/^(positional)and(?<named>foo)$/"
   * );
   *
   * if ($match is nonnull) {
   *   $match[0];  // OK, full matched string
   *   $match[1]; // OK, first positional group
   *   $match['named']; // OK, named group
   *   $match['nonexistent']; // ERROR: The field nonexistent is undefined (Typing[4108])
   *                          // (that field is not given by T, the specific
   *                          // instantiation of the Pattern<T>.
   * }
   */
  type Match = shape(...);

  /**
   * Example usage of a Pattern<T>:
   *
   * $pattern = re"/^(positional)and(?<named>foo)$/";
   *
   * The 're' prefix with a double-quoted string is interpreted by the Hack
   * server which will extract information such as the number and names of
   * positional arguments. This can then be used with various Regex\ functions,
   * the results of which represent a Match.
   */
  newtype Pattern<+T as Match> = string;
}

namespace HH\Lib\_Private\_Regex {
  /**
   * Tries to match $pattern in $haystack (starting at $offset). If it matches
   * then $offset is updated to the start of the match, and a 2-tuple with
   * the matches and a null error is returned. If it does not match but there
   * is no error, a 2-tuple with 2 nulls is returned. If there is an error, a
   * 2-tuple with the error number in the second position is returned.
   */
  function match<T as \HH\Lib\Regex\Match>(
    string $haystack,
    \HH\Lib\Regex\Pattern<T> $pattern,
    inout int $offset,
  )[]: (?T, ?int);

  /**
   * Tries to match $pattern in $haystack, replacing any matches with
   * $replacement (with backreference support in the replacement). On success
   * a 2-tuple with the resulting string and a null error is returned (no
   * match is a success and returns the $haystack unmodified). On error, a
   * 2-tuple with the error number in the second position is returned.
   */
  function replace(
    string $haystack,
    \HH\Lib\Regex\Pattern<\HH\Lib\Regex\Match> $pattern,
    string $replacement,
  )[]: (?string, ?int);
}
