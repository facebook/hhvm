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

/**
 * Returns a 2-tuple containing dicts for which the given predicate returned
 * `true` and `false`, respectively.
 *
 * Time complexity: O(n * p), where p is the complexity of `$predicate`.
 * Space complexity: O(n)
 */
function partition<Tk as arraykey, Tv>(
  KeyedTraversable<Tk, Tv> $traversable,
  (function(Tv)[_]: bool) $predicate,
)[ctx $predicate]: (dict<Tk, Tv>, dict<Tk, Tv>) {
  $success = dict[];
  $failure = dict[];
  foreach ($traversable as $key => $value) {
    if ($predicate($value)) {
      $success[$key] = $value;
    } else {
      $failure[$key] = $value;
    }
  }
  return tuple($success, $failure);
}

/**
 * Returns a 2-tuple containing dicts for which the given keyed predicate
 * returned `true` and `false`, respectively.
 *
 * Time complexity: O(n * p), where p is the complexity of `$predicate`.
 * Space complexity: O(n)
 */
function partition_with_key<Tk as arraykey, Tv>(
  KeyedTraversable<Tk, Tv> $traversable,
  (function(Tk, Tv)[_]: bool) $predicate,
)[ctx $predicate]: (dict<Tk, Tv>, dict<Tk, Tv>) {
  $success = dict[];
  $failure = dict[];
  foreach ($traversable as $key => $value) {
    if ($predicate($key, $value)) {
      $success[$key] = $value;
    } else {
      $failure[$key] = $value;
    }
  }
  return tuple($success, $failure);
}
