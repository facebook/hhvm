<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HH\Lib\Dict;

use namespace HH\Lib\{C, Vec};

/**
 * Returns a new dict where each element in `$keys` maps to the
 * corresponding element in `$values`.
 *
 * Time complexity: O(n) where n is the size of `$keys` (which must be the same
 * as the size of `$values`)
 * Space complexity: O(n) where n is the size of `$keys` (which must be the same
 * as the size of `$values`)
 */
function associate<Tk as arraykey, Tv>(
  Traversable<Tk> $keys,
  Traversable<Tv> $values,
)[]: dict<Tk, Tv> {
  $key_vec = Vec\cast_clear_legacy_array_mark($keys);
  $value_vec = Vec\cast_clear_legacy_array_mark($values);
  invariant(
    C\count($key_vec) === C\count($value_vec),
    'Expected length of keys and values to be the same',
  );
  $result = dict[];
  foreach ($key_vec as $idx => $key) {
    $result[$key] = $value_vec[$idx];
  }
  return $result;
}

/**
 * Merges multiple KeyedTraversables into a new dict. In the case of duplicate
 * keys, later values will overwrite the previous ones.
 *
 * Time complexity: O(n + m), where n is the size of `$first` and m is the
 * combined size of all the `...$rest`
 * Space complexity: O(n + m), where n is the size of `$first` and m is the
 * combined size of all the `...$rest`
 */
function merge<Tk as arraykey, Tv>(
  KeyedTraversable<Tk, Tv> $first,
  KeyedContainer<Tk, Tv> ...$rest
)[]: dict<Tk, Tv> {
  $result = cast_clear_legacy_array_mark($first);
  foreach ($rest as $traversable) {
    foreach ($traversable as $key => $value) {
      $result[$key] = $value;
    }
  }
  return $result;
}
