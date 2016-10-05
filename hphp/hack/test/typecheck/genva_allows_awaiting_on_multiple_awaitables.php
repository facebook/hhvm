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

async function slow_adder(Awaitable<int> $sturm, Awaitable<int> $drang) : Awaitable<int> {
  list($so, $deep) = await genva($sturm, $drang);
  return $so + $deep;
}

async function slow_adder2(Awaitable<int> $sturm, Awaitable<int> $drang) : Awaitable<int> {
  list($so, $deep) = await \HH\Asio\va($sturm, $drang);
  return $so + $deep;
}
