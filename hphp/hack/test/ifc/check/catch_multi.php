<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class X {
  <<Policied("A")>>
  public int $a = 0;

  <<Policied("B")>>
  public int $b = 1;
}

class E1 extends Exception {}
class E2 extends Exception {}

function no_catch_all(X $x, E2 $e): void {
  try {
    if ($x->a > 0) throw $e;
  } catch (E1 $_) {}
  // This leaks A via the PC
  $x->b = 0;
}

function store_in_finally(X $x, E1 $e): void {
  try {
    if ($x->a > 0) throw $e;
  } finally {
    // This is ok because finally runs regardless of exceptions
    $x->b = 123;
  }
}

function return_in_try(X $x, E1 $e): void {
  try {
    if ($x->a > 0) return;
  } finally {
    // This is ok
    $x->b = 123;
  }
}

function multi_catch(X $x, E1 $e): void {
  try {
    if ($x->a > 0) throw $e;
  } catch (E1 $_) {
    $x->b = 1;
  } catch (E2 $_) {
    $x->b = 2;
  }
}

function no_catch(X $x, E1 $e, bool $b): void {
  try {
    $val = 0;
    if ($b) {
      $val = $x->a;
      throw $e;
    }
    $val = $x->b;
  } finally {
    // This should be an error
    $x->b = $val;
  }
}

function throw_in_catch(X $x, E1 $e): void {
  try {
    if ($x->a > 0) throw $e;
  } catch (Exception $e) {
    throw $e;
  } finally {}
  // This leaks the pc because we rethrow the exception
  $x->b = 123;
}

function local_bindings(bool $b, Exception $e): int {
  try {
    try {
      if ($b) {
        $x = 0;
        throw $e;
      }
    } catch (E1 $_) {
    }
  } catch (Exception $_) {
    return $x;
  }
  return 0;
}

function unreachable_assign(X $x, Exception $e): void {
  $val = 0;
  try {
    throw $e;
  } catch (Exception $_) {
    $val = $x->a;
    throw $e;
    $val = 0;
  } finally {
    // This leaks A into B
    $x->b = $val;
  }
}
