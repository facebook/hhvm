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

function f(): (function(int): Awaitable<int>) {
  return async $v ==> 1;
}

function g(): (function(int, int): Awaitable<int>) {
  return async ($v, $w) ==> 1;
}

function h(): (function(int, int): Awaitable<int>) {
  return async (int $v, int $w): Awaitable<int> ==> 1;
}
