<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class D {
  public function foo(): void { }
}
class C {
  public ?D $d;
  public function __construct() { }
}
function testit():void {
  $c1 = new C();
  $c2 = new C();
  if ($c1->d is nonnull) {
    $c1 = $c2;
    $c1->d->foo();
  }
}
