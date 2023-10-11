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

function get_arr(): varray<int> {
  return varray[1, 2, 3];
}

function use_arr(varray<bool> $arr): void {}

function test(): void {
  $a = get_arr();
  use_arr($a);
}
