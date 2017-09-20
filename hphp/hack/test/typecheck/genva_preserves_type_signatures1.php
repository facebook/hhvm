 <?hh // strict
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

use namespace HH\Lib\Tuple;

async function funtimes() : Awaitable<int> {
  return 314159;
}

async function animals(): Awaitable<string> {
  return "llama llama duck";
}

class PositionOrVelocity {}

async function heisenberg(): Awaitable<PositionOrVelocity> {
  return new PositionOrVelocity();
}

function test() : Awaitable<(int, string, PositionOrVelocity)> {
  return genva(funtimes(), animals(), heisenberg());
}

function test2() : Awaitable<(int, string, PositionOrVelocity)> {
  return \HH\Asio\va(funtimes(), animals(), heisenberg());
}

function test3() : Awaitable<(int, string, PositionOrVelocity)> {
  return Tuple\gen(funtimes(), animals(), heisenberg());
}

function test4() : Awaitable<(int, string, PositionOrVelocity)> {
  return Tuple\from_async(funtimes(), animals(), heisenberg());
}
