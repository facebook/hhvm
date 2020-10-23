<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C {
  <<__Policied("A")>>
  public int $a = 0;

  <<__Policied("B")>>
  public int $b = 0;
}

<<__InferFlows>>
function apply((function(C): int) $f, C $c): void {
  $x = $f($c);
  $c->a = $x;
}

<<__InferFlows>>
function store_a_in_a(C $c): void {
  $f = $x ==> $x->a;
  apply($f, $c);
}

<<__InferFlows>>
function store_b_in_a(C $c): void {
  $f = $x ==> $x->b;
  apply($f, $c);
}

<<__InferFlows>>
function store_b_in_a2(C $c): void {
  $f = $x ==> {
    if ($x->b > 0) {
      return 1;
    } else {
      return 0;
    }
  };
  apply($f, $c);
}

<<__InferFlows>>
function leak_pc(C $c): void {
  if ($c->a > 0) {
    $f = () ==> 0;
  } else {
    $f = () ==> 1;
  }
  $c->b = $f();
}

<<__InferFlows>>
function lambda_throw(C $c, Exception $e): void {
  $f = (int $x): void ==> {
    if ($x > 0) {
      throw $e;
    }
  };
  try {
    $f($c->a);
  } catch (Exception $_) {
    // The PC is tainted by A
    $c->b = 99;
  }
}

<<__InferFlows>>
function apply_lambda(C $c): void {
  $c->a = ($x ==> $x)($c->b);
}

<<__InferFlows>>
function apply_var(C $c): void {
  $id = $x ==> $x;
  $c->a = $id($c->b);
}
