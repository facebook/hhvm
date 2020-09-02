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

<<Cipp>>
function call_cipp(): int {
  return call_inferred(123);
}

<<Cipp>>
function call_inferred(int $x): int {
  // This is allowed
  return plus($x, 2);
}

// Functions below should trigger errors

<<Cipp>>
function leak_value(int $x): void {
  // This leaks $x (and pc) by storing into PUBLIC
  store($x);
}

<<Cipp>>
function leak_pc(): void {
  // This leaks the PC by storing into PUBLIC
  store(123);
}
