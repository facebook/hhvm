<?hh

/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HH\Lib\C;

use namespace HH\Lib\Vec;

/**
 * Returns true if the given Traversable<Tv> is sorted in ascending order.
 * If two neighbouring elements compare equal, this will be considered sorted.
 *
 * If no $comparator is provided, the `<=>` operator will be used.
 * This will sort numbers by value, strings by alphabetical order
 * or by the numeric value, if the strings are well-formed numbers,
 * and DateTime/DateTimeImmutable by their unixtime.
 *
 * To check the order of other types or mixtures of the
 * aforementioned types, see C\is_sorted_by.
 *
 * If the comparison operator `<=>` is not useful on Tv
 * and no $comparator is provided, the result of is_sorted
 * will not be useful.
 *
 * Time complexity: O((n * c), where c is the complexity of the
 * comparator function (which is O(1) if not provided explicitly)
 * Space complexity: O(n)
 */
function is_sorted<Tv>(
  Traversable<Tv> $traversable,
  ?(function(Tv, Tv)[_]: num) $comparator = null,
)[ctx $comparator]: bool {
  $vec = Vec\cast_clear_legacy_array_mark($traversable);
  if (is_empty($vec)) {
    return true;
  }

  $comparator ??= (Tv $a, Tv $b) ==>
    /*HH_FIXME[4240] Comparison may not be useful on Tv*/$a <=> $b;

  $previous = firstx($vec);
  foreach ($vec as $next) {
    if ($comparator($next, $previous) < 0) {
      return false;
    }
    $previous = $next;
  }

  return true;
}

/**
 * Returns true if the given Traversable<Tv> would be sorted in ascending order
 * after having been `Vec\map`ed with $scalar_func sorted in ascending order.
 * If two neighbouring elements compare equal, this will be considered sorted.
 *
 * If no $comparator is provided, the `<=>` operator will be used.
 * This will sort numbers by value, strings by alphabetical order
 * or by the numeric value, if the strings are well-formed numbers,
 * and DateTime/DateTimeImmutable by their unixtime.
 *
 * To check the order without a mapping function,
 * see `C\is_sorted`.
 *
 * If the comparison operator `<=>` is not useful on Ts
 * and no $comparator is provided, the result of is_sorted_by
 * will not be useful.
 *
 * Time complexity: O((n * c), where c is the complexity of the
 * comparator function (which is O(1) if not provided explicitly)
 * Space complexity: O(n)
 */
function is_sorted_by<Tv, Ts>(
  Traversable<Tv> $traversable,
  (function(Tv)[_]: Ts) $scalar_func,
  ?(function(Ts, Ts)[_]: num) $comparator = null,
)[ctx $scalar_func, ctx $comparator]: bool {
  $vec = Vec\cast_clear_legacy_array_mark($traversable);
  if (is_empty($vec)) {
    return true;
  }

  $comparator ??= (Ts $a, Ts $b) ==>
    /*HH_FIXME[4240] Comparison may not be useful on Ts*/$a <=> $b;

  $previous = $scalar_func(firstx($vec));
  foreach ($vec as $next) {
    $next = $scalar_func($next);
    if ($comparator($next, $previous) < 0) {
      return false;
    }
    $previous = $next;
  }

  return true;
}
