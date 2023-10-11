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

class Stream<T> {}

async function g(): Awaitable<bool> {
  return false;
}

async function f(): Awaitable<int> {
  $x = await g();
  if ($x) {
    return 1;
  } else {
    return 2;
  }
}

function intgen(): Stream<int> {
  yield 1;
  yield 2;
}
