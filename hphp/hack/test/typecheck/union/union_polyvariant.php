<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C {
  public function bar():int { return 3; }
}
class B {
  public function bar():string { return "a"; }
}
class D {
  public function bar():string { return "a"; }
}

// This is rejected
function foo1(Vector<C> $v1, Vector<B> $v2, bool $b):void {
  $v = $b ? $v1 : $v2;
  $xs = $v->filter($x ==> $x->bar() === 5);
}

// This is accepted. Only difference is name of class (D not B)!
function foo2(Vector<C> $v1, Vector<D> $v2, bool $b):void {
  $v = $b ? $v1 : $v2;
  $xs = $v->filter($x ==> $x->bar() === 5);
}
