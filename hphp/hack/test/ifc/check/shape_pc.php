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
function pc_ok(C $c): void {
  $s = $c->a;

  if ($c->b["x"] > 0) {
    // Only field "x" depends on the PC
    $s["x"] = 1;
  }

  $c->a["y"] = $s["y"]; // ok
}

<<__InferFlows>>
function pc_bad(C $c): void {
  $s = $c->a;

  if ($c->b["x"] > 0) {
    // Only field "x" depends on the PC
    $s["x"] = 1;
  }

  $c->a["x"] = $s["x"]; // error
}
