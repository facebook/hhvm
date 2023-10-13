<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {}

function apply<Tv1, Tv2>(Tv1 $x, (function(Tv1): Tv2) $f): Tv2 {
  return $f($x);
}

function test(A $a): void {
  apply($a, $x ==> $x->x);
}
