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

class X<T> {
  public function foo(T $x): T { return $x; }
}

function test2(): (function(X<int>, int): int) {
  return meth_caller(X::class, 'foo');
}
