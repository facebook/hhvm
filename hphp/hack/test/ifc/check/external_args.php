<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class A {
  public int $value = 0;
  public function __construct() {}
}

class B {
  public A $a;

  public function __construct() { $this->a = new A(); }
}

<<Governed>>
function read_external(<<External>> A $x, A $y) : void {
  // Ok! External flows into the function
  $y->value = $x->value;
}

<<Governed>>
function write_external(<<External>> A $x, A $y) : void {
  // Illegal! Implicit does not flow into External
  $x->value = $y->value;
}

<<Governed>>
function return_external(<<External>> A $x): A {
  // Illegal! This implicitly casts the invariant lump
  return $x;
}

<<Governed>>
function store_external(<<External>> A $a, B $b): void {
  // Illegal! This would make $a mutable
  $b->a = $a;

  $b->a->value = 1234;
}

<<Governed>>
function store_int(A $a, <<External>> int $x): void {
  // Ok! Immutable external data can be stored
  $a->value = $x;
}

<<Governed>>
function mix_externals(<<External>> A $x, <<External>> A $y): void {
  // Illegal! Externals cannot mix
  $x->value = $y->value;
}
