<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class A<T> {}
final class B extends A<this::T> {
  const type T = int;
}

function foo(): A<int> {
  return new A<int>();
}
class Something {
  final protected function f(A<int> $it): void {}

  public function foo(): A<int> {
    if (true) {
      $it = foo();
    } else {
      $it = new B();
    }

    $this->f($it);
    return $it;
  }
}
