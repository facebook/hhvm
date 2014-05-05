<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

class A {}
class B extends A {}

function foobar2(Vector<B> $x): void {}

function foobar1() {
  $v = Vector {};
  $v[] = new B();
  foobar2($v);
  $v[] = new A();
} 
