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

function f((function(T): T) $g, T $x): T {
  return call_user_func($g, $x);
}

function test(): void {
  f(function($x) { return $x + 1.0; }, 0);
}
