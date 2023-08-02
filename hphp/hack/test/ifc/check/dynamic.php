<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C {
  <<__Policied("PRIVATE")>>
  public ~int $priv = 0;

  <<__Policied("PUBLIC")>>
  public ~int $pub = 0;
}

<<__SupportDynamicType>>
class A {
  public int $val = 0;
}

<<__InferFlows>>
function write_b(dynamic $d, C $c): void {
  $c->pub = $d;
}

<<__InferFlows>>
function simple_write(C $c): void {
  write_b($c->priv, $c);
}

<<__InferFlows>>
function write_private(dynamic $d, C $c): void {
  $d->val = $c->priv;
}

<<__InferFlows>>
function write_public(dynamic $d, C $c): void {
  $d->val = $c->pub;
}

<<__InferFlows>>
function read_dyn(dynamic $d): ~int {
  return $d->val;
}

<<__Policied("PUBLIC")>>
function complex_write(A $a, C $c): void {
  // All inputs have public lump
  write_public($a, $c); // ok
  write_private($a, $c); // illegal
}

<<__Policied("PRIVATE")>>
function complex_read(A $a, C $c) : void {
  $c->priv = read_dyn($a); // ok, anything can be stored in priv
  $c->pub = read_dyn($a); // illegal! $a has a private lump
}
