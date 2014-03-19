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

function g(int $i): void {}

function get_untyped_val() {
  return array(1, 2);
}

function f(array $x): void {
  list($x, $y) = get_untyped_val();
  g($x);
  g($y);
}
