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

function foo(): void {
  $y = ($a, $b) ==> $a + $b;
  $y = () ==> {
    return ($x, $y) ==> $x + $y;
  };
  $z = $y();
  if ($z(1, 2) < 10) {
    echo "yay\n";
  }
  $y = () ==> ($x, $y) ==> $x + $y;
  $z = $y();
  if ($z(1, 2) < 10) {
    echo "yay\n";
  }
}
