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

function f1(): ?resource {
  // UNSAFE
  return fopen('/dev/null', 'r');
}

function f2(resource $x): void {
}

function f3(): void {
  $x = f1();
  if ($x is resource) {
    f2($x);
  }
}
