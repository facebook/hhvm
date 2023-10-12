<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  public function foo():void { }
  public function id<T>(T $x):T { return $x; }
  public function empty<T>():vec<T> { return vec[]; }
}
class D {
  public function bar<T,Tu>(T $x, Tu $y):T { return $x; }
}
class E {
  public function bar<T>(T $x, T $y): T { return $x; }
}

function testit(bool $b):void {
  $x = new C();
  $y = $x->id($x);
  $y->foo();

  $f = () ==> $x->empty();
  $x = $f();
  $x[] = new C();
  $x[0]->foo();

  $obj = $b ? new D() : new E();
  $z = $obj->bar(new C(), new C());
  $z->foo();
}
