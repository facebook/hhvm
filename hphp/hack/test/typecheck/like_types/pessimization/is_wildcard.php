<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C<T as arraykey> {}

function f(mixed $x): void {
  if ($x is C<_>) {
    // We expect $x to be C<T#1> where T#1 has the same bounds
    // as T in C's declaration
    hh_show($x);
  }
}
