<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class E { }

class D {
  const type T = E::T3;
}
class C {
  const type T2 = D::T;
  public function foo():this::T2 { throw new Exception("E"); }
}
