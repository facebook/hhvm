<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HH\Lib\_Private;

use namespace HH\Lib\{Regex, Str};

/**
 * Returns the first match found in `$haystack` given the regex pattern `$pattern`
 * and an offset at which to start the search. The offset is updated to point
 * to the start of the match.
 *
 * Throws Invariant[Violation]Exception if `$offset` is not within plus/minus the length of `$haystack`
 * Returns null, or a Match containing
 *   - the entire matching string, at key 0,
 *   - the results of unnamed capture groups, at integer keys corresponding to
 *       the groups' occurrence within the pattern, and
 *   - the results of named capture groups, at string keys matching their respective names,
 */
function regex_match<T as Regex\Match>(
  string $haystack,
  Regex\Pattern<T> $pattern,
  inout int $offset,
)[]: ?T {
  $offset = validate_offset($offset, Str\length($haystack));
  list ($matches, $error) = _Regex\match($haystack, $pattern, inout $offset);
  if ($error is nonnull) {
    throw new Regex\Exception($pattern, $error);
  }
  return $matches;
}
