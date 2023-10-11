<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {}
class B extends A {}
class C extends A {}
class Foo<T> {
  public function bar<Tu super T>(Tu $x): Tu {
    return $x;
  }
}
function f(Foo<C> $x, B $y): A {
  return $x->bar($y);
}
