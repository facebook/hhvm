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

async function tweedledum(): Awaitable<string> {
  return "alice";
}

async function tweedledee(): Awaitable<Awaitable<string>> {
  return tweedledum();
}

async function whats_in_the_rabbit_hole(): Awaitable<void> {
  print(await await tweedledee());
}
