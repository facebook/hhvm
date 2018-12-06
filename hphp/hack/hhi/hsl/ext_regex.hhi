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
