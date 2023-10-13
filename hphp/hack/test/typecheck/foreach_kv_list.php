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

function takes_int(int $a): void {}

function takes_string(string $a): void {}

function foo(): varray<(int, string)> {
  return varray[];
}

function f(): void {
  foreach (foo() as $k => list($x, $y)) {
    takes_int($x);
    takes_string($y);
  }
}
