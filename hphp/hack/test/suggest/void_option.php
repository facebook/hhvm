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

function v(): void {}

function f() {
  if (true) {
    return null;
  }
}

async function g() {
  if (true) {
    return null;
  }

  // This is weird but it's the only case we know for sure this is supposed to
  // be Awaitable<void> due to our crazy rules around "yield result(null)" that
  // async functions inherited.
  return v();
}
