<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C {
  <<Policied("A")>>
  public int $a = 0;

  <<Policied("B")>>
    public int $b = 0;
}

function incr_pc(C $c): void {
  if (!($c->a > 0)) {
    $c->a--; // ok
    --$c->a;
    $c->b++; // This is illegal because the PC depends on A
    ++$c->b;
  }
}

function assign_uop(C $c): void {
  // All illegal
  $c->b = ~$c->a;
  $c->b = -$c->a;
}
