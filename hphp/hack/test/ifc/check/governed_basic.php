<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C {
  <<__Policied("A")>>
  public int $a = 0;

  <<__Policied("PUBLIC")>>
  public int $pub = 0;
}

// Valid Examples:

<<__Policied>>
function governed_identity(int $x): int {
  return $x;
}

<<__Policied>>
function governed_basic(int $x, int $y): int {
  return $x + $y;
}

<<__Policied("A")>>
function write_a_to_a(int $x, C $c): void {
  $c->a = $x;
}

<<__Policied("PUBLIC")>>
function write_public_to_a(int $x, C $c): void {
  $c->a = $x;
}

// Invalid Examples

<<__Policied>>
function write_existential_into_A(int $x, C $c): void {
  $c->a = $x;
}

<<__Policied>>
function write_existential_into_PUBLIC(int $x, C $c): void {
  $c->pub = $x;
}

<<__Policied>>
function write_PUBLIC_into_PUBLIC(int $x, C $c): void {
  // This is still illegal because there is a dependency on the PC
  $c->pub = 0;
}
