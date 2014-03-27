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

function do_nothing() {
  $x = 2;
}

function do_nothing_and_return() {
  $x = 2;
  return;
}

async function gen_an_int(): Awaitable<int> {
  return 0;
}

async function await_void() {
  $i = await gen_an_int();
}

async function await_int() {
  $i = await gen_an_int();
  return $i;
}

async function await_maybe() {
  $i = await gen_an_int();
  if ($i) {
    return $i;
  } else {
    return null;
  }
}

async function await_maybe2() {
  $i = await gen_an_int();
  if ($i) {
    return $i;
  }
}

async function await_null() { // No way to tell what the optional type is.
  return null;
}

class C {
  private int $x = 1;

  public function frob() {
    $this->x *= 2;
  }
}
