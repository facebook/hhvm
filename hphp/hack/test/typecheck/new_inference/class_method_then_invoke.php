<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  public function foo():void { }
  public static function id<T>(T $x):T { return $x; }
}

function testit():void {
  $x = new C();
  $y = C::id($x);
  $y->foo();
  $a = new C();
  $b = C::id<_>($x);
  $b->foo();
}
