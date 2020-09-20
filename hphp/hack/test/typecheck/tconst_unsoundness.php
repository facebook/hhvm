<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

abstract class A {
  abstract const type T;
}

abstract class B {
  abstract const type Ta as A;
}

function f<Tb as B, T>(?Tb $_): void where Tb::Ta::T = T {}

function g<Tb as B, Tin, Tout>(?Tb $b, Tin $x): Tout where Tb::Ta::T = Tin {
  f($b);
  return $x;
}

function unsafe_coerce<Tin, Tout>(Tin $x): Tout {
  return g(null, $x);
}
