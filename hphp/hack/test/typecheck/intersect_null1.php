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

function get_arr(): varray<int> { return vec[1]; }
function foo(int $x): void {}

function test(): void {
  foreach (get_arr() as $_) {
    $y = null;
    if (true) {
      $y = 2;
    } else {
      $y = 3;
    }

    foo($y);
  }
}
