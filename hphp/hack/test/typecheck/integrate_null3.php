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

function get_arr(): array<?int> {
  return varray[1, null, 2];
}

function f(int $x) {}

function test() {
  $arr = get_arr();
  foreach ($arr as $x) {
    if (!$x) {
      continue;
    }

    f($x);
  }
}
