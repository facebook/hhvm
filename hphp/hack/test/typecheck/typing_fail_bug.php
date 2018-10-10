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

function f(mixed $arg): void {
  if ($arg is int) {
    $tmp = 0;
  } else {
    throw new Exception('');
  }
  g($tmp);
}

function g(string $s): void {}
