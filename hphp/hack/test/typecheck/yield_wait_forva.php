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

async function as_tuple(): Awaitable<(int, string)> {
  $ret = await genva(
    gen_int(),
    gen_string()
  );
  return $ret;
}

async function maybe_null(): Awaitable<(int, ?int)> {
  $gen = null;
  if (flip_coin()) {
    $gen = gen_int();
  }
  $ret = await genva(
    gen_int(),
    $gen
  );
  return $ret;
}

async function def_null(): Awaitable<(int, ?int, ?string, ?int)> {
  $ret = await genva(
    gen_int(),
    gen_int_opt(),
    null,
    coin_flip() ? gen_int() : null
  );
  return $ret;
}

async function gen_int(): Awaitable<int> {
  return 123;
}

async function gen_int_opt(): Awaitable<?int> {
  if (coin_flip()) {
    return 123;
  }
}

async function gen_string(): Awaitable<string> {
  return 'hello';
}
