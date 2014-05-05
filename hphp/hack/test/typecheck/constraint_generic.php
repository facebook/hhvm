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

interface A {
  public function render(): string;
}

class B implements A {
  public function render(): string { return 'foo'; }
}

function toto(A $x): void {
}


function foo<T as A>(Vector<T> $x): Vector<T> {
  $x[0]->render();
  toto($x[0]);
  return $x;
}

function test_vector(): void {
  $x = foo(Vector {new B()});
}

function foo2<T as A>(T $x): T {
  $x->render();
  return $x;
}


class X {

  public function foo<T as A>(T $x): T {
    $x->render();
    return $x;
  }
}


class Y extends X implements A {
  
  public function test(): Y {
    return $this->foo($this);
  }

  public function render(): string {
    return '0';
  }
}

function test(): Vector<B> {
  $x = new B();
  return foo(Vector {});
}


class L<T as A> {
  private ?T $data = null;
  
  public function set(T $x): void { $this->data = $x; }
  public function get(): ?T { return $this->data; }
}

function test2(): void {
  $x = new L();
  $x->set(new B());
}

class U<T as A> extends L<T> {
  public function get(): ?T { return parent::get(); }
}

function test3(): void {
  $x = new U();
}

function test4<T1, T2, T3 as KeyedIterable<T1, T2>>(T3 $x): void {
  foreach($x as $k => $v) {
    echo $k; echo $v;
  }
}

function test5<T as ?int>(T $x): T {
  return $x;
}
