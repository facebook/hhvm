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

function f(int $x): bool {
  return true;
}

function test(int $x): void {
  switch (1) {
    case 1:
      $x = f($x);
      // FALLTHROUGH
    case 2:
      $x = f($x);
      break;
  }
}
