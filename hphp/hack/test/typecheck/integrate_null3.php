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

function get_arr(): varray<?int> {
  return vec[1, null, 2];
}

function f(int $x): void {}

function test(): void {
  $arr = get_arr();
  foreach ($arr as $x) {
    if (!$x) {
      continue;
    }

    f($x);
  }
}
