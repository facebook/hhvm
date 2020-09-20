<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class B { }
class C extends B {
  public static function foo(): void { }
}
class Inv<Tinv> {
  public function __construct(private Tinv $item) { }
}

function id<T>(T $x): T {
  return $x;
}

function bar(C $c): void {
  // So: T=v, have classname<C> <: v and $x:v
  $x = id(C::class);
  // Here, how do we "guess" that v = classname<C>?
  $x::foo();
}
