<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  public function foo():void { }
}

function testit(C $y):void {
  $x = dict["a" => $y];
  $x["a"]->foo();
}
