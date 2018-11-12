<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class B { }
class C extends B {
  public function foo():void { }
}
class Inv<Tinv> {
  public function __construct(public Tinv $item) { }
}

function make_inv<T>(T $x): (T,Inv<T>) {
  return tuple($x, new Inv($x));
}
function setInvB(Inv<B> $ib):void {
  $ib->item = new B();
}

function bar(C $c): Inv<B> {
  // So: T=v, have C <: v and $x:v and $y:Inv<v>
  list($x, $y) = make_inv($c);
  // Here, how do we "guess" v?
  $x->foo();
//  setInvB($y);
  return $y;
}
