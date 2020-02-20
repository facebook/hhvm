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

class A {}
class B extends A {}
class C extends A {}

function test($x): array<int, A> {
  $v = darray[0 => new A(), 1 => new B(), 2 => new C()];
  return $v;
}
