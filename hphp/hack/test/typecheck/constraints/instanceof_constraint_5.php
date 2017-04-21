<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class B {}
function foo<T as B>(T $x): void {}
class C<T> {}
class D extends C<B> {}
function test<T>(C<T> $x, T $y): void {
  if ($x instanceof D) {
    foo($y);
  }
}
