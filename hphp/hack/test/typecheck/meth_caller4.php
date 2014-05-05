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

class X<T> {
  public function foo(T $x): T { return $x; }
}

function use_with_ints((function(X<int>): int) $caller): void {
}

function test(): (function(X<bool>): bool) {
  $caller = meth_caller('X', 'foo');
  use_with_ints($caller);
  return $caller;
}
