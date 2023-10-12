<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

type A = shape(?'a' => int, ?'b' => int, 'c' => int);
type B = shape(?'c' => string);
type C = shape(...);
type D = shape('d' => int);
type E = shape('a' => int, ...);
type F = shape('b' => string, ...);
type G = shape('a' => int, 'b' => string, ...);

function test(A $a, C $c, E $e): void {
  if ($a is B) {
    expect<shape('c' => string)>($a);
    expect<shape('c' => int)>($a);
  }
  if ($a is C) {
    expect<A>($a);
  }
  if ($a is D) {
    expect<nothing>($a);
  }
  if ($e is F) {
    expect<G>($e);
  }
}

function expect<T>(T $_): void {}
