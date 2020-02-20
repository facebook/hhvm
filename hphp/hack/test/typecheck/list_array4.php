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

function g(int $i): void {}

function get_untyped_val() {
  return varray[1, 2];
}

function f(array $x): void {
  list($x, $y) = get_untyped_val();
  g($x);
  g($y);
}
