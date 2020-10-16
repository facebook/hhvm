<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class X {
  <<Policied("A")>>
  public int $a = 0;

  <<Policied("B")>>
  public int $b = 1;
}

function leak_pc_in_catch(X $x, Exception $e): void {
  try {
    if ($x->a > 0)
      throw $e;
  } catch (Exception $exn) {
    // This leaks A via the pc
    $x->b = 12;
  }
}

function do_not_leak_pc_after_catch(X $x, Exception $e): void {
  try {
    if ($x->a > 0) throw $e;
  } catch (Exception $_) {}
  // We will reach this code whether or not the exception was thrown.
  // so there is no violation
  $x->b = 1;
}

function nested_try(X $x, Exception $e): void {
  try {
    if ($x->a > 0) throw $e;
    try {} catch (Exception $_) {}
  } catch (Exception $_) {
    // This should leak A via the PC
    $x->b = 0;
  }
}

function throw_in_catch(X $x, Exception $e): void {
  try {
    throw $e;
  } catch (Exception $_) {
    if ($x->a > 0) throw $e;
  }
  $x->b = 1;
}
