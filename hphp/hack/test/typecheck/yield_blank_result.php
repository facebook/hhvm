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

async function r(): Awaitable<int> {
  return 123;
}

async function r_nullable(): Awaitable<?int> {
  return null;
}

async function r_void(): Awaitable<void> {
  return;
}

async function rstring(): Awaitable<string> {
  return 'string';
}

async function wfr(): Awaitable<int> {
  return await r();
}

async function wfr_nullable(): Awaitable<?int> {
  return await r_nullable();
}

async function wfr_void(): Awaitable<void> {
  return await r_void();
}
