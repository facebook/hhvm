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

function prep<T>(Awaitable<T> $x): T {
  throw new Exception();
}

async function a(): Awaitable<int> {
  return 1;
}

function f(): void {
  $x = a();
  $y = prep($x);
}
