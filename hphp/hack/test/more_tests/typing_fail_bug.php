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

function f(mixed $arg): void {
  if (is_int($arg)) {
    $tmp = 0;
  } else {
    throw new Exception('');
  }
  g($tmp);
}

function g(string $s): void { }
