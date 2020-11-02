<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C {
  <<__Policied("PUBLIC")>>
  public shape("x" => int, "y" => string) $pub = shape("x" => 1, "y" => "");

  <<__Policied("A")>>
  public shape("y" => string, "x" => int) $a = shape("x" => 1, "y" => "");

  <<__Policied("B")>>
  public shape("x" => int, "y" => string) $b = shape("x" => 1, "y" => "");
}

<<__InferFlows>>
function assign_literal(C $c): void {
  // ok because literals are public
  $c->a["x"] = 1;
  $c->b["x"] = 2;
}

<<__InferFlows>>
function assign_pc(C $c): void {
  if ($c->a["x"] > 0) {
    // illegal because pc depends on A
    $c->b["y"] = "";
  }
}

<<__InferFlows>>
function ok_assign(C $c): void {
  // ok
  $c->a["x"] = $c->pub["x"];
}

<<__InferFlows>>
function bad_assign(C $c): void {
  // illegal!
  $c->a["y"] = $c->b["y"];
}

<<__InferFlows>>
function cow_ok(C $c): void {
  $alias = $c->pub;

  // ok because shapes are CoW
  $alias["x"] = $c->a["x"];
  $alias["y"] = $c->b["y"];
}
