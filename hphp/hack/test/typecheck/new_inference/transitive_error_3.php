<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Inv<T> {}

function inv<T>(): Inv<T> {
  return new Inv();
}

function f<T as string>(): (function(Inv<T>): void) {
  return $_ ==> {};
}

function test(): Inv<int> {
  $x = inv();
  $f = f();
  $f($x);
  return $x;
}
