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
