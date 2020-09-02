<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class A {
}

class C {
  <<Policied("PUBLIC")>>
  public int $value = 0;

  <<Policied("FOO")>>
  public bool $foo = false;

  <<Policied("PUBLIC")>>
  public A $a;

  public function __construct() { $this->a = new A(); }
}

function apply((function(int): int) $f, C $c): void {
  // This is fine since we're inferring flows
  $c->value = $f(0);
}

// Function arguments of cipp function take in cipp
// data and return cipp data

<<Cipp>>
function cipp_apply((function(int): int) $f, int $arg): int {
  return $f($arg);
}

<<Cipp>>
function cipp_apply0((function(int): int) $f, C $c): void {
  // PUBLIC flows into CIPP, the call works because ints
  // are immutable
  $f($c->value);
}

// Functions below trigger errors

<<Cipp>>
function cipp_apply1((function(int): int) $f, C $c): void {
  $c->value = $f(0);
}

<<Cipp>>
function cipp_apply2((function(): void) $f, C $c): void {
  if ($c->foo) {
    $f();
  }
}

<<Cipp>>
function cipp_apply3((function(A): void) $f, C $c): void {
  // Error, $c->a is mutable
  $f($c->a);
}
