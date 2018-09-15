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

function foo(): void {
  $y = 2;

  $x = $z ==> $y + $z; // $y is captured
  $l = $x(2);

  $x = function($z) use ($y) {
    return $y + $z; // $y is captured
  }
  $l = $x(2);

  $x = ($a, $b) ==> $a + $b; // no captures
  $l = $x(2, $y)
}
