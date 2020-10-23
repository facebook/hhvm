<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C {
  <<__Policied("A")>>
  public int $a = 0;

  <<__Policied("B")>>
  public int $b = 0;

}

<<__InferFlows>>
function id<T>(T $x): T {
  return $x;
}

<<__InferFlows>>
function id_int(int $x): int {
  return id($x); // ok
}

<<__InferFlows>>
function write_a_to_b(C $c): void {
  // not ok!
  $c->b = id($c->a);
}

<<__InferFlows>>
function precision_ok(C $c, Exception $e): void {
  $f = ($a, $b) ==> {
    $c->a = $a;
    $c->b = $b;
    return $c->b;
  };

  // ok
  $f($c->a, $c->b);
}

<<__InferFlows>>
function precision_ko(C $c, Exception $e): void {
  $f = ($a, $b) ==> {
    $c->a = $a;
    $c->b = $b;
    return $c->b;
  };

  // calling id on $f leads to imprecision
  $f2 = id($f);
  $f2($c->a, $c->b);
}

<<__InferFlows>>
function apply<T1, T2>((function(T1): T2) $f, T1 $x): T2 {
  return $f($x);
}

<<__InferFlows>>
function a_gets_b(C $c): void {
  $c->a = apply($x ==> $x, $c->b);
}

<<__InferFlows>>
function assign_in_func_ok(C $c): void {
  $f = $x ==> {
    $c->a = $x;
  };
  apply($f, $c->a); // ok
}

<<__InferFlows>>
function assign_in_func_ko(C $c): void {
  $f = $x ==> {
    $c->a = $x;
  };
  apply($f, $c->b); // error
}
