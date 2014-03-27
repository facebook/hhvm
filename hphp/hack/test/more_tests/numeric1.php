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

function f1(): bool {
  $x = 1.0;
  return $x < 42;
}

function f2(): bool {
  if (true) {
    $x = 1;
  } else {
    $x = 1.0;
  }
  return $x < 42;
}

function f3(): bool {
  $x = 1.0;
  if (true) {
    $y = "foo";
  }
  return $x < 42;
}

