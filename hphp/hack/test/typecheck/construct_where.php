<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

interface I {
  abstract const type T2;
}

abstract class A<T1 as I, T2> {
  public function __construct(T1 $_) where T2 = T1::T2 {}
}

final class B implements I {
  const type T2 = int;
}

final class C extends A<B, B::T2> {
  public function __construct(B $b) {
    parent::__construct($b);
  }
}
