<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Test {
  const type Ta = A::Tx;
  const type Tb = A::Ty;
  const type Tcc = B::Tyy;

  public function f(): void {
    3 as this::Ta;
    4 as this::Tb;
    6 as this::Tcc;
  }
}

abstract class A {
  abstract const type Tx as X;
  const type Ty = Y;
}

class B extends A {
  const type Tx = Y;
  const type Tyy = YY<int>;
}

abstract class X {}
class Y extends X {}
class YY<T> extends Y {}
