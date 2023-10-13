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

class I {}
class A extends I {}
class B extends I {}

function foo<T as I>(T $x): void {}

function test(): void {
  if(true) {
    $y = new B();
  }
  else {
    $y = new A();
  }
  foo($y);
}
