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

async function foo(): Awaitable<void> {
  await gena(gen_arr());
}

async function gen_arr(): Awaitable<array> {
  return array(1, 2, 3);
}
