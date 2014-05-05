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

function get_pair(): Pair<string, bool> {
  return Pair {'foo', true};
}

function use_pair(Pair<mixed, mixed> $p): void {
}

function test(): void {
  $p = get_pair();
  use_pair($p);
}
