<?hh // strict
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

class MyPhonyVector<T> {
  private Vector<T> $x;
  
  public function __construct() {
    $this->x = Vector {};
  }

  public function add(T $x): void {
    $this->x[] = $x;
  }

  public function get(int $x): T {
    return $this->x[0];
  }
}

class X {}
class A extends X {}
class B extends X {}

function test(MyPhonyVector<X> $v): void {
  $x = new MyPhonyVector();
  $x->add(new B());
  $x->add(new A());
  test($x);
}
