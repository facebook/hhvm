<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  public function foo():void { }
}

function testit():void {
  $x = vec[new C()];
  $y = array_filter($x, $c ==> true);
  $y[0]->foo();
}
