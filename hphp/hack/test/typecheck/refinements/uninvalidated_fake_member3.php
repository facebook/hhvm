<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class D {
  public int $foo = 42;
}
class C {
  public ?D $d;
  public function __construct() { }
}
function bar(): void { }

function testit():void {
  $c1 = new C();
  $c2 = new C();
  $c3 = new C();
  if ($c1->d is nonnull) {
    bar();
    $c2 = $c3;
    // The following should fail because of bar()
    $c1->d->foo;
  }
}
