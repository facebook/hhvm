<?hh // strict
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

function get_arr(): array<int> {
  return array(1, 2, 3);
}

function use_arr(array<bool> $arr): void {
}

function test(): void {
  $a = get_arr();
  use_arr($a);
}
