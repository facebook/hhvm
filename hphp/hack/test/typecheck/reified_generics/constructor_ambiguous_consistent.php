<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

<<__ConsistentConstruct>>
class A {
  public function __construct() {}
}

<<__ConsistentConstruct>>
class B {
  public function __construct(public float $f) {}
}

// Ignoring the multiple consistent constraint errors for now,
// just observing the effect on new

function f<<<__Newable>> reify T as A as B>(): void {
  $local = new T();
}

function g<<<__Newable>> reify T as B as A>(): void {
  $local = new T(3.14);
}
