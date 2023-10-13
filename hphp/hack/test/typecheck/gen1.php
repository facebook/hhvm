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

class A {
}

class B {
}

function foo<T as A>(T $x): T {
  return $x;
}

function foo2<T as B>(T $x): void {
  foo($x);
}
