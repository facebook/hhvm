<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

enum F: int as int {
  A = 0;
}

enum G: int {
  use F;
  B = 1;
}

function foo1() : G {
  return G::A;
}

function foo2() : G {
  return F::A;
}

function foo3() : F {
  return F::A;
}
