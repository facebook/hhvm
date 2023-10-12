<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

interface A {}
class B {
  public function f(): void {}
}

function mymap<T1, T2>(
  Traversable<T1> $x,
  (function (T1): T2) $f,
) : vec<T2> {
  return vec[];
}

function f(vec<A> $as): void {
  $bs = mymap($as, $x ==> $x as B);
  $bs[0]->f();
}
