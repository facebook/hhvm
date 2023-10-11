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

trait MyTrait<T as A> {
  protected Vector<T> $a;
  public function test(): T {
    $this->a[0]->test();
    return $this->a[0];
  }
}

class A {
  use MyTrait<A>;

  public function __construct(A $x) {
    $this->a = Vector { $x };
  }
}
