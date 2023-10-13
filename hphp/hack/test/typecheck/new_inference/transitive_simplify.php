<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Inv<Tinv> {
  public function __construct(public Tinv $item):void { }
}

class C {
  public function foo():void { }
}

function make_inv<T>(T $x):Inv<T> {
  return new Inv($x);
}

function expectInv<T1>(Inv<T1> $ic):T1 {
  return $ic->item;
}

function testit(Inv<C> $ic): void {
  // So Inv<C> <: v
  // $z : Inv<v>
  $z = make_inv($ic);
  // Now we will generate w
  // and require v<:Inv<w>
  $w = expectInv($z->item);
  // So we have Inv<C> <: v and v <: Inv<w>
  // So we should be able to infer that w=C
  // Thereby checking this call
  $w->foo();
}
