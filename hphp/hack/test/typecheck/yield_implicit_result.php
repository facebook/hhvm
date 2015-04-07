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

async function trything(): Awaitable<void> {
  try {
    return null;
  } catch (Exception $e) {
  }
}

async function trything2(): Awaitable<void> {
  try {
    something();
  } catch (Exception $e) {
    return null;
  }
}

async function nullable(): Awaitable<?int> {
  if (coin_flip()) {
    return 123;
  }
}
