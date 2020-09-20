<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

final class C {
  const type T = int;

  public function f(this::T $x): void {
    hh_show($x);
  }
}
