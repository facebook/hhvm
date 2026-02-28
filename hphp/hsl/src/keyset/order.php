<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

namespace HH\Lib\Keyset;

/**
 * Returns a new keyset sorted by the values of the given Traversable. If the
 * optional comparator function isn't provided, the values will be sorted in
 * ascending order.
 *
 * When specified, a comparator should exhibit the same behavior as the `<=>`
 * operator: a negative value when the left operand compares less than the
 * right operand, 0 when the operands compare equal, and a positive value when
 * the left operand compares greater than the right operand.
 *
 * Time complexity: O((n log n) * c), where c is the complexity of the
 * comparator function (which is O(1) if not explicitly provided)
 * Space complexity: O(n)
 */
function sort<Tv as arraykey>(
  Traversable<Tv> $traversable,
  ?(function(Tv, Tv)[_]: num) $comparator = null,
)[ctx $comparator]: keyset<Tv> {
  $keyset = keyset($traversable);
  if ($comparator) {
    \uksort(inout $keyset, $comparator);
  } else {
    \ksort(inout $keyset);
  }
  return keyset($keyset);
}
