<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class C<T> {}
class Inv<T> {}

function f<T>(Inv<C<T>> $_): void {}

function test(): Inv<C<int>> {
  $x = new Inv();
  f($x);
  return $x;
}
