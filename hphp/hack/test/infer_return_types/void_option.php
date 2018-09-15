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

function v(): void {}

function f(bool $b) {
  if ($b) {
    return null;
  }
}

async function g(bool $b) {
  if ($b) {
    return null;
  }

  // This is weird but it's the only case we know for sure this is supposed to
  // be Awaitable<void> due to our crazy rules around "yield result(null)" that
  // async functions inherited.
  return v();
}
