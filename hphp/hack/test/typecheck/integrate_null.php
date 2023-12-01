<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

function foo(): int {
  $x = null;
  foreach (vec[1,2,3] as $y) {
    if ($y % 2) { continue; }
    $x = vec[$y];
  }
  if (!$x) {
    throw new Exception('Boom');
  }
  return $x[123];
}
