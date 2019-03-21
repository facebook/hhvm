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

function foo(): int {
  static $x = 0;
  $x++;
  return $x;
}

function main(): void {
  $acc = 0;
  $acc += foo();
  $acc += foo();
  $acc += foo();
  if($acc === 6) {
    echo 'OK';
  }
  else {
    echo 'Failure: test_static.1';
  }
}
