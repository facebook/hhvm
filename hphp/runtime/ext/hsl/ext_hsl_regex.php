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

namespace HH\Lib\Regex {
  /**
   * A shape representing a regular expression match and capture groups.
   *
   * See `Pattern<T>` for details.
   */
  type Match = shape(...);

  /**
   * The type of regular expression prefix-strings, e.g. `re"/foo/"`.
   *
   * Regular expression patterns must be string literals, and must include
   * delimiters.
   *
   * The generic type `T` is inferred from capture groups in the pattern;
   * for example:
   * - `re"/foo/"` is a `Pattern<shape(0 => string)>`
   * - `re"/(foo)?bar/"` is a `Pattern<shape(0 => string, 1 => string)>`
   * - `re"/(?<name>foo)?bar/"` is a
   *   `Pattern<shape(0 => string, 1 => string, 'name' => string)>`
   *
   * Given the integer keys, these `Match` types can not be explicitly
   * declared in Hack code.
   *
   * Other delimiters can be used such as:
   * - `re",/useful/if/pattern/contains/literal/slashes,"`
   * - `re"(bracket-like delimiters are matched pairs)"`
   *
   * Standard modifiers can follow the final delimiter, such as
   * `re"/this regular-expression is case-insensitive/i"`.
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
  <<__Native>>
  function match<T as \HH\Lib\Regex\Match>(
    string $haystack,
    string $pattern, // actually \HH\Lib\Regex\Pattern<T>
    inout int $offset,
  )[]: (?T, ?int);

  /**
   * Tries to match $pattern in $haystack, replacing any matches with
   * $replacement (with backreference support in the replacement). On success
   * a 2-tuple with the resulting string and a null error is returned (no
   * match is a success and returns the $haystack unmodified). On error, a
   * 2-tuple with the error number in the second position is returned.
   */
  <<__Native>>
  function replace(
    string $haystack,
    string $pattern, // actually \HH\Lib\Regex\Pattern<\HH\Lib\Regex\Match>
    string $replacement,
  )[]: (?string, ?int);
}
