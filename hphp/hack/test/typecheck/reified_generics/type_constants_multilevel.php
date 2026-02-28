<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class Test {
  abstract const type Ta as A;
  const type Tb = B;

  public function f1(): void {
    3 as this::Ta::Tx;
    4 as this::Ta::Ty;
    5 as this::Ta::Tz;
  }

  // This shows that only the last type constant is relevant. It doesn't matter
  // that Tb is enforceable, only that Tw, Tx, Tz are not enforceable
  public function f2(): void {
    3 as this::Tb::Tx;
    4 as this::Tb::Ty;
    5 as this::Tb::Tz;
  }
}

abstract class A {
  abstract const type Tx as X;
  const type Ty = Y;
  const type Tz = Z<int>;
}

class B extends A {
  // this shows bad cases that can be assigned to abstract type consts
  const type Tx = Z<int>;
  const type Tw = YY<int>;
}

abstract class X {}
class Y extends X {}
class Z<T> extends X {}

class YY<T> extends Y {}
