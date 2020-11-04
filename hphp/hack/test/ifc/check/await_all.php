<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C {
  <<__Policied("A")>>
  public int $a = 0;

  <<__Policied("B")>>
  public int $b = 0;
}

<<__InferFlows>>
async function plus(int $x, int $y): Awaitable<int> {
  return $x + $y;
}

<<__InferFlows>>
async function illegal_access(C $c): Awaitable<void> {
  $c->a = $c->b;
  return;
}

<<__InferFlows>>
async function store_a_in_b (C $c): Awaitable<void> {
  concurrent {
    $c->b = await plus($c->a, 2); // illegal access
    $c->a = await plus($c->a, 3); // ok
    await illegal_access($c); //illegal access
  }
}
