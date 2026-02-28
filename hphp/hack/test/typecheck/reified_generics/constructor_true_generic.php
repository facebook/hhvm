<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

<<__ConsistentConstruct>>
class A {
  public function __construct(public int $i, public string $s) {}
}

interface I {
  public function f(): int;
}

class B extends A implements I {
  public function f(): int {
    return 4;
  }
}

function f<<<__Newable>> reify T as A as I>(T $param): void {
  $param->f();
  $local = new T(42, "right");
  $local->f();
}

function g<<<__Newable>> reify T as A>(): void {
  $local = new T(42, "right");
  $local->f(); // error
}
