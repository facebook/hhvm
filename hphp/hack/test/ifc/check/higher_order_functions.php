<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class A {
}

class C {
  <<__Policied("PUBLIC")>>
  public int $value = 0;

  <<__Policied("FOO")>>
  public bool $foo = false;

  <<__Policied("PUBLIC")>>
  public A $a;

  <<__InferFlows>>
  public function __construct() { $this->a = new A(); }
}

<<__InferFlows>>
function apply_ok((function(int): int) $f, C $c): void {
  // This is fine since we're inferring flows
  $c->value = $f(0);
}

// Function arguments of cipp function take in cipp
// data and return cipp data

<<__Policied>>
function apply((function(int): int) $f, int $arg): int {
  return $f($arg);
}

<<__Policied>>
function apply0((function(int): int) $f, C $c): void {
  // PUBLIC flows into CIPP, the call works because ints
  // are immutable
  $f($c->value);
}

// Functions below trigger errors

<<__Policied>>
function apply1((function(int): int) $f, C $c): void {
  $c->value = $f(0);
}

<<__Policied>>
function apply2((function(): void) $f, C $c): void {
  if ($c->foo) {
    $f();
  }
}

<<__Policied>>
function apply3((function(A): void) $f, C $c): void {
  // Error, $c->a is mutable
  $f($c->a);
}
