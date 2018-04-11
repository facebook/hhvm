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

function takes_int(int $a): void {}

function takes_string(string $a): void {}

async function bar(): AsyncIterator<(int, string)> {
  yield tuple(1, "a");
}

async function test(): Awaitable<void> {
  foreach (bar() await as list($x, $y)) {
    takes_int($x);
    takes_string($y);
  }
}
