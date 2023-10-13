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

async function foo<T>(T $x): Awaitable<T> {
  return $x;
}

async function foo_error<T>(T $x): T {
  return $x;
}
