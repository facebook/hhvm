<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Inv<T> {
  public function __construct(public (function(T):bool) $item) { }
}
function newInv<T>((function(T):bool) $f):Inv<T> {
  return new Inv($f);
}
function expectBool(bool $ak):void { }
function testit(bool $b):void {
  $obj = new Inv($x ==> { return $b ? $x->foo() : $x[0]; });
  $r = $obj->item;
  expectBool(($r)("a"));
}
