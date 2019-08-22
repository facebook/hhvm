<?hh // strict
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
function prep<T>(Awaitable<T> $x): T {

}

async function a(): Awaitable<int> {
  return 1;
}

function f(): void {
  $x = a();
  $y = prep($x);
}
