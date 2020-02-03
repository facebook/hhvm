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

function foo(string $x): void {}

function test(): void {
  $arr = varray[varray[1, 2], varray[3, 4]];
  foreach ($arr as list($x, $y)) {
    foo($x);
  }
}
