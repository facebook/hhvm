<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class B { }
class C extends B {
  public function foo():void { }
}
class Inv<Tinv> {
  public function __construct(public Tinv $item) { }
}

function make_inv<T>(T $x): Inv<T> {
  return new Inv($x);
}
function setInvB(Inv<B> $ib):void {
  $ib->item = new B();
}

function bar(C $c): Inv<B> {
  // So: T=v, have C <: v and $x:Inv<v>
  $x = make_inv($c);
  // Here, how do we "guess" v?
  $x->item->foo();
//  setInvB($x);
  return $x;
}
