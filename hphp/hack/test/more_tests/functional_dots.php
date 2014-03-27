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

function f((function(int, ...): string) $g): string {
  return $g(5);
}

function test(): void {
  f(function(int $x, string $y, ...): string { return $x . "<-"; });
}
