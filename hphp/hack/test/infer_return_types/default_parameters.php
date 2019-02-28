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

function f($x = null): ?int {
  return $x;
}

function g(): void {
  f(1);
}

function f2($x = 1): int {
  return $x;
}
