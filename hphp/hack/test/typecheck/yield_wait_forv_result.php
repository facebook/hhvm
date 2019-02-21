<?hh // partial
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

async function foo(): Awaitable<int> {
  return 123;
}

async function duck(): Awaitable<string> {
  return 'duck';
}

async function bar(): Awaitable<array<int>> {
  $a = array(foo(), foo(), foo());
  return await gena($a);
}

async function lala(): Awaitable<(int, string)> {
  $a = tuple(foo(), duck());
  return await gena($a);
}
