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

function foo(string $x): void {}

async function bar(): AsyncIterator<(int, int)> {
  yield tuple(1, 1);
}

async function test(): Awaitable<void> {
  foreach (bar() await as list($x, $y)) {
    foo($x);
  }
}
