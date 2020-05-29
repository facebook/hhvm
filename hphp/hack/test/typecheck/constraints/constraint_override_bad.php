<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class B {
  public function foo<T super C>(): T {
    return new C();
  }
}
class C extends B {
  public function foo<T super B>(): T {
    return new B();
  }
  public function bar(): void {
    echo 'bar';
  }
}

function CallOnB(B $b): void {
  $x = $b->foo();
  $x->bar();
}
function TestIt(): void {
  $c = new C();
  CallOnB($c);
}

function main(): void {
  TestIt();
}
