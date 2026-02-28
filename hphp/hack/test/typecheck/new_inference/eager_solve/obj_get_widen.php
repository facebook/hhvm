<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  public function __construct(public int $x) {}
}

class B extends A {}

class C {
  public function __construct(public string $y) {}
}

function apply<Tv1, Tv2>(Tv1 $x, (function(Tv1): Tv2) $f): Tv2 {
  return $f($x);
}

function test(bool $test, B $b, C $c): void {
  $_ = apply(
    $test ? $b : $c,
    $v ==> {
      return $v->x ?? $v->y;
    },
  );
}
