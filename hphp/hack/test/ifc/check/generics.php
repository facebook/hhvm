<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C {
  <<Policied("A")>>
  public int $a = 0;

  <<Policied("B")>>
  public int $b = 0;

}

function id<T>(T $x): T {
  return $x;
}

function id_int(int $x): int {
  return id($x); // ok
}

function write_a_to_b(C $c): void {
  // not ok!
  $c->b = id($c->a);
}

function precision_loss(C $c, Exception $e): void {
  $f = ($a, $b) ==> {
    $c->a = $a;
    $c->b = $b;
    return $c->b;
  };
  // ok
  $f($c->a, $c->b);

  // calling id on $f leads to imprecision
  $f2 = id($f);
  $f2($c->a, $c->b);
}

function apply<T1, T2>((function(T1): T2) $f, T1 $x): T2 {
  return $f($x);
}

function a_gets_b(C $c): void {
  $c->a = apply($x ==> $x, $c->b);
}

function assign_in_func(C $c): void {
  $f = $x ==> {
    $c->a = $x;
  };
  apply($f, $c->a); // ok
  apply($f, $c->b); // error
}
