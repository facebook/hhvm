<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

function coin_flip(): bool { return false; }

async function gen_f2(Awaitable<void> $a, Awaitable<void> $b): Awaitable<void> {
  if (coin_flip()) { return await $a; }
  if (coin_flip()) { return await $b; }
  // Implicit return null in an async function is compatible with
  // Awaitable<void>
}
