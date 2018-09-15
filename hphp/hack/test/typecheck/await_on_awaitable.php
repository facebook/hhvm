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

async function slow_print(Awaitable<string> $x): Awaitable<void> {
  $a = await $x;
  display($a);
}

function display(string $y): void {
  print $y;
}
