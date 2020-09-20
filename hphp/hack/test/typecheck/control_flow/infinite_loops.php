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

function cond(): bool {
  return false;
}

function test1(): string {
  $x = 1;
  while (true) {
    $x = "derp";
    if (cond()) {
      break;
    }
    $x = 2;
  }

  return $x;
}

function test2(): string {
  $x = 1;
  do {
    $x = "derp";
    if (cond()) {
      break;
    }
    $x = 2;
  } while (true);

  return $x;
}

function test3(): string {
  $x = 1;
  for (; ; ) {
    $x = "derp";
    if (cond()) {
      break;
    }
    $x = 2;
  }

  return $x;
}
