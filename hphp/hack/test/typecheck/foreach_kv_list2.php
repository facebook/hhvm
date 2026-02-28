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

function foo(string $x): void {}

function test(): void {
  $arr = vec[vec[1, 2], vec[3, 4]];
  foreach ($arr as $k => list($x, $y)) {
    foo($x);
  }
}
