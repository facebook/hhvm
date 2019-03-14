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

async function bar(): Awaitable<array<string>> {
  return await gen_array_rec(array(foo(), array(1, 2, 3), foo()));
}
