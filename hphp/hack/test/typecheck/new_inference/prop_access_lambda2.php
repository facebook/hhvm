<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  public function __construct(public int $x) {}
}

function apply<Tv1, Tv2>(Tv1 $x, (function(Tv1): Tv2) $f): Tv2 {
  return $f($x);
}

function test(A $a): void {
  apply($a, $x ==> expect<string>($x->x));
}

function expect<T>(T $_): void {}
