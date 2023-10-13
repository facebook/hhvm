<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Cov<+T> { }

class C<+T> {
  public function foo<Tu>(Tu $x):Tu where Cov<Tu> super Cov<T> {
    return $x;
  }
}
