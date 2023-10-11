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
class UnrelatedClass {}

function foo<T as I>(T $x): void {}

function test(bool $b): void {
  if ($b) {
    $y = new B();
  } else {
    $y = new UnrelatedClass();
  }
  foo($y);
}
