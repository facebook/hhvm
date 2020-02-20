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

function any() {}

async function f(): Awaitable<void> {
  if (true) {
    $x = any();
  } else {
    $x = cached_result(varray[1, false]);
  }

  $x = await $x;
  list($one, $two) = $x;
}
