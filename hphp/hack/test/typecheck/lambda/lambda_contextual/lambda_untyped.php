<?hh // partial
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class Fake {
  abstract public function foo(...$args);
  abstract public function MyMethod($args);
}

class C {
  public function foo((function(int): bool) $f): void {}
}
function makeObj() {
  return new C();
}

function testit(Fake $f): void {
  $x = makeObj();
  $y = $x->foo($z ==> $z + 1);
  $f->MyMethod($w ==> $w - 1);
  $f->foo();
}
