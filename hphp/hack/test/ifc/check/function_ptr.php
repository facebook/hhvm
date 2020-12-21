<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C {
  <<__Policied("A")>>
  public int $a = 0;

  <<__Policied("B")>>
  public int $b = 0;
}

<<__InferFlows>>
function id(int $x): int {
  return $x;
}

<<__InferFlows>>
function simple_ok(C $c): void {
  $f = id<>;
  $c->a = $f($c->a); // ok, A -> A
}

<<__InferFlows>>
function simple_bad(C $c): void {
  $f = id<>;
  $c->b = $f($c->a); // bad, A -> B
}

<<__InferFlows>>
function write_a(C $c, int $x): void {
  $c->a = $x;
}

<<__InferFlows>>
function write_b(C $c, int $x): void {
  $c->a = $x;
}

<<__InferFlows>>
function write_lit(C $c): void {
  $f = write_a<>;
  $f($c, 123); // ok
}

<<__InferFlows>>
function write_leak(C $c): void {
  $f = write_a<>;
  $f($c, $c->b); // bad, B -> A
}

<<__InferFlows>>
function leak_pc(C $c): void {
  $f = write_a<>;
  if ($c->b > 0) {
    $f($c, 123); // bad, B -> A via the PC
  }
}

<<__InferFlows>>
function leak_data(C $c): void {
  if ($c->b > 0) {
    $f = write_a<>;
  } else {
    $f = write_b<>;
  }
  $f($c, 123); // bad, if A gets written, we know something about B
}

<<__InferFlows>>
function throw_on_a(C $c, Exception $e): void {
  if ($c->a > 0) {
    throw $e;
  }
}

<<__InferFlows>>
function throw_ok(C $c, Exception $e): void {
  $f = throw_on_a<>;
  $f($c, $e);
  $c->a = 1; // ok
}

<<__InferFlows>>
function throw_bad(C $c, Exception $e): void {
  $f = throw_on_a<>;
  $f($c, $e);
  $c->b = 1; // bad, the PC now depends on A
}
