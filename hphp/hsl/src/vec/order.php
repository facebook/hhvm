<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

namespace HH\Lib\Vec;

use namespace HH\Lib\{C, Dict, Math, Str};

/**
 * Returns a new vec containing the range of numbers from `$start` to `$end`
 * inclusive, with the step between elements being `$step` if provided, or 1 by
 * default. If `$start > $end`, it returns a descending range instead of
 * an empty one.
 *
 * If you don't need the items to be enumerated, consider Vec\fill.
 *
 * Time complexity: O(n), where `n` is the size of the resulting vec
 * Space complexity: O(n), where `n` is the size of the resulting vec
 */
function range<Tv as num>(
  Tv $start,
  Tv $end,
  ?Tv $step = null,
)[]: vec<Tv> {
  $step ??= 1;
  invariant($step > 0, 'Expected positive step.');
  if ($step > Math\abs($end - $start)) {
    return vec[$start];
  }
  return vec(\range($start, $end, $step));
}

/**
 * Returns a new vec with the values of the given Traversable in reversed
 * order.
 *
 * Time complexity: O(n)
 * Space complexity: O(n)
 */
function reverse<Tv>(
  Traversable<Tv> $traversable,
)[]: vec<Tv> {
  $vec = cast_clear_legacy_array_mark($traversable);
  for ($lo = 0, $hi = C\count($vec) - 1; $lo < $hi; $lo++, $hi--) {
    $temp = $vec[$lo];
    $vec[$lo] = $vec[$hi];
    $vec[$hi] = $temp;
  }
  return $vec;
}

/**
 * Returns a new vec with the values of the given Traversable in a random
 * order.
 *
 * Vec\shuffle is not using cryptographically secure randomness.
 *
 * Time complexity: O(n)
 * Space complexity: O(n)
 */
function shuffle<Tv>(
  Traversable<Tv> $traversable,
)[defaults]: vec<Tv> {
  $vec = cast_clear_legacy_array_mark($traversable);
  \shuffle(inout $vec);
  return $vec;
}

/**
 * Returns a new vec sorted by the values of the given Traversable. If the
 * optional comparator function isn't provided, the values will be sorted in
 * ascending order.
 *
 * To sort by some computable property of each value, see `Vec\sort_by()`.
 *
 * Time complexity: O((n log n) * c), where c is the complexity of the
 * comparator function (which is O(1) if not provided explicitly)
 * Space complexity: O(n)
 */
function sort<Tv>(
  Traversable<Tv> $traversable,
  ?(function(Tv, Tv)[_]: num) $comparator = null,
)[ctx $comparator]: vec<Tv> {
  $vec = cast_clear_legacy_array_mark($traversable);
  if ($comparator) {
    \usort(inout $vec, $comparator);
  } else {
    \sort(inout $vec);
  }
  return $vec;
}

/**
 * Returns a new vec sorted by some scalar property of each value of the given
 * Traversable, which is computed by the given function. If the optional
 * comparator function isn't provided, the values will be sorted in ascending
 * order of scalar key.
 *
 * To sort by the values of the Traversable, see `Vec\sort()`.
 *
 * Time complexity: O((n log n) * c + n * s), where c is the complexity of the
 * comparator function (which is O(1) if not provided explicitly) and s is the
 * complexity of the scalar function
 * Space complexity: O(n)
 */
function sort_by<Tv, Ts>(
  Traversable<Tv> $traversable,
  (function(Tv)[_]: Ts) $scalar_func,
  ?(function(Ts, Ts)[_]: num) $comparator = null,
)[ctx $scalar_func, ctx $comparator]: vec<Tv> {
  $vec = cast_clear_legacy_array_mark($traversable);
  $order_by = Dict\map($vec, $scalar_func);
  if ($comparator) {
    \uasort(inout $order_by, $comparator);
  } else {
    \asort(inout $order_by);
  }
  return map_with_key($order_by, ($k, $v) ==> $vec[$k]);
}
