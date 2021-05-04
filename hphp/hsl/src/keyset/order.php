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
 * Returns a new keyset sorted by the values of the given Traversable. If the
 * optional comparator function isn't provided, the values will be sorted in
 * ascending order.
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
