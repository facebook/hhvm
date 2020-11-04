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
async function does_nothing(C $c): Awaitable<void> {
  return;
}

<<__InferFlows>>
async function leak_x (C $c): Awaitable<void> {
  concurrent {
    $x = await plus($c->a, 2);
    await does_nothing($c);
  }
  $c->b = $x; //illegal access
}
