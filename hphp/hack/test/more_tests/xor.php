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

function int_xor(int $a, int $b): int {
  return $a ^ $b;
}

function bool_xor(bool $a, bool $b): bool {
  return $a ^ $b;
}

function unknown_int_xor1(int $a, $b): int {
  return $a ^ $b;
}

function unknown_int_xor2($a, int $b): int {
  return $a ^ $b;
}

function unknown_bool_xor1(bool $a, $b): bool {
  return $a ^ $b;
}

function unknown_bool_xor2($a, bool $b): bool {
  return $a ^ $b;
}

// Relatively arbitrarily lets says that $a^$b is an int if we don't know the
// types of either $a or $b. This is slightly better than bool, since
// technically the ^ operator in PHP operates on ints.
function unknown_xor($a, $b): int {
  return $a ^ $b;
}
