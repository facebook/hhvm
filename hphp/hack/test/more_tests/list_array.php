<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

function test(): int {
  list($x, $y) = array(1, 2);
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
