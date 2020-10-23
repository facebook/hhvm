<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C {
  <<__Policied("A")>>
  public int $a = 0;

  <<__Policied("B")>>
  public int $b = 0;
}

<<__InferFlows>>
function f(C $x, Exception $e): void {
  if ($x->a > 0) {
    throw $e;
  }
}

<<__InferFlows>>
function f2(C $x, Exception $e): void {
  f($x, $e);
}

<<__InferFlows>>
function g(C $x, Exception $e): void {
  try {
    // f() may throw depending on the the value A
    f($x, $e);
  } catch (Exception $_) {
    // If f() threw, then the PC here is dependent on A
    $x->b = 1234;
  }
}

<<__InferFlows>>
function g2(C $x, Exception $e): void {
  try {
    // f2() may throw depending on the the value A
    f2($x, $e);
  } catch (Exception $_) {
    // If f() threw, then the PC here is dependent on A
    $x->b = 1234;
  }
}
