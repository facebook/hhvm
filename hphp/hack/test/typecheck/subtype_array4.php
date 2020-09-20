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

function get_arr(): array<string, bool> {
  return darray[
    'foo' => true,
    'bar' => false,
  ];
}

function use_arr(array<mixed, int> $arr): void {
}

function test(): void {
  $a = get_arr();
  use_arr($a);
}
