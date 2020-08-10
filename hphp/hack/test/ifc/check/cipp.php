<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C {
  <<Policied("A")>>
  public int $a = 0;

  <<Policied("PUBLIC")>>
  public int $pub = 0;
}

// Valid Examples:

<<Cipp>>
function cipp_identity(int $x): int {
  return $x;
}

<<Cipp>>
function cipp_basic(int $x, int $y): int {
  return $x + $y;
}

// Invalid Examples

<<Cipp>>
function write_cipp_into_A(int $x, C $c): void {
  $c->a = $x;
}

<<Cipp>>
function write_cipp_into_PUBLIC(int $x, C $c): void {
  $c->pub = $x;
}

<<Cipp>>
function write_PUBLIC_into_PUBLIC(int $x, C $c): void {
  // This is still illegal because there is a dependency on the PC
  $c->pub = 0;
}
