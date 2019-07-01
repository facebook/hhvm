<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C<T as arraykey> {}

function f(mixed $x): void {
  $x as C<_>;
  hh_show($x);
}
