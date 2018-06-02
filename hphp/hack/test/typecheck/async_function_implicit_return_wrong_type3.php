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

function coin_flip(): bool { return false; }

function f(Awaitable<void> $a, Awaitable<void> $b): Awaitable<void> {
  if (coin_flip()) { return $a; }
  if (coin_flip()) { return $b; }
  // Implicit return null in a normal function is not compatible with
  // Awaitable<void>
}
