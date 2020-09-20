<?hh // strict
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
