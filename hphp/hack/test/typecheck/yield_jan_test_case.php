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

class A{}
class A1 extends A{}
class A2 extends A{}

async function gen1() : Awaitable<A1> {
  return new A1();
}
async function gen2() : Awaitable<A2> {
  return new A2();
}
async function gen() : Awaitable<A> {
  if (coin_flip()) {
    return await gen1();
  } else {
    return await gen2();
  }
}
