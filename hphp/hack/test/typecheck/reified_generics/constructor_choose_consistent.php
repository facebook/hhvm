<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class AA {
  public function __construct(public float $f) {}
}

<<__ConsistentConstruct>>
class A {
  public function __construct(public int $i, public string $s) {}
}

// It is currently impossible to write a type that satisfies T, but we still
// need to choose the constructor from A because only A is annotated with
// <<__ConsistentConstruct>>
function f<<<__Newable>> reify T as AA as A>(): void {
  $local = new T(2, "arguments");
}
