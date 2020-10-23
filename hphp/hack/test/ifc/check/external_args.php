<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class A {
  public int $value = 0;
  <<__InferFlows>>
  public function __construct() {}
}

class B {
  public A $a;

  <<__InferFlows>>
  public function __construct() { $this->a = new A(); }
}

<<__Policied>>
function read_external(<<__External>> A $x, A $y) : void {
  // Ok! External flows into the function
  $y->value = $x->value;
}

<<__Policied>>
function write_external(<<__External>> A $x, A $y) : void {
  // Illegal! Implicit does not flow into External
  $x->value = $y->value;
}

<<__Policied>>
function return_external(<<__External>> A $x): A {
  // Illegal! This implicitly casts the invariant lump
  return $x;
}

<<__Policied>>
function store_external(<<__External>> A $a, B $b): void {
  // Illegal! This would make $a mutable
  $b->a = $a;

  $b->a->value = 1234;
}

<<__Policied>>
function store_int(A $a, <<__External>> int $x): void {
  // Ok! Immutable external data can be stored
  $a->value = $x;
}

<<__Policied>>
function mix_externals(<<__External>> A $x, <<__External>> A $y): void {
  // Illegal! Externals cannot mix
  $x->value = $y->value;
}
