<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C {
  <<Policied("A")>>
  public int $a = 0;

  <<Policied("B")>>
  public int $b = 0;
}

async function plus(int $x, int $y): Awaitable<int> {
  return $x + $y;
}

async function store_a_in_b (C $c): Awaitable<void> {
  $c->b = await plus($c->a, 2);
}

async function store_a_in_b_lambda (C $c): Awaitable<void> {
  $f = async $x ==> {
    $c->b = $x;
  };
  await $f($c->a);
}
