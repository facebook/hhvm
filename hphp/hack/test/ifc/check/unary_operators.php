<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C {
  <<__Policied("A")>>
  public int $a = 0;

  <<__Policied("B")>>
    public int $b = 0;
}

<<__InferFlows>>
function incr_pc_ok1(C $c): void {
  if (!($c->a > 0)) {
    $c->a--; // ok
  }
}

<<__InferFlows>>
function incr_pc_ok2(C $c): void {
  if (!($c->a > 0)) {
    --$c->a; // ok
  }
}

<<__InferFlows>>
function incr_pc_ko1(C $c): void {
  if (!($c->a > 0)) {
    $c->b++; // This is illegal because the PC depends on A
  }
}

<<__InferFlows>>
function incr_pc_ko2(C $c): void {
  if (!($c->a > 0)) {
    ++$c->b; // This is illegal because the PC depends on A
  }
}

<<__InferFlows>>
function assign_uop_ko1(C $c): void {
  // illegal
  $c->b = ~$c->a;
}

<<__InferFlows>>
function assign_uop_ko2(C $c): void {
  // illegal
  $c->b = -$c->a;
}
