<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C<+T> {}

function f<T>(): (C<T>, (function(C<T>): void)) {
  return tuple(new C(), $_ ==> {});
}

function g<T as string>(C<T> $_): void {}

function test(C<int> $z): void {
  $p = f();
  $x = $p[0];
  $f = $p[1];
  g($x);
  $f($z);
}
