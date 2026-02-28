<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Inv<T> {
  public function __construct(public T $item) { }
}

class C {
  public function bar():void { }
  public function foo():int { return 3; }
}

function test_refine_nullable(
  ?C $c,
): void {
  // Turns a concrete type into a lower bound on a Tvar
  $c = (new Inv($c))->item;
  if ($c !== null) {
    $c->bar();
  }
  $c?->bar();
  $r = $c === null ? 4 : $c->foo();
}
