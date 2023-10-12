<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class Test {
  abstract const type Ta as A;
  const type Tb = B;
  const type Tc = C<int>;

  public function f(): void {
    3 as this::Ta;
    4 as this::Tb;
    5 as this::Tc;
  }
}

abstract class A {}
class B extends A {}
class C<T> extends A {}
