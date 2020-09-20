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

class Option<T> {
}

class X<T> {
  private T $x;

  public function __construct(T $x) {
    $this->x = $x;
  }

  public function get(): T {
    return $this->x;
  }

}

class Y<T> extends X<Option<T>> {

  public function __construct() {
    parent::__construct(null);
  }

  public function get(): Option<T> {
    return parent::get();
  }
}

class A {
  public function f(): int { return 0; }
}

class B extends A {
  public function g(): int { return 0; }
}

function useA(A $x): void {
}

function test(): void {
  useA(new B());
}

class C {
  public function f(B $x): A { return new A(); }
}

class D extends C {
  public function f(A $x): B { return new B(); }
}

class Z<T1, T2> extends Y<Z<T2, T1>> {
}

class G extends Z<int, float> {
}
