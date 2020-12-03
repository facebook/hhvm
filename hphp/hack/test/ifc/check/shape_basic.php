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
  $sh = shape("x" => 123, "y" => "hello world");
  // all ok
  $c->pub = $sh;
  $c->a = $sh;
  $c->b = $sh;
}

<<__InferFlows>>
function assign_pub(C $c): void {
  // all ok
  $c->a = $c->pub;
  $c->b = $c->pub;
}

<<__InferFlows>>
function bad_assign_ko1(C $c): void {
  // illegal!
  $c->pub = $c->a;
}

<<__InferFlows>>
function bad_assign_ko2(C $c): void {
  // illegal!
  $c->a = $c->b;
}

<<__Policied("PUBLIC")>>
function takes_pub(shape("x" => int, "y" => string) $sh): void {}

<<__Policied("A")>>
function takes_a(shape("x" => int, "y" => string) $sh): void {}

<<__InferFlows>>
function pass_to_pub_ok(C $c): void {
  // ok
  takes_pub($c->pub);
}

<<__InferFlows>>
function pass_to_pub_ko(C $c): void {
  // illegal
  takes_pub($c->a);
}

<<__InferFlows>>
function pass_to_a_ok(C $c): void {
  // ok
  takes_a($c->pub);
  takes_a($c->a);
}

<<__InferFlows>>
function pass_to_a_ko(C $c): void {
  // illegal
  takes_a($c->b);
}

<<__Policied("PUBLIC")>>
function id(shape("x" => int) $s): shape("x" => int) {
  return $s;
}
