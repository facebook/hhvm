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

class C {
  public async function genVec(): Awaitable<Vector<int>> {
    return Vector {};
  }
}

function nullthrows<T>(?T $x): T {
  // UNSAFE
}

async function f(): Awaitable<int> {
  $va = nullthrows(new C())->genVec();
  $v = await $va;
  list($i) = $v;
  return $i;
}

async function g(): Awaitable<int> {
  $va = nullthrows(new C())->genVec();
  list($i) = await $va;
  return $i;
}
