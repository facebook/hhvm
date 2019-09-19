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

/* HH_FIXME[4336] */
function cached_result<T>(T $x): Awaitable<T> {
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
