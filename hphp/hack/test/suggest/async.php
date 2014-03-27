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

async function g() {
  return 1;
}

async function gen_an_int(): Awaitable<int> {
  return 1;
}

async function h() {
  return await gen_an_int();
}
