<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  public function foo():void { }
}
class D {
  public function foo():void { }
}

function testit(C $y, D $z):void {
  $x = dict["a" => $y, "b" => $z];
  $x["a"]->foo();
}
