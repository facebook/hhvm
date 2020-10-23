<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C {
  <<__Policied("PUBLIC")>>
  public int $value = 0;

  <<__InferFlows>>
  public function __construct() {}
}

<<__InferFlows>>
function plus(int $x, int $y): int {
  return $x + $y;
}

<<__InferFlows>>
function store(int $x): void {
  $c = new C();
  $c->value = $x;
}

<<__Policied>>
function call_governed(): int {
  return call_inferred(123);
}

<<__Policied>>
function call_inferred(int $x): int {
  // This is allowed
  return plus($x, 2);
}

// Functions below should trigger errors

<<__Policied>>
function leak_value(int $x): void {
  // This leaks $x (and pc) by storing into PUBLIC
  store($x);
}

<<__Policied>>
function leak_pc(): void {
  // This leaks the PC by storing into PUBLIC
  store(123);
}


<<__Policied("PUBLIC")>>
function public_pc(): void {
  // This does not leak the PC, because the context is public
  store(123);
}
