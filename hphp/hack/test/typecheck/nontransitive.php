<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

interface I<-Ti> { }
class B { }
class C extends B implements I<B> { }
class D<T as C> {
  public function fromTtoC(T $x):C {
    return $x;
  }
  public function fromCtoIT(C $c):I<T> {
    return $c;
  }
  public function fromTtoIT(T $x):I<T> {
    return $x;
  }
}
