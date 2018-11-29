<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  public function foo():void { }
  public function id<T>(T $x):T { return $x; }
}

function testit():void {
  $x = new C();
  $y = $x->id($x);
  $y->foo();
}
