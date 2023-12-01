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

function get_arr(): darray<string, bool> {
  return dict[
    'foo' => true,
    'bar' => false,
  ];
}

function use_arr(darray<mixed, int> $arr): void {
}

function test(): void {
  $a = get_arr();
  use_arr($a);
}
