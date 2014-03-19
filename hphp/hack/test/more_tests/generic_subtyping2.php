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

class I {}
class A extends I {}
class B extends I {}
class UnrelatedClass {}

function foo<T as I>(T $x): void {}

function test(): void {
  if(true) {
    $y = new B();
  }
  else {
    $y = new UnrelatedClass();
  }
  foo($y);
}
