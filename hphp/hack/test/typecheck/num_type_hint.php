<?hh // strict
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

function f(num $n1, num $n2): num {
  return $n1 / $n2;
}

function div(int $n1, int $n2): num {
  return $n1 / $n2;
}

function cast(num $n1, num $n2): (float, int) {
  return tuple((float) $n1, (int) $n2);
}

// We _could_ be a tiny bit smarter and consider the following code correct
// by observing that the is_int and is_float guards are exhaustive.
//
// function disjoint_conditions(num $n): int {
//   if (is_int($n)) {
//     $x = 1;
//   } else if (is_float($n)) {
//     invariant_violation('This should be it');
//   }
//   return $x;
// }

function f_opt(?num $n1, num $n2): num {
  if (null === $n1) {
    return 1.0;
  }
  return f($n1, $n2);
}

function test(): void {
  f(1, 1);
  f(1.5, 1.5);
  f(1.0, 1);
  f(1, 1.5);

  f_opt(1, 1);
  f_opt(1.5, 1.5);
  f_opt(1.0, 1);
  f_opt(1, 1.5);
  f_opt(null, 1);
}

function test_switch(int $x, int $y): bool {
  $n = $x / $y;
  hh_show($n);
  $c = 1.0;
  if ($c == $n) {
    return true;
  }

  switch ($n) {
    case $c: // == comparison, $n type shouldn't change
      hh_show($n);
      return true;
    case 2.0:
      $res = false;
      break;
    case 3:
      $res = true;
      break;
    default:
      $res = true;
  }
  return $res;
}
