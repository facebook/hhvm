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

function get_arr(): array<int> { return array(1); }
function foo(int $x) {}

function test() {
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
