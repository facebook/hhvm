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

function get_arr(): array<?int> {
  return array(1, null, 2);
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
