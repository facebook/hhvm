<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C {
  <<__Policied("A")>>
  public int $a = 0;
  <<__Policied("A")>>
  public mixed $a_mixed = 0;

  <<__Policied("B")>>
  public int $b = 0;
  <<__Policied("B")>>
  public mixed $b_mixed = 0;

  <<__Policied("PRIVATE")>>
  public mixed $priv = 0;

  <<__Policied("PUBLIC")>>
  public mixed $pub = 0;
}

<<__InferFlows>>
function get_x(shape(...) $s): mixed {
  return Shapes::idx($s, "x");
}

<<__InferFlows>>
function get_optional_x(shape(?"x" => int) $s): mixed {
  return Shapes::idx($s, "x");
}

<<__InferFlows>>
function write_a_to_a(C $c): void {
  $s = shape("x" => $c->a);
  // all ok
  $c->a_mixed = Shapes::idx($s, "x");
  $c->a_mixed = get_x($s);
}

<<__InferFlows>>
function write_a_to_b(C $c): void {
  $s = shape("x" => $c->a);
  // leak!
  $c->b_mixed = get_x($s);
}

<<__InferFlows>>
function write_a_to_b2(C $c): void {
  $s = shape("x" => $c->b, "y" => $c->a);
  // Even though "x" has policy B, A also flows into the shape's lump
  $c->b_mixed = get_x($s);
}

<<__InferFlows>>
function write_private(C $c): void {
  $s = shape("x" => $c->b, "y" => $c->a);
  // ok, because the covariant ints can flow to private
  $c->priv = get_x($s);
}

<<__InferFlows>>
function write_private_bad(C $c): void {
  $s = shape("x" => $c->b_mixed, "y" => $c->a_mixed);
  // bad, because the mixed values have invariant policies
  $c->priv = get_x($s);
}

<<__InferFlows>>
function optional_leaks_pc(C $c): void {
  $s = shape();
  if ($c->a > 0) {
    $s["x"] = 1;
  }
  $c->b_mixed = Shapes::idx($s, "x");
}

<<__InferFlows>>
function default_leaks(C $c): void {
  $s = shape();
  if ($c->a > 0) {
    $s["x"] = 1;
  }
  // B leaks to A because "x" might not be defined
  $c->a_mixed = Shapes::idx($s, "x", $c->b);
}

<<__InferFlows>>
function default_no_leak(C $c): void {
  $s = shape("x" => 1);
  // We consider this a leak because the analysis is conservative
  $c->a_mixed = Shapes::idx($s, "x", $c->b);
}

<<__InferFlows>>
function get_public_null(C $c): void {
  // ok, the shape has no non-public data
  $c->pub = get_x(shape());
  $c->pub = get_optional_x(shape());
}

<<__InferFlows>>
function non_public_null(C $c): void {
  $s = shape();
  if ($c->a > 0) {
    $s["x"] = 1;
  }
  // Illegal! The optional field has info about the PC
  $c->pub = get_optional_x($s);
}
