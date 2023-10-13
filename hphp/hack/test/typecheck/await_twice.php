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

async function tweedledum(): Awaitable<string> {
  return "alice";
}

async function tweedledee(): Awaitable<Awaitable<string>> {
  return tweedledum();
}

async function whats_in_the_rabbit_hole(): Awaitable<void> {
  $a = await tweedledee();
  $b = await $a;
  print($b);
}
