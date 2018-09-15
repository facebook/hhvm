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

async function something_complex(
  Awaitable<int> $i,
  Awaitable<string> $s,
): Awaitable<void> {
  if (true) {
    $v = $i;
  } else {
    $v = $s;
  }
  $z = await $v;
}
