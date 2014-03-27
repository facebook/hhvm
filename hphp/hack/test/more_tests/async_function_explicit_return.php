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

async function foo<T>(T $x): Awaitable<T> {
  return $x;
}

class Baz {
  public static async function foo(): Awaitable<string> {
    return "hello";
  }
}

async function sun<T>(T $x): Awaitable<Awaitable<T>> {
  return foo($x);
}

async function bar(): Awaitable<mixed> {
  if (true) {
    return 5;
  }
  return "hello";
}
