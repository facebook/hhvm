<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

async function foo(): Awaitable<int> {
  return 123;
}

async function bar(): Awaitable<array<string>> {
  return await gen_array_rec(array(foo(), array(1, 2, 3), foo()));
}
