<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class A<T> {
  public function __construct(
    <<Policied("A")>>
    public T $a
  ) {}
}

class B<T> {
  public function __construct(
    <<Policied("B")>>
    public T $b
  ) {}
}

// Identity function that forces input to have policy A
function identity<T>(T $x): T {
  $a = new A($x);
  return $a->a;
}

function write_a_to_b(): B<int> {
  // Illegal because the output of identity has policy A
  return new B(identity(1));
}

function leak_b_to_a(B<int> $x): int {
  // Illegal because the input to identity must flow to A
  return identity($x->b);
}

function f(): void {
  $x = new A(new B(0));
  // Pulling the int out of $x will have the join of both policies
  $y = new A($x->a->b); // no
  $z = new B($x->a->b); // no
}
