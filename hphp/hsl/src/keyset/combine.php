<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HH\Lib\Keyset;

/**
 * Returns a new keyset containing all of the elements of the given
 * Traversables.
 *
 * For a variable number of Traversables, see `Keyset\flatten()`.
 *
 * Time complexity: O(n + m), where n is the size of `$first` and m is the
 * combined size of all the `...$rest`
 * Space complexity: O(n + m), where n is the size of `$first` and m is the
 * combined size of all the `...$rest`
 */
function union<Tv as arraykey>(
  Traversable<Tv> $first,
  Container<Tv> ...$rest
)[]: keyset<Tv> {
  $result = keyset($first);
  foreach ($rest as $traversable) {
    foreach ($traversable as $value) {
      $result[] = $value;
    }
  }
  return $result;
}
