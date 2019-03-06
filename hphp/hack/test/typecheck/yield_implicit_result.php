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

async function trything(): Awaitable<void> {
  try {
    return;
  } catch (Exception $e) {
  }
}

async function trything2(): Awaitable<void> {
  try {
    something();
  } catch (Exception $e) {
    return;
  }
}

async function nullable(): Awaitable<?int> {
  if (coin_flip()) {
    return 123;
  }
  return null;
}

function something(): void {}

function coin_flip(): bool {
  return true;
}
