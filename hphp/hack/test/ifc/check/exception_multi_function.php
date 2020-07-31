<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C {
  <<Policied("A")>>
  public int $a = 0;

  <<Policied("B")>>
  public int $b = 0;
}

function f(C $x, Exception $e): void {
  if ($x->a > 0) {
    throw $e;
  }
}

function f2(C $x, Exception $e): void {
  f($x, $e);
}

function g(C $x, Exception $e): void {
  try {
    // f() may throw depending on the the value A
    f($x, $e);
  } catch (Exception $_) {
    // If f() threw, then the PC here is dependent on A
    $x->b = 1234;
  }
}

function g2(C $x, Exception $e): void {
  try {
    // f2() may throw depending on the the value A
    f2($x, $e);
  } catch (Exception $_) {
    // If f() threw, then the PC here is dependent on A
    $x->b = 1234;
  }
}
