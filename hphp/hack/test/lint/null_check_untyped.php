<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

interface I<+T> {
  public function f(): T;
}

function f<T>(I<T> $x): T {
  return $x->f();
}

function test($untyped_value): nonnull {
  $x = f($untyped_value);
  if ($x !== null) {
    return $x;
  }
  return false;
}
