<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Inv<Tinv> {
  public function __construct(public Tinv $item):void { }
}

class B { }

class C extends B {
  public function foo():void { }
}
class D extends B {
}

function make_inv<T>(T $x):Inv<T> {
  return new Inv($x);
}

function expectD(D $d):void {
}
function expectB(B $b):void {
}

function testit(C $c): void {
  // So C <: v
  // $z : Inv<v>
  $z = make_inv($c);
  $a = $z->item;
  // This is benign; we should now have C <: v <: B
  expectB($a);
  // Now we will require v <: D
  // Even though we haven't "solved" for v, transitivity will require that C <: D
  expectD($a);
}
