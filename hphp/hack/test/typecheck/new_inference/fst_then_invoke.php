<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class B { }
class C extends B {
  public function foo():void { }
}
class D extends B {
  public function foo():void { }
}
class Inv<Tinv> {
  public function __construct(private Tinv $item) { }
}

function fst<T>(T $x, T $y): T {
  return $x;
}

function bar(C $c, D $d): Inv<B> {
  // Here, let $x:v and C<:v and D<:v
  $x = fst($c, $d);
  // Here, we find v:=C|D
  $x->foo();
  // Here, we must instantiate Tinv=B
  return new Inv($x);
}
