<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C {
  <<Policied("PUBLIC")>>
  public int $value = 0;

  public function __construct() {}
}

<<InferFlows>>
function plus(int $x, int $y): int {
  return $x + $y;
}

<<InferFlows>>
function store(int $x): void {
  $c = new C();
  $c->value = $x;
}

<<Governed>>
function call_governed(): int {
  return call_inferred(123);
}

<<Governed>>
function call_inferred(int $x): int {
  // This is allowed
  return plus($x, 2);
}

// Functions below should trigger errors

<<Governed>>
function leak_value(int $x): void {
  // This leaks $x (and pc) by storing into PUBLIC
  store($x);
}

<<Governed>>
function leak_pc(): void {
  // This leaks the PC by storing into PUBLIC
  store(123);
}


<<Governed("PUBLIC")>>
function public_pc(): void {
  // This does not leak the PC, because the context is public
  store(123);
}
