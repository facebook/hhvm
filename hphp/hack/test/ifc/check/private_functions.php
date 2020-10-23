<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class A {
  <<__Policied("A")>>
  public int $value = 0;
}

<<__Policied>>
function apply(<<__CanCall>> (function(int): int) $f, int $x): void {
  try {
    $f($x);
  } catch (Exception $_) {}
}

<<__InferFlows>>
function throw_private(A $a, Exception $e): void {
  $lambda = $x ==> {
    if ($x > $a->value) {
      throw $e;
    }
    return $x;
  };
  apply($lambda, 123);
}

<<__Policied>>
function no_catch(<<__CanCall>> (function(): void) $f): void {
  // This is illegal because $f could leak info via exceptions
  $f();
}

<<__Policied>>
function use_result(<<__CanCall>> (function(): int) $f): int {
  $x = 0;
  try {
    $x = $f();
  } catch (Exception $_) {}
  // This is illegal because $x is private
  return $x;
}
