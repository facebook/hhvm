<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Test {
  const type Ta = A::Tx;
  const type Tb = A::Ty;
  const type Tc = A::Tyy; // even though this is partially abstract, it is referenced by class, so it is fine
  const type Tcc = B::Tyy;

  public function f(): void {
    3 as this::Ta;
    4 as this::Tb;
    5 as this::Tc;
    6 as this::Tcc;
  }
}

abstract class A {
  abstract const type Tx as X;
  const type Ty = Y;
  const type Tyy as X = Y;
}

class B extends A {
  const type Tx = Y;
  const type Tyy = YY<int>;
}

abstract class X {}
class Y extends X {}
class YY<T> extends Y {}
