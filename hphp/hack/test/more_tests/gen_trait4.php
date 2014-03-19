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

trait MyTrait<T> {
  protected Vector<T> $a;
}

class B { }

function takesVectorA(Vector<A> $x): void {}

class A extends B {
  use MyTrait<B>;


  public function __construct() {
    $a = Vector {new A()};
    $this->a = $a;
    takesVectorA($a);
  }
}
