<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class B { }
class C extends B {
  public function foo():void { }
}
class Inv<Tinv> {
  public function __construct(private Tinv $item) { }
}

function id<T>(T $x): T {
  return $x;
}

function bar(C $c): Inv<B> {
  // So: T=v, have C <: v and $x:v
  // Also, v appears only covariantly in the expression
  $x = id($c);
  // Here, we find a type variable and solve it, because covariant
  // So v:=C
  $x->foo();
  // Here, we must instantiate Tinv=B
  return new Inv($x);
}
