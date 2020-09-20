<?hh // partial
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

function cached_result<T>(T $x): Awaitable<T> {
  throw new Exception();
}

function consume<T>(Awaitable<T> $x): void {}

function f(): void {
  if (true) {
    $x = cached_result('hi');
  } else {
    $x = cached_result(null);
  }

  consume($x);
}
