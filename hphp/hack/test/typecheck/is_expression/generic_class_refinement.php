<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

interface I<T> {}
interface J<T> extends I<T> {
  public function f(): T;
}
interface K<T> extends J<T> {}

function f<T>(J<T> $x): T {
  return $x->f();
}

function test<T>(I<T> $x0, I<T> $x1): ?T {
  $y = null;

  if ($x0 is K<_>) {
    $y = f($x0);
  } else {
    if (!($x0 is J<_>)) {
    }
  }

  if ($x1 is K<_>) {
    $y = f($x1);
  } else {
    if (!($x1 is J<_>)) {
    }
  }

  return $y;
}
