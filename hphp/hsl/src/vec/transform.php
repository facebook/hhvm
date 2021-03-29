<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HH\Experimental\BuiltInLib\VecBI;

/**
 * Returns a new vec where each value is the result of calling the given
 * function on the original value.
 *
 * For async functions, see `Vec\map_async()`.
 *
 * Time complexity: O(n)
 * Space complexity: O(n)
 */
<<__ProvenanceSkipFrame>>
function map<Tv1, Tv2>(
  Traversable<Tv1> $traversable,
  (function(Tv1)[_]: Tv2) $value_func,
)[ctx $value_func]: vec<Tv2> {
  $result = vec[];
  foreach ($traversable as $value) {
    $result[] = $value_func($value);
  }
  return $result;
}
