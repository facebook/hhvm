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

const int X = 1;

function f(int $a): int {
  $b = (X) ? $a : 2;
  $b = (X) * 2;
  $b = (X) / 2;
  $b = (X) % 2;
  $b = (X) >> 2;
  $b = (X) << 2;
  $b = (X) ^ 2;
  $b = vec[1, 2][(X)];
  $b = (X);
  return $b;
}
