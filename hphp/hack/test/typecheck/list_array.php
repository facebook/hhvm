<?hh // partial
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

function test(): int {
  list($x, $y) = varray[1, 2];
  return $x;
}

function test2(array $x): int {
  list($x, $y) = $x;
  return $x;
}

function test3(array<int> $x): int {
  list($x, $y) = $x;
  return $x;
}
