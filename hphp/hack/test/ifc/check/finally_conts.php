<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class X {
  <<Policied("A")>>
  public int $a = 0;

  <<Policied("B")>>
  public int $b = 1;
}

function f(X $x, Exception $e, bool $b): void {
  $z = 0;
  try {
    try {
      $y = 0;
      throw $e;
    } catch (Exception $_) {
      if ($b) {
        // In the catch continuation, $y depends on A
        $y = $x->a;
        throw $e;
      }
      $y = 2;
    } finally {
      $z = $y;
    }
  } catch (Exception $_) {
    // Error! In this flow, $z depends on A
    $x->b = $z;
    $z = 0;
  }
  // This is ok; in the Next continuation, $z does not depend on A
  $x->b = $z;
}

function g(X $x, Exception $e): void {
  try {
    try {
      if ($x->a > 0) throw $e;
    } finally {
      // noop
    }
  } catch (Exception $_) {
    $x->b = 0;
  }
}
