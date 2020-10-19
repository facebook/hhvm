<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C {
  <<Policied("PUBLIC")>>
  public shape("x" => int, "y" => string) $pub = shape("x" => 1, "y" => "");

  <<Policied("A")>>
  public shape("y" => string, "x" => int) $a = shape("x" => 1, "y" => "");

  <<Policied("B")>>
  public shape("x" => int, "y" => string) $b = shape("x" => 1, "y" => "");
}

function assign_literal(C $c): void {
  $sh = shape("x" => 123, "y" => "hello world");
  // all ok
  $c->pub = $sh;
  $c->a = $sh;
  $c->b = $sh;
}

function assign_pub(C $c): void {
  // all ok
  $c->a = $c->pub;
  $c->b = $c->pub;
}

function bad_assign(C $c): void {
  // illegal!
  $c->a = $c->b;
  $c->pub = $c->a;
}

<<Governed("PUBLIC")>>
function takes_pub(shape("x" => int, "y" => string) $sh): void {}

<<Governed("A")>>
function takes_a(shape("x" => int, "y" => string) $sh): void {}

function pass_to_pub(C $c): void {
  // ok
  takes_pub($c->pub);

  // illegal
  takes_pub($c->a);
}

function pass_to_a(C $c): void {
  // ok
  takes_a($c->pub);
  takes_a($c->a);

  // illegal
  takes_a($c->b);
}
