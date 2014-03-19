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

function f(int $x): int { return 0; }

function test(): void {
  $f = function($x) { return $x; };
  $f = function($x) use ($f) { return call_user_func($f, $x); };
  call_user_func(fun(f), 0);
}
