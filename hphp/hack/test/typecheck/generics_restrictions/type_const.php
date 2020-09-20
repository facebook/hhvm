<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

interface I {
  const type TThis = this;
}

class BrokenRef<+T as I> {
  public function __construct(private T $value) {}
  public function get<Tu>(): Tu where Tu = T::TThis {
    return $this->value;
  }
  public function set<Tu>(Tu $value): void where Tu = T::TThis {
    $this->value = $value;
  }
}

function swap(BrokenRef<A> $r1, BrokenRef<A> $r2): void {
  $v1 = $r1->get();
  $r1->set($r2->get());
  $r2->set($v1);
}

class A implements I {}

class B extends A {
  public function f(): void {}
}

<<__EntryPoint>>
function test(): void {
  $r1 = new BrokenRef(new A());
  $r2 = new BrokenRef(new B());
  swap($r1, $r2);
  $r2->get()->f();
}
